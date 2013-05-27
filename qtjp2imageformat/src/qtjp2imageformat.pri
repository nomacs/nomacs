INCLUDEPATH += $$PWD
HEADERS+= $$PWD/qtjp2imagehandler.h 
SOURCES+= $$PWD/qtjp2imagehandler.cpp 
unix: LIBS += -ljasper

win32 {
#
# Insert the path to JasPer (see README.TXT) below, for instance like this:
#
# JASPERINCLUDE = c:/JasPer/jasper-1.900.1/src/libjasper/include
# CONFIG(debug, debug|release): {
#    JASPERLIB = c:/JasPer/jasper-1.900.1/src/msvc/win32_debug
# } else {
#    JASPERLIB = c:/JasPer/jasper-1.900.1/src/msvc/win32_release
# }


JASPERINCLUDE = ../../jasper-1.900.1/src/libjasper/include
CONFIG(debug, debug|release): {
   JASPERLIB = ../../jasper-1.900.1/src/msvc/Win32_Debug
} else {
   JASPERLIB = ../../jasper-1.900.1/src/msvc/Win32_Release
}


!exists($$JASPERINCLUDE/jasper/jasper.h) {
    error("JasPer header files not found, please set JASPERINCLUDE in src/qtjp2imageformat.pri")
}
!exists($$JASPERLIB/libjasper.lib) {
    error("JasPer lib file not found, please set JASPERLIB in src/qtjp2imageformat.pri")
}

INCLUDEPATH +=$$JASPERINCLUDE
LIBS += -L$$JASPERLIB
LIBS += libjasper.lib

}
