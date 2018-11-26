# nomacs - Plugins

This repository contains officially supported plugins for the image viewer [nomacs](https://github.com/nomacs/nomacs). You can find additional (unstable) plugins [here](https://github.com/diemmarkus/nomacs-plugins).

[![Build Status](https://travis-ci.org/nomacs/nomacs-plugins.svg?branch=master)](https://travis-ci.org/nomacs/nomacs-plugins)

## Building on Windows

### Prerequisites

- CMake
- IDE (i.e. Visual Studio)
- [nomacs](https://github.com/nomacs/nomacs)
- [OpenCV](https://github.com/TUWien/opencv) (>= 3.4) _optional_
- `Qt` SDK or the compiled sources (>= 5.2.1)

### Building Plugins (Windows)

1. Build [nomacs](https://github.com/nomacs/nomacs)
2. Open CMake GUI
3. set the plugin repository's root folder containing CMakeLists.txt to `where is the source code`
4. choose a build folder
5. Set OpenCV_DIR, QT_MAKE_EXECUTABLE, and nomacs_DIR
    - Copy `CMakeUserPathsGit.cmake` and rename it to `CMakeUserPaths.cmake`
    - Set all paths to your own builds of nomacs etc.
6. Hit `Configure` then `Generate`
7. You will find a nomacs-plugins.sln which builds all plugins and dedicated plugin projects in the respective folders (e.g. PaintPlugin/paintPlugin.sln)
8. Tip: Go to Plugin > Properties > Configuration Properties > Debugging and set the Command to your nomacs path (e.g. C:/nomacs/build2015-x64/Debug/nomacs.exe) to better debug your plugin

## Build Plugins (Ubuntu)

- Clone [nomacs](https://github.com/nomacs/nomacs.git)
- cd to `nomacs/ImageLounge`
- clone this [repository](https://github.com/nomacs/nomacs-plugins.git) into `plugins`:
````terminal
git clone https://github.com/nomacs/nomacs-plugins.git plugins
````
- in `ImageLounge` create a folder `build` and run cmake:
````
mkdir build
cd build
cmake -DENABLE_PLUGINS=ON .
````
- then build the project:
````terminal
make
````

This will build all the Plugins set to 'ON' in nomacs-plugins/CMakeLists.txt. Running ./nomacs from the nomacs/ImageLounge directory should now list all the built Plugins in the Plugins tab.


## Links

- [nomacs.org](http://nomacs.org)
- [Bugtracker](https://github.com/nomacs/nomacs-plugins/issues)
- [GitHub](https://github.com/nomacs)
