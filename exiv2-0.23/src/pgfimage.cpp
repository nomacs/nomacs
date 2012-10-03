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
  File:    pgfimage.cpp
  Version: $Rev: 2681 $
  Author(s): Gilles Caulier (cgilles) <caulier dot gilles at gmail dot com>
  History: 16-Jun-09, gc: submitted
  Credits: See header file
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: pgfimage.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// *****************************************************************************

//#define DEBUG 1

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "pgfimage.hpp"
#include "image.hpp"
#include "pngimage.hpp"
#include "basicio.hpp"
#include "error.hpp"
#include "futils.hpp"

// + standard includes
#include <cstdio>                               // for EOF
#include <string>
#include <cstring>
#include <iostream>
#include <cassert>

// Signature from front of PGF file
const unsigned char pgfSignature[3] = { 0x50, 0x47, 0x46 };

const unsigned char pgfBlank[] = { 0x50,0x47,0x46,0x36,0x10,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x00,0x00,
                                   0x00,0x00,0x18,0x03,0x03,0x00,0x00,0x00,0x14,0x00,0x67,0x08,0x20,0x00,0xc0,0x01,
                                   0x00,0x00,0x37,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x37,0x00,
                                   0x00,0x78,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x37,0x00,0x00,0x78,0x00,0x00,
                                   0x00,0x00,0x01,0x00,0x00,0x00,0x37,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x01,0x00,
                                   0x00,0x00,0x37,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x37,0x00,
                                   0x00,0x78,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00
                                 };

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    PgfImage::PgfImage(BasicIo::AutoPtr io, bool create)
            : Image(ImageType::pgf, mdExif | mdIptc| mdXmp | mdComment, io)
    {
        if (create)
        {
            if (io_->open() == 0)
            {
#ifdef DEBUG
                std::cerr << "Exiv2::PgfImage:: Creating PGF image to memory\n";
#endif
                IoCloser closer(*io_);
                if (io_->write(pgfBlank, sizeof(pgfBlank)) != sizeof(pgfBlank))
                {
#ifdef DEBUG
                    std::cerr << "Exiv2::PgfImage:: Failed to create PGF image on memory\n";
#endif
                }
            }
        }
    } // PgfImage::PgfImage

    void PgfImage::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Exiv2::PgfImage::readMetadata: Reading PGF file " << io_->path() << "\n";
#endif
        if (io_->open() != 0)
        {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isPgfType(*io_, true))
        {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "PGF");
        }
        clearMetadata();

        readPgfMagicNumber(*io_);

        uint32_t headerSize = readPgfHeaderSize(*io_);

        readPgfHeaderStructure(*io_, &pixelWidth_, &pixelHeight_);

        // And now, the most interresting, the user data byte array where metadata are stored as small image.

        long size = 8 + headerSize - io_->tell();

#ifdef DEBUG
        std::cout << "Exiv2::PgfImage::readMetadata: Found Image data (" << size << " bytes)\n";
