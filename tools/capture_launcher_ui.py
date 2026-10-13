"""Capture the launcher and verify real fades without starting games or saving settings."""
from pathlib import Path
import argparse
import json
from playwright.sync_api import sync_playwright

IDLE = "() => typeof navigationPending === 'boolean' && typeof sceneTransitions !== 'undefined' && !!sceneTransitions && !navigationPending && !sceneTransitions.busy"
APP_READY = "() => typeof apiConnected === 'boolean' && apiConnected && typeof render === 'function' && document.querySelector('#stage[data-screen]') && !navigationPending && !!sceneTransitions && !sceneTransitions.busy"

# Function expressions avoid Playwright's expression-eval fallback under the page CSP.
FADE_PROBE = r"""() => {
  window.__fadeProbe = null;
  window.__initialFadeTrace = null;
  window.__startFadeProbe = label => {
    const overlay = document.querySelector('#screen-transition');
    const stage = document.querySelector('#stage');
    const probe = {label, samples:[], swaps:[], start:performance.now(), stopped:false};
    const snapshot = () => ({
      ms:Math.round((performance.now()-probe.start)*10)/10,
      opacity:Number(getComputedStyle(overlay).opacity),
      screen:stage.dataset.screen,
      tab:typeof optionsTab === 'undefined' ? null : optionsTab,
      busy:Boolean(sceneTransitions?.busy || navigationPending),
      pointerEvents:getComputedStyle(overlay).pointerEvents
    });
    let lastRoute=stage.dataset.screen, lastTab=optionsTab;
    probe.observer = new MutationObserver(() => {
      const next=snapshot();
      // API refreshes and async art append nodes too. Record navigation swaps,
      // including settings subpages, rather than same-screen refresh noise.
      if(next.screen!==lastRoute || next.tab!==lastTab)probe.swaps.push(next);
      lastRoute=next.screen;lastTab=next.tab;
    });
    probe.observer.observe(document.querySelector('#screen'), {childList:true});
    const sample = () => {
      if(probe.stopped)return;
      const state=snapshot();
      probe.samples.push(state);
      if(label==='initial-reveal' && state.opacity<=.001 && !state.busy && sceneTransitions) {
        window.__initialFadeTrace=window.__stopFadeProbe();
        // Start before the eight-second notice timer can advance the route.
        window.__startFadeProbe('notice-to-title');
        return;
      }
      probe.frame=requestAnimationFrame(sample);
    };
    window.__fadeProbe = probe;
    sample();
  };
  window.__stopFadeProbe = () => {
    const probe=window.__fadeProbe;
    probe.stopped=true;
    cancelAnimationFrame(probe.frame);
    probe.observer.disconnect();
    const overlay=document.querySelector('#screen-transition');
    const result={label:probe.label,samples:probe.samples,swaps:probe.swaps,
      final:{opacity:Number(getComputedStyle(overlay).opacity),
        screen:document.querySelector('#stage').dataset.screen,
        tab:optionsTab,busy:Boolean(sceneTransitions?.busy || navigationPending),
        pointerEvents:getComputedStyle(overlay).pointerEvents}};
    window.__fadeProbe=null;
    return result;
  };
  document.addEventListener('DOMContentLoaded',()=>window.__startFadeProbe('initial-reveal'),{once:true});
}"""


