win32 {
    !contains(QMAKE_HOST.arch, x86_64) {
	SUFF="x86"
    } else {
        ## Windows x64 (64bit) specific build here
	SUFF="x86_64"
    }
}

CONFIG(debug,debug|release) {
	win32:OUTD=debug-$$SUFF
	macx:OUTD=debug
} else {
	win32:OUTD=release-$$SUFF
	macx:OUTD=release
}
INCLUDEPATH+=../../LibRaw
OBJECTS_DIR = $$OUTD/
MOC_DIR = $$OUTD/
RCC_DIR = $$OUTD/
UI_DIR = $$OUTD/
DESTDIR = $$OUTD/
LIBS+=-L$$OUTD
CONFIG+=warn_off

