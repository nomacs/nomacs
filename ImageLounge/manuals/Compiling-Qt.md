# Compiling Qt 🍸
If you just want to build nomacs - go for the latest installer [1]. However, if
you (like us) develop nomacs, compiling Qt on your machine has a few
advantages:
- Debugging (also Qt) is easy
- You can choose which packages you need
- New IDEs take some time until Qt officially supports them
So here is a short guide how we build Qt for nomacs.

## Compile Qt 5.xx on Windows
here are the official qt5 compile instructions [4]. First get the source code from [5].

- ``python`` has to be in the system path
- open a ``cmd`` and dir to the ``Qt5`` folder. Type:
````
perl init-repository
````
- if it is complaining about git, add your ``git/bin`` and ``git/cmd`` folders to the environment variables.
- open a `Native Tools Command Prompt` from Visuas Studio, dir to `qtbase` and type:
```bat
REM Visual Studio 2015
configure.bat -debug-and-release -opensource -confirm-license -qt-zlib -qt-pcre -qt-freetype -opengl dynamic -qt-sql-sqlite -no-dbus -strip -plugin-sql-sqlite -make libs -nomake tools -nomake examples -nomake tests -platform win32-msvc2015
```
```bat
REM Visual Studio 2017
configure.bat -debug-and-release -opensource -confirm-license -opengl dynamic -no-dbus -nomake tools -nomake examples -nomake tests -skip qtwebengine  -platform win32-msvc```


- after it's finished run ``nmake``
- now it's time to have a cup of coffee

### Clean Qt5 configuration
To get a really clean tree, use:

- without git: ``nmake -distclean``
- with git:
````
git submodule foreach --recursive "git clean -dfx"
````
since ``make confclean`` no longer works from the top-level of the repo.
further reading: http://qt-project.org/wiki/Building_Qt_5_from_Git#0647f0776aed243f58bfec4dd81de863

If today is March 2017 and you try to compile Qt 5.8 with Visual Studio 2017
you might need to patch the qalgorithms.h file, see [6].

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
[5] https://git.gitorious.org/qt/qt5.git
[6] https://codereview.qt-project.org/#/c/177743/7/src/corelib/tools/qalgorithms.h
