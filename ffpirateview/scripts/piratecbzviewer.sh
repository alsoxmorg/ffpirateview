# Create and clean up temp dir

VIEWER="tiv"
TMPDIR="/tmp/cbzviewer_$$"
mkdir -p "$TMPDIR"
trap 'rm -rf "$TMPDIR"' EXIT
CBZFILE="$1"
PAGENO="$2"

# Extract ZIP into temp dir
unzip -qq "$CBZFILE" -d "$TMPDIR"

# Gather and sort file list (full paths), supports nested folders if needed
IMAGES=$(find "$TMPDIR" -type f | sort)

# Zero-indexed access
INDEX=$PAGENO
if [ "$INDEX" -lt 0 ] || [ "$INDEX" -ge "${#IMAGES[@]}" ]; then
    echo "Page number out of range"
    exit 1
fi

INPUT="${IMAGES[$INDEX]}"
EXT="${INPUT##*.}"
TMPFILE="$TMPDIR/view.png"

# Optional: show what file is being processed
echo "Viewing page $PAGENO: $(basename "$INPUT") (.$EXT)"

# Decide how to process based on extension
case "$EXT" in
    jpg|jpeg)
	cp "$INPUT" "$TMPFILE"
	;;
    png)
	cp "$INPUT" "$TMPFILE"
	;;
    ff)
        ff2png < "$INPUT" > "$TMPFILE"
        ;;
    ffz|lz)
        lzip -dc "$INPUT" | ff2png > "$TMPFILE"
        ;;
    ffbz|bz2)
        bzip2 -dc "$INPUT" | ff2png > "$TMPFILE"
        ;;
    ffzx|xz)
        xz -dc "$INPUT" | ff2png > "$TMPFILE"
        ;;
    zst)
        zstd -dc "$INPUT" | ff2png > "$TMPFILE"
        ;;
    1f|1ff)
        1ff2ff "$INPUT" > "$TMPDIR/temp.ff"
        ff2png < "$TMPDIR/temp.ff" > "$TMPFILE"
        ;;
    png|jpg|jpeg)
        cp "$INPUT" "$TMPFILE"
        ;;
    *)
        echo "Unsupported format: .$EXT"
        exit 1
        ;;
esac

# Show image with TerminalImageViewer
"$VIEWER" "$TMPFILE"
