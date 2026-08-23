#!/usr/bin/env zsh
set -e
bin=$1
baseline=$(cat "$2")
out=$(mktemp /tmp/opencode/golden-XXXXXX.ppm)
trap "rm -f $out" EXIT
"$bin" "$out"
hash=$(md5sum "$out" | awk '{print $1}')
if [ "$hash" != "$baseline" ]; then
    echo "GOLDEN MISMATCH: got $hash want $baseline"
    exit 1
fi
echo "GOLDEN OK: $hash"
