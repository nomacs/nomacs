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
  File:      matroskavideo.cpp
  Version:   $Rev: 3845 $
  Author(s): Abhinav Badola for GSoC 2012 (AB) <mail.abu.to@gmail.com>
  History:   18-Jun-12, AB: created
  Credits:   See header file
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: matroskavideo.cpp 3845 2015-06-07 16:29:06Z ahuggel $")

// *****************************************************************************
// included header files
#include "config.h"

#ifdef EXV_ENABLE_VIDEO
#include "matroskavideo.hpp"
#include "futils.hpp"
#include "basicio.hpp"
#include "tags.hpp"
#include "tags_int.hpp"

// + standard includes
#include <cmath>
#include <cassert>

// *****************************************************************************
// class member definitions
namespace Exiv2 {
    namespace Internal {

    //! List of composite tags. They are skipped and the child tags are read immediately
    uint64_t compositeTagsList[] = {
        0x0000, 0x000e, 0x000f, 0x0020, 0x0026, 0x002e, 0x0036,
        0x0037, 0x003b, 0x005b, 0x0060, 0x0061, 0x0068, 0x05b9,
        0x0dbb, 0x1034, 0x1035, 0x1854, 0x21a7, 0x2240, 0x23c0,
        0x2624, 0x27c8, 0x2911, 0x2924, 0x2944, 0x2d80, 0x3373,
        0x35a1, 0x3e5b, 0x3e7b,
        0x14d9b74, 0x254c367, 0x549a966, 0x654ae6b, 0x8538067,
        0x941a469, 0xa45dfa3, 0xb538667, 0xc53bb6b, 0xf43b675
    };

    //! List of tags which are ignored, i.e., tag and value won't be read
    uint64_t ignoredTagsList[] = {
        0x0021, 0x0023, 0x0033, 0x0071, 0x0077, 0x006c, 0x0067, 0x007b, 0x02f2, 0x02f3,
        0x1031, 0x1032, 0x13ab, 0x13ac, 0x15ee, 0x23a2, 0x23c6, 0x2e67, 0x33a4, 0x33c5,
        0x3446, 0x2de7, 0x2df8, 0x26bf, 0x28ca, 0x3384, 0x13b8, 0x037e, 0x0485, 0x18d7,
        0x0005, 0x0009, 0x0011, 0x0012, 0x0016, 0x0017, 0x0018, 0x0022, 0x0024, 0x0025,
        0x0027, 0x002b, 0x002f, 0x003f, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f, 0x006a,
        0x006b, 0x006e, 0x007a, 0x007d, 0x0255, 0x3eb5, 0x3ea5, 0x3d7b, 0x33c4, 0x2fab,
        0x2ebc, 0x29fc, 0x29a5, 0x2955, 0x2933, 0x135f, 0x2922, 0x26a5, 0x26fc, 0x2532,
        0x23c9, 0x23c4, 0x23c5, 0x137f, 0x1378, 0x07e2, 0x07e3, 0x07e4, 0x0675, 0x05bc,
        0x05bd, 0x05db, 0x05dd, 0x0598, 0x050d, 0x0444, 0x037c,

        0x3314f, 0x43a770, 0x1eb923, 0x1cb923, 0xeb524, 0x1c83ab, 0x1e83bb
    };

