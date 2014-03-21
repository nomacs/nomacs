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

#ifndef HHTTP_HEADER_P_H
#define HHTTP_HEADER_P_H

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpCore/HUpnp>

#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

//
// The design and implementation of this class were heavily influenced by
// the Qt's QHttpHeader. The main reason this class exists is that Qt
// has deprecated QHttpHeader.
//
class H_UPNP_CORE_EXPORT HHttpHeader
{

private:

    bool parseLine(const QString&);

    virtual bool parseFirstLine(const QString&) = 0;

protected:

    QList<QPair<QString, QString> > m_values;

    bool m_valid;
    int m_majorVersion;
    int m_minorVersion;

    bool parse(const QString&);
    inline void addValue(const QString& key, const QString& value)
    {
        m_values.append(qMakePair(key, value));
    }

    HHttpHeader(const HHttpHeader&);
    HHttpHeader& operator=(const HHttpHeader&);

public:

    HHttpHeader();
    virtual ~HHttpHeader() = 0;

    void setValue(const QString& key, const QString& value);
    bool hasKey(const QString&) const;
    QString value(const QString&) const;

    inline bool hasContentLength() const
    {
        return hasKey("content-length");
    }

    inline uint contentLength() const
    {
        return value("content-length").toUInt();
    }

    inline void setContentLength(int len)
    {
        setValue("content-length", QString::number(len));
    }

    inline bool hasContentType() const
    {
        return hasKey("content-type");
    }

    QString contentType(bool includeCharset=false) const;

    inline void setContentType(const QString& type)
    {
        setValue("content-type", type);
    }

    virtual QString toString() const;

    inline bool isValid() const { return m_valid; }

    inline int majorVersion() const { return m_majorVersion; }
    inline int minorVersion() const { return m_minorVersion; }
};

//
// The design and implementation of this class were heavily influenced by
// the Qt's QHttpResponseHeader. The main reason this class exists is
// that Qt has deprecated QHttpResponseHeader.
//
class H_UPNP_CORE_EXPORT HHttpResponseHeader :
    public HHttpHeader
{
private:

    virtual bool parseFirstLine(const QString& line);

protected:

    int m_statusCode;
    QString m_reasonPhrase;

public:

    HHttpResponseHeader();
    HHttpResponseHeader(const QString &str);
    HHttpResponseHeader(
        int code, const QString& text = QString(),
        int majorVer = 1, int minorVer = 1);

    HHttpResponseHeader(const HHttpResponseHeader&);
    HHttpResponseHeader& operator=(const HHttpResponseHeader&);

    bool setStatusLine(
        int code, const QString& text,
        int majorVer = 1, int minorVer = 1);

    inline int statusCode() const
    {
        return m_statusCode;
    }

    inline QString reasonPhrase() const
    {
        return m_reasonPhrase;
    }

    virtual QString toString() const;
};

//
// The design and implementation of this class were heavily influenced by
// the Qt's QHttpRequestHeader. The main reason this class exists is
// that Qt has deprecated QHttpRequestHeader.
//
class H_UPNP_CORE_EXPORT HHttpRequestHeader :
    public HHttpHeader
{

private:

    virtual bool parseFirstLine(const QString&);

protected:

    QString m_method;
    QString m_path;

public:

    HHttpRequestHeader();

    HHttpRequestHeader(
        const QString& method, const QString& path,
        int majorVer = 1, int minorVer = 1);

    HHttpRequestHeader(const QString&);

    HHttpRequestHeader(const HHttpRequestHeader&);
    HHttpRequestHeader& operator=(const HHttpRequestHeader&);

    bool setRequest(
        const QString& method, const QString& path,
        int majorVer = 1, int minorVer = 1);

    inline QString method() const
    {
        return m_method;
    }

    inline QString path() const
    {
        return m_path;
    }

    virtual QString toString() const;
};

}
}

#endif // HHTTP_HEADER_P_H
