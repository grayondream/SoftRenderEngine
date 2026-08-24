#!/usr/bin/env zsh
set -e
bin=$1
golddir=$2
fail=0
for md5file in "$golddir"/*.md5; do
    mode=$(basename "$md5file" .md5)
    out=$(mktemp /tmp/opencode/golden-XXXXXX.ppm)
    if ! "$bin" "$out" "$mode" > /dev/null 2>&1; then
        echo "GOLDEN RUN FAIL: $mode"
        rm -f "$out"; fail=$((fail+1)); continue
    fi
    want=$(cat "$md5file")
    got=$(md5sum "$out" | awk '{print $1}')
    if [ "$got" != "$want" ]; then
        echo "GOLDEN MISMATCH [$mode]: got $got want $want"
        fail=$((fail+1))
    else
        echo "GOLDEN OK [$mode]: $got"
    fi
    rm -f "$out"
done
[ "$fail" -eq 0 ] && echo "ALL GOLDEN PASS" || exit 1
