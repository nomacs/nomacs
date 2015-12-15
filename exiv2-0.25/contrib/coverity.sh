#!/bin/sh

# Copyright (c) 2013-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
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
if [ -d "build.cmake" ]; then
    rm -rfv ./build.cmake
fi

if [[ "$OSTYPE" == "linux-gnu" ]]; then
    ./bootstrap.linux
elif [[ "$OSTYPE" == "darwin"* ]]; then
    ./bootstrap.macports
else
    echo "Unsupported platform..."
    exit -1
fi

# Get active svn branch path to create SCAN import description string
svn info | grep "URL" | sed '/svn/{s/.*\(svn.*\)/\1/};' > ./build.cmake/svn_branch.txt
desc=$(<build.cmake/svn_branch.txt)

cd ./build.cmake

cov-build --dir cov-int --tmpdir ~/tmp make -j8
tar czvf myproject.tgz cov-int

echo "-- SCAN Import description --"
echo $desc
echo "-----------------------------"

echo "Coverity Scan tarball 'myproject.tgz' uploading in progress..."

nslookup scan5.coverity.com
SECONDS=0

curl -# \
     --form token=$EXIVCoverityToken \
     --form email=$EXIVCoverityEmail \
     --form file=@myproject.tgz \
     --form version=svn-trunk \
     --form description="$desc" \
     https://scan.coverity.com/builds?project=Exiv2 \
     > /dev/null

echo "Done. Coverity Scan tarball 'myproject.tgz' is uploaded and ready for analyse."
echo "That took approximately $SECONDS seconds to upload."

