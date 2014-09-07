/*
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP (HUPnP) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HUPNP_DATATYPES_H_
#define HUPNP_DATATYPES_H_

#include <HUpnpCore/HUpnp>

#include <QtCore/QString>
#include <QtCore/QVariant>

class QUrl;
class QDate;
class QTime;
struct QUuid;
class QDateTime;
class QByteArray;

namespace Herqq
{

namespace Upnp
{

/*!
 * An utility class for working with UPnP data types.
 *
 * \headerfile hupnp_datatypes.h HUpnpDataTypes
 */
class H_UPNP_CORE_EXPORT HUpnpDataTypes
{
H_DISABLE_COPY(HUpnpDataTypes)

private:

    HUpnpDataTypes();
    ~HUpnpDataTypes();

public:

    /*!
     * This enum type defines the UPnP data types set by the UPnP Device Architecture
     * v1.1 specification.
     */
    enum DataType
    {
        /*!
         * Undefined, illegal data type.
         */
        Undefined = 0,

        /*!
         * Unsigned 1 Byte int. Same format as int without leading sign.
         */
        ui1,

        /*!
         * Unsigned 2 Byte int. Same format as int without leading sign.
         */
        ui2,

        /*!
         * Unsigned 4 Byte int. Same format as int without leading sign.
         */
        ui4

        /*!
         * 1 Byte int. Same format as int.
         */,
        i1,

        /*!
         * 2 Byte int. Same format as int.
         */
        i2,

        /*!
         * 4 Byte int. Same format as int. MUST be between -2147483648 and 2147483647.
         */
        i4,

        /*!
         * Fixed point, integer number.
         */
        integer,

        /*!
         * 4 Byte float. Same format as float. MUST be between 3.40282347E+38 to
         * 1.17549435E-38.
         */
        r4,

        /*!
         * 8 Byte float. Same format as float. MUST be between -1.79769313486232E308 and -
         * 4.94065645841247E-324 for negative values, and between 4.94065645841247E-324
         * and 1.79769313486232E308 for positive values, i.e., IEEE 64-bit (8-Byte) double.
         */
        r8,

        /*!
         * Same as r8.
         */
        number,

        /*!
         * Same as r8 but no more than 14 digits to the left of the decimal point and no more
         * than 4 to the right.
         */
        fixed_14_4,

        /*!
         * Floating point number. Mantissa (left of the decimal) and/or exponent MAY have a
         * leading sign. Mantissa and/or exponent MAY have leading zeros, which SHOULD be
         * ignored by the recipient. Decimal character in mantissa is a period, i.e., whole
         * digits in mantissa separated from fractional digits by period ("."). Mantissa
         * separated from exponent by "E".
         */
        fp,

        /*!
         * Unicode string. One character long.
         */
        character,

        /*!
         * Unicode string. No limit on length.
         */
        string,

        /*!
         * Date in a subset of ISO 8601 format without time data.
         */
        date,

        /*!
         * Date in ISO 8601 format with OPTIONAL time but no time zone.
         */
        dateTime,

        /*!
         * Date in ISO 8601 format with OPTIONAL time and OPTIONAL time zone.
         */
        dateTimeTz,

        /*!
         * Time in a subset of ISO 8601 format with no date and no time zone.
         */
        time,

        /*!
         * Time in a subset of ISO 8601 format with OPTIONAL time zone but no date.
         */
        timeTz,

        /*!
         * "0" for false or "1" for true. The values "true", "yes", "false", or "no" are
         * deprecated and MUST NOT be sent but MUST be accepted when received. When
         * received, the values "true" and "yes" MUST be interpreted as true and the values
         * "false" and "no" MUST be interpreted as false.
         */
        boolean,

        /*!
         * MIME-style Base64 encoded binary BLOB. Takes 3 Bytes, splits them into 4 parts, and
         * maps each 6 bit piece to an octet. (3 octets are encoded as 4.) No limit on size.
         */
        bin_base64,

        /*!
         * Hexadecimal digits representing octets. Treats each nibble as a hex digit and
         * encodes as a separate Byte. (1 octet is encoded as 2.) No limit on size.
         */
        bin_hex,

        /*!
         * Universal Resource Identifier.
         */
        uri,

        /*!
         * Universally Unique ID
         */
        uuid
    };

    /*******************************************************************************
     * Data types as strings
     *******************************************************************************/

    /*!
     * The string identifier for an unsigned 1 byte int.
     *
     * \sa DataType::ui1
     */
    const static QString& ui1_str()
    {
        static QString retVal = "ui1";
        return retVal;
    }

    /*!
     * The string identifier for an unsigned 2 byte int.
     *
     * \sa DataType::ui2
     */
    const static QString& ui2_str()
    {
        static QString retVal = "ui2";
        return retVal;
    }

