#!/usr/bin/env bash

# setup or reset back to a minimum build environment
# similar to github actions runner
# before running, setup ramdisks with ./ramdisk.sh

export NONINTERACTIVE=1 # homebrew
export HOMEBREW_TEMP=/usr/local/tmp

# with ramdisk mounted this won't touch actual /usr/local
sudo rm -rf /usr/local/*
sudo rm -rf /opt/opencv

set -e

sudo chown root:wheel /usr/local
sudo chmod 755 /usr/local

sudo mkdir $HOMEBREW_TEMP
sudo chown root:wheel $HOMEBREW_TEMP
sudo chmod 755 $HOMEBREW_TEMP

/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

brew install --force-bottle act cmake pkgconfig ninja

echo SETUP SUCCESSFUL
