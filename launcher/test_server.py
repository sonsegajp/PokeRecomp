"""Integration checks use only temporary saves, settings, executables, and HTTP ports."""
import http.client
import hashlib
import shutil
import importlib.util
import json
from pathlib import Path
import tempfile
import threading
import unittest
from unittest.mock import patch

SPEC = importlib.util.spec_from_file_location("launcher_server", Path(__file__).with_name("server.py"))
server = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(server)


class FakeProcess:
    pid = 8675309
    exitcode = None
    def poll(self):
        return self.exitcode


class Fixture(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        (self.root / "launcher" / "assets").mkdir(parents=True)
        (self.root / "FireRedPC.exe").write_bytes(b"MZfake-native-executable")
        (self.root / "launcher" / "index.html").write_text("<!doctype html><title>launcher</title>")
        (self.root / "launcher" / "assets" / "safe.txt").write_text("safe")
        (self.root / "launcher" / "server.py").write_text("private server source")
        self.commands = []
        self.process = FakeProcess()
        self.process.exitcode = None
        def popen(command, **kwargs):
            self.commands.append((command, kwargs))
            return self.process
        self.app = server.Launcher(self.root, popen=popen)
        self.native_processes = patch.object(server, "windows_processes", return_value=[])
        self.native_processes.start()

    def tearDown(self):
        self.native_processes.stop()
        self.temporary.cleanup()


class LauncherTests(Fixture):
    def test_default_data_dir_prefers_existing_root_save_then_dist(self):
        dist = self.root / "dist"
        dist.mkdir()
        (dist / "firered.sav").write_bytes(b"dist")
        self.assertEqual(server.Launcher(self.root).data_dir, dist)
        (self.root / "firered.sav").write_bytes(b"root")
        self.assertEqual(server.Launcher(self.root).data_dir, self.root)

    def test_settings_native_ranges_preserve_unknowns_and_backup(self):
        settings = self.root / "settings.ini"
        original = "# user settings\nvolume = 8\ncustom_key = 99\nwidescreen = 2\n"
        settings.write_text(original)
        result = self.app.update_settings({"volume": 3, "fullscreen": 1})
        self.assertEqual(result["settings"]["widescreen"], 2)
        self.assertEqual(self.app.settings()["volume"], 3)
        self.assertIn("custom_key = 99", settings.read_text())
        backups = list((self.app.state_dir / "backups").glob("settings.ini.*.bak"))
        self.assertEqual(backups[0].read_text(), original)
        for changes in ({"volume": 11}, {"unknown": 1}, {"volume": "3"}, {"volume": True}, {}):
            with self.assertRaises(server.LauncherError):
                self.app.update_settings(changes)

    def test_boxes_require_save_and_pass_real_native_arguments(self):
        with self.assertRaisesRegex(server.LauncherError, "Create and save"):
            self.app.launch("firered", "boxes")
        save = self.root / "firered.sav"
        contents = bytes(range(256)) * 512
        save.write_bytes(contents)
        result = self.app.launch("firered", "organize")
        self.assertEqual(result["running"]["mode"], "organize")
        command, kwargs = self.commands[0]
        self.assertEqual(command, [str(self.root / "FireRedPC.exe"), "--data-dir", str(self.root), "--save-file", str(save), "--pc-boxes", "organize"])
        self.assertEqual(kwargs["cwd"], str(self.root))
        self.assertEqual(save.read_bytes(), contents)
        backup = list((self.app.state_dir / "backups").glob("firered.sav.*.bak"))[0]
        self.assertEqual(backup.read_bytes(), contents)

    def test_running_game_locks_launch_settings_import_and_tracks_exit(self):
        self.app.launch("firered")
        with self.assertRaisesRegex(server.LauncherError, "Close the running game"):
            self.app.launch("firered")
        with self.assertRaises(server.LauncherError):
            self.app.update_settings({"volume": 3})
        save = self.root / "backup.sav"
        save.write_bytes(b"\xff" * 131072)
        with self.assertRaises(server.LauncherError):
            self.app.import_save(str(save))
        self.process.exitcode = 0
        self.assertIsNone(self.app.running())
        self.assertEqual(self.app.state()["lastExit"]["exitCode"], 0)
        self.app.update_settings({"volume": 3})

    def test_nonzero_native_exit_has_actionable_error(self):
        expected = {2: "valid FireRed", 3: "previous save", 4: "already open", 99: "unexpectedly"}
        for code, phrase in expected.items():
            self.process.exitcode = None
            self.app.launch("firered")
            self.process.exitcode = code
            state = self.app.state()
            self.assertIsNone(state["running"])
            self.assertIn(phrase, state["lastExit"]["error"])
            self.assertEqual(state["lastExit"]["exitCode"], code)

    def test_embedding_handle_is_passed_only_to_bundled_firered(self):
        self.app.embed_parent = 123456
        self.assertTrue(self.app.state()["embedded"])
        self.app.launch("firered")
        self.assertEqual(self.commands[-1][0][-2:], ["--launcher-parent", "123456"])
        self.process.exitcode = 0
        self.app.running()
        other = self.root / "RubyPC.exe"
        other.write_bytes(b"MZnative")
        self.app.import_game("ruby", str(other))
        self.process.exitcode = None
        self.app.launch("ruby")
        self.assertEqual(self.commands[-1][0], [str(other)])
        with self.assertRaises(ValueError):
            server.Launcher(self.root, embed_parent=-1)

    def test_library_persists_across_instance_state_directories(self):
        library = self.root / "shared-library"
        first = server.Launcher(self.root, state_dir=self.root / "instance-one", library_dir=library)
        executable = self.root / "RubyPC.exe"
        executable.write_bytes(b"MZnative")
        first.import_game("ruby", str(executable))
        save = self.root / "linked-firered.sav"
        shutil.copyfile(server.ROOT / "build" / "deposit.sav", save)
        first.storage_import("firered", str(save))
        second = server.Launcher(self.root, state_dir=self.root / "instance-two", library_dir=library)
        self.assertEqual(second.executable("ruby"), executable)
        self.assertEqual(second.storage_path("firered"), save)
        self.assertEqual(second.journal_dir, library / "gen3-transfers")
        self.assertFalse((second.state_dir / "games.json").exists())
        self.assertTrue((library / "games.json").is_file())
        self.assertTrue((library / "saves.json").is_file())

    def test_storage_reads_real_save_without_launch_and_rejects_duplicate_link(self):
        source = self.root / "firered.sav"
        shutil.copyfile(server.ROOT / "build" / "deposit.sav", source)
        raw = source.read_bytes()
        result = self.app.storage("firered")
        self.assertEqual(result["boxes"][0]["mons"][0]["nickname"], "RATTATA")
        self.assertEqual(len(result["boxes"]), 14)
        self.assertEqual(result["revision"], hashlib.sha256(raw).hexdigest())
        self.assertFalse(self.commands)
        self.assertEqual(source.read_bytes(), raw)
        self.assertTrue(next(game for game in self.app.storage()["games"] if game["id"] == "firered")["registered"])
        with self.assertRaisesRegex(server.LauncherError, "already linked"):
            self.app.storage_import("leafgreen", str(source))
        invalid = self.root / "broken.sav"
        invalid.write_bytes(b"broken save")
        with self.assertRaises(server.LauncherError):
            self.app.storage_import("ruby", str(invalid))
        self.assertFalse(self.app.storage_path("ruby", required=False))

    def test_storage_rejects_running_game_and_requires_revisions(self):
        source = self.root / "firered.sav"
        shutil.copyfile(server.ROOT / "build" / "deposit.sav", source)
        self.app.launch("firered")
        with self.assertRaisesRegex(server.LauncherError, "Close the running game"):
            self.app.storage("firered")
        with self.assertRaises(server.LauncherError):
            self.app.storage_import("firered", str(source))
        self.process.exitcode = 0
        self.app.running()
        with self.assertRaisesRegex(server.LauncherError, "revisions"):
            self.app.storage_move(dict(sourceGame="firered", targetGame="firered", sourceBox=0, sourceSlot=0, targetBox=0, targetSlot=1))

    def test_native_game_uses_explicit_linked_save_without_copying(self):
        save = self.root / "trainer-backup.sav"
        shutil.copyfile(server.ROOT / "build" / "deposit.sav", save)
        self.app.storage_import("firered", str(save))
        self.app.launch("firered")
        command = self.commands[-1][0]
        self.assertEqual(command[-2:], ["--save-file", str(save)])
        self.assertFalse((self.root / "firered.sav").exists())
        self.assertEqual(self.app.state()["savePath"], str(save))
        self.process.exitcode = 0
        self.app.running()
        save.unlink()
        with self.assertRaisesRegex(server.LauncherError, "linked FireRed save cannot be found"):
            self.app.launch("firered")

    def test_legacy_native_save_guard_allows_separate_profiles_only(self):
        preferred = self.root / "dist" / "launcher-native" / "FireRedPC.exe"
        preferred.parent.mkdir(parents=True)
        preferred.write_bytes(b"MZnative")
        legacy = self.root / "FireRedPC.exe"
        with patch.object(server, "windows_processes", return_value=[(77, legacy)]):
            with patch.object(server, "native_process_save_paths", return_value={77: self.root / "separate" / "firered.sav"}):
                self.app.require_idle()
            self.app._legacy_process_saves.clear()
            with patch.object(server, "native_process_save_paths", return_value={77: self.root / "firered.sav"}):
                with self.assertRaisesRegex(server.LauncherError, "older FireRed"):
                    self.app.require_idle()
            self.app._legacy_process_saves.clear()
            with patch.object(server, "native_process_save_paths", return_value={}):
                with self.assertRaisesRegex(server.LauncherError, "location cannot be verified"):
                    self.app.require_idle()

    def test_external_native_instance_locks_files(self):
        with patch.object(server, "windows_processes", return_value=[(123, self.root / "FireRedPC.exe")]):
            self.assertTrue(self.app.running()["external"])
            with self.assertRaises(server.LauncherError):
                self.app.launch("firered")

    def test_explicit_native_executable_registration_only(self):
        with self.assertRaises(server.LauncherError):
            self.app.launch("ruby")
        rom = self.root / "ruby.gba"
        rom.write_bytes(b"MZnot-exe")
        with self.assertRaises(server.LauncherError):
            self.app.import_game("ruby", str(rom))
        executable = self.root / "RubyPC.exe"
        executable.write_bytes(b"MZnative")
        self.app.import_game("ruby", str(executable))
        self.assertEqual(server.Launcher(self.root).executable("ruby"), executable)
        with self.assertRaises(server.LauncherError):
            self.app.launch("ruby", "boxes")
        self.app.launch("ruby")
        self.assertEqual(self.commands[0][0], [str(executable)])

    def test_save_import_is_atomic_backup_first_and_size_checked(self):
        target = self.root / "firered.sav"
        original = b"a" * 131072
        target.write_bytes(original)
        source = self.root / "chosen.sav"
        source.write_bytes(b"bad")
        with self.assertRaises(server.LauncherError):
            self.app.import_save(str(source))
        self.assertEqual(target.read_bytes(), original)
        source.write_bytes(b"b" * 131072)
        with self.assertRaises(server.LauncherError):
            self.app.import_save(str(source))
        self.assertEqual(target.read_bytes(), original)
        shutil.copyfile(server.ROOT / "build" / "deposit.sav", source)
        self.app.import_save(str(source))
        self.assertEqual(target.read_bytes(), source.read_bytes())
        self.assertEqual(list((self.app.state_dir / "backups").glob("*.bak"))[0].read_bytes(), original)


class HTTPTests(Fixture):
    def setUp(self):
        super().setUp()
        self.http = server.make_server(self.app)
        self.thread = threading.Thread(target=self.http.serve_forever, daemon=True)
        self.thread.start()
        self.port = self.http.server_address[1]

    def tearDown(self):
        self.http.shutdown()
        self.http.server_close()
        self.thread.join()
        super().tearDown()

    def request(self, method, path, body=None, headers=None):
        connection = http.client.HTTPConnection("127.0.0.1", self.port)
        connection.request(method, path, json.dumps(body) if body is not None else None, headers or {})
        response = connection.getresponse()
        result = response.status, response.read()
        connection.close()
        return result

    def test_http_static_traversal_and_private_file_isolation(self):
        self.assertEqual(self.request("GET", "/")[0], 200)
        self.assertEqual(self.request("GET", "/assets/safe.txt")[0], 200)
        for path in ("/server.py", "/state/games.json", "/assets/../server.py", "/assets/%2e%2e/server.py",
                     "/assets/%2e%2e%5cserver.py", "/assets/../../settings.ini"):
            self.assertEqual(self.request("GET", path)[0], 404, path)

    def test_mutation_requires_secret_and_same_origin(self):
        status, payload = self.request("GET", "/api/state")
        self.assertEqual(status, 200)
        token = json.loads(payload)["token"]
        body = {"settings": {"volume": 3}}
        self.assertEqual(self.request("POST", "/api/settings", body)[0], 403)
        self.assertEqual(self.request("POST", "/api/settings", body,
                                     {"X-Launcher-Token": token, "Origin": "https://hostile.example"})[0], 403)
        self.assertEqual(self.request("POST", "/api/settings", body,
                                     {"X-Launcher-Token": token, "Origin": f"http://127.0.0.1:{self.port}"})[0], 200)
        self.assertEqual(self.request("GET", "/api/state", headers={"Host": "hostile.example"})[0], 403)

    def test_http_storage_transfer_preserves_record_and_rejects_stale_revision(self):
        from test_gen3 import fixture
        source = self.root / "firered.sav"
        shutil.copyfile(server.ROOT / "build" / "deposit.sav", source)
        record = server.gen3.SaveImage(source.read_bytes(), "firered").mon(0, 0)
        target = self.root / "leafgreen.sav"
        target.write_bytes(fixture("leafgreen"))
        headers = {"X-Launcher-Token": self.app.token}
        self.assertEqual(self.request("POST", "/api/storage/import", {"game": "leafgreen", "path": str(target)})[0], 403)
        status, _ = self.request("POST", "/api/storage/import", {"game": "leafgreen", "path": str(target)}, headers)
        self.assertEqual(status, 200)
        status, payload = self.request("GET", "/api/storage")
        self.assertEqual(status, 200)
        self.assertEqual(sum(game["registered"] for game in json.loads(payload)["games"]), 2)
        a = json.loads(self.request("GET", "/api/storage?game=firered")[1])
        b = json.loads(self.request("GET", "/api/storage?game=leafgreen")[1])
        move = dict(sourceGame="firered", targetGame="leafgreen", sourceBox=0, sourceSlot=0,
                    targetBox=1, targetSlot=3, sourceRevision=a["revision"], targetRevision=b["revision"])
        status, payload = self.request("POST", "/api/storage/move", move, headers)
        self.assertEqual(status, 200, payload)
        result = json.loads(payload)
        self.assertTrue(result["ok"])
        self.assertEqual(server.gen3.SaveImage(source.read_bytes(), "firered").mon(0, 0), bytes(80))
        self.assertEqual(server.gen3.SaveImage(target.read_bytes(), "leafgreen").mon(1, 3), record)
        before_source, before_target = source.read_bytes(), target.read_bytes()
        self.assertEqual(self.request("POST", "/api/storage/move", move, headers)[0], 409)
        self.assertEqual(source.read_bytes(), before_source)
        self.assertEqual(target.read_bytes(), before_target)
        self.assertFalse(self.commands)
        self.assertTrue(all(Path(path).is_file() for path in result["backups"]))
        self.assertEqual(self.request("GET", "/api/storage?game=missing")[0], 400)
        self.assertEqual(self.request("GET", "/api/storage?game=ruby")[0], 404)

    def test_launch_validation_and_quit_lock(self):
        headers = {"X-Launcher-Token": self.app.token}
        self.assertEqual(self.request("POST", "/api/launch", {"game": "../../cmd.exe"}, headers)[0], 400)
        self.assertEqual(self.request("POST", "/api/launch", {"game": "firered", "mode": "--bad"}, headers)[0], 400)
        self.assertEqual(self.request("POST", "/api/launch", {"game": "firered"}, headers)[0], 200)
        self.assertEqual(self.request("POST", "/api/quit", {}, headers)[0], 409)


if __name__ == "__main__":
    unittest.main()
