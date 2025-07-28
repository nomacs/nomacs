/*******************************************************************************************************
 DkConnection.cpp
 Created on:	20.07.2011

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2013 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2013 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2013 Florian Kleber <florian@nomacs.org>

 This file is part of nomacs.

 nomacs is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 nomacs is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 *******************************************************************************************************/

#include "DkConnection.h"
#include "DkSettings.h"

#pragma warning(push, 0) // no warnings from includes - begin
#include <QBuffer>
#include <QByteArray>
#include <QDebug>
#include <QHostInfo>
#include <QThread>
#include <QTimer>
#pragma warning(pop) // no warnings from includes - end

namespace nmc
{

// DkConnection --------------------------------------------------------------------

DkConnection::DkConnection(QObject *parent)
    : QTcpSocket(parent)
{
    mNumBytesForCurrentDataType = -1;
    mIsGreetingMessageSent = false;
    mIsSynchronizeMessageSent = false;
    connectionCreated = false;
    mSynchronizedTimer = new QTimer(this);

    connect(mSynchronizedTimer, &QTimer::timeout, this, &DkConnection::synchronizedTimerTimeout);
    connect(this, &DkConnection::readyRead, this, &DkConnection::processReadyRead);

    setReadBufferSize(MaxBufferSize);
}

void DkConnection::setTitle(const QString &newTitle)
{
    mCurrentTitle = newTitle;
}

void DkConnection::sendStartSynchronizeMessage()
{
    // qDebug() << "sending Synchronize Message to " << this->peerName() << ":" << this->peerPort();
    if (mIsSynchronizeMessageSent == false) // initialize sync message, not the response
        mSynchronizedTimer->start(1000);

    QByteArray ba;
    QDataStream ds(&ba, QIODevice::ReadWrite);
    ds << quint16(mSynchronizedPeersServerPorts.size());
    for (int i = 0; i < mSynchronizedPeersServerPorts.size(); i++) {
        qDebug() << "mSynchronizedPeersServerPorts: " << mSynchronizedPeersServerPorts[i];
        ds << mSynchronizedPeersServerPorts[i];
    }
    // QByteArray data = "SYNCHRONIZE" + SeparatorToken + QByteArray::number(synchronize.size()) + SeparatorToken + synchronize;
    QByteArray data = "STARTSYNCHRONIZE";
    data.append(SeparatorToken).append(QByteArray::number(ba.size())).append(SeparatorToken).append(ba);
    qDebug() << "sending startsynchronize:" << data;
    if (write(data) == data.size())
        mIsSynchronizeMessageSent = true;
}

void DkConnection::sendStopSynchronizeMessage()
{
    if (mState == Synchronized) { // only send message if connection is synchronized
        // qDebug() << "sending disable synchronize Message to " << this->peerName() << ":" << this->peerPort();
        QByteArray synchronize = "disable synchronizing";
        // QByteArray data = "DISABLESYNCHRONIZE" + SeparatorToken + QByteArray::number(synchronize.size()) + SeparatorToken + synchronize;
        QByteArray data = "STOPSYNCHRONIZE";
        data.append(SeparatorToken).append(QByteArray::number(synchronize.size())).append(SeparatorToken).append(synchronize);
        if (write(data) == data.size())
            mIsSynchronizeMessageSent = false;
        mState = ReadyForUse;
    }
}

void DkConnection::sendNewTitleMessage(const QString &newtitle)
{
    mCurrentTitle = newtitle;
    // qDebug() << "sending new Title (\"" << newtitle << "\") Message to " << this->peerName() << ":" << this->peerPort();

    QByteArray newTitleBA = newtitle.toUtf8();
    // QByteArray data = "NEWTITLE" + SeparatorToken + QByteArray::number(newTitleBA.size()) + SeparatorToken + newTitleBA;
    QByteArray data = "NEWTITLE";
    data.append(SeparatorToken).append(QByteArray::number(newTitleBA.size())).append(SeparatorToken).append(newTitleBA);
    write(data);
}

void DkConnection::sendNewPositionMessage(QRect position, bool opacity, bool overlaid)
{
    // qDebug() << "sending new Position to " << this->peerName() << ":" << this->peerPort();
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::ReadWrite);
    ds << position;
    ds << opacity;
    ds << overlaid;

    // QByteArray data = "NEWTITLE" + SeparatorToken + QByteArray::number(ba.size()) + SeparatorToken + ba;
    QByteArray data = "NEWPOSITION";
    data.append(SeparatorToken).append(QByteArray::number(ba.size())).append(SeparatorToken).append(ba);
    write(data);
}

