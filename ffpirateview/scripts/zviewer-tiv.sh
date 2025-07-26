#!/bin/bash

set -e  # Exit on errors

input="$1"
ext="${input##*.}"
base=$(basename "$input")
tmpfile="tmp436536456.png"

cleanup() {
    rm -f "$tmpfile"
}
trap cleanup EXIT

case "$ext" in
    ff) ff2png < "$input" > "$tmpfile" ;;
    ffz|lz) lzip -d -c "$input" | ff2png > "$tmpfile" ;;
    ffbz|bz2) bzip2 -d -c "$input" | ff2png > "$tmpfile" ;;
    ffzx|xz) xz -d -c "$input" | ff2png > "$tmpfile" ;;
    1f|1ff) 1ff2ff "$input" > pretemp.ff
	    ff2png < pretemp.ff > "$tmpfile"
	    rm pretemp.ff ;;
    png|jpg|jpeg) cp "$input" "$tmpfile" ;;
    *)
        echo "Unsupported format: .$ext"
        exit 1
        ;;
esac

#[ -f pretemp.ff ] && rm pretemp.ff

tiv "$tmpfile"
[ -f pretemp.ff ] && rm pretemp.ff
[ -f "$tmpfile" ] && rm "$tmpfile"
