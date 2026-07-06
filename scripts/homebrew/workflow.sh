#!/usr/bin/env bash

# simulate github actions runner
# run ./init.sh one time to setup environment
# run ./reset.sh first to clean the environment after each run

# homebrew doesn't like TEMP on a different file system
export HOMEBREW_TEMP=/usr/local/tmp

NMC_SRC=~/nfs/sw/nomacs
RUN_DIR=/Volumes/Scratch # ramdisk

set -e

# brew doctor
mkdir -p $RUN_DIR
rsync -auv --delete --exclude='.git/' $NMC_SRC $RUN_DIR/

# syntax check workflow; we can't run it since Docker cannot run macOS
cd $RUN_DIR/nomacs
#rm -rf ~/.cache/act
act -P macos-13=-self-hosted --workflows .github/workflows/macos.yml \
  --strict --validate
  #-s GITHUB_TOKEN=ghp_dummy

brew install --force-bottle qt5compat qtbase qtimageformats qtsvg qttools \
  googletest exiv2 libtiff libraw

# the quazip brew package has a link error on MacOS 15 SDK (at least)
cd $RUN_DIR
rm -rf quazip
git clone --depth 1 -b v1.7.2 https://github.com/stachenov/quazip.git
cd quazip && mkdir build && cd build
cmake -G Ninja -D CMAKE_INSTALL_PREFIX=/opt/quazip ..
ninja
sudo ninja install

cd $RUN_DIR
rm -rf opencv
git clone --depth 1 -b 4.13.0 https://github.com/opencv/opencv.git
cd opencv
$RUN_DIR/nomacs/scripts/build-opencv.sh -D CMAKE_INSTALL_PREFIX=/opt/opencv

export CMAKE_PREFIX_PATH="/opt/opencv:/opt/quazip"

cd $RUN_DIR/nomacs
rm -rf build
mkdir build
cd build
cmake -G Ninja -D ENABLE_QUAZIP=ON ../ImageLounge

ninja

ninja check

ninja kimageformats

ninja portable

cd portable && zip -ry nomacs.app.zip nomacs.app

