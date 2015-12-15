TEMPLATE=lib
TARGET=libraw
INCLUDEPATH+=../
include (libraw-common-lib.pro)
# JPEG section (libjpeg is linked later)
DEFINES+=USE_JPEG

## RawSpeed section
DEFINES+=USE_RAWSPEED
INCLUDEPATH+=../../RawSpeed/
win32:INCLUDEPATH+=d:/Qt/local/include d:/Qt/local/include/libxml2
win32:LIBS+=-lrawspeed -Ld:/Qt/local/lib/$$SUFF -llibxml2 -ljpeg


HEADERS=../libraw/libraw.h \
	 ../libraw/libraw_alloc.h \
	../libraw/libraw_const.h \
	../libraw/libraw_datastream.h \
	../libraw/libraw_types.h \
	../libraw/libraw_internal.h \
	../libraw/libraw_version.h \
	../internal/defines.h \
	../internal/var_defines.h \
	../internal/libraw_internal_funcs.h

win32: {
PREPROCESS_FILES=../dcraw/dcraw.c
preprocess.name=dcraw.c preprocess
preprocess.input=PREPROCESS_FILES
preprocess.output+=../internal/dcraw_common_fake.cpp 
preprocess.commands=..\\win32pre.cmd
preprocess.CONFIG+= no_link
preprocess.clean=
preprocess.variable_out=SOURCES
QMAKE_EXTRA_COMPILERS+=preprocess
}

CONFIG-=qt
CONFIG+=warn_off
macx: CONFIG+= static x86 x86_64
macx: QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.5
DEFINES+=LIBRAW_BUILDLIB

SOURCES+= \
	 ../internal/dcraw_common.cpp \
	 ../internal/dcraw_fileio.cpp \
	../internal/demosaic_packs.cpp \
	../src/libraw_cxx.cpp \
	../src/libraw_datastream.cpp \
	../src/libraw_c_api.cpp

