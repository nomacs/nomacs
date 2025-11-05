#!/usr/bin/env bash

# build a minimal OpenCV for nomacs

set -e

git submodule update --init --depth 1 3rd-party/opencv

mkdir 3rd-party/opencv/build 

cd 3rd-party/opencv/build

cmake .. \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_LIST=core,imgproc \
  -DBUILD_opencv_world=OFF \
  -DBUILD_TESTS=OFF \
  -DBUILD_PERF_TESTS=OFF \
  -DBUILD_EXAMPLES=OFF \
  -DWITH_IMGCODEC_GIF=OFF \
  -DWITH_IMGCODEC_HDR=OFF \
  -DWITH_IMGCODEC_PFM=OFF \
  -DWITH_IMGCODEC_PXM=OFF \
  -DWITH_IMGCODEC_SUNRASTER=OFF \
  -DWITH_JPEG=OFF \
  -DWITH_JPEG2000=OFF \
  -DWITH_JASPER=OFF \
  -DWITH_OPENJPEG=OFF \
  -DWITH_PNG=OFF \
  -DWITH_TIFF=OFF \
  -DWITH_WEBP=OFF \
  -DWITH_OPENEXR=OFF \
  -DWITH_GDCM=OFF \
  -DWITH_AVIF=OFF \
  -DWITH_FFMPEG=OFF \
  -DWITH_GSTREAMER=OFF \
  -DWITH_1394=OFF \
  -DWITH_V4L=OFF \
  -DWITH_OPENCL=OFF \
  -DWITH_IPP=OFF \
  -DWITH_PROTOBUF=OFF \
  -DWITH_QT=OFF \
  -DWITH_GTK=OFF \
  -DWITH_GTK_2_X=OFF \
  -DWITH_OPENGL=OFF \
  "$@"
   
ninja

sudo ninja install
 