void DkConnection::sendNewTransformMessage(QTransform transform, QTransform imgTransform, QPointF canvasSize)
{
    // qDebug() << "sending new Transform Message to " << this->peerName() << ":" << this->peerPort();
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::ReadWrite);
    ds << transform;
    ds << imgTransform;
    ds << canvasSize;

    // QByteArray data = "NEWTRANSFORM" + SeparatorToken + QByteArray::number(ba.size()) + SeparatorToken + ba;
    QByteArray data = "NEWTRANSFORM";
    data.append(SeparatorToken).append(QByteArray::number(ba.size())).append(SeparatorToken).append(ba);
    write(data);
}

void DkConnection::sendNewFileMessage(qint16 op, const QString &filename)
{
    // qDebug() << "sending new File Message to " << this->peerName() << ":" << this->peerPort();
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::ReadWrite);
    ds << op;
    ds << filename;
    QByteArray data = "NEWFILE";
    data.append(SeparatorToken).append(QByteArray::number(ba.size())).append(SeparatorToken).append(ba);
    write(data);
}

void DkConnection::sendNewGoodbyeMessage()
{
    // qDebug() << "sending good bye to " << peerName() << ":" << this->peerPort();

    QByteArray ba = "GoodBye"; // scherz?
    QByteArray data = "GOODBYE";
    data.append(SeparatorToken).append(QByteArray::number(ba.size())).append(SeparatorToken).append(ba);
    write(data);
    waitForBytesWritten();
}

void DkConnection::synchronizedPeersListChanged(QList<quint16> newList)
{
    mSynchronizedPeersServerPorts = newList;
}

bool DkConnection::readProtocolHeader()
{
    QByteArray greetingBA = QByteArray("GREETING").append(SeparatorToken);
    QByteArray synchronizeBA = QByteArray("STARTSYNCHRONIZE").append(SeparatorToken);
    QByteArray disableSynchronizeBA = QByteArray("STOPSYNCHRONIZE").append(SeparatorToken);
    QByteArray newtitleBA = QByteArray("NEWTITLE").append(SeparatorToken);
    QByteArray newtransformBA = QByteArray("NEWTRANSFORM").append(SeparatorToken);
    QByteArray newpositionBA = QByteArray("NEWPOSITION").append(SeparatorToken);
    QByteArray newFileBA = QByteArray("NEWFILE").append(SeparatorToken);
    QByteArray goodbyeBA = QByteArray("GOODBYE").append(SeparatorToken);

    if (mBuffer == greetingBA) {
        // qDebug() << "Greeting received from:" << this->peerAddress() << ":" << this->peerPort();
        mCurrentDataType = Greeting;
    } else if (mBuffer == synchronizeBA) {
        // qDebug() << "Synchronize received from:" << this->peerAddress() << ":" << this->peerPort();
        mCurrentDataType = startSynchronize;
    } else if (mBuffer == disableSynchronizeBA) {
        // qDebug() << "StopSynchronize received from:" << this->peerAddress() << ":" << this->peerPort();
        mCurrentDataType = stopSynchronize;
    } else if (mBuffer == newtitleBA) {
        // qDebug() << "New Title received from:" << this->peerAddress() << ":" << this->peerPort();
        mCurrentDataType = newTitle;
    } else if (mBuffer == newtransformBA) {
        // qDebug() << "New Transform received from:" << this->peerAddress() << ":" << this->peerPort();
        mCurrentDataType = newTransform;
    } else if (mBuffer == newpositionBA) {
        // qDebug() << "New Position received from:" << this->peerAddress() << ":" << this->peerPort();
        mCurrentDataType = newPosition;
    } else if (mBuffer == newFileBA) {
        // qDebug() << "New File received from:" << this->peerAddress() << ":" << this->peerPort();
        mCurrentDataType = newFile;
    } else if (mBuffer == goodbyeBA) {
        // qDebug() << "Goodbye received from:" << this->peerAddress() << ":" << this->peerPort();
        mCurrentDataType = GoodBye;
    } else {
        qDebug() << QString(mBuffer);
        qDebug() << "Undefined received from:" << this->peerAddress() << ":" << this->peerPort();
        mCurrentDataType = Undefined;
        // abort();
        // return false;
        return true;
    }

    mBuffer.clear();
    mNumBytesForCurrentDataType = dataLengthForCurrentDataType();
    return true;
}

