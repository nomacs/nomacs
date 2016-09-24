// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2015 Andreas Huggel <ahuggel@gmx.net>
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
  File:      asfvideo.cpp
  Version:   $Rev: 3845 $
  Author(s): Abhinav Badola for GSoC 2012 (AB) <mail.abu.to@gmail.com>
  History:   08-Aug-12, AB: created
  Credits:   See header file
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: asfvideo.cpp 3845 2015-06-07 16:29:06Z ahuggel $")

// *****************************************************************************
// included header files
#include "config.h"

#ifdef EXV_ENABLE_VIDEO
#include "asfvideo.hpp"
#include "futils.hpp"
#include "basicio.hpp"
#include "tags.hpp"
#include "tags_int.hpp"
#include "types.hpp"
#include "riffvideo.hpp"
#include "convert.hpp"

// + standard includes
#include <cmath>
#include <cstring>
#include <ctype.h>
#include <cassert>

// *****************************************************************************
// class member definitions
namespace Exiv2 {
    namespace Internal {

    /*!
      Tag Look-up list for ASF Type Video Files
      Associates the GUID of a Tag with its Tag Name(i.e. Human Readable Form)
      Tags have been diferentiated into Various Categories.
      The categories have been listed above the Tag Groups

     */
    extern const TagVocabulary GUIDReferenceTags[] =  {
        /// Top-level ASF object GUIDS
        {   "75B22630-668E-11CF-A6D9-00AA0062CE6C", "Header" },
        {   "75B22636-668E-11CF-A6D9-00AA0062CE6C", "Data" },
        {   "33000890-E5B1-11CF-89F4-00A0C90349CB", "Simple_Index" },
        {   "D6E229D3-35DA-11D1-9034-00A0C90349BE", "Index" },
        {   "FEB103F8-12AD-4C64-840F-2A1D2F7AD48C", "Media_Index" },
        {   "3CB73FD0-0C4A-4803-953D-EDF7B6228F0C", "Timecode_Index" },

        /// Header Object GUIDs
        {   "8CABDCA1-A947-11CF-8EE4-00C00C205365", "File_Properties" },
        {   "B7DC0791-A9B7-11CF-8EE6-00C00C205365", "Stream_Properties" },
        {   "5FBF03B5-A92E-11CF-8EE3-00C00C205365", "Header_Extension" },
        {   "86D15240-311D-11D0-A3A4-00A0C90348F6", "Codec_List" },
        {   "1EFB1A30-0B62-11D0-A39B-00A0C90348F6", "Script_Command" },
        {   "F487CD01-A951-11CF-8EE6-00C00C205365", "Marker" },
        {   "D6E229DC-35DA-11D1-9034-00A0C90349BE", "Bitrate_Mutual_Exclusion" },
        {   "75B22635-668E-11CF-A6D9-00AA0062CE6C", "Error_Correction" },
        {   "75B22633-668E-11CF-A6D9-00AA0062CE6C", "Content_Description" },
        {   "D2D0A440-E307-11D2-97F0-00A0C95EA850", "Extended_Content_Description" },
        {   "2211B3FA-BD23-11D2-B4B7-00A0C955FC6E", "Content_Branding" },
        {   "7BF875CE-468D-11D1-8D82-006097C9A2B2", "Stream_Bitrate_Properties" },
        {   "2211B3FB-BD23-11D2-B4B7-00A0C955FC6E", "Content_Encryption" },
        {   "298AE614-2622-4C17-B935-DAE07EE9289C", "Extended_Content_Encryption" },
        {   "2211B3FC-BD23-11D2-B4B7-00A0C955FC6E", "Digital_Signature" },
        {   "1806D474-CADF-4509-A4BA-9AABCB96AAE8", "Padding" },

        /// Header Extension Object GUIDs
        {   "14E6A5CB-C672-4332-8399-A96952065B5A", "Extended_Stream_Properties" },
        {   "A08649CF-4775-4670-8A16-6E35357566CD", "Advanced_Mutual_Exclusion" },
        {   "D1465A40-5A79-4338-B71B-E36B8FD6C249", "Group_Mutual_Exclusion" },
        {   "D4FED15B-88D3-454F-81F0-ED5C45999E24", "Stream_Prioritization" },
        {   "A69609E6-517B-11D2-B6AF-00C04FD908E9", "Bandwidth_Sharing" },
        {   "7C4346A9-EFE0-4BFC-B229-393EDE415C85", "Language_List" },
        {   "C5F8CBEA-5BAF-4877-8467-AA8C44FA4CCA", "Metadata" },
        {   "44231C94-9498-49D1-A141-1D134E457054", "Metadata_Library" },
        {   "D6E229DF-35DA-11D1-9034-00A0C90349BE", "Index_Parameters" },
        {   "6B203BAD-3F11-48E4-ACA8-D7613DE2CFA7", "Media_Index_Parameters" },
        {   "F55E496D-9797-4B5D-8C8B-604DFE9BFB24", "Timecode_Index_Parameters" },
        {   "26F18B5D-4584-47EC-9F5F-0E651F0452C9", "Compatibility" },
        {   "43058533-6981-49E6-9B74-AD12CB86D58C", "Advanced_Content_Encryption" },

        /// Stream Properties Object Stream Type GUIDs
        {   "F8699E40-5B4D-11CF-A8FD-00805F5C442B", "Audio_Media" },
        {   "BC19EFC0-5B4D-11CF-A8FD-00805F5C442B", "Video_Media" },
        {   "59DACFC0-59E6-11D0-A3AC-00A0C90348F6", "Command_Media" },
        {   "B61BE100-5B4E-11CF-A8FD-00805F5C442B", "JFIF_Media" },
        {   "35907DE0-E415-11CF-A917-00805F5C442B", "Degradable_JPEG_Media" },
        {   "91BD222C-F21C-497A-8B6D-5AA86BFC0185", "File_Transfer_Media" },
        {   "3AFB65E2-47EF-40F2-AC2C-70A90D71D343", "Binary_Media" },

        /// Web stream Type-Specific Data GUIDs
        {   "776257D4-C627-41CB-8F81-7AC7FF1C40CC", "Web_Stream_Media_Subtype" },
        {   "DA1E6B13-8359-4050-B398-388E965BF00C", "Web_Stream_Format" },

        /// Stream Properties Object Error Correction Type GUIDs
        {   "20FB5700-5B55-11CF-A8FD-00805F5C442B", "No_Error_Correction" },
        {   "BFC3CD50-618F-11CF-8BB2-00AA00B4E220", "Audio_Spread" },

        /// Header Extension Object GUIDs
        {   "ABD3D211-A9BA-11cf-8EE6-00C00C205365", "Reserved_1" },

        /// Advanced Content Encryption Object System ID GUIDs
        {   "7A079BB6-DAA4-4e12-A5CA-91D38DC11A8D", "Content_Encryption_System_Windows_Media_DRM_Network_Devices" },

        /// Codec List Object GUIDs
        {   "86D15241-311D-11D0-A3A4-00A0C90348F6", "Reserved_2" },

        /// Script Command Object GUIDs
        {   "4B1ACBE3-100B-11D0-A39B-00A0C90348F6", "Reserved_3" },

        /// Marker Object GUIDs
        {   "4CFEDB20-75F6-11CF-9C0F-00A0C90349CB", "Reserved_4" },

        /// Mutual Exclusion Object Exclusion Type GUIDs
        {   "D6E22A00-35DA-11D1-9034-00A0C90349BE", "Mutex_Language" },
        {   "D6E22A01-35DA-11D1-9034-00A0C90349BE", "Mutex_Bitrate" },
        {   "D6E22A02-35DA-11D1-9034-00A0C90349BE", "Mutex_Unknown" },

        /// Bandwidth Sharing Object GUIDs
        {   "AF6060AA-5197-11D2-B6AF-00C04FD908E9", "Bandwidth_Sharing_Exclusive" },
        {   "AF6060AB-5197-11D2-B6AF-00C04FD908E9", "Bandwidth_Sharing_Partial" },

        /// Standard Payload Extension System GUIDs
        {   "399595EC-8667-4E2D-8FDB-98814CE76C1E", "Payload_Extension_System_Timecode" },
        {   "E165EC0E-19ED-45D7-B4A7-25CBD1E28E9B", "Payload_Extension_System_File_Name" },
        {   "D590DC20-07BC-436C-9CF7-F3BBFBF1A4DC", "Payload_Extension_System_Content_Type" },
        {   "1B1EE554-F9EA-4BC8-821A-376B74E4C4B8", "Payload_Extension_System_Pixel_Aspect_Ratio" },
        {   "C6BD9450-867F-4907-83A3-C77921B733AD", "Payload_Extension_System_Sample_Duration" },
        {   "6698B84E-0AFA-4330-AEB2-1C0A98D7A44D", "Payload_Extension_System_Encryption_Sample_ID" },
        {   "00E1AF06-7BEC-11D1-A582-00C04FC29CFB", "Payload_Extension_System_Degradable_JPEG" }
    };

