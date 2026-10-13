"""Windows integration test for our native host, using only an isolated save copy.

python tests/launcher_host.py --exe build/launcher-host-final/PokeRecomp.exe
Exercises the actual SDL child messages, native confirmation and monitor bounds.
"""
import argparse
import ctypes as C
from ctypes import wintypes as W
import hashlib
import json
from pathlib import Path
import shutil
import socket
import subprocess
import tempfile
import time
from urllib.request import Request, urlopen
from playwright.sync_api import sync_playwright

ROOT = Path(__file__).resolve().parents[1]
u = C.WinDLL('user32', use_last_error=True)
CALLBACK = C.WINFUNCTYPE(W.BOOL, W.HWND, W.LPARAM)
u.EnumWindows.argtypes = [CALLBACK, W.LPARAM]
u.EnumChildWindows.argtypes = [W.HWND, CALLBACK, W.LPARAM]
u.GetWindowThreadProcessId.argtypes = [W.HWND, C.POINTER(W.DWORD)]
u.GetParent.argtypes = [W.HWND]
u.GetParent.restype = W.HWND
u.GetWindowLongPtrW.argtypes = [W.HWND, C.c_int]
u.GetWindowLongPtrW.restype = C.c_ssize_t
u.GetWindowRect.argtypes = [W.HWND, C.POINTER(W.RECT)]
u.GetClientRect.argtypes = [W.HWND, C.POINTER(W.RECT)]
u.GetWindowTextW.argtypes = [W.HWND, W.LPWSTR, C.c_int]
u.GetClassNameW.argtypes = [W.HWND, W.LPWSTR, C.c_int]
u.IsWindowVisible.argtypes = [W.HWND]
u.PostMessageW.argtypes = [W.HWND, W.UINT, W.WPARAM, W.LPARAM]
u.MonitorFromWindow.argtypes = [W.HWND, W.DWORD]
u.MonitorFromWindow.restype = W.HMONITOR
u.GetMonitorInfoW.argtypes = [W.HMONITOR, C.c_void_p]
u.GetForegroundWindow.restype = W.HWND
u.GetAncestor.argtypes = [W.HWND, W.UINT]
u.GetAncestor.restype = W.HWND
u.SetProcessDPIAware()


class MonitorInfo(C.Structure):
    _fields_ = [('size', W.DWORD), ('monitor', W.RECT), ('work', W.RECT), ('flags', W.DWORD)]


def bounds(hwnd, client=False):
    rect = W.RECT()
    assert (u.GetClientRect if client else u.GetWindowRect)(hwnd, C.byref(rect))
    return [rect.left, rect.top, rect.right, rect.bottom]


def label(hwnd, class_name=False):
    value = C.create_unicode_buffer(1024)
    (u.GetClassNameW if class_name else u.GetWindowTextW)(hwnd, value, len(value))
    return value.value


def windows(pid, parent=None):
    result = []

    @CALLBACK
    def visit(hwnd, unused):
        owner = W.DWORD()
        u.GetWindowThreadProcessId(hwnd, C.byref(owner))
        if owner.value == pid and u.IsWindowVisible(hwnd):
            result.append(hwnd)
        return True

    if parent:
        u.EnumChildWindows(parent, visit, 0)
    else:
        u.EnumWindows(visit, 0)
    return result


def wait_for(check, description, timeout=15):
    end = time.monotonic() + timeout
    while time.monotonic() < end:
        value = check()
        if value:
            return value
        time.sleep(.1)
    raise AssertionError('Timed out: ' + description)


