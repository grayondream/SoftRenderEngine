#!/usr/bin/env zsh
# Golden image comparison with pixel tolerance (robust across compiler versions).
# Usage: check_golden_all.sh <golden_render-bin> <golden-dir>
# Env: GOLDEN_MAX_BAD_PIXELS (default 400), GOLDEN_MAX_DIFF (default 12)
set -e
bin=$1
golddir=$2
maxBad=${GOLDEN_MAX_BAD_PIXELS:-400}
maxDiff=${GOLDEN_MAX_DIFF:-12}
fail=0
for ref in "$golddir"/*.ppm; do
    mode=$(basename "$ref" .ppm)
    mode=${mode#ref_}
    out=$(mktemp /tmp/opencode/golden-XXXXXX.ppm)
    trap "rm -f $out" EXIT
    if ! "$bin" "$out" "$mode" > /dev/null 2>&1; then
        echo "GOLDEN RUN FAIL: $mode"
        fail=$((fail+1)); continue
    fi
    python3 - "$ref" "$out" "$maxBad" "$maxDiff" "$mode" <<'PY'
import sys
def load(p):
    with open(p,'rb') as f:
        assert f.readline().strip()==b'P6'
        wh=f.readline().split()
        w,h=int(wh[0]),int(wh[1])
        assert f.readline().strip()==b'255'
        data=f.read()
    return w,h,data
w,h,a=load(sys.argv[1]); w2,h2,b=load(sys.argv[2])
assert (w,h)==(w2,h2), f"size mismatch {w}x{h} vs {w2}x{h2}"
maxBad=int(sys.argv[3]); maxDiff=int(sys.argv[4]); mode=sys.argv[5]
bad=0; worst=0
for i in range(len(a)):
    d=abs(a[i]-b[i])
    if d>maxDiff:
        bad+=1
        if d>worst: worst=d
        if bad>maxBad:
            print(f"GOLDEN MISMATCH [{mode}]: >{maxBad} pixels exceed diff {maxDiff}")
            sys.exit(1)
print(f"GOLDEN OK [{mode}]: bad={bad}/{w*h} (worst ch diff {worst})")
PY
    [ $? -ne 0 ] && fail=$((fail+1))
done
[ "$fail" -eq 0 ] && echo "ALL GOLDEN PASS" || exit 1
