# Compiling Qt 🍸
If you just want to build nomacs - go for the latest installer [1]. However, if
you (like us) develop nomacs, compiling Qt on your machine has a few
advantages:
- Debugging (also Qt) is easy
- You can choose which packages you need
- New IDEs take some time until Qt officially supports them
So here is a short guide how we build Qt for nomacs.

## Compile Qt 5.xx on Windows
here are the official qt5 compile instructions [4]. First get the source code (google qt everywhere).

- ``python`` has to be in the system path
- open a `Native Tools Command Prompt` from Visuas Studio, dir to `qtbase` and type:
```bat
REM Visual Studio 2015
configure.bat -debug-and-release -opensource -confirm-license -qt-zlib -qt-pcre -qt-freetype -opengl dynamic -no-dbus -strip -plugin-sql-sqlite -make libs -nomake tools -nomake examples -nomake tests -platform win32-msvc2015 -prefix build
```
```bat
REM Visual Studio 2017
configure.bat -debug-and-release -opensource -confirm-license -opengl dynamic -no-dbus -nomake tools -nomake examples -nomake tests -skip qtwebengine  -platform win32-msvc -prefix build
```
```bat
REM Visual Studio 2019
REM replace -prefix build with i.e. -prefix C:/Qt/Qt5.13.0-x64
configure.bat -debug-and-release -opensource -confirm-license -opengl dynamic -no-dbus -nomake examples -nomake tests -skip qtwebengine -platform win32-msvc -prefix build
```
- after it's finished run ``nmake``
- now it's time to have a cup of coffee
- having compiled and a cup of coffee type ``nmake install``

### Shadow build

The build directory is where the build-related files such as Makefiles, object files, and other intermediate files are stored. The install directory is where the binaries and libraries are installed, for use either by the system or by the application.

It is recommended to keep these directories separate by shadow-building and using the -prefix option. This enables you to keep the Qt source tree clean from the build artifacts and binaries, which are stored in a separate directory. This method is very convenient if you want to have multiple builds from the same source tree, but for different configurations. To shadow-build, run configure from a separate directory

see https://doc.qt.io/qt-5/configure-options.html#

### Build Modules (Qt 5.13)
As of Qt 5.13 we build modules using Qt creator. Therefore you need to open the `*.pro` project files in Qt Creator. Set up the environment to match your build system. Currently we use the vs 2019 compiler & debugger together with the newly built Qt version (see above). Be sure to update the build path of your Release configuration to your Qt install dir (i.e. `C:/Qt/Qt5.13.0-x64`). For nomacs we need these modules:
- qttools
- qtsvg
- qtimageformats
- qtwinextras

### Clean Qt5 configuration
To get a really clean tree, use:
- without git: ``nmake -distclean``

### Qt > 5.9 and translations
if you use Qt > 5.9 it is possible, that Visual Studio is unable to compile the translations and thus erroring. Check if you can execute the lrelease.exe in [QtPath]/qttools/bin . 
It may be, that Qt5Core.dll and Qt5Xml.dll is needed, than copy these file in the directory and everything should run fine

### Build Image Formats
Some image formats like jp2 are not maintained and therefore not built with Qt anymore. Therefore we need to manually build them
- Install `Qt Creator`
- Open `QT_DIR\qtimageformats\src\plugins\imageformats\jp2\jp2.pro` with QtCreator
- Hit `Build All` (a few warnings might appear)
- The dll's are now in QT_DIR\qtimageformats\plugins\imageformats so running CMake again will copy them to nomacs
- We add JP2 and MNG

## Qt 4.xx
Compile QT with Visual Studio, see also [2], [3].

Short version of the first link:
- Install the Qt Visual Studio Add-In: http://qt.nokia.com/downloads/visual-studio-add-in
- Download qt-everywhere-opensource-src-4.x.x.zip from: http://qt-project.org/downloads
- extract the `zip`
- Open a Visual Studio Command Prompt (either x86 or x64).
- dir to the Qt dir
- Run the following command:
````
C:\Qt\qt-everywhere-opensource-src-4.8.5-x86-native-gestures>configure -debug-and-release -opensource -shared -no-qt3support -no-webkit -no-script -nomake examples -nomake demos -platform win32-msvc2012 -no-native-gestures
````

### Explanation
- ``-opensource``: install open source edition of Qt.
- ``-shared``: makes the Qt libraries shared (dll).
- ``-no-qt3support``: sorry my retro friends, don’t like old code.
- ``-qt-sql-sqlite``: enable support for the sqlite driver.
- ``-phonon``: compile the Phonon module (some multimedia thingy of Qt).
- ``-phonon-backend``: compile the platform-specific Phonon backend-plugin.
- ``-no-webkit``: disable WebKit module (heard of some bugs in combination with VS, just to be safe and since I don’t need it).
- ``-no-script``: same as no-webkit.
- ``-platform win32-msvc2012``: build for VS 2012!
- ``-no-native-gestures``: nomacs does not support qt gestures (they are a mess in Qt 4.xx) this command allows us to directly grab windows gestures which are way cleaner


- Skip reading and press ``y``.
- Wait while Qt is getting configured for your platform.
- When done, run ``nmake`` to start compiling Qt (now it's time to take a cup of coffee/cigarette)
- rerun the nomacs ``CMake`` with the qt path

## Links
[1] https://www.qt.io/download/  
[2] http://thomasstockx.blogspot.com/2011/03/qt-472-in-visual-studio-2010.html  
[3] http://www.holoborodko.com/pavel/2011/02/01/how-to-compile-qt-4-7-with-visual-studio-2010/  
[4] http://qt-project.org/wiki/Building_Qt_5_from_Git  
[6] https://codereview.qt-project.org/#/c/177743/7/src/corelib/tools/qalgorithms.h  
