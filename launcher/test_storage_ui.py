"""Real standalone box-UI acceptance on an isolated QA launcher.
The FireRed record comes from build/deposit.sav; the empty LeafGreen layout is
synthetic source-derived test scaffolding, not a user cartridge save.
"""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import tempfile
from urllib.request import Request, urlopen
from playwright.sync_api import sync_playwright, expect
import gen3
from test_gen3 import fixture

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--url', default='http://127.0.0.1:49641')
    args = parser.parse_args()
    base = args.url.rstrip('/')

    def api(path, body=None):
        headers = {'Content-Type': 'application/json'}
        if body is not None:
            headers['X-Launcher-Token'] = token
        request = Request(base + '/api/' + path, data=json.dumps(body).encode() if body is not None else None, headers=headers)
        with urlopen(request, timeout=15) as response:
            return json.load(response)

    state = api('state')
    token = state['token']
    qa_dir = Path(state['dataDir']).resolve()
    if 'launcher-qa' not in qa_dir.name or state['running']:
        raise SystemExit('This test requires an idle isolated launcher-qa data directory.')
    library = Path(state['libraryDir'])
    registry = library / 'saves.json'
    previous_registry = registry.read_bytes() if registry.exists() else None
    output = qa_dir / 'screenshots'
    output.mkdir(exist_ok=True)
    errors, launches = [], []
    try:
        with tempfile.TemporaryDirectory(prefix='storage-ui-test-', dir=qa_dir) as temporary:
            source = Path(temporary) / 'firered.sav'
            target = Path(temporary) / 'leafgreen.sav'
            shutil.copyfile(Path(__file__).resolve().parents[1] / 'build/deposit.sav', source)
            target.write_bytes(fixture('leafgreen'))
            original_source, original_target = source.read_bytes(), target.read_bytes()
            original_record = gen3.SaveImage(original_source, 'firered').mon(0, 0)
            api('storage/import', {'game': 'firered', 'path': str(source)})
            with sync_playwright() as playwright:
                browser = playwright.chromium.launch(headless=True)
                page = browser.new_page(viewport={'width': 1440, 'height': 900}, device_scale_factor=1)
                page.on('pageerror', lambda error: errors.append(str(error)))
                page.on('request', lambda request: launches.append(request.url) if '/api/launch' in request.url else None)
                page.goto(base, wait_until='networkidle')
                page.keyboard.press('Enter')
                page.get_by_role('button', name='PC Boxes', exact=True).click()
                page.get_by_role('button', name='View Boxes', exact=True).click()
                expect(page.locator('#stage')).to_have_attribute('data-screen', 'storage')
                expect(page.locator('[data-pc-game="0"]')).to_have_value('firered')
                expect(page.locator('[data-pc-slot="0"][data-side="0"]')).to_have_attribute('aria-label', 'Slot 1: RATTATA, level 5')
                page.locator('[data-pc-game="1"]').select_option('leafgreen')
                expect(page.locator('#pc-path-1')).to_be_visible()
                page.locator('#pc-path-1').fill(str(target))
                page.locator('[data-pc-action="link-path"][data-side="1"]').click()
                expect(page.locator('[data-pc-box="1"]')).to_be_visible()
                expect(page.locator('[data-pc-box="0"] option')).to_have_count(14)
                expect(page.locator('[data-pc-game="1"] option')).to_have_count(5)
                expect(page.locator('[role="gridcell"]')).to_have_count(60)
                page.locator('[data-pc-box="1"]').select_option('1')
                page.locator('[data-pc-slot="0"][data-side="0"]').click()
                expect(page.locator('.pc-mon-info')).to_contain_text('Quirky')
                expect(page.locator('.pc-mon-info')).to_contain_text('TACKLE')
                page.locator('[data-pc-slot="1"][data-side="1"]').click()
                expect(page.locator('[data-pc-action="move"]')).to_be_enabled()
                page.screenshot(path=str(output / '10-storage-before-transfer.png'))
                page.locator('[data-pc-action="move"]').click()
                expect(page.get_by_role('dialog')).to_contain_text('Move RATTATA?')
                expect(page.get_by_role('dialog')).to_contain_text('LeafGreen')
                with page.expect_response(lambda response: response.url.endswith('/api/storage/move')) as moved:
                    page.locator('[data-dialog="confirm"]').click()
                assert moved.value.status == 200, moved.value.text()
                result = moved.value.json()
                expect(page.locator('[data-pc-slot="0"][data-side="0"]')).to_have_attribute('aria-label', 'Slot 1: Empty')
                expect(page.locator('[data-pc-slot="1"][data-side="1"]')).to_have_attribute('aria-label', 'Slot 2: RATTATA, level 5')
                expect(page.locator('.pc-status')).to_contain_text('Saved directly to both game saves')
                page.screenshot(path=str(output / '11-storage-after-transfer.png'))
                assert gen3.SaveImage(source.read_bytes(), 'firered').mon(0, 0) == bytes(80)
                assert gen3.SaveImage(target.read_bytes(), 'leafgreen').mon(1, 1) == original_record
                assert api('state')['running'] is None
                assert not launches, launches
                assert not errors, errors
                assert len(result['backups']) == 2
                backup_contents = [Path(path).read_bytes() for path in result['backups']]
                assert original_source in backup_contents and original_target in backup_contents
                browser.close()
            report = {'passed': True, 'source': 'Captured FireRed Rattata Lv5',
                      'destination': 'Source-derived empty LeafGreen test layout, box 2 slot 2',
                      'recordHash': hashlib.sha256(original_record).hexdigest(),
                      'recordBytesPreserved': 80, 'gameLaunchRequests': launches,
                      'javascriptErrors': errors, 'backupCount': len(result['backups']),
                      'registeredGames': 5, 'boxesPerGame': 14, 'visibleSlots': 60,
                      'userSavesTouched': False}
            (output / 'storage-ui-report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
            print(json.dumps(report, indent=2))
    finally:
        if previous_registry is None:
            if registry.exists():
                registry.unlink()
        else:
            registry.write_bytes(previous_registry)



if __name__ == "__main__":
    main()
