#!/bin/bash
# remove build paths from the rpath
# this would be anything not under /usr/local or /opt
FILE="$1"

RPATHS=$(otool -l "$FILE" | grep -A2 LC_RPATH | grep " path " | cut -w -f3 | grep -vE "^(/usr/local|/opt)") || exit 1

LIB=$(basename "$FILE")
ARGS=
for RPATH in $RPATHS; do
  echo "strip_rpath: removing $RPATH : $LIB"
  ARGS="$ARGS -delete_rpath $RPATH"
done

if [ -n "$ARGS" ]; then
  install_name_tool $ARGS $FILE || exit 2
fi