    /*!
      Tag Look-up list for Matroska Type Video Files
      The Tags have been categorized in 4 categories. Which are
      mentioned as a comment in front of them.
      s   --  Tag to be Skipped
      sd  --  Tag to be Skipped along with its data
      u   --  Tag used directly for storing metadata
      ui  --  Tag used only internally
     */
    extern const MatroskaTags matroskaTags[] = {
        {      0x0000, "ChapterDisplay" },                              //s
        {      0x0003, "TrackType" },                                   //ui
        {      0x0005, "ChapterString" },                               //sd
        {      0x0006, "VideoCodecID/AudioCodecID/CodecID" },           //ui
        {      0x0008, "TrackDefault" },                                //ui
        {      0x0009, "ChapterTrackNumber" },                          //sd
        {      0x000e, "Slices" },                                      //s
        {      0x000f, "ChapterTrack" },                                //s
        {      0x0011, "ChapterTimeStart" },                            //sd
        {      0x0012, "ChapterTimeEnd" },                              //sd
        {      0x0016, "CueRefTime" },                                  //sd
        {      0x0017, "CueRefCluster" },                               //sd
        {      0x0018, "ChapterFlagHidden" },                           //sd
        {      0x001a, "Xmp.video.VideoScanTpye" },                     //u
        {      0x001b, "BlockDuration" },                               //s
        {      0x001c, "TrackLacing" },                                 //ui
        {      0x001f, "Xmp.audio.ChannelType" },                       //u
        {      0x0020, "BlockGroup" },                                  //s
        {      0x0021, "Block" },                                       //sd
        {      0x0022, "BlockVirtual" },                                //sd
        {      0x0023, "SimpleBlock" },                                 //sd
        {      0x0024, "CodecState" },                                  //sd
        {      0x0025, "BlockAdditional" },                             //sd
        {      0x0026, "BlockMore" },                                   //s
        {      0x0027, "Position" },                                    //sd
        {      0x002a, "CodecDecodeAll" },                              //ui
        {      0x002b, "PrevSize" },                                    //sd
        {      0x002e, "TrackEntry" },                                  //s
        {      0x002f, "EncryptedBlock" },                              //sd
        {      0x0030, "Xmp.video.Width" },                             //u
        {      0x0033, "CueTime" },                                     //sd
        {      0x0035, "Xmp.audio.SampleRate" },                        //u
        {      0x0036, "ChapterAtom" },                                 //s
        {      0x0037, "CueTrackPositions" },                           //s
        {      0x0039, "TrackUsed" },                                   //ui
        {      0x003a, "Xmp.video.Height" },                            //u
        {      0x003b, "CuePoint" },                                    //s
        {      0x003f, "CRC-32" },                                      //sd
        {      0x004b, "BlockAdditionalID" },                           //sd
        {      0x004c, "LaceNumber" },                                  //sd
        {      0x004d, "FrameNumber" },                                 //sd
        {      0x004e, "Delay" },                                       //sd
        {      0x004f, "ClusterDuration" },                             //sd
        {      0x0057, "TrackNumber" },                                 //ui
        {      0x005b, "CueReference" },                                //s
        {      0x0060, "Video" },                                       //s
        {      0x0061, "Audio" },                                       //s
        {      0x0067, "Timecode" },                                    //sd
        {      0x0068, "TimeSlice" },                                   //s
        {      0x006a, "CueCodecState" },                               //sd
        {      0x006b, "CueRefCodecState" },                            //sd
        {      0x006c, "Void" },                                        //sd
        {      0x006e, "BlockAddID" },                                  //sd
        {      0x0071, "CueClusterPosition" },                          //sd
        {      0x0077, "CueTrack" },                                    //sd
        {      0x007a, "ReferencePriority" },                           //sd
        {      0x007b, "ReferenceBlock" },                              //sd
        {      0x007d, "ReferenceVirtual" },                            //sd
        {      0x0254, "Xmp.video.ContentCompressAlgo" },               //u
        {      0x0255, "ContentCompressionSettings" },                  //sd
        {      0x0282, "Xmp.video.DocType" },                           //u
        {      0x0285, "Xmp.video.DocTypeReadVersion" },                //u
        {      0x0286, "Xmp.video.EBMLVersion" },                       //u
        {      0x0287, "Xmp.video.DocTypeVersion" },                    //u
        {      0x02f2, "EBMLMaxIDLength" },                             //sd
        {      0x02f3, "EBMLMaxSizeLength" },                           //sd
        {      0x02f7, "Xmp.video.EBMLReadVersion" },                   //u
        {      0x037c, "ChapterLanguage" },                             //sd
        {      0x037e, "ChapterCountry" },                              //sd
        {      0x0444, "SegmentFamily" },                               //sd
        {      0x0461, "Xmp.video.DateUTC" }, //Date Time Original - measured in seconds relatively to Jan 01, 2001, 0:00:00 GMT+0h
        {      0x047a, "Xmp.video.TagLanguage" },                       //u
        {      0x0484, "Xmp.video.TagDefault" },                        //u
        {      0x0485, "TagBinary" },                                   //sd
        {      0x0487, "Xmp.video.TagString" },                         //u
        {      0x0489, "Xmp.video.Duration" },                          //u
        {      0x050d, "ChapterProcessPrivate" },                       //sd
        {      0x0598, "ChapterFlagEnabled" },                          //sd
        {      0x05a3, "Xmp.video.TagName" },                           //u
        {      0x05b9, "EditionEntry" },                                //s
        {      0x05bc, "EditionUID" },                                  //sd
        {      0x05bd, "EditionFlagHidden" },                           //sd
        {      0x05db, "EditionFlagDefault" },                          //sd
        {      0x05dd, "EditionFlagOrdered" },                          //sd
        {      0x065c, "Xmp.video.AttachFileData" },                    //u
        {      0x0660, "Xmp.video.AttachFileMIME" },                    //u
        {      0x066e, "Xmp.video.AttachFileName" },                    //u
        {      0x0675, "AttachedFileReferral" },                        //sd
        {      0x067e, "Xmp.video.AttachFileDesc" },                    //u
        {      0x06ae, "Xmp.video.AttachFileUID" },                     //u
        {      0x07e1, "Xmp.video.ContentEncryptAlgo" },                //u
        {      0x07e2, "ContentEncryptionKeyID" },                      //sd
        {      0x07e3, "ContentSignature" },                            //sd
        {      0x07e4, "ContentSignatureKeyID" },                       //sd
        {      0x07e5, "Xmp.video.ContentSignAlgo" },                   //u
        {      0x07e6, "Xmp.video.ContentSignHashAlgo" },               //u
        {      0x0d80, "Xmp.video.MuxingApp" },                         //u
        {      0x0dbb, "Seek" },                                        //s
        {      0x1031, "ContentEncodingOrder" },                        //sd
        {      0x1032, "ContentEncodingScope" },                        //sd
        {      0x1033, "Xmp.video.ContentEncodingType" },               //u
        {      0x1034, "ContentCompression" },                          //s
        {      0x1035, "ContentEncryption" },                           //s
        {      0x135f, "CueRefNumber" },                                //sd
        {      0x136e, "Xmp.video.TrackName" },                         //u
        {      0x1378, "CueBlockNumber" },                              //sd
        {      0x137f, "TrackOffset" },                                 //sd
        {      0x13ab, "SeekID" },                                      //sd
        {      0x13ac, "SeekPosition" },                                //sd
        {      0x13b8, "Stereo3DMode" },                                //sd
        {      0x14aa, "Xmp.video.CropBottom" },                        //ui
        {      0x14b0, "Xmp.video.Width" },                             //u
        {      0x14b2, "Xmp.video.DisplayUnit" },                       //u
        {      0x14b3, "Xmp.video.AspectRatioType" },                   //u
        {      0x14ba, "Xmp.video.Height" },                            //u
        {      0x14bb, "Xmp.video.CropTop" },                           //ui
        {      0x14cc, "Xmp.video.CropLeft" },                          //ui
        {      0x14dd, "Xmp.video.CropRight" },                         //ui
        {      0x15aa, "TrackForced" },                                 //ui
        {      0x15ee, "MaxBlockAdditionID" },                          //sd
        {      0x1741, "Xmp.video.WritingApp" },                        //u
        {      0x1854, "SilentTracks" },                                //s
        {      0x18d7, "SilentTrackNumber" },                           //sd
        {      0x21a7, "AttachedFile" },                                //s
        {      0x2240, "ContentEncoding" },                             //s
        {      0x2264, "Xmp.audio.BitsPerSample" },                     //u
        {      0x23a2, "CodecPrivate" },                                //sd
        {      0x23c0, "Targets" },                                     //s
        {      0x23c3, "Xmp.video.PhysicalEquivalent" },                //u
        {      0x23c4, "TagChapterUID" },                               //sd
        {      0x23c5, "TagTrackUID" },                                 //sd
        {      0x23c6, "TagAttachmentUID" },                            //sd
        {      0x23c9, "TagEditionUID" },                               //sd
        {      0x23ca, "Xmp.video.TargetType" },                        //u
        {      0x2532, "SignedElement" },                               //sd
        {      0x2624, "TrackTranslate" },                              //s
        {      0x26a5, "TrackTranslateTrackID" },                       //sd
        {      0x26bf, "TrackTranslateCodec" },                         //sd
        {      0x26fc, "TrackTranslateEditionUID" },                    //sd
        {      0x27c8, "SimpleTag" },                                   //s
        {      0x28ca, "TargetTypeValue" },                             //sd
        {      0x2911, "ChapterProcessCommand" },                       //s
        {      0x2922, "ChapterProcessTime" },                          //sd
        {      0x2924, "ChapterTranslate" },                            //s
        {      0x2933, "ChapterProcessData" },                          //sd
        {      0x2944, "ChapterProcess" },                              //s
        {      0x2955, "ChapterProcessCodecID" },                       //sd
        {      0x29a5, "ChapterTranslateID" },                          //sd
        {      0x29bf, "Xmp.video.TranslateCodec" },                    //u
        {      0x29fc, "ChapterTranslateEditionUID" },                  //sd
        {      0x2d80, "ContentEncodings" },                            //s
        {      0x2de7, "MinCache" },                                    //sd
        {      0x2df8, "MaxCache" },                                    //sd
        {      0x2e67, "ChapterSegmentUID" },                           //sd
        {      0x2ebc, "ChapterSegmentEditionUID" },                    //sd
        {      0x2fab, "TrackOverlay" },                                //sd
        {      0x3373, "Tag" },                                         //s
        {      0x3384, "SegmentFileName" },                             //sd
        {      0x33a4, "SegmentUID" },                                  //sd
        {      0x33c4, "ChapterUID" },                                  //sd
        {      0x33c5, "TrackUID" },                                    //sd
        {      0x3446, "TrackAttachmentUID" },                          //sd
        {      0x35a1, "BlockAdditions" },                              //s
        {      0x38b5, "Xmp.audio.OutputSampleRate" },                  //u
        {      0x3ba9, "Xmp.video.Title" },                             //u
        {      0x3d7b, "ChannelPositions" },                            //sd
        {      0x3e5b, "SignatureElements" },                           //s
        {      0x3e7b, "SignatureElementList" },                        //s
        {      0x3e8a, "Xmp.video.ContentSignAlgo" },                   //u
        {      0x3e9a, "Xmp.video.ContentSignHashAlgo" },               //u
        {      0x3ea5, "SignaturePublicKey" },                          //sd
        {      0x3eb5, "Signature" },                                   //sd
        {     0x2b59c, "TrackLanguage" },                               //ui
        {     0x3314f, "TrackTimecodeScale" },                          //sd
        {     0x383e3, "Xmp.video.FrameRate" },                         //u
        {     0x3e383, "VideoFrameRate/DefaultDuration" },              //ui
        {     0x58688, "VideoCodecName/AudioCodecName/CodecName" },     //ui
        {     0x6b240, "CodecDownloadURL" },                            //ui
        {     0xad7b1, "TimecodeScale" },                               //ui
        {     0xeb524, "ColorSpace" },                                  //sd
        {     0xfb523, "Xmp.video.OpColor" },                           //u
        {    0x1a9697, "CodecSettings" },                               //ui
        {    0x1b4040, "CodecInfoURL" },                                //ui
        {    0x1c83ab, "PrevFileName" },                                //sd
        {    0x1cb923, "PrevUID" },                                     //sd
        {    0x1e83bb, "NextFileName" },                                //sd
        {    0x1eb923, "NextUID" },                                     //sd
        {    0x43a770, "Chapters" },                                    //sd
        {   0x14d9b74, "SeekHead" },                                    //s
        {   0x254c367, "Tags" },                                        //s
        {   0x549a966, "Info" },                                        //s
        {   0x654ae6b, "Tracks" },                                      //s
        {   0x8538067, "SegmentHeader" },                               //s
        {   0x941a469, "Attachments" },                                 //s
        {   0xa45dfa3, "EBMLHeader" },                                  //s
        {   0xb538667, "SignatureSlot" },                               //s
        {   0xc53bb6b, "Cues" },                                        //s
        {   0xf43b675, "Cluster" },                                     //s
    };

