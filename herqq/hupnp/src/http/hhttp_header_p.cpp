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

#include "hhttp_header_p.h"

#include <QtCore/QStringList>

namespace
{
int searchKey(
    const QString& key,
    const QList<QPair<QString, QString> >& values)
{
    QString lowCaseKey = key.toLower();
    for (int i = 0; i < values.size(); ++i)
    {
        if (values[i].first.toLower() == lowCaseKey)
        {
            return i;
        }
    }
    return -1;
}

bool parseVersion(const QString& version, int* major, int* minor)
{
    if (version.length() >= 8 &&
        version.left(5) == "HTTP/" &&
        version[5].isDigit() && version[6] == '.' &&
        version[7].isDigit())
    {
        *major = version[5].toLatin1() - '0';
        *minor = version[7].toLatin1() - '0';
        return true;
    }

    return false;
}
}

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HHttpHeader
 ******************************************************************************/
HHttpHeader::HHttpHeader() :
    m_values(), m_valid(false), m_majorVersion(0), m_minorVersion(0)
{
}

HHttpHeader::~HHttpHeader()
{
}

HHttpHeader::HHttpHeader(const HHttpHeader& other) :
    m_values(), m_valid(false), m_majorVersion(0), m_minorVersion(0)
{
    Q_ASSERT(this != &other);

    m_values = other.m_values;
    m_valid = other.m_valid;
    m_majorVersion = other.m_majorVersion;
    m_minorVersion = other.m_minorVersion;
}

HHttpHeader& HHttpHeader::operator=(const HHttpHeader& other)
{
    Q_ASSERT(this != &other);

    m_majorVersion = other.m_majorVersion;
    m_minorVersion = other.m_minorVersion;
    m_valid = other.m_valid;
    m_values = other.m_values;

    return *this;
}

bool HHttpHeader::parse(const QString& str)
{
    QStringList lines = str.trimmed().split("\r\n");

    if (lines.isEmpty())
    {
        return false;
    }

    parseFirstLine(lines[0]);
    lines.removeFirst();

    foreach(const QString& line, lines)
    {
        if (line.isEmpty())
        {
            break;
        }
        else if (!parseLine(line))
        {
            m_valid = false;
            return false;
        }
    }

    return true;
}

QString HHttpHeader::value(const QString& key) const
{
    int index = searchKey(key, m_values);
    return index >= 0 ? m_values[index].second : QString();
}

bool HHttpHeader::hasKey(const QString& key) const
{
    return searchKey(key, m_values) >= 0;
}

void HHttpHeader::setValue(const QString& key, const QString& value)
{
    int index = searchKey(key, m_values);
    if (index >= 0)
    {
        m_values[index].second = value;
    }
    else
    {
        addValue(key, value);
    }
}

bool HHttpHeader::parseLine(const QString& line)
{
    int i = line.indexOf(QLatin1Char(':'));
    if (i == -1)
    {
        return false;
    }

    addValue(line.left(i).trimmed(), line.mid(i + 1).trimmed());

    return true;
}

QString HHttpHeader::toString() const
{
    if (!isValid())
    {
        return "";
    }

    QString retVal;

    QList<QPair<QString, QString> >::const_iterator it = m_values.constBegin();
    for (; it != m_values.constEnd(); ++it) {
        retVal.append((*it).first)
              .append(": ")
              .append((*it).second)
              .append("\r\n");
    }

    return retVal;
}

QString HHttpHeader::contentType(bool includeCharset) const
{
    QString type = value("content-type");
    if (type.isEmpty())
    {
        return type;
    }

    if (includeCharset)
    {
        return type.trimmed();
    }

    int pos = type.indexOf(';');
    if (pos == -1)
    {
        return type;
    }

    return type.left(pos).trimmed();
}

/*******************************************************************************
 * HHttpResponseHeader
 ******************************************************************************/
HHttpResponseHeader::HHttpResponseHeader() :
    HHttpHeader(), m_statusCode(0), m_reasonPhrase()
{
}

HHttpResponseHeader::HHttpResponseHeader(const QString& str) :
    HHttpHeader(), m_statusCode(0), m_reasonPhrase()
{
    if (parse(str))
    {
        m_valid = true;
    }
}

