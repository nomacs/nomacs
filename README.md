# nomacs - Image Lounge 🍸

nomacs is a free, open source image viewer, which supports multiple platforms. You can use it for viewing all common image formats including RAW and psd images. nomacs is licensed under the GNU General Public License v3 and available for Windows, Linux, FreeBSD, Mac, and OS/2.

[![Build Status](https://travis-ci.org/nomacs/nomacs.svg?branch=master)](https://travis-ci.org/nomacs/nomacs)
[![Build status](https://ci.appveyor.com/api/projects/status/0lw27jchw3ymaqd4?svg=true)](https://ci.appveyor.com/project/diemmarkus/nomacs)
[![Downloads](https://img.shields.io/github/downloads/nomacs/nomacs/total.svg)](https://github.com/nomacs/nomacs/releases/latest)
[![Crowdin](https://badges.crowdin.net/nomacs/localized.svg)](http://translate.nomacs.org/project/nomacs)

- [Compiling Qt](./ImageLounge/manuals/Compiling-Qt.md)
- [Releasing](./ImageLounge/manuals/Releasing.md)

## Build nomacs (Windows)

### Compile dependencies

- [Qt](https://github.com/qt/qtbase) SDK or the compiled sources (>= 5.2.1)
- [expat](https://github.com/nomacs/expat) (needed for exiv2)
- [Exiv2](https://github.com/nomacs/exiv2) (>= 0.26)
- [OpenCV](https://github.com/TUWien/opencv) (>= 3.4) _optional_
- [LibRaw](https://github.com/nomacs/LibRaw) (>= 0.17) _optional_
- [QuaZip](https://github.com/nomacs/quazip) (>= 0.7.6) _optional_
- [Python](https://github.com/python/cpython) (>= 3.6) for build scripts

### Compile LibRaw

- only needed if ENABLE_RAW is checked
- clone [libraw](https://github.com/nomacs/LibRaw)
- checkout to the latest `0.XX-stable` branch
- follow the build instructions
- in the nomacs `CMakeUserPaths.cmake`, add the build path to `${CMAKE_PREFIX_PATH}`

### Compile nomacs

- copy `CMakeUserPathsGit.cmake` and rename it to `CMakeUserPaths.cmake`
- add your library paths to the `${CMAKE_PREFIX_PATH}` in `CMakeUserPaths.cmake`
- Open CMake GUI
- set your ImageLounge folder to `where is the source code`
- choose a build folder (i.e. `build2017-x64`)
- Hit `Configure`then `Generate`
- Open the Project
- Right-click the nomacs project and choose `Set as StartUp Project`
- Compile the Solution

### If anything did not work

- check if you have setup opencv (otherwise uncheck ENABLE_OPENCV)
- check if your Qt is set correctly (otherwise set the path to `qt_install_dir/qtbase/bin/qmake.exe`)
- check if your builds proceeded correctly

## Build nomacs (Ubuntu)

Get the required packages:

``` console
sudo apt-get install debhelper cdbs qt5-qmake qttools5-dev-tools qt5-default qttools5-dev libqt5svg5-dev qt5-image-formats-plugins libexiv2-dev libraw-dev libopencv-dev cmake libtiff-dev libquazip5-dev libwebp-dev git build-essential lcov libzip-dev
```

Get the nomacs sources from github:
``` console
git clone https://github.com/nomacs/nomacs.git
```

This will by default place the source into ~/nomacs
Go to the nomacs/ImageLounge directory and run `cmake` to get the Makefiles:
``` console
mkdir build
cd build
cmake -DUSE_SYSTEM_QUAZIP=ON ..
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
after a successfully install.

Install the [heif plugin](https://github.com/jakar/qt-heif-image-plugin) for HEIF support.

### For Package Maintainers

- Set `ENABLE_TRANSLATIONS` to `true` (default: `false`)
- Build all officially supported [plugins](https://github.com/nomacs/nomacs-plugins/)

## Build nomacs (MacOS)

Install [Homebrew](http://brew.sh/) for easier installation of dependencies.
Install required dependencies:

``` console
$ brew install qt5 exiv2 opencv libraw quazip cmake pkg-config
```

Go to the `nomacs` directory and run cmake to get the Makefiles:

``` console
$ mkdir build
$ cd build
$ Qt5_DIR=/usr/local/opt/qt5/ cmake -DQT_QMAKE_EXECUTABLE=/usr/local/opt/qt5/bin ../ImageLounge/.
```

Run make:

```console
$ make
```

You will now have a binary (`nomacs.app`), which you can test (or use directly). To install it to `/usr/local/bin`, use

```console
$ sudo make install
```

## Build in Docker
We have created a docker image that best simulates the travis system (currently it's ubuntu xenial 16.04). To build nomacs in a docker, you have to create the image:
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

[![nomacs-icon](https://nomacs.org/startpage/nomacs.svg)](https://nomacs.org)