    extern const MatroskaTags matroskaTrackType[] =  {
        {   0x1,  "Video" },
        {   0x2,  "Audio" },
        {   0x3,  "Complex" },
        {   0x10, "Logo" },
        {   0x11, "Subtitle" },
        {   0x12, "Buttons" },
        {   0x20, "Control" }
    };

    extern const MatroskaTags compressionAlgorithm[] =  {
        {   0, "zlib " },
        {   1, "bzlib" },
        {   2, "lzo1x" },
        {   3, "Header Stripping" }
    };

    extern const MatroskaTags audioChannels[] =  {
        {   1, "Mono" },
        {   2, "Stereo" },
        {   5, "5.1 Surround Sound" },
        {   7, "7.1 Surround Sound" }
    };

    extern const MatroskaTags displayUnit[] =  {
        {   0x0,  "Pixels" },
        {   0x1,  "cm" },
        {   0x2,  "inches" }
    };

    extern const MatroskaTags encryptionAlgorithm[] =  {
        {   0, "Not Encrypted" },
        {   1, "DES" },
        {   2, "3DES" },
        {   3, "Twofish" },
        {   4, "Blowfish" },
        {   5, "AES" }
    };

    extern const MatroskaTags chapterPhysicalEquivalent[] =  {
        {   10, "Index" },
        {   20, "Track" },
        {   30, "Session" },
        {   40, "Layer" },
        {   50, "Side" },
        {   60, "CD / DVD" },
        {   70, "Set / Package" },
    };

