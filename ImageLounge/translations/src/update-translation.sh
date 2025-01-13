#!/bin/bash

lupdate  ../../src/ -no-obsolete -ts nomacs.ts > update.log

cd ..
for f in *.ts; do
    lupdate ../src/ -no-obsolete -ts "$f" >> src/update.log
done

echo translation files updated, check update.log
