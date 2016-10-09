# Nomacs Ocr Plugin
This plugin enables Nomacs to perform optical character recognition on a loaded image.

## building tesseract vs2015 ( visual studio 14 )
1) build dependencies
- git clone git://github.com/pvorb/tesseract-vs2013.git
- upgrade the project first to vs2015
- build
- create LeptonicaConfig.cmake
Insert:
set (Leptonica_INCLUDE_DIRS "E:/dev/tesseract-vs2013/liblept/include")
set (Leptonica_LIBRARIES "E:/dev/tesseract-vs2013/build/lib/Win32/liblept171.lib")
( ofc adapt your paths ^_^ )

2) build tesseract
- cmake tesseract from trunk
- set Leptonica_DIR to the created LeptonicaConfig.cmake file
- build

## troubleshooting
Loading the plugin fails if the dependent libraries aren't inside PATH. This just silently fails with an error message telling that the dll wasn't found, but actually some dependencies weren't found. 
Make sure that following dlls are present in PATH:
* liblept171.dll ( liblept171d.dll )
* tesseract305.dll ( tesseract305d.dll )

## development
Properties >> Debugging >> Environemnt
extends PATH with needed libraries for development ( below for debug config )
PATH=%PATH%;E:\dev\Qt5.6.0_msvc2015_x86\qtbase\lib;E:\dev\opencv\build_x86\bin\Debug;E:\dev\nomacs\exiv2-0.25\msvc2015-precompiled\Win32\DebugDLL;E:\dev\tesseract\build_x86\bin\Debug

## more info
above I also added Nomacs deps to PATH. usually that's not necessary but the .py script to collect and copy libraries fails for me. on the plus side: less discspace used ( success! )  