    extern const MatroskaTags encodingType[] =  {
        {   0, "Compression" },
        {   1, "Encryption" }
    };

    extern const MatroskaTags videoScanType[] =  {
        {   0, "Progressive" },
        {   1, "Interlaced" }
    };

    extern const MatroskaTags chapterTranslateCodec[] =  {
        {   0, "Matroska Script" },
        {   1, "DVD Menu" }
    };

    extern const MatroskaTags aspectRatioType[] =  {
        {   0, "Free Resizing" },
        {   1, "Keep Aspect Ratio" },
        {   2, "Fixed" }
    };

    extern const MatroskaTags contentSignatureAlgorithm[] =  {
        {   0, "Not Signed" },
        {   1, "RSA" }
    };

    extern const MatroskaTags contentSignatureHashAlgorithm[] =  {
        {   0, "Not Signed" },
        {   1, "SHA1-160" },
        {   2, "MD5" }
    };

    extern const MatroskaTags trackEnable[] =  {
        {   0x1,  "Xmp.video.Enabled" },
        {   0x2,  "Xmp.audio.Enabled" },
        {   0x11, "Xmp.video.SubTEnabled" }
    };

    extern const MatroskaTags defaultOn[] =  {
        {   0x1,  "Xmp.video.DefaultOn" },
        {   0x2,  "Xmp.audio.DefaultOn" },
        {   0x11, "Xmp.video.SubTDefaultOn" }
    };

