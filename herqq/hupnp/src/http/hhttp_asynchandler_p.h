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

#ifndef HTTP_ASYNCHANDLER_P_H_
#define HTTP_ASYNCHANDLER_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpCore/HUpnp>

#include "hhttp_p.h"
#include "hhttp_header_p.h"
#include "hhttp_messaginginfo_p.h"

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtNetwork/QAbstractSocket>

class QtSoapMessage;

namespace Herqq
{

namespace Upnp
{

class HHttpAsyncHandler;

//
//
//
class HHttpAsyncOperation :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HHttpAsyncOperation)
friend class HHttpAsyncHandler;

public:

    enum OpType
    {
        MsgIO,
        SendOnly,
        ReceiveRequest,
        ReceiveResponse
    };

private:

    enum InternalState
    {
        Internal_Failed,
        Internal_NotStarted,
        Internal_WritingBlob,
        Internal_WritingChunkedSizeLine,
        Internal_WritingChunk,
        Internal_ReadingHeader,
        Internal_ReadingData,
        Internal_ReadingChunkSizeLine,
        Internal_ReadingChunk,
        Internal_FinishedSuccessfully
    };

    HMessagingInfo* m_mi;

    QByteArray m_dataToSend;
    // the data which will be sent to the target socket

    qint64 m_dataSend;
    // used only with chunked encoding when a chunk cannot be sent in full and
    // the operation needs to be continued later

    qint64 m_dataSent;
    // the amount of data that has been successfully sent

    InternalState m_state;
    // the current state of this "state machine"

    HHttpHeader* m_headerRead;
    // the http reader read from the target socket
    // (request / response, depends of the setup)

    QByteArray m_dataRead;
    // the response data that is currently read from the target socket

    qint64 m_dataToRead;
    // the amount of data that should be available (once the operation is
    // successfully completed)

    unsigned int m_id;
    // id for the operation

    const QByteArray m_loggingIdentifier;

    OpType m_opType;
    // what the operation is supposed to do

private:

    void sendChunked();

    void readBlob();
    bool readChunkedSizeLine();
    bool readChunk();

    // the return value of these two methods indicate if it is okay to continue
    // the operation. when returned false, the operation has signaled completion
    // and thus must be aborted immediately.
    bool readHeader();
    bool readData();

    bool run();
    void done_(InternalState state, bool emitSignal = true);

private Q_SLOTS:

    void bytesWritten(qint64);
    void readyRead();
    void error(QAbstractSocket::SocketError);

public:

    enum State
    {
        Failed,
        NotStarted,
        Writing,
        Reading,
        Succeeded
    };

    HHttpAsyncOperation(
        const QByteArray& loggingIdentifier, unsigned int id, HMessagingInfo* mi,
        bool waitingRequest, QObject* parent);

    HHttpAsyncOperation(
        const QByteArray& loggingIdentifier, unsigned int id, HMessagingInfo* mi,
        const QByteArray& data, bool sendOnly, QObject* parent);

    virtual ~HHttpAsyncOperation();

    State state() const;

    inline unsigned int id() const { return m_id; }

    // the data of the response
    inline QByteArray dataRead() const { return m_dataRead; }

    // the header of the response
    inline const HHttpHeader* headerRead() const { return m_headerRead; }

    inline HMessagingInfo* messagingInfo() const { return m_mi; }

    inline HMessagingInfo* takeMessagingInfo()
    {
        HMessagingInfo* retVal = m_mi; m_mi = 0;
        return retVal;
    }

    inline OpType opType() const { return m_opType; }

Q_SIGNALS:

    void done(unsigned int);
};

//
// Performs async messaging utilizing the event loop.
// This class is not thread-safe.
//
class H_UPNP_CORE_EXPORT HHttpAsyncHandler :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HHttpAsyncHandler)
friend class HHttpAsyncOperation;

private:

    const QByteArray m_loggingIdentifier;

    QHash<unsigned int, HHttpAsyncOperation*> m_operations;

    unsigned int m_lastIdUsed;

private Q_SLOTS:

    void done(unsigned int);

Q_SIGNALS:

    // user is expected to delete the transferred object
    void msgIoComplete(HHttpAsyncOperation*);

public:

    HHttpAsyncHandler(const QByteArray& loggingIdentifier, QObject* parent);
    virtual ~HHttpAsyncHandler();

    //
    // \param mi
    // \param data contains an entire HTTP message, including headers.
    //
    // \return an object that contains state data for the operation.
    // once the operation is done, user is expected to delete the object, but
    // NOT any sooner!
    HHttpAsyncOperation* msgIo(HMessagingInfo* mi, const QByteArray& data);

    //
    // Helper overload
    //
    HHttpAsyncOperation* msgIo(
        HMessagingInfo*, HHttpRequestHeader&, const QtSoapMessage&);

    //
    //
    //
    HHttpAsyncOperation* send(HMessagingInfo*, const QByteArray& data);

    //
    // waitingRequest == expecting to receive HHttpRequestHeader, otherwise
    // expecting to receive HHttpResponseHeader
    //
    HHttpAsyncOperation* receive(HMessagingInfo*, bool waitingRequest);
};

}
}

#endif /* HTTP_ASYNCHANDLER_P_H_ */
