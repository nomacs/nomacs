/*******************************************************************************************************
 DkNetwork.h
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
 along with this program.  If not, see <http://www.gnu.org/licenses/>..

 *******************************************************************************************************/

#pragma once

#define local_tcp_port_start 45454
#define local_tcp_port_end 45484

#include <QMutex>
#include <QSharedPointer>
#include <QTcpServer>
#include <QThread>

#include "DkConnection.h"

class QMimeData;

namespace nmc
{

// nomacs defines
class DkLocalTcpServer;
class DkLANTcpServer;
class DkLANUdpSocket;

class DkPeer : public QObject
{
    Q_OBJECT

public:
    DkPeer(quint16 port,
           quint16 peerId,
           const QHostAddress &hostAddress,
           quint16 peerServerPort,
           const QString &title,
           DkConnection *connection,
           bool sychronized = false,
           const QString &clientName = "",
           bool showInMenu = false,
           QObject *parent = nullptr);

    ~DkPeer() override;

    bool operator==(const DkPeer &peer) const;

    bool isActive() const
    {
        return mHasChangedRecently;
    };
    void setSynchronized(bool flag);
    bool isSynchronized() const
    {
        return mSychronized;
    };
    bool isLocal() const
    {
        return mHostAddress == QHostAddress::LocalHost;
    };

    quint16 mPeerId;
    quint16 mLocalServerPort;
    quint16 mPeerServerPort;
    QHostAddress mHostAddress;
    QString mClientName;
    QString mTitle;
    DkConnection *mConnection;
    QTimer *mTimer;
    bool mShowInMenu;

signals:
    void sendGoodByeMessage();
private slots:
    void timerTimeout();

private:
    bool mHasChangedRecently = false;
    bool mSychronized;
};

class DkPeerList
{
public:
    DkPeerList();
    bool addPeer(DkPeer *peer);
    bool removePeer(quint16 peerId);
    bool setSynchronized(quint16 peerId, bool synchronized);
    bool setTitle(quint16 peerId, const QString &title);
    bool setShowInMenu(quint16 peerId, bool showInMenu);
    QList<DkPeer *> getPeerList();
    DkPeer *getPeerById(quint16 id);
    DkPeer *getPeerByAddress(const QHostAddress &address, quint16 port) const;

    QList<DkPeer *> getSynchronizedPeers() const;
    QList<quint16> getSynchronizedPeerServerPorts() const;
    QList<DkPeer *> getActivePeers() const;

    DkPeer *getPeerByServerport(quint16 port) const;
    bool alreadyConnectedTo(const QHostAddress &address, quint16 port) const;
    void print() const;

private:
    QMultiHash<quint16, DkPeer *> peerList;
};

class DkClientManager : public QObject
{
    Q_OBJECT
public:
    explicit DkClientManager(const QString &title, QObject *parent = nullptr);
    ~DkClientManager() override;
    virtual QList<DkPeer *> getPeerList() = 0;

signals:
    void receivedTransformation(QTransform transform, QTransform imgTransform, QPointF canvasSize);
    void receivedPosition(QRect position, bool opacity, bool overlaid);
    void receivedNewFile(qint16 op, const QString &filename);
    void receivedImage(const QImage &image);
    void receivedImageTitle(const QString &title);
    void sendInfoSignal(const QString &msg, int time = 3000);
    void sendGreetingMessage(const QString &title);
    void sendSynchronizeMessage();
    void sendDisableSynchronizeMessage();
    void sendNewTitleMessage(const QString &newtitle);
    void sendNewPositionMessage(QRect position, bool opacity, bool overlaid);
    void sendNewTransformMessage(QTransform transform, QTransform imgTransform, QPointF canvasSize);
    void sendNewFileMessage(qint16 op, const QString &filename);
    void sendNewImageMessage(QImage image, const QString &title);
    void sendNewUpcomingImageMessage(const QString &imageTitle);
    void sendGoodByeMessage();
    void synchronizedPeersListChanged(QList<quint16> newList);
    void updateConnectionSignal(const QString &);
    void clientConnectedSignal(bool);

