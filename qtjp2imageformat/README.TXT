Qt Solutions Component: JPEG 2000 Image Format

The JPEG 2000 Image Format provides a QImageIOHandler plugin for
reading and writing JPEG 2000 files.

This QImageIOHandler plugin adds support for the JPEG 2000 image
file format to Qt. Currently, it is only availabe as a dynamically
loaded plugin. We have plans to add support for statically linked
plugins later. JPEG 2000 is designed to produce better compression
and less quality loss than ordinary JPEG. QtJp2ImageHandler uses
the free, Open Source codec JasPer, which is an official reference
implementation of the JPEG 2000 Part-1 codec standard.

Notes:

The JPEG 2000 image format provides support for storing image data
in several color spaces (such as RGB or YCrBr). In addition
high-precision image data is supported. The Qt JPEG 2000 reader
supports loading such images, but the image data will be converted
to RGB(A) with 8 bits of precicion per component when stored in a
QImage.

The JasPer library can be downloaded from
http://www.ece.uvic.ca/~mdadams/jasper/. Packages exist for many
linux distributions. It can also be installed on Windows.

Windows build notes:

If you are on Windows, you need to change the JASPERINCLUDE and
JASPERLIB settings in src\qtjp2imageformat.pri to point to the
JasPer header files and the jasper library. Inside the JasPer
library package the header files are located at
src\libjasper\include\jasper. (Note that Qt JPEG 2000 #include's
the header files as "jasper\xxxx.h")

The library itself can be built with src\msvc\libjasper.dsw.
Before building the library it is neceasary to change the run-time
library settings to reflect the settings used when building your
Qt application. This is typically done by adding a switch like /MD
for release and /MDd for debug in the project options. This
setting is found in the Project Properties dialog, under "C/C++ ->
Code Generation -> Runtime Library". (See
http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vccore/html/_core_.2f.md.2c_2f.ml.2c_2f.mt.2c_2f.ld.asp
for a description of possible run-time library settings).

Note that if you only want to use JasPer for the Qt JPEG 2000
Solution, you do not need to build all the 5 jasper projects. Only
the "libjasper" project needs to be built. This also means that
you do not need to install the GLUT and/or IJG libraries, as the
JasPer documentation explains.

The library will be built as src\msvc\Win32_Debug\libjasper.lib or
src\msvc\Win32_Release\libjasper.lib, depending on if it is a
debug or release build.

Version history:

2.0: - Latest version ported to Qt 4.0

2.1: - QImageReader::imageFormat() now returns the correct format
     string.

2.2: - Various build and documentation issues fixed.
     - Updated to latest Jasper version: 1.900.1.

2.3: - Misc. minor fixes.
     - LGPL release.

2.4: - Build fix.