    extern const MatroskaTags trackForced[] =  {
        {   0x1,  "Xmp.video.TrackForced" },
        {   0x2,  "Xmp.audio.TrackForced" },
        {   0x11, "Xmp.video.SubTTrackForced" }
    };

    extern const MatroskaTags trackLacing[] =  {
        {   0x1,  "Xmp.video.TrackLacing" },
        {   0x2,  "Xmp.audio.TrackLacing" },
        {   0x11, "Xmp.video.SubTTrackLacing" }
    };

    extern const MatroskaTags codecDecodeAll[] =  {
        {   0x1,  "Xmp.video.CodecDecodeAll" },
        {   0x2,  "Xmp.audio.CodecDecodeAll" },
        {   0x11, "Xmp.video.SubTCodecDecodeAll" }
    };

    extern const MatroskaTags codecDownloadUrl[] =  {
        {   0x1,  "Xmp.video.CodecDownloadUrl" },
        {   0x2,  "Xmp.audio.CodecDownloadUrl" },
        {   0x11, "Xmp.video.SubTCodecDownloadUrl" }
    };

    extern const MatroskaTags codecSettings[] =  {
        {   0x1,  "Xmp.video.CodecSettings" },
        {   0x2,  "Xmp.audio.CodecSettings" },
        {   0x11, "Xmp.video.SubTCodecSettings" }
    };

