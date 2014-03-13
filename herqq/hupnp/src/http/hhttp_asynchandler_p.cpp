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

#include "hhttp_asynchandler_p.h"
#include "hhttp_messagecreator_p.h"
#include "hhttp_utils_p.h"

#include "../general/hupnp_global_p.h"
#include "../devicehosting/messages/hevent_messages_p.h"

#include <QtNetwork/QTcpSocket>

#include <QtSoapMessage>

namespace Herqq
{

namespace Upnp
{

HHttpAsyncOperation::HHttpAsyncOperation(
    const QByteArray& loggingIdentifier, unsigned int id, HMessagingInfo* mi,
    bool waitingRequest, QObject* parent) :
        QObject(parent),
            m_mi(mi),
            m_dataToSend(),
            m_dataSend(0),
            m_dataSent(0),
            m_state(Internal_NotStarted),
            m_headerRead(0),
            m_dataRead(),
            m_dataToRead(0),
            m_id(id),
            m_loggingIdentifier(loggingIdentifier),
            m_opType(waitingRequest ? ReceiveRequest : ReceiveResponse)
{
    bool ok = connect(
        &m_mi->socket(), SIGNAL(readyRead()), this, SLOT(readyRead()));

    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = connect(
        &m_mi->socket(), SIGNAL(error(QAbstractSocket::SocketError)),
        this, SLOT(error(QAbstractSocket::SocketError)));

    Q_ASSERT(ok);
}

HHttpAsyncOperation::HHttpAsyncOperation(
    const QByteArray& loggingIdentifier, unsigned int id, HMessagingInfo* mi,
    const QByteArray& data, bool sendOnly, QObject* parent) :
        QObject(parent),
            m_mi(mi),
            m_dataToSend(data),
            m_dataSend(0),
            m_dataSent(0),
            m_state(Internal_NotStarted),
            m_headerRead(0),
            m_dataRead(),
            m_dataToRead(0),
            m_id(id),
            m_loggingIdentifier(loggingIdentifier),
            m_opType(sendOnly ? SendOnly : MsgIO)
{
    bool ok = connect(
        &m_mi->socket(), SIGNAL(bytesWritten(qint64)),
        this, SLOT(bytesWritten(qint64)));

    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = connect(
        &m_mi->socket(), SIGNAL(readyRead()), this, SLOT(readyRead()));

    Q_ASSERT(ok);

    ok = connect(
        &m_mi->socket(), SIGNAL(error(QAbstractSocket::SocketError)),
        this, SLOT(error(QAbstractSocket::SocketError)));

    Q_ASSERT(ok);
}

HHttpAsyncOperation::~HHttpAsyncOperation()
{
    delete m_mi;
    delete m_headerRead;
}

void HHttpAsyncOperation::sendChunked()
{
    static const char crlf[] = {"\r\n"};

    // then start sending the data in chunks
    qint64 bytesWritten = 0;

    if (m_dataSent < m_dataToSend.size())
    {
        qint32 dataToSendSize =
            m_dataSend > 0 ? m_dataSend :
                qMin(m_dataToSend.size() - m_dataSent,
                    static_cast<qint64>(m_mi->chunkedInfo().max()));

        if (m_state == Internal_WritingChunkedSizeLine)
        {
            // write the size line of the next chunk
            QByteArray sizeLine;
            sizeLine.setNum(dataToSendSize, 16);
            sizeLine.append(crlf, 2);

            bytesWritten = m_mi->socket().write(sizeLine);
            if (bytesWritten != sizeLine.size())
            {
                m_mi->setLastErrorDescription("failed to send chunked data");
                done_(Internal_Failed);
                return;
            }

            m_state = Internal_WritingChunk;
        }

        // write the chunk
        bytesWritten =
            m_mi->socket().write(m_dataToSend.data() + m_dataSent, dataToSendSize);

        if (bytesWritten < 0)
        {
            m_mi->setLastErrorDescription("failed to send chunked data");
            done_(Internal_Failed);
            return;
        }

        m_dataSent += bytesWritten;

        if (bytesWritten != dataToSendSize)
        {
            m_dataSend = dataToSendSize - bytesWritten;

            // wait for bytesWritten() and then attempt to send the data remaining
            // in the chunk
            return;
        }
        else
        {
            m_dataSend = 0;
        }

        // and after the chunk, write the trailing crlf and start again if there's
        // chunks left
        bytesWritten = m_mi->socket().write(crlf, 2);
        if (bytesWritten != 2)
        {
            m_mi->setLastErrorDescription("failed to send chunked data");
            done_(Internal_Failed);
            return;
        }

        m_state = Internal_WritingChunkedSizeLine;
    }

    if (m_dataSent >= m_dataToSend.size())
    {
        // write the "eof" == zero + crlf
        const char eof[] = "0\r\n";
        m_mi->socket().write(&eof[0], 3);
        m_mi->socket().flush();

        if (m_opType == SendOnly)
        {
            done_(Internal_FinishedSuccessfully);
            return;
        }

        m_state = Internal_ReadingHeader;
    }
}

void HHttpAsyncOperation::readBlob()
{
    QByteArray buf; buf.resize(m_dataToRead+1);
    do
    {
        qint64 retVal = m_mi->socket().read(
            buf.data(), qMin(static_cast<qint64>(buf.size()), m_dataToRead));

        if (retVal < 0)
        {
            m_mi->setLastErrorDescription(
                QString("failed to read data: %1").arg(
                    m_mi->socket().errorString()));

            done_(Internal_Failed);
            return;
        }
        else if (retVal > 0)
        {
            m_dataToRead -= retVal;
            m_dataRead.append(QByteArray(buf.data(), retVal));
        }
        else
        {
            break;
        }
    }
    while(m_dataToRead > 0);

    if (m_dataToRead <= 0)
    {
        done_(Internal_FinishedSuccessfully);
    }
}

bool HHttpAsyncOperation::readChunkedSizeLine()
{
    if (m_mi->socket().bytesAvailable() <= 0)
    {
        return false;
    }

    QByteArray buf;
    if (!HHttpUtils::readLines(m_mi->socket(), buf, 1))
    {
        // No size line. It should be available at this point.
        m_mi->setLastErrorDescription("missing chunk-size line");
        done_(Internal_Failed);
        return false;
    }

    qint32 endOfSize = buf.indexOf(';');
    if (endOfSize < 0)
    {
        // no extensions
        endOfSize = buf.size() - 2; // 2 == crlf
    }
    QByteArray sizeLine = buf.left(endOfSize);

    bool ok = false;
    qint32 chunkSize = sizeLine.toInt(&ok, 16);
    if (!ok || chunkSize < 0)
    {
        m_mi->setLastErrorDescription(
            QString("invalid chunk-size line: %1").arg(
                  QString::fromUtf8(sizeLine)));

        done_(Internal_Failed);
        return false;
    }

    if (chunkSize == 0)
    {
        // the last chunk, ignore possible trailers
        done_(Internal_FinishedSuccessfully);
        return false;
    }

    m_dataToRead = chunkSize;
    m_state = Internal_ReadingChunk;

    return true;
}

bool HHttpAsyncOperation::readChunk()
{
    QByteArray tmp;
    tmp.resize(m_dataToRead);

    qint32 read = m_mi->socket().read(tmp.data(), tmp.size());

    if (read < 0)
    {
        m_mi->setLastErrorDescription(QString(
            "failed to read chunk: %1").arg(m_mi->socket().errorString()));

        done_(Internal_Failed);
        return false;
    }
    else if (read == 0)
    {
        // couldn't read the entire chunk in one pass
        return false;
    }

    tmp.resize(read);
    m_dataRead.append(tmp);

    m_dataToRead -= read;
    if (m_dataToRead > 0)
    {
        // couldn't read the entire chunk in one pass
        return false;
    }

    // if here, the entire chunk data is read.
    // clear the remaining crlf and move to the next chunk

    char c;
    m_mi->socket().getChar(&c);
    m_mi->socket().getChar(&c);

    m_state = Internal_ReadingChunkSizeLine;

    return true;
}

bool HHttpAsyncOperation::readHeader()
{
    if (!HHttpUtils::readLines(m_mi->socket(), m_dataRead, 2))
    {
        m_mi->setLastErrorDescription(QString(
            "failed to read HTTP header: %1").arg(m_mi->socket().errorString()));

        done_(Internal_Failed);
        return false;
    }

    if (m_opType == ReceiveRequest)
    {
        m_headerRead = new HHttpRequestHeader(QString::fromUtf8(m_dataRead));
    }
    else
    {
        m_headerRead = new HHttpResponseHeader(QString::fromUtf8(m_dataRead));
    }

    m_dataRead.clear();

    if (!m_headerRead->isValid())
    {
        m_mi->setLastErrorDescription("read invalid HTTP header");
        done_(Internal_Failed);
        return false;
    }

    m_mi->setKeepAlive(HHttpUtils::keepAlive(*m_headerRead));

    if (m_headerRead->hasContentLength())
    {
        m_dataToRead = m_headerRead->contentLength();
        if (m_dataToRead == 0)
        {
            done_(Internal_FinishedSuccessfully);
            return false;
        }
    }
    else if (m_headerRead->value("TRANSFER-ENCODING") != "chunked")
    {
        done_(Internal_FinishedSuccessfully);
        return false;
    }

    m_state = Internal_ReadingData;
    return true;
}

bool HHttpAsyncOperation::readData()
{
    if (!m_mi->socket().bytesAvailable())
    {
        return false;
    }

    bool chunked = m_headerRead->value("TRANSFER-ENCODING") == "chunked";
    if (chunked)
    {
        if (m_headerRead->hasContentLength())
        {
            m_mi->setLastErrorDescription("read invalid HTTP header where both "
                "TRANSFER-ENCODING and CONTENT-LENGTH where defined");

            done_(Internal_Failed);
            return false;
        }

        m_state = Internal_ReadingChunkSizeLine;
    }
    else
    {
        if (m_headerRead->hasContentLength())
        {
            readBlob();
        }
        else
        {
            // not chunked and content length is not specified ==>
            // no way to know what to expect ==> read all that is available
            QByteArray body = m_mi->socket().readAll();
            m_dataRead.append(body);

            done_(Internal_FinishedSuccessfully);
            return false;
        }
    }

    return true;
}

bool HHttpAsyncOperation::run()
{
    if (m_dataToSend.isEmpty())
    {
        m_state = Internal_ReadingHeader;
        return true;
    }

    if (m_mi->socket().state() != QTcpSocket::ConnectedState)
    {
        m_mi->setLastErrorDescription("socket is not connected");
        return false;
    }

    qint32 indexOfData = m_dataToSend.indexOf("\r\n\r\n");
    Q_ASSERT(indexOfData > 0);

    if (m_mi->chunkedInfo().max() > 0 &&
        m_dataToSend.size() - indexOfData > m_mi->chunkedInfo().max())
    {
        // send the http header first (it is expected that the header has been
        // properly setup for chunked transfer, as it should be, since this is
        // private stuff not influenced by public input)

        qint32 endOfHdr = m_dataToSend.indexOf("\r\n\r\n") + 4;
        m_dataSent = m_mi->socket().write(m_dataToSend.data(), endOfHdr);

        if (m_dataSent != endOfHdr)
        {
            m_mi->setLastErrorDescription(QString(
                "failed to send HTTP header %1").arg(
                    m_mi->socket().errorString()));

            done_(Internal_Failed, false);
            return false;
        }

        m_state = Internal_WritingChunkedSizeLine;
        sendChunked();
    }
    else
    {
        m_dataSent = m_mi->socket().write(m_dataToSend);

        if (m_dataSent < 0)
        {
            m_mi->setLastErrorDescription(
                QString("failed to send data: %1").arg(
                    m_mi->socket().errorString()));

            done_(Internal_Failed, false);
            return false;
        }

        m_state = Internal_WritingBlob;

        if (m_mi->sendWait() > 0)
        {
            if (m_mi->socket().waitForBytesWritten(m_mi->sendWait()))
            {
                bytesWritten(-1);
            }
            else
            {
                m_mi->setLastErrorDescription(QString(
                    "failed to send data %1").arg(m_mi->socket().errorString()));
                done_(Internal_Failed, false);
                return false;
            }
        }
    }

    return true;
}

void HHttpAsyncOperation::done_(InternalState state, bool emitSignal)
{
    m_mi->socket().disconnect(this);

    Q_ASSERT((state == Internal_FinishedSuccessfully && (headerRead() || m_opType == SendOnly)) ||
              state != Internal_FinishedSuccessfully);

    m_state = state;
    if (emitSignal)
    {
        emit done(m_id);
    }
}

void HHttpAsyncOperation::bytesWritten(qint64)
{
    if (m_state == Internal_WritingBlob)
    {
        if (m_dataSent < m_dataToSend.size())
        {
            qint64 dataSent = m_mi->socket().write(
                m_dataToSend.data() + m_dataSent,
                m_dataToSend.size() - m_dataSent);

            if (dataSent < 0)
            {
                m_mi->setLastErrorDescription(
                    QString("failed to send data: %1").arg(
                        m_mi->socket().errorString()));

                done_(Internal_Failed);
                return;
            }

            m_dataSent += dataSent;
        }

        if (m_dataSent >= m_dataToSend.size())
        {
            if (m_opType == SendOnly)
            {
                done_(Internal_FinishedSuccessfully);
            }
            else
            {
                m_state = Internal_ReadingHeader;
            }
        }
    }
    else if (m_state == Internal_WritingChunk ||
             m_state == Internal_WritingChunkedSizeLine)
    {
        sendChunked();
    }
}

void HHttpAsyncOperation::readyRead()
{
    if (m_state == Internal_ReadingHeader)
    {
        if (!readHeader())
        {
            return;
        }
    }

    if (m_state == Internal_ReadingData)
    {
        if (!readData())
        {
            return;
        }
    }

    for(; m_state == Internal_ReadingChunkSizeLine ||
          m_state == Internal_ReadingChunk;)
    {
        // the request contained chunked data

        if (m_state == Internal_ReadingChunkSizeLine)
        {
            if (!readChunkedSizeLine())
            {
                // no more data available at the moment
                return;
            }
        }

        if (m_state == Internal_ReadingChunk)
        {
            if (!readChunk())
            {
                // no more data available at the moment
                return;
            }
        }
    }
}

void HHttpAsyncOperation::error(QAbstractSocket::SocketError err)
{
    if (err != QAbstractSocket::RemoteHostClosedError)
    {
        done_(Internal_Failed);
        return;
    }
    else if (m_state >= Internal_Failed && m_state < Internal_ReadingHeader)
    {
        done_(Internal_Failed);
        return;
    }
    else if (m_dataToRead > 0)
    {
        m_mi->setLastErrorDescription(
            "remote host closed connection before all data could be read");

        done_(Internal_Failed);
        return;
    }
    else if (m_state == Internal_ReadingHeader)
    {
        if (m_dataRead.size() <= 0)
        {
            m_mi->setLastErrorDescription(
                QString("failed to read HTTP header: %1").arg(m_mi->socket().errorString()));
            done_(Internal_Failed);
            return;
        }

        if (m_opType == ReceiveRequest)
        {
            m_headerRead = new HHttpRequestHeader(QString::fromUtf8(m_dataRead));
        }
        else
        {
            m_headerRead = new HHttpResponseHeader(QString::fromUtf8(m_dataRead));
        }

        if (!m_headerRead->isValid())
        {
            m_mi->setLastErrorDescription("read invalid HTTP header");
            done_(Internal_Failed);
            return;
        }
    }

    // at this point a header is successfully read and possibly some data ==>
    // it is up to the user to check the contents of the data to determine was the
    // operation "really" successful
    done_(Internal_FinishedSuccessfully);
}

HHttpAsyncOperation::State HHttpAsyncOperation::state() const
{
    switch(m_state)
    {
    case Internal_Failed:
        return Failed;

    case Internal_NotStarted:
        return NotStarted;

    case Internal_WritingBlob:
    case Internal_WritingChunkedSizeLine:
    case Internal_WritingChunk:
        return Writing;

    case Internal_ReadingHeader:
    case Internal_ReadingData:
    case Internal_ReadingChunkSizeLine:
    case Internal_ReadingChunk:
        return Reading;

    case Internal_FinishedSuccessfully:
        return Succeeded;

    default:
        Q_ASSERT(false);
        return Failed;
    }
}

/*******************************************************************************
 * HHttpAsyncHandler
 ******************************************************************************/
HHttpAsyncHandler::HHttpAsyncHandler(
    const QByteArray& loggingIdentifier, QObject* parent) :
        QObject(parent),
            m_loggingIdentifier(loggingIdentifier), m_operations(),
            m_lastIdUsed(0)
{
}

HHttpAsyncHandler::~HHttpAsyncHandler()
{
}

void HHttpAsyncHandler::done(unsigned int id)
{
    HHttpAsyncOperation* ao = m_operations.value(id);

    Q_ASSERT(ao);
    Q_ASSERT(ao->state() != HHttpAsyncOperation::NotStarted);

    bool ok = ao->disconnect(this);
    Q_ASSERT(ok); Q_UNUSED(ok)

    m_operations.remove(id);

    emit msgIoComplete(ao);
}

HHttpAsyncOperation* HHttpAsyncHandler::msgIo(
    HMessagingInfo* mi, const QByteArray& req)
{
    Q_ASSERT(mi);
    Q_ASSERT(!req.isEmpty());

    HHttpAsyncOperation* ao =
        new HHttpAsyncOperation(
            m_loggingIdentifier, ++m_lastIdUsed, mi, req, false, this);

    bool ok = connect(ao, SIGNAL(done(unsigned int)), this, SLOT(done(unsigned int)));

    Q_ASSERT(ok); Q_UNUSED(ok)

    m_operations.insert(ao->id(), ao);

    if (!ao->run())
    {
        m_operations.remove(ao->id());
        delete ao;
        return 0;
    }

    return ao;
}

HHttpAsyncOperation* HHttpAsyncHandler::msgIo(
    HMessagingInfo* mi, HHttpRequestHeader& reqHdr, const QtSoapMessage& soapMsg)
{
    QByteArray dataToSend =
        HHttpMessageCreator::setupData(
            reqHdr, soapMsg.toXmlString().toUtf8(), *mi, ContentType_TextXml);

    return msgIo(mi, dataToSend);
}

HHttpAsyncOperation* HHttpAsyncHandler::send(
    HMessagingInfo* mi, const QByteArray& data)
{
    Q_ASSERT(mi);
    Q_ASSERT(!data.isEmpty());

    HHttpAsyncOperation* ao =
        new HHttpAsyncOperation(
            m_loggingIdentifier, ++m_lastIdUsed, mi, data, true, this);

    bool ok = connect(ao, SIGNAL(done(unsigned int)), this, SLOT(done(unsigned int)));
    Q_ASSERT(ok); Q_UNUSED(ok)

    m_operations.insert(ao->id(), ao);

    if (!ao->run())
    {
        m_operations.remove(ao->id());
        delete ao;
        return 0;
    }

    return ao;
}

HHttpAsyncOperation* HHttpAsyncHandler::receive(
    HMessagingInfo* mi, bool waitingRequest)
{
    Q_ASSERT(mi);

    HHttpAsyncOperation* ao =
        new HHttpAsyncOperation(
            m_loggingIdentifier, ++m_lastIdUsed, mi, waitingRequest, this);

    bool ok = connect(ao, SIGNAL(done(unsigned int)), this, SLOT(done(unsigned int)));
    Q_ASSERT(ok); Q_UNUSED(ok)

    m_operations.insert(ao->id(), ao);

    if (!ao->run())
    {
        m_operations.remove(ao->id());
        delete ao;
        return 0;
    }

    return ao;
}

}
}
