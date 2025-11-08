#!/usr/bin/env bash

# simulate github actions runner
# run ./reset.sh first to clean the environment

# homebrew doesn't like TEMP on a different filessytem
export HOMEBREW_TEMP=/usr/local/tmp

NMC_SRC=~/nfs/sw/nomacs
RUN_DIR=/Volumes/Scratch # ramdisk

set -e

# brew doctor
mkdir -p $RUN_DIR
rsync -auv --delete $NMC_SRC $RUN_DIR/ 

cd $RUN_DIR/nomacs

#rm -rf ~/.cache/act

# syntax check workflow; we can't run it since Docker cannot run macOS
act -P macos-13=-self-hosted --workflows .github/workflows/macos.yml \
  --strict --validate
  #-s GITHUB_TOKEN=ghp_dummy

brew install --force-bottle qt5compat qtbase qtimageformats qtsvg qttools \
  googletest exiv2 libtiff libraw

./scripts/build-opencv.sh -D CMAKE_INSTALL_PREFIX=/opt/opencv
export CMAKE_PREFIX_PATH=/opt/opencv

mkdir build
cd build
cmake -G Ninja ../ImageLounge

ninja

ninja check

ninja kimageformats

ninja portable

cd portable && zip -ry nomacs.app.zip nomacs.app

