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
/*!
  @file    exivsimple.h
  @brief   Limited metadata dll for win32
  @version $Rev: 3777 $
  @author  Brad Schick (brad)
           <a href="mailto:brad@robotbattle.com">brad@robotbattle.com</a>
           Christian Kuster (kusti)
           <a href="mailto:christian@kusti.ch">christian@kusti.ch</a>
  @date    12-Nov-04, brad: created
 */
#ifndef EXIVSIMPLE_H_
#define EXIVSIMPLE_H_

// *****************************************************************************


#ifdef EXIVSIMPLE_EXPORTS
#define EXIVSIMPLE_API __declspec(dllexport)
#else
#define EXIVSIMPLE_API __declspec(dllimport)
#endif

DECLARE_HANDLE (HIMAGE);

#ifdef __cplusplus
extern "C"
{
#endif

// These types should match those in types.hpp. Copied here so that
// exiv2 headers are not needed.
// For all of the functions
// that take a type, passing invalidTypeId causes the type to be guessed.
// Guessing types is accurate for IPTC but not for EXIF.
enum DllTypeId { invalidTypeId, unsignedByte, asciiString, unsignedShort,
                unsignedLong, unsignedRational, invalid6, undefined,
                signedShort, signedLong, signedRational,
                string, isoDate, isoTime,
                lastTypeId };

typedef bool (CALLBACK* METAENUMPROC)(const char *key, const char *value, void *user);

EXIVSIMPLE_API HIMAGE OpenFileImage(const char *file);
EXIVSIMPLE_API HIMAGE OpenMemImage(const BYTE *data, unsigned int size);
EXIVSIMPLE_API void FreeImage(HIMAGE img);
EXIVSIMPLE_API int SaveImage(HIMAGE img);
EXIVSIMPLE_API int ImageSize(HIMAGE img);
EXIVSIMPLE_API int ImageData(HIMAGE img, BYTE *buffer, unsigned int size);
EXIVSIMPLE_API int ReadMeta(HIMAGE img, const char *key, char *buff, int buffsize);
EXIVSIMPLE_API int EnumMeta(HIMAGE img, METAENUMPROC proc, void *user);
EXIVSIMPLE_API int AddMeta(HIMAGE img, const char *key, const char *val, DllTypeId type);
EXIVSIMPLE_API int ModifyMeta(HIMAGE img, const char *key, const char *val, DllTypeId type);
EXIVSIMPLE_API int RemoveMeta(HIMAGE img, const char *key);
/*!
  @brief Set the Thumbnail

  @param img    Handle to the image
  @param buffer Pointer to the Thumbnail data (JPEG)
  @param size   Size of the thumbnail in bytes
*/
EXIVSIMPLE_API void SetThumbnail(HIMAGE img, const BYTE *buffer, unsigned int size);
/*!
  @brief Get the Thumbnail

  @param img    Handle to the image
  @param buffer Pointer where the thumbnaildata is written to (large enough!)
  @param size   Size of buffer

  @return size of the thumbnail, 0 if failed to read the thumbnail,
          (unsigned int)-1 if buffer is too small.
*/
EXIVSIMPLE_API unsigned int GetThumbnail(HIMAGE img, BYTE *buffer, unsigned int size);

#ifdef __cplusplus
}
#endif

#endif                                  // #ifndef EXIVSIMPLE_H_