def verify_fade(trace, expected_screen, expected_tab=None, initial=False):
    samples = trace['samples']
    final = trace['final']
    assert final['screen'] == expected_screen, trace
    assert final['tab'] == expected_tab, trace
    assert final['opacity'] <= .001 and not final['busy'], trace
    assert final['pointerEvents'] == 'none', trace
    assert any(.01 < sample['opacity'] < .99 for sample in samples), trace
    if initial:
        assert max(sample['opacity'] for sample in samples) >= .98, trace
    else:
        assert trace['swaps'], trace
        assert all(swap['opacity'] >= .98 for swap in trace['swaps']), trace
        assert any(sample['busy'] for sample in samples), trace
        swap_ms = trace['swaps'][0]['ms']
        assert any(sample['ms'] < swap_ms and .01 < sample['opacity'] < .99 for sample in samples), trace
        assert any(sample['ms'] > swap_ms and .01 < sample['opacity'] < .99 for sample in samples), trace
    return {'transition':trace['label'], 'samples':len(samples),
            'maxOpacity':max(sample['opacity'] for sample in samples),
            'swapOpacity':[swap['opacity'] for swap in trace['swaps']],
            'endOpacity':final['opacity'], 'screen':final['screen'], 'tab':final['tab']}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--url', required=True)
    parser.add_argument('--output', default='build/launcher-visual-check')
    args = parser.parse_args()
    out = Path(args.output)
    out.mkdir(parents=True, exist_ok=True)
    errors, console_errors, traces, transitions = [], [], [], []
    with sync_playwright() as pw:
        browser = pw.chromium.launch(headless=True, args=['--enable-unsafe-swiftshader'])
        page = browser.new_page(viewport={'width':1280,'height':720}, reduced_motion='no-preference')
        page.on('pageerror', lambda error: errors.append(str(error)))
        page.on('console', lambda message: console_errors.append(message.text) if message.type == 'error' else None)
        page.add_init_script("(" + FADE_PROBE + ")();")
        page.goto(args.url, wait_until='domcontentloaded')
        page.wait_for_function('() => window.__initialFadeTrace !== null')
        initial = page.evaluate('() => window.__initialFadeTrace')
        traces.append(initial)
        transitions.append(verify_fade(initial, 'notice', initial=True))

        def capture(name):
            page.screenshot(path=str(out / (name + '.png')))

        def navigate(label, trigger, screen, tab=None):
            page.evaluate('label => window.__startFadeProbe(label)', label)
            trigger()
            page.wait_for_function(IDLE)
            trace = page.evaluate('() => window.__stopFadeProbe()')
            traces.append(trace)
            transitions.append(verify_fade(trace, screen, tab))

        if page.locator('#stage').get_attribute('data-screen') == 'notice':
            capture('notice')
        # Audio preloads may remain active beyond the notice timer; app readiness
        # is explicit and does not depend on all network requests becoming idle.
        page.wait_for_function(APP_READY)
        page.evaluate("() => document.querySelector('[data-action=\"notice-continue\"]')?.click()")
        page.wait_for_function("() => document.querySelector('#stage').dataset.screen === 'title' && !navigationPending && !sceneTransitions.busy")
        notice = page.evaluate('() => window.__stopFadeProbe()')
        traces.append(notice)
        transitions.append(verify_fade(notice, 'title'))
        page.wait_for_selector('#title-effects canvas')
        page.wait_for_timeout(400)
        capture('title')
        theme = page.locator('#stage').get_attribute('data-title-theme')
        navigate('title-to-main', lambda: page.keyboard.press('Enter'), 'main')
        capture('main')
        for route, label in [('games','Game Select'),('boxes','PC Boxes'),('gifts','Mystery Gifts'),('options','Options')]:
            navigate('main-to-' + route, lambda label=label: page.get_by_role('button', name=label, exact=True).click(), route)
            page.wait_for_timeout(1250 if route == 'games' else 350)
            capture(route)
            if route == 'games':
                assert page.locator('#game-carousel canvas').count() == 1
                assert page.locator('.game-tab').count() == 5
            if route == 'options':
                for tab, label in [('game','Game Settings'),('display','Display Settings'),('audio','Audio Settings'),('controls','Control Settings'),('language','Language')]:
                    navigate('options-to-' + tab, lambda label=label: page.get_by_role('button', name=label, exact=True).click(), 'options', tab)
                    capture('settings-' + tab)
                    navigate(tab + '-to-options', lambda: page.keyboard.press('Escape'), 'options')
            navigate(route + '-to-main', lambda: page.keyboard.press('Escape'), 'main')
        browser.close()
    report = {'screens':str(out), 'titleTheme':theme, 'transitions':transitions,
              'pageErrors':errors, 'consoleErrors':console_errors}
    (out / 'fade-traces.json').write_text(json.dumps(traces, indent=2), encoding='utf-8')
    (out / 'capture-report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
    print(json.dumps(report, indent=2))
    if errors or console_errors:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