    /*!
     * The string identifier for an unsigned 4 byte int.
     *
     * \sa DataType::ui4
     */
    const static QString& ui4_str()
    {
        static QString retVal = "ui4";
        return retVal;
    }

    /*!
     * The string identifier for a 1 byte int.
     *
     * \sa DataType::i1
     */
    const static QString& i1_str()
    {
        static QString retVal = "i1";
        return retVal;
    }

    /*!
     * The string identifier for a 2 byte int.
     *
     * \sa DataType::i2
     */
    const static QString& i2_str()
    {
        static QString retVal = "i2";
        return retVal;
    }

    /*!
     * The string identifier for a 4 byte int.
     *
     * \sa DataType::i4
     */
    const static QString& i4_str()
    {
        static QString retVal = "i4";
        return retVal;
    }

    /*!
     * The string identifier for a fixed point, integer number.
     *
     * \sa DataType::integer
     */
    const static QString& integer_str()
    {
        static QString retVal = "int";
        return retVal;
    }

    /*!
     * The string identifier for a 4 byte float.
     *
     * \sa DataType::r4
     */
    const static QString& r4_str()
    {
        static QString retVal = "r4";
        return retVal;
    }

    /*!
     * The string identifier for a 8 byte float.
     *
     * \sa DataType::r8
     */
    const static QString& r8_str()
    {
        static QString retVal = "r8";
        return retVal;
    }

    /*!
     * The string identifier for a 8 byte float. This is an alias for r8.
     *
     * \sa DataType::number
     */
    const static QString& number_str()
    {
        static QString retVal = "number";
        return retVal;
    }

    /*!
     * The string identifier for a 8 byte float that has no more than
     * 14 digits to the left of the decimal point and no more
     * than 4 to the right.
     *
     * \sa DataType::fixed_14_4
     */
    const static QString& fixed_14_4_str()
    {
        static QString retVal = "fixed.14.4";
        return retVal;
    }

    /*!
     * The string identifier for a floating point number.
     *
     * \sa DataType::fp
     */
    const static QString& fp_str()
    {
        static QString retVal = "float";
        return retVal;
    }

    /*!
     * The string identifier for a unicode string that is one character long.
     *
     * \sa DataType::char
     */
    const static QString& character_str()
    {
        static QString retVal = "char";
        return retVal;
    }

    /*!
     * The string identifier for a unicode string that has no limit on length.
     *
     * \sa DataType::string
     */
    static const QString& string_str()
    {
        static QString retVal = "string";
        return retVal;
    }

    /*!
     * The string identifier for a date in a subset of ISO 8601 format
     * without time data.
     *
     * \sa DataType::date
     */
    const static QString& date_str()
    {
        static QString retVal = "date";
        return retVal;
    }

    /*!
     * The string identifier for a date in ISO 8601 format with OPTIONAL
     * time but no time zone.
     *
     * \sa DataType::dateTime
     */
    const static QString& dateTime_str()
    {
        static QString retVal = "dateTime";
        return retVal;
    }

    /*!
     * The string identifier for a date in ISO 8601 format with OPTIONAL
     * time and OPTIONAL time zone.
     *
     * \sa DataType::dateTimeTz
     */
    const static QString& dateTimeTz_str()
    {
        static QString retVal = "dateTime.tz";
        return retVal;
    }

    /*!
     * The string identifier for a time in a subset of ISO 8601 format
     * with no date and no time zone.
     *
     * \sa DataType::time
     */
    const static QString& time_str()
    {
        static QString retVal = "time";
        return retVal;
    }

    /*!
     * The string identifier for a time in a subset of ISO 8601 format
     * with OPTIONAL time zone but no date.
     *
     * \sa DataType::timeTz
     */
    const static QString& time_tz_str()
    {
        static QString retVal = "time.tz";
        return retVal;
    }

    /*!
     * The string identifier for a boolean.
     *
     * \sa DataType::boolean
     */
    const static QString& boolean_str()
    {
        static QString retVal = "boolean";
        return retVal;
    }

    /*!
     * The string identifier for a MIME-style Base64 encoded binary BLOB.
     *
     * \sa DataType::bin_base64
     */
    const static QString& bin_base64_str()
    {
        static QString retVal = "bin.base64";
        return retVal;
    }

    /*!
     * The string identifier for a hexadecimal digits representing octets.
     *
     * \sa DataType::bin_hex
     */
    const static QString& bin_hex_str()
    {
        static QString retVal = "bin.hex";
        return retVal;
    }

    /*!
     * The string identifier for a universal Resource Identifier.
     *
     * \sa DataType::uri
     */
    const static QString& uri_str()
    {
        static QString retVal = "uri";
        return retVal;
    }

