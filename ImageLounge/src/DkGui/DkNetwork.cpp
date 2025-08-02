/*******************************************************************************************************
 DkNetwork.cpp
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

#include "DkNetwork.h"
#include "DkActionManager.h"
#include "DkControlWidget.h" // needed for a connection
#include "DkSettings.h"
#include "DkTimer.h"
#include "DkUtils.h"

#pragma warning(push, 0) // no warnings from includes - begin
#include <QAbstractButton>
#include <QApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QHostInfo>
#include <QList>
#include <QMessageBox>
#include <QMimeData>
#include <QMutex>
#include <QMutexLocker>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QNetworkInterface>
#include <QNetworkProxyFactory>
#include <QProcess>
#include <QScreen>
#include <QStringBuilder>
#include <QTcpSocket>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QXmlStreamReader>
#include <qmath.h>

#ifdef QT_NO_DEBUG_OUTPUT
#pragma warning(disable : 4127) // no 'conditional expression is constant' if qDebug() messages are removed
#endif

#pragma warning(pop) // no warnings from includes - end

#include <assert.h>

namespace nmc
{

// DkClientManager --------------------------------------------------------------------
DkClientManager::DkClientManager(const QString &title, QObject *parent)
    : QObject(parent)
{
    mNewPeerId = 0;
    this->mCurrentTitle = title;
    qRegisterMetaType<QList<quint16>>("QList<quint16>");
    qRegisterMetaType<QList<DkPeer *>>("QList<DkPeer*>");
}

DkClientManager::~DkClientManager()
{
    // save settings
    sendGoodByeToAll(); // does nothing...
}

void DkClientManager::connectionReadyForUse(quint16 peerServerPort, const QString &title, DkConnection *connection)
{
    // qDebug() << "connection ready for use" << connection->peerPort() << " with title:" << title << " peerServerPort:"
    // << peerServerPort;

    mNewPeerId++;
    DkPeer *peer = new DkPeer(connection->peerPort(),
                              mNewPeerId,
                              connection->peerAddress(),
                              peerServerPort,
                              title,
                              connection,
                              false,
                              "",
                              false,
                              this);
    connection->setPeerId(mNewPeerId);
    mPeerList.addPeer(peer);

    sendTitle(mCurrentTitle);
}

void DkClientManager::disconnected()
{
    if (DkConnection *connection = qobject_cast<DkConnection *>(sender())) {
        removeConnection(connection);
    }
}

void DkClientManager::removeConnection(DkConnection *connection)
{
    mPeerList.setSynchronized(connection->getPeerId(), false);
    emit synchronizedPeersListChanged(mPeerList.getSynchronizedPeerServerPorts());

    auto aps = mPeerList.getActivePeers();
    emit clientConnectedSignal(!aps.isEmpty());

    qDebug() << "connection Disconnected:" << connection->getPeerPort();
    mPeerList.removePeer(connection->getPeerId());

    auto p = mPeerList.getPeerById(connection->getPeerId());
    if (p && p->isSynchronized()) {
        QString msg = listConnections(aps, false);
        emit updateConnectionSignal(msg);
    }
}

void DkClientManager::connectionSentNewTitle(DkConnection *connection, const QString &newTitle)
{
    mPeerList.setTitle(connection->getPeerId(), newTitle);
}

void DkClientManager::connectionReceivedTransformation(DkConnection *,
                                                       const QTransform &transform,
                                                       const QTransform &imgTransform,
                                                       const QPointF &canvasSize)
{
    emit receivedTransformation(transform, imgTransform, canvasSize);
}

void DkClientManager::connectionReceivedPosition(DkConnection *, const QRect &rect, bool opacity, bool overlaid)
{
    emit receivedPosition(rect, opacity, overlaid);
}

void DkClientManager::connectionReceivedNewFile(DkConnection *, qint16 op, const QString &filename)
{
    emit receivedNewFile(op, filename);
}

void DkClientManager::connectionReceivedGoodBye(DkConnection *connection)
{
    mPeerList.removePeer(connection->getPeerId());
    qDebug() << "goodbye received from " << connection->getPeerId();

    emit synchronizedPeersListChanged(mPeerList.getSynchronizedPeerServerPorts());

    auto aps = mPeerList.getActivePeers();
    QString msg = listConnections(aps, false);
    emit updateConnectionSignal(msg);
    emit clientConnectedSignal(!aps.isEmpty());
}

void DkClientManager::connectionShowStatusMessage(DkConnection *, const QString &msg)
{
    emit sendInfoSignal(msg, 2000);
}

void DkClientManager::sendTitle(const QString &newTitle)
{
    this->mCurrentTitle = newTitle;

    QList<DkPeer *> peers = mPeerList.getPeerList();
    foreach (DkPeer *peer, peers) {
        if (!peer)
            continue;

        // TODO: why not call the method directly?
        connect(this, &DkClientManager::sendNewTitleMessage, peer->connection, &DkConnection::sendNewTitleMessage);
        emit sendNewTitleMessage(newTitle);
        disconnect(this, &DkClientManager::sendNewTitleMessage, peer->connection, &DkConnection::sendNewTitleMessage);
    }
}

void DkClientManager::sendTransform(QTransform transform, QTransform imgTransform, QPointF canvasSize)
{
    QList<DkPeer *> synchronizedPeers = mPeerList.getSynchronizedPeers();
    foreach (DkPeer *peer, synchronizedPeers) {
        if (!peer)
            continue;

        connect(this,
                &DkClientManager::sendNewTransformMessage,
                peer->connection,
                &DkConnection::sendNewTransformMessage);
        emit sendNewTransformMessage(transform, imgTransform, canvasSize);
        disconnect(this,
                   &DkClientManager::sendNewTransformMessage,
                   peer->connection,
                   &DkConnection::sendNewTransformMessage);
    }
}

void DkClientManager::sendPosition(QRect newRect, bool overlaid)
{
    QList<DkPeer *> synchronizedPeers = mPeerList.getSynchronizedPeers();
    foreach (DkPeer *peer, synchronizedPeers) {
        if (!peer)
            continue;

        connect(this,
                &DkClientManager::sendNewPositionMessage,
                peer->connection,
                &DkConnection::sendNewPositionMessage);
        emit sendNewPositionMessage(newRect, true, overlaid);
        disconnect(this,
                   &DkClientManager::sendNewPositionMessage,
                   peer->connection,
                   &DkConnection::sendNewPositionMessage);
    }
}

void DkClientManager::sendNewFile(qint16 op, const QString &filename)
{
    QList<DkPeer *> synchronizedPeers = mPeerList.getSynchronizedPeers();
    foreach (DkPeer *peer, synchronizedPeers) {
        if (!peer)
            continue;

        connect(this, &DkClientManager::sendNewFileMessage, peer->connection, &DkConnection::sendNewFileMessage);
        emit sendNewFileMessage(op, filename);
        disconnect(this, &DkClientManager::sendNewFileMessage, peer->connection, &DkConnection::sendNewFileMessage);
    }
}

void DkClientManager::newConnection(int socketDescriptor)
{
    DkConnection *connection = createConnection();
    connection->setSocketDescriptor(socketDescriptor);
    connection->setTitle(mCurrentTitle);
    mStartUpConnections.append(connection);
    // qDebug() << "new Connection " << connection->peerPort();
}

void DkClientManager::connectConnection(DkConnection *connection)
{
    qRegisterMetaType<QList<quint16>>("QList<quint16>");
    connect(connection, &DkConnection::connectionReadyForUse, this, &DkClientManager::connectionReadyForUse);
    connect(connection, &DkConnection::connectionStopSynchronize, this, &DkClientManager::connectionStopSynchronized);
    connect(connection, &DkConnection::connectionStartSynchronize, this, &DkClientManager::connectionSynchronized);
    connect(connection, &DkConnection::disconnected, this, &DkClientManager::disconnected);
    connect(connection, &DkConnection::connectionTitleHasChanged, this, &DkClientManager::connectionSentNewTitle);
    connect(connection, &DkConnection::connectionNewPosition, this, &DkClientManager::connectionReceivedPosition);
    connect(connection,
            &DkConnection::connectionNewTransform,
            this,
            &DkClientManager::connectionReceivedTransformation);
    connect(connection, &DkConnection::connectionNewFile, this, &DkClientManager::connectionReceivedNewFile);
    connect(connection, &DkConnection::connectionGoodBye, this, &DkClientManager::connectionReceivedGoodBye);
    connect(connection,
            &DkConnection::connectionShowStatusMessage,
            this,
            &DkClientManager::connectionShowStatusMessage);

    connection->synchronizedPeersListChanged(mPeerList.getSynchronizedPeerServerPorts());
}

QString DkClientManager::listConnections(QList<DkPeer *> peers, bool connected)
{
    QString newPeers;

    if (!peers.empty()) {
        if (connected) {
            newPeers = tr("connected with: ");
        } else {
            newPeers = tr("disconnected with: ");
        }

        newPeers.append("\n\t");
    }

    for (const DkPeer *cp : peers) {
        if (!cp->clientName.isEmpty())
            newPeers.append(cp->clientName);
        if (!cp->clientName.isEmpty() && !cp->title.isEmpty())
            newPeers.append(": ");
        if (!cp->title.isEmpty())
            newPeers.append(cp->title);
    }

    return newPeers;
}

void DkClientManager::sendGoodByeToAll()
{
    foreach (DkPeer *peer, mPeerList.getPeerList()) {
        if (!peer)
            continue;

        connect(this, &DkClientManager::sendGoodByeMessage, peer->connection, &DkConnection::sendNewGoodbyeMessage);
        emit sendGoodByeMessage();
        disconnect(this, &DkClientManager::sendGoodByeMessage, peer->connection, &DkConnection::sendNewGoodbyeMessage);
    }
}

// DkLocalClientManager --------------------------------------------------------------------

DkLocalClientManager::DkLocalClientManager(const QString &title, QObject *parent)
    : DkClientManager(title, parent)
{
    startServer();
}

QList<DkPeer *> DkLocalClientManager::getPeerList()
{
    return mPeerList.getPeerList();
}

quint16 DkLocalClientManager::getServerPort() const
{
    if (!mServer)
        return 0;

    return mServer->serverPort();
}

QMimeData *DkLocalClientManager::mimeData() const
{
    QByteArray connectionData;
    QDataStream dataStream(&connectionData, QIODevice::WriteOnly);
    dataStream << getServerPort();

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("network/sync-dir", connectionData);

    return mimeData;
}

void DkLocalClientManager::startServer()
{
    mServer = new DkLocalTcpServer(this);
    connect(mServer, &DkLocalTcpServer::serverReiceivedNewConnection, this, &DkLocalClientManager::newConnection);

    // TODO: hook on thread
    searchForOtherClients();

    DkActionManager &am = DkActionManager::instance();
    connect(am.action(DkActionManager::menu_sync_connect_all),
            &QAction::triggered,
            this,
            &DkLocalClientManager::connectAll);
}

// slots
void DkLocalClientManager::connectAll()
{
    QList<DkPeer *> peers = getPeerList();

    for (auto p : peers)
        synchronizeWithServerPort(p->peerServerPort);
}

void DkLocalClientManager::synchronizeWithServerPort(quint16 port)
{
    // qDebug() << "DkClientManager::synchronizeWithServerPort port:" << port;
    DkPeer *peer = mPeerList.getPeerByServerport(port);
    if (!peer)
        return;
    synchronizeWith(peer->peerId);
}

void DkLocalClientManager::searchForOtherClients()
{
    assert(mServer);

    for (int i = local_tcp_port_start; i <= local_tcp_port_end; i++) {
        if (i == mServer->serverPort())
            continue;

        DkConnection *connection = createConnection();
        connection->connectToHost(QHostAddress::LocalHost, (qint16)i);
    }
}

void DkLocalClientManager::connectionSynchronized(QList<quint16> synchronizedPeersOfOtherClient,
                                                  DkConnection *connection)
{
    qDebug() << "Connection synchronized with:" << connection->getPeerPort();
    mPeerList.setSynchronized(connection->getPeerId(), true);
    emit synchronizedPeersListChanged(mPeerList.getSynchronizedPeerServerPorts());

    auto aps = mPeerList.getActivePeers();

    QString msg = listConnections(aps, true);
    emit updateConnectionSignal(msg);

    emit clientConnectedSignal(!aps.isEmpty());

    for (int i = 0; i < synchronizedPeersOfOtherClient.size(); i++) {
        if (synchronizedPeersOfOtherClient[i] != mServer->serverPort()) {
            DkPeer *peer = mPeerList.getPeerByServerport(synchronizedPeersOfOtherClient[i]);
            if (!peer)
                continue;

            connect(this,
                    &DkLocalClientManager::sendSynchronizeMessage,
                    peer->connection,
                    &DkConnection::sendStartSynchronizeMessage);
            emit sendSynchronizeMessage();
            disconnect(this,
                       &DkLocalClientManager::sendSynchronizeMessage,
                       peer->connection,
                       &DkConnection::sendStartSynchronizeMessage);
        }
    }
    // qDebug() << "--------------------";
    // qDebug() << "current peer list:";
    // peerList.print();
    // qDebug() << "--------------------";
}

void DkLocalClientManager::connectionStopSynchronized(DkConnection *connection)
{
    qDebug() << "Connection no longer synchronized with: " << connection->getPeerPort();
    mPeerList.setSynchronized(connection->getPeerId(), false);

    // qDebug() << "--------------------";
    // qDebug() << "current peer list:";
    // peerList.print();
    // qDebug() << "--------------------";
    emit synchronizedPeersListChanged(mPeerList.getSynchronizedPeerServerPorts());

    auto aps = mPeerList.getActivePeers();

    QString msg = listConnections(aps, false);
    emit updateConnectionSignal(msg);
    emit clientConnectedSignal(!aps.isEmpty());
}

void DkLocalClientManager::synchronizeWith(quint16 peerId)
{
    qDebug() << "DkLocalClientManager::synchronizeWith  peerId:" << peerId;

    mPeerList.setSynchronized(peerId, true); // will be reset if other client does not response within 1 sec
    emit synchronizedPeersListChanged(mPeerList.getSynchronizedPeerServerPorts());

    DkPeer *peer = mPeerList.getPeerById(peerId);
    if (peer == nullptr || peer->connection == nullptr) {
        // qDebug() << "TcpClient: synchronizeWith: Peer is null or connection is null";
        return;
    }

    // qDebug() << "synchronizing with: " << peerId;

    connect(this,
            &DkLocalClientManager::sendSynchronizeMessage,
            peer->connection,
            &DkConnection::sendStartSynchronizeMessage);
    emit sendSynchronizeMessage();
    disconnect(this,
               &DkLocalClientManager::sendSynchronizeMessage,
               peer->connection,
               &DkConnection::sendStartSynchronizeMessage);
}

void DkLocalClientManager::stopSynchronizeWith(quint16)
{
    QList<DkPeer *> synchronizedPeers = mPeerList.getSynchronizedPeers();

    foreach (DkPeer *peer, synchronizedPeers) {
        if (!peer)
            continue;

        connect(this,
                &DkLocalClientManager::sendDisableSynchronizeMessage,
                peer->connection,
                &DkConnection::sendStopSynchronizeMessage);
        emit sendDisableSynchronizeMessage();
        mPeerList.setSynchronized(peer->peerId, false);
        disconnect(this,
                   &DkLocalClientManager::sendDisableSynchronizeMessage,
                   peer->connection,
                   &DkConnection::sendStopSynchronizeMessage);
    }

    emit synchronizedPeersListChanged(mPeerList.getSynchronizedPeerServerPorts());

    auto aps = mPeerList.getActivePeers();

    QString msg = listConnections(aps, false);
    emit updateConnectionSignal(msg);
    emit clientConnectedSignal(!aps.isEmpty());
}

void DkLocalClientManager::sendArrangeInstances(bool overlaid)
{
    const QRect screenGeometry = QApplication::primaryScreen()->availableGeometry();
    int connectedInstances = mPeerList.getSynchronizedPeers().size() + 1; // +1 because of this instance itself
    if (connectedInstances == 1)
        return;
    int instancesPerRow = (connectedInstances == 2 || connectedInstances == 4) ? 2 : 3;

    int rows = (int)qCeil((float)connectedInstances / (float)instancesPerRow);
    int width = screenGeometry.width() / instancesPerRow;
    int height = screenGeometry.height() / rows;

    int curX = screenGeometry.topLeft().x();
    int curY = screenGeometry.topLeft().y();
    emit receivedPosition(QRect(curX, curY, width, height), false, overlaid);
    curX += width;
    int count = 1;
    for (DkPeer *peer : mPeerList.getSynchronizedPeers()) {
        if (!peer)
            continue;

        QRect newPosition = QRect(curX, curY, width, height);
        connect(this,
                &DkLocalClientManager::sendNewPositionMessage,
                peer->connection,
                &DkConnection::sendNewPositionMessage);
        emit sendNewPositionMessage(newPosition, false, overlaid);
        disconnect(this,
                   &DkLocalClientManager::sendNewPositionMessage,
                   peer->connection,
                   &DkConnection::sendNewPositionMessage);

        count++;
        if (count < instancesPerRow)
            curX += width;
        else {
            curX = screenGeometry.topLeft().x();
            curY += height;
            count = 0;
        }
    }
}

void DkLocalClientManager::sendQuitMessageToPeers()
{
    emit sendQuitMessage();
}

void DkLocalClientManager::connectToNomacs()
{
    DkConnection *c = static_cast<DkConnection *>(QObject::sender());

    if (c) {
        c->sendGreetingMessage(mCurrentTitle);
        mStartUpConnections.append(c);
    }
}

void DkLocalClientManager::connectionReceivedQuit()
{
    emit receivedQuit();
}

DkLocalConnection *DkLocalClientManager::createConnection()
{
    // wow - there is no one owning connection (except for QOBJECT)
    DkLocalConnection *connection = new DkLocalConnection(this);
    connection->setLocalTcpServerPort(mServer->serverPort());
    connection->setTitle(mCurrentTitle);
    connectConnection(connection);
    connect(this,
            &DkLocalClientManager::synchronizedPeersListChanged,
            connection,
            &DkLocalConnection::synchronizedPeersListChanged);
    connect(this, &DkLocalClientManager::sendQuitMessage, connection, &DkLocalConnection::sendQuitMessage);
    connect(connection,
            &DkLocalConnection::connectionQuitReceived,
            this,
            &DkLocalClientManager::connectionReceivedQuit);
    connect(connection, &DkLocalConnection::connected, this, &DkLocalClientManager::connectToNomacs);

    return connection;
}

// DkLocalTcpServer --------------------------------------------------------------------
DkLocalTcpServer::DkLocalTcpServer(QObject *parent)
    : QTcpServer(parent)
{
    for (int i = local_tcp_port_start; i < local_tcp_port_end; i++) {
        if (listen(QHostAddress::LocalHost, (quint16)i)) {
            break;
        }
    }
    // qDebug() << "TCP Listening on port " << this->serverPort();
}

void DkLocalTcpServer::incomingConnection(qintptr socketDescriptor)
{
    emit serverReiceivedNewConnection((int)socketDescriptor);
    // qDebug() << "Server: NEW CONNECTION AVAIABLE";
}

DkPeer::DkPeer(quint16 port,
               quint16 peerId,
               const QHostAddress &hostAddress,
               quint16 peerServerPort,
               const QString &title,
               DkConnection *connection,
               bool sychronized,
               const QString &clientName,
               bool showInMenu,
               QObject *parent)
    : QObject(parent)
{
    this->peerId = peerId;
    this->localServerPort = port;
    this->peerServerPort = peerServerPort;
    this->hostAddress = hostAddress;
    this->title = title;
    this->sychronized = sychronized;
    this->connection = connection;
    this->timer = new QTimer(this);
    timer->setSingleShot(true);
    this->clientName = clientName;
    this->showInMenu = showInMenu;
    mHasChangedRecently = false;
    connect(timer, &QTimer::timeout, this, &DkPeer::timerTimeout, Qt::UniqueConnection);
}

DkPeer::~DkPeer() = default;

void DkPeer::setSynchronized(bool flag)
{
    sychronized = flag;
    mHasChangedRecently = true;
    connect(timer, &QTimer::timeout, this, &DkPeer::timerTimeout, Qt::UniqueConnection);
    timer->start(1000);
}

bool DkPeer::operator==(const DkPeer &peer) const
{
    return localServerPort == peer.localServerPort && sychronized == peer.sychronized && title == peer.title
        && hostAddress == peer.hostAddress;
}

void DkPeer::timerTimeout()
{
    mHasChangedRecently = false;
}

// DkPeerList --------------------------------------------------------------------
DkPeerList::DkPeerList() = default;

bool DkPeerList::addPeer(DkPeer *peer)
{
    if (!peer) {
        qDebug() << "[WARNING] you try to append a NULL peer!";
        return false;
    }

    if (peerList.contains(peer->peerId))
        return false;
    else {
        peerList.insert(peer->peerId, peer);
        return true;
    }
}

bool DkPeerList::removePeer(quint16 peerId)
{
    if (!peerList.contains(peerId))
        return false;
    else {
        peerList.remove(peerId);
        return true;
    }
}

bool DkPeerList::setSynchronized(quint16 peerId, bool synchronized)
{
    if (!peerList.contains(peerId))
        return false;
    DkPeer *peer = peerList.value(peerId);
    peer->setSynchronized(synchronized);
    // peerList.replace(peerId, peer);

    return true;
}

bool DkPeerList::setTitle(quint16 peerId, const QString &title)
{
    if (!peerList.contains(peerId))
        return false;
    DkPeer *peer = peerList.value(peerId);
    peer->title = title;
    // peerList.replace(peerId, peer);

    return true;
}

bool DkPeerList::setShowInMenu(quint16 peerId, bool showInMenu)
{
    if (!peerList.contains(peerId))
        return false;
    DkPeer *peer = peerList.value(peerId);
    peer->showInMenu = showInMenu;
    // peerList.replace(peerId, peer);

    return true;
}

QList<DkPeer *> DkPeerList::getSynchronizedPeers() const
{
    QList<DkPeer *> sychronizedPeers;
    foreach (DkPeer *peer, peerList) {
        if (peer->isSynchronized())
            sychronizedPeers.push_back(peer);
    }
    return sychronizedPeers;
}

QList<DkPeer *> DkPeerList::getPeerList()
{
    return peerList.values();
}

QList<quint16> DkPeerList::getSynchronizedPeerServerPorts() const
{
    QList<quint16> sychronizedPeerServerPorts;
    foreach (DkPeer *peer, peerList) {
        if (peer->isSynchronized())
            sychronizedPeerServerPorts.push_back(peer->peerServerPort);
    }
    return sychronizedPeerServerPorts;
}

QList<DkPeer *> DkPeerList::getActivePeers() const
{
    QList<DkPeer *> activePeers;
    foreach (DkPeer *peer, peerList) {
        if (peer->isActive())
            activePeers.push_back(peer);
    }
    return activePeers;
}

DkPeer *DkPeerList::getPeerByServerport(quint16 port) const
{
    foreach (DkPeer *peer, peerList) {
        if (peer->peerServerPort == port)
            return peer;
    }
    return nullptr;
}

bool DkPeerList::alreadyConnectedTo(const QHostAddress &address, quint16 port) const
{
    foreach (DkPeer *peer, peerList) {
        if (peer->hostAddress == address
            && peer->localServerPort == port) // TODO: wieso localserver port ... aber es funkt
            return true;
    }
    return false;
}

DkPeer *DkPeerList::getPeerById(quint16 id)
{
    return peerList.value(id, 0);
}

DkPeer *DkPeerList::getPeerByAddress(const QHostAddress &address, quint16 port) const
{
    foreach (DkPeer *peer, peerList) {
        if (peer->hostAddress == address && peer->localServerPort == port)
            return peer;
    }
    return nullptr; // should not happen
}

void DkPeerList::print() const
{
    foreach (DkPeer *peer, peerList) {
        if (!peer)
            continue;

        qDebug() << peer->peerId << " " << peer->clientName << " " << peer->hostAddress
                 << " serverPort:" << peer->peerServerPort << " localPort:" << peer->localServerPort << " "
                 << peer->title << " sync:" << peer->isSynchronized() << " menu:" << peer->showInMenu
                 << " connection:" << peer->connection;
    }
}

// DkStatusBarManager --------------------------------------------------------------------
DkSyncManager::DkSyncManager()
{
    DkTimer dt;
    mClient = new DkLocalClientManager("nomacs | Image Lounge", nullptr);

    qInfo() << "local client created in: " << dt; // takes 1 sec in the client thread
}

DkSyncManager &DkSyncManager::inst()
{
    static DkSyncManager inst;
    return inst;
}

DkClientManager *DkSyncManager::client()
{
    if (!mClient) {
        qWarning() << "DkSyncManager::client() which is not created yet...";
        return nullptr;
    }

    return mClient;
}

}
