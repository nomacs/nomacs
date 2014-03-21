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

#include "hupnp_datatypes.h"
#include "hupnp_datatypes_p.h"

#include <QtCore/QUrl>
#include <QtCore/QDateTime>

namespace Herqq
{

namespace Upnp
{


SoapType::SoapType(
    const QString& name, HUpnpDataTypes::DataType dt, const QVariant& value) :
        QtSoapSimpleType()
{
    Q_ASSERT(!name.isEmpty());
    Q_ASSERT_X(value.isValid(), "", name.toLocal8Bit());
    Q_ASSERT(dt != HUpnpDataTypes::Undefined);

    n = QtSoapQName(name);
    t = convertToSoapType(dt);

    if (dt == HUpnpDataTypes::uri)
    {
        // the qtsoap library handles its "anyURI" type as it does almost any other
        // type, as string. However, at the time of writing this (with Qt 4.5.3)
        // the QVariant does not support toString() for Url types.
        v = value.toUrl().toString();
    }
    else
    {
        v = value;
    }
}

QtSoapType::Type convertToSoapType(HUpnpDataTypes::DataType upnpDataType)
{
    switch (upnpDataType)
    {
    case HUpnpDataTypes::i1:
        return QtSoapType::Byte;

    case HUpnpDataTypes::i2:
        return QtSoapType::Short;

    case HUpnpDataTypes::i4:
    case HUpnpDataTypes::integer:
        return QtSoapType::Integer;

    case HUpnpDataTypes::ui1:
        return QtSoapType::UnsignedByte;

    case HUpnpDataTypes::ui2:
        return QtSoapType::UnsignedShort;

    case HUpnpDataTypes::ui4:
        return QtSoapType::UnsignedInt;

    case HUpnpDataTypes::r4:
    case HUpnpDataTypes::r8:
    case HUpnpDataTypes::number:
    case HUpnpDataTypes::fixed_14_4:
        return QtSoapType::Double;

    case HUpnpDataTypes::fp:
        return QtSoapType::Float;

    case HUpnpDataTypes::character:
    case HUpnpDataTypes::string:
        return QtSoapType::String;

    case HUpnpDataTypes::date:
        return QtSoapType::Date;

    case HUpnpDataTypes::dateTime:
    case HUpnpDataTypes::dateTimeTz:
        return QtSoapType::DateTime;

    case HUpnpDataTypes::time:
    case HUpnpDataTypes::timeTz:
        return QtSoapType::Time;

    case HUpnpDataTypes::boolean:
        return QtSoapType::Boolean;

    case HUpnpDataTypes::bin_base64:
        return QtSoapType::Base64Binary;

    case HUpnpDataTypes::bin_hex:
        return QtSoapType::HexBinary;

    case HUpnpDataTypes::uri:
        return QtSoapType::AnyURI;

    case HUpnpDataTypes::uuid:
        return QtSoapType::ID;

    case HUpnpDataTypes::Undefined:
        return QtSoapType::Other;

    default:
        Q_ASSERT(false);
    }

    Q_ASSERT(false);
    return QtSoapType::Other;
}

HUpnpDataTypes::HUpnpDataTypes()
{
}

HUpnpDataTypes::~HUpnpDataTypes()
{
}

HUpnpDataTypes::DataType HUpnpDataTypes::dataType(const QString& dataTypeAsStr)
{
    if (dataTypeAsStr.compare(ui1_str()) == 0)
    {
        return ui1;
    }
    else if (dataTypeAsStr.compare(ui2_str()) == 0)
    {
        return ui2;
    }
    else if (dataTypeAsStr.compare(ui4_str()) == 0)
    {
        return ui4;
    }
    else if (dataTypeAsStr.compare(i1_str()) == 0)
    {
        return i1;
    }
    else if (dataTypeAsStr.compare(i2_str()) == 0)
    {
        return i2;
    }
    else if (dataTypeAsStr.compare(i4_str()) == 0)
    {
        return i4;
    }
    else if (dataTypeAsStr.compare(integer_str()) == 0)
    {
        return integer;
    }
    else if (dataTypeAsStr.compare(r4_str()) == 0)
    {
        return r4;
    }
    else if (dataTypeAsStr.compare(r8_str()) == 0)
    {
        return r8;
    }
    else if (dataTypeAsStr.compare(number_str()) == 0)
    {
        return number;
    }
    else if (dataTypeAsStr.compare(fixed_14_4_str()) == 0)
    {
        return fixed_14_4;
    }
    else if (dataTypeAsStr.compare(fp_str()) == 0)
    {
        return fp;
    }
    else if (dataTypeAsStr.compare(character_str()) == 0)
    {
        return character;
    }
    else if (dataTypeAsStr.compare(string_str()) == 0)
    {
        return string;
    }
    else if (dataTypeAsStr.compare(date_str()) == 0)
    {
        return date;
    }
    else if (dataTypeAsStr.compare(dateTime_str()) == 0)
    {
        return dateTime;
    }
    else if (dataTypeAsStr.compare(dateTimeTz_str()) == 0)
    {
        return dateTimeTz;
    }
    else if (dataTypeAsStr.compare(time_str()) == 0)
    {
        return time;
    }
    else if (dataTypeAsStr.compare(time_tz_str()) == 0)
    {
        return timeTz;
    }
    else if (dataTypeAsStr.compare(boolean_str()) == 0)
    {
        return boolean;
    }
    else if (dataTypeAsStr.compare(bin_base64_str()) == 0)
    {
        return bin_base64;
    }
    else if (dataTypeAsStr.compare(bin_hex_str()) == 0)
    {
        return bin_hex;
    }
    else if (dataTypeAsStr.compare(uri_str()) == 0)
    {
        return uri;
    }
    else if (dataTypeAsStr.compare(uuid_str()) == 0)
    {
        return uuid;
    }

    return Undefined;
}

QString HUpnpDataTypes::toString(DataType dataType)
{
    switch(dataType)
    {
    case Undefined:
        return "Undefined";
    case ui1:
        return ui1_str();
    case ui2:
        return ui2_str();
    case ui4:
        return ui4_str();
    case i1:
        return i1_str();
    case i2:
        return i2_str();
    case i4:
        return i4_str();
    case integer:
        return integer_str();
    case r4:
        return r4_str();
    case r8:
        return r8_str();
    case number:
        return number_str();
    case fixed_14_4:
        return fixed_14_4_str();
    case fp:
        return fp_str();
    case character:
        return character_str();
    case string:
        return string_str();
    case date:
        return date_str();
    case dateTime:
        return dateTime_str();
    case dateTimeTz:
        return dateTimeTz_str();
    case time:
        return time_str();
    case timeTz:
        return time_tz_str();
    case boolean:
        return boolean_str();
    case bin_base64:
        return bin_base64_str();
    case bin_hex:
        return bin_hex_str();
    case uri:
        return uri_str();
    case uuid:
        return uuid_str();
    default:
        return "Undefined";
    }
}

QVariant::Type HUpnpDataTypes::convertToVariantType(
    HUpnpDataTypes::DataType upnpDataType)
{
    switch (upnpDataType)
    {
    case HUpnpDataTypes::character:
        return QVariant::Char;

    case HUpnpDataTypes::i1:
    case HUpnpDataTypes::i2:
    case HUpnpDataTypes::i4:
    case HUpnpDataTypes::integer:
        return QVariant::Int;

    case HUpnpDataTypes::ui1:
    case HUpnpDataTypes::ui2:
    case HUpnpDataTypes::ui4:
        return QVariant::UInt;

    case HUpnpDataTypes::fp:
    case HUpnpDataTypes::r4:
    case HUpnpDataTypes::r8:
    case HUpnpDataTypes::number:
    case HUpnpDataTypes::fixed_14_4:
        return QVariant::Double;

    case HUpnpDataTypes::string:
        return QVariant::String;

    case HUpnpDataTypes::date:
        return QVariant::Date;

    case HUpnpDataTypes::dateTime:
    case HUpnpDataTypes::dateTimeTz:
        return QVariant::DateTime;

    case HUpnpDataTypes::time:
    case HUpnpDataTypes::timeTz:
        return QVariant::Time;

    case HUpnpDataTypes::boolean:
        return QVariant::Bool;

    case HUpnpDataTypes::bin_hex:
    case HUpnpDataTypes::bin_base64:
        return QVariant::ByteArray;

    case HUpnpDataTypes::uri:
        return QVariant::Url;

    case HUpnpDataTypes::uuid:
        return QVariant::String;

    default:
        Q_ASSERT(false);
    }

    Q_ASSERT(false);
    return QVariant::Invalid;
}

QVariant HUpnpDataTypes::convertToRightVariantType(
    const QString& value, HUpnpDataTypes::DataType upnpDataType)
{
    QVariant retVal;

    switch (upnpDataType)
    {
    case HUpnpDataTypes::character:
        return !value.isEmpty() ? QChar(value[0]) : QVariant(QVariant::Char);

    case HUpnpDataTypes::i1:
    case HUpnpDataTypes::i2:
    case HUpnpDataTypes::i4:
    case HUpnpDataTypes::integer:
    {
        bool ok = false;
        retVal = value.toInt(&ok);
        break;
    }

    case HUpnpDataTypes::ui1:
    case HUpnpDataTypes::ui2:
    case HUpnpDataTypes::ui4:
    {
        bool ok = false;
        retVal = value.toUInt(&ok);
        break;
    }

    case HUpnpDataTypes::fp:
    case HUpnpDataTypes::r4:
    case HUpnpDataTypes::r8:
    case HUpnpDataTypes::number:
    case HUpnpDataTypes::fixed_14_4:
    {
        bool ok = false;
        retVal = value.toDouble(&ok);
        break;
    }

    case HUpnpDataTypes::string:
        return value;

    case HUpnpDataTypes::date:
    {
        retVal = QDate::fromString(value, Qt::ISODate);
        break;
    }

    case HUpnpDataTypes::dateTime:
    case HUpnpDataTypes::dateTimeTz:
    {
        retVal = QDateTime::fromString(value, Qt::ISODate);
        break;
    }

    case HUpnpDataTypes::time:
    case HUpnpDataTypes::timeTz:
    {
        retVal = QTime::fromString(value, Qt::ISODate);
        break;
    }

    case HUpnpDataTypes::boolean:
    {
        if (value.compare("true", Qt::CaseInsensitive) == 0 ||
            value.compare("yes", Qt::CaseInsensitive) == 0 ||
            value.compare("1") == 0)
        {
            retVal = true;
        }
        else if (value.compare("false", Qt::CaseInsensitive) == 0 ||
            value.compare("no", Qt::CaseInsensitive) == 0 ||
            value.compare("0") == 0)
        {
            retVal = false;
        }

        break;
    }

    case HUpnpDataTypes::bin_hex:
        return value;//.toUtf8().toHex();

    case HUpnpDataTypes::bin_base64:
        return value;//.toUtf8().toBase64();

    case HUpnpDataTypes::uri:
    {
        retVal = QUrl(value);
        break;
    }

    case HUpnpDataTypes::uuid:
        return value;

    default:
        Q_ASSERT(false);
    }

    return retVal;
}

}
}
