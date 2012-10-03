# ************************************************************* -*- Makefile -*-
#
# Copyright (C) 2004-2012 Andreas Huggel <ahuggel@gmx.net>
#
# This Makefile is part of the Exiv2 distribution.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#    1. Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#    2. Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#    3. The name of the author may not be used to endorse or promote
#       products derived from this software without specific prior
#       written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# File:      Makefile
# Version:   $Rev: 2681 $
# Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
# History:   10-Dec-03, ahu: created
#
# Description:
#  Do NOT change this file! All system specific settings and configs
#  go into config.mk.
#
#  This makefile contains (supposedly) generic build rules to build the
#  library and applications. It includes all system specific settings
#  from config.mk. The idea is that configuring and porting the
#  software to a new platform should only require changes in config.mk.
#
# Restrictions:
#  Requires GNU make.
#

# Default make target
all: bin

# Include system configuration
top_srcdir = ..
include $(top_srcdir)/config/config.mk

# ******************************************************************************
# Source files

# Add standalone C++ header files to this list
CCHDR =  exiv2.hpp             \
         exv_conf.h            \
         exv_msvc.h            \
         version.hpp

# Add library C++ source files to this list
CCSRC =  basicio.cpp           \
	 bmpimage.cpp          \
	 canonmn.cpp           \
	 convert.cpp           \
	 cr2image.cpp          \
	 crwimage.cpp          \
	 datasets.cpp          \
	 easyaccess.cpp        \
	 epsimage.cpp          \
	 error.cpp             \
	 exif.cpp              \
	 futils.cpp            \
	 fujimn.cpp            \
	 gifimage.cpp          \
	 image.cpp             \
	 iptc.cpp              \
	 jp2image.cpp          \
	 jpgimage.cpp          \
	 makernote.cpp         \
	 metadatum.cpp         \
	 minoltamn.cpp         \
	 mrwimage.cpp          \
	 nikonmn.cpp           \
	 olympusmn.cpp         \
	 orfimage.cpp          \
	 panasonicmn.cpp       \
	 pgfimage.cpp
ifdef HAVE_LIBZ
CCSRC += pngimage.cpp          \
	 pngchunk.cpp
endif
CCSRC += preview.cpp           \
	 properties.cpp        \
	 psdimage.cpp          \
	 rafimage.cpp          \
	 rw2image.cpp          \
	 samsungmn.cpp         \
	 sigmamn.cpp           \
	 pentaxmn.cpp          \
	 sonymn.cpp            \
	 tags.cpp              \
	 tgaimage.cpp          \
	 tiffcomposite.cpp     \
	 tiffimage.cpp         \
	 tiffvisitor.cpp       \
	 types.cpp             \
	 value.cpp             \
	 version.cpp           \
	 xmp.cpp               \
	 xmpsidecar.cpp

# Add library C source files to this list
ifndef HAVE_TIMEGM
CSRC =   localtime.c
endif

# Add source files of simple applications to this list
# Todo: How can we still support crwedit.cpp, crwparse.cpp
BINSRC = taglist.cpp

# Source files for the Exiv2 application
EXIV2MAIN = exiv2.cpp
EXIV2SRC  = actions.cpp       \
	    utils.cpp
# C source files for the Exiv2 application
ifndef HAVE_TIMEGM
EXIVCSRC  = localtime.c
endif

# Source files for the metacopy sample application
MCMAIN = metacopy.cpp
MCSRC  = utils.cpp

# ******************************************************************************
# Library
LIBRARY = libexiv2.la

# ******************************************************************************
# Initialisations
SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .c .cpp .o .so

.PRECIOUS: %.cpp

CPPFLAGS += $(XMPSDK_CPPFLAGS)
LDFLAGS += $(EXPAT_LDFLAGS) $(XMPSDK_LDFLAGS)
LIBS += $(EXPAT_LIBS) $(XMPSDK_LIBS)

# Generic variables
CCHDR := $(CCHDR) $(CCSRC:.cpp=.hpp)
CCOBJ = $(CCSRC:.cpp=.o)
CCLOBJ = $(CCSRC:.cpp=.lo)

COBJ = $(CSRC:.c=.o)
CLOBJ = $(CSRC:.c=.lo)

SRC = $(CCSRC) $(CSRC)
HDR = $(CCHDR)
OBJ = $(CCOBJ) $(COBJ)
LOBJ = $(CCLOBJ) $(CLOBJ)

