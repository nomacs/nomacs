
CONFIG(debug,debug|release) {
	LIBS+=debug/libraw.lib
} else {
	LIBS+=release/libraw.lib
}
INCLUDEPATH=../
CONFIG-=qt
win32 {
	CONFIG+=console
	SOURCES=../samples/half_mt_win32.c
} else {
	SOURCES=../samples/half_mt.c
}
CONFIG+=warn_off
