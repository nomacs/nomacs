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

#include "hproduct_tokens.h"

#include "../general/hlogger_p.h"

#include <QtCore/QRegExp>
#include <QtCore/QVector>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HProductToken
 ******************************************************************************/
HProductToken::HProductToken() :
    m_token(), m_productVersion()
{
}

HProductToken::HProductToken(const QString& token, const QString& productVersion) :
    m_token(), m_productVersion()
{
    HLOG(H_AT, H_FUN);

    QString tokenTmp(token.simplified());
    QString productVersionTmp(productVersion.simplified());
    if (tokenTmp.isEmpty() || productVersionTmp.isEmpty())
    {
        HLOG_WARN(QString(
            "Invalid product token. Token: [%1], Product Version: [%2]").arg(
                tokenTmp, productVersionTmp));
    }
    else
    {
        m_token = tokenTmp;
        m_productVersion = productVersionTmp;
    }
}

HProductToken::~HProductToken()
{
}

bool HProductToken::isValid(HValidityCheckLevel checkLevel) const
{
    bool looselyValid = !m_token.isEmpty() && !m_productVersion.isEmpty();

    if (!looselyValid)
    {
        return false;
    }
    else if (checkLevel == LooseChecks)
    {
        return true;
    }

    bool ok = false;
    qint32 separatorIndex = m_productVersion.indexOf('.');
    if (separatorIndex < 0)
    {
        m_productVersion.toInt(&ok);
        return ok;
    }

    m_productVersion.left(separatorIndex).toInt(&ok);
    if (ok)
    {
        m_productVersion.mid(
            separatorIndex+1, m_productVersion.indexOf('.', separatorIndex+1)).toInt(&ok);
    }

    return ok;
}

bool HProductToken::isValidUpnpToken() const
{
    QString vrs = version();

    return (m_token.compare("upnp", Qt::CaseInsensitive) == 0) &&
           (vrs.size() == 3    &&
           (vrs[0]     == '1') &&
            vrs[1]     == '.'  &&
           (vrs[2] == '0' || vrs[2] == '1'));
}

bool HProductToken::isValidDlnaDocToken() const
{
    QString vrs = version();

    bool b = m_token.compare("DLNADOC", Qt::CaseInsensitive) == 0 &&
             vrs.size() >= 3 &&
             vrs[0] == '1' &&
             vrs[1] == '.';
    if (b)
    {
        for(int i = 2; i < vrs.count(); ++i)
        {
            if (!vrs[i].isDigit())
            {
                b = false;
                break;
            }
        }
    }

    return b;
}

QString HProductToken::toString() const
{
    if (!isValid(LooseChecks))
    {
        return QString();
    }

    return QString("%1/%2").arg(m_token, m_productVersion);
}

qint32 HProductToken::minorVersion()
{
    if (!isValid(LooseChecks))
    {
        return -1;
    }

    QString tokenVersion = version();

    qint32 separatorIndex = tokenVersion.indexOf('.');
    if (separatorIndex < 0)
    {
        return -1;
    }

    bool ok = false;

    qint32 minTmp = tokenVersion.mid(
        separatorIndex+1, tokenVersion.indexOf('.', separatorIndex+1)).toInt(&ok);

    return ok ? minTmp : -1;
}

qint32 HProductToken::majorVersion()
{
    if (!isValid(LooseChecks))
    {
        return -1;
    }

    QString tokenVersion = version();

    bool ok = false;
    qint32 majTmp = -1;
    qint32 separatorIndex = tokenVersion.indexOf('.');
    if (separatorIndex < 0)
    {
        majTmp = tokenVersion.toInt(&ok);
        return ok ? majTmp : -1;
    }

    majTmp = tokenVersion.left(separatorIndex).toInt(&ok);
    return ok ? majTmp : -1;
}

bool operator==(const HProductToken& obj1, const HProductToken& obj2)
{
    return obj1.toString() == obj2.toString();
}

bool operator!=(const HProductToken& obj1, const HProductToken& obj2)
{
    return !(obj1 == obj2);
}

/*******************************************************************************
 * HProductTokensPrivate
 ******************************************************************************/
