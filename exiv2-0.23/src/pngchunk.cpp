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
  File:    pngchunk.cpp
  Version: $Rev: 2681 $
  Author(s): Gilles Caulier (cgilles) <caulier dot gilles at gmail dot com>
  History: 12-Jun-06, gc: submitted
  Credits: See header file
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: pngchunk.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

//#define DEBUG 1
#ifdef EXV_HAVE_LIBZ

extern "C" {
#include <zlib.h>     // To uncompress or compress text chunk
}

#include "pngchunk_int.hpp"
#include "tiffimage.hpp"
#include "jpgimage.hpp"
#include "exif.hpp"
#include "iptc.hpp"
#include "image.hpp"
#include "error.hpp"

// + standard includes
#include <sstream>
#include <iomanip>
#include <string>
#include <cstring>
#include <iostream>
#include <cassert>
#include <cstdio>

/*

URLs to find informations about PNG chunks :

tEXt and zTXt chunks : http://www.vias.org/pngguide/chapter11_04.html
iTXt chunk           : http://www.vias.org/pngguide/chapter11_05.html
PNG tags             : http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/PNG.html#TextualData

*/

// *****************************************************************************
// class member definitions
namespace Exiv2 {
    namespace Internal {

    void PngChunk::decodeIHDRChunk(const DataBuf& data,
                                   int*           outWidth,
                                   int*           outHeight)
    {
        // Extract image width and height from IHDR chunk.

        *outWidth  = getLong((const byte*)data.pData_,     bigEndian);
        *outHeight = getLong((const byte*)data.pData_ + 4, bigEndian);

    } // PngChunk::decodeIHDRChunk

    void PngChunk::decodeTXTChunk(Image*         pImage,
                                  const DataBuf& data,
                                  TxtChunkType   type)
    {
        DataBuf key = keyTXTChunk(data);

#ifdef DEBUG
        std::cout << "Exiv2::PngChunk::decodeTXTChunk: TXT chunk key: "
                  << std::string((const char*)key.pData_, key.size_) << "\n";
#endif
        DataBuf arr = parseTXTChunk(data, key.size_, type);

#ifdef DEBUG
        std::cout << "Exiv2::PngChunk::decodeTXTChunk: TXT chunk data: "
                  << std::string((const char*)arr.pData_, arr.size_) << "\n";
#endif
        parseChunkContent(pImage, key.pData_, key.size_, arr);

    } // PngChunk::decodeTXTChunk

    DataBuf PngChunk::keyTXTChunk(const DataBuf& data, bool stripHeader)
    {
        // From a tEXt, zTXt, or iTXt chunk,
        // we get the key, it's a null terminated string at the chunk start
        if (data.size_ <= (stripHeader ? 8 : 0)) throw Error(14);
        const byte *key = data.pData_ + (stripHeader ? 8 : 0);

        // Find null string at end of key.
        int keysize=0;
        for ( ; key[keysize] != 0 ; keysize++)
        {
            // look if keysize is valid.
            if (keysize >= data.size_)
                throw Error(14);
        }

        return DataBuf(key, keysize);

    } // PngChunk::keyTXTChunk