HHttpResponseHeader::HHttpResponseHeader(
    int code, const QString& text, int majorVer, int minorVer) :
        HHttpHeader(),
            m_statusCode(0), m_reasonPhrase()
{
    setStatusLine(code, text, majorVer, minorVer);
}

HHttpResponseHeader::HHttpResponseHeader(const HHttpResponseHeader& other) :
    HHttpHeader(other),
        m_statusCode(other.m_statusCode), m_reasonPhrase(other.m_reasonPhrase)
{
}

HHttpResponseHeader& HHttpResponseHeader::operator=(
    const HHttpResponseHeader& other)
{
    HHttpHeader::operator=(other);

    m_reasonPhrase = other.m_reasonPhrase;
    m_statusCode = other.m_statusCode;

    return *this;
}

bool HHttpResponseHeader::setStatusLine(
    int code, const QString& text, int majorVer, int minorVer)
{
    if (code < 0 || text.simplified().isEmpty())
    {
        return false;
    }

    m_statusCode = code;
    m_reasonPhrase = text.simplified();
    m_majorVersion = majorVer;
    m_minorVersion = minorVer;
    m_valid = true;

    return true;
}

bool HHttpResponseHeader::parseFirstLine(const QString& constLine)
{
    QString line = constLine.simplified();
    if (line.length() < 10)
    {
        return false;
    }

    if (!parseVersion(line, &m_majorVersion, &m_minorVersion))
    {
        return false;
    }

    if (line[8] == ' ' && line[9].isDigit())
    {
        int pos = line.indexOf(' ', 9);
        if (pos != -1)
        {
            m_reasonPhrase = line.mid(pos + 1);
            m_statusCode = line.mid(9, pos - 9).toInt();
        }
        else {
            m_statusCode = line.mid(9).toInt();
            m_reasonPhrase.clear();
        }

        return true;
    }

    return false;
}

QString HHttpResponseHeader::toString() const
{
    if (!isValid())
    {
        return "";
    }

    return QString("HTTP/%1.%2 %3 %4\r\n%5\r\n").arg(m_majorVersion).arg(
        m_minorVersion).arg(m_statusCode).arg(m_reasonPhrase).arg(
            HHttpHeader::toString());
}

/*******************************************************************************
 * HHttpRequestHeader
 ******************************************************************************/
HHttpRequestHeader::HHttpRequestHeader() :
    HHttpHeader(),
        m_method(), m_path()
{
}

HHttpRequestHeader::HHttpRequestHeader(
    const QString& method, const QString& path, int majorVer, int minorVer) :
        HHttpHeader(),
            m_method(), m_path()
{
    setRequest(method, path, majorVer, minorVer);
}

HHttpRequestHeader::HHttpRequestHeader(const QString& str) :
    HHttpHeader(),
        m_method(), m_path()
{
    if (parse(str))
    {
        m_valid = true;
    }
}

HHttpRequestHeader::HHttpRequestHeader(const HHttpRequestHeader& other) :
    HHttpHeader(other),
        m_method(other.m_method), m_path(other.m_path)
{
}

HHttpRequestHeader& HHttpRequestHeader::operator=(
    const HHttpRequestHeader& other)
{
    HHttpHeader::operator=(other);

    m_method = other.m_method;
    m_path = other.m_path;

    return *this;
}

bool HHttpRequestHeader::setRequest(
    const QString& method, const QString& path, int majorVer, int minorVer)
{
    if (method.simplified().isEmpty())
    {
        return false;
    }

    m_method = method.simplified();
    m_path = path;
    m_majorVersion = majorVer;
    m_minorVersion = minorVer;
    m_valid = true;

    return true;
}

bool HHttpRequestHeader::parseFirstLine(const QString& line)
{
    QStringList list = line.simplified().split(" ");
    if (list.size() > 0)
    {
        m_method = list[0];
        if (list.size() > 1)
        {
            m_path = list[1];
            if (list.size() > 2)
            {
                return parseVersion(list[2], &m_majorVersion, &m_minorVersion);
            }
        }
    }

    return false;
}

QString HHttpRequestHeader::toString() const
{
    if (!isValid())
    {
        return "";
    }

    return QString("%1 %2 HTTP/%3.%4\r\n%5\r\n").arg(
        m_method, m_path, QString::number(m_majorVersion),
        QString::number(m_minorVersion), HHttpHeader::toString());
}

}
}
