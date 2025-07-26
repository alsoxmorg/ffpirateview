#!/bin/bash

#This one "works"
# Usage: cbzviewer FILE.cbz PAGE_NUMBER

CBZFILE="$1"
PAGENO="$2"
VIEWER="sxiv"


if [ -z "$CBZFILE" ] || [ -z "$PAGENO" ]; then
    echo "Usage: $0 <file.cbz> <page_number>"
    exit 1
fi

if [ ! -f "$CBZFILE" ]; then
    echo "File not found: $CBZFILE"
    exit 1
fi

# Create a temp directory for this session
TMPDIR="/tmp/cbzviewer_$$"
mkdir -p "$TMPDIR"

# Extract images if not already done
unzip -qq "$CBZFILE" -d "$TMPDIR"

# Sort files (to handle 001.jpg, 002.jpg, etc. properly)
IMAGES=($(ls "$TMPDIR" | sort))

# Convert page number to array index (bash arrays are 0-indexed)
INDEX=$((PAGENO - 1))

if [ $INDEX -lt 0 ] || [ $INDEX -ge ${#IMAGES[@]} ]; then
    echo "Page number out of range."
    exit 1
fi

# Show the selected image using tiv
FILEME="$TMPDIR/${IMAGES[$INDEX]}"

EXT="${FILEME##*.}"

case "$EXT" in
    jpeg|jpg)
	"$VIEWER" "$FILEME" #"$TMPDIR/${IMAGES[$INDEX]}"
	;;
    png)
	"$VIEWER" "$FILEME" #"$TMPDIR/${IMAGES[$INDEX]}"
	;;
    ff)
	ff2png < "$FILEME" > "/tmp/comicffff.png"
	"$VIEWER" "/tmp/comicffff.png"
	;;
    ffz|lz)
	lzip -cd "$FILEME" | lel #| ff2png > "/tmp/comicffff.png"
	#"$VIEWER" "/tmp/comicffff.png"
	;;
esac
