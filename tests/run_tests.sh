#!/usr/bin/env bash
# run_tests.sh — diff each example's output against its expected file.
#
# Usage:
#   ./tests/run_tests.sh                  # use ./build/cvm by default
#   CVM=./build/cvm ./tests/run_tests.sh  # explicit binary
#
# Expected outputs live alongside each .cvm script with a .expected suffix.
# Skips any test that has no .expected file.

set -u

CVM="${CVM:-./build/cvm}"
EX_DIR="$(dirname "$0")/../examples"
PASS=0
FAIL=0
SKIP=0

# On Windows the binary lands at cvm.exe; auto-detect if the bare name is missing.
if [[ ! -x "$CVM" && -x "${CVM}.exe" ]]; then
    CVM="${CVM}.exe"
fi

if [[ ! -x "$CVM" ]]; then
    echo "error: '$CVM' not found or not executable"
    echo "       build first:  cmake -B build && cmake --build build"
    exit 2
fi

for script in "$EX_DIR"/*.cvm; do
    name=$(basename "$script")
    expected="${script%.cvm}.expected"

    if [[ ! -f "$expected" ]]; then
        printf "  SKIP  %s (no .expected file)\n" "$name"
        SKIP=$((SKIP+1))
        continue
    fi

    # Strip trailing \r so CRLF expected files match LF output.
    actual=$("$CVM" "$script" 2>/dev/null | tr -d '\r')
    expected_norm=$(tr -d '\r' < "$expected")
    if diff -u <(echo "$actual") <(echo "$expected_norm") >/dev/null 2>&1; then
        printf "  PASS  %s\n" "$name"
        PASS=$((PASS+1))
    else
        printf "  FAIL  %s\n" "$name"
        diff -u <(echo "$actual") <(echo "$expected_norm") | sed 's/^/        /'
        FAIL=$((FAIL+1))
    fi
done

echo
echo "summary: $PASS passed, $FAIL failed, $SKIP skipped"
exit $((FAIL > 0 ? 1 : 0))
