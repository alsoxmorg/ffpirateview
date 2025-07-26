#!/bin/bash

# Check if file is provided
if [ -z "$1" ]; then
    echo "Usage: $0 file.ff.lz"
    exit 1
fi

input="$1"

# Check if file exists
if [ ! -f "$input" ]; then
    echo "Error: File '$input' not found."
    exit 1
fi

# Dependencies check
command -v ff2png >/dev/null 2>&1 || { echo "ff2png not found. Install it first."; exit 1; }
command -v convert >/dev/null 2>&1 || command -v magick >/dev/null 2>&1 || { echo "ImageMagick not found."; exit 1; }

# Convert to PNG
base="${input%.*}"          # strip .lz or .ff.lz
png_out="${base}.png"
lzip -d -c "$input" | ff2png  > "$png_out" || { echo "ff2png failed."; exit 1; }

# Create thumbnail
thumb="thumb.png"
if command -v magick >/dev/null 2>&1; then
    magick "$png_out" -thumbnail 128x128 "$thumb"
else
    convert "$png_out" -thumbnail 128x128 "$thumb"
fi

# Compute URI and MD5
uri="file://$(realpath "$input")"
hash=$(echo -n "$uri" | md5sum | awk '{print $1}')
thumb_dir="$HOME/.cache/thumbnails/normal"
mkdir -p "$thumb_dir"
mv "$thumb" "$thumb_dir/${hash}.png"

echo "Thumbnail created for $input → $thumb_dir/${hash}.png"
