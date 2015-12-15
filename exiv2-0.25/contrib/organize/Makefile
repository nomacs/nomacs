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
# Version:   $Rev: 3777 $
# Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
# History:   31-Jan-09, ahu: created
#
# Description:
#  Simple Makefile to build the organize application. Requires installed
#  exiv2 library and headers. Adapted from samples/Makefile.
#
# Restrictions:
#  Requires GNU make.
#

# ******************************************************************************
# Default make target
all: ozbin

# Include system configuration
top_srcdir = ../..
include $(top_srcdir)/config/config.mk
include boost.mk

# ******************************************************************************
# Source files

# Source files for the organize application
OZMAIN = organize.cpp
OZSRC  = helpers.cpp MD5.cpp

# ******************************************************************************
# Initialisations
SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .c .cpp .o .so

.PRECIOUS: %.cpp

CPPFLAGS := -I$(BOOST_INC_DIR) `pkg-config exiv2 --cflags`
ifdef HAVE_STDINT
	CPPFLAGS += -DEXV_HAVE_STDINT_H=1
endif

LDFLAGS := $(BOOST_LIBS) `pkg-config exiv2 --libs`

OZOBJ = $(OZSRC:.cpp=.o) $(OZMAIN:.cpp=.o)
OZBIN = $(OZMAIN:.cpp=)
OZEXE = $(OZMAIN:.cpp=$(EXEEXT))

ifdef DEP_TRACKING
DEP = $(OZMAIN:%.cpp=$(DEPDIR)/%.d) $(OZSRC:%.cpp=$(DEPDIR)/%.d)
endif

# ******************************************************************************
# Rules
ozbin: $(OZBIN)

$(OZOBJ): %.o: %.cpp
	$(COMPILE.cc) -o $@ $<
	@$(MAKEDEPEND)
	@$(POSTDEPEND)

%.ii: %.cpp
	set -e; \
	$(CXXCPP) -E $(CPPFLAGS) $< | sed '/^[ 	]*$$/d' > $@

# ******************************************************************************
# Targets
.PHONY: all ozbin relink binclean install uninstall mostlyclean clean distclean maintainer-clean

ifdef DEP_TRACKING
# Include targets from dependency files
-include $(DEP)
endif

$(OZBIN): $(OZOBJ)
	$(LIBTOOL) --mode=link $(LINK.cc) -o $@ $(OZOBJ)

relink: binclean organize

install:
	$(INSTALL_DIRS) $(DESTDIR)$(bindir)
	@$(LIBTOOL) --mode=install $(INSTALL_PROGRAM) $(OZEXE) $(DESTDIR)$(bindir)/$(OZEXE)

uninstall:
	@$(LIBTOOL) --mode=uninstall $(RM) $(DESTDIR)$(bindir)/$(OZEXE)
	-rmdir $(DESTDIR)$(bindir)

# Remove binaries, e.g., to relink them
binclean:
	$(RM) $(OZEXE)

mostlyclean:
	$(RM) core
	$(RM) $(OZMAIN:.cpp=.ii) $(OZSRC:.cpp=.ii)
	$(RM) $(OZMAIN:%.cpp=.libs/%.d) $(OZSRC:%.cpp=.libs/%.d)
	-rmdir .libs
	$(RM) $(OZOBJ)

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
maintainer-clean: uninstall distclean