def key(hwnd, value):
    scan = u.MapVirtualKeyW(value, 0)
    assert u.PostMessageW(hwnd, 0x100, value, 1 | scan << 16)
    assert u.PostMessageW(hwnd, 0x101, value, 1 | scan << 16 | 0xC0000000)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--exe', type=Path, default=ROOT / 'dist/launcher/PokeRecomp.exe')
    args = parser.parse_args()
    work = Path(tempfile.mkdtemp(prefix='launcher-qa-host-', dir=ROOT / 'build'))
    fixture = ROOT / 'build/deposit.sav'
    shutil.copyfile(fixture, work / 'firered.sav')
    original = hashlib.sha256(fixture.read_bytes()).hexdigest()
    (work / 'settings.ini').write_text('fullscreen=0\nvolume=8\nvsync=1\n', encoding='ascii')
    with socket.socket() as debug_socket:
        debug_socket.bind(('127.0.0.1', 0))
        debug_port = debug_socket.getsockname()[1]
    host = subprocess.Popen([str(args.exe.resolve()), '--root', str(ROOT), '--data-dir', str(work), '--debug-port', str(debug_port)], cwd=ROOT)
    playwright = sync_playwright().start()
    host_window = game_window = 0
    report = {'profile': str(work), 'checks': []}
    session = ROOT / 'launcher/state' / ('native-' + str(host.pid)) / 'session.json'
    try:
        wait_for(session.exists, 'service session')
        base = json.loads(session.read_text())['url']

        def state():
            with urlopen(base + '/api/state', timeout=4) as response:
                return json.load(response)

        token = state()['token']

        def post(route, body):
            request = Request(base + '/api/' + route, data=json.dumps(body).encode(),
                              headers={'Content-Type': 'application/json', 'X-Launcher-Token': token})
            with urlopen(request, timeout=10) as response:
                return json.load(response)

        host_window = wait_for(lambda: next((w for w in windows(host.pid) if 'GEN 3' in label(w)), None), 'host window')
        wait_for(lambda: session.with_name('host.log').exists() and 'Ready ' in session.with_name('host.log').read_text(), 'WebView ready')
        browser = playwright.chromium.connect_over_cdp('http://127.0.0.1:' + str(debug_port))
        page = browser.contexts[0].pages[0]
        page.wait_for_url(base + '/**', wait_until='domcontentloaded')
        page.wait_for_function('() => typeof go === "function" && apiConnected && !navigationPending && !sceneTransitions?.busy')
        def audio_state():
            return page.evaluate('''() => Object.fromEntries([...document.querySelectorAll('[data-audio-track]')].map(a=>[a.dataset.audioTrack,{paused:a.paused,volume:a.volume,time:a.currentTime,muted:a.muted,ready:a.readyState,error:a.error?.message||null}]))''')

        if page.locator('#stage[data-screen="notice"]').count():
            page.get_by_role('button', name='CONTINUE', exact=False).click()
            page.wait_for_function('() => !navigationPending && !sceneTransitions?.busy')
        if page.locator('#stage[data-screen="title"]').count():
            page.wait_for_function('() => document.querySelector("[data-audio-track=title]")?.volume >= .199')
            title_audio = audio_state()
            assert not title_audio['title']['paused'] and not title_audio['title']['muted']
            assert title_audio['title']['ready'] >= 3 and not title_audio['title']['error']
            assert title_audio['menu']['paused']
            report['titleAudio'] = title_audio['title']
            page.evaluate('''() => {window.audioFadeSamples=[];const sample=()=>{const a=document.querySelector('[data-audio-track=menu]');audioFadeSamples.push({time:performance.now(),volume:a.volume,paused:a.paused});if(audioFadeSamples.length<160)requestAnimationFrame(sample);};requestAnimationFrame(sample);}''')
            page.keyboard.press('Enter')
            page.wait_for_function('() => !navigationPending && !sceneTransitions?.busy')
            page.wait_for_function('() => document.querySelector("[data-audio-track=menu]")?.volume >= .199')
            fade_samples = page.evaluate('audioFadeSamples')
            assert len({round(s['volume'], 3) for s in fade_samples if 0 < s['volume'] < .199}) >= 5, fade_samples
            report['menuFadeSamples'] = fade_samples
        menu_before = audio_state()['menu']
        page.get_by_role('button', name='Game Select', exact=True).click()
        page.wait_for_function('() => route === "games" && !navigationPending && !sceneTransitions?.busy')
        menu_after = audio_state()['menu']
        assert not menu_after['paused'] and menu_after['time'] > menu_before['time']
        assert .199 <= menu_after['volume'] <= .201
        report['checks'].append('Native title and menu audio decode and play; music fades through intermediate levels to 20% at the default master setting.')
        initial = bounds(host_window)
        assert u.GetWindowLongPtrW(host_window, -16) & 0xC00000
        post('settings', {'settings': {'fullscreen': 1}})
        wait_for(lambda: not u.GetWindowLongPtrW(host_window, -16) & 0xC00000, 'borderless fullscreen')
        monitor = MonitorInfo()
        monitor.size = C.sizeof(monitor)
        assert u.GetMonitorInfoW(u.MonitorFromWindow(host_window, 2), C.byref(monitor))
        screen = [monitor.monitor.left, monitor.monitor.top, monitor.monitor.right, monitor.monitor.bottom]
        assert bounds(host_window) == screen, (bounds(host_window), screen)
        report['fullscreenBounds'] = screen
        report['checks'].append('Saved fullscreen setting removes host title/borders and fills the monitor.')
        post('settings', {'settings': {'fullscreen': 0}})
        wait_for(lambda: bounds(host_window) == initial, 'windowed bounds restored')
        page.locator('[data-action="confirm"]').click()
        game_pid = wait_for(lambda: (state().get('running') or {}).get('pid'), 'game process')
        game_window = wait_for(lambda: next(iter(windows(game_pid, host_window)), None), 'embedded game HWND')
        parent = u.GetParent(game_window)
        assert u.GetAncestor(game_window, 2) == host_window
        assert u.GetWindowLongPtrW(game_window, -16) & 0x40000000
        assert not u.GetWindowLongPtrW(game_window, -16) & 0xC00000
        assert bounds(game_window) == bounds(parent) == bounds(host_window, client=False) or bounds(game_window, True) == bounds(host_window, True)
        assert not windows(game_pid), 'Separate top-level game window'
        assert not any('Return to launcher' in label(w) or 'Save your adventure' in label(w) for w in windows(host.pid, host_window))
        muted_during_game = audio_state()
        assert muted_during_game['title']['paused'] and muted_during_game['menu']['paused']
        report['checks'].append('Game is a borderless child filling the launcher client area, with no gameplay toolbar.')
        key(game_window, 0x7A)
        wait_for(lambda: state()['settings']['fullscreen'] == 1, 'native F11 setting')
        wait_for(lambda: bounds(host_window) == screen, 'F11 fullscreen host')
        wait_for(lambda: bounds(game_window) == screen, 'F11 fullscreen game')
        report['checks'].append('F11 inside SDL gameplay persists fullscreen and resizes both host and game to the monitor.')
        time.sleep(1)
        if u.GetForegroundWindow() == host_window:
            from PIL import ImageGrab
            ImageGrab.grab(bbox=tuple(screen)).save(work / 'fullscreen-game.png')
            report['capture'] = str(work / 'fullscreen-game.png')
        key(game_window, 0x1B)
        dialog = wait_for(lambda: next((w for w in windows(host.pid) if label(w, True) == '#32770'), None), 'Esc confirmation')
        assert 'Return to launcher?' == label(dialog)
        texts = [label(w) for w in windows(host.pid, dialog)]
        assert any('progress since your last in-game save' in t for t in texts), texts
        u.PostMessageW(dialog, 0x111, 7, 0)  # Standard MessageBox IDNO, our test process only.
        wait_for(lambda: not u.IsWindowVisible(dialog), 'cancel confirmation')
        assert state()['running']['pid'] == game_pid
        report['checks'].append('Esc opens the native confirmation; No returns to the same running game.')
        key(game_window, 0x7A)
        wait_for(lambda: state()['settings']['fullscreen'] == 0, 'native F11 windowed setting')
        wait_for(lambda: bounds(host_window) == initial, 'native F11 restores original host size')
        key(game_window, 0x1B)
        dialog = wait_for(lambda: next((w for w in windows(host.pid) if label(w, True) == '#32770'), None), 'second Esc confirmation')
        u.PostMessageW(dialog, 0x111, 6, 0)  # Standard MessageBox IDYES.
        wait_for(lambda: state()['running'] is None, 'return to launcher')
        assert hashlib.sha256((work / 'firered.sav').read_bytes()).hexdigest() == original
        report['checks'].append('Second F11 restores windowed size; Esc then Yes returns to launcher without altering the save.')
        page.wait_for_function('() => !document.hidden && !state.running && !pending && !navigationPending && !sceneTransitions?.busy', timeout=15000)
        page.keyboard.press('Escape')
        page.wait_for_function('() => route === "main" && !navigationPending && !sceneTransitions?.busy')
        page.wait_for_function('() => document.querySelector("[data-audio-track=menu]")?.volume >= .199')
        report['returnAudio'] = audio_state()
        assert not report['returnAudio']['menu']['paused'] and report['returnAudio']['title']['paused']
        for menu_name, destination in [('Options', 'options'), ('PC Boxes', 'boxes'), ('Mystery Gifts', 'gifts'), ('Game Select', 'games')]:
            before = audio_state()['menu']
            page.get_by_role('button', name=menu_name, exact=True).click()
            page.wait_for_function('(destination) => route === destination && !navigationPending && !sceneTransitions?.busy', arg=destination)
            after = audio_state()['menu']
            assert not after['paused'] and after['time'] > before['time'] and .199 <= after['volume'] <= .201
            page.keyboard.press('Escape')
            page.wait_for_function('() => route === "main" && !navigationPending && !sceneTransitions?.busy')
        report['checks'].append('After returning, mouse clicks open all four menus and Escape navigates back each time.')
        report['checks'].append('Menu music resumes after gameplay and its playback time advances continuously across every menu at 20% volume.')
        page.evaluate('''() => {
            navigationPending=true; pending=true; launchingGame=true;
            sceneTransitions.run(() => new Promise(() => {}));
        }''')
        page.wait_for_timeout(180)
        page.evaluate("() => window.dispatchEvent(new Event('launcher-game-return'))")
        page.wait_for_function('() => !pending && !launchingGame && !navigationPending && !sceneTransitions?.busy && getComputedStyle(document.querySelector("#screen-transition")).pointerEvents === "none"')
        page.get_by_role('button', name='Options', exact=True).click()
        page.wait_for_function('() => route === "options" && !navigationPending && !sceneTransitions?.busy')
        report['checks'].append('A return notification cancels an interrupted transition and clears stale launch/input locks.')
        u.PostMessageW(host_window, 0x10, 0, 0)
        assert host.wait(timeout=15) == 0
        (work / 'host-report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
        print(json.dumps({key:value for key,value in report.items() if key != 'menuFadeSamples'}, indent=2))
    finally:
        playwright.stop()
        if host.poll() is None:
            for dialog in windows(host.pid):
                if label(dialog, True) == '#32770':
                    u.PostMessageW(dialog, 0x111, 7, 0)
            if game_window:
                u.PostMessageW(game_window, 0x10, 0, 0)
            time.sleep(.5)
            if host_window:
                u.PostMessageW(host_window, 0x10, 0, 0)


if __name__ == '__main__':
    main()
