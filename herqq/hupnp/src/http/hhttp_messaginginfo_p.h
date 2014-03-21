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

#ifndef HHTTP_MESSAGINGINFO_P_H_
#define HHTTP_MESSAGINGINFO_P_H_

#include <HUpnpCore/HUpnp>
#include <HUpnpCore/HProductTokens>

#include <QtCore/QPair>
#include <QtCore/QString>
#include <QtCore/QPointer>
#include <QtNetwork/QTcpSocket>

class QUrl;

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

namespace Herqq
{

namespace Upnp
{

//
//
//
class H_UPNP_CORE_EXPORT HChunkedInfo
{
private:

    volatile int m_maxChunkSize;
    // if this is non-zero, it means that chunked-encoding should be used
    // if the data to be sent is larger than that of the specified max chunk size
    // and that the max chunk size is this

    volatile int m_minChunkSize;
    // if this is non-zero, it means that when the size of the data to be sent
    // is not known in advance, how big _at least_ each chunk must be in size.

public:

    inline HChunkedInfo(int max = 0) :
        m_maxChunkSize(max), m_minChunkSize(0)
    {
    }

    inline HChunkedInfo(int min, int max) :
        m_maxChunkSize(max), m_minChunkSize(min)
    {
    }

    inline int max() const { return m_maxChunkSize; }
    inline int min() const { return m_minChunkSize; }

    inline void setMax(int arg) { m_maxChunkSize = arg; }
    inline void setMin(int arg) { m_minChunkSize = arg; }
};

//
//
//
class H_UPNP_CORE_EXPORT HMessagingInfo
{
H_DISABLE_COPY(HMessagingInfo)

private:

    QPair<QPointer<QTcpSocket>, bool> m_sock;

    bool m_keepAlive;
    qint32 m_receiveTimeoutForNoData;

    HChunkedInfo m_chunkedInfo;

    QString m_hostInfo;

    QString m_lastErrorDescription;

    qint32 m_msecsToWaitOnSend;

    HProductTokens m_serverTokens;

public:

     //
    // The default timeout in milliseconds that is waited before a read operation
    // is terminated unless _some_ data is received (not necessarily the desired amount).
    //
    static inline qint32 defaultReceiveTimeoutForNoData()
    {
        const qint32 retVal = 5000;
        return retVal;
    }

    explicit HMessagingInfo(
        QPair<QTcpSocket*, bool> sock,
        qint32 receiveTimeoutForNoData = defaultReceiveTimeoutForNoData());

    explicit HMessagingInfo(
        QTcpSocket& sock,
        qint32 receiveTimeoutForNoData = defaultReceiveTimeoutForNoData());

    HMessagingInfo(
        QPair<QTcpSocket*, bool>, bool keepAlive,
        qint32 receiveTimeoutForNoData = defaultReceiveTimeoutForNoData());

    HMessagingInfo(
        QTcpSocket&,
        bool keepAlive,
        qint32 receiveTimeoutForNoData = defaultReceiveTimeoutForNoData());

    ~HMessagingInfo();

    inline QTcpSocket& socket() const
    {
        Q_ASSERT(!m_sock.first.isNull());
        return *m_sock.first;
    }

    inline void setKeepAlive(bool arg)
    {
        m_keepAlive = arg;
    }

    inline bool keepAlive() const
    {
        return m_keepAlive;
    }

    void setHostInfo(const QUrl& hostInfo);

    inline void setHostInfo(const QString& hostInfo)
    {
        m_hostInfo = hostInfo.trimmed();
    }

    QString hostInfo() const;

    inline void setReceiveTimeoutForNoData(qint32 arg)
    {
        m_receiveTimeoutForNoData = arg;
    }

    inline qint32 receiveTimeoutForNoData() const
    {
        return m_receiveTimeoutForNoData;
    }

    inline const HChunkedInfo& chunkedInfo() const
    {
        return m_chunkedInfo;
    }

    inline void setChunkedInfo(const HChunkedInfo& arg)
    {
        m_chunkedInfo = arg;
    }

    inline void setLastErrorDescription(const QString& errDescr)
    {
        m_lastErrorDescription = errDescr;
    }

    inline void setSendWait(qint32 msecsToWait)
    {
        m_msecsToWaitOnSend = msecsToWait;
    }

    inline qint32 sendWait() const
    {
        return m_msecsToWaitOnSend;
    }

    QString lastErrorDescription() const;

    inline void setServerInfo(const HProductTokens& serverTokens)
    {
        m_serverTokens = serverTokens;
    }

    inline HProductTokens serverInfo() const
    {
        return m_serverTokens;
    }
};


}
}

#endif /* HHTTP_MESSAGINGINFO_P_H_ */