BINOBJ = $(BINSRC:.cpp=.o)
BINARY = $(BINSRC:.cpp=)
EXECUTABLE = $(BINSRC:.cpp=$(EXEEXT))

EXIV2OBJ = $(EXIV2MAIN:.cpp=.o) $(EXIV2SRC:.cpp=.o)
EXIV2COBJ = $(EXIVCSRC:.c=.o)
EXIV2BIN = $(EXIV2MAIN:.cpp=)
EXIV2EXE = $(EXIV2MAIN:.cpp=$(EXEEXT))

MCOBJ = $(MCMAIN:.cpp=.o) $(MCSRC:.cpp=.o)
MCBIN = $(MCMAIN:.cpp=)
MCEXE = $(MCMAIN:.cpp=$(EXEEXT))

ifdef DEP_TRACKING
DEP = $(CCSRC:%.cpp=$(DEPDIR)/%.d) $(CSRC:%.c=$(DEPDIR)/%.d) \
      $(BINSRC:%.cpp=$(DEPDIR)/%.d) \
      $(EXIV2MAIN:%.cpp=$(DEPDIR)/%.d) $(EXIV2SRC:%.cpp=$(DEPDIR)/%.d) \
      $(EXIVCSRC:%.c=$(DEPDIR)/%.d) $(MCMAIN:%.cpp=$(DEPDIR)/%.d) \
      $(MCSRC:%.cpp=$(DEPDIR)/%.d) $(DEPDIR)/path-test.d

endif

# ******************************************************************************
# Rules
$(CCOBJ): %.o: %.cpp
	@$(LIBTOOL) --mode=compile $(COMPILE.cc) -DEXV_BUILDING_LIB=1 -o $@ $<
	@$(MAKEDEPEND)
	@$(POSTDEPEND)

$(COBJ): %.o: %.c
	@$(LIBTOOL) --mode=compile $(COMPILE.c) -DEXV_BUILDING_LIB=1 -o $@ $<
	@$(MAKEDEPEND)
	@$(POSTDEPEND)

$(sort $(BINOBJ) $(EXIV2OBJ) $(MCOBJ) path-test.o): %.o: %.cpp
	$(COMPILE.cc) -o $@ $<
	@$(MAKEDEPEND)
	@$(POSTDEPEND)

%.o: %.c
	$(COMPILE.c) -o $@ $<
	@$(MAKEDEPEND)
	@$(POSTDEPEND)

%.ii: %.cpp
	set -e; \
	$(CXXCPP) $(CPPFLAGS) -DEXV_BUILDING_LIB=1 $< | sed '/^[ 	]*$$/d' > $@

# ******************************************************************************
# Targets
.PHONY: all bin check ctags doc                                        \
        clean mostlyclean distclean maintainer-clean                   \
        install install-header install-lib                             \
        uninstall uninstall-header uninstall-lib

ifdef DEP_TRACKING
# Include targets from dependency files
-include $(DEP)
endif

actions.cpp basicio.cpp exif.cpp exiv2.cpp futils.cpp image.cpp jpgimage.cpp utils.cpp: exv_conf.h

exv_conf.h: $(top_srcdir)/config/config.h
	sed 's/#define \([A-Z]\)/#define EXV_\1/; s/#undef \([A-Z]\)/#undef EXV_\1/' < $< > $@

$(LIBTOOL): $(LIBTOOL_DEPS)
	$(SHELL) $(top_srcdir)/config.status --recheck

bin: lib $(BINARY) $(EXIV2BIN) $(MCBIN) path-test

lib: $(OBJ)
	$(LIBTOOL) --mode=link $(LINK.cc) -o $(LIBRARY) $(LOBJ) -rpath $(libdir) -version-info $(EXIV2_LTVERSION) $(LIBS)
	@touch lib

path-test: path-test.o utils.o
	$(CXX) $(CXXFLAGS) path-test.o utils.o -o $@

$(BINARY): %: %.o lib
	@$(LIBTOOL) --mode=link $(LINK.cc) -o $@ $(LIBRARY) $@.o -rpath $(libdir)

$(EXIV2BIN): lib $(EXIV2OBJ) $(EXIV2COBJ)
	@$(LIBTOOL) --mode=link $(LINK.cc) -o $@ $(LIBRARY) $(EXIV2OBJ) $(EXIV2COBJ) -rpath $(libdir)

