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
         convert-test.cpp     \
         easyaccess-test.cpp  \
         exifcomment.cpp      \
         exifdata-test.cpp    \
         exifprint.cpp        \
         iotest.cpp           \
         iptceasy.cpp         \
         iptcprint.cpp        \
         iptctest.cpp         \
         key-test.cpp         \
         largeiptc-test.cpp   \
         mmap-test.cpp        \
         prevtest.cpp         \
         stringto-test.cpp    \
         tiff-test.cpp        \
         werror-test.cpp      \
         write-test.cpp       \
         write2-test.cpp      \
         xmpparse.cpp         \
         xmpparser-test.cpp   \
         xmpsample.cpp

# ******************************************************************************
# Initialisations
SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .c .cpp .o .so

.PRECIOUS: %.cpp

CPPFLAGS := `pkg-config exiv2 --cflags`
LDFLAGS := `pkg-config exiv2 --libs`

BINOBJ = $(BINSRC:.cpp=.o)
BINARY = $(BINSRC:.cpp=)
EXECUTABLE = $(BINSRC:.cpp=$(EXEEXT))

ifdef DEP_TRACKING
DEP = $(BINSRC:%.cpp=$(DEPDIR)/%.d)
endif

# ******************************************************************************
# Rules
samples: $(BINARY)

$(BINOBJ): %.o: %.cpp
	$(COMPILE.cc) -o $@ $<
	@$(MAKEDEPEND)
	@$(POSTDEPEND)

$(BINARY): %: %.o
	$(LIBTOOL) --mode=link $(LINK.cc) -o $@ $@.o

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

# Remove binaries, e.g., to relink them
binclean:
	$(RM) $(EXECUTABLE)

mostlyclean:
	$(RM) core
	$(RM) $(CCSRC:.cpp=.ii)
	$(RM) $(CCSRC:%.cpp=.libs/%.d) $(CSRC:%.c=.libs/%.d)
	-rmdir .libs
	$(RM) $(BINOBJ)

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
