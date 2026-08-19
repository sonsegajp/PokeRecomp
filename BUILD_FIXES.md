# Build Fixes & Troubleshooting — pokefirered PC Port

Quick reference for every known build issue and how to fix it.

---

## 1. Temp File Errors (`Cannot create temporary file in C:\WINDOWS\`)

**Symptom:**
```
C:\WINDOWS\ccXXXXXX.s: Assembler messages:
Error: can't open C:\WINDOWS\ccXXXXXX.s for reading: No such file or directory
```
Or:
```
Cannot create temporary file in C:\WINDOWS\: Permission denied
i686-w64-mingw32-g++.exe: internal compiler error: Aborted signal terminated program collect2
```

**Cause:** MinGW g++ writes intermediate `.s` files to the system temp directory. On some Windows installs, this resolves to `C:\WINDOWS\` which isn't writable.

**Fix:** The Makefile exports `TMPDIR`, `TMP`, and `TEMP` to `build/pc/tmp/`. If this breaks:
- Verify `build/pc/tmp/` exists (it's auto-created by the Makefile's `mkdir -p`)
- If running make manually, export before invoking: `export TMP="$PWD/build/pc/tmp"`
- The `-pipe` flag on `IMGUI_CXXFLAGS` and the link step bypasses temp files for most C++ compilation — if `-pipe` is missing, add it back

---

## 2. REG_BASE Compile Conflict

**Symptom:**
```
src/platform/bios.c: error: expected identifier before numeric constant
unsigned char REG_BASE[0x400]
```

**Cause:** `include/gba/io_reg.h` defines `#define REG_BASE 0x4000000` for GBA builds. Without the `PORTABLE` guard, this macro expands inside `bios.c` and conflicts with the array declaration.

**Fix:** The top of `io_reg.h` must have:
```c
#ifndef PORTABLE
#define REG_BASE 0x4000000
#else
extern unsigned char REG_BASE[];
#endif
```
Additionally, `constants/gba_constants.inc` has `.set REG_BASE, 0x4000000` which creates an absolute linker symbol. The `ASFLAGS` must include `--defsym PORTABLE=1` and `CPPFLAGS` must include `-DPORTABLE` so the C code uses the array from `bios.c` instead.

---

## 3. Data Assembly Pipeline Failures

**Symptom:** Errors from `data/*.s` files — syntax errors, missing symbols, split macro arguments.

**Cause:** GBA assembly uses ARM pseudo-ops that x86 GAS doesn't understand. The pipeline converts them through multiple stages.

**Pipeline (in order):**
1. `$(PREPROC) $< charmap.txt` — text substitutions
2. `$(CPP) -xc -I include -` — C preprocessor expansion
3. `$(PREPROC) -ie $< charmap.txt` — include expansion
4. `$(ASM_PSEUDO_OP_CONV)` — sed script converting `.4byte`→`.int`, `.2byte`→`.short`, `.word`→`.int`, strips `.size` directives, strips `@` comments, handles `\@` macro counters
5. `$(WRAP_MACRO_ARGS)` — Python script wrapping macro args with arithmetic in extra parens
6. `$(AS) $(ASFLAGS)` — GNU assembler (x86)
7. `$(FIX_UNDERSCORE)` — `objcopy --prefix-symbol _` for Win32 symbol mangling

**Common sub-issues:**

- **`.word` not converted:** Make sure `ASM_PSEUDO_OP_CONV` includes `'s/^[[:space:]]*\.word\b/\t.int/'`
- **Macro arg splitting:** If you see `too many arguments` from GAS, check that `tools/wrap_macro_args.py` exists and `WRAP_MACRO_ARGS` is in the pipeline
- **Missing `_` prefix:** Every `.o` from assembly must go through `$(FIX_UNDERSCORE)` or C code can't find the symbols
- **`\@` stripped incorrectly:** The sed script must preserve `\@` (macro counter) while stripping `@` (ARM comments). Check the `__MACRO_CTR__` round-trip in `ASM_PSEUDO_OP_CONV`

---

## 4. ImGui Compilation Errors

**Symptom:** Errors building `lib/imgui/*.cpp` or `src/platform/imgui_bridge.cpp`.

