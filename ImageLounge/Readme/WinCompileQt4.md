# Compile Qt 4 🍸

## Compile QT with Visual Studio

see also:
- http://thomasstockx.blogspot.com/2011/03/qt-472-in-visual-studio-2010.html
- http://www.holoborodko.com/pavel/2011/02/01/how-to-compile-qt-4-7-with-visual-studio-2010/

### Short version of the first link

- Install the Qt Visual Studio Add-In: http://qt.nokia.com/downloads/visual-studio-add-in
- Download qt-everywhere-opensource-src-4.x.x.zip from: http://qt-project.org/downloads
- Extract the zip
- Open a Visual Studio Command Prompt (either x86 or x64)
- dir to the qt dir
- Run
```
C:\Qt\qt-everywhere-opensource-src-4.8.5-x86-native-gestures>configure -debug-and-release -opensource -shared -no-qt3support -no-webkit -no-script -nomake examples -nomake demos -platform win32-msvc2012 -no-native-gestures
```
- Wait while Qt is getting configured for your platform.
- When done, run ``nmake`` to start compiling Qt (now it's time to take a cup of coffee/cigarette)
- rerun the nomacs cmake with the qt path

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
- Skip reading and press “y”.
