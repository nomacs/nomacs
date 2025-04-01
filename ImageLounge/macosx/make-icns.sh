#!/bin/bash

#OUTPUT=nomacs.icns # implied
INPUT=nomacs.png # svg rasterized to 1024x1024
TMP=nomacs.iconset

rm -R $TMP
mkdir -pv $TMP

sips -z 16 16     $INPUT --out $TMP/icon_16x16.png
sips -z 32 32     $INPUT --out $TMP/icon_16x16@2x.png
sips -z 32 32     $INPUT --out $TMP/icon_32x32.png
sips -z 64 64     $INPUT --out $TMP/icon_32x32@2x.png
sips -z 128 128   $INPUT --out $TMP/icon_128x128.png
sips -z 256 256   $INPUT --out $TMP/icon_128x128@2x.png
sips -z 256 256   $INPUT --out $TMP/icon_256x256.png
sips -z 512 512   $INPUT --out $TMP/icon_256x256@2x.png
sips -z 512 512   $INPUT --out $TMP/icon_512x512.png
sips -z 1024 1024 $INPUT --out $TMP/icon_512x512@2x.png

iconutil -c icns $TMP
rm -R $TMP
