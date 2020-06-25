@echo off
REM build nomacs
REM make-nomacs "C:/Qt/Qt-5.14.1-installer/5.14.2/msvc2017_64/bin" debug "C:/coding/nomacs/nomacs/3rd-party/build"
REM note to myself: it is already complex enough that we should script it in python

set my_path=%~dp0
set qt5=%1
set release="%2"
set dependency_path=%3

if NOT "%3" == "" (
    goto :custom_3rdparty_path
) 

REM default to repo build
set dependency_path=%my_path%3rd-party/build

:custom_3rdparty_path

REM fix windows backslashed paths : /
set my_path=%my_path:\=/%
set qt5=%qt5:\=/%

set name=nomacs
set build_dir=%my_path%build/%name%

echo Qt path: %qt5%

cmake^
 -DCMAKE_PREFIX_PATH="%qt5%;"^
 -DDEPENDENCY_PATH=%dependency_path%^
 -DENABLE_TRANSLATIONS=ON^
 -DENABLE_HEIF=ON^
 -DENABLE_AVIF=ON^
 -B%build_dir% %my_path%/ImageLounge

cmake --build %build_dir% --config Release -- -m

if not %RELEASE% == "release" (
    echo building %name% - [Debug]
    cmake --build %build_dir% --config Debug -- -m
)

cmake --build %build_dir% --config Release --target INSTALL -- -m