    DataBuf PngChunk::parseTXTChunk(const DataBuf& data,
                                    int            keysize,
                                    TxtChunkType   type)
    {
        DataBuf arr;

        if(type == zTXt_Chunk)
        {
            // Extract a deflate compressed Latin-1 text chunk

            // we get the compression method after the key
            const byte* compressionMethod = data.pData_ + keysize + 1;
            if ( *compressionMethod != 0x00 )
            {
                // then it isn't zlib compressed and we are sunk
#ifdef DEBUG
                std::cerr << "Exiv2::PngChunk::parseTXTChunk: Non-standard zTXt compression method.\n";
#endif
                throw Error(14);
            }

            // compressed string after the compression technique spec
            const byte* compressedText      = data.pData_ + keysize + 2;
            unsigned int compressedTextSize = data.size_  - keysize - 2;

            zlibUncompress(compressedText, compressedTextSize, arr);
        }
        else if(type == tEXt_Chunk)
        {
            // Extract a non-compressed Latin-1 text chunk

            // the text comes after the key, but isn't null terminated
            const byte* text = data.pData_ + keysize + 1;
            long textsize    = data.size_  - keysize - 1;

            arr = DataBuf(text, textsize);
        }
        else if(type == iTXt_Chunk)
        {
            // Extract a deflate compressed or uncompressed UTF-8 text chunk

            // we get the compression flag after the key
            const byte* compressionFlag   = data.pData_ + keysize + 1;
            // we get the compression method after the compression flag
            const byte* compressionMethod = data.pData_ + keysize + 2;
            // language description string after the compression technique spec
            std::string languageText((const char*)(data.pData_ + keysize + 3));
            unsigned int languageTextSize = static_cast<unsigned int>(languageText.size());
            // translated keyword string after the language description
            std::string translatedKeyText((const char*)(data.pData_ + keysize + 3 + languageTextSize +1));
            unsigned int translatedKeyTextSize = static_cast<unsigned int>(translatedKeyText.size());

            if ( compressionFlag[0] == 0x00 )
            {
                // then it's an uncompressed iTXt chunk
#ifdef DEBUG
                std::cout << "Exiv2::PngChunk::parseTXTChunk: We found an uncompressed iTXt field\n";
#endif

                // the text comes after the translated keyword, but isn't null terminated
                const byte* text = data.pData_ + keysize + 3 + languageTextSize + 1 + translatedKeyTextSize + 1;
                long textsize    = data.size_ - (keysize + 3 + languageTextSize + 1 + translatedKeyTextSize + 1);

                arr.alloc(textsize);
                arr = DataBuf(text, textsize);
            }
            else if ( compressionFlag[0] == 0x01 && compressionMethod[0] == 0x00 )
            {
                // then it's a zlib compressed iTXt chunk
#ifdef DEBUG
                std::cout << "Exiv2::PngChunk::parseTXTChunk: We found a zlib compressed iTXt field\n";
#endif

                // the compressed text comes after the translated keyword, but isn't null terminated
                const byte* compressedText = data.pData_ + keysize + 3 + languageTextSize + 1 + translatedKeyTextSize + 1;
                long compressedTextSize    = data.size_ - (keysize + 3 + languageTextSize + 1 + translatedKeyTextSize + 1);

                zlibUncompress(compressedText, compressedTextSize, arr);
            }
            else
            {
                // then it isn't zlib compressed and we are sunk
#ifdef DEBUG
                std::cerr << "Exiv2::PngChunk::parseTXTChunk: Non-standard iTXt compression method.\n";
#endif
                throw Error(14);
            }
        }
        else
        {
#ifdef DEBUG
            std::cerr << "Exiv2::PngChunk::parseTXTChunk: We found a field, not expected though\n";
#endif
            throw Error(14);
        }

        return arr;

    } // PngChunk::parsePngChunk

