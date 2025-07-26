#!/bin/bash

set -e  # Exit on errors

input="$1"
ext="${input##*.}"
base=$(basename "$input")
tmpfile="tmp436536456.ff"

cleanup() {
    rm -f "$tmpfile"
}
trap cleanup EXIT

case "$ext" in
    ff) cp "$input" "$tmpfile" ;;
    ffz|lz) lzip -d -c "$input" > "$tmpfile" ;;
    ffbz|bz2) bzip2 -d -c "$input" > "$tmpfile" ;;
    ffzx|xz) xz -d -c "$input" > "$tmpfile" ;;
    # wanted to make a way to call feh, if you dont want
    # to convert.
    # png|jpg) 2ff < "$input" > "$tmpfile" ;;
    png|jpg) feh "$input" ; exit ;;
    *)
        echo "Unsupported format: .$ext"
        exit 1
        ;;
esac

./viewer "$tmpfile"
