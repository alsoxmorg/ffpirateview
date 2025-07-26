#!/bin/bash

# Check for input
if [ -z "$1" ]; then
  echo "Usage: $0 <file.cbz>"
  exit 1
fi

CBZ_FILE="$1"
BASENAME=$(basename "$CBZ_FILE" .cbz)
WORKDIR="$BASENAME.sent"

# 1. Extract the CBZ file
mkdir -p "$WORKDIR"
unzip -q "$CBZ_FILE" -d "$WORKDIR"

# 2. Sort images (assumes standard comic naming: page001.jpg, etc.)
#IMAGES=$(find "$WORKDIR" -type f \( -iname '*.jpg' -o -iname '*.png' -o -iname '*.jpeg' \) | sort)

IMAGES=$(find "$WORKDIR" -type f \( \
  -name '*.jpg' -o -name '*.JPG' -o \
  -name '*.jpeg' -o -name '*.JPEG' -o \
  -name '*.png' -o -name '*.PNG' \
\) | sort)

# 3. Create .sent file
SENT_FILE="$WORKDIR/$BASENAME.sent"
> "$SENT_FILE"
for img in $IMAGES; do
    echo "@$img" >> "$SENT_FILE"
    echo "" >> "$SENT_FILE"
done

# 4. Launch sent
sent "$SENT_FILE"
