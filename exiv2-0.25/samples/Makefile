# ************************************************************* -*- Makefile -*-
#
# Copyright (C) 2004-2015 Andreas Huggel <ahuggel@gmx.net>
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
# Version:   $Rev: 3861 $
# Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
# History:   08-Oct-07, ahu: created
#
# Description:
#  Simple Makefile to build sample programs. Requires installed exiv2 library
#  and headers. Adapted from the main Exiv2 src/Makefile.
#
# Restrictions:
#  Requires GNU make.
#

# Default make target
all: samples

# Include system configuration
top_srcdir = ..
include $(top_srcdir)/config/config.mk

# ******************************************************************************
# Source files

# Add source files of sample programs to this list
BINSRC = addmoddel.cpp        \
         conntest.cpp         \
         convert-test.cpp     \
         easyaccess-test.cpp  \
         exifcomment.cpp      \
         exifdata-test.cpp    \
         exifdata.cpp         \
         exifprint.cpp        \
         exifvalue.cpp        \
         httptest.cpp         \
         iotest.cpp           \
         iptceasy.cpp         \
         iptcprint.cpp        \
         iptctest.cpp         \
         key-test.cpp         \
         largeiptc-test.cpp   \
         mmap-test.cpp        \
         prevtest.cpp         \
         remotetest.cpp       \
         stringto-test.cpp    \
         taglist.cpp          \
         tiff-test.cpp        \
         werror-test.cpp      \
         write-test.cpp       \
         write2-test.cpp      \
         xmpparse.cpp         \
         xmpparser-test.cpp   \
         xmpsample.cpp

# Samples that need special treatment during compilation or linking
OTHERSRC = exiv2json.cpp      \
           geotag.cpp         \
           metacopy.cpp       \
           path-test.cpp

# ******************************************************************************
# Initialisations
SHELL = /bin/sh

PKGCONFIG=pkg-config
BAR='****************************************'

.SUFFIXES:
.SUFFIXES: .c .cpp .o .so

.PRECIOUS: %.cpp

CPPFLAGS := `pkg-config exiv2 --cflags`
LDFLAGS := `pkg-config exiv2 --libs`

BINOBJ = $(BINSRC:.cpp=.o)
BINARY = $(BINSRC:.cpp=)

OTHEROBJ = $(OTHERSRC:.cpp=.o) Jzon.o
OTHERBIN = $(OTHERSRC:.cpp=)

EXECUTABLE = $(BINSRC:%.cpp=../bin/%$(EXEEXT)) $(OTHERSRC:%.cpp=../bin/%$(EXEEXT))

ifdef DEP_TRACKING
DEP = $(BINSRC:%.cpp=$(DEPDIR)/%.d) $(OTHERSRC:%.cpp=$(DEPDIR)/%.d) $(DEPDIR)/Jzon.d
endif

PREFIX = /usr/local/lib
LINK.cc += -rpath $(PREFIX) -L$(PREFIX)

# TODO: This should really not be here. We should use the installed headers
# to build the samples. Some errors only show up in this scenario, e.g., errors
# due to missing headers which are not installed (depending on compile-time
# settings) but may be wrongly included from other headers.
COMPILE.cc += -I$(top_srcdir)/include/exiv2

# ******************************************************************************
# Rules
samples: $(BINARY) $(OTHERBIN)

$(BINOBJ): %.o: %.cpp
	@if [ -z `which $(PKGCONFIG)`  ]; then echo $(BAR) $'\\n***' utility $(PKGCONFIG) not available $'***\n'$(BAR) ; exit 42 ; fi
	$(COMPILE.cc) -o $@ $<
	@$(MAKEDEPEND)
	@$(POSTDEPEND)

$(BINARY): %: %.o
	mkdir -pv ../bin 2>&1 > /dev/null
	$(LIBTOOL) --mode=link $(LINK.cc) -o ../bin/$@ $@.o

%.o: %.cpp
	$(COMPILE.cc) -o $@ $<
	@$(MAKEDEPEND)
	@$(POSTDEPEND)

%.ii: %.cpp
	set -e; \
	$(CXXCPP) -E $(CPPFLAGS) $< | sed '/^[ 	]*$$/d' > $@

# ******************************************************************************
# Targets
.PHONY: all relink binclean mostlyclean clean distclean maintainer-clean

ifdef DEP_TRACKING
# Include targets from dependency files
-include $(DEP)
endif

relink: binclean samples

# link expat directly to geotag
geotag: %: %.cpp
	$(COMPILE.cc) -o $@.o $<
	@$(MAKEDEPEND)
	@$(POSTDEPEND)
	$(LIBTOOL) --mode=link $(LINK.cc) -lexpat $@.o -o ../bin/$@

# link applications which require utils.cpp support
metacopy path-test: %: %.cpp ../src/utils.o
	$(COMPILE.cc) -I../src -I../include/exiv2 -o $@.o $<
	@$(MAKEDEPEND)
	@$(POSTDEPEND)
	$(LIBTOOL) --mode=link $(LINK.cc) $@.o ../src/utils.o -o ../bin/$@

# exiv2json sample requires Jzon
exiv2json: %: %.cpp Jzon.o
	$(COMPILE.cc) -o $@.o $<
	@$(MAKEDEPEND)
	@$(POSTDEPEND)
	$(LIBTOOL) --mode=link $(LINK.cc) Jzon.o $@.o -o ../bin/$@

# Remove binaries, e.g., to relink them
binclean:
	$(RM) $(EXECUTABLE)

mostlyclean:
	$(RM) core
	$(RM) $(BINSRC:.cpp=.ii) $(OTHERSRC:.cpp=.ii) Jzon.ii
	$(RM) $(BINOBJ) $(OTHEROBJ)

clean: binclean mostlyclean

# Run `make distclean' from the top source directory to also remove
# files created by configuring the program.
distclean: clean
ifdef DEP_TRACKING
	$(RM) $(DEP)
	-rmdir $(DEPDIR)
endif
	$(RM) *~ *.bak *#

# This command is intended for maintainers to use; it deletes files
# that may need special tools to rebuild.
maintainer-clean: distclean