    void PngChunk::parseChunkContent(      Image*  pImage,
                                     const byte*   key,
                                           long    keySize,
                                     const DataBuf arr)
    {
        // We look if an ImageMagick EXIF raw profile exist.

        if (   keySize >= 21 
            && (   memcmp("Raw profile type exif", key, 21) == 0
                || memcmp("Raw profile type APP1", key, 21) == 0)
            && pImage->exifData().empty())
        {
            DataBuf exifData = readRawProfile(arr);
            long length      = exifData.size_;

            if (length > 0)
            {
                // Find the position of Exif header in bytes array.

                const byte exifHeader[] = { 0x45, 0x78, 0x69, 0x66, 0x00, 0x00 };
                long pos = -1;

                for (long i=0 ; i < length-(long)sizeof(exifHeader) ; i++)
                {
                    if (memcmp(exifHeader, &exifData.pData_[i], sizeof(exifHeader)) == 0)
                    {
                        pos = i;
                        break;
                    }
                }

                // If found it, store only these data at from this place.

                if (pos !=-1)
                {
#ifdef DEBUG
                    std::cout << "Exiv2::PngChunk::parseChunkContent: Exif header found at position " << pos << "\n";
#endif
                    pos = pos + sizeof(exifHeader);
                    ByteOrder bo = TiffParser::decode(pImage->exifData(),
                                                      pImage->iptcData(),
                                                      pImage->xmpData(),
                                                      exifData.pData_ + pos,
                                                      length - pos);
                    pImage->setByteOrder(bo);
                }
                else
                {
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Failed to decode Exif metadata.\n";
#endif
                    pImage->exifData().clear();
                }
            }
        }

        // We look if an ImageMagick IPTC raw profile exist.

        if (   keySize >= 21
            && memcmp("Raw profile type iptc", key, 21) == 0
            && pImage->iptcData().empty()) {
            DataBuf psData = readRawProfile(arr);
            if (psData.size_ > 0) {
                Blob iptcBlob;
                const byte *record = 0;
                uint32_t sizeIptc = 0;
                uint32_t sizeHdr = 0;

                const byte* pEnd = psData.pData_ + psData.size_;
                const byte* pCur = psData.pData_;
                while (   pCur < pEnd
                       && 0 == Photoshop::locateIptcIrb(pCur,
                                                        static_cast<long>(pEnd - pCur),
                                                        &record,
                                                        &sizeHdr,
                                                        &sizeIptc)) {
                    if (sizeIptc) {
#ifdef DEBUG
                        std::cerr << "Found IPTC IRB, size = " << sizeIptc << "\n";
#endif
                        append(iptcBlob, record + sizeHdr, sizeIptc);
                    }
                    pCur = record + sizeHdr + sizeIptc;
                    pCur += (sizeIptc & 1);
                }
                if (   iptcBlob.size() > 0
                    && IptcParser::decode(pImage->iptcData(),
                                          &iptcBlob[0],
                                          static_cast<uint32_t>(iptcBlob.size()))) {
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Failed to decode IPTC metadata.\n";
#endif
                    pImage->clearIptcData();
                }
                // If there is no IRB, try to decode the complete chunk data
                if (   iptcBlob.empty()
                    && IptcParser::decode(pImage->iptcData(),
                                          psData.pData_,
                                          psData.size_)) {
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Failed to decode IPTC metadata.\n";
#endif
                    pImage->clearIptcData();
                }
            } // if (psData.size_ > 0)
        }

        // We look if an ImageMagick XMP raw profile exist.

        if (   keySize >= 20
            && memcmp("Raw profile type xmp", key, 20) == 0
            && pImage->xmpData().empty()) 
        {
            DataBuf xmpBuf = readRawProfile(arr);
            long length    = xmpBuf.size_;

            if (length > 0)
            {
                std::string& xmpPacket = pImage->xmpPacket();
                xmpPacket.assign(reinterpret_cast<char*>(xmpBuf.pData_), length);
                std::string::size_type idx = xmpPacket.find_first_of('<');
                if (idx != std::string::npos && idx > 0)
                {
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Removing " << idx
                                << " characters from the beginning of the XMP packet\n";
#endif
                    xmpPacket = xmpPacket.substr(idx);
                }
                if (XmpParser::decode(pImage->xmpData(), xmpPacket))
                {
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Failed to decode XMP metadata.\n";
#endif
                }
            }
        }

        // We look if an Adobe XMP string exist.

        if (   keySize >= 17
            && memcmp("XML:com.adobe.xmp", key, 17) == 0
            && pImage->xmpData().empty())
        {
            if (arr.size_ > 0)
            {
                std::string& xmpPacket = pImage->xmpPacket();
                xmpPacket.assign(reinterpret_cast<char*>(arr.pData_), arr.size_);
                std::string::size_type idx = xmpPacket.find_first_of('<');
                if (idx != std::string::npos && idx > 0)
                {
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Removing " << idx << " characters "
                                << "from the beginning of the XMP packet\n";
#endif
                    xmpPacket = xmpPacket.substr(idx);
                }
                if (XmpParser::decode(pImage->xmpData(), xmpPacket))
                {
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Failed to decode XMP metadata.\n";
#endif
                }
            }
        }

        // We look if a comments string exist. Note than we use only 'Description' keyword which
        // is dedicaced to store long comments. 'Comment' keyword is ignored.

        if (   keySize >= 11
            && memcmp("Description", key, 11) == 0
            && pImage->comment().empty())
        {
            pImage->setComment(std::string(reinterpret_cast<char*>(arr.pData_), arr.size_));
        }

    } // PngChunk::parseChunkContent

    std::string PngChunk::makeMetadataChunk(const std::string& metadata,
                                                  MetadataId   type)
    {
        std::string chunk;
        std::string rawProfile;

        switch (type) {
        case mdComment:
            chunk = makeUtf8TxtChunk("Description", metadata, true);
            break;
        case mdExif:
            rawProfile = writeRawProfile(metadata, "exif");
            chunk = makeAsciiTxtChunk("Raw profile type exif", rawProfile, true);
            break;
        case mdIptc:
            rawProfile = writeRawProfile(metadata, "iptc");
            chunk = makeAsciiTxtChunk("Raw profile type iptc", rawProfile, true);
            break;
        case mdXmp:
            chunk = makeUtf8TxtChunk("XML:com.adobe.xmp", metadata, false);
            break;
        case mdNone:
            assert(false);
	}

        return chunk;

    } // PngChunk::makeMetadataChunk

