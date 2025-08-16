#!/bin/sh

# convert all text files to unix line endings
find . \
    -type f \
    -and '(' \
        -name '*.cpp' \
        -or -name '*.h' \
        -or -name '*.c' \
        -or -name '*.py' \
        -or -name '*.sh' \
        -or -name '*.svg' \
        -or -name '*.ts' \
        -or -name '*.cmake' \
        -or -name '*.in' \
        -or -name '*.txt' \
        -or -name '*.yml' \
        -or -name '*.md' \
        -or -name 'COPYRIGHT' \
        -or -name 'LICENSE' \
        -or -name 'LICENSE.*' \
        -or -name 'README' \
        -or -name 'TODO' \
        -or -name '*.qrc' \
        -or -name '*.json' \
        -or -name '*.xml' \
        -or -name '*.plist' \
    ')' \
    -and '(' -not -path './3rd-party/*' ')' \
    -and '(' -not -path './.git/*' ')' \
    -and '(' -not -path './build/*' ')' \
    -and '(' -not -path './ImageLounge/build/*' ')' \
    -exec dos2unix '{}' ';'
