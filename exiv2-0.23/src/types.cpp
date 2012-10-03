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
  File:      types.cpp
  Version:   $Rev: 2681 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   26-Jan-04, ahu: created
             11-Feb-04, ahu: isolated as a component
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: types.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "i18n.h"                               // for _exvGettext

// + standard includes
#ifdef EXV_UNICODE_PATH
# include <windows.h> // for MultiByteToWideChar etc
#endif
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <utility>
#include <cctype>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>

// *****************************************************************************
namespace {

    //! Information pertaining to the defined %Exiv2 value type identifiers.
    struct TypeInfoTable {
        Exiv2::TypeId typeId_;                  //!< Type id
        const char* name_;                      //!< Name of the type
        long size_;                             //!< Bytes per data entry
        //! Comparison operator for \em typeId
        bool operator==(Exiv2::TypeId typeId) const
        {
            return typeId_ == typeId;
        }
        //! Comparison operator for \em name
        bool operator==(const std::string& name) const
        {
            return 0 == strcmp(name_, name.c_str());
        }
    }; // struct TypeInfoTable

    //! Lookup list with information of Exiv2 types
    const TypeInfoTable typeInfoTable[] = {
        { Exiv2::invalidTypeId,    "Invalid",     0 },
        { Exiv2::unsignedByte,     "Byte",        1 },
        { Exiv2::asciiString,      "Ascii",       1 },
        { Exiv2::unsignedShort,    "Short",       2 },
        { Exiv2::unsignedLong,     "Long",        4 },
        { Exiv2::unsignedRational, "Rational",    8 },
        { Exiv2::signedByte,       "SByte",       1 },
        { Exiv2::undefined,        "Undefined",   1 },
        { Exiv2::signedShort,      "SShort",      2 },
        { Exiv2::signedLong,       "SLong",       4 },
        { Exiv2::signedRational,   "SRational",   8 },
        { Exiv2::tiffFloat,        "Float",       4 },
        { Exiv2::tiffDouble,       "Double",      8 },
        { Exiv2::tiffIfd,          "Ifd",         4 },
        { Exiv2::string,           "String",      1 },
        { Exiv2::date,             "Date",        8 },
        { Exiv2::time,             "Time",       11 },
        { Exiv2::comment,          "Comment",     1 },
        { Exiv2::directory,        "Directory",   1 },
        { Exiv2::xmpText,          "XmpText",     1 },
        { Exiv2::xmpAlt,           "XmpAlt",      1 },
        { Exiv2::xmpBag,           "XmpBag",      1 },
        { Exiv2::xmpSeq,           "XmpSeq",      1 },
        { Exiv2::langAlt,          "LangAlt",     1 }
    };

}

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    const char* TypeInfo::typeName(TypeId typeId)
    {
        const TypeInfoTable* tit = find(typeInfoTable, typeId);
        if (!tit) return 0;
        return tit->name_;
    }

    TypeId TypeInfo::typeId(const std::string& typeName)
    {
        const TypeInfoTable* tit = find(typeInfoTable, typeName);
        if (!tit) return invalidTypeId;
        return tit->typeId_;
    }

    long TypeInfo::typeSize(TypeId typeId)
    {
        const TypeInfoTable* tit = find(typeInfoTable, typeId);
        if (!tit) return 0;
        return tit->size_;
    }

    DataBuf::DataBuf(DataBuf& rhs)
        : pData_(rhs.pData_), size_(rhs.size_)
    {
        rhs.release();
    }

    DataBuf::DataBuf(const byte* pData, long size)
        : pData_(0), size_(0)
    {
        if (size > 0) {
            pData_ = new byte[size];
            std::memcpy(pData_, pData, size);
            size_ = size;
        }
    }

    DataBuf& DataBuf::operator=(DataBuf& rhs)
    {
        if (this == &rhs) return *this;
        reset(rhs.release());
        return *this;
    }

    void DataBuf::alloc(long size)
    {
        if (size > size_) {
            delete[] pData_;
            pData_ = 0;
            size_ = 0;
            pData_ = new byte[size];
            size_ = size;
        }
    }

    std::pair<byte*, long> DataBuf::release()
    {
        std::pair<byte*, long> p = std::make_pair(pData_, size_);
        pData_ = 0;
        size_ = 0;
        return p;
    }

    void DataBuf::reset(std::pair<byte*, long> p)
    {
        if (pData_ != p.first) {
            delete[] pData_;
            pData_ = p.first;
        }
        size_ = p.second;
    }

    // *************************************************************************
    // free functions

    std::ostream& operator<<(std::ostream& os, const Rational& r)
    {
        return os << r.first << "/" << r.second;
    }

    std::istream& operator>>(std::istream& is, Rational& r)
    {
        int32_t nominator;
        int32_t denominator;
        char c('\0');
        is >> nominator >> c >> denominator;
        if (c != '/') is.setstate(std::ios::failbit);
        if (is) r = std::make_pair(nominator, denominator);
        return is;
    }

    std::ostream& operator<<(std::ostream& os, const URational& r)
    {
        return os << r.first << "/" << r.second;
    }

    std::istream& operator>>(std::istream& is, URational& r)
    {
        uint32_t nominator;
        uint32_t denominator;
        char c('\0');
        is >> nominator >> c >> denominator;
        if (c != '/') is.setstate(std::ios::failbit);
        if (is) r = std::make_pair(nominator, denominator);
        return is;
    }

    uint16_t getUShort(const byte* buf, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            return (byte)buf[1] << 8 | (byte)buf[0];
        }
        else {
            return (byte)buf[0] << 8 | (byte)buf[1];
        }
    }

    uint32_t getULong(const byte* buf, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            return   (byte)buf[3] << 24 | (byte)buf[2] << 16
                   | (byte)buf[1] <<  8 | (byte)buf[0];
        }
        else {
            return   (byte)buf[0] << 24 | (byte)buf[1] << 16
                   | (byte)buf[2] <<  8 | (byte)buf[3];
        }
    }

    URational getURational(const byte* buf, ByteOrder byteOrder)
    {
        uint32_t nominator = getULong(buf, byteOrder);
        uint32_t denominator = getULong(buf + 4, byteOrder);
        return std::make_pair(nominator, denominator);
    }

    int16_t getShort(const byte* buf, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            return (byte)buf[1] << 8 | (byte)buf[0];
        }
        else {
            return (byte)buf[0] << 8 | (byte)buf[1];
        }
    }

    int32_t getLong(const byte* buf, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            return   (byte)buf[3] << 24 | (byte)buf[2] << 16
                   | (byte)buf[1] <<  8 | (byte)buf[0];
        }
        else {
            return   (byte)buf[0] << 24 | (byte)buf[1] << 16
                   | (byte)buf[2] <<  8 | (byte)buf[3];
        }
    }

    Rational getRational(const byte* buf, ByteOrder byteOrder)
    {
        int32_t nominator = getLong(buf, byteOrder);
        int32_t denominator = getLong(buf + 4, byteOrder);
        return std::make_pair(nominator, denominator);
    }

    float getFloat(const byte* buf, ByteOrder byteOrder)
    {
        // This algorithm assumes that the internal representation of the float
        // type is the 4-byte IEEE 754 binary32 format, which is common but not
        // required by the C++ standard.
        assert(sizeof(float) == 4);
        union {
            uint32_t ul_;
            float    f_;
        } u;
        u.ul_ = getULong(buf, byteOrder);
        return u.f_;
    }

    double getDouble(const byte* buf, ByteOrder byteOrder)
    {
        // This algorithm assumes that the internal representation of the double
        // type is the 8-byte IEEE 754 binary64 format, which is common but not
        // required by the C++ standard.
        assert(sizeof(double) == 8);
        union {
            uint64_t ull_;
            double   d_;
        } u;
        u.ull_ = 0;
        if (byteOrder == littleEndian) {
            u.ull_ =   static_cast<uint64_t>(buf[7]) << 56
                     | static_cast<uint64_t>(buf[6]) << 48
                     | static_cast<uint64_t>(buf[5]) << 40
                     | static_cast<uint64_t>(buf[4]) << 32
                     | static_cast<uint64_t>(buf[3]) << 24
                     | static_cast<uint64_t>(buf[2]) << 16
                     | static_cast<uint64_t>(buf[1]) <<  8
                     | static_cast<uint64_t>(buf[0]);
        }
        else {
            u.ull_ =   static_cast<uint64_t>(buf[0]) << 56
                     | static_cast<uint64_t>(buf[1]) << 48
                     | static_cast<uint64_t>(buf[2]) << 40
                     | static_cast<uint64_t>(buf[3]) << 32
                     | static_cast<uint64_t>(buf[4]) << 24
                     | static_cast<uint64_t>(buf[5]) << 16
                     | static_cast<uint64_t>(buf[6]) <<  8
                     | static_cast<uint64_t>(buf[7]);
        }
        return u.d_;
    }

    long us2Data(byte* buf, uint16_t s, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            buf[0] = (byte) (s & 0x00ff);
            buf[1] = (byte)((s & 0xff00) >> 8);
        }
        else {
            buf[0] = (byte)((s & 0xff00) >> 8);
            buf[1] = (byte) (s & 0x00ff);
        }
        return 2;
    }

    long ul2Data(byte* buf, uint32_t l, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            buf[0] = (byte) (l & 0x000000ff);
            buf[1] = (byte)((l & 0x0000ff00) >> 8);
            buf[2] = (byte)((l & 0x00ff0000) >> 16);
            buf[3] = (byte)((l & 0xff000000) >> 24);
        }
        else {
            buf[0] = (byte)((l & 0xff000000) >> 24);
            buf[1] = (byte)((l & 0x00ff0000) >> 16);
            buf[2] = (byte)((l & 0x0000ff00) >> 8);
            buf[3] = (byte) (l & 0x000000ff);
        }
        return 4;
    }

    long ur2Data(byte* buf, URational l, ByteOrder byteOrder)
    {
        long o = ul2Data(buf, l.first, byteOrder);
        o += ul2Data(buf+o, l.second, byteOrder);
        return o;
    }

    long s2Data(byte* buf, int16_t s, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            buf[0] =  (byte)(s & 0x00ff);
            buf[1] = (byte)((s & 0xff00) >> 8);
        }
        else {
            buf[0] = (byte)((s & 0xff00) >> 8);
            buf[1] =  (byte)(s & 0x00ff);
        }
        return 2;
    }

    long l2Data(byte* buf, int32_t l, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            buf[0] =  (byte)(l & 0x000000ff);
            buf[1] = (byte)((l & 0x0000ff00) >> 8);
            buf[2] = (byte)((l & 0x00ff0000) >> 16);
            buf[3] = (byte)((l & 0xff000000) >> 24);
        }
        else {
            buf[0] = (byte)((l & 0xff000000) >> 24);
            buf[1] = (byte)((l & 0x00ff0000) >> 16);
            buf[2] = (byte)((l & 0x0000ff00) >> 8);
            buf[3] =  (byte)(l & 0x000000ff);
        }
        return 4;
    }

    long r2Data(byte* buf, Rational l, ByteOrder byteOrder)
    {
        long o = l2Data(buf, l.first, byteOrder);
        o += l2Data(buf+o, l.second, byteOrder);
        return o;
    }

    long f2Data(byte* buf, float f, ByteOrder byteOrder)
    {
        // This algorithm assumes that the internal representation of the float
        // type is the 4-byte IEEE 754 binary32 format, which is common but not
        // required by the C++ standard.
        assert(sizeof(float) == 4);
        union {
            uint32_t ul_;
            float    f_;
        } u;
        u.f_ = f;
        return ul2Data(buf, u.ul_, byteOrder);
    }

    long d2Data(byte* buf, double d, ByteOrder byteOrder)
    {
        // This algorithm assumes that the internal representation of the double
        // type is the 8-byte IEEE 754 binary64 format, which is common but not
        // required by the C++ standard.
        assert(sizeof(double) == 8);
        union {
            uint64_t ull_;
            double   d_;
        } u;
        u.d_ = d;
        uint64_t m = 0xff;
        if (byteOrder == littleEndian) {
            buf[0] =  (byte)(u.ull_ & m);
            buf[1] = (byte)((u.ull_ & (m <<  8)) >>  8);
            buf[2] = (byte)((u.ull_ & (m << 16)) >> 16);
            buf[3] = (byte)((u.ull_ & (m << 24)) >> 24);
            buf[4] = (byte)((u.ull_ & (m << 32)) >> 32);
            buf[5] = (byte)((u.ull_ & (m << 40)) >> 40);
            buf[6] = (byte)((u.ull_ & (m << 48)) >> 48);
            buf[7] = (byte)((u.ull_ & (m << 56)) >> 56);
        }
        else {
            buf[0] = (byte)((u.ull_ & (m << 56)) >> 56);
            buf[1] = (byte)((u.ull_ & (m << 48)) >> 48);
            buf[2] = (byte)((u.ull_ & (m << 40)) >> 40);
            buf[3] = (byte)((u.ull_ & (m << 32)) >> 32);
            buf[4] = (byte)((u.ull_ & (m << 24)) >> 24);
            buf[5] = (byte)((u.ull_ & (m << 16)) >> 16);
            buf[6] = (byte)((u.ull_ & (m <<  8)) >>  8);
            buf[7] =  (byte)(u.ull_ & m);
        }
        return 8;
    }

    void hexdump(std::ostream& os, const byte* buf, long len, long offset)
    {
        const std::string::size_type pos = 8 + 16 * 3 + 2;
        const std::string align(pos, ' ');

        long i = 0;
        while (i < len) {
            os << "  "
               << std::setw(4) << std::setfill('0') << std::hex
               << i + offset << "  ";
            std::ostringstream ss;
            do {
                byte c = buf[i];
                os << std::setw(2) << std::setfill('0') << std::right
                   << std::hex << (int)c << " ";
                ss << ((int)c >= 31 && (int)c < 127 ? char(buf[i]) : '.');
            } while (++i < len && i%16 != 0);
            std::string::size_type width = 9 + ((i-1)%16 + 1) * 3;
            os << (width > pos ? "" : align.substr(width)) << ss.str() << "\n";
        }
        os << std::dec << std::setfill(' ');
    } // hexdump

    bool isHex(const std::string& str, size_t size, const std::string& prefix)
    {
        if (   str.size() <= prefix.size()
            || str.substr(0, prefix.size()) != prefix) return false;
        if (   size > 0
            && str.size() != size + prefix.size()) return false;

        for (size_t i = prefix.size(); i < str.size(); ++i) {
            if (!isxdigit(str[i])) return false;
        }
        return true;
    } // isHex

    int exifTime(const char* buf, struct tm* tm)
    {
        assert(buf != 0);
        assert(tm != 0);
        int rc = 1;
        int year, mon, mday, hour, min, sec;
        int scanned = std::sscanf(buf, "%4d:%2d:%2d %2d:%2d:%2d",
                                  &year, &mon, &mday, &hour, &min, &sec);
        if (scanned == 6) {
            tm->tm_year = year - 1900;
            tm->tm_mon  = mon - 1;
            tm->tm_mday = mday;
            tm->tm_hour = hour;
            tm->tm_min  = min;
            tm->tm_sec  = sec;
            rc = 0;
        }
        return rc;
    } // exifTime

    const char* exvGettext(const char* str)
    {
#ifdef EXV_ENABLE_NLS
        return _exvGettext(str);
#else
        return str;
#endif
    }