    void PngChunk::zlibUncompress(const byte*  compressedText,
                                  unsigned int compressedTextSize,
                                  DataBuf&     arr)
    {
        uLongf uncompressedLen = compressedTextSize * 2; // just a starting point
        int zlibResult;
        int dos = 0;

        do {
            arr.alloc(uncompressedLen);
            zlibResult = uncompress((Bytef*)arr.pData_,
                                    &uncompressedLen,
                                    compressedText,
                                    compressedTextSize);
            if (zlibResult == Z_OK) {
                assert((uLongf)arr.size_ >= uncompressedLen);
                arr.size_ = uncompressedLen;
            }
            else if (zlibResult == Z_BUF_ERROR) {
                // the uncompressedArray needs to be larger
                uncompressedLen *= 2;
                // DoS protection. can't be bigger than 64k
                if (uncompressedLen > 131072) {
                    if (++dos > 1) break;
                    uncompressedLen = 131072;
                }
            }
            else {
                // something bad happened
                throw Error(14);
            }
        }
        while (zlibResult == Z_BUF_ERROR);

        if (zlibResult != Z_OK) {
            throw Error(14);
        }
    } // PngChunk::zlibUncompress

    std::string PngChunk::zlibCompress(const std::string& text)
    {
        uLongf compressedLen = static_cast<uLongf>(text.size() * 2); // just a starting point
        int zlibResult;

        DataBuf arr;
        do {
            arr.alloc(compressedLen);
            zlibResult = compress2((Bytef*)arr.pData_, &compressedLen,
                                   (const Bytef*)text.data(), static_cast<uLong>(text.size()),
                                   Z_BEST_COMPRESSION);

            switch (zlibResult) {
            case Z_OK:
                assert((uLongf)arr.size_ >= compressedLen);
                arr.size_ = compressedLen;
                break;
            case Z_BUF_ERROR:
                // The compressed array needs to be larger
#ifdef DEBUG
                std::cout << "Exiv2::PngChunk::parsePngChunk: doubling size for compression.\n";
#endif
                compressedLen *= 2;
                // DoS protection. Cap max compressed size
                if ( compressedLen > 131072 ) throw Error(14);
                break;
            default:
                // Something bad happened
                throw Error(14);
            }
        } while (zlibResult == Z_BUF_ERROR);

        return std::string((const char*)arr.pData_, arr.size_);

    } // PngChunk::zlibCompress

    std::string PngChunk::makeAsciiTxtChunk(const std::string& keyword,
                                            const std::string& text,
                                            bool               compress)
    {
        // Chunk structure: length (4 bytes) + chunk type + chunk data + CRC (4 bytes)
        // Length is the size of the chunk data
        // CRC is calculated on chunk type + chunk data

        // Compressed text chunk using zlib.
        // Chunk data format : keyword + 0x00 + compression method (0x00) + compressed text

        // Not Compressed text chunk.
        // Chunk data format : keyword + 0x00 + text

        // Build chunk data, determine chunk type
        std::string chunkData = keyword + '\0';
        std::string chunkType;
        if (compress) {
            chunkData += '\0' + zlibCompress(text);
            chunkType = "zTXt";
        }
        else {
            chunkData += text;
            chunkType = "tEXt";
        }
        // Determine length of the chunk data
        byte length[4];
        ul2Data(length, static_cast<uint32_t>(chunkData.size()), bigEndian);
        // Calculate CRC on chunk type and chunk data
        std::string crcData = chunkType + chunkData;
        uLong tmp = crc32(0L, Z_NULL, 0);
        tmp       = crc32(tmp, (const Bytef*)crcData.data(), static_cast<uInt>(crcData.size()));
        byte crc[4];
        ul2Data(crc, tmp, bigEndian);
        // Assemble the chunk
        return std::string((const char*)length, 4) + chunkType + chunkData + std::string((const char*)crc, 4);

    } // PngChunk::makeAsciiTxtChunk