    //! Audio codec type-specific data in ASF
    extern const TagDetails audioCodec[] =  {
        {    0x161, "Windows Media Audio (7, 8, and 9 Series)" },
        {    0x162, "Windows Media Audio 9 Professional" },
        {    0x163, "Windows Media Audio 9 Lossless" },
        {   0x7A21, "GSM-AMR (CBR, no SID)" },
        {   0x7A22, "GSM-AMR (VBR including SID)" }
    };

    extern const TagDetails filePropertiesTags[] =  {
        {    7, "Xmp.video.FileLength" },
        {    6, "Xmp.video.CreationDate" },
        {    5, "Xmp.video.DataPackets" },
        {    4, "Xmp.video.Duration" },
        {    3, "Xmp.video.SendDuration" },
        {    2, "Xmp.video.Preroll" },
        {    1, "Xmp.video.MaxBitRate" }
    };

    extern const TagDetails contentDescriptionTags[] =  {
        {    0, "Xmp.video.Title" },
        {    1, "Xmp.video.Author" },
        {    2, "Xmp.video.Copyright" },
        {    3, "Xmp.video.Description" },
        {    4, "Xmp.video.Rating" }
    };

    /*!
      @brief Function used to read data from data buffer, reads 16-bit character
          array and stores it in std::string object.
      @param buf Exiv2 data buffer, which stores the information
      @return Returns std::string object .
     */
    std::string toString16(Exiv2::DataBuf& buf)
    {
        std::ostringstream os; char t;

        for(int i = 0; i <= buf.size_; i += 2 ) {
            t = buf.pData_[i] + 16 * buf.pData_[i + 1];
            if(t == 0) {
                if(i)
                    os << '\0';
                break;
            }
            os<< t;
        }
        return os.str();
    }

