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
/*!
  @file    types.hpp
  @brief   Type definitions for %Exiv2 and related functionality
  @version $Rev: 2681 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    09-Jan-04, ahu: created<BR>
           11-Feb-04, ahu: isolated as a component<BR>
           31-Jul-04, brad: added Time, Data and String values
 */
#ifndef TYPES_HPP_
#define TYPES_HPP_

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "version.hpp"

// + standard includes
#include <string>
#include <vector>
#include <iosfwd>
#include <utility>
#include <algorithm>
#include <sstream>

#ifdef EXV_HAVE_STDINT_H
# include <stdint.h>
#elif defined(_MSC_VER)
// MSVC (before 2010) doesn't provide C99 types, but it has MS specific variants
typedef unsigned __int8  uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
typedef __int8           int8_t;
typedef __int16          int16_t;
typedef __int32          int32_t;
typedef __int64          int64_t;
#endif

// MSVC macro to convert a string to a wide string
#ifdef EXV_UNICODE_PATH
# define EXV_WIDEN(t) L ## t
#endif

/*!
  @brief Macro to make calls to member functions through a pointer more readable.
         See the C++ FAQ LITE, item
         <a href="http://www.parashift.com/c++-faq-lite/pointers-to-members.html#faq-33.5" title="[33.5] How can I avoid syntax errors when calling a member function using a pointer-to-member-function?">[33.5] How can I avoid syntax errors when calling a member function using a pointer-to-member-function?</a>.
 */
#define EXV_CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

// Simple min and max macros
//! Simple common min macro
#define EXV_MIN(a,b) ((a) < (b) ? (a) : (b))
//! Simple common max macro
#define EXV_MAX(a,b) ((a) > (b) ? (a) : (b))

// *****************************************************************************
// forward declarations
struct tm;

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// type definitions

    //! 1 byte unsigned integer type.
    typedef uint8_t byte;

    //! 8 byte unsigned rational type.
    typedef std::pair<uint32_t, uint32_t> URational;
    //! 8 byte signed rational type.
    typedef std::pair<int32_t, int32_t> Rational;

    //! Type to express the byte order (little or big endian)
    enum ByteOrder { invalidByteOrder, littleEndian, bigEndian };

    //! Type to indicate write method used by TIFF parsers
    enum WriteMethod { wmIntrusive, wmNonIntrusive };

    //! An identifier for each type of metadata
    enum MetadataId { mdNone=0, mdExif=1, mdIptc=2, mdComment=4, mdXmp=8 };

    //! An identifier for each mode of metadata support
    enum AccessMode { amNone=0, amRead=1, amWrite=2, amReadWrite=3 };

    /*!
      @brief %Exiv2 value type identifiers.

      Used primarily as identifiers when creating %Exiv2 Value instances.
      See Value::create. 0x0000 to 0xffff are reserved for TIFF (Exif) types.
     */
    enum TypeId {
        unsignedByte       = 1, //!< Exif BYTE type, 8-bit unsigned integer.
        asciiString        = 2, //!< Exif ASCII type, 8-bit byte.
        unsignedShort      = 3, //!< Exif SHORT type, 16-bit (2-byte) unsigned integer.
        unsignedLong       = 4, //!< Exif LONG type, 32-bit (4-byte) unsigned integer.
        unsignedRational   = 5, //!< Exif RATIONAL type, two LONGs: numerator and denumerator of a fraction.
        signedByte         = 6, //!< Exif SBYTE type, an 8-bit signed (twos-complement) integer.
        undefined          = 7, //!< Exif UNDEFINED type, an 8-bit byte that may contain anything.
        signedShort        = 8, //!< Exif SSHORT type, a 16-bit (2-byte) signed (twos-complement) integer.
        signedLong         = 9, //!< Exif SLONG type, a 32-bit (4-byte) signed (twos-complement) integer.
        signedRational     =10, //!< Exif SRATIONAL type, two SLONGs: numerator and denumerator of a fraction.
        tiffFloat          =11, //!< TIFF FLOAT type, single precision (4-byte) IEEE format.
        tiffDouble         =12, //!< TIFF DOUBLE type, double precision (8-byte) IEEE format.
        tiffIfd            =13, //!< TIFF IFD type, 32-bit (4-byte) unsigned integer.
        string        =0x10000, //!< IPTC string type.
        date          =0x10001, //!< IPTC date type.
        time          =0x10002, //!< IPTC time type.
        comment       =0x10003, //!< %Exiv2 type for the Exif user comment.
        directory     =0x10004, //!< %Exiv2 type for a CIFF directory.
        xmpText       =0x10005, //!< XMP text type.
        xmpAlt        =0x10006, //!< XMP alternative type.
        xmpBag        =0x10007, //!< XMP bag type.
        xmpSeq        =0x10008, //!< XMP sequence type.
        langAlt       =0x10009, //!< XMP language alternative type.
        invalidTypeId =0x1fffe, //!< Invalid type id.
        lastTypeId    =0x1ffff  //!< Last type id.
    };

    //! Container for binary data
    typedef std::vector<byte> Blob;

