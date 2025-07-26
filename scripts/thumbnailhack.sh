#!/bin/bash

# XDG thumbnail dir (for file managers like Nautilus, Thunar, etc.)
#thumbdir="${XDG_CACHE_HOME:-$HOME/.cache}/thumbnails/normal"
thumbdir="$HOME/.cache/thumbnails/normal"
mkdir -p "$thumbdir"

# Thumbnail size
size=256

# Loop through farbfeld-like files
find . -type f \( \
    -iname "*.ff" -o \
    -iname "*.ffz" -o -iname "*.ff.lz" -o \
    -iname "*.ffbz" -o -iname "*.ff.bz2" -o \
    -iname "*.ffzx" -o -iname "*.ff.xz" \
\) | while read -r file; do
    absfile=$(readlink -f "$file")
    uri="file://$absfile"
    hash=$(echo -n "$uri" | md5sum | cut -d' ' -f1)
    thumbfile="$thumbdir/$hash.png"

    # Skip if thumbnail exists
    [[ -f "$thumbfile" ]] && continue

    # Create a temporary farbfeld file
    tmpff=$(mktemp --suffix .ff)
    tmppng="tmp555.png"

    # Decompress or copy
    case "$file" in
        *.ff) ff2png < "$file" > "$tmppng" ;;
        *.ffz|*.ff.lz) lzip -dc | ff2png > "$tmppng" ;;
        *.ffbz|*.ff.bz2) bunzip2 -dc | ff2png > "$tmppng" ;;
        *.ffzx|*.ff.xz) xz -dc  | ff2png > "$tmppng" ;;
    esac

    # Convert tmp png to→ thumbnail PNG using ImageMagick
    convert "$tmppng" -thumbnail "${size}x${size}" "$thumbfile" && \
        echo "Created thumbnail for: $file"

    rm -f "$tmppng"
done
