#!/usr/bin/env bash
#
# pack_release.sh — build a clean, distributable PokeRecomp archive.
#
# The point of this script is that the archive is clean *structurally*, not
# because someone remembered to delete things. It works from an explicit
# allow-list: only files named here are copied. Anything else in deploy/ —
# ROMs, .pkmn archives, save files, accounts, crash logs, debug dumps — is
# never staged in the first place.
#
# It then runs a verification gate over the staged tree and REFUSES to produce
# an archive if anything that looks like copyrighted ROM data is present.
#
# Usage:
#   tools/pack_release.sh [output-name]
#
# Example:
#   tools/pack_release.sh PokeRecomp-v0.9
#
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$REPO_ROOT/deploy"
OUT_NAME="${1:-PokeRecomp-release}"
STAGE="$REPO_ROOT/build/release/$OUT_NAME"

if [ ! -d "$SRC" ]; then
    echo "error: deploy/ not found at $SRC" >&2
    exit 1
fi

echo "==> Staging clean release: $OUT_NAME"
rm -rf "$STAGE"
mkdir -p "$STAGE"

# ---------------------------------------------------------------------------
# Allow-list. Only these are copied.
# ---------------------------------------------------------------------------

# Executables
BINARIES=(
    "PokeRecomp.exe"
    "PokeServer.exe"
)

# Config templates that are safe to ship (no user data)
CONFIGS=(
    "server.cfg"
)

# Docs pulled from the repo root, not from deploy/
DOCS=(
    "LICENSE"
    "THIRD_PARTY_LICENSES.txt"
    "README.md"
)

copied=0
for f in "${BINARIES[@]}" "${CONFIGS[@]}"; do
    if [ -f "$SRC/$f" ]; then
        cp -v "$SRC/$f" "$STAGE/"
        copied=$((copied + 1))
    else
        echo "  (skip, not present: $f)"
    fi
done

# All runtime DLLs (these are libraries, never user data)
dllcount=0
for f in "$SRC"/*.dll; do
    [ -e "$f" ] || continue
    cp "$f" "$STAGE/"
    dllcount=$((dllcount + 1))
done
echo "  copied $dllcount DLLs"

# Launcher support directory: assets and DLLs only
if [ -d "$SRC/launcher" ]; then
    mkdir -p "$STAGE/launcher"
    find "$SRC/launcher" -maxdepth 1 -type f \
        \( -name '*.dll' -o -name '*.png' -o -name '*.jpg' -o -name '*.ogg' \
           -o -name '*.wav' -o -name '*.ttf' \) \
        -exec cp {} "$STAGE/launcher/" \;
    echo "  staged launcher/ ($(find "$STAGE/launcher" -type f | wc -l) files)"
fi

for f in "${DOCS[@]}"; do
    if [ -f "$REPO_ROOT/$f" ]; then
        cp -v "$REPO_ROOT/$f" "$STAGE/"
    else
        echo "  WARNING: missing doc $f" >&2
    fi
done

# ---------------------------------------------------------------------------
# Verification gate. Abort rather than ship something we shouldn't.
# ---------------------------------------------------------------------------

echo "==> Verifying staged tree"
FORBIDDEN_GLOBS=(
    '*.pkmn'    # extracted ROM archives (contain a full ROM image)
    '*.gba'     # raw GBA ROMs
    '*.gb'      # raw GB/GBC ROMs
    '*.gbc'
    '*.sav'     # user save files
    '*.srm'
    '*.log'     # crash/debug logs may contain memory contents
)

violations=0
for glob in "${FORBIDDEN_GLOBS[@]}"; do
    while IFS= read -r hit; do
        echo "  FORBIDDEN: $hit" >&2
        violations=$((violations + 1))
    done < <(find "$STAGE" -iname "$glob" 2>/dev/null)
done

# accounts/ holds password hashes + player saves; must never ship
if [ -d "$STAGE/accounts" ]; then
    echo "  FORBIDDEN: accounts/ directory present" >&2
    violations=$((violations + 1))
fi

# Belt and braces: nothing in the tree should be >= 1 MiB and start with a
# GBA ROM header signature, regardless of its extension.
while IFS= read -r f; do
    sz=$(stat -c %s "$f" 2>/dev/null || echo 0)
    if [ "$sz" -ge 1048576 ]; then
        magic=$(dd if="$f" bs=1 skip=172 count=4 2>/dev/null | tr -d '\0' || true)
        case "$magic" in
            BPRE|BPGE|AXVE|AXPE|BPEE)
                echo "  FORBIDDEN: $f contains a GBA ROM header ($magic)" >&2
                violations=$((violations + 1))
                ;;
        esac
    fi
done < <(find "$STAGE" -type f)

if [ "$violations" -gt 0 ]; then
    echo "==> ABORTED: $violations forbidden item(s) in staged tree." >&2
    echo "    Nothing was archived. Fix the allow-list above and re-run." >&2
    exit 1
fi

echo "  clean: no ROMs, saves, accounts, or logs staged"

# ---------------------------------------------------------------------------
# Archive
# ---------------------------------------------------------------------------

echo "==> Creating archive"
ARCHIVE="$REPO_ROOT/build/release/$OUT_NAME.zip"
rm -f "$ARCHIVE"

if command -v 7z >/dev/null 2>&1; then
    (cd "$REPO_ROOT/build/release" && 7z a -tzip "$OUT_NAME.zip" "$OUT_NAME" >/dev/null)
elif command -v zip >/dev/null 2>&1; then
    (cd "$REPO_ROOT/build/release" && zip -qr "$OUT_NAME.zip" "$OUT_NAME")
else
    echo "  no 7z or zip found; staged tree left at:" >&2
    echo "  $STAGE" >&2
    exit 0
fi

echo "==> Done"
echo "    Staged:  $STAGE"
echo "    Archive: $ARCHIVE"
echo
echo "Reminder: users supply their own ROM. The launcher builds the .pkmn"
echo "archive locally on first run; it is never distributed."
