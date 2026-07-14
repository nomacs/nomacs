1) Required software needed to compile nomacs dependencies:
Visual Studio Community
Qt (same version as used on AppVeyor)
MSYS2 ( https://www.msys2.org/ )
Meson ( https://github.com/mesonbuild/meson/releases/download/1.11.0/meson-1.11.0-64.msi )
nasm ( https://github.com/novomesk/downloads/raw/main/nasm-2.16.03-win64.zip )
Python ( https://www.python.org/downloads/windows/ )
Git for Windows

2) copy content of this folder into C:\projects\nomacs-dependencies\ folder

3) Prepare empty folder C:/projects/nomacs/3rd-party where the *.h *.dll *.lib will be installed.

4) Run 'x64 Native Tools Command Prompt for VS 2022'
Make sure that the tools (meson.exe, ninja.exe, nasm.exe, python.exe, ...) can be executed (PATH variable set to their folders)

5) cd C:\projects\nomacs-dependencies

6) Set envirnment variables inside Native Tools Command Prompt

Important! Use / as folder separator for NOMACS_DEPENDENCIES variable
SET NOMACS_DEPENDENCIES=C:/projects/nomacs/3rd-party

Use same Qt version for dependencies and for the nomacs itself
SET QT6=C:\Qt\6.11.1\msvc2022_64

Adjust path for Python for the location where you installed it:
SET PYTHON3_EXECUTABLE="C:\Program Files\Python313\python.exe"

7) run download.bat

8) run make-all-dependencies.bat

if all builds were successful,
9) Create archive with dependencies:

cd C:\projects\nomacs\3rd-party
7z a -r -mx9 ..\nomacs-dependencies-2026-07-03 *


if you want to try to build nomacs locally, inside this Command Prompt (assuming nomacs sources are in c:\projects\nomacs):

cd C:\projects\nomacs
mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_PREFIX_PATH=%QT6%;%NOMACS_DEPENDENCIES% -DCMAKE_INSTALL_PREFIX=C:/projects/nomacs/installer/nomacs.x64 -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DOpenCV_DIR=%NOMACS_DEPENDENCIES%/lib -DENABLE_QUAZIP=ON ../ImageLounge
nmake
nmake install
cd C:\projects\nomacs\installer\nomacs.x64
nomacs.exe

Test images from following repository:
https://github.com/nomacs/formats_testset
