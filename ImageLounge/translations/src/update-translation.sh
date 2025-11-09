#!/bin/bash

# This script scans .cpp files and creates a .ts file for
# linguist or uploading to CrowdIn.
#
# To reduce churn in .ts files, remove line numbers by default.
# they can be added back temporarily for editing purposes
# using --keep-line-numbers
#
STRIP=1
LUPDATE_OPTIONS="-no-obsolete"
if [ "$1" = "--keep-line-numbers" ]; then
    echo $0: keeping line numbers, please remove before committing
    STRIP=0
fi

lupdate  ../../src/ $LUPDATE_OPTIONS -ts nomacs.ts > update.log || exit 1
[ $STRIP -eq 1 ] && (sed -ri 's#(<location.*) line=.*>#\1/>#g' nomacs.ts || exit 5)

cd ..
for f in *.ts; do
    lupdate ../src/ $LUPDATE_OPTIONS -ts "$f" >> src/update.log || exit 2
    ./src/clean-unfinished.py "$f" "$f" >> src/update.log  || exit 3
    [ $STRIP -eq 1 ] && (sed -ri 's#(<location.*) line=.*>#\1/>#g' "$f" || exit 6)
done

echo $0: translation files updated, check update.log