    std::string PngChunk::makeUtf8TxtChunk(const std::string& keyword,
                                           const std::string& text,
                                           bool               compress)
    {
        // Chunk structure: length (4 bytes) + chunk type + chunk data + CRC (4 bytes)
        // Length is the size of the chunk data
        // CRC is calculated on chunk type + chunk data

        // Chunk data format : keyword + 0x00 + compression flag (0x00: uncompressed - 0x01: compressed)
        //                     + compression method (0x00: zlib format) + language tag (null) + 0x00
        //                     + translated keyword (null) + 0x00 + text (compressed or not)

        // Build chunk data, determine chunk type
        std::string chunkData = keyword;
        if (compress) {
            static const char flags[] = { 0x00, 0x01, 0x00, 0x00, 0x00 };
            chunkData += std::string(flags, 5) + zlibCompress(text);
        }
        else {
            static const char flags[] = { 0x00, 0x00, 0x00, 0x00, 0x00 };
            chunkData += std::string(flags, 5) + text;
        }
        // Determine length of the chunk data
        byte length[4];
        ul2Data(length, static_cast<uint32_t>(chunkData.size()), bigEndian);
        // Calculate CRC on chunk type and chunk data
        std::string chunkType = "iTXt";
        std::string crcData = chunkType + chunkData;
        uLong tmp = crc32(0L, Z_NULL, 0);
        tmp       = crc32(tmp, (const Bytef*)crcData.data(), static_cast<uInt>(crcData.size()));
        byte crc[4];
        ul2Data(crc, tmp, bigEndian);
        // Assemble the chunk
        return std::string((const char*)length, 4) + chunkType + chunkData + std::string((const char*)crc, 4);

    } // PngChunk::makeUtf8TxtChunk

    DataBuf PngChunk::readRawProfile(const DataBuf& text)
    {
        DataBuf                 info;
        register long           i;
        register unsigned char *dp;
        const char             *sp;
        unsigned int            nibbles;
        long                    length;
        unsigned char           unhex[103]={0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
                                            0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
                                            0,0,0,0,0,0,0,0,0,1, 2,3,4,5,6,7,8,9,0,0,
                                            0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
                                            0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,10,11,12,
                                            13,14,15};
        if (text.size_ == 0) {
            return DataBuf();
        }

        sp = (char*)text.pData_+1;

        // Look for newline

        while (*sp != '\n')
            sp++;

        // Look for length

        while (*sp == '\0' || *sp == ' ' || *sp == '\n')
            sp++;

        length = (long) atol(sp);

        while (*sp != ' ' && *sp != '\n')
            sp++;

        // Allocate space

        if (length == 0)
        {
#ifdef DEBUG
            std::cerr << "Exiv2::PngChunk::readRawProfile: Unable To Copy Raw Profile: invalid profile length\n";
#endif
            return DataBuf();
        }

        info.alloc(length);

        if (info.size_ != length)
        {
#ifdef DEBUG
            std::cerr << "Exiv2::PngChunk::readRawProfile: Unable To Copy Raw Profile: cannot allocate memory\n";
#endif
            return DataBuf();
        }

        // Copy profile, skipping white space and column 1 "=" signs

        dp      = (unsigned char*)info.pData_;
        nibbles = length * 2;

        for (i = 0; i < (long) nibbles; i++)
        {
            while (*sp < '0' || (*sp > '9' && *sp < 'a') || *sp > 'f')
            {
                if (*sp == '\0')
                {
#ifdef DEBUG
                    std::cerr << "Exiv2::PngChunk::readRawProfile: Unable To Copy Raw Profile: ran out of data\n";
#endif
                    return DataBuf();
                }

                sp++;
            }

            if (i%2 == 0)
                *dp = (unsigned char) (16*unhex[(int) *sp++]);
            else
                (*dp++) += unhex[(int) *sp++];
        }

        return info;

    } // PngChunk::readRawProfile

    std::string PngChunk::writeRawProfile(const std::string& profileData,
                                          const char*        profileType)
    {
        static byte hex[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

        std::ostringstream oss;
        oss << '\n' << profileType << '\n' << std::setw(8) << profileData.size();
        const char* sp = profileData.data();
        for (std::string::size_type i = 0; i < profileData.size(); ++i) {
            if (i % 36 == 0) oss << '\n';
            oss << hex[((*sp >> 4) & 0x0f)];
            oss << hex[((*sp++) & 0x0f)];
        }
        oss << '\n';
        return oss.str();

    } // PngChunk::writeRawProfile

}}                                      // namespace Internal, Exiv2
#endif // ifdef EXV_HAVE_LIBZ