    /*!
      @brief Function used to check equality of two Tags (ignores case).
      @param str1 char* Pointer to First Tag
      @param str2 char* Pointer to Second Tag
      @return Returns true if both are equal.
     */
    bool compareTag(const char* str1, const char* str2) {
        if ( strlen(str1) != strlen(str2))
            return false;

        for ( uint64_t i = 0 ; i < strlen(str1); ++i )
            if (tolower(str1[i]) != tolower(str2[i]))
                return false;

        return true;
    }

    /*!
      @brief Function used to convert a decimal number to its Hexadecimal
          equivalent, then parsed into a character
      @param n Integer which is to be parsed as Hexadecimal character
      @return Return a Hexadecimal number, in character
     */
    char returnHEX(int n) {
        if(n >= 0 && n <= 9)
            return (char)(n + 48);
        else
            return (char)(n + 55);
    }

    /*!
      @brief Function used to calulate GUID, Tags comprises of 16 bytes.
          The Buffer contains the Tag in Binary Form. The information is then
          parsed into a character array GUID.
     */
    void getGUID (byte buf[], char GUID[]) {
        int i;
        for (i = 0; i < 4; ++i) {
            GUID[(3 - i) * 2]      = returnHEX(buf[i] / 0x10);
            GUID[(3 - i) * 2 + 1]  = returnHEX(buf[i] % 0x10);
        }
        for (i = 4; i < 6; ++i) {
            GUID[(9 - i) * 2 + 1]  = returnHEX(buf[i] / 0x10);
            GUID[(9 - i) * 2 + 2]  = returnHEX(buf[i] % 0x10);
        }
        for (i = 6; i < 8; ++i) {
            GUID[(14 - i) * 2]     = returnHEX(buf[i] / 0x10);
            GUID[(14 - i) * 2 + 1] = returnHEX(buf[i] % 0x10);
        }
        for (i = 8; i < 10; ++i) {
            GUID[ i * 2 + 3]       = returnHEX(buf[i] / 0x10);
            GUID[ i * 2 + 4]       = returnHEX(buf[i] % 0x10);
        }
        for (i = 10; i < 16; ++i) {
            GUID[ i * 2 + 4]       = returnHEX(buf[i] / 0x10);
            GUID[ i * 2 + 5]       = returnHEX(buf[i] % 0x10);
        }
        GUID[36] = '\0'; GUID[8] = GUID[13] = GUID[18] = GUID[23] = '-';
    }

