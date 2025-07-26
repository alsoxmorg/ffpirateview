#!/bin/bash

set -e  # Exit on errors

input="$1"
ext="${input##*.}"
base=$(basename "$input")
tmpfile="tmp436536456.1ff"

cleanup() {
    rm -f "$tmpfile"
}
trap cleanup EXIT

case "$ext" in
    ff) cp "$input" "$tmpfile" ;;
    ffz|lz) lzip -d -c "$input" > "$tmpfile" ;;
    ffbz|bz2) bzip2 -d -c "$input" > "$tmpfile" ;;
    ffzx|xz) xz -d -c "$input" > "$tmpfile" ;;
    png|jpg) 2ff < "$input" > "$tmpfile" ;;
    1ff|1f) cp "$input" "$tmpfile" ;;
    *)
        echo "Unsupported format: .$ext"
        exit 1
        ;;
esac

1fpirate "$tmpfile"