// *****************************************************************************
// class definitions

    //! Type information lookup functions. Implemented as a static class.
    class EXIV2API TypeInfo {
        //! Prevent construction: not implemented.
        TypeInfo();
        //! Prevent copy-construction: not implemented.
        TypeInfo(const TypeInfo& rhs);
        //! Prevent assignment: not implemented.
        TypeInfo& operator=(const TypeInfo& rhs);

    public:
        //! Return the name of the type, 0 if unknown.
        static const char* typeName(TypeId typeId);
        //! Return the type id for a type name
        static TypeId typeId(const std::string& typeName);
        //! Return the size in bytes of one element of this type
        static long typeSize(TypeId typeId);

    };

    /*!
      @brief Auxiliary type to enable copies and assignments, similar to
             std::auto_ptr_ref. See http://www.josuttis.com/libbook/auto_ptr.html
             for a discussion.
     */
    struct EXIV2API DataBufRef {
        //! Constructor
        DataBufRef(std::pair<byte*, long> rhs) : p(rhs) {}
        //! Pointer to a byte array and its size
        std::pair<byte*, long> p;
    };

    /*!
      @brief Utility class containing a character array. All it does is to take
             care of memory allocation and deletion. Its primary use is meant to
             be as a stack variable in functions that need a temporary data
             buffer.
     */
    class EXIV2API DataBuf {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        DataBuf() : pData_(0), size_(0) {}
        //! Constructor with an initial buffer size
        explicit DataBuf(long size) : pData_(new byte[size]), size_(size) {}
        //! Constructor, copies an existing buffer
        DataBuf(const byte* pData, long size);
        /*!
          @brief Copy constructor. Transfers the buffer to the newly created
                 object similar to std::auto_ptr, i.e., the original object is
                 modified.
         */
        DataBuf(DataBuf& rhs);
        //! Destructor, deletes the allocated buffer
        ~DataBuf() { delete[] pData_; }
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Assignment operator. Transfers the buffer and releases the
                 buffer at the original object similar to std::auto_ptr, i.e.,
                 the original object is modified.
         */
        DataBuf& operator=(DataBuf& rhs);
        /*!
          @brief Allocate a data buffer of at least the given size. Note that if
                 the requested \em size is less than the current buffer size, no
                 new memory is allocated and the buffer size doesn't change.
         */
        void alloc(long size);
        /*!
          @brief Release ownership of the buffer to the caller. Returns the
                 buffer as a data pointer and size pair, resets the internal
                 buffer.
         */
        std::pair<byte*, long> release();
        //! Reset value
        void reset(std::pair<byte*, long> =std::make_pair((byte*)(0),long(0)));
        //@}

        /*!
          @name Conversions

          Special conversions with auxiliary type to enable copies
          and assignments, similar to those used for std::auto_ptr.
          See http://www.josuttis.com/libbook/auto_ptr.html for a discussion.
         */
        //@{
        DataBuf(DataBufRef rhs) : pData_(rhs.p.first), size_(rhs.p.second) {}
        DataBuf& operator=(DataBufRef rhs) { reset(rhs.p); return *this; }
        operator DataBufRef() { return DataBufRef(release()); }
        //@}

        // DATA
        //! Pointer to the buffer, 0 if none has been allocated
        byte* pData_;
        //! The current size of the buffer
        long size_;
    }; // class DataBuf


