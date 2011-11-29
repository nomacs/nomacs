
CONFIG(debug,debug|release) {
	LIBS+=debug/libraw.lib
} else {
	LIBS+=release/libraw.lib
}
INCLUDEPATH=../
CONFIG-=qt
win32:CONFIG+=console
SOURCES=../samples/dcraw_half.c
CONFIG+=warn_off
