# Compile Qt 5 🍸

- here are the official qt5 compile instructions: http://qt-project.org/wiki/Building_Qt_5_from_Git
- The next steps only need to be performed if you compile from the repo
  - first get the source code from: https://git.gitorious.org/qt/qt5.git
  - python has to be in the system path
  - open a cmd and dir to the qt5 folder. Then type
    ```
    perl init-repository
    ```
  - if it is complaining about git, add your git/bin and git/cmd folders to the environment variables.
- open an msvc cmd (e.g. `VS2015 x64 Native Tools Command Prompt) and type:
```
configure.bat -debug-and-release -opensource -confirm-license -qt-zlib -qt-pcre -qt-freetype -opengl dynamic -qt-sql-sqlite -no-dbus -strip -plugin-sql-sqlite -make libs -nomake tools -nomake examples -nomake tests -platform win32-msvc2015
```
- NOTE: win32-msvc2015 has to be changed according to your toolchain
- NOTE: win32-msvc2015 does not change the bitness (the CMD sets the bitness)
- after it's finished run: ``nmake``
- now it's time to have a cup of coffee

## Clean Qt5 configuration
To get a really clean tree use:
- without git: ``nmake -distclean``
- with git: ``git submodule foreach --recursive "git clean -dfx"``

since make confclean no longer works from the top-level of the repo.
see: http://qt-project.org/wiki/Building_Qt_5_from_Git#0647f0776aed243f58bfec4dd81de863
