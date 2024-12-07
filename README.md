# nomacs - Image Lounge 🍸

nomacs is a free, open source image viewer, which supports multiple platforms. You can use it for viewing all common image formats including RAW and psd images. nomacs is licensed under the GNU General Public License v3 and available for Windows, Linux, FreeBSD, Mac, Haiku, and OS/2.

[![Build status](https://ci.appveyor.com/api/projects/status/ye6wd1hap4cebyo8?svg=true)](https://ci.appveyor.com/project/novomesk/nomacs)
[![Downloads](https://img.shields.io/github/downloads/nomacs/nomacs/total.svg)](https://github.com/nomacs/nomacs/releases/latest)
[![Crowdin](https://badges.crowdin.net/nomacs/localized.svg)](http://translate.nomacs.org/project/nomacs)

## Get the Source

```console
git clone https://github.com/nomacs/nomacs.git
cd nomacs
git submodule init
git submodule update
```

## Build nomacs (Windows)

We assume you have an IDE (i.e. Visual Studio), python, git, and [Qt](https://www.qt.io/download-open-source) installed.

Get all dependencies:

```bash
git submodule init
git submodule update
```

Project folders in ``3rd-party`` will not be empty anymore. Now call:

```bash
python scripts/make.py "qtpath/bin"
```

This will build nomacs into `build/nomacs`. If you are using Visual Studio, you can then double-click `build/nomacs/nomacs.sln`. Right-click the nomacs project and choose `Set as StartUp Project`.

Build individual projects using:

```bash
python scripts/make.py "qt/bin" --project quazip,libraw --force
```

### Developer Build

I like having a separate developer build (without submodules) that uses 3rd party libs already compiled. To do so you need to:

```bash
git submodule update --init --remote scripts 

# python scripts/make.py "C:\Qt\Qt-5.14.1-installer\5.14.2\msvc2017_64\bin" --lib-path C:\coding\nomacs\nomacs\3rd-party\build
python scripts/make.py "qt/bin" --lib-path "nomacs/3rd-party/build"
```

### If anything did not work

- check if you have setup opencv (otherwise uncheck ENABLE_OPENCV)
- check if your Qt is set correctly (otherwise set the path to `qt_install_dir/qtbase/bin/qmake.exe`)
- check if your builds proceeded correctly

## Build nomacs (Linux/Unix)

Before you build nomacs, please note the following:

- We recommend using the Qt6 version of nomacs. However, you may want to match the Qt version of your desktop environment (e.g. KDE in 24.04 is Qt5-based).
- [kimageformats-plugins](https://github.com/KDE/kimageformats) provides additional formats (AVIF,HEIC/HEIF) and is sometimes only available for the Qt5 version. Otherwise, you must compile it separately.
- Zip file support requires Quazip, which has varied support in distributions. If the system package is missing or does not work, you can set `USE_SYSTEM_QUAZIP=NO` to use the version in nomacs/3rdparty. However, you may also need to remove the any system quazip development package temporarily. (`llibquazip*-dev`)

### Get the required packages

#### Ubuntu 24.04

```console
# qt6
sudo apt install qt6-base-dev qt6-tools-dev qt6-svg-dev qt6-image-formats-plugins libexiv2-dev libraw-dev libopencv-dev libtiff-dev libtiff-dev libquazip1-qt6-dev build-essential git cmake lcov libgtest-dev

# qt5
# note: cmake configuration will fail if libquazip1-qt6-dev is also installed; or you can use USE_SYSTEM_QUAZIP=OFF
sudo apt install qtbase5-dev qttools5-dev libqt5svg5-dev qt5-image-formats-plugins libexiv2-dev libraw-dev libopencv-dev libtiff-dev libtiff-dev libquazip1-qt5-dev build-essential git cmake lcov libgtest-dev kimageformat-plugins
```

#### Ubuntu 22.04

```console
# qt6
sudo apt install qt6-base-dev qt6-tools-dev qt6-tools-dev-tools libqt6svg6-dev libqt6core5compat6-dev qt6-l10n-tools qt6-image-formats-plugins libexiv2-dev libraw-dev libopencv-dev libtiff-dev libtiff-dev build-essential git cmake lcov libgtest-dev libgl-dev

# qt5
sudo apt install qtbase5-dev qttools5-dev libqt5svg5-dev qt5-image-formats-plugins libexiv2-dev libraw-dev libopencv-dev libtiff-dev libtiff-dev libquazip5-dev build-essential git cmake lcov libgtest-dev kimageformat-plugins
```

#### Arch

```console
# qt6
sudo pacman --sync qt6-base qt6-imageformats qt6-svg qt6-tools quazip-qt6 kimageformats exiv2 libraw libtiff opencv kimageformats git cmake gtest base-devel make
```

#### Redhat/Fedora/CentOS (tested on Rocky 9.5)
```console
# qt6
sudo dnf install qt6-qtbase-devel qt6-qtimageformats qt6-qtsvg-devel qt6-qttools-devel qt6-qt5compat-devel LibRaw-devel opencv-devel exiv2-devel libtiff-devel git cmake lcov gtest-devel gcc-c++

# qt5
sudo dnf install qt5-qtbase-devel qt5-qtimageformats qt5-qtsvg-devel qt5-qttools-devel LibRaw-devel opencv-devel exiv2-devel libtiff-devel git cmake lcov gtest-devel gcc-c++ quazip-qt5-devel
```

#### FreeBSD (14.2 release)

```
# qt6
sudo pkg install qt6-base qt6-imageformats qt6-svg qt6-5compat qt6-tools quazip-qt6 tiff exiv2 kf6-kimageformats libraw opencv git cmake googletest gcc

#qt5
sudo pkg install qt5-gui qt5-imageformats
qt5-svg qt5-linguisttools qt5-qmake qt5-buildtools qt5-uitools qt5-concurrent quazip-qt5 tiff exiv2 kf5-kimageformats libraw opencv git cmake googletest gcc
```

#### Haiku (r1 beta 5)
```console
# qt6
pkgman install qt6_base_devel qt6_tools_devel qt6_svg_devel qt6_5compat_devel quazip1_qt6_devel tiff_devel libraw_devel opencv_devel gtest_devel exiv2_devel kimageformats6 qt6_imageformats cake git gcc make pkgconfig lcms_devel
```

### Configure nomacs

Nomacs is configured with cmake. These cmake options are often needed:
- QT_VERSION_MAJOR=[5|6] - Default 5, 6 recommended
- ENABLE_QUAZIP=[ON|OFF] - Default OFF, ON recommended
- USE_SYSTEM_QUAZIP=[ON|OFF] - Default ON, recommended
- CMAKE_BUILD_TYPE=[Debug|Release|RelWithDebInfo] - Debug builds have more logging as well as debug symbols.


```console
cd nomacs
mkdir build
cd build
cmake -D QT_VERSION_MAJOR=5 -D ENABLE_QUAZIP=ON -D USE_SYSTEM_QUAZIP=ON ../ImageLounge
```

### Compile nomacs

```console
make [-j 8]
```

You will now have a binary (~/nomacs/build/nomacs), which you can use directly, however you may need to set LD_LIBRARY_PATH for this to work. You can check the log output to see that nomacs is loading configuration files from the expected locations.

```console
# running nomacs from ~/nomacs/build/
LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH ./nomacs
```

## Build and run tests

Nomacs uses googletest (libgtest-dev), this is included in package lists above. A `check`target will be created if GoogleTest is present. To build and run tests:

```bash
make check
```


### Install nomacs

By default nomacs is installed to /usr/local/ unless you set `CMAKE_INSTALL_PREFIX`

```console
sudo make install
# ldconfig is required on most linux distros
# Some systems you also require changes to 
# `/etc/ld.so.conf` to add `/usr/local/lib` 
sudo ldconfig
```

### For Package Maintainers

- Recommended additional packages: kimageformats provides HEIF/HEIC and AVIF image loading, and many others
- Set `ENABLE_TRANSLATIONS` to `true` (default: `true`)
- Build all officially supported [plugins](https://github.com/nomacs/nomacs-plugins/), enabled by default if `3rd-party/plugins` is present.

## Build nomacs (MacOS)

Install [Homebrew](http://brew.sh/) for easier installation of dependencies.
Install required dependencies:

```console
brew install qt5 exiv2 opencv libraw quazip cmake pkg-config
```

Go to the `nomacs` directory and run cmake to get the Makefiles:

```console
cd nomacs
mkdir build
cd build
```

For Homebrew on Intel models:

```console
# qt5
$ export CPLUS_INCLUDE_PATH=/usr/local/include
$ Qt5_DIR=/usr/local/opt/qt5/ cmake -D QT_VERSION_MAJOR=5 --install-prefix /Applications ../ImageLounge/.

# qt6
$ cmake -D QT_VERSION_MAJOR=6 -D ENABLE_QUAZIP=ON --install-prefix /Applications ../ImageLounge/.
```

For Homebrew on Apple Silicon models:

```console
$ Qt5_DIR=/opt/homebrew/opt/qt5/ cmake -DQT_QMAKE_EXECUTABLE=/opt/homebrew/opt/qt5/bin/qmake --install-prefix /Applications ../ImageLounge/.
```

Run make:

```console
$ make
```

You will now have a binary (`nomacs.app`), which you can test (or use directly). To install it to `/Applications`, use

```console
sudo make install
```

If you want to have an independent bundle image (`nomacs.dmg`) you c
an create it by using

```console
$ make bundle
```

If macdeployqt complains about `ERROR: Cannot resolve rpath "@rpath/QtGui.framework/Versions/A/QtGui"` [here](https://github.com/orgs/Homebrew/discussions/2823#discussioncomment-2010340) is the solution:
```console
$ cd /usr/local/lib/QtGui.framework/Versions/A
$ install_name_tool -id '@rpath/QtGui.framework/Versions/A/QtGui' QtGui
% otool -L QtGui| head -2
QtGui:
        @rpath/QtGui.framework/Versions/A/QtGui (compatibility version 6.0.0, current version 6.7.0)
```

## Build nomacs (Windows Cross-Compile)

Compiles nomacs for Windows using M Cross Environment (MXE) from a Linux/Unix host. 

MXE environment is usually compiled from source, however you may be able to skip this if MXE has packages for your platform:

```bash
git clone <mxe url>
cd mxe

# qt5
make MXE_TARGETS='x86_64-w64-mingw32.shared' qtbase qtimageformats qtwinextras opencv quazip tiff exiv2 libraw

# qt6 (quazip-qt6 is unavailable)
make MXE_TARGETS='x86_64-w64-mingw32.shared' qt6-qtbase qt6-qtimageformats qt6-qttools qt6-qt5compat opencv tiff exiv2 libraw
```

Setup build environment:

```bash
export MXE_DIR=~/mxe
export MXE_TARGET=x86_64-w64-mingw32.shared
export PATH="${MXE_DIR}/usr/bin:$PATH"

alias pkg-config=${MXE_TARGET}-pkg-config
alias qmake=${MXE_TARGET}-qmake
alias cmake=${MXE_TARGET}-cmake
```

Run cmake out-of-tree:

```bash
mkdir build-nomacs
cd build-nomacs

cmake -D QT_VERSION_MAJOR=5 -D ENABLE_TRANSLATIONS=ON -D ENABLE_HEIF=ON -D ENABLE_AVIF=ON  \
-DENABLE_JXL=ON -D USE_SYSTEM_QUAZIP=OFF <nomacs-path>/ImageLounge
```

Compile nomacs:

```bash
make -j8
make collect
```

Run on WINE:

```bash
wine nomacs-mingw/nomacs.exe
```

Run on Windows:

```console
cd C:\
xcopy /DEY <shared-folder>\build-nomacs\nomacs-mingw nomacs
cd nomacs
nomacs.exe
```

## Build nomacs (Windows with MSYS2)

```bash
export target=mingw-w64-x86_64

pacman -S $target-qt5-base $target-qt5-svg $target-qt5-winextras $target-qt5-tools \
          $target-libraw $target-libtiff $target-exiv2 $target-opencv $target-cmake $target-gcc

export PATH=/ming64:$PATH

cd <build-dir>

cmake -D QT_MAJOR_VERSION=5 -D ENABLE_TRANSLATIONS=ON -D USE_SYSTEM_QUAZIP=ON <nomacs-dir>/ImageLounge

cmake --build . --parallel 8
```


## Build in Docker

We have created a docker image that best simulates the old travis system (currently it's ubuntu xenial 16.04). To build nomacs in a docker, you have to create the image:

```bash
docker build --rm -f "Dockerfile" -t nomacs:latest empty-docker-dir
```

To deploy nomacs in a docker on your system, you can mount this directory using:

```bash
docker run --rm -it -v C:\\coding\\nomacs:/usr/nomacs nomacs:latest
```

If needed, you can upload the image:

```bash
docker login
docker tag nomacs diemmarkus/nomacs
docker push diemmarkus/nomacs:latest
```


## Enable Debug output

To enable Qt debug messages set

```bash
cmake -D CMAKE_BUILD_TYPE=Debug ...
```

at the Makefiles generation phase.


## Links

- [nomacs.org](https://nomacs.org)
- [GitHub](https://github.com/nomacs)
- [Matrix chat room](https://matrix.to/#/#nomacs:matrix.org)

[![nomacs-icon](https://nomacs.org/nomacs.svg)](https://nomacs.org)
