"""Optional rendered UI acceptance: python launcher/test_ui.py --url http://127.0.0.1:PORT
Requires Python Playwright and its Chromium browser. Use an isolated --data-dir server.
"""
import argparse
import json
from pathlib import Path
from urllib.request import Request, urlopen

def main():
    from playwright.sync_api import sync_playwright, expect

    parser = argparse.ArgumentParser()
    parser.add_argument('--url', required=True)
    parser.add_argument('--output', type=Path, default=Path('build/launcher-qa/screenshots'))
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    base = args.url.rstrip('/')
    with urlopen(base + '/api/state') as response:
        initial = json.load(response)
    if 'launcher-qa' not in initial['dataDir']:
        raise SystemExit('Refusing to change settings outside an isolated launcher-qa data directory.')
    if initial.get('running'):
        raise SystemExit('Close the game running in the isolated QA launcher before UI acceptance.')
    errors, failed_resources, checks = [], [], []

    def restore_settings():
        body = json.dumps({'settings': initial['settings']}).encode()
        request = Request(base + '/api/settings', data=body, headers={'Content-Type': 'application/json', 'X-Launcher-Token': initial['token']})
        with urlopen(request) as response:
            assert response.status == 200

    try:
        with sync_playwright() as playwright:
            browser = playwright.chromium.launch(headless=True)
            page = browser.new_page(viewport={'width': 1440, 'height': 900}, device_scale_factor=1)
            page.on('pageerror', lambda error: errors.append(str(error)))
            page.on('response', lambda response: failed_resources.append({'url': response.url, 'status': response.status}) if response.status >= 400 and '/api/' not in response.url else None)
            page.goto(base, wait_until='networkidle')
            expect(page.locator('#stage')).to_have_attribute('data-screen', 'title')
            page.screenshot(path=str(args.output / '01-title.png'))
            page.keyboard.press('Enter')
            expect(page.locator('#stage')).to_have_attribute('data-screen', 'main')
            expect(page.get_by_role('button', name='Game Select', exact=True)).to_be_visible()
            page.screenshot(path=str(args.output / '02-main.png'))
            checks.append('Title screen accepts Enter; main menu renders all destinations.')
            page.keyboard.press('ArrowDown')
            expect(page.locator('.menu-button.selected')).to_have_text('PC Boxes')
            page.keyboard.press('Enter')
            expect(page.locator('#stage')).to_have_attribute('data-screen', 'boxes')
            expect(page.get_by_role('button', name='View Boxes', exact=True)).to_be_visible()
            expect(page.locator('.selected-save')).to_contain_text('GEN 3 STORAGE')
            expect(page.locator('.selected-save')).to_contain_text('Emerald')
            page.screenshot(path=str(args.output / '04-boxes.png'))
            page.get_by_role('button', name='See Details', exact=True).click()
            expect(page.locator('#stage')).to_have_attribute('data-screen', 'storage')
            expect(page.locator('.pc-pane')).to_have_count(2)
            expect(page.locator('[data-pc-game="0"] option')).to_have_count(5)
            expect(page.locator('[data-pc-game="1"] option')).to_have_count(5)
            expect(page.locator('#storage-root')).to_have_attribute('aria-busy', 'false')
            page.wait_for_function('() => storageUI && !storageUI.sides.some(side => side.loading)')
            with urlopen(base + '/api/state') as response:
                assert json.load(response).get('running') is None, 'PC Boxes must not launch a game.'
            page.screenshot(path=str(args.output / '04-storage.png'))
            page.keyboard.press('Escape')
            expect(page.locator('#stage')).to_have_attribute('data-screen', 'boxes')
            page.keyboard.press('Escape')
            expect(page.locator('#stage')).to_have_attribute('data-screen', 'main')
            checks.append('Keyboard menus and standalone PC Boxes work; both panes list all five games; Details opens without launching a game; Escape returns to the submenu.')
            page.get_by_role('button', name='Game Select', exact=True).click()
            expect(page.locator('.game-tab')).to_have_count(5)
            expect(page.locator('.game-tab.selected')).to_contain_text('READY TO PLAY')
            expect(page.locator('#game-carousel canvas')).to_be_visible()
            page.wait_for_timeout(1400)
            canvas_bounds = page.locator('#game-carousel canvas').bounding_box()
            assert canvas_bounds['width'] > 1200 and canvas_bounds['height'] > 450, canvas_bounds
            page.screenshot(path=str(args.output / '03-games.png'))
            page.keyboard.press('ArrowLeft')
            expect(page.locator('.game-tab.selected')).to_have_attribute('data-game', '1')
            page.get_by_role('button', name='Add game', exact=True).click()
            expect(page.get_by_role('dialog')).to_contain_text('Add Sapphire')
            page.locator('#import-path').fill(str(Path('build/nonexistent.exe').resolve()))
            page.locator('[data-dialog="confirm"]').click()
            expect(page.get_by_role('alert')).to_contain_text('existing native Windows executable')
            page.keyboard.press('Escape')
            page.keyboard.press('Escape')
            checks.append('Five game cases render; carousel keyboard input works; unavailable game registration validates executable.')
            page.get_by_role('button', name='Mystery Gifts', exact=True).click()
            expect(page.locator('#stage')).to_have_attribute('data-screen', 'gifts')
            page.screenshot(path=str(args.output / '05-gifts.png'))
            page.get_by_role('button', name='Gift Information', exact=True).click()
            expect(page.get_by_role('dialog')).to_contain_text('no configured internet gift service')
            page.get_by_role('button', name='Got it', exact=True).click()
            page.keyboard.press('Escape')
            checks.append('Mystery Gift screen and truthful service information render.')
            page.get_by_role('button', name='Options', exact=True).click()
            page.screenshot(path=str(args.output / '06-options.png'))
            page.get_by_role('button', name='Audio Settings', exact=True).click()
            page.locator('[data-setting="volume"]').fill('3')
            with page.expect_response(lambda response: response.url.endswith('/api/settings') and response.request.method == 'POST') as saved:
                page.get_by_role('button', name='Save settings', exact=True).click()
            assert saved.value.status == 200
            with urlopen(base + '/api/state') as response:
                assert json.load(response)['settings']['volume'] == 3
            page.screenshot(path=str(args.output / '07-audio.png'))
            page.keyboard.press('Escape')
            page.get_by_role('button', name='Display Settings', exact=True).click()
            expect(page.locator('[data-setting]')).to_have_count(6 if initial.get('embedded') else 7)
            page.screenshot(path=str(args.output / '08-display.png'))
            page.keyboard.press('Escape')
            page.get_by_role('button', name='Control Settings', exact=True).click()
            expect(page.locator('.controls-table')).to_contain_text('Hold Tab')
            page.keyboard.press('Escape')
            page.get_by_role('button', name='Language', exact=True).click()
            expect(page.get_by_role('combobox', name='Game language')).to_have_value('English')
            page.keyboard.press('Escape')
            page.get_by_role('button', name='Reset to Default', exact=True).click()
            expect(page.get_by_role('dialog')).to_contain_text('Your adventure and save data are kept')
            page.keyboard.press('Escape')
            checks.append('All options screens render; audio changes persist in actual QA settings.ini; controls/language/reset dialog work.')
            page.set_viewport_size({'width': 960, 'height': 540})
            page.keyboard.press('Escape')
            page.screenshot(path=str(args.output / '09-small-window.png'))
            geometry = page.locator('#stage').bounding_box()
            assert geometry['width'] <= 961 and geometry['height'] <= 541, geometry
            checks.append('Stage fits a 960 x 540 window.')
            browser.close()
    finally:
        restore_settings()
    report = {'checks': checks, 'pageErrors': errors, 'missingResources': failed_resources}
    (args.output / 'ui-report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
    print(json.dumps(report, indent=2))
    if errors:
        raise SystemExit('Browser JavaScript errors detected.')
    if failed_resources:
        raise SystemExit('Launcher resources failed to load.')


if __name__ == "__main__":
    main()