    /*!
     * The string identifier for a universally Unique ID.
     *
     * \sa DataType::uuid
     */
    const static QString& uuid_str()
    {
        static QString retVal = "uuid";
        return retVal;
    }

    /*******************************************************************************
     * Type definitions for the Upnp data types
     *******************************************************************************/

    /*!
     * Type definition for a DataType::ui1.
     */
    typedef quint8 ui1T;

    /*!
     * Type definition for a DataType::ui2.
     */
    typedef quint16 ui2T;

    /*!
     * Type definition for a DataType::ui4.
     */
    typedef quint32 ui4T;

    /*!
     * Type definition for a DataType::i1.
     */
    typedef qint8 i1T;

    /*!
     * Type definition for a DataType::i2.
     */
    typedef qint16 i2T;

    /*!
     * Type definition for a DataType::i4.
     */
    typedef qint32 i4T;

    /*!
     * Type definition for a DataType::integer.
     */
    typedef i4T integerT;

    /*!
     * Type definition for a DataType::r4.
     */
    typedef float r4T;

    /*!
     * Type definition for a DataType::r8.
     */
    typedef qreal r8T;

    /*!
     * Type definition for a DataType::number.
     */
    typedef r8T numberT;

    /*!
     * Type definition for a DataType::fixed_14_4.
     */
    typedef qreal fixed_14_4T;

    /*!
     * Type definition for a DataType::fp.
     */
    typedef float fpT;

    /*!
     * Type definition for a DataType::char.
     */
    typedef char characterT;

    /*!
     * Type definition for a DataType::string.
     */
    typedef QString stringT;

    /*!
     * Type definition for a DataType::date
     */
    typedef QDate dateT;

    /*!
     * Type definition for a DataType::dateTime.
     */
    typedef QDateTime dateTimeT;

    /*!
     * Type definition for a DataType::time.
     */
    typedef QTime timeT;

    /*!
     * Type definition for a DataType::timeTz.
     */
    typedef QTime time_tzT;

    /*!
     * Type definition for a DataType::boolean.
     */
    typedef bool booleanT;

    /*!
     * Type definition for a DataType::bin_base64.
     */
    typedef QByteArray bin_base64T;

    /*!
     * Type definition for a DataType::bin_hex.
     */
    typedef QByteArray bin_hexT;

    /*!
     * Type definition for a DataType::uri.
     */
    typedef QUrl uriT;

    /*!
     * Type definition for a DataType::uuid.
     */
    typedef QUuid uuidT;

    /*!
     * \brief Returns the UPnP data type enum value that matches the content
     * of the specified string, if any.
     *
     * \param dataTypeAsStr specifies the "name" of the UPnP data type as string.
     * For example, the string could contain "i4", which in the UDA v1.1 specification
     * is defined as 4-byte signed integer.
     *
     * \return The UPnP data type enum value that matches the content
     * of the specified string. If the specified string does not correspond
     * to any UPnP data type, HUpnpDataTypes::Undefined is returned.
     */
    static DataType dataType(const QString& dataTypeAsStr);

    /*!
     * \brief Indicates whether or not the specified data type is numeric.
     *
     * \param datatype specifies the data type to be checked.
     *
     * \retval true if the specified data type is numeric.
     * \retval false in case the data type is not numeric or it is undefined.
     */
    inline static bool isNumeric(DataType datatype)
    {
        return datatype >= ui1 && datatype <= fp;
    }

    /*!
     * \brief Indicates whether or not the specified data type is an integer.
     *
     * \param datatype specifies the data type to be checked.
     *
     * \retval true if the specified data type is an integer.
     * \retval false in case the data type is not an integer or it is undefined.
     */
    inline static bool isInteger(DataType datatype)
    {
        return datatype >= ui1 && datatype <= integer;
    }

    /*!
     * \brief Indicates whether or not the specified data type is a rational number.
     *
     * \param arg specifies the data type to be checked.
     *
     * \retval true if the specified data type is a rational number.
     * \retval false in case the data type is not rational or it is undefined.
     */
    inline static bool isRational(DataType arg)
    {
        switch(arg)
        {
            case r4:
            case r8:
            case number:
            case fp:
            case fixed_14_4:
                return true;
            default:
                return false;
        }
    }

    /*!
     * \brief Returns the UDA defined string representation of the specified data type.
     *
     * \param datatype specifies the data type which string representation is requested.
     *
     * \return The UDA defined string representation of the specified data type.
     */
    static QString toString(DataType datatype);

    static QVariant::Type convertToVariantType(HUpnpDataTypes::DataType);
    static QVariant convertToRightVariantType(
        const QString& value, HUpnpDataTypes::DataType);
};

}
}

#endif /* HUPNP_DATATYPES_H_ */
