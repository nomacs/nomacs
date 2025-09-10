#!/usr/bin/env bash

#
# Import/merge new external translations files (.ts) into nomacs.
#
# - the ts file must have unix line endings
# - the ts file name must match the nomacs file name
# - modified strings are merged; the new string wins if there is a conflict
# - after running, you probably want to also run ./update-translation.sh
#
# Arguments: $1 - directory with the new translation file(s) in .ts format
#
for SRC_FILE in "$1"/*.ts; do
  DST_FILE="../$(basename "$SRC_FILE")"
  if [ ! -f "$DST_FILE" ]; then
     echo "no translation to merge with: $DST_FILE, copying..."
     cp -v --update=none "$SRC_FILE" "$DST_FILE" || exit 1
     continue
  fi

  lconvert -verbose -sort-contexts -o "$DST_FILE" "$DST_FILE" "$SRC_FILE" || exit 2
done

echo All files processed succesfully, now run ./update-translation.sh to clean up the files
