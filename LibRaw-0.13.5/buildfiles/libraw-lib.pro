TEMPLATE=lib
TARGET=libraw
INCLUDEPATH+=../
HEADERS=../libraw/libraw.h \
	 ../libraw/libraw_alloc.h \
	../libraw/libraw_const.h \
	../libraw/libraw_datastream.h \
	../libraw/libraw_internal.h \
	../libraw/libraw_version.h \
	../internal/defines.h \
	../internal/var_defines.h \
	../internal/libraw_internal_funcs.h

CONFIG-=qt
CONFIG+=warn_off
DEFINES+=LIBRAW_BUILDLIB

SOURCES=../internal/dcraw_common.cpp \
	../internal/dcraw_fileio.cpp \
	../internal/demosaic_packs.cpp \
	../src/libraw_cxx.cpp \
	../src/libraw_c_api.cpp
