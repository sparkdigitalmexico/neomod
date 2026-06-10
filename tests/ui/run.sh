#!/bin/sh
# headless UI test runner: pipes each scripts/*.txt into the binary, checks UITEST
# assertions and (when a golden exists) diffs the uitrace event log.
# usage: ./run.sh [script-name ...]   (no args = run all)
# env: NEOMOD_BIN overrides the binary path

set -u

TESTS_DIR="$(cd "$(dirname "$0")" && pwd)"
BIN="${NEOMOD_BIN:-$TESTS_DIR/../../debug-clang/dist/bin-aarch64/neomod}"
OUT_DIR="$TESTS_DIR/out"
RECORD="${RECORD_GOLDEN:-0}"

if [ ! -x "$BIN" ]; then
    echo "error: binary not found at $BIN (set NEOMOD_BIN)" >&2
    exit 2
fi

mkdir -p "$OUT_DIR"

# test fixtures: the skins dropdown only opens if at least one skin folder exists
# (skins_dropdown_hover relies on the deterministic "default" + "UITestSkin" item pair);
# the empty osu folder keeps songbrowser db loads empty and machine-independent
mkdir -p "$(dirname "$BIN")/skins/UITestSkin"
mkdir -p "$(dirname "$BIN")/uitest_osu_folder"

if [ "$#" -gt 0 ]; then
    scripts=""
    for name in "$@"; do
        scripts="$scripts $TESTS_DIR/scripts/$name.txt"
    done
else
    scripts=$(ls "$TESTS_DIR"/scripts/*.txt 2>/dev/null)
fi

pass=0
fail=0
for script in $scripts; do
    name=$(basename "$script" .txt)
    log="$OUT_DIR/$name.log"

    # binary must run from its install dir (assets are relative)
    (cd "$(dirname "$BIN")" && "./$(basename "$BIN")" -headless <"$script" >"$log" 2>&1)
    rc=$?

    status=ok
    reasons=""

    if [ "$rc" -ne 0 ]; then
        status=fail
        reasons="$reasons crash(rc=$rc)"
    fi

    if grep -q "UITEST FAIL" "$log"; then
        status=fail
        reasons="$reasons assert"
    fi

    golden="$TESTS_DIR/golden/$name.trace"
    grep "^uitrace " "$log" >"$OUT_DIR/$name.trace" || true
    if [ "$RECORD" = "1" ]; then
        if [ "$status" != "ok" ]; then
            echo "RECORD FAILED $name ($reasons ) -- golden NOT updated, see $log"
            fail=$((fail + 1))
            continue
        fi
        mkdir -p "$TESTS_DIR/golden"
        cp "$OUT_DIR/$name.trace" "$golden"
        echo "RECORDED $name ($(wc -l <"$golden" | tr -d ' ') trace lines)"
        continue
    fi
    if [ -f "$golden" ]; then
        if ! diff -u "$golden" "$OUT_DIR/$name.trace" >"$OUT_DIR/$name.trace.diff" 2>&1; then
            status=fail
            reasons="$reasons trace-diff"
        else
            rm -f "$OUT_DIR/$name.trace.diff"
        fi
    fi

    if [ "$status" = "ok" ]; then
        pass=$((pass + 1))
        echo "PASS $name"
    else
        fail=$((fail + 1))
        echo "FAIL $name ($reasons ) -- see $log"
        grep "UITEST FAIL" "$log" | sed 's/^/    /'
        [ -f "$OUT_DIR/$name.trace.diff" ] && head -15 "$OUT_DIR/$name.trace.diff" | sed 's/^/    /'
    fi
done

echo "----"
echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