int DkConnection::readDataIntoBuffer(int maxSize)
{
    if (maxSize > MaxBufferSize)
        return 0;

    int numBytesBeforeRead = mBuffer.size();
    if (numBytesBeforeRead == MaxBufferSize) {
        qDebug() << "DkConnection::readDataIntoBuffer: Connection aborted";
        abort();
        return 0;
    }

    while (bytesAvailable() > 0 && mBuffer.size() < maxSize) {
        mBuffer.append(read(1));
        if (mBuffer.endsWith(SeparatorToken)) {
            break;
        }
    }
    return mBuffer.size() - numBytesBeforeRead;
}

bool DkConnection::hasEnoughData()
{
    if (mNumBytesForCurrentDataType <= 0) {
        mNumBytesForCurrentDataType = dataLengthForCurrentDataType();
    }

    // qDebug() << "numBytesForCurrentDataType:" << numBytesForCurrentDataType;
    // qDebug() << "bytesAvailable:" << bytesAvailable();
    // qDebug() << "buffer size:" << buffer.size();

    if (bytesAvailable() < mNumBytesForCurrentDataType || mNumBytesForCurrentDataType <= 0) {
        return false;
    }

    return true;
}

int DkConnection::dataLengthForCurrentDataType()
{
    if (bytesAvailable() <= 0 || readDataIntoBuffer() <= 0 || !mBuffer.endsWith(SeparatorToken))
        return 0;

    mBuffer.chop(1);
    int number = mBuffer.toInt();
    mBuffer.clear();
    return number;
}

void DkConnection::processReadyRead()
{
    if (readDataIntoBuffer() <= 0)
        return;
    if (!readProtocolHeader())
        return;
    checkState();

    readWhileBytesAvailable();
}

void DkConnection::checkState()
{
    if (mState == WaitingForGreeting) {
        if (mCurrentDataType != Greeting) {
            abort();
            return;
        }

        if (!hasEnoughData())
            return;

        mBuffer = read(mNumBytesForCurrentDataType);
        if (mBuffer.size() != mNumBytesForCurrentDataType) {
            abort();
            return;
        }

        if (!isValid()) {
            abort();
            return;
        }

        if (!mIsGreetingMessageSent)
            sendGreetingMessage(mCurrentTitle);

        mState = ReadyForUse;
        mPortOfPeer = peerPort(); // save peer port ... otherwise connections where this instance is server can not be removed from peerList

        readGreetingMessage();

        mBuffer.clear();
        mNumBytesForCurrentDataType = 0;
        mCurrentDataType = Undefined;
        return;
    }

    if (mState == ReadyForUse && mCurrentDataType == startSynchronize) {
        if (!hasEnoughData())
            return;

        mBuffer = read(mNumBytesForCurrentDataType);
        if (mBuffer.size() != mNumBytesForCurrentDataType) {
            abort();
            return;
        }

        QDataStream ds(mBuffer);
        QList<quint16> synchronizedPeersOfOtherInstance;
        quint16 numberOfSynchronizedPeers;
        ds >> numberOfSynchronizedPeers;

        // qDebug() << "other client is sychronized with: ";
        for (int i = 0; i < numberOfSynchronizedPeers; i++) {
            quint16 peerId;
            ds >> peerId;
            synchronizedPeersOfOtherInstance.push_back(peerId);
            // qDebug() << peerId;
        }
        mCurrentDataType = Undefined;
        mNumBytesForCurrentDataType = 0;
        mBuffer.clear();

        if (!isValid()) {
            abort();
            return;
        }

        mState = Synchronized;
        if (!mIsSynchronizeMessageSent)
            sendStartSynchronizeMessage();

        mSynchronizedTimer->stop();
        emit connectionStartSynchronize(synchronizedPeersOfOtherInstance, this);
        return;
    }

    if (mState == Synchronized && mCurrentDataType == stopSynchronize) {
        mState = ReadyForUse;
        this->mIsSynchronizeMessageSent = false;
        emit connectionStopSynchronize(this);
        mBuffer = read(mNumBytesForCurrentDataType);
        if (mBuffer.size() != mNumBytesForCurrentDataType) {
            abort();
            return;
        }

        mCurrentDataType = Undefined;
        mNumBytesForCurrentDataType = 0;
        mBuffer.clear();

        return;
    }

    if (mCurrentDataType == GoodBye) {
        // qDebug() << "received GoodBye from " << peerAddress() << ":" << peerPort();
        emit connectionGoodBye(this);
        mCurrentDataType = Undefined;
        mNumBytesForCurrentDataType = 0;
        mBuffer.clear();
        abort();
        return;
    }
}