    /*!
      @brief Function used to check if data stored in buf is equivalent to
          ASF Header Tag's GUID.
      @param buf Exiv2 byte buffer
      @return Returns true if the buffer data is equivalent to Header GUID.
     */
    bool isASFType (byte buf[]) {

        if(buf[0]  == 0x30   && buf[1]  == 0x26   && buf[2]  == 0xb2   && buf[3]  == 0x75 &&
           buf[4]  == 0x8e   && buf[5]  == 0x66   && buf[6]  == 0xcf   && buf[7]  == 0x11 &&
           buf[8]  == 0xa6   && buf[9]  == 0xd9   && buf[10] == 0x00   && buf[11] == 0xaa &&
           buf[12] == 0x00   && buf[13] == 0x62   && buf[14] == 0xce   && buf[15] == 0x6c )
                return true;

        return false;
    }

    //! Function used to convert buffer data into 64-bit Integer, information stored in littleEndian format
    uint64_t getUint64_t(Exiv2::DataBuf& buf) {
        uint64_t temp = 0;

        for(int i = 0; i < 8; ++i){
            temp = temp + static_cast<uint64_t>(buf.pData_[i]*(pow(static_cast<float>(256), i)));
        }
        return temp;
    }

}}                                      // namespace Internal, Exiv2

namespace Exiv2 {

    using namespace Exiv2::Internal;

    AsfVideo::AsfVideo(BasicIo::AutoPtr io)
        : Image(ImageType::asf, mdNone, io)
    {
    } // AsfVideo::AsfVideo

    std::string AsfVideo::mimeType() const
    {
        return "video/asf";
    }

    void AsfVideo::writeMetadata()
    {
    }

    void AsfVideo::readMetadata()
    {
        if (io_->open() != 0) throw Error(9, io_->path(), strError());

        // Ensure that this is the correct image type
        if (!isAsfType(*io_, false)) {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "ASF");
        }

        IoCloser closer(*io_);
        clearMetadata();
        continueTraversing_ = true;
        io_->seek(0, BasicIo::beg);
        height_ = width_ = 1;

        xmpData_["Xmp.video.FileSize"] = (double)io_->size()/(double)1048576;
        xmpData_["Xmp.video.FileName"] = io_->path();
        xmpData_["Xmp.video.MimeType"] = mimeType();

        while (continueTraversing_) decodeBlock();