#ifdef EXV_UNICODE_PATH
    std::string ws2s(const std::wstring& s)
    {
        int len;
        int slength = (int)s.length() + 1;
        len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
        char* buf = new char[len];
        WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, buf, len, 0, 0);
        std::string r(buf);
        delete[] buf;
        return r;
    }

    std::wstring s2ws(const std::string& s)
    {
        int len;
        int slength = (int)s.length() + 1;
        len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
        wchar_t* buf = new wchar_t[len];
        MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
        std::wstring r(buf);
        delete[] buf;
        return r;
    }

#endif // EXV_UNICODE_PATH
    template<>
    bool stringTo<bool>(const std::string& s, bool& ok)
    {
        std::string lcs(s); /* lowercase string */
        for(unsigned i = 0; i < lcs.length(); i++) {
            lcs[i] = std::tolower(s[i]);
        }
        /* handle the same values as xmp sdk */
        if (lcs == "false" || lcs == "f" || lcs == "0") {
            ok = true;
            return false;
        }
        if (lcs == "true" || lcs == "t" || lcs == "1") {
            ok = true;
            return true;
        }
        ok = false;
        return false;
    }

    long parseLong(const std::string& s, bool& ok)
    {
        long ret = stringTo<long>(s, ok);
        if (ok) return ret;

        float f = stringTo<float>(s, ok);
        if (ok) return static_cast<long>(f);

        Rational r = stringTo<Rational>(s, ok);
        if (ok) {
            if (r.second == 0) {
                ok = false;
                return 0;
            }
            return static_cast<long>(static_cast<float>(r.first) / r.second);
        }

        bool b = stringTo<bool>(s, ok);
        if (ok) return b ? 1 : 0;

        // everything failed, return from stringTo<long> is probably the best fit
        return ret;
    }

    float parseFloat(const std::string& s, bool& ok)
    {
        float ret = stringTo<float>(s, ok);
        if (ok) return ret;

        Rational r = stringTo<Rational>(s, ok);
        if (ok) {
            if (r.second == 0) {
                ok = false;
                return 0.0;
            }
            return static_cast<float>(r.first) / r.second;
        }

        bool b = stringTo<bool>(s, ok);
        if (ok) return b ? 1.0f : 0.0f;

        // everything failed, return from stringTo<float> is probably the best fit
        return ret;
    }

    Rational parseRational(const std::string& s, bool& ok)
    {
        Rational ret = stringTo<Rational>(s, ok);
        if (ok) return ret;

        long l = stringTo<long>(s, ok);
        if (ok) return Rational(l, 1);

        float f = stringTo<float>(s, ok);
        if (ok) return floatToRationalCast(f);

        bool b = stringTo<bool>(s, ok);
        if (ok) return b ? Rational(1, 1) : Rational(0, 1);

        // everything failed, return from stringTo<Rational> is probably the best fit
        return ret;
    }

    Rational floatToRationalCast(float f)
    {
        // Beware: primitive conversion algorithm
        int32_t den = 1000000;
        if (std::labs(static_cast<long>(f)) > 2147) den = 10000;
        if (std::labs(static_cast<long>(f)) > 214748) den = 100;
        if (std::labs(static_cast<long>(f)) > 21474836) den = 1;
        const float rnd = f >= 0 ? 0.5f : -0.5f;
        const int32_t nom = static_cast<int32_t>(f * den + rnd);
        const int32_t g = gcd(nom, den);

        return Rational(nom/g, den/g);
    }

}                                       // namespace Exiv2

#ifdef EXV_ENABLE_NLS
// Declaration is in i18n.h
const char* _exvGettext(const char* str)
{
    static bool exvGettextInitialized = false;

    if (!exvGettextInitialized) {
        bindtextdomain(EXV_PACKAGE, EXV_LOCALEDIR);
# ifdef EXV_HAVE_BIND_TEXTDOMAIN_CODESET
        bind_textdomain_codeset (EXV_PACKAGE, "UTF-8");
# endif
        exvGettextInitialized = true;
    }

    return dgettext(EXV_PACKAGE, str);
}
#endif // EXV_ENABLE_NLS
