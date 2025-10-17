#!/bin/bash

lupdate  ../../src/ -no-obsolete -ts nomacs.ts > update.log || exit 1

cd ..
for f in *.ts; do
    lupdate ../src/ -no-obsolete -ts "$f" >> src/update.log || exit 2
    ./src/clean-unfinished.py "$f" "$f" >> src/update.log  || exit 3
done

echo $0: translation files updated, check update.log