        aspectRatio();
    } // AsfVideo::readMetadata

    void AsfVideo::decodeBlock()
    {
        const long bufMinSize = 9;
        DataBuf buf(bufMinSize);
        unsigned long size = 0;
        buf.pData_[8] = '\0' ;
        const TagVocabulary* tv;
        uint64_t cur_pos = io_->tell();

        byte guidBuf[16];
        io_->read(guidBuf, 16);

        if(io_->eof()) {
            continueTraversing_ = false;
            return;
        }

        char GUID[37] = ""; //the getGUID function write the GUID[36],

        getGUID(guidBuf, GUID);
        tv = find( GUIDReferenceTags, GUID);

        std::memset(buf.pData_, 0x0, buf.size_);
        io_->read(buf.pData_, 8);
        size = static_cast<unsigned long>(getUint64_t(buf));

        if(tv) {
            tagDecoder(tv,size-24);
        }
        else
            io_->seek(cur_pos + size, BasicIo::beg);

        localPosition_ = io_->tell();
    } // AsfVideo::decodeBlock

    void AsfVideo::tagDecoder(const TagVocabulary *tv, uint64_t size)
    {
        uint64_t cur_pos = io_->tell();
        DataBuf buf(1000);
        unsigned long count = 0, tempLength = 0;
        buf.pData_[4] = '\0' ;
        Exiv2::Value::AutoPtr v = Exiv2::Value::create(Exiv2::xmpSeq);

        if(compareTag( exvGettext(tv->label_), "Header")) {
            localPosition_ = 0;
            io_->read(buf.pData_, 4);
            io_->read(buf.pData_, 2);

            while(localPosition_ < cur_pos + size) decodeBlock();
        }

        else if(compareTag( exvGettext(tv->label_), "File_Properties"))
            fileProperties();

        else if(compareTag( exvGettext(tv->label_), "Stream_Properties"))
            streamProperties();

        else if(compareTag( exvGettext(tv->label_), "Metadata"))
            metadataHandler(1);

        else if(compareTag( exvGettext(tv->label_), "Extended_Content_Description"))
            metadataHandler(2);

        else if(compareTag( exvGettext(tv->label_), "Metadata_Library"))
            metadataHandler(3);

        else if(compareTag( exvGettext(tv->label_), "Codec_List"))
            codecList();

        else if(compareTag( exvGettext(tv->label_), "Content_Description"))
            contentDescription(size);

        else if(compareTag( exvGettext(tv->label_), "Extended_Stream_Properties"))
            extendedStreamProperties(size);

        else if(compareTag( exvGettext(tv->label_), "Header_Extension")) {
            localPosition_ = 0;
            headerExtension(size);
        }

        else if(compareTag( exvGettext(tv->label_), "Language_List")) {
            std::memset(buf.pData_, 0x0, buf.size_);
            io_->read(buf.pData_, 2);
            count = Exiv2::getUShort(buf.pData_, littleEndian);

            while(count--) {
                std::memset(buf.pData_, 0x0, buf.size_);
                io_->read(buf.pData_, 1);   tempLength = (int)buf.pData_[0];

                io_->read(buf.pData_, tempLength);
                v->read(toString16(buf));
            }
            xmpData_.add(Exiv2::XmpKey("Xmp.video.TrackLang"), v.get());
        }

        io_->seek(cur_pos + size, BasicIo::beg);
        localPosition_ = io_->tell();
    } // AsfVideo::tagDecoder

    void AsfVideo::extendedStreamProperties(uint64_t size)
    {
        uint64_t cur_pos = io_->tell(), avgTimePerFrame = 0;
        DataBuf buf(8);
        static int previousStream;
        io_->seek(cur_pos + 48, BasicIo::beg);

        std::memset(buf.pData_, 0x0, buf.size_);
        io_->read(buf.pData_, 2);
        streamNumber_ = Exiv2::getUShort(buf.pData_, littleEndian);

        io_->read(buf.pData_, 2);
        io_->read(buf.pData_, 8);
        avgTimePerFrame = getUint64_t(buf);

        if(previousStream < streamNumber_  &&  avgTimePerFrame != 0)
            xmpData_["Xmp.video.FrameRate"] = (double)10000000/(double)avgTimePerFrame;

        previousStream = streamNumber_;
        io_->seek(cur_pos + size, BasicIo::beg);
    } // AsfVideo::extendedStreamProperties

    void AsfVideo::contentDescription(uint64_t size)
    {
        const long pos = io_->tell();
        if (pos == -1) throw Error(14);
        long length[5];
        for (int i = 0 ; i < 5 ; ++i) {
            byte buf[2];
            io_->read(buf, 2);
            if (io_->error() || io_->eof()) throw Error(14);
            length[i] = getUShort(buf, littleEndian);
        }
        for (int i = 0 ; i < 5 ; ++i) {
            DataBuf buf(length[i]);
            std::memset(buf.pData_, 0x0, buf.size_);
            io_->read(buf.pData_, length[i]);
            if (io_->error() || io_->eof()) throw Error(14);
            const TagDetails* td = find(contentDescriptionTags, i);
            assert(td);
            std::string str((const char*)buf.pData_, length[i]);
            if (convertStringCharset(str, "UCS-2LE", "UTF-8")) {
                xmpData_[td->label_] = str;
            }
            else {
                xmpData_[td->label_] = toString16(buf);
            }
        }
        if (io_->seek(pos + size, BasicIo::beg)) throw Error(14);
    } // AsfVideo::contentDescription

    void AsfVideo::streamProperties()
    {
        DataBuf buf(20);
        buf.pData_[8] = '\0' ;
        byte guidBuf[16]; int stream = 0;
        io_->read(guidBuf, 16);
        char streamType[37] = "";
        Exiv2::RiffVideo *test = NULL;

        getGUID(guidBuf, streamType);
        const TagVocabulary* tv;
        tv = find( GUIDReferenceTags, streamType);
        io_->read(guidBuf, 16);

        if(compareTag( exvGettext(tv->label_), "Audio_Media"))
            stream = 1;
        else if(compareTag( exvGettext(tv->label_), "Video_Media"))
            stream = 2;

        io_->read(buf.pData_, 8);
        if(stream == 2)
            xmpData_["Xmp.video.TimeOffset"] = getUint64_t(buf);
        else if(stream == 1)
            xmpData_["Xmp.audio.TimeOffset"] = getUint64_t(buf);

        io_->read(buf.pData_, 8);
        std::memset(buf.pData_, 0x0, buf.size_);
        io_->read(buf.pData_, 1);
        streamNumber_ = (int)buf.pData_[0] & 127;

        io_->read(buf.pData_, 5);
        std::memset(buf.pData_, 0x0, buf.size_);
        io_->read(buf.pData_, 2);
        long temp = Exiv2::getUShort(buf.pData_, littleEndian);

        if(stream == 2) {
            xmpData_["Xmp.video.Width"] = temp;
            width_ = temp;
        }
        else if(stream == 1) {
            xmpData_["Xmp.audio.Codec"] = test->printAudioEncoding(temp);
        }

        io_->read(buf.pData_, 2);
        temp = Exiv2::getUShort(buf.pData_, littleEndian);
        if(stream == 1)
            xmpData_["Xmp.audio.ChannelType"] = temp;

        io_->read(buf.pData_, 4);
        temp = Exiv2::getULong(buf.pData_, littleEndian);

        if(stream == 2) {
            xmpData_["Xmp.video.Height"] = temp;
            height_ = temp;
        }
        else if(stream == 1) {
            xmpData_["Xmp.audio.SampleRate"] = temp;
        }
    } // AsfVideo::streamProperties

    void AsfVideo::codecList()
    {
        DataBuf buf(200);
        io_->read(buf.pData_, 16);
        std::memset(buf.pData_, 0x0, buf.size_);
        io_->read(buf.pData_, 4);
        int codecCount = Exiv2::getULong(buf.pData_, littleEndian), descLength = 0, codecType = 0;

        while(codecCount--) {
            std::memset(buf.pData_, 0x0, buf.size_);
            io_->read(buf.pData_, 2);
            codecType = Exiv2::getUShort(buf.pData_, littleEndian);

            io_->read(buf.pData_, 2);
            descLength = Exiv2::getUShort(buf.pData_, littleEndian) * 2;

            if (descLength < 0) {
            #ifndef SUPPRESS_WARNINGS
                    EXV_ERROR   << " Description found in this ASF file is not of valid size ."
                                << " Entries considered invalid. Not Processed.\n";
            #endif
            }
            else {
                io_->read(buf.pData_, descLength);
                if(codecType == 1)
                    xmpData_["Xmp.video.Codec"] = toString16(buf);
                else if(codecType == 2)
                    xmpData_["Xmp.audio.Compressor"] = toString16(buf);
            }

            std::memset(buf.pData_, 0x0, buf.size_);
            io_->read(buf.pData_, 2);
            descLength = Exiv2::getUShort(buf.pData_, littleEndian) * 2;

            if (descLength < 0) {
            #ifndef SUPPRESS_WARNINGS
                    EXV_ERROR   << " Description found in this ASF file is not of valid size ."
                                << " Entries considered invalid. Not Processed.\n";
            #endif
            }
            else {
                io_->read(buf.pData_, descLength);

                if(codecType == 1)
                    xmpData_["Xmp.video.CodecDescription"] = toString16(buf);
                else if(codecType == 2)
                    xmpData_["Xmp.audio.CodecDescription"] = toString16(buf);
            }

            std::memset(buf.pData_, 0x0, buf.size_);
            io_->read(buf.pData_, 2);
            descLength = Exiv2::getUShort(buf.pData_, littleEndian);

            if (descLength < 0) {
            #ifndef SUPPRESS_WARNINGS
                    EXV_ERROR   << " Description found in this ASF file is not of valid size ."
                                << " Entries considered invalid. Not Processed.\n";
            #endif
            }
            else {
                io_->read(buf.pData_, descLength);
            }
        }
    } // AsfVideo::codecList

    void AsfVideo::headerExtension(uint64_t size)
    {
        uint64_t cur_pos = io_->tell();
        DataBuf buf(20);
        io_->read(buf.pData_, 18);
        buf.pData_[4] = '\0' ;
        io_->read(buf.pData_, 4);

        while(localPosition_ < cur_pos + size) decodeBlock();

        io_->seek(cur_pos + size, BasicIo::beg);
    } // AsfVideo::headerExtension

    void AsfVideo::metadataHandler(int meta)
    {
        DataBuf buf(5000);
        io_->read(buf.pData_, 2);
        int recordCount = Exiv2::getUShort(buf.pData_, littleEndian), nameLength = 0, dataLength = 0, dataType = 0;
        Exiv2::Value::AutoPtr v = Exiv2::Value::create(Exiv2::xmpSeq);
        byte guidBuf[16];   char fileID[37] = "";

        while(recordCount--) {
            std::memset(buf.pData_, 0x0, buf.size_);

            if(meta == 1 || meta == 3) {
                io_->read(buf.pData_, 4);
                io_->read(buf.pData_, 2);
                nameLength = Exiv2::getUShort(buf.pData_, littleEndian);
                io_->read(buf.pData_, 2);
                dataType = Exiv2::getUShort(buf.pData_, littleEndian);
                io_->read(buf.pData_, 4);
                dataLength = Exiv2::getULong(buf.pData_, littleEndian);

                if (nameLength > 5000) {
#ifndef SUPPRESS_WARNINGS
                    EXV_ERROR << "Xmp.video.Metadata nameLength was found to be larger than 5000 "
                              << " entries considered invalid; not read.\n";
#endif
                    io_->seek(io_->tell() + nameLength, BasicIo::beg);
                }
            else
                io_->read(buf.pData_, nameLength);

                v->read(toString16(buf));
                if(dataType == 6) {
                    io_->read(guidBuf, 16);
                    getGUID(guidBuf, fileID);
                }
                else
                    // Sanity check with an "unreasonably" large number
                    if (dataLength > 5000) {
#ifndef SUPPRESS_WARNINGS
                        EXV_ERROR << "Xmp.video.Metadata dataLength was found to be larger than 5000 "
                                  << " entries considered invalid; not read.\n";
#endif
                        io_->seek(io_->tell() + dataLength, BasicIo::beg);
                    }
                else
                        io_->read(buf.pData_, dataLength);
            }

            else if(meta == 2) {
                io_->read(buf.pData_, 2);
                nameLength = Exiv2::getUShort(buf.pData_, littleEndian);

                if (nameLength > 5000) {
#ifndef SUPPRESS_WARNINGS
                    EXV_ERROR << "Xmp.video.Metadata nameLength was found to be larger than 5000 "
                              << " entries considered invalid; not read.\n";
#endif
                    io_->seek(io_->tell() + nameLength, BasicIo::beg);
                }
            else
                io_->read(buf.pData_, nameLength);

                v->read(toString16(buf));

                io_->read(buf.pData_, 2);
                dataType = Exiv2::getUShort(buf.pData_, littleEndian);

                io_->read(buf.pData_, 2);
                dataLength = Exiv2::getUShort(buf.pData_, littleEndian);

                // Sanity check with an "unreasonably" large number
                if (dataLength > 5000) {
#ifndef SUPPRESS_WARNINGS
                    EXV_ERROR << "Xmp.video.Metadata dataLength was found to be larger than 5000 "
                              << " entries considered invalid; not read.\n";
#endif
                    io_->seek(io_->tell() + dataLength, BasicIo::beg);
                }
            else
                io_->read(buf.pData_, dataLength);
            }

            if(dataType == 0) {                       // Unicode String
                v->read(toString16(buf));
            }
            else if(dataType == 2 || dataType == 5) { // 16-bit Unsigned Integer
                v->read( Exiv2::toString( Exiv2::getUShort(buf.pData_, littleEndian)));
            }
            else if(dataType == 3) {                  // 32-bit Unsigned Integer
                v->read( Exiv2::toString( Exiv2::getULong( buf.pData_, littleEndian)));
            }
            else if(dataType == 4) {                  // 64-bit Unsigned Integer
                v->read(Exiv2::toString(getUint64_t(buf)));
            }
            else if(dataType == 6) {                  // 128-bit GUID
                v->read(Exiv2::toString(fileID));
            }
            else {                                    // Byte array
                v->read( Exiv2::toString(buf.pData_));
            }
        }

        if(meta == 1) {
            xmpData_.add(Exiv2::XmpKey("Xmp.video.Metadata"), v.get());
        }
        else if(meta == 2) {
            xmpData_.add(Exiv2::XmpKey("Xmp.video.ExtendedContentDescription"), v.get());
        }
        else {
            xmpData_.add(Exiv2::XmpKey("Xmp.video.MetadataLibrary"), v.get());
        }
    } // AsfVideo::metadataHandler

    void AsfVideo::fileProperties()
    {
        DataBuf buf(9);
        buf.pData_[8] = '\0' ;

        byte guidBuf[16];
        io_->read(guidBuf, 16);
        char fileID[37] = ""; int count = 7;
        getGUID(guidBuf, fileID);
        xmpData_["Xmp.video.FileID"] = fileID;

        const TagDetails* td;

        while(count--) {
            td = find(filePropertiesTags , (count + 1));
            io_->read(buf.pData_, 8);

            if(count == 0) {
                buf.pData_[4] = '\0' ;
                io_->read(buf.pData_, 4); io_->read(buf.pData_, 4);
            }

            if(count == 3 || count == 2) {
                xmpData_[exvGettext(td->label_)] = getUint64_t(buf) / 10000;
            }
            else {
                xmpData_[exvGettext(td->label_)] = getUint64_t(buf);
            }
        }
    } // AsfVideo::fileProperties

    void AsfVideo::aspectRatio()
    {
        //TODO - Make a better unified method to handle all cases of Aspect Ratio

        double aspectRatio = (double)width_ / (double)height_;
        aspectRatio = floor(aspectRatio*10) / 10;
        xmpData_["Xmp.video.AspectRatio"] = aspectRatio;

        int aR = (int) ((aspectRatio*10.0)+0.1);

        switch  (aR) {
            case 13 : xmpData_["Xmp.video.AspectRatio"] = "4:3"     ; break;
            case 17 : xmpData_["Xmp.video.AspectRatio"] = "16:9"    ; break;
            case 10 : xmpData_["Xmp.video.AspectRatio"] = "1:1"     ; break;
            case 16 : xmpData_["Xmp.video.AspectRatio"] = "16:10"   ; break;
            case 22 : xmpData_["Xmp.video.AspectRatio"] = "2.21:1"  ; break;
            case 23 : xmpData_["Xmp.video.AspectRatio"] = "2.35:1"  ; break;
            case 12 : xmpData_["Xmp.video.AspectRatio"] = "5:4"     ; break;
            default : xmpData_["Xmp.video.AspectRatio"] = aspectRatio;break;
        }
    } // AsfVideo::aspectRatio


    Image::AutoPtr newAsfInstance(BasicIo::AutoPtr io, bool /*create*/)
    {
        Image::AutoPtr image(new AsfVideo(io));
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isAsfType(BasicIo& iIo, bool advance)
    {
        const int32_t len = 16;
        byte buf[len];
        iIo.read(buf, len);

        if (iIo.error() || iIo.eof()) {
            return false;
        }

        bool matched = isASFType(buf);
        if (!advance || !matched) {
            iIo.seek(0, BasicIo::beg);
        }

        return matched;
    }

}                                       // namespace Exiv2
#endif // EXV_ENABLE_VIDEO
