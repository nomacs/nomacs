# nomacs - Image Lounge 🍸
nomacs is a free, open source image viewer, which supports multiple platforms. You can use it for viewing all common image formats including RAW and psd images. nomacs is licensed under the GNU General Public License v3 and available for Windows, Linux, FreeBSD, Mac, and OS/2.

[![Build Status](https://travis-ci.org/nomacs/nomacs.svg?branch=master)](https://travis-ci.org/nomacs/nomacs)

## Build nomacs (Windows)
### Compile dependencies
- `Qt` SDK or the compiled sources (>= 5.2.1)
- `Exiv2` (>= 0.25)
- `OpenCV` (>= 2.4.6) _optional_
- `LibRaw` (>= 0.17) _optional_
- `Python` (>= 3.6) for build scripts

### Compile LibRaw
- only needed if ENABLE_RAW is checked
- checkout https://github.com/nomacs/LibRaw
- switch to 0.XX-stable branch
- follow the build instructions
- in the nomacs CMake, set `libraw_DIR` to that build path

### Compile nomacs
1. Open CMake GUI
2. set your ImageLounge folder to `where is the source code`
3. choose a build folder
4. Set `QT_QMAKE_EXECUTABLE` by locating the qmake.exe
5. Set `OpenCV_DIR` to your OpenCV build folder
6. Hit `Configure`then `Generate`
7. Open the `nomacs.sln` which is in your new build directory
8. Right-click the nomacs project and choose `Set as StartUp Project`
9. Compile the Solution

### If anything did not work
- check if you have setup opencv (otherwise uncheck ENABLE_OPENCV)
- check if your Qt is set correctly (otherwise set the path to `qt_install_dir/qtbase/bin/qmake.exe`)
- check if your builds proceeded correctly

## Build nomacs (Ubuntu)

Get the required packages:

``` console
sudo apt-get install debhelper cdbs qt5-qmake qttools5-dev-tools qt5-default qttools5-dev libqt5svg5-dev qt5-image-formats-plugins libexiv2-dev libraw-dev libopencv-dev cmake libtiff-dev libquazip-dev libwebp-dev git build-essential lcov libzip-dev
```
Get the nomacs sources from github:
``` console
git clone https://github.com/nomacs/nomacs.git
```
This will by default place the source into ~/nomacs

Go to the nomacs/ImageLounge directory and run `cmake` to get the Makefiles:
``` console
cmake .
```

Compile nomacs:
``` console
make
```

You will now have a binary (~/nomacs/nomacs), which you can test (or use directly). To install it to /usr/local/bin, use:
``` console
sudo make install
```
note that you have to execute
``` console
sudo ldconfig
```
on your own after a successfully install

## Build nomacs (OSX)
Install [Homebrew](http://brew.sh/) for easier installation of depenedencies.

Install required dependencies:

```
$ brew install qt5 exiv2 opencv libraw quazip
```

Link `qmake` binary into `/usr/local/bin`:

```
$ ln -s /usr/local/opt/qt5/bin/qmake /usr/local/bin/
```

Go to the `nomacs` directory and run cmake to get the Makefiles

```
$ mkdir build
$ cd build
$ Qt5_DIR=/usr/local/opt/qt5/ cmake ../ImageLounge/.
```

Run make:

```
$ make
```

You will now have a binary (`nomacs.app`), which you can test (or use directly). To install it to `/usr/local/bin`, use

```
$ sudo make install
```

## Links
- [nomacs.org](http://nomacs.org)
- [Bugtracker](http://www.nomacs.org/redmine/projects/nomacs)
- [GitHub](https://github.com/nomacs)

[![nomacs-icon](http://nomacs.org/startpage/nomacs.svg)](https://nomacs.org)
