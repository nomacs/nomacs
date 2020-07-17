# nomacs - Plugins

This repository contains officially supported plugins for the image viewer [nomacs](https://github.com/nomacs/nomacs). You can find additional (unstable) plugins [here](https://github.com/diemmarkus/nomacs-plugins).

[![Build Status](https://travis-ci.org/nomacs/nomacs-plugins.svg?branch=master)](https://travis-ci.org/nomacs/nomacs-plugins)

## Building on Windows

### Prerequisites

- CMake
- git
- python
- [nomacs](https://github.com/nomacs/nomacs)
- [OpenCV](https://github.com/TUWien/opencv) (>= 3.4) _optional_
- `Qt` SDK or the compiled sources (>= 5.2.1)

## Building Plugins

- Clone [nomacs](https://github.com/nomacs/nomacs.git)
- cd to the repository, then:
```bash
git submodule init
git submodule update
```
- this will clone the plugins repository to ./ImageLounge/plugins
- follow the build instructions of nomacs
- This will build all the Plugins set to 'ON' in nomacs-plugins/CMakeLists.txt.


### Debug Plugins (Visual Studio)

Tip: Go to Your Plugin > Properties > Configuration Properties > Debugging and set the command to your nomacs path (e.g. C:/nomacs/build/Debug/nomacs.exe) to better debug your plugin

## Links

- [nomacs.org](http://nomacs.org)
- [Bugtracker](https://github.com/nomacs/nomacs-plugins/issues)
- [GitHub](https://github.com/nomacs)
