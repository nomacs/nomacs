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
# Version:   $Rev: 2714 $
# Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
# History:   28-May-05, ahu: created
#
# Description:
#  Simple Makefile to create html documentation from templates. Requires
#  some special tools (awk, python, xsltproc) but really only needs to 
#  be used to update the documentation after changing Exiv2 tags in the
#  source code.
#
# Restrictions:
#  Only tested with GNU make.
#

TABLES = Exif \
         Canon \
         CanonCs \
         CanonSi \
         CanonCf \
         CanonPi \
         CanonFi \
         CanonPa \
         CanonPr \
         Fujifilm \
         Minolta \
         MinoltaCsNew \
         MinoltaCs5D \
         MinoltaCs7D \
         Nikon1 \
         Nikon2 \
         Nikon3 \
         NikonVr \
         NikonPc \
         NikonWt \
         NikonIi \
         NikonAf \
         NikonAf2 \
         NikonAFT \
         NikonFi \
         NikonMe \
         NikonFl1 \
         NikonFl2 \
         NikonFl3 \
         NikonSiD80 \
         NikonSiD40 \
         NikonSiD300a \
         NikonSiD300b \
         NikonSi01xx \
         NikonCb1 \
         NikonCb2 \
         NikonCb2a \
         NikonCb2b \
         NikonCb3 \
         NikonCb4 \
         NikonLd1 \
         NikonLd2 \
         NikonLd3 \
         Olympus \
         OlympusCs \
         OlympusEq \
         OlympusRd \
         OlympusRd2 \
         OlympusIp \
         OlympusFi \
         OlympusFe1 \
         OlympusRi \
	 Panasonic \
         PanasonicRaw \
         Pentax \
         Samsung2 \
         SamsungPictureWizard \
         Sigma \
         Sony1 \
         SonyMinolta \
         Sony1Cs \
         Sony1Cs2 \
         Sony1MltCs7D \
         Sony1MltCsOld \
         Sony1MltCsA100

SCHEMA = xmp_dc \
         xmp_xmp \
         xmp_xmpRights \
         xmp_xmpMM \
         xmp_xmpBJ \
         xmp_xmpTPg \
         xmp_xmpDM \
         xmp_pdf \
         xmp_photoshop \
         xmp_crs \
         xmp_tiff \
         xmp_exif \
         xmp_aux \
         xmp_iptc \
         xmp_MicrosoftPhoto \
         xmp_digiKam \
         xmp_kipi \
         xmp_iptcExt \
         xmp_plus \
         xmp_mediapro \
         xmp_expressionmedia \
         xmp_MP \
         xmp_MPRI \
         xmp_MPReg \
         xmp_mwg-rs

TAGLIST = ../../src/taglist

# **********************************************************************
# ======================================================================
# **********************************************************************

# Initialisations
SHELL = /bin/sh

.SUFFIXES:

# Default make target
all: tags

.PHONY: tags mostlyclean clean distclean maintainer-clean

tags: $(TABLES) Iptc $(SCHEMA)
	@./gen.py *.html.in

$(TABLES): $(TAGLIST)
	@echo Generating $@ table...
	@$(TAGLIST) $@ | sed -e"s/</\&lt;/g" -e"s/>/\&gt;/g" | awk -f tags.awk > $@.xml
	@xsltproc tags.xsl $@.xml > $@.tmp
	@sed "s/report1/$@/" $@.tmp > __$@__
	@touch $@
	@rm -f $@.tmp

Iptc: $(TAGLIST)
	@echo Generating $@ table...
	@$(TAGLIST) $@ | sed -e"s/</\&lt;/g" -e"s/>/\&gt;/g" -e"s/±/\&#177;/g" \
	               | awk -f iptc.awk > $@.xml
	@xsltproc iptc.xsl $@.xml > $@.tmp
	@sed "s/report1/$@/g" $@.tmp > __$@__
	@touch $@
	@rm -f $@.tmp

$(SCHEMA): $(TAGLIST)
	@echo Generating $@ table...
	@echo $@ | sed "s/xmp_//" | xargs $(TAGLIST) | sed -e"s/</\&lt;/g" -e"s/>/\&gt;/g" | awk -f xmp.awk > $@.xml
	@xsltproc xmp.xsl $@.xml > $@.tmp
	@sed "s/report1/$@/" $@.tmp > __$@__
	@touch $@
	@rm -f $@.tmp

$(TAGLIST): 
	$(error File $(TAGLIST) does not exist. Did you build the library and programs in the src/ directory?)

mostlyclean:
	rm -f *.xml
	rm -f *.tmp

clean: mostlyclean
	rm -f $(TABLES:%=__%__) __Iptc__
	rm -f $(SCHEMA:%=__%__)
	rm -f $(TABLES) $(SCHEMA) Iptc
	rm -f *.html

distclean: clean
	rm -f *~

maintainer-clean: distclean