#endif

        if (size < 0) throw Error(20);
        if (size == 0) return;

        DataBuf imgData(size);
        std::memset(imgData.pData_, 0x0, imgData.size_);
        long bufRead = io_->read(imgData.pData_, imgData.size_);
        if (io_->error()) throw Error(14);
        if (bufRead != imgData.size_) throw Error(20);

        Image::AutoPtr image = Exiv2::ImageFactory::open(imgData.pData_, imgData.size_);
        image->readMetadata();
        exifData() = image->exifData();
        iptcData() = image->iptcData();
        xmpData()  = image->xmpData();

    } // PgfImage::readMetadata

    void PgfImage::writeMetadata()
    {
        if (io_->open() != 0)
        {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        BasicIo::AutoPtr tempIo(io_->temporary()); // may throw
        assert (tempIo.get() != 0);

        doWriteMetadata(*tempIo); // may throw
        io_->close();
        io_->transfer(*tempIo); // may throw

    } // PgfImage::writeMetadata

    void PgfImage::doWriteMetadata(BasicIo& outIo)
    {
        if (!io_->isopen()) throw Error(20);
        if (!outIo.isopen()) throw Error(21);

#ifdef DEBUG
        std::cout << "Exiv2::PgfImage::doWriteMetadata: Writing PGF file " << io_->path() << "\n";
        std::cout << "Exiv2::PgfImage::doWriteMetadata: tmp file created " << outIo.path() << "\n";
#endif

        // Ensure that this is the correct image type
        if (!isPgfType(*io_, true))
        {
            if (io_->error() || io_->eof()) throw Error(20);
            throw Error(22);
        }

        // Ensure PGF version.
        byte mnb            = readPgfMagicNumber(*io_);

        readPgfHeaderSize(*io_);

        int w, h;
        DataBuf header      = readPgfHeaderStructure(*io_, &w, &h);

        Image::AutoPtr img  = ImageFactory::create(ImageType::png);

        img->setExifData(exifData_);
        img->setIptcData(iptcData_);
        img->setXmpData(xmpData_);
        img->writeMetadata();
        int imgSize    = img->io().size();
        DataBuf imgBuf = img->io().read(imgSize);

#ifdef DEBUG
        std::cout << "Exiv2::PgfImage::doWriteMetadata: Creating image to host metadata (" << imgSize << " bytes)\n";
#endif

        //---------------------------------------------------------------

        // Write PGF Signature.
        if (outIo.write(pgfSignature, 3) != 3) throw Error(21);

        // Write Magic number.
        if (outIo.putb(mnb) == EOF) throw Error(21);

        // Write new Header size.
        uint32_t newHeaderSize = header.size_ + imgSize;
        DataBuf buffer(4);
        memcpy (buffer.pData_, &newHeaderSize, 4);
        if (outIo.write(buffer.pData_, 4) != 4) throw Error(21);

#ifdef DEBUG
        std::cout << "Exiv2::PgfImage: new PGF header size : " << newHeaderSize << " bytes\n";

        printf("%x\n", buffer.pData_[0]);
        printf("%x\n", buffer.pData_[1]);
        printf("%x\n", buffer.pData_[2]);
        printf("%x\n", buffer.pData_[3]);
#endif

        // Write Header data.
        if (outIo.write(header.pData_, header.size_) != header.size_) throw Error(21);

        // Write new metadata byte array.
        if (outIo.write(imgBuf.pData_, imgBuf.size_) != imgBuf.size_) throw Error(21);

        // Copy the rest of PGF image data.

        DataBuf buf(4096);
        long readSize = 0;
        while ((readSize=io_->read(buf.pData_, buf.size_)))
        {
            if (outIo.write(buf.pData_, readSize) != readSize) throw Error(21);
        }
        if (outIo.error()) throw Error(21);

    } // PgfImage::doWriteMetadata

    byte PgfImage::readPgfMagicNumber(BasicIo& iIo)
    {
        byte b = iIo.getb();
        if (iIo.error()) throw Error(14);

        if (b < 0x36)   // 0x36 = '6'.
        {
            // Not right Magick version.
#ifdef DEBUG
            std::cout << "Exiv2::PgfImage::readMetadata: wrong Magick number\n";
#endif
        }

        return b;
    } // PgfImage::readPgfMagicNumber

    uint32_t PgfImage::readPgfHeaderSize(BasicIo& iIo)
    {
        DataBuf buffer(4);
        long bufRead = iIo.read(buffer.pData_, buffer.size_);
        if (iIo.error()) throw Error(14);
        if (bufRead != buffer.size_) throw Error(20);

        uint32_t headerSize = 0;
        memcpy (&headerSize, buffer.pData_, 4);      // TODO : check endianness.
        if (headerSize <= 0 ) throw Error(22);

#ifdef DEBUG
        std::cout << "Exiv2::PgfImage: PGF header size : " << headerSize << " bytes\n";
#endif

        return headerSize;
    } // PgfImage::readPgfHeaderSize

    DataBuf PgfImage::readPgfHeaderStructure(BasicIo& iIo, int* width, int* height)
    {
        DataBuf header(16);
        long bufRead = iIo.read(header.pData_, header.size_);
        if (iIo.error()) throw Error(14);
        if (bufRead != header.size_) throw Error(20);

        memcpy(width,  &header.pData_[0], 4);      // TODO : check endianness.
        memcpy(height, &header.pData_[4], 4);      // TODO : check endianness.

        /* NOTE: properties not yet used
        byte nLevels  = buffer.pData_[8];
        byte quality  = buffer.pData_[9];
        byte bpp      = buffer.pData_[10];
        byte channels = buffer.pData_[11];
        */
        byte mode     = header.pData_[12];

        if (mode == 2)  // Indexed color image. We pass color table (256 * 3 bytes).
        {
            header.alloc(16 + 256*3);

            bufRead = iIo.read(&header.pData_[16], 256*3);
            if (iIo.error()) throw Error(14);
            if (bufRead != 256*3) throw Error(20);
        }

        return header;
    } // PgfImage::readPgfHeaderStructure

    // *************************************************************************
    // free functions
    Image::AutoPtr newPgfInstance(BasicIo::AutoPtr io, bool create)
    {
        Image::AutoPtr image(new PgfImage(io, create));
        if (!image->good())
        {
            image.reset();
        }
        return image;
    }

    bool isPgfType(BasicIo& iIo, bool advance)
    {
        const int32_t len = 3;
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof())
        {
            return false;
        }
        int rc = memcmp(buf, pgfSignature, 3);
        if (!advance || rc != 0)
        {
            iIo.seek(-len, BasicIo::cur);
        }

        return rc == 0;
    }
}                                       // namespace Exiv2
