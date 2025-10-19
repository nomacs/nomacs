#!/usr/bin/env bash

# pull all crowdin translations and merge them

abort() { echo -e "\npull-crowdin-translations: $1\n" >&2; exit 1; }

# output directories
CROWDIN=tmp.pull.crowdin
CLEAN=tmp.pull.clean

if [ -d $CROWDIN ]; then rmdir $CROWDIN || exit 1; fi
if [ -d $CLEAN   ]; then rmdir $CLEAN   || exit 2; fi

mkdir -v $CROWDIN || exit 3;
mkdir -v $CLEAN || exit 4;

 ./download-translations.py "$CROWDIN" || abort "crowdin download failed"

#
# fix incorrect source locations due to nomacs.ts and nomacs_<lang>.ts
# being located in different directories
#
for f in $CROWDIN/*.ts; do
    name=$(basename $f)
    echo "cleaning $name"
    sed 's#filename="../../src#filename="../src#' $f > $CLEAN/$name || abort "failed to clean $f"
done

# merge into current translations
./import-translations.sh $CLEAN

# cleanup to reduce churn as much as possible
./update-translation.sh