    extern const MatroskaTags trackCodec[] =  {
        {   0x1,  "Xmp.video.Codec" },
        {   0x2,  "Xmp.audio.Compressor" },
        {   0x11, "Xmp.video.SubTCodec" }
    };

    extern const MatroskaTags trackLanguage[] =  {
        {   0x1,  "Xmp.video.TrackLang" },
        {   0x2,  "Xmp.audio.TrackLang" },
        {   0x11, "Xmp.video.SubTLang" }
    };

    extern const MatroskaTags codecInfo[] =  {
        {   0x1,  "Xmp.video.CodecInfo" },
        {   0x2,  "Xmp.audio.CodecInfo" },
        {   0x11, "Xmp.video.SubTCodecInfo" }
    };

    extern const MatroskaTags streamRate[] =  {
        {   0x1,  "Xmp.video.FrameRate" },
        {   0x2,  "Xmp.audio.DefaultDuration" }
    };

    /*!
      @brief Function used to calculate Tags, Tags may comprise of more than
          one byte. The first byte calculates size of the Tag and the remaining
          bytes are used to calculate the rest of the Tag.
          Returns Tag Value.
     */
    uint64_t returnTagValue(const byte* buf, long size)
    {
        assert(size > 0 && size <= 8);

        uint64_t b0 = buf[0] & (0xff >> size);
        uint64_t tag = b0 << ((size - 1) * 8);
        for (long i = 1; i < size; ++i) {
            tag |= static_cast<uint64_t>(buf[i]) << ((size - i - 1) * 8);
        }

        return tag;
    }

    /*!
        @brief Function used to convert buffer data into numerical information,
            information stored in BigEndian format
     */
    int64_t returnValue(const byte* buf, long size)
    {

        int64_t temp = 0;

        for(int i = size-1; i >= 0; i--) {
            temp = temp + static_cast<int64_t>(buf[i]*(pow(256.0, (double)size-i-1)));
        }
// Todo: remove debug output
//        std::cerr << "size = " << size << ", val = " << temp << std::hex << " (0x" << temp << std::dec << ")";

        uint64_t ret = 0;
        for (long i = 0; i < size; ++i) {
            ret |= static_cast<uint64_t>(buf[i]) << ((size - i - 1) * 8);
        }

// Todo: remove debug output
//        std::cerr << ", ret = " << ret << std::hex << " (0x" << ret << std::dec << ")\n";

        return ret;
    }

}}                                      // namespace Internal, Exiv2

namespace Exiv2 {

    using namespace Exiv2::Internal;

    MatroskaVideo::MatroskaVideo(BasicIo::AutoPtr io)
        : Image(ImageType::mkv, mdNone, io)
    {
    } // MatroskaVideo::MatroskaVideo

    std::string MatroskaVideo::mimeType() const
    {
        return "video/matroska";
    }

    void MatroskaVideo::writeMetadata()
    {
    }

    void MatroskaVideo::readMetadata()
    {
        if (io_->open() != 0) throw Error(9, io_->path(), strError());

        // Ensure that this is the correct image type
        if (!isMkvType(*io_, false)) {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "Matroska");
        }

        IoCloser closer(*io_);
        clearMetadata();
        continueTraversing_ = true;
        height_ = width_ = 1;

        xmpData_["Xmp.video.FileName"] = io_->path();
        xmpData_["Xmp.video.FileSize"] = (double)io_->size()/(double)1048576;
        xmpData_["Xmp.video.MimeType"] = mimeType();

        while (continueTraversing_) decodeBlock();

