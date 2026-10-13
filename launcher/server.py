#!/usr/bin/env python3
"""Loopback-only PokeRecomp launcher. Python standard library, no installation needed."""
from __future__ import annotations

import argparse
import ctypes
import json
import mimetypes
import os
from pathlib import Path
import re
import secrets
import shutil
import subprocess
import sys
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import parse_qs, unquote, urlsplit

try:
    from . import gen3
except ImportError:
    import gen3

PROCESS_CREATION_TIMES = {}
ROOT = Path(__file__).resolve().parent.parent
DEFAULTS = dict(window_scale=4, fullscreen=0, filter=1, scaling=0,
                color_correct=0, widescreen=0, vsync=1, volume=8,
                hq_audio=1, fast_forward=4)
RANGES = dict(window_scale=(1, 8), fullscreen=(0, 1), filter=(0, 4),
              scaling=(0, 2), color_correct=(0, 1), widescreen=(0, 3),
              vsync=(0, 1), volume=(0, 10), hq_audio=(0, 1), fast_forward=(2, 8))
GAME_TITLES = {"ruby": "Pokémon Ruby Version", "sapphire": "Pokémon Sapphire Version",
               "firered": "Pokémon FireRed Version", "leafgreen": "Pokémon LeafGreen Version",
               "emerald": "Pokémon Emerald Version"}
# Games are DLLs in games/ that PokeRecompGame.exe runs; each plays off the
# .pkmn file extracted from the player's own ROM (docs/GAME_DLL.md).
RUNNER = "PokeRecompGame.exe"
ROM_ERRORS = {1: "That isn't a {title} ROM. Choose your own {title} cartridge file (.gba, USA).",
              2: "The ROM file could not be read. Check that it still exists and try again.",
              3: "The extracted game could not be saved. Check free space and folder permissions."}
BOX_MODES = {"boxes": "view", "organize": "organize", "move": "move", "details": "view",
             "deposit": "deposit", "withdraw": "withdraw", "items": "items"}


class LauncherError(Exception):
    def __init__(self, message, status=400):
        super().__init__(message)
        self.status = status


def atomic_text(path: Path, content: str):
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + ".tmp")
    temporary.write_text(content, encoding="utf-8")
    os.replace(temporary, path)


def windows_processes():
    """Resolve native process paths without spawning shell commands."""
    if os.name != "nt":
        return []
    from ctypes import wintypes
    class PROCESSENTRY32W(ctypes.Structure):
        _fields_ = [("dwSize", wintypes.DWORD), ("cntUsage", wintypes.DWORD),
                    ("th32ProcessID", wintypes.DWORD), ("th32DefaultHeapID", ctypes.c_size_t),
                    ("th32ModuleID", wintypes.DWORD), ("cntThreads", wintypes.DWORD),
                    ("th32ParentProcessID", wintypes.DWORD), ("pcPriClassBase", wintypes.LONG),
                    ("dwFlags", wintypes.DWORD), ("szExeFile", wintypes.WCHAR * 260)]
    kernel = ctypes.WinDLL("kernel32", use_last_error=True)
    kernel.CreateToolhelp32Snapshot.restype = wintypes.HANDLE
    kernel.CreateToolhelp32Snapshot.argtypes = [wintypes.DWORD, wintypes.DWORD]
    kernel.Process32FirstW.argtypes = [wintypes.HANDLE, ctypes.POINTER(PROCESSENTRY32W)]
    kernel.Process32NextW.argtypes = [wintypes.HANDLE, ctypes.POINTER(PROCESSENTRY32W)]
    kernel.OpenProcess.restype = wintypes.HANDLE
    kernel.OpenProcess.argtypes = [wintypes.DWORD, wintypes.BOOL, wintypes.DWORD]
    kernel.QueryFullProcessImageNameW.argtypes = [wintypes.HANDLE, wintypes.DWORD,
                                                wintypes.LPWSTR, ctypes.POINTER(wintypes.DWORD)]
    kernel.CloseHandle.argtypes = [wintypes.HANDLE]
    kernel.GetProcessTimes.argtypes = [wintypes.HANDLE] + [ctypes.POINTER(wintypes.FILETIME)] * 4
    snapshot = kernel.CreateToolhelp32Snapshot(2, 0)
    if snapshot in (None, ctypes.c_void_p(-1).value):
        return []
    processes = []
    try:
        entry = PROCESSENTRY32W()
        entry.dwSize = ctypes.sizeof(entry)
        more = kernel.Process32FirstW(snapshot, ctypes.byref(entry))
        while more:
            handle = kernel.OpenProcess(0x1000, False, entry.th32ProcessID)
            if handle:
                try:
                    size = wintypes.DWORD(32768)
                    buffer = ctypes.create_unicode_buffer(size.value)
                    if kernel.QueryFullProcessImageNameW(handle, 0, buffer, ctypes.byref(size)):
                        path = Path(buffer.value)
                        processes.append((entry.th32ProcessID, path))
                        if path.name.lower() == "fireredpc.exe":
                            times = [wintypes.FILETIME() for _ in range(4)]
                            if kernel.GetProcessTimes(handle, *(ctypes.byref(value) for value in times)):
                                PROCESS_CREATION_TIMES[entry.th32ProcessID] = (times[0].dwHighDateTime << 32) | times[0].dwLowDateTime
                finally:
                    kernel.CloseHandle(handle)
            more = kernel.Process32NextW(snapshot, ctypes.byref(entry))
    finally:
        kernel.CloseHandle(snapshot)
    return processes