// *****************************************************************************
// free functions

    //! Read a 2 byte unsigned short value from the data buffer
    EXIV2API uint16_t getUShort(const byte* buf, ByteOrder byteOrder);
    //! Read a 4 byte unsigned long value from the data buffer
    EXIV2API uint32_t getULong(const byte* buf, ByteOrder byteOrder);
    //! Read an 8 byte unsigned rational value from the data buffer
    EXIV2API URational getURational(const byte* buf, ByteOrder byteOrder);
    //! Read a 2 byte signed short value from the data buffer
    EXIV2API int16_t getShort(const byte* buf, ByteOrder byteOrder);
    //! Read a 4 byte signed long value from the data buffer
    EXIV2API int32_t getLong(const byte* buf, ByteOrder byteOrder);
    //! Read an 8 byte signed rational value from the data buffer
    EXIV2API Rational getRational(const byte* buf, ByteOrder byteOrder);
    //! Read a 4 byte single precision floating point value (IEEE 754 binary32) from the data buffer
    EXIV2API float getFloat(const byte* buf, ByteOrder byteOrder);
    //! Read an 8 byte double precision floating point value (IEEE 754 binary64) from the data buffer
    EXIV2API double getDouble(const byte* buf, ByteOrder byteOrder);

    //! Output operator for our fake rational
    EXIV2API std::ostream& operator<<(std::ostream& os, const Rational& r);
    //! Input operator for our fake rational
    EXIV2API std::istream& operator>>(std::istream& is, Rational& r);
    //! Output operator for our fake unsigned rational
    EXIV2API std::ostream& operator<<(std::ostream& os, const URational& r);
    //! Input operator for our fake unsigned rational
    EXIV2API std::istream& operator>>(std::istream& is, URational& r);

    /*!
      @brief Convert an unsigned short to data, write the data to the buffer,
             return number of bytes written.
     */
    EXIV2API long us2Data(byte* buf, uint16_t s, ByteOrder byteOrder);
    /*!
      @brief Convert an unsigned long to data, write the data to the buffer,
             return number of bytes written.
     */
    EXIV2API long ul2Data(byte* buf, uint32_t l, ByteOrder byteOrder);
    /*!
      @brief Convert an unsigned rational to data, write the data to the buffer,
             return number of bytes written.
     */
    EXIV2API long ur2Data(byte* buf, URational l, ByteOrder byteOrder);
    /*!
      @brief Convert a signed short to data, write the data to the buffer,
             return number of bytes written.
     */
    EXIV2API long s2Data(byte* buf, int16_t s, ByteOrder byteOrder);
    /*!
      @brief Convert a signed long to data, write the data to the buffer,
             return number of bytes written.
     */
    EXIV2API long l2Data(byte* buf, int32_t l, ByteOrder byteOrder);
    /*!
      @brief Convert a signed rational to data, write the data to the buffer,
             return number of bytes written.
     */
    EXIV2API long r2Data(byte* buf, Rational l, ByteOrder byteOrder);
    /*!
      @brief Convert a single precision floating point (IEEE 754 binary32) float
             to data, write the data to the buffer, return number of bytes written.
     */
    EXIV2API long f2Data(byte* buf, float f, ByteOrder byteOrder);
    /*!
      @brief Convert a double precision floating point (IEEE 754 binary64) double
             to data, write the data to the buffer, return number of bytes written.
     */
    EXIV2API long d2Data(byte* buf, double d, ByteOrder byteOrder);

    /*!
      @brief Print len bytes from buf in hex and ASCII format to the given
             stream, prefixed with the position in the buffer adjusted by
             offset.
     */
    EXIV2API void hexdump(std::ostream& os, const byte* buf, long len, long offset =0);

    /*!
      @brief Return true if str is a hex number starting with prefix followed
             by size hex digits, false otherwise. If size is 0, any number of
             digits is allowed and all are checked.
     */
    EXIV2API bool isHex(const std::string& str,
               size_t size =0,
               const std::string& prefix ="");

    /*!
      @brief Converts a string in the form "%Y:%m:%d %H:%M:%S", e.g.,
             "2007:05:24 12:31:55" to broken down time format,
             returns 0 if successful, else 1.
     */
    EXIV2API int exifTime(const char* buf, struct tm* tm);

    /*!
      @brief Translate a string using the gettext framework. This wrapper hides
             all the implementation details from the interface.
     */
    EXIV2API const char* exvGettext(const char* str);

#ifdef EXV_UNICODE_PATH
    //! Convert an std::string s to a unicode string returned as a std::wstring.
    EXIV2API std::wstring s2ws(const std::string& s);
    //! Convert a unicode std::wstring s to an std::string.
    EXIV2API std::string ws2s(const std::wstring& s);

#endif
    /*!
      @brief Return a \em long set to the value represented by \em s.

      Besides strings that represent \em long values, the function also
      handles \em float, \em Rational and boolean
      (see also: stringTo(const std::string& s, bool& ok)).

      @param  s  String to parse
      @param  ok Output variable indicating the success of the operation.
      @return Returns the \em long value represented by \em s and sets \em ok
              to \c true if the conversion was successful or \c false if not.
    */
    EXIV2API long parseLong(const std::string& s, bool& ok);

    /*!
      @brief Return a \em float set to the value represented by \em s.

      Besides strings that represent \em float values, the function also
      handles \em long, \em Rational and boolean
      (see also: stringTo(const std::string& s, bool& ok)).

      @param  s  String to parse
      @param  ok Output variable indicating the success of the operation.
      @return Returns the \em float value represented by \em s and sets \em ok
              to \c true if the conversion was successful or \c false if not.
    */
    EXIV2API float parseFloat(const std::string& s, bool& ok);

    /*!
      @brief Return a \em Rational set to the value represented by \em s.

      Besides strings that represent \em Rational values, the function also
      handles \em long, \em float and boolean
      (see also: stringTo(const std::string& s, bool& ok)).
      Uses floatToRationalCast(float f) if the string can be parsed into a
      \em float.

      @param  s  String to parse
      @param  ok Output variable indicating the success of the operation.
      @return Returns the \em Rational value represented by \em s and sets \em ok
              to \c true if the conversion was successful or \c false if not.
    */
    EXIV2API Rational parseRational(const std::string& s, bool& ok);

    /*!
      @brief Very simple conversion of a \em float to a \em Rational.

      Test it with the values that you expect and check the implementation
      to see if this is really what you want!
     */
    EXIV2API Rational floatToRationalCast(float f);

