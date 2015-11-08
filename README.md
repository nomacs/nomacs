# nomacs
nomacs is a free image viewer for Windows, Linux, and OSX.

# Installation
Download and install [CMake](https://cmake.org/download/) which we will use for generating the project files on the different platforms.

## Get the code
Checkout the code from github:

```
$ mkdir workspace
$ cd workspace
$ git clone https://github.com/nomacs/nomacs.git
```

## Linux
On Linux, nomacs should be easy to compile (following tested on 64 bit Ubuntu 14.04, but should work the same on all Debian-based systems):

Get the required packages:

```
$ sudo apt-get install debhelper cdbs qt4-qmake libqt4-dev libexiv2-dev libraw-dev libopencv-dev cmake libtiff-dev libquazip-dev libwebp-dev git
```

Go to the `nomacs` directory and run cmake to get the Makefiles

```
$ mkdir build
$ cd build
$ cmake ../ImageLounge/.
```

Run make:

```
$ make
```

You will now have a binary (`build/nomacs`), which you can test (or use directly). To install it to `/usr/local/bin`, use

```
$ sudo make install
```


## Windows
You need the Qt SDK or the compiled sources (>= 4.7.0) to compile nomacs.

### Compile dependencies
#### OpenCV (*Only needed if raw images are enabled.*)
* Download [OpenCV](http://opencv.org/downloads.html)
* Compile OpenCV (>= 2.2.0) using cmake

#### LibRaw-0.16.0 (*Only needed if raw images are enabled*)

* Open `LibRaw-0.16.0/LibRaw-nomacs.sln` and convert projects if necessary.
* Build libraw (debug & release) (*don't care if some projects fail*)

#### exiv2-0.24
* Open `exiv2-0.24/msvc2012-nomacs/exiv2.sln` and convert projects if necessary
* Build all (DebugDLL & ReleaseDLL)

### Make project
cmake
* Load cmake gui
* Set your ImageLounge folder to (where is the source code)
* Set the output to nomacs/build (or whatever you like)
* Press configure and generate

If anything did not work:

* check if you have setup opencv (otherwise uncheck ENABLE_RAW)
* check if your Qt is set correctly (otherwise set the path to qt_install_dir/bin/qmake.exe
  * check if your builds proceeded correctly (see *Compile dependencies*)

### Compile nomacs
* Open `nomacs.sln` in your build directory (see *Make project*)
* Right-click the solution set single startup project to nomacs
* Build all

nomacs should start now.

If nomacs starts but no images can be loaded (check your qt_install_dir/plugin/imageformats).
If necessary copy this folder to your release/debug path


## OSX
Install [Homebrew](http://brew.sh/) for easier installation of depenedencies.

Install required dependencies:

```
$ brew install qt4 exiv2 homebrew/science/opencv libraw quazip

```

Go to the `nomacs` directory and run cmake to get the Makefiles

```
$ mkdir build
$ cd build
$ cmake ../ImageLounge/.
```

Run make:

```
$ make
```

You will now have a binary (`nomacs.app`), which you can test (or use directly). To install it to `/usr/local/bin`, use

```
$ sudo make install
```