**Checklist:**
- ImGui source must exist at `lib/imgui/` with backends in `lib/imgui/backends/`
- The `IMGUI_CXXFLAGS` must include `-I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -I$(SDL_DIR)/include -I$(SDL_DIR)/include/SDL2`
- SDL2 dev headers must be installed: `pacman -S mingw-w64-i686-SDL2` in MSYS2
- OpenGL3 backend requires OpenGL headers (usually come with MinGW)
- The `-pipe` flag avoids the temp file issue (see fix #1)

---

## 5. Linker: Unresolved Symbols

**Symptom:**
```
undefined reference to `_SomeFunction'
```

**Common causes:**

| Missing symbol pattern | Cause | Fix |
|---|---|---|
| `_FunctionName` | C source file not compiled or not in wildcard | Check `C_SRCS` wildcard picks it up |
| `__imp__WinAPI@N` | Windows API not linked | Add `-lkernel32`, `-luser32`, etc. to link step |
| `rb__FunctionName` | Ruby prefixed object missing | Rebuild pokeruby: `make -f Makefile_pc prefix-ruby` |
| `gd__FunctionName` | Gold prefixed object missing | Rebuild pokegold-native |
| No `_` prefix at all | Assembly `.o` skipped `FIX_UNDERSCORE` | Add `$(FIX_UNDERSCORE) $@` after the assembly rule |

**Non-fatal warnings to ignore:**
- Ruby COMDAT symbol mismatches (`rb_.text$_ZN6ImPool...`) — cosmetic, from symbol prefixing on C++ objects
- Gold/Silver `multiple definition` on shared functions (`GBCRom_CreatePkmn`, etc.) — first-defined wins, both are identical
- `--warn-unresolved-symbols` makes unresolved refs non-fatal — the linker produces the exe anyway

---

## 6. Crash at Startup (Access Violation)

**Symptom:** `crash.log` shows `CRASH code=0xC0000005` with `WRITE address 0x74FBXXXX` or similar during early init.

**Cause:** GBA code writing to `REG_BASE + offset` where REG_BASE is the literal address `0x04000000` instead of the allocated array.

**Fix:** Verify the full chain:
1. `io_reg.h` has the `#ifndef PORTABLE` guard → `extern unsigned char REG_BASE[]`
2. `bios.c` declares `unsigned char REG_BASE[0x400]`
3. `CPPFLAGS` includes `-DPORTABLE`
4. `ASFLAGS` includes `--defsym PORTABLE=1`
5. After linking, check with: `nm PokeRecomp.exe | grep REG_BASE` — should show a `.bss` address (like `08XXXXXX B _REG_BASE`), NOT `04000000 a _REG_BASE`

If `nm` shows both addresses, the `.set` in `gba_constants.inc` is leaking. The `PORTABLE=1` defsym should prevent code from using it, but verify the assembly files are actually being built with that flag.

---

## 7. Battle Animation Crashes (Metal Claw, etc.)

**Symptom:** Crash during specific move animations. `crash.log` shows access to `0xFFFFXXXX` addresses.

**Cause:** GBA battle animation scripts use 16-bit signed offsets that get sign-extended to 32-bit on x86, producing invalid pointers like `0xFFFF8000`.

**Fix:** `src/platform/crash_guard.c` installs a Vectored Exception Handler that:
- Catches access violations at bad addresses (`0xFFFF0000+` or `< 0x10000`)
- Redirects the faulting read/write to a dummy buffer
- Skips the instruction and continues execution
- This file auto-installs via `__attribute__((constructor))` — just make sure it compiles (the C wildcard picks it up automatically)

If crashes persist, check `crash_debug.log` for the faulting EIP and address.

---

## 8. `-O3` Optimization Breaks Game Logic

**Symptom:** Pokemon Centers don't work, NPCs behave incorrectly, event scripts break.

**Cause:** GCC `-O3` optimizes away safety guards in movement/event code that the GBA relied on being present.

**Fix:** Specific files are compiled at `-O0`:
```makefile
# In Makefile_pc — special -O0 rules
$(C_BUILDDIR)/event_object_movement.o : ... $(CC1) $(CFLAGS) -O0 ...
$(C_BUILDDIR)/music_player_port.o : ... $(CC1) $(CFLAGS) -O0 ...
$(C_BUILDDIR)/m4a.o : ... $(CC1) $(CFLAGS) -O0 ...
```

If a new file needs `-O0`, copy one of these rules and change the filename. The default is `-O3` (set by `CFLAGS` when `DINFO` is not 1).

---

## 9. Custom Items Shifting Memory Layout

**Symptom:** Adding items to `graphics.c` causes battle animations or other sprite systems to crash.

**Cause:** `graphics.c` contains a massive array of graphics data. Adding entries shifts the memory layout of everything after it, breaking hardcoded offsets in battle animation scripts.

**Fix:** Put custom item graphics in separate compilation units:
- `src/custom_item_gfx.c` — icon tile data
- `src/poke_radar.c` — PokeRadar graphics + logic
- These compile as independent `.o` files and don't affect `graphics.o`'s layout

---

## 10. PowerShell Corrupts Source Files

**Symptom:** After using PowerShell `Set-Content` to write files, source files have BOM headers and stripped newlines. Build fails with cryptic preprocessor errors.

**Fix:** NEVER use PowerShell to write source files. Always use bash:
```bash
git show COMMIT:path/to/file > path/to/file
```
If files are already corrupted, restore from git: `git checkout HEAD -- path/to/file`

---

## 11. Missing Build Directories

**Symptom:**
```
No such file or directory: 'build/pc/data/something.o'
```

**Cause:** `rm -rf build/pc` or first build on a clean checkout.

**Fix:** The Makefile auto-creates directories via `$(shell mkdir -p $(SUBDIRS) $(OBJ_DIR)/tmp)`. If a new subdirectory is needed (e.g., new source folder), it should be covered by the `C_SRCS` wildcard which feeds into `SUBDIRS`. If not, add it to the `mkdir -p` line.

---

## 12. PATH Issues

**Symptom:** `make: command not found`, wrong assembler used, tools not found.

**Required PATH order:**
```bash
export PATH="/c/msys64/mingw32/bin:/c/msys64/usr/bin:$PATH"
```
- `/c/msys64/mingw32/bin` — MinGW i686 toolchain (gcc, g++, as, ld, objcopy)
- `/c/msys64/usr/bin` — MSYS2 utilities (make, bash, sed, python)

MinGW bin MUST come first so the correct `as` (x86 GNU assembler) is found.

---

## Quick Recovery Checklist

If the build is completely broken:

1. **Don't panic.** `ship/PokeRecomp.exe` is a known-good fallback
2. **Don't run git stash/checkout/reset** — this will make things worse
3. Check `io_reg.h` has the PORTABLE guard
4. Check `Makefile_pc` has TMPDIR/TMP/TEMP exports and `-pipe`
5. `rm -rf build/pc` and rebuild: `make -f Makefile_pc -j4`
6. If source files are corrupted, restore individually: `git show HEAD:file > file`
7. If the stash commit exists, restore from it: `git show 386aa70:file > file`
