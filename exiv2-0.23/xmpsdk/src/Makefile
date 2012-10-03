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
# History:   30-Sep-07, ahu: created
#
# Description:
#  Makefile to build the Adobe XMP Toolkit integrated and distributed with
#  Exiv2. Adapted from the main Exiv2 src/Makefile. This builds what the
#  libtool manual calls a "convenience library".
#
# Restrictions:
#  Requires GNU make.
#

# Default make target
all: xmpsdk

# Include system configuration
top_srcdir = ../..
include $(top_srcdir)/config/config.mk

# ******************************************************************************
# Source files

# Add library C++ source files to this list
CCSRC = XML_Node.cpp           \
        XMPMeta.cpp            \
        XMPMeta-GetSet.cpp     \
        XMPMeta-Parse.cpp      \
        XMPMeta-Serialize.cpp  \
        XMPIterator.cpp        \
        XMPUtils.cpp           \
        XMPUtils-FileInfo.cpp  \
        XMPCore_Impl.cpp       \
        ExpatAdapter.cpp       \
        ParseRDF.cpp           \
        UnicodeConversions.cpp \
        MD5.cpp                \
        WXMPMeta.cpp           \
        WXMPIterator.cpp       \
        WXMPUtils.cpp

# ******************************************************************************
# Library
LIBRARY := lib$(XMPSDK_LIBRARY).la

# ******************************************************************************
# Initialisations
SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .c .cpp .o .so

.PRECIOUS: %.cpp

# For a debug build, replace -DNDEBUG=1 with -DDEBUG=1 -D_DEBUG=1
CPPFLAGS += -I../include -funsigned-char -DNDEBUG=1 $(EXPAT_CPPFLAGS)
ifdef HAVE_STDINT
	CPPFLAGS += -DEXV_HAVE_STDINT_H=1
endif

LDFLAGS += $(EXPAT_LDFLAGS)
LIBS += $(EXPAT_LIBS)

# Generic variables
CCOBJ = $(CCSRC:.cpp=.o)
LOBJ = $(CCSRC:.cpp=.lo)

ifdef DEP_TRACKING
DEP = $(CCSRC:%.cpp=$(DEPDIR)/%.d)
endif

# ******************************************************************************
# Rules
$(CCOBJ): %.o: %.cpp
	@$(LIBTOOL) --mode=compile $(COMPILE.cc) -o $@ $<
	@$(MAKEDEPEND)
	@$(POSTDEPEND)

%.ii: %.cpp
	set -e; \
	$(CXXCPP) $(CPPFLAGS) $< | sed '/^[ 	]*$$/d' > $@

# ******************************************************************************
# Targets
.PHONY: all mostlyclean clean distclean maintainer-clean

ifdef DEP_TRACKING
# Include targets from dependency files
-include $(DEP)
endif

$(LIBTOOL): $(LIBTOOL_DEPS)
	$(SHELL) $(top_srcdir)/config.status --recheck

xmpsdk: $(LIBRARY)

$(LIBRARY): $(CCOBJ)
	$(LIBTOOL) --mode=link $(LINK.cc) -o $(LIBRARY) $(LOBJ)

mostlyclean:
	$(RM) core
	$(RM) $(CCSRC:.cpp=.ii)
	$(RM) $(CCSRC:%.cpp=.libs/%.d)
	@$(LIBTOOL) --mode=clean $(RM) $(LOBJ)

clean: mostlyclean
	@$(LIBTOOL) --mode=clean $(RM) $(LIBRARY)

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
