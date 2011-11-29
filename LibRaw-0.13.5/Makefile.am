# autoconf macros
ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS=subdir-objects

LIBRAW_SHLIB_VER = @LIBRAW_SHLIB_VERSION@
LIBRAW_RELEASE_VER = @LIBRAW_RELEASE_VERSION@

# Headers
nobase_include_HEADERS = 	libraw/libraw.h \
							libraw/libraw_alloc.h \
							libraw/libraw_const.h \
							libraw/libraw_datastream.h \
							libraw/libraw_internal.h \
							libraw/libraw_types.h \
							libraw/libraw_version.h

# Docs
doc_DATA =	COPYRIGHT \
			LICENSE.CDDL \
			LICENSE.LGPL \
			LICENSE.LibRaw.pdf \
			Changelog.txt

# pkg-config .pc files
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libraw.pc libraw_r.pc

# Libraries
lib_LIBRARIES = lib/libraw.a lib/libraw_r.a
lib_LTLIBRARIES = lib/libraw.la lib/libraw_r.la

lib_libraw_a_CPPFLAGS = -DLIBRAW_NOTHREADS -w
lib_libraw_a_SOURCES = internal/dcraw_common.cpp \
				 internal/dcraw_fileio.cpp \
				 internal/demosaic_packs.cpp \
				 src/libraw_cxx.cpp \
				 src/libraw_c_api.cpp

lib_libraw_r_a_CXXFLAGS = -pthread -w
lib_libraw_r_a_CFLAGS = -pthread -w
lib_libraw_r_a_SOURCES = $(lib_libraw_a_SOURCES)
lib_libraw_la_SOURCES = $(lib_libraw_a_SOURCES)
lib_libraw_r_la_SOURCES = $(lib_libraw_a_SOURCES)

lib_libraw_la_LDFLAGS = -no-undefined -version-info $(LIBRAW_SHLIB_VER)
lib_libraw_r_la_LDFLAGS = -no-undefined -version-info $(LIBRAW_SHLIB_VER)


# Sample binaries
if EXAMPLES
bin_PROGRAMS =	bin/raw-identify \
				bin/unprocessed_raw \
				bin/4channels \
				bin/simple_dcraw \
				bin/mem_image \
				bin/dcraw_half \
				bin/half_mt \
				bin/dcraw_emu
endif

bin_raw_identify_SOURCES = samples/raw-identify.cpp
bin_raw_identify_CPPFLAGS = $(lib_libraw_a_CPPFLAGS)
bin_raw_identify_LDADD = -Llib/  -lraw 

bin_unprocessed_raw_SOURCES = samples/unprocessed_raw.cpp
bin_unprocessed_raw_CPPFLAGS = $(lib_libraw_a_CPPFLAGS)
bin_unprocessed_raw_LDADD = -Llib/ -lraw

bin_4channels_SOURCES = samples/4channels.cpp
bin_4channels_CPPFLAGS = $(lib_libraw_a_CPPFLAGS)
bin_4channels_LDADD = -Llib/ -lraw 

bin_simple_dcraw_SOURCES = samples/simple_dcraw.cpp
bin_simple_dcraw_CPPFLAGS = $(lib_libraw_a_CPPFLAGS)
bin_simple_dcraw_LDADD = -Llib/ -lraw

bin_mem_image_SOURCES = samples/mem_image.cpp
bin_mem_image_CPPFLAGS = $(lib_libraw_a_CPPFLAGS)
bin_mem_image_LDADD = -Llib/ -lraw 

bin_dcraw_half_SOURCES = samples/dcraw_half.c
bin_dcraw_half_CPPFLAGS = $(lib_libraw_a_CPPFLAGS)
bin_dcraw_half_LDADD = -Llib/ -lraw 

bin_half_mt_SOURCES = samples/half_mt.c
bin_half_mt_CFLAGS = $(lib_libraw_r_a_CXXFLAGS)
bin_half_mt_LDADD = -Llib/ -lraw_r 

bin_dcraw_emu_SOURCES = samples/dcraw_emu.cpp
bin_dcraw_emu_CPPFLAGS = $(lib_libraw_a_CPPFLAGS)
bin_dcraw_emu_LDADD = -Llib/ -lraw 
