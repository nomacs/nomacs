// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2012 Andreas Huggel <ahuggel@gmx.net>
 *
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*
  File:      xmpsidecar.cpp
  Version:   $Rev: 2681 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   07-Mar-08, ahu: created
  Credits:   See header file
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: xmpsidecar.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "xmpsidecar.hpp"
#include "image.hpp"
#include "basicio.hpp"
#include "error.hpp"
#include "xmp.hpp"
#include "futils.hpp"
#include "convert.hpp"

// + standard includes
#include <string>
#include <iostream>
#include <cassert>

// *****************************************************************************
namespace {
    const char* xmlHeader = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    const long  xmlHdrCnt = 39; // without the trailing 0-character
}

// class member definitions
namespace Exiv2 {


    XmpSidecar::XmpSidecar(BasicIo::AutoPtr io, bool create)
        : Image(ImageType::xmp, mdXmp, io)
    {
        if (create) {
            if (io_->open() == 0) {
                IoCloser closer(*io_);
                io_->write(reinterpret_cast<const byte*>(xmlHeader), xmlHdrCnt);
            }
        }
    } // XmpSidecar::XmpSidecar

    std::string XmpSidecar::mimeType() const
    {
        return "application/rdf+xml";
    }

    void XmpSidecar::setComment(const std::string& /*comment*/)
    {
        // not supported
        throw(Error(32, "Image comment", "XMP"));
    }

    void XmpSidecar::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Reading XMP file " << io_->path() << "\n";
#endif
        if (io_->open() != 0) {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isXmpType(*io_, false)) {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "XMP");
        }
        // Read the XMP packet from the IO stream
        std::string xmpPacket;
        const long len = 64 * 1024;
        byte buf[len];
        long l;
        while ((l = io_->read(buf, len)) > 0) {
            xmpPacket.append(reinterpret_cast<char*>(buf), l);
        }
        if (io_->error()) throw Error(14);
        clearMetadata();
        xmpPacket_ = xmpPacket;
        if (xmpPacket_.size() > 0 && XmpParser::decode(xmpData_, xmpPacket_)) {
#ifndef SUPPRESS_WARNINGS
            EXV_WARNING << "Failed to decode XMP metadata.\n";
#endif
        }
        copyXmpToIptc(xmpData_, iptcData_);
        copyXmpToExif(xmpData_, exifData_);
    } // XmpSidecar::readMetadata

    void XmpSidecar::writeMetadata()
    {
        if (io_->open() != 0) {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);

        if (writeXmpFromPacket() == false) {
            copyExifToXmp(exifData_, xmpData_);
            copyIptcToXmp(iptcData_, xmpData_);
            if (XmpParser::encode(xmpPacket_, xmpData_,
                                  XmpParser::omitPacketWrapper|XmpParser::useCompactFormat) > 1) {
#ifndef SUPPRESS_WARNINGS
                EXV_ERROR << "Failed to encode XMP metadata.\n";
#endif
            }
        }
        if (xmpPacket_.size() > 0) {
            if (xmpPacket_.substr(0, 5)  != "<?xml") {
                xmpPacket_ = xmlHeader + xmpPacket_;
            }
            BasicIo::AutoPtr tempIo(io_->temporary()); // may throw
            assert(tempIo.get() != 0);
            // Write XMP packet
            if (   tempIo->write(reinterpret_cast<const byte*>(xmpPacket_.data()),
                                 static_cast<long>(xmpPacket_.size()))
                != static_cast<long>(xmpPacket_.size())) throw Error(21);
            if (tempIo->error()) throw Error(21);
            io_->close();
            io_->transfer(*tempIo); // may throw
        }
    } // XmpSidecar::writeMetadata

    // *************************************************************************
    // free functions
    Image::AutoPtr newXmpInstance(BasicIo::AutoPtr io, bool create)
    {
        Image::AutoPtr image(new XmpSidecar(io, create));
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isXmpType(BasicIo& iIo, bool advance)
    {
        /*
          Check if the file starts with an optional XML declaration followed by
          either an XMP header (<?xpacket ... ?>) or an <x:xmpmeta> element.

          In addition, in order for empty XmpSidecar objects as created by
          Exiv2 to pass the test, just an XML header is also considered ok.
         */
        const int32_t len = 80;
        byte buf[len];
        iIo.read(buf, xmlHdrCnt + 1);
        if (   iIo.eof()
            && 0 == strncmp(reinterpret_cast<const char*>(buf), xmlHeader, xmlHdrCnt)) {
            return true;
        }
        if (iIo.error() || iIo.eof()) {
            return false;
        }
        iIo.read(buf + xmlHdrCnt + 1, len - xmlHdrCnt - 1);
        if (iIo.error() || iIo.eof()) {
            return false;
        }
        // Skip leading BOM
        int32_t start = 0;
        if (0 == strncmp(reinterpret_cast<const char*>(buf), "\xef\xbb\xbf", 3)) {
            start = 3;
        }
        bool rc = false;
        std::string head(reinterpret_cast<const char*>(buf + start), len - start);
        if (head.substr(0, 5)  == "<?xml") {
            // Forward to the next tag
            for (unsigned i = 5; i < head.size(); ++i) {
                if (head[i] == '<') {
                    head = head.substr(i);
                    break;
                }
            }
        }
        if (   head.size() > 9
            && (   head.substr(0, 9)  == "<?xpacket"
                || head.substr(0, 10) == "<x:xmpmeta")) {
            rc = true;
        }
        if (!advance || !rc) {
            iIo.seek(-(len - start), BasicIo::cur); // Swallow the BOM
        }
        return rc;

    }

}                                       // namespace Exiv2