void DkConnection::readWhileBytesAvailable()
{
    do {
        if (mCurrentDataType == Undefined) {
            if (readDataIntoBuffer() <= 0)
                return;
            if (!readProtocolHeader())
                return;
            checkState();
        }
        if (!hasEnoughData()) {
            return;
        }

        mBuffer = read(mNumBytesForCurrentDataType);
        if (mBuffer.size() != mNumBytesForCurrentDataType) {
            abort();
            return;
        }
        processData();

    } while (bytesAvailable() > 0);
}

bool DkConnection::readDataTypeIntoBuffer()
{
    mBuffer = read(mNumBytesForCurrentDataType);
    if (mBuffer.size() != mNumBytesForCurrentDataType) {
        abort();
        return false;
    }
    return true;
}

void DkConnection::processData()
{
    switch (mCurrentDataType) {
    case newTitle:
        emit connectionTitleHasChanged(this, QString::fromUtf8(mBuffer));
        break;
    case newPosition: {
        if (mState == Synchronized) {
            QRect rect;
            bool opacity;
            bool overlaid;
            QDataStream ds(mBuffer);
            ds >> rect;
            ds >> opacity;
            ds >> overlaid;
            emit connectionNewPosition(this, rect, opacity, overlaid);
        }
        break;
    }
    case newTransform: {
        if (mState == Synchronized) {
            QTransform transform;
            QTransform imgTransform;
            QPointF canvasSize;
            QDataStream dsTransform(mBuffer);
            dsTransform >> transform;
            dsTransform >> imgTransform;
            dsTransform >> canvasSize;
            emit connectionNewTransform(this, transform, imgTransform, canvasSize);
        }
        break;
    }
    case newFile: {
        if (mState == Synchronized) {
            qint16 op;
            QString filename;

            QDataStream dsTransform(mBuffer);
            dsTransform >> op;
            dsTransform >> filename;
            emit connectionNewFile(this, op, filename);
        }
        break;
    }
    default:
        break;
    }

    mCurrentDataType = Undefined;
    mNumBytesForCurrentDataType = 0;
    mBuffer.clear();
}

void DkConnection::synchronizedTimerTimeout()
{
    mSynchronizedTimer->stop();
    emit connectionStopSynchronize(this);
}

// DkLocalConnection --------------------------------------------------------------------
DkLocalConnection::DkLocalConnection(QObject *parent /* =0 */)
    : DkConnection(parent)
{
}

void DkLocalConnection::processReadyRead()
{
    if (mCurrentLocalDataType == Quit) { // long message (copied from lan connection) -> does this work here correctly?
        readWhileBytesAvailable();
        return;
    }

    // if (readDataIntoBuffer() <= 0)
    //	return;
    // if (!readProtocolHeader())
    //	return;

    DkConnection::processReadyRead();
}

void DkLocalConnection::processData()
{
    if (mCurrentLocalDataType == Quit) {
        emit connectionQuitReceived();
    }

    DkConnection::processData();
}

bool DkLocalConnection::readProtocolHeader()
{
    QByteArray quitBA = QByteArray("QUIT").append(SeparatorToken);

    if (mBuffer == quitBA) {
        mCurrentLocalDataType = Quit;
    } else {
        return DkConnection::readProtocolHeader();
    }

    mBuffer.clear();
    mNumBytesForCurrentDataType = dataLengthForCurrentDataType();
    return true;
}

void DkLocalConnection::sendGreetingMessage(const QString &currentTitle)
{
    mCurrentTitle = currentTitle;
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::ReadWrite);
    ds << mLocalTcpServerPort;
    ds << mCurrentTitle;

    // qDebug() << "title: " << mCurrentTitle;
    // qDebug() << "local tcp: " << mLocalTcpServerPort;
    // qDebug() << "peer id: " << mPeerId;

    QByteArray data = "GREETING";
    data.append(SeparatorToken);
    data.append(QByteArray::number(ba.size()));
    data.append(SeparatorToken);
    data.append(ba);

    // qDebug() << "greeting message: " << data;

    if (write(data) == data.size()) {
        mIsGreetingMessageSent = true;
    }
}

void DkLocalConnection::readGreetingMessage()
{
    QString title;
    QDataStream ds(mBuffer);
    ds >> this->mPeerServerPort;
    ds >> title;

    // qDebug() << "emitting readyForUse";
    emit connectionReadyForUse(mPeerServerPort, title, this);
}

void DkLocalConnection::sendQuitMessage()
{
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::ReadWrite);
    ds << "updating";

    QByteArray data = "QUIT";
    data.append(SeparatorToken);
    data.append(QByteArray::number(ba.size()));
    data.append(SeparatorToken);
    data.append(ba);

    if (write(data) == data.size()) {
        mIsGreetingMessageSent = true;
    }
}

}
