# nomacs - Plugins

This repository contains officially supported plugins for the image viewer [nomacs](https://github.com/nomacs/nomacs).

[![Build Status](https://travis-ci.org/nomacs/nomacs-plugins.svg?branch=master)](https://travis-ci.org/nomacs/nomacs-plugins)

## Building on Windows

### Prerequisites

- CMake
- IDE (i.e. Visual Studio)
- nomacs
- OpenCV
- Qt

### Building Plugins (Windows)

1. Build [nomacs](https://github.com/nomacs/nomacs)
2. Open CMake GUI
3. set the plugin repository's root folder containing CMakeLists.txt to `where is the source code`
4. choose a build folder
5. Set OpenCV_DIR, QT_MAKE_EXECUTABLE, and nomacs_DIR
    - Copy `CMakeUserSkel.cmake` and rename it to `CMakeUser.cmake`
    - Set all paths to your own builds of nomacs etc.
6. Hit `Configure` then `Generate`
7. You will find a nomacs-plugins.sln which builds all plugins and dedicated plugin projects in the respective folders (e.g. PaintPlugin/paintPlugin.sln)
8. Tip: Go to Plugin > Properties > Configuration Properties > Debugging and set the Command to your nomacs path (e.g. C:/nomacs/build2015-x64/Debug/nomacs.exe) to better debug your plugin

## Build nomacs (Ubuntu)

Get the required packages:

``` console
sudo apt-get install debhelper cdbs qt5-qmake qttools5-dev-tools qt5-default qttools5-dev libqt5svg5-dev qt5-image-formats-plugins libexiv2-dev libraw-dev libopencv-dev cmake libtiff-dev libquazip-dev libwebp-dev git build-essential
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

## Links

- [nomacs.org](http://nomacs.org)
- [Bugtracker](https://github.com/nomacs/nomacs-plugins/issues)
- [GitHub](https://github.com/nomacs)
