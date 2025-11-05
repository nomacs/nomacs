#!/usr/bin/env bash
set -e
sudo apt install -y patchelf cimg-dev libpng-dev libjpeg-dev libgmock-dev \
  nlohmann-json3-dev qt6-declarative-dev-tools libgpgme-dev libgcrypt-dev \
  squashfs-tools desktop-file-utils zsync

# added for Ubuntu 22.04
sudo apt install -y libglib2.0-dev libcurl4-openssl-dev

(
  git clone https://github.com/AppImage/appimagetool
  cd appimagetool &&
  git checkout 07aa21f &&
  git submodule update --init --recursive &&
  cmake -G Ninja -D USE_CCACHE=OFF . &&
  ninja &&
  sudo ninja install &&
  appimagetool --version
)

(
  git clone https://github.com/linuxdeploy/linuxdeploy
  cd linuxdeploy &&
  git checkout 98f393c &&
  git submodule update --init --recursive &&
  cmake -G Ninja -D USE_CCACHE=OFF . &&
  ninja &&
  ninja test &&
  sudo ninja install &&
  linuxdeploy --version
)

(
  git clone https://github.com/linuxdeploy/linuxdeploy-plugin-appimage
  cd linuxdeploy-plugin-appimage &&
  git checkout 61217a7 &&
  git submodule update --init --recursive &&
  cmake -G Ninja -D USE_CCACHE=OFF . &&
  ninja &&
  #ninja test &&
  sudo ninja install &&
  linuxdeploy-plugin-appimage --plugin-api-version
)

(
  git clone https://github.com/linuxdeploy/linuxdeploy-plugin-qt
  cd linuxdeploy-plugin-qt &&
  git checkout b0cb921 &&
  git submodule update --init --recursive &&
  cmake -G Ninja -D USE_CCACHE=OFF . &&
  ninja &&
  #PATH=/usr/lib/qt6/libexec:$PATH ninja test &&
  sudo ninja install &&
  linuxdeploy-plugin-qt --plugin-api-version
)

linuxdeploy --list-plugins