class HProductTokensPrivate :
    public QSharedData
{
H_DISABLE_COPY(HProductTokensPrivate)

private:

    // tries to parse the string into "token/version" pairs
    // the pairs have to be delimited with white-space or commas
    // a pair can contain "trailing" data until the last delimiter after which
    // the token of a new pair is started. for instance, this is valid:
    // token/version (some data; some more data) otherToken/otherVersion finalToken/finalVersion (data)
    bool parse(const QString& tokens)
    {
        HLOG(H_AT, H_FUN);

        QVector<HProductToken> productTokens;

        QString token, buf;

        qint32 i = tokens.indexOf('/'), j = 0, lastDelim = 0;
        if (i < 0)
        {
            return false;
        }

        token = tokens.left(i);
        // the first special case "token/version token/version token/version"
        //                         ^^^^^

        for(i = i + 1; i < tokens.size(); ++i, ++j)
        {
            if (tokens[i] == '/')
            {
                if (lastDelim <= 0)
                {
                    // there must have been at least one space between the previous '/'
                    // and this one. it is an error otherwise.
                    return false;
                }

                HProductToken newToken(token, buf.left(lastDelim));
                if (newToken.isValid(LooseChecks))
                {
                    productTokens.append(newToken);
                }
                else
                {
                    return false;
                }

                token = buf.mid(lastDelim+1);
                buf.clear(); j = -1;

                continue;
            }
            else if (tokens[i] == ' ')
            {
                lastDelim = j;
            }

            buf.append(tokens[i]);
        }

        HProductToken lastToken(token, buf);
        if (lastToken.isValid(LooseChecks))
        {
            productTokens.append(lastToken);
        }
        else
        {
            return false;
        }

        // at this point the provided token string is parsed into
        // valid token/version pairs, but it is not known if the tokens string
        // contained the UPnP token.

        int upnpTokenIndex = -1, dlnadocTokenIndex = -1;
        for(i = 0; i < productTokens.count(); ++i)
        {
            if (productTokens[i].isValidUpnpToken())
            {
                upnpTokenIndex = i;
            }
            else if (productTokens[i].isValidDlnaDocToken())
            {
                dlnadocTokenIndex = i;
            }
        }

        if (upnpTokenIndex >= 0)
        {
            m_upnpTokenIndex = upnpTokenIndex;
            m_productTokens = productTokens;
        }

        m_dlnaTokenIndex = dlnadocTokenIndex;

        return upnpTokenIndex >= 0;
    }

public:

    int m_upnpTokenIndex, m_dlnaTokenIndex;
    QString m_originalTokenString;
    QVector<HProductToken> m_productTokens;

public:

    HProductTokensPrivate() :
        m_upnpTokenIndex(-1), m_dlnaTokenIndex(-1),
        m_originalTokenString(), m_productTokens()
    {
    }

    HProductTokensPrivate(const QString& tokens) :
        m_upnpTokenIndex(-1), m_dlnaTokenIndex(-1),
        m_originalTokenString(tokens.simplified()), m_productTokens()
    {
        HLOG(H_AT, H_FUN);

        bool ok = parse(m_originalTokenString);
        if (ok)
        {
            return;
        }

        if (m_originalTokenString.contains(','))
        {
            // some sloppy UPnP implementations uses the comma as the delimiter.
            // technically, comma could be part of the "version" part of the token,
            // but in practice, it if is present it is used as the delimiter.

            ok = parse(QString(m_originalTokenString).replace(',', ' '));
            if (ok)
            {
                HLOG_WARN_NONSTD(QString(
                    "Comma should not be used as a delimiter in "
                    "product tokens: [%1]").arg(tokens));

                return;
            }
        }

        if (!ok)
        {
            // tokenization failed.
            // fall back for scanning the UPnP/version only
            QRegExp rexp("(\\b|\\s+)UPnP/");
            qint32 index = m_originalTokenString.indexOf(
                rexp, Qt::CaseInsensitive);

            if (index >= 0)
            {
                qint32 matchedLength = rexp.matchedLength();
                qint32 slash = index + matchedLength;
                qint32 nextDelim =
                    m_originalTokenString.indexOf(QRegExp("\\s|,"), slash);

                HProductToken token(
                    m_originalTokenString.mid(index, matchedLength-1),
                    m_originalTokenString.mid(slash,
                        nextDelim < 0 ? -1 : nextDelim-slash));

                if (token.isValidUpnpToken())
                {
                    m_upnpTokenIndex = 0;
                    m_productTokens.push_back(token);
                }
                else
                {
                    HLOG_WARN_NONSTD(QString(
                        "Missing the mandatory UPnP token: [%1]").arg(
                            m_originalTokenString));
                }
            }
            else
            {
                HLOG_WARN_NONSTD(QString(
                    "Missing the mandatory UPnP token: [%1]").arg(
                        m_originalTokenString));
            }
        }
    }
};


/*******************************************************************************
 * HProductTokens
 ******************************************************************************/
HProductTokens::HProductTokens() :
    h_ptr(new HProductTokensPrivate())
{
}

HProductTokens::HProductTokens(const QString& tokens) :
    h_ptr(new HProductTokensPrivate(tokens))
{
}

HProductTokens::HProductTokens(const HProductTokens& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HProductTokens& HProductTokens::operator=(const HProductTokens& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

HProductTokens::~HProductTokens()
{
}

bool HProductTokens::hasUpnpToken() const
{
    return h_ptr->m_upnpTokenIndex >= 0;
}

bool HProductTokens::hasDlnaDocToken() const
{
    return h_ptr->m_dlnaTokenIndex >= 0;
}

bool HProductTokens::isEmpty() const
{
    return h_ptr->m_originalTokenString.isEmpty();
}

HProductToken HProductTokens::upnpToken() const
{
    return h_ptr->m_upnpTokenIndex >= 0 ?
               h_ptr->m_productTokens[h_ptr->m_upnpTokenIndex] : HProductToken();
}

HProductToken HProductTokens::dlnaDocToken() const
{
    return h_ptr->m_dlnaTokenIndex >= 0 ?
               h_ptr->m_productTokens[h_ptr->m_dlnaTokenIndex] : HProductToken();
}

QVector<HProductToken> HProductTokens::tokens() const
{
    return h_ptr->m_productTokens;
}

QString HProductTokens::toString() const
{
    return h_ptr->m_originalTokenString;
}

bool operator==(const HProductTokens& ht1, const HProductTokens& ht2)
{
    return ht1.toString() == ht2.toString();
}

}
}