$(MCBIN): lib $(MCOBJ)
	@$(LIBTOOL) --mode=link $(LINK.cc) -o $@ $(LIBRARY) $(MCOBJ) -rpath $(libdir)

install-header:
	$(INSTALL_DIRS) $(DESTDIR)$(incdir)
	@list='$(HDR)'; for p in $$list; do \
	    if test -f $$p; then \
	        echo "$(INSTALL_DATA) $$p $(DESTDIR)$(incdir)/$$p"; \
	        $(INSTALL_DATA) $$p $(DESTDIR)$(incdir)/$$p; \
	    else :; fi; \
	done

install-lib: lib install-header
	$(INSTALL_DIRS) $(DESTDIR)$(libdir)
	@$(LIBTOOL) --mode=install $(INSTALL_DATA) $(LIBRARY) $(DESTDIR)$(libdir)/$(LIBRARY)
	$(INSTALL_DIRS) $(DESTDIR)$(bindir)
	$(INSTALL_DIRS) $(DESTDIR)$(libdir)/pkgconfig
	$(INSTALL_DATA) $(top_srcdir)/config/exiv2.pc $(DESTDIR)$(libdir)/pkgconfig/exiv2.pc

install: $(EXIV2BIN) install-lib
	$(INSTALL_DIRS) $(DESTDIR)$(bindir)
	@$(LIBTOOL) --mode=install $(INSTALL_PROGRAM) $(EXIV2EXE) $(DESTDIR)$(bindir)/$(EXIV2EXE)
	$(INSTALL_DIRS) $(DESTDIR)$(mandir)
	$(INSTALL_DIRS) $(DESTDIR)$(man1dir)
	$(INSTALL_DATA) exiv2.1 $(DESTDIR)$(man1dir)/exiv2.1

uninstall-header:
	@list='$(HDR)'; for p in $$list; do \
	    echo "$(RM) $(DESTDIR)$(incdir)/$$p"; \
	    $(RM) $(DESTDIR)$(incdir)/$$p; \
	done
	-rmdir $(DESTDIR)$(incdir)

uninstall-lib: uninstall-header
	$(RM) $(DESTDIR)$(libdir)/pkgconfig/exiv2.pc
	-rmdir $(DESTDIR)$(libdir)/pkgconfig
	@$(LIBTOOL) --mode=uninstall $(RM) $(DESTDIR)$(libdir)/$(LIBRARY)
	-rmdir $(DESTDIR)$(libdir)

uninstall: uninstall-lib
	$(RM) $(DESTDIR)$(man1dir)/exiv2.1
	-rmdir $(DESTDIR)$(man1dir)
	-rmdir $(DESTDIR)$(mandir)
	@$(LIBTOOL) --mode=uninstall $(RM) $(DESTDIR)$(bindir)/$(EXIV2EXE)
	-rmdir $(DESTDIR)$(bindir)

ctags:
	ebrowse $(HDR) $(SRC)
#	ctags-exuberant --extra=+q -e *
#	ctags-exuberant --extra=+q *

check:
	@echo "No checks available for this library."

mostlyclean:
	$(RM) core
	$(RM) $(CCSRC:.cpp=.ii)
	$(RM) lib
	$(RM) path-test.o
	$(RM) $(CCSRC:%.cpp=.libs/%.d) $(CSRC:%.c=.libs/%.d)
	@$(LIBTOOL) --mode=clean $(RM) $(LOBJ) $(sort $(BINOBJ) $(EXIV2OBJ) $(EXIV2COBJ) $(MCOBJ))

clean: mostlyclean
	@$(LIBTOOL) --mode=clean $(RM) $(LIBRARY)
	@$(LIBTOOL) --mode=clean $(RM) $(EXECUTABLE) $(EXIV2EXE) $(MCEXE)
	@$(LIBTOOL) --mode=clean $(RM) path-test$(EXEEXT)

# Run `make distclean' from the top source directory to also remove
# files created by configuring the program.
distclean: clean
	$(RM) exv_conf.h doxygen.hpp
ifdef DEP_TRACKING
	$(RM) $(DEP)
	-rmdir $(DEPDIR)
endif
	$(RM) tags TAGS
	$(RM) *~ *.bak *#

# This command is intended for maintainers to use; it deletes files
# that may need special tools to rebuild.
maintainer-clean: uninstall distclean
	-$(RM) -r .deps
	-$(RM) -r .libs
