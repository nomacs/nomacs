# nomacs - Image Lounge 🍸

nomacs is a free, open source image viewer, which supports multiple platforms. You can use it for viewing all common image formats including RAW and psd images. nomacs is licensed under the GNU General Public License v3 and available for Windows, Linux, FreeBSD, Mac, and OS/2.

[![Build status](https://ci.appveyor.com/api/projects/status/ye6wd1hap4cebyo8?svg=true)](https://ci.appveyor.com/project/novomesk/nomacs)
[![Downloads](https://img.shields.io/github/downloads/nomacs/nomacs/total.svg)](https://github.com/nomacs/nomacs/releases/latest)
[![Crowdin](https://badges.crowdin.net/nomacs/localized.svg)](http://translate.nomacs.org/project/nomacs)

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

## Build nomacs (Ubuntu)

Get the required packages:

- For Ubuntu 21.04 and above:

  ``` console
  sudo apt-get install debhelper cdbs qt5-qmake qttools5-dev-tools qtbase5-dev qttools5-dev libqt5svg5-dev qt5-image-formats-plugins libexiv2-dev libraw-dev libopencv-dev cmake libtiff-dev libquazip5-dev libwebp-dev git build-essential lcov libzip-dev
  ```

- For older Ubuntu versions:

  ``` console
  sudo apt-get install debhelper cdbs qt5-qmake qttools5-dev-tools qt5-default qttools5-dev libqt5svg5-dev qt5-image-formats-plugins libexiv2-dev libraw-dev libopencv-dev cmake libtiff-dev libquazip5-dev libwebp-dev git build-essential lcov libzip-dev
  ```

Clone the nomacs repository from GitHub:

``` console
git clone https://github.com/nomacs/nomacs.git
```

This will by default place the source into ~/nomacs
Go to the nomacs/ImageLounge directory and run `cmake` to get the Makefiles:

``` console
mkdir build
cd build
cmake ../ImageLounge/.
```

Compile nomacs:

``` console
make
```

You will now have a binary (~/nomacs/build/nomacs), which you can test (or use directly). To install it to /usr/local/bin, use:

``` console
sudo make install
```

note that you have to execute

``` console
sudo ldconfig
```

after a successful install.

Install the [heif plugin](https://github.com/jakar/qt-heif-image-plugin) for HEIF support.

### For Package Maintainers

- Set `ENABLE_TRANSLATIONS` to `true` (default: `false`)
- Build all officially supported [plugins](https://github.com/nomacs/nomacs-plugins/)

## Build nomacs (MacOS)

Install [Homebrew](http://brew.sh/) for easier installation of dependencies.
Install required dependencies:

``` console
brew install qt5 exiv2 opencv libraw quazip cmake pkg-config
```

Clone the nomacs repository from GitHub:

``` console
git clone https://github.com/nomacs/nomacs.git
```

Go to the `nomacs` directory and run cmake to get the Makefiles:

``` console
cd nomacs
mkdir build
cd build
```

For Homebrew on Intel models:

```console
Qt5_DIR=/usr/local/opt/qt5/ cmake -DQT_QMAKE_EXECUTABLE=/usr/local/opt/qt5/bin/qmake ../ImageLounge/.
```

For Homebrew on Apple Silicon models:

```console
Qt5_DIR=/opt/homebrew/opt/qt5/ cmake -DQT_QMAKE_EXECUTABLE=/opt/homebrew/opt/qt5/bin/qmake ../ImageLounge/.
```

Run make:

```console
make
```

You will now have a binary (`nomacs.app`), which you can test (or use directly). To install it to `/usr/local/bin`, use

```console
sudo make install
```

## Build nomacs (Windows Cross-Compile)

Compiles nomacs for Windows using M Cross Environment (MXE) from a Linux/Unix host. 

MXE environment is usually compiled from source, however you may be able to skip this if MXE has packages for your platform:

````bash
git clone <mxe url>
cd mxe

make MXE_TARGETS=`x86_64-w64-mingw32.shared` qtbase qtimageformats qtwinextras opencv quazip tiff exiv2 libraw
````

Setup build environment:

````bash
export MXE_DIR=~/mxe
export MXE_TARGET=x86_64-w64-mingw32.shared
export PATH="${MXE_DIR}/usr/bin:$PATH"

alias pkg-config=${MXE_TARGET}-pkg-config
alias qmake=${MXE_TARGET}-qmake
alias cmake=${MXE_TARGET}-cmake
````

Run cmake out-of-tree:

````bash
mkdir build-nomacs
cd build-nomacs

cmake -D QT_VERSION_MAJOR=5 -D ENABLE_TRANSLATIONS=ON -D ENABLE_HEIF=ON -D ENABLE_AVIF=ON  \
-DENABLE_JXL=ON -D USE_SYSTEM_QUAZIP=OFF <nomacs-path>/ImageLounge
````

Compile nomacs:

````bash
make -j8
make collect
````

Run on WINE:

````bash
wine nomacs-mingw/nomacs.exe
````

Run on Windows:

````console
cd C:\
xcopy /DEY <shared-folder>\build-nomacs\nomacs-mingw nomacs
cd nomacs
nomacs.exe
````

## Build nomacs (Windows with MSYS2)

```bash
export target=mingw-w64-x86_64

pacman -S $target-qt5-base $target-qt5-svg $target-qt5-winextras $target-qt5-tools \
          $target-libraw $target-libtiff $target-exiv2 $target-opencv $target-cmake $target-gcc

export PATH=/ming64:$PATH

cd <build-dir>

cmake  -D  QT_MAJOR_VERSION=5  -D ENABLE_TRANSLATIONS=ON -D  USE_SYSTEM_QUAZIP=ON <nomacs-dir>/ImageLounge

cmake --build . --parallel 8
```


## Build in Docker

We have created a docker image that best simulates the old travis system (currently it's ubuntu xenial 16.04). To build nomacs in a docker, you have to create the image:

````bash
docker build --rm -f "Dockerfile" -t nomacs:latest empty-docker-dir
`````

To deploy nomacs in a docker on your system, you can mount this directory using:

````bash
docker run --rm -it -v C:\\coding\\nomacs:/usr/nomacs nomacs:latest
````

If needed, you can upload the image:

````bash
docker login
docker tag nomacs diemmarkus/nomacs
docker push diemmarkus/nomacs:latest
````

## Links

- [nomacs.org](https://nomacs.org)
- [GitHub](https://github.com/nomacs)
- [Matrix chat room](https://matrix.to/#/#nomacs:matrix.org)

[![nomacs-icon](https://nomacs.org/nomacs.svg)](https://nomacs.org)
