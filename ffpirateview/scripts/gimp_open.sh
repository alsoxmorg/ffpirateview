#!/bin/bash

set -e

input="$1"
[ -z "$input" ] && echo "Usage: $0 <file.ff | file.ffz | file.ffbz | ...>" && exit 1

ext="${input##*.}"
tmpff="/tmp/tmp.$RANDOM.gimp.ff"
tmppng="/tmp/tmp.$RANDOM.gimp.png"

# Decompress based on extension
case "$input" in
    *.ff) cp "$input" "$tmpff" ;;
    *.ffz|*.ff.lz)  lzip -cd "$input" > "$tmpff" ;;
    *.ffbz|*.ff.bz2) bunzip2 -c "$input" > "$tmpff" ;;
    *.ffxz|*.ff.xz)  xz -cd "$input" > "$tmpff" ;;
    *) echo "Unknown or unsupported file type: $input" && exit 1 ;;
esac

# Convert to PNG
ff2png < "$tmpff" > "$tmppng"

# Open in GIMP
gimp "$tmppng" &

# Optional: clean up temp files later
# Uncomment this if you want auto-delete after GIMP exits
# ( gimp "$tmppng"; rm -f "$tmpff" "$tmppng" ) &

exit 0