        aspectRatio();
    } // MatroskaVideo::readMetadata

    void MatroskaVideo::decodeBlock()
    {
        byte buf[8];
        io_->read(buf, 1);

        if(io_->eof()) {
            continueTraversing_ = false;
            return;
        }

        uint32_t sz = findBlockSize(buf[0]); // 0-8
        if (sz > 0) io_->read(buf + 1, sz - 1);

        const MatroskaTags* mt = find(matroskaTags, returnTagValue(buf, sz));
        if ( !mt ) {
            continueTraversing_ = false;
            return;
        }

        if(mt->val_ == 0xc53bb6b || mt->val_ == 0xf43b675) {
            continueTraversing_ = false;
            return;
        }

        bool skip = find(compositeTagsList, mt->val_) != 0;
        bool ignore = find(ignoredTagsList, mt->val_) != 0;

        io_->read(buf, 1);
        sz = findBlockSize(buf[0]); // 0-8

        if (sz > 0) io_->read(buf + 1, sz - 1);
        uint64_t size = returnTagValue(buf, sz);

        if (skip && !ignore) return;

        const uint64_t bufMinSize = 200;

#ifndef SUPPRESS_WARNINGS
        if (!ignore && size > bufMinSize) {
            EXV_WARNING << "Size " << size << " of Matroska tag 0x"
                        << std::hex << mt->val_ << std::dec
                        << " is greater than " << bufMinSize << ": ignoring it.\n";
        }
#endif
        if (ignore || size > bufMinSize) {
            io_->seek(size, BasicIo::cur);
            return;
        }

        DataBuf buf2(bufMinSize+1);
        std::memset(buf2.pData_, 0x0, buf2.size_);
        long s = static_cast<long>(size) ;
        io_->read(buf2.pData_,s);
        contentManagement(mt, buf2.pData_,s);
    } // MatroskaVideo::decodeBlock

    void MatroskaVideo::contentManagement(const MatroskaTags* mt, const byte* buf, long size)
    {
        int64_t duration_in_ms = 0;
        static double time_code_scale = 1.0, temp = 0;
        static long stream = 0, track_count = 0;
        char str[4] = "No";
        const MatroskaTags* internalMt = 0;

        switch (mt->val_) {

        case 0x0282: case 0x0d80: case 0x1741: case 0x3ba9: case 0x066e: case 0x0660:
        case 0x065c: case 0x067e: case 0x047a: case 0x0487: case 0x05a3: case 0x136e:
        case 0x23ca: case 0xeb524:
            xmpData_[mt->label_] = buf;
            break;

        case 0x0030: case 0x003a: case 0x0287: case 0x14b0: case 0x14ba: case 0x285:
        case 0x06ae: case 0x0286: case 0x02f7: case 0x2264: case 0x14aa: case 0x14bb:
        case 0x14cc: case 0x14dd:
            xmpData_[mt->label_] = returnValue(buf, size);

            if (mt->val_ == 0x0030 || mt->val_ == 0x14b0) {
                width_ = returnValue(buf, size);
            }
            else if (mt->val_ == 0x003a || mt->val_ == 0x14ba) {
                height_ = returnValue(buf, size);
            }
            break;

        case 0x001a: case 0x001f: case 0x0254: case 0x07e1: case 0x07e5: case 0x07e6:
        case 0x1033: case 0x14b2: case 0x14b3: case 0x23c3: case 0x29bf: case 0x3e8a:
        case 0x3e9a:
            switch (mt->val_) {
            case 0x001a: internalMt = find(videoScanType, returnValue(buf, size));                 break;
            case 0x001f: internalMt = find(audioChannels, returnValue(buf, size));                 break;
            case 0x0254: internalMt = find(compressionAlgorithm, returnValue(buf, size));          break;
            case 0x07e1: internalMt = find(encryptionAlgorithm, returnValue(buf, size));           break;
            case 0x1033: internalMt = find(encodingType, returnValue(buf, size));                  break;
            case 0x3e8a:
            case 0x07e5: internalMt = find(contentSignatureAlgorithm, returnValue(buf, size));     break;
            case 0x3e9a:
            case 0x07e6: internalMt = find(contentSignatureHashAlgorithm, returnValue(buf, size)); break;
            case 0x14b2: internalMt = find(displayUnit, returnValue(buf, size));                   break;
            case 0x14b3: internalMt = find(aspectRatioType, returnValue(buf, size));               break;
            case 0x23c3: internalMt = find(chapterPhysicalEquivalent, returnValue(buf, size));     break;
            case 0x29bf: internalMt = find(chapterTranslateCodec, returnValue(buf, size));         break;
            }
            if (internalMt) xmpData_[mt->label_] = internalMt->label_;
            break;

        case 0x0035: case 0x38b5:
            xmpData_[mt->label_] = getFloat(buf, bigEndian);
            break;

        case 0x0039: case 0x0008: case 0x15aa: case 0x001c: case 0x002a: case 0x1a9697:
        case 0x0484:
            if (returnValue(buf, size)) strcpy(str, "Yes");
            switch (mt->val_) {
            case 0x0039: internalMt = find(trackEnable, stream);     break;
            case 0x0008: internalMt = find(defaultOn, stream);       break;
            case 0x15aa: internalMt = find(trackForced, stream);     break;
            case 0x001c: internalMt = find(trackLacing, stream);     break;
            case 0x002a: internalMt = find(codecDecodeAll, stream);  break;
            case 0x1a9697: internalMt = find(codecSettings, stream); break;
            case 0x0484: internalMt = mt; break;
            }
            if (internalMt) xmpData_[internalMt->label_] = str;
            break;

        case 0x0006: case 0x2b59c: case 0x58688: case 0x6b240: case 0x1b4040:
            switch (mt->val_) {
            case 0x0006: internalMt = find(trackCodec, stream);         break;
            case 0x2b59c: internalMt = find(trackLanguage, stream);     break;
            case 0x58688: internalMt = find(codecInfo, stream);         break;
            case 0x6b240:
            case 0x1b4040: internalMt = find(codecDownloadUrl, stream); break;
            }
            if (internalMt) xmpData_[internalMt->label_] = buf;
            break;

        case 0x0489: case 0x0461:
            switch (mt->val_) {
            case 0x0489:
                if(size <= 4) {
                    duration_in_ms = static_cast<int64_t>(getFloat(buf, bigEndian) * time_code_scale * 1000);
                }
                else {
                    duration_in_ms = static_cast<int64_t>(getDouble(buf, bigEndian) * time_code_scale * 1000);
                }
                break;
            case 0x0461: {
                duration_in_ms = returnValue(buf, size)/1000000000; break;
            }
            }
            xmpData_[mt->label_] = duration_in_ms;
            break;

        case 0x0057:
            track_count++;
            xmpData_["Xmp.video.TotalStream"] = track_count;
            break;

        case 0xad7b1:
            time_code_scale = (double)returnValue(buf, size)/(double)1000000000;
            xmpData_["Xmp.video.TimecodeScale"] = time_code_scale;
            break;

        case 0x0003:
            internalMt = find(matroskaTrackType, returnValue(buf, size));
            stream = static_cast<long>(internalMt->val_);
            break;

        case 0x3e383: case 0x383e3:
            internalMt = find(streamRate, stream);
            if (returnValue(buf, size)) {
                switch (stream) {
                case 1: temp = (double)1000000000/(double)returnValue(buf, size); break;
                case 2: temp = static_cast<double>(returnValue(buf, size) / 1000); break;
                }
                if (internalMt) xmpData_[internalMt->label_] = temp;
            }
            else
                if (internalMt) xmpData_[internalMt->label_] = "Variable Bit Rate";
            break;

        default:
            break;
        }
    } // MatroskaVideo::contentManagement

    void MatroskaVideo::aspectRatio()
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
    } // MatroskaVideo::aspectRatio

    uint32_t MatroskaVideo::findBlockSize(byte b)
    {
        if      (b & 128) return 1;
        else if (b &  64) return 2;
        else if (b &  32) return 3;
        else if (b &  16) return 4;
        else if (b &   8) return 5;
        else if (b &   4) return 6;
        else if (b &   2) return 7;
        else if (b &   1) return 8;
        else              return 0;
    } // MatroskaVideo::findBlockSize

    Image::AutoPtr newMkvInstance(BasicIo::AutoPtr io, bool /*create*/)
    {
        Image::AutoPtr image(new MatroskaVideo(io));
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isMkvType(BasicIo& iIo, bool advance)
    {
        bool result = true;
        byte tmpBuf[4];
        iIo.read(tmpBuf, 4);

        if (iIo.error() || iIo.eof()) return false;

        if (0x1a != tmpBuf[0] || 0x45 != tmpBuf[1] || 0xdf != tmpBuf[2] || 0xa3 != tmpBuf[3]) {
            result = false;
        }

        if (!advance || !result ) iIo.seek(0, BasicIo::beg);
        return result;
    }

}                                       // namespace Exiv2
#endif // EXV_ENABLE_VIDEO