// *****************************************************************************
// template and inline definitions

    /*!
      @brief Find an element that matches \em key in the array \em src.

      Designed to be used with lookup tables as shown in the example below.
      Requires a %Key structure (ideally in the array) and a comparison operator
      to compare a key with an array element.  The size of the array is
      determined automagically. Thanks to Stephan Broennimann for this nifty
      implementation.

      @code
      struct Bar {
          int i;
          int k;
          const char* data;

          struct Key;
          bool operator==(const Bar::Key& rhs) const;
      };

      struct Bar::Key {
          Key(int a, int b) : i(a), k(b) {}
          int i;
          int k;
      };

      bool Bar::operator==(const Bar::Key& key) const // definition
      {
          return i == key.i && k == key.k;
      }

      const Bar bars[] = {
          { 1, 1, "bar data 1" },
          { 1, 2, "bar data 2" },
          { 1, 3, "bar data 3" }
      };

      int main ( void ) {
          const Bar* bar = find(bars, Bar::Key(1, 3));
          if (bar) std::cout << bar->data << "\n";
          else std::cout << "Key not found.\n";
          return 0;
      }
      @endcode
    */
    template<typename T, typename K, int N>
    const T* find(T (&src)[N], const K& key)
    {
        const T* rc = std::find(src, src + N, key);
        return rc == src + N ? 0 : rc;
    }

    //! Template used in the COUNTOF macro to determine the size of an array
    template <typename T, int N> char (&sizer(T (&)[N]))[N];
//! Macro to determine the size of an array
#define EXV_COUNTOF(a) (sizeof(Exiv2::sizer(a)))

    //! Utility function to convert the argument of any type to a string
    template<typename T>
    std::string toString(const T& arg)
    {
        std::ostringstream os;
        os << arg;
        return os.str();
    }

    /*!
      @brief Utility function to convert a string to a value of type \c T.

      The string representation of the value must match that recognized by
      the input operator for \c T for this function to succeed.

      @param  s  String to convert
      @param  ok Output variable indicating the success of the operation.
      @return Returns the converted value and sets \em ok to \c true if the
              conversion was successful or \c false if not.
     */
    template<typename T>
    T stringTo(const std::string& s, bool& ok)
    {
        std::istringstream is(s);
        T tmp;
        ok = is >> tmp ? true : false;
        std::string rest;
        is >> std::skipws >> rest;
        if (!rest.empty()) ok = false;
        return tmp;
    }

    /*!
      @brief Specialization of stringTo(const std::string& s, bool& ok) for \em bool.

      Handles the same string values as the XMP SDK. Converts the string to lowercase
      and returns \c true if it is "true", "t" or "1", and \c false if it is
      "false", "f" or "0".
     */
    template<>
    bool stringTo<bool>(const std::string& s, bool& ok);

    /*!
      @brief Return the greatest common denominator of n and m.
             (Implementation from Boost rational.hpp)

      @note We use n and m as temporaries in this function, so there is no
            value in using const IntType& as we would only need to make a copy
            anyway...
     */
    template <typename IntType>
    IntType gcd(IntType n, IntType m)
    {
        // Avoid repeated construction
        IntType zero(0);

        // This is abs() - given the existence of broken compilers with Koenig
        // lookup issues and other problems, I code this explicitly. (Remember,
        // IntType may be a user-defined type).
#ifdef _MSC_VER
#pragma warning( disable : 4146 )
#endif
        if (n < zero)
            n = -n;
        if (m < zero)
            m = -m;
#ifdef _MSC_VER
#pragma warning( default : 4146 )
#endif

        // As n and m are now positive, we can be sure that %= returns a
        // positive value (the standard guarantees this for built-in types,
        // and we require it of user-defined types).
        for(;;) {
            if(m == zero)
                return n;
            n %= m;
            if(n == zero)
                return m;
            m %= n;
        }
    }

}                                       // namespace Exiv2

#endif                                  // #ifndef TYPES_HPP_
