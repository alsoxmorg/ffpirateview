#!/bin/bash
#convert -dither FloydSteinberg -remap pattern:gray50 aidungeongirls.png aidungeongirls.1b.png

TMPFILE="/tmp/e65b370d9bc8483199fd549d6a602708.png"

convert  -dither FloydSteinberg -remap pattern:gray50  "$1" $TMPFILE 
png21f /tmp/234c5342.png "$1".1ff
rm /tmp/234c5342.png