def native_process_save_paths(pids):
    """Read only command lines of identified native games; never execute their text."""
    if os.name != "nt" or not pids:
        return {}
    filters = " OR ".join("ProcessId=" + str(int(pid)) for pid in pids)
    script = "[Console]::OutputEncoding=[Text.UTF8Encoding]::new(); Get-CimInstance Win32_Process -Filter '" + filters + "' | Select-Object ProcessId,CommandLine | ConvertTo-Json -Compress"
    try:
        result = subprocess.run(["powershell.exe", "-NoProfile", "-NonInteractive", "-Command", script],
                                capture_output=True, text=True, encoding="utf-8", timeout=4,
                                creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        rows = json.loads(result.stdout) if result.stdout.strip() else []
        rows = rows if isinstance(rows, list) else [rows]
    except (OSError, ValueError, subprocess.TimeoutExpired):
        return {}
    from ctypes import wintypes
    shell = ctypes.WinDLL("shell32", use_last_error=True)
    shell.CommandLineToArgvW.argtypes = [wintypes.LPCWSTR, ctypes.POINTER(ctypes.c_int)]
    shell.CommandLineToArgvW.restype = ctypes.POINTER(wintypes.LPWSTR)
    kernel = ctypes.WinDLL("kernel32", use_last_error=True)
    kernel.LocalFree.argtypes = [ctypes.c_void_p]
    parsed = {}
    for row in rows:
        if not row.get("CommandLine"):
            continue
        count = ctypes.c_int()
        argv = shell.CommandLineToArgvW(row["CommandLine"], ctypes.byref(count))
        if not argv:
            continue
        try:
            arguments = [argv[index] for index in range(count.value)]
        finally:
            kernel.LocalFree(argv)
        values = {arguments[index]: arguments[index + 1] for index in range(len(arguments) - 1)
                  if arguments[index] in ("--data-dir", "--save-file")}
        save = Path(values["--save-file"]) if "--save-file" in values else None
        directory = Path(values["--data-dir"]) if "--data-dir" in values else None
        if save is not None and save.is_absolute():
            parsed[int(row["ProcessId"])] = save.resolve()
        elif save is None and directory is not None and directory.is_absolute():
            parsed[int(row["ProcessId"])] = (directory / "firered.sav").resolve()
        # Relative or omitted locations have an unknown process working directory.
    return parsed


class Launcher:
    def __init__(self, root=ROOT, data_dir=None, state_dir=None, popen=subprocess.Popen, embed_parent=None, library_dir=None):
        if embed_parent is not None and (type(embed_parent) is not int or not 0 < embed_parent < 2**64):
            raise ValueError("Embedded parent must be a positive window handle.")
        self.embed_parent = embed_parent
        self.root = Path(root).resolve()
        self.web_root = self.root / "launcher"
        self.data_dir = Path(data_dir).resolve() if data_dir else self.default_data_dir()
        self.state_dir = Path(state_dir).resolve() if state_dir else self.web_root / "state"
        self.library_dir = Path(library_dir).resolve() if library_dir else self.state_dir
        self.journal_dir = self.library_dir / "gen3-transfers"
        self.token = secrets.token_urlsafe(32)
        self.lock = threading.RLock()
        self.process = None
        self.current = None
        self.last_exit = None
        self._last_processes = []
        self._legacy_process_saves = {}
        self.popen = popen
        self.registrations = self.read_registrations()
        self.extract_job = None

    def default_data_dir(self):
        if (self.root / "firered.sav").is_file():
            return self.root
        if (self.root / "dist" / "firered.sav").is_file():
            return self.root / "dist"
        return self.root

    def read_registrations(self):
        try:
            content = json.loads((self.library_dir / "games.json").read_text(encoding="utf-8"))
            return {key: value for key, value in content.items()
                    if key in GAME_TITLES and key != "firered" and isinstance(value, str)}
        except (OSError, ValueError, AttributeError):
            return {}

    def game_dll(self, game):
        path = self.root / "games" / (game + ".dll")
        return path if path.is_file() and (self.root / RUNNER).is_file() else None

    def pkmn(self, game):
        return self.root / (game + ".pkmn")

    def executable(self, game):
        """The runner, once the game is installed and its ROM has been extracted."""
        if game not in GAME_TITLES:
            raise LauncherError("Choose one of the five supported game entries.")
        if self.game_dll(game) and self.pkmn(game).is_file():
            return (self.root / RUNNER).resolve()
        return None

    def backup(self, path):
        if not path.is_file():
            return None
        directory = self.library_dir / "backups"
        directory.mkdir(parents=True, exist_ok=True)
        destination = directory / (path.name + "." + str(time.time_ns()) + ".bak")
        shutil.copy2(path, destination)
        return str(destination)

    def settings(self):
        values = DEFAULTS.copy()
        try:
            contents = (self.data_dir / "settings.ini").read_text(encoding="utf-8-sig")
        except (OSError, UnicodeError):
            return values
        for line in contents.splitlines():
            match = re.match(r"\s*([a-z_]+)\s*=\s*(-?\d+)", line)
            if match and match[1] in RANGES:
                low, high = RANGES[match[1]]
                values[match[1]] = max(low, min(high, int(match[2])))
        return values

    def running(self):
        with self.lock:
            if self.process:
                code = self.process.poll()
                if code is None:
                    return self.current.copy()
                self.last_exit = dict(self.current, exitCode=code)
                if code:
                    errors = {
                        2: "FireRed could not open this save. Import a valid FireRed cartridge save, or start and save a new adventure.",
                        3: "FireRed could not back up or save your changes. The previous save is retained; check free space and folder permissions before trying again.",
                        4: "This FireRed save is already open in another game window, or its save lock could not be created. Close the other game and try again.",
                    }
                    self.last_exit["error"] = errors.get(code, "The game exited unexpectedly (code " + str(code) + ").") if self.current["game"] == "firered" else "The game exited unexpectedly (code " + str(code) + ")."
                self.process = self.current = None
            # games started outside the launcher by the older FireRed build
            installed = {str(path.resolve()).casefold(): "firered"
                         for path in (self.root / "dist" / "launcher-native" / "FireRedPC.exe",
                                      self.root / "FireRedPC.exe", self.root / "dist" / "FireRedPC.exe")}
            self._last_processes = windows_processes()
            for pid, path in self._last_processes:
                if game := installed.get(str(path).casefold()):
                    return dict(game=game, title=GAME_TITLES[game], mode="game", pid=pid, external=True)
            return None

    def require_idle(self):
        if self.running():
            raise LauncherError("Close the running game before changing files or opening another session.", 409)
        known_native = {(self.root / "FireRedPC.exe").resolve(), (self.root / "dist" / "FireRedPC.exe").resolve()}
        candidates = [(pid, path.resolve()) for pid, path in self._last_processes if path.resolve() in known_native]
        missing = [pid for pid, path in candidates if (pid, PROCESS_CREATION_TIMES.get(pid), str(path)) not in self._legacy_process_saves]
        if missing:
            read_paths = native_process_save_paths(missing)
            for pid, path in candidates:
                if pid in missing:
                    self._legacy_process_saves[(pid, PROCESS_CREATION_TIMES.get(pid), str(path))] = read_paths.get(pid)
        protected = {self.data_dir / "firered.sav"}
        protected.update(path for game in GAME_TITLES if (path := self.storage_path(game, required=False)))
        for pid, executable in candidates:
            save = self._legacy_process_saves[(pid, PROCESS_CREATION_TIMES.get(pid), str(executable))]
            if save is None or save in protected:
                raise LauncherError("Close the older FireRed game (process " + str(pid) + ") before editing or opening this save. Its save is active or its location cannot be verified.", 409)

    def games(self):
        return [dict(id=key, title=title, available=bool(path := self.executable(key)),
                     executable=str(path) if path else None,
                     importable=bool(self.game_dll(key)),
                     saveExists=bool((save := self.storage_path(key, required=False)) and save.is_file()),
                     nativeBoxes=key == "firered") for key, title in GAME_TITLES.items()]

    def read_save_registrations(self):
        try:
            content = json.loads((self.library_dir / "saves.json").read_text(encoding="utf-8"))
            return {key: value for key, value in content.items()
                    if key in GAME_TITLES and isinstance(value, str)}
        except (OSError, ValueError, AttributeError):
            return {}

    def storage_path(self, game, required=True):
        if game not in GAME_TITLES:
            raise LauncherError("Choose Ruby, Sapphire, FireRed, LeafGreen, or Emerald.")
        registered = self.read_save_registrations()
        path = Path(registered[game]).resolve() if game in registered else (self.data_dir / "firered.sav" if game == "firered" else None)
        if required and (path is None or not path.is_file()):
            raise LauncherError("Link an existing " + GAME_TITLES[game] + " cartridge save to open its boxes.", 404)
        return path

    def storage_games(self):
        games = []
        for game, title in GAME_TITLES.items():
            path = self.storage_path(game, required=False)
            exists = bool(path and path.is_file())
            games.append(dict(id=game, title=title, registered=exists, saveExists=exists,
                              path=str(path) if path else None))
        return games

    def storage(self, game=None):
        with self.lock:
            if game is None:
                return dict(ok=True, games=self.storage_games())
            self.require_idle()
            path = self.storage_path(game)
            try:
                return dict(ok=True, **gen3.inspect_save(path, game, journal_dir=self.journal_dir))
            except gen3.Gen3Error as error:
                raise LauncherError(str(error)) from error

    def storage_import(self, game, source):
        if game not in GAME_TITLES:
            raise LauncherError("Choose Ruby, Sapphire, FireRed, LeafGreen, or Emerald.")
        if not isinstance(source, str) or not source.strip():
            raise LauncherError("Choose an existing cartridge save (.sav).")
        path = Path(source).expanduser().resolve()
        if not path.is_file() or path.suffix.lower() != ".sav":
            raise LauncherError("Choose an existing cartridge save (.sav), not an emulator save state.")
        with self.lock:
            self.require_idle()
            try:
                parsed = gen3.inspect_save(path, game, journal_dir=self.journal_dir)
                with gen3.operation_lock(self.journal_dir):
                    registrations = self.read_save_registrations()
                    for other in GAME_TITLES:
                        other_path = self.storage_path(other, required=False)
                        if other != game and other_path and other_path.is_file() and os.path.samefile(path, other_path):
                            raise LauncherError("This save is already linked as " + GAME_TITLES[other] + ". Link a different save.")
                    registrations[game] = str(path)
                    atomic_text(self.library_dir / "saves.json", json.dumps(registrations, indent=2) + "\n")
            except gen3.Gen3Error as error:
                raise LauncherError(str(error)) from error
            return dict(ok=True, game=game, path=str(path), revision=parsed["revision"],
                        storage=parsed, games=self.storage_games())

    def storage_move(self, body):
        source_game, target_game = body.get("sourceGame"), body.get("targetGame")
        if source_game not in GAME_TITLES or target_game not in GAME_TITLES:
            raise LauncherError("Choose a registered source and destination game.")
        for field, limit in (("sourceBox", 14), ("sourceSlot", 30), ("targetBox", 14), ("targetSlot", 30)):
            if type(body.get(field)) is not int or not 0 <= body[field] < limit:
                raise LauncherError("Invalid box or slot: " + field)
        for field in ("sourceRevision", "targetRevision"):
            if not isinstance(body.get(field), str) or not re.fullmatch(r"[0-9a-f]{64}", body[field]):
                raise LauncherError("Refresh both saves before moving Pokemon; valid save revisions are required.", 409)
        with self.lock:
            self.require_idle()
            source = self.storage_path(source_game)
            target = self.storage_path(target_game)
            try:
                return gen3.transfer_pokemon(source, source_game, body["sourceBox"], body["sourceSlot"],
                                             target, target_game, body["targetBox"], body["targetSlot"],
                                             body["sourceRevision"], body["targetRevision"],
                                             journal_dir=self.journal_dir)
            except gen3.Gen3Error as error:
                status = 409 if "changed" in str(error) or "progress" in str(error) else 400
                raise LauncherError(str(error), status) from error

    def gift_status(self):
        return dict(configured=False, nativeAvailable=bool(self.executable("firered")),
                    message="Open the original FireRed Wonder Card / Wonder News screen. "
                    "No internet gift distribution service is configured.")

    def state(self):
        with self.lock:
            running = self.running()
            return dict(ok=True, token=self.token, games=self.games(), settings=self.settings(),
                        defaults=DEFAULTS, running=running, lastExit=self.last_exit,
                        giftStatus=self.gift_status(), dataDir=str(self.data_dir),
                        savePath=str(self.storage_path("firered", required=False) or self.data_dir / "firered.sav"), embedded=self.embed_parent is not None,
                        libraryDir=str(self.library_dir))

    def update_settings(self, changes):
        if not isinstance(changes, dict) or not changes:
            raise LauncherError("Provide the settings to change.")
        for key, value in changes.items():
            if key not in RANGES or type(value) is not int:
                raise LauncherError("Unknown setting or invalid value: " + str(key))
            if not RANGES[key][0] <= value <= RANGES[key][1]:
                raise LauncherError("Setting out of range: " + key)
        with self.lock:
            self.require_idle()
            path = self.data_dir / "settings.ini"
            values = self.settings()
            values.update(changes)
            try:
                lines = path.read_text(encoding="utf-8-sig").splitlines()
            except (OSError, UnicodeError):
                lines = ["# FireRed PC settings (also editable in-game: OPTIONS > PC SETTINGS)"]
            output, seen = [], set()
            for line in lines:
                match = re.match(r"\s*([a-z_]+)\s*=", line)
                if match and match[1] in values:
                    key = match[1]
                    if key not in seen:
                        output.append(f"{key} = {values[key]}")
                        seen.add(key)
                else:
                    output.append(line)
            output.extend(f"{key} = {value}" for key, value in values.items() if key not in seen)
            self.backup(path)
            atomic_text(path, "\n".join(output) + "\n")
            return dict(ok=True, settings=values)

    def import_game(self, game, source):
        """ROM import: extract the player's ROM into the game's .pkmn, in the background."""
        if game not in GAME_TITLES:
            raise LauncherError("Choose one of the five supported game entries.")
        if not self.game_dll(game):
            raise LauncherError(GAME_TITLES[game] + " isn't available in this build yet.")
        if not isinstance(source, str) or not source.strip():
            raise LauncherError("Choose your game ROM (.gba).")
        path = Path(source).expanduser().resolve()
        if path.suffix.lower() != ".gba" or not path.is_file():
            raise LauncherError("Select your game's ROM file (.gba).")
        with self.lock:
            self.require_idle()
            if self.extract_job and not self.extract_job["done"]:
                raise LauncherError("A ROM is already being extracted.", 409)
            job = dict(game=game, title=GAME_TITLES[game], progress=0, done=False, error=None)
            self.extract_job = job
        threading.Thread(target=self._extract, args=(job, path), daemon=True).start()
        return dict(ok=True, extract=job.copy())

    def _extract(self, job, rom):
        try:
            command = [str(self.root / RUNNER), "--extract", str(rom), "--game", job["game"],
                       "--out", str(self.pkmn(job["game"]))]
            environment = os.environ.copy()
            environment["PATH"] = str(self.root / "dist") + os.pathsep + environment.get("PATH", "")
            process = self.popen(command, cwd=str(self.root), env=environment, stdin=subprocess.DEVNULL,
                                 stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True,
                                 creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            done = code = None
            for line in process.stdout:
                try:
                    message = json.loads(line)
                except ValueError:
                    continue
                if "progress" in message:
                    job["progress"] = max(0, min(100, int(message["progress"])))
                elif "error" in message:
                    code = int(message["error"])
                elif "id" in message:
                    done = message
            process.wait()
            if process.returncode == 0 and done:
                job["progress"] = 100
            else:
                title = job["title"].replace(" Version", "")
                job["error"] = ROM_ERRORS.get(code or process.returncode,
                                              "The ROM could not be extracted (code " + str(process.returncode) + ").").format(title=title)
        except (OSError, ValueError) as error:
            job["error"] = "The ROM could not be extracted: " + str(error)
        finally:
            job["done"] = True

    def extract_status(self):
        with self.lock:
            return dict(ok=True, extract=self.extract_job.copy() if self.extract_job else None, games=self.games())

    def import_save(self, source):
        if not isinstance(source, str) or not source.strip():
            raise LauncherError("Choose a FireRed cartridge save (.sav).")
        path = Path(source).expanduser().resolve()
        if not path.is_file() or path.suffix.lower() != ".sav" or path.stat().st_size != 131072:
            raise LauncherError("Choose a 128 KiB FireRed cartridge save (.sav). Emulator save states are not supported.")
        with self.lock:
            self.require_idle()
            destination = self.data_dir / "firered.sav"
            try:
                parsed = gen3.inspect_save(path, "firered", journal_dir=self.journal_dir)
                with gen3.operation_lock(self.journal_dir), gen3._native_save_locks([path, destination]):
                    raw = path.read_bytes()
                    if gen3._sha(raw) != parsed["revision"]:
                        raise LauncherError("The selected save changed during import. Choose the file again.", 409)
                    if path != destination:
                        self.backup(destination)
                        self.data_dir.mkdir(parents=True, exist_ok=True)
                        temporary = destination.with_name("firered.sav.launcher.tmp")
                        temporary.write_bytes(raw)
                        os.replace(temporary, destination)
                    registrations = self.read_save_registrations()
                    registrations["firered"] = str(destination)
                    atomic_text(self.library_dir / "saves.json", json.dumps(registrations, indent=2) + "\n")
            except gen3.Gen3Error as error:
                raise LauncherError(str(error)) from error
            return dict(ok=True, savePath=str(destination))

    def launch(self, game, mode="game"):
        if mode not in {"game", "gifts", *BOX_MODES}:
            raise LauncherError("Unknown launch mode.")
        with self.lock:
            self.require_idle()
            runner = self.executable(game)
            if not runner:
                raise LauncherError("Import your " + GAME_TITLES[game].replace(" Version", "") + " ROM first.")
            if mode != "game" and game != "firered":
                raise LauncherError("Original PC boxes and Mystery Gift are available in FireRed.")
            directory = self.data_dir
            directory.mkdir(parents=True, exist_ok=True)
            command = [str(runner), "--game", game, "--pkmn", str(self.pkmn(game)), "--data-dir", str(directory)]
            if self.embed_parent is not None:
                command += ["--launcher-parent", str(self.embed_parent)]
            save = self.storage_path(game, required=False)
            if save is not None and save.is_file():
                command += ["--save-file", str(save)]
            elif game in self.read_save_registrations():
                raise LauncherError("The linked " + GAME_TITLES[game].replace(" Version", "") + " save cannot be found. Link the save again before starting the game.")
            else:
                save = directory / (game + ".sav")          # each game its own save
                command += ["--save-file", str(save)]
            if mode != "game":
                if not save.is_file() or save.stat().st_size != 131072:
                    raise LauncherError("Create and save a game in FireRed first, or import an existing 128 KiB FireRed .sav file.")
                self.backup(save)
                command += ["--mystery-gift"] if mode == "gifts" else ["--pc-boxes", BOX_MODES[mode]]
            environment = os.environ.copy()
            environment["PATH"] = str(self.root / "dist") + os.pathsep + environment.get("PATH", "")
            try:
                process = self.popen(command, cwd=str(directory), env=environment,
                                     stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL,
                                     stderr=subprocess.DEVNULL,
                                     creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            except OSError as error:
                raise LauncherError("Could not start the game: " + str(error), 500) from error
            self.process = process
            self.current = dict(game=game, title=GAME_TITLES[game], mode=mode, pid=process.pid, external=False)
            self.last_exit = None
            return dict(ok=True, running=self.current.copy())

    def browse(self, kind="game"):
        if kind not in ("game", "save", "rom"):
            raise LauncherError("Unknown file picker type.")
        # A separate GUI process keeps tkinter's Windows message loop on its main thread.
        script = """import tkinter as tk, json, sys
from tkinter import filedialog
r=tk.Tk();r.withdraw();r.attributes('-topmost',True)
save=sys.argv[1]=='save'
p=filedialog.askopenfilename(parent=r,title='Choose a Generation III cartridge save' if save else 'Choose your game ROM',filetypes=[('Cartridge save','*.sav')] if save else [('Game Boy Advance ROM','*.gba')])
print(json.dumps({'path':p}));r.destroy()
"""
        python = Path(sys.executable)
        if python.name.lower() == "pythonw.exe":
            python = python.with_name("python.exe")
        try:
            result = subprocess.run([str(python), "-c", script, kind], capture_output=True,
                                    text=True, timeout=300,
                                    creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            chosen = json.loads(result.stdout)
            return dict(ok=True, path=chosen["path"])
        except (OSError, ValueError, KeyError, subprocess.TimeoutExpired) as error:
            raise LauncherError("The file picker could not open. Enter the full file path instead.", 500) from error


class Handler(BaseHTTPRequestHandler):
    server_version = "PokeRecomp/1"

    @property
    def app(self):
        return self.server.app

    def log_message(self, message, *args):
        if sys.stderr:
            super().log_message(message, *args)

    def valid_host(self):
        return self.headers.get("Host", "") in self.server.allowed_hosts

    def reply(self, status, data):
        payload = json.dumps(data, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(payload)))
        self.send_header("Cache-Control", "no-store")
        self.send_header("X-Content-Type-Options", "nosniff")
        self.end_headers()
        self.wfile.write(payload)

    def error(self, message, status=400):
        self.reply(status, dict(ok=False, error=message))

    def do_GET(self):
        if not self.valid_host():
            return self.error("Unrecognized launcher host.", 403)
        path = urlsplit(self.path).path
        if path == "/api/state":
            return self.reply(200, self.app.state())
        if path == "/api/extract":
            return self.reply(200, self.app.extract_status())
        if path == "/api/gifts":
            return self.reply(200, dict(ok=True, **self.app.gift_status()))
        if path == "/api/storage":
            try:
                query = parse_qs(urlsplit(self.path).query)
                game = query.get("game", [None])[0]
                return self.reply(200, self.app.storage(game))
            except LauncherError as error:
                return self.error(str(error), error.status)
            except OSError as error:
                return self.error("The save could not be read: " + str(error), 500)
        if path.startswith("/api/"):
            return self.error("Unknown endpoint.", 404)
        decoded = unquote(path).replace("\\", "/")
        if decoded == "/":
            decoded = "/index.html"
        # Only front-end documents and the extracted asset tree are public.
        relative = decoded.lstrip("/")
        if relative not in ("index.html", "app.js", "styles.css", "style.css", "favicon.svg") and not relative.startswith("assets/"):
            return self.error("Not found.", 404)
        file = (self.app.web_root / relative).resolve()
        allowed_root = (self.app.web_root / "assets").resolve() if relative.startswith("assets/") else self.app.web_root.resolve()
        if not file.is_relative_to(allowed_root) or not file.is_file():
            return self.error("Not found.", 404)
        payload = file.read_bytes()
        self.send_response(200)
        self.send_header("Content-Type", mimetypes.guess_type(str(file))[0] or "application/octet-stream")
        self.send_header("Content-Length", str(len(payload)))
        self.send_header("Cache-Control", "no-cache")
        self.send_header("X-Content-Type-Options", "nosniff")
        self.send_header("Content-Security-Policy", "default-src 'self'; img-src 'self' data:; style-src 'self' 'unsafe-inline'; script-src 'self'; connect-src 'self'; frame-ancestors 'none'; base-uri 'none'")
        self.end_headers()
        self.wfile.write(payload)

    def do_POST(self):
        if not self.valid_host():
            return self.error("Unrecognized launcher host.", 403)
        origin = self.headers.get("Origin")
        if origin and origin not in self.server.allowed_origins:
            return self.error("Requests must come from this launcher window.", 403)
        if not secrets.compare_digest(self.headers.get("X-Launcher-Token", ""), self.app.token):
            return self.error("Launcher session expired. Reload this window.", 403)
        try:
            length = int(self.headers.get("Content-Length", "0"))
            if not 0 < length <= 16384:
                raise LauncherError("Invalid request length.", 413)
            body = json.loads(self.rfile.read(length))
            if not isinstance(body, dict):
                raise LauncherError("Request must be a JSON object.")
            path = urlsplit(self.path).path
            if path == "/api/launch":
                result = self.app.launch(body.get("game", "firered"), body.get("mode", "game"))
            elif path == "/api/settings":
                result = self.app.update_settings(body.get("settings"))
            elif path == "/api/import":
                result = self.app.import_game(body.get("game"), body.get("path"))
            elif path == "/api/import-save":
                result = self.app.import_save(body.get("path"))
            elif path == "/api/storage/import":
                result = self.app.storage_import(body.get("game"), body.get("path"))
            elif path == "/api/storage/move":
                result = self.app.storage_move(body)
            elif path == "/api/browse":
                result = self.app.browse(body.get("kind", "game"))
            elif path == "/api/quit":
                with self.app.lock:
                    self.app.require_idle()
                    result = dict(ok=True)
                self.reply(200, result)
                threading.Thread(target=self.server.shutdown, daemon=True).start()
                return
            else:
                raise LauncherError("Unknown endpoint.", 404)
            self.reply(200, result)
        except LauncherError as error:
            self.error(str(error), error.status)
        except (ValueError, TypeError, UnicodeError):
            self.error("Invalid JSON request.")
        except OSError as error:
            self.error("The requested file operation failed: " + str(error), 500)


def make_server(app, port=0):
    server = ThreadingHTTPServer(("127.0.0.1", port), Handler)
    server.daemon_threads = True
    server.app = app
    port = server.server_address[1]
    server.allowed_hosts = {f"127.0.0.1:{port}", f"localhost:{port}"}
    server.allowed_origins = {"http://" + host for host in server.allowed_hosts}
    return server


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", type=int, default=0)
    parser.add_argument("--data-dir", type=Path, help="Isolated native save/settings directory, also useful for QA")
    parser.add_argument("--state-dir", type=Path, help="Override launcher state and backup directory")
    parser.add_argument("--library-dir", type=Path, help="Persistent game/save registrations and backups, separate from per-instance session state")
    parser.add_argument("--embed-parent", type=int, help="Native launcher game-panel HWND (decimal)")
    args = parser.parse_args()
    app = Launcher(data_dir=args.data_dir, state_dir=args.state_dir, embed_parent=args.embed_parent, library_dir=args.library_dir)
    server = make_server(app, args.port)
    session_file = app.state_dir / "session.json"
    session = dict(pid=os.getpid(), url=f"http://127.0.0.1:{server.server_address[1]}",
                   dataDir=str(app.data_dir))
    atomic_text(session_file, json.dumps(session, indent=2) + "\n")
    if sys.stdout:
        print(session["url"], flush=True)
    try:
        server.serve_forever(poll_interval=0.25)
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()
        try:
            if json.loads(session_file.read_text())["pid"] == os.getpid():
                session_file.unlink()
        except (OSError, ValueError, KeyError):
            pass


if __name__ == "__main__":
    main()
