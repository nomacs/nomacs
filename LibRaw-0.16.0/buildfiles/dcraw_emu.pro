include (libraw-common.pro)
win32:LIBS+=libraw.lib
unix:LIBS+=-lraw
SOURCES=../samples/dcraw_emu.cpp
