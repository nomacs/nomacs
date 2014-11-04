#!/bin/sh

# Copyright (c) 2013, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# Before to run this script you must set these shell variable :
# $EXIVCoverityToken with token of Exiv2 project given by Coverity SCAN
# $EXIVCoverityEmail with email adress to send SCAN result.
#
# Coverity Scan bin dir must be appended to PATH variable.
#
# See this url to see how to prepare your computer with Coverity SCAN tool:
# http://scan.coverity.com/projects/297/upload_form

cd ..

# Manage build sub-dir
if [ -d "build" ]; then
    rm -rfv ./build
fi

./bootstrap.linux

# Get active svn branch path to create SCAN import description string
svn info | grep "URL" | sed '/svn/{s/.*\(svn.*\)/\1/};' > ./build/svn_branch.txt
desc=$(<build/svn_branch.txt)

cd ./build

cov-build --dir cov-int --tmpdir ~/tmp make
tar czvf myproject.tgz cov-int

echo "-- SCAN Import description --"
echo $desc
echo "-----------------------------"

echo "Coverity Scan tarball 'myproject.tgz' uploading in progress..."

nslookup scan5.coverity.com
SECONDS=0

curl --form project=Exiv2 \
     --form token=$EXIVCoverityToken \
     --form email=$EXIVCoverityEmail \
     --form file=@myproject.tgz \
     --form version=svn-trunk \
     --form description="$desc" \
     http://scan5.coverity.com/cgi-bin/upload.py

echo "Done. Coverity Scan tarball 'myproject.tgz' is uploaded and ready for analyse."
echo "That took approximately $SECONDS seconds to upload."

