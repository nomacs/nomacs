@echo off

IF NOT EXIST pkgconf (
  git clone --depth 1 -b pkgconf-3.0.3 https://github.com/pkgconf/pkgconf.git pkgconf
)

IF NOT EXIST zlib (
  git clone --depth 1 -b 2.3.3 https://github.com/zlib-ng/zlib-ng.git zlib 
)

IF NOT EXIST libjpeg-turbo (
  git clone --depth 1 -b 3.2.0 https://github.com/libjpeg-turbo/libjpeg-turbo.git libjpeg-turbo
)

IF NOT EXIST openjpeg (
  git clone --depth 1 -b v2.5.4 https://github.com/uclouvain/openjpeg.git openjpeg 
)

IF NOT EXIST dav1d (
  git clone --depth 1 -b 1.5.4 https://code.videolan.org/videolan/dav1d.git dav1d 
)

IF NOT EXIST lcms2 (
  git clone --depth 1 -b lcms2.19.1 https://github.com/mm2/Little-CMS.git lcms2
)

IF NOT EXIST libjxl (
  git clone --depth 1 --shallow-submodules --recursive -b v0.12.0 https://github.com/libjxl/libjxl.git libjxl 
)

IF NOT EXIST libaom (
  git clone --depth 1 -b v3.14.1 https://aomedia.googlesource.com/aom libaom 
)

IF NOT EXIST libwebp (
  git clone -b v1.6.0 --depth 1 https://chromium.googlesource.com/webm/libwebp libwebp
)

IF NOT EXIST libavif (
  git clone --depth 1 -b v1.4.2 https://github.com/AOMediaCodec/libavif.git libavif
  cd libavif\ext
  git clone --single-branch https://chromium.googlesource.com/libyuv/libyuv
  cd libyuv
  git checkout 5d03bf9ba
  cd ..\..\..
)

IF NOT EXIST openexr (
  git clone --depth 1 -b v3.4.13 https://github.com/AcademySoftwareFoundation/openexr.git openexr
)

IF NOT EXIST xz (
  git clone --depth 1 -b v5.8.3 https://github.com/tukaani-project/xz.git xz
)

IF NOT EXIST zstd (
  git clone --depth 1 -b v1.5.7 https://github.com/facebook/zstd.git zstd
)

IF NOT EXIST libtiff (
  git clone --depth 1 -b v4.7.2 https://gitlab.com/libtiff/libtiff.git libtiff
)

IF NOT EXIST opencv (
  git clone --depth 1 -b 5.0.0 https://github.com/opencv/opencv.git opencv
)

IF NOT EXIST libexpat (
  git clone --depth 1 -b R_2_8_2 https://github.com/libexpat/libexpat.git libexpat
)

IF NOT EXIST exiv2 (
  git clone --depth 1 -b v0.28.8 https://github.com/Exiv2/exiv2.git exiv2
)

IF NOT EXIST libraw (
  mkdir libraw
  cd libraw
  git clone --depth 1 https://github.com/LibRaw/LibRaw-cmake.git
  git clone --depth 1 -b 0.22.2 https://github.com/LibRaw/LibRaw.git
  copy ..\!patches\libraw-build-source.txt CMakeLists.txt
  cd ..
)

IF NOT EXIST quazip (
  git clone --depth 1 -b v1.7.2 https://github.com/stachenov/quazip.git quazip
)

IF NOT EXIST libde265 (
  git clone --depth 1 https://github.com/strukturag/libde265.git libde265
)

IF NOT EXIST openh264 (
  git clone --depth 1 -b v2.6.0 https://github.com/cisco/openh264.git openh264
)

IF NOT EXIST x264 (
  git clone --depth 1 -b stable https://code.videolan.org/videolan/x264.git x264
)

IF NOT EXIST libheif (
  git clone --depth 1 -b v1.23.1 https://github.com/strukturag/libheif.git libheif
)

IF NOT EXIST extra-cmake-modules (
  git clone --depth 1 -b v6.28.0 https://invent.kde.org/frameworks/extra-cmake-modules.git extra-cmake-modules
)

IF NOT EXIST jxrlib (
  git clone --depth 1 https://github.com/mircomir/jxrlib.git jxrlib
  copy !patches\jxrlib-build-source.txt jxrlib\CMakeLists.txt
)

IF NOT EXIST karchive (
  git clone --depth 1 -b v6.28.0 https://invent.kde.org/frameworks/karchive.git karchive
)

IF NOT EXIST kimageformats (
  git clone --single-branch https://invent.kde.org/frameworks/kimageformats.git kimageformats
  cd kimageformats
  git checkout ad0fb9dde4ffb1977a3ea800d90a26665de687b8
  cd ..
)
