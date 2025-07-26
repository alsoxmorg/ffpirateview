#!/bin/bash
#This script, allows you to use "tiv/TerminalImageViewer" to view
#iamges from the terminal/esp through ssh
#ff2png < youriamge.ff > tmp.png ; tiv tmp.png ; rm tmp.png

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
    png|jpg) 2ff < "$input" > "$tmpfile" ;;
    *)
        echo "Unsupported format: .$ext"
        exit 1
        ;;
esac

ff2png < "$tmpfile" > tmp5xc1.png ; tiv tmp5xc1.png; rm tmp5xc1.png
