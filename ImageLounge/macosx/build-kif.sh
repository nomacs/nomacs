#!/bin/bash

#
# script to build kimageformats on homebrew, tested on 10.13 / XCode 15.2
#
# FIXME:
#   - downgrade needed for extra-cmake-modules
#
# NOTE:
#   - no conflicts with homebrew as it doesn't have kimageformats package
#
#   - homebrew enables the old Qt jpeg2000 plugin based on libjasper, however
#     it will crash nomacs. If this plugin is deleted, the associated
#     cmake files must also be removed or else cmake will fail
#

# final install dir; should be an argument...probably
INSTALL_DIR="$1"
if [ ! -d "$INSTALL_DIR" ]; then echo "plugins/imageformats location invalid: \"$INSTALL_DIR\""; exit 99; fi

# highly annoying auto-update behavior potentially breaking things
export HOMEBREW_NO_AUTO_UPDATE=1

# downgrade for extra-cmake-modules 6.13, unavailable in homebrew
KIF_VERSION="v6.12.0-rc1"

SRC_DIR="kimageformats"
BUILD_DIR="kimageformats/build"

brew install --quiet libde265 openexr libavif libheif jpeg-xl openjpeg jxrlib ninja \
  extra-cmake-modules karchive vulkan-headers || exit 1

if [ ! -d $SRC_DIR ]; then git clone https://invent.kde.org/frameworks/kimageformats.git || exit 2; fi

(rm -rf $BUILD_DIR &&
 mkdir -p $BUILD_DIR
) || exit 3

(cd $BUILD_DIR &&
  git checkout $KIF_VERSION
) || exit 4

(cd $BUILD_DIR &&
 cmake -D CMAKE_TOOLCHAIN_FILE=/usr/local/lib/cmake/Qt6/qt.toolchain.cmake \
       -D CMAKE_BUILD_TYPE=Release \
       -D CMAKE_INSTALL_PREFIX=./install \
       -D KIMAGEFORMATS_JP2=ON \
       -D KIMAGEFORMATS_JXR=ON \
       -D KIMAGEFORMATS_JXL=ON \
       -D KIMAGEFORMATS_HEIF=ON \
      ..
) || exit 5

(cd $BUILD_DIR &&
  cmake --build . --parallel &&
  cmake --install .
) || exit 6

rsync -auv kimageformats/build/install/lib/plugins/imageformats/ \
           "$INSTALL_DIR/" || exit 7

echo SUCCESS: kimageformats plugins installed in $INSTALL_DIR/
