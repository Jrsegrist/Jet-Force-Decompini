#!/bin/bash
# The ROM-free checks, as CI runs them (.github/workflows/ci.yml).
#
# Run from the root of a clean checkout that has no ROM and has never run
# `make extract` (the compile check writes placeholder assembly into asm/),
# after `make setup` and with binutils-mips-linux-gnu installed:
#
#   bash tools/ci_norom.sh
set -euo pipefail
cd "$(dirname "$0")/.."
PY=.venv/bin/python3
JOBS=$(nproc 2>/dev/null || echo 2)

if ls baserom.* >/dev/null 2>&1; then
    echo "ci_norom: a ROM is here; run this in a clean checkout without one" >&2
    exit 1
fi

echo "== no ROM data in tracked files"
$PY tools/ci_rom_free.py

echo "== Python tools: syntax and pyflakes"
$PY -m compileall -q tools/*.py tools/match/*.py diff_settings.py
if $PY -m pyflakes --version >/dev/null 2>&1; then
    $PY -m pyflakes tools/*.py tools/match/*.py diff_settings.py
else
    echo "(pyflakes not installed, skipped)"
fi

echo "== matching tools load (--help; they need the ROM to do anything)"
for t in tools/match/*.py; do
    [ "$(basename "$t")" = common.py ] && continue
    $PY "$t" --help >/dev/null
done

echo "== tool self-tests (synthetic inputs)"
$PY tools/ci_test_tools.py

echo "== every C file compiles (IDO 5.3, project flags, placeholder asm)"
$PY tools/ci_stub_asm.py
mapfile -t OBJS < <(git ls-files 'src/*.c' | sed 's#^#build/#; s#\.c$#.o#')
make -j"$JOBS" "${OBJS[@]}"
echo "${#OBJS[@]} objects"

echo "== NON_MATCHING bodies compile (make nonmatching)"
make -j"$JOBS" nonmatching

echo "ci_norom: all checks passed"