    void receivedQuit();

public slots:
    virtual void synchronizeWith(quint16 peerId) = 0;
    virtual void synchronizeWithServerPort(quint16 port) = 0;
    virtual void stopSynchronizeWith(quint16 peerId) = 0;
    virtual void sendTitle(const QString &newTitle);
    void sendTransform(QTransform transform, QTransform imgTransform, QPointF canvasSize);
    void sendPosition(QRect newRect, bool overlaid);

    void sendNewFile(qint16 op, const QString &filename);
    virtual void sendNewImage(QImage, const QString &) {}; // dummy
    void sendGoodByeToAll();

protected slots:
    void newConnection(int socketDescriptor);
    virtual void connectionReadyForUse(quint16 peerId, const QString &title, DkConnection *connection);
    virtual void connectionSynchronized(QList<quint16> synchronizedPeersOfOtherClient, DkConnection *connection) = 0;
    virtual void connectionStopSynchronized(DkConnection *connection) = 0;
    virtual void connectionSentNewTitle(DkConnection *connection, const QString &newTitle);
    virtual void connectionReceivedTransformation(DkConnection *connection,
                                                  const QTransform &transform,
                                                  const QTransform &imgTransform,
                                                  const QPointF &canvasSize);
    virtual void connectionReceivedPosition(DkConnection *connection, const QRect &rect, bool opacity, bool overlaid);
    virtual void connectionReceivedNewFile(DkConnection *connection, qint16 op, const QString &filename);
    virtual void connectionReceivedGoodBye(DkConnection *connection);
    void connectionShowStatusMessage(DkConnection *connection, const QString &msg);
    void disconnected();

protected:
    void removeConnection(DkConnection *connection);
    void connectConnection(DkConnection *connection);
    virtual DkConnection *createConnection() = 0;
    QString listConnections(QList<DkPeer *> peers, bool connected);

    DkPeerList mPeerList;
    QString mCurrentTitle;
    quint16 mNewPeerId;
    QList<DkConnection *> mStartUpConnections;
};

class DkLocalClientManager : public DkClientManager
{
    Q_OBJECT

public:
    explicit DkLocalClientManager(const QString &title, QObject *parent = nullptr);
    QList<DkPeer *> getPeerList() override;
    quint16 getServerPort() const;

    QMimeData *mimeData() const;
    void startServer();

signals:
    void receivedQuit();
    void sendQuitMessage();

public slots:
    void stopSynchronizeWith(quint16 peerId) override;
    void synchronizeWithServerPort(quint16 port) override;
    void synchronizeWith(quint16 peerId) override;
    void sendArrangeInstances(bool overlaid);
    void sendQuitMessageToPeers();
    void connectToNomacs();
    void connectAll();

private slots:
    void connectionSynchronized(QList<quint16> synchronizedPeersOfOtherClient, DkConnection *connection) override;
    void connectionStopSynchronized(DkConnection *connection) override;
    void connectionReceivedQuit();

private:
    DkLocalConnection *createConnection() final; // called from constructor, prevent override in subclasses
    void searchForOtherClients();

    DkLocalTcpServer *mServer;
};

class DkLocalTcpServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit DkLocalTcpServer(QObject *parent = nullptr);

signals:
    void serverReiceivedNewConnection(int DkDescriptor);

protected:
    void incomingConnection(qintptr socketDescriptor) override;
};

class DllCoreExport DkSyncManager
{
public:
    static DkSyncManager &inst();

    // singleton
    DkSyncManager(DkSyncManager const &) = delete;
    void operator=(DkSyncManager const &) = delete;

    DkClientManager *client();

private:
    DkSyncManager();

    DkLocalClientManager *mClient = nullptr;
};

}
