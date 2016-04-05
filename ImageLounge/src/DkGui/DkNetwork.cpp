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
#include "DkSettings.h"
#include "DkTimer.h"
#include "DkControlWidget.h"	// needed for a connection
#include "DkUtils.h"

// that's a bit nasty
#include "DkNoMacs.h"
#include "DkViewPort.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QTcpSocket>
#include <QStringBuilder>
#include <QDir>
#include <QNetworkInterface>
#include <QList>
#include <QThread>
#include <QTimer>
#include <QHostInfo>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QApplication>
#include <QDesktopWidget>
#include <QMutex>
#include <QMutexLocker>
#include <QNetworkCookieJar>
#include <QDesktopServices>
#include <QDebug>
#include <QNetworkProxyFactory>
#include <QXmlStreamReader>
#include <QMessageBox>
#include <QAbstractButton>
#include <QProcess>
#include <qmath.h>
#ifdef WITH_UPNP
#include "DkUpnp.h"
#endif // WITH_UPNP

#ifdef QT_NO_DEBUG_OUTPUT
#pragma warning(disable: 4127)		// no 'conditional expression is constant' if qDebug() messages are removed
#endif

#pragma warning(pop)		// no warnings from includes - end

namespace nmc {

// DkClientManager --------------------------------------------------------------------
DkClientManager::DkClientManager(const QString& title, QObject* parent) : QThread(parent) {
	mNewPeerId = 0;
	this->mCurrentTitle = title;
	qRegisterMetaType<QList<quint16> >("QList<quint16>");
	qRegisterMetaType<QList<DkPeer*> >("QList<DkPeer*>");
}

DkClientManager::~DkClientManager() {
	
	sendGoodByeToAll();	// does nothing...
};


void DkClientManager::connectionReadyForUse(quint16 peerServerPort, const QString& title, DkConnection* connection) {
	//qDebug() << "connection ready for use" << connection->peerPort() << " with title:" << title << " peerServerPort:" << peerServerPort;

	mNewPeerId++;
	DkPeer* peer = new DkPeer(connection->peerPort(), mNewPeerId, connection->peerAddress(), peerServerPort, title, connection, false, "", false, this);
	connection->setPeerId(mNewPeerId);
	mPeerList.addPeer(peer); 

	//connect(this,SIGNAL(sendNewTitleMessage(const QString&)), peer.connection, SLOT(sendNewTitleMessage(const QString&)));
	//emit sendNewTitleMessage(title);
	//disconnect(this,SIGNAL(sendNewTitleMessage(const QString&)), peer.connection, SLOT(sendNewTitleMessage(const QString&)));
	sendTitle(mCurrentTitle);

}

void DkClientManager::disconnected() {

	if (DkConnection *connection = qobject_cast<DkConnection *>(sender())) {		
		removeConnection(connection);
	}
}

void DkClientManager::removeConnection(DkConnection* connection) {
	mPeerList.setSynchronized(connection->getPeerId(), false);
	emit synchronizedPeersListChanged(mPeerList.getSynchronizedPeerServerPorts());
	emit updateConnectionSignal(mPeerList.getActivePeers());	

	qDebug() << "connection Disconnected:" << connection->getPeerPort();
	mPeerList.removePeer(connection->getPeerId());

	//qDebug() << "--------------------";
	//qDebug() << "current peer list:";
	//peerList.print();
	//qDebug() << "--------------------";

}

void DkClientManager::connectionSentNewTitle(DkConnection* connection, const QString& newTitle) {
	mPeerList.setTitle(connection->getPeerId(), newTitle);
}

void DkClientManager::connectionReceivedTransformation(DkConnection*, const QTransform& transform, const QTransform& imgTransform, const QPointF& canvasSize) {
	emit receivedTransformation(transform, imgTransform, canvasSize);
}

void DkClientManager::connectionReceivedPosition(DkConnection*, const QRect& rect, bool opacity, bool overlaid) {
	emit receivedPosition(rect, opacity, overlaid);
}

void DkClientManager::connectionReceivedNewFile(DkConnection*, qint16 op, const QString& filename) {
	emit receivedNewFile(op, filename);
}

void DkClientManager::connectionReceivedGoodBye(DkConnection* connection) {
	mPeerList.removePeer(connection->getPeerId());
	qDebug() << "goodbye received from " << connection->getPeerId();

	emit synchronizedPeersListChanged(mPeerList.getSynchronizedPeerServerPorts());
	emit updateConnectionSignal(mPeerList.getActivePeers());
}

void DkClientManager::connectionShowStatusMessage(DkConnection*, const QString& msg) {
	emit sendInfoSignal(msg, 2000);
}

void DkClientManager::sendTitle(const QString& newTitle) {
	this->mCurrentTitle = newTitle;

	QList<DkPeer*> peers = mPeerList.getPeerList();
	foreach (DkPeer* peer , peers) {
		
		if (!peer)
			continue;

		connect(this,SIGNAL(sendNewTitleMessage(const QString&)), peer->connection, SLOT(sendNewTitleMessage(const QString&)));
		emit sendNewTitleMessage(newTitle);
		disconnect(this,SIGNAL(sendNewTitleMessage(const QString&)), peer->connection, SLOT(sendNewTitleMessage(const QString&)));
	}

}

void DkClientManager::sendTransform(QTransform transform, QTransform imgTransform, QPointF canvasSize) {
	QList<DkPeer*> synchronizedPeers = mPeerList.getSynchronizedPeers();
	foreach (DkPeer* peer , synchronizedPeers) {
		
		if (!peer)
			continue;
		
		connect(this,SIGNAL(sendNewTransformMessage(QTransform, QTransform, QPointF)), peer->connection, SLOT(sendNewTransformMessage(QTransform, QTransform, QPointF)));
		emit sendNewTransformMessage(transform, imgTransform, canvasSize);
		disconnect(this,SIGNAL(sendNewTransformMessage(QTransform, QTransform, QPointF)), peer->connection, SLOT(sendNewTransformMessage(QTransform, QTransform, QPointF)));
	}
}

void DkClientManager::sendPosition(QRect newRect, bool overlaid) {
	QList<DkPeer*> synchronizedPeers = mPeerList.getSynchronizedPeers();
	foreach (DkPeer* peer , synchronizedPeers) {
		
		if (!peer)
			continue;
		
		connect(this,SIGNAL(sendNewPositionMessage(QRect, bool, bool)), peer->connection, SLOT(sendNewPositionMessage(QRect, bool, bool)));
		emit sendNewPositionMessage(newRect, true, overlaid);
		disconnect(this,SIGNAL(sendNewPositionMessage(QRect, bool, bool)), peer->connection, SLOT(sendNewPositionMessage(QRect, bool, bool)));
	}
}

void DkClientManager::sendNewFile(qint16 op, const QString& filename) {
	QList<DkPeer*> synchronizedPeers = mPeerList.getSynchronizedPeers();
	foreach (DkPeer* peer , synchronizedPeers) {
		
		if (!peer)
			continue;
		
		connect(this,SIGNAL(sendNewFileMessage(qint16, const QString&)), peer->connection, SLOT(sendNewFileMessage(qint16, const QString&)));
		emit sendNewFileMessage(op, filename);
		disconnect(this,SIGNAL(sendNewFileMessage(qint16, const QString&)), peer->connection, SLOT(sendNewFileMessage(qint16, const QString&)));
	}
}

void DkClientManager::newConnection( int socketDescriptor ) {
	DkConnection* connection = createConnection();
	connection->setSocketDescriptor(socketDescriptor);
	connection->setTitle(mCurrentTitle);
	mStartUpConnections.append(connection);
	//qDebug() << "new Connection " << connection->peerPort();
}

void DkClientManager::connectConnection(DkConnection* connection) {
	qRegisterMetaType<QList<quint16> >("QList<quint16>");
	connect(connection, SIGNAL(connectionReadyForUse(quint16, const QString&, DkConnection*)), this, SLOT(connectionReadyForUse(quint16, const QString& , DkConnection*)));
	connect(connection, SIGNAL(connectionStopSynchronize(DkConnection*)), this, SLOT(connectionStopSynchronized(DkConnection*)));
	connect(connection, SIGNAL(connectionStartSynchronize(QList<quint16>,DkConnection*)), this, SLOT(connectionSynchronized(QList<quint16>,DkConnection*)));
	connect(connection, SIGNAL(disconnected()), this, SLOT(disconnected()));
	connect(connection, SIGNAL(connectionTitleHasChanged(DkConnection*, const QString&)), this, SLOT(connectionSentNewTitle(DkConnection*, const QString&)));
	connect(connection, SIGNAL(connectionNewPosition(DkConnection*, QRect, bool, bool)), this, SLOT(connectionReceivedPosition(DkConnection*, QRect, bool, bool)));
	connect(connection, SIGNAL(connectionNewTransform(DkConnection*, QTransform, QTransform, QPointF)), this, SLOT(connectionReceivedTransformation(DkConnection*, QTransform, QTransform, QPointF)));
	connect(connection, SIGNAL(connectionNewFile(DkConnection*, qint16, const QString&)), this, SLOT(connectionReceivedNewFile(DkConnection*, qint16, const QString&)));
	connect(connection, SIGNAL(connectionGoodBye(DkConnection*)), this, SLOT(connectionReceivedGoodBye(DkConnection*)));
	connect(connection, SIGNAL(connectionShowStatusMessage(DkConnection*, const QString&)), this, SLOT(connectionShowStatusMessage(DkConnection*, const QString&)));

	connection->synchronizedPeersListChanged(mPeerList.getSynchronizedPeerServerPorts());
}

void DkClientManager::sendGoodByeToAll() {
	
	foreach (DkPeer* peer, mPeerList.getPeerList()) {
		
		if (!peer)
			continue;

		connect(this,SIGNAL(sendGoodByeMessage()), peer->connection, SLOT(sendNewGoodbyeMessage()));
		emit sendGoodByeMessage();
		disconnect(this,SIGNAL(sendGoodByeMessage()), peer->connection, SLOT(sendNewGoodbyeMessage()));
	}
}

// DkLocalClientManager --------------------------------------------------------------------

DkLocalClientManager::DkLocalClientManager(const QString& title, QObject* parent ) : DkClientManager(title, parent) {
	server = new DkLocalTcpServer(this);
	connect(server, SIGNAL(serverReiceivedNewConnection(int)), this, SLOT(newConnection(int)));
	searchForOtherClients();
	//QFuture<void> future = QtConcurrent::run(this, &DkLocalClientManager::searchForOtherClients);
}

QList<DkPeer*> DkLocalClientManager::getPeerList() {
	return mPeerList.getPeerList();
}

quint16 DkLocalClientManager::getServerPort() {
	//qDebug() << "SERVER PORT: " << server->serverPort();
	return server->serverPort();
}

void DkLocalClientManager::synchronizeWithServerPort(quint16 port) {
	//qDebug() << "DkClientManager::synchronizeWithServerPort port:" << port;
	DkPeer* peer = mPeerList.getPeerByServerport(port);
	if (!peer)
		return;
	synchronizeWith(peer->peerId);
}

void DkLocalClientManager::searchForOtherClients() {
		
	for (int i = server->startPort; i <= server->endPort; i++) {
		if (i == server->serverPort())
			continue;
		//qDebug() << "search For other clients on port:" << i;
		DkConnection* connection = createConnection();
		connection->connectToHost(QHostAddress::LocalHost, (qint16)i);

		if (connection->waitForConnected(20)) {
			//qDebug() << "Connected to " << i ;
			connection->sendGreetingMessage(mCurrentTitle);		// WTF: who owns DkConnection??
			mStartUpConnections.append(connection);
		} else
			delete connection;
	}
	
}

void DkLocalClientManager::run() {
	
	//exec();
}

void DkLocalClientManager::connectionSynchronized(QList<quint16> synchronizedPeersOfOtherClient, DkConnection* connection) {
	qDebug() << "Connection synchronized with:" << connection->getPeerPort();
	mPeerList.setSynchronized(connection->getPeerId(), true);
	emit synchronizedPeersListChanged(mPeerList.getSynchronizedPeerServerPorts());

	emit updateConnectionSignal(mPeerList.getActivePeers());
	
	for (int i=0; i < synchronizedPeersOfOtherClient.size(); i++) {
		if (synchronizedPeersOfOtherClient[i]!=server->serverPort()) {
			
			DkPeer* peer = mPeerList.getPeerByServerport(synchronizedPeersOfOtherClient[i]);
			if (!peer)
				continue;

			connect(this,SIGNAL(sendSynchronizeMessage()), peer->connection, SLOT(sendStartSynchronizeMessage()));
			emit sendSynchronizeMessage();
			disconnect(this,SIGNAL(sendSynchronizeMessage()), peer->connection, SLOT(sendStartSynchronizeMessage()));				

		}
	}
	//qDebug() << "--------------------";
	//qDebug() << "current peer list:";
	//peerList.print();
	//qDebug() << "--------------------";
}

void DkLocalClientManager::connectionStopSynchronized(DkConnection* connection) {
	qDebug() << "Connection no longer synchronized with: " << connection->getPeerPort();
	mPeerList.setSynchronized(connection->getPeerId(), false);

	//qDebug() << "--------------------";
	//qDebug() << "current peer list:";
	//peerList.print();
	//qDebug() << "--------------------";
	emit synchronizedPeersListChanged(mPeerList.getSynchronizedPeerServerPorts());
	emit updateConnectionSignal(mPeerList.getActivePeers());
}


void DkLocalClientManager::synchronizeWith(quint16 peerId) {
	qDebug() << "DkLocalClientManager::synchronizeWith  peerId:" << peerId;

	mPeerList.setSynchronized(peerId, true); // will be reset if other client does not response within 1 sec
	emit synchronizedPeersListChanged(mPeerList.getSynchronizedPeerServerPorts());

	DkPeer* peer = mPeerList.getPeerById(peerId);
	if (peer == 0 || peer->connection == 0) {
		//qDebug() << "TcpClient: synchronizeWith: Peer is null or connection is null";
		return;
	}

	//qDebug() << "synchronizing with: " << peerId;


	connect(this,SIGNAL(sendSynchronizeMessage()), peer->connection, SLOT(sendStartSynchronizeMessage()));
	emit sendSynchronizeMessage();
	disconnect(this,SIGNAL(sendSynchronizeMessage()), peer->connection, SLOT(sendStartSynchronizeMessage()));

}

void DkLocalClientManager::stopSynchronizeWith(quint16) {
	QList<DkPeer*> synchronizedPeers = mPeerList.getSynchronizedPeers();
	
	foreach (DkPeer* peer , synchronizedPeers) {

		if (!peer)
			continue;

		connect(this,SIGNAL(sendDisableSynchronizeMessage()), peer->connection, SLOT(sendStopSynchronizeMessage()));
		emit sendDisableSynchronizeMessage();
		mPeerList.setSynchronized(peer->peerId, false);
		disconnect(this,SIGNAL(sendDisableSynchronizeMessage()), peer->connection, SLOT(sendStopSynchronizeMessage()));
	}

	emit synchronizedPeersListChanged(mPeerList.getSynchronizedPeerServerPorts());
	emit updateConnectionSignal(mPeerList.getActivePeers());	
}

void DkLocalClientManager::sendArrangeInstances(bool overlaid) {
	int screen = QApplication::desktop()->screenNumber(QApplication::activeWindow());
	const QRect screenGeometry =  QApplication::desktop()->availableGeometry(screen);
	int connectedInstances = mPeerList.getSynchronizedPeers().size() + 1 ; // +1 because of this instance itself
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
	for (DkPeer* peer : mPeerList.getSynchronizedPeers()) {

		if (!peer)
			continue;

		QRect newPosition = QRect(curX, curY, width, height);
		connect(this,SIGNAL(sendNewPositionMessage(QRect, bool, bool)), peer->connection, SLOT(sendNewPositionMessage(QRect, bool, bool)));
		emit sendNewPositionMessage(newPosition, false, overlaid);
		disconnect(this,SIGNAL(sendNewPositionMessage(QRect, bool, bool)), peer->connection, SLOT(sendNewPositionMessage(QRect, bool, bool)));
		
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

void DkLocalClientManager::sendQuitMessageToPeers() {
	emit sendQuitMessage();
}

void DkLocalClientManager::connectionReceivedQuit() {
	emit receivedQuit();
}

DkLocalConnection* DkLocalClientManager::createConnection() {

	//qDebug() << "SERVER server port: " << server->serverPort();
	DkLocalConnection* connection = new DkLocalConnection(this);
	connection->setLocalTcpServerPort(server->serverPort());
	connection->setTitle(mCurrentTitle);
	connectConnection(connection);
	connect(this, SIGNAL(synchronizedPeersListChanged(QList<quint16>)), connection, SLOT(synchronizedPeersListChanged(QList<quint16>)));
	connect(this, SIGNAL(sendQuitMessage()), connection, SLOT(sendQuitMessage()));
	connect(connection, SIGNAL(connectionQuitReceived()), this, SLOT(connectionReceivedQuit()));
	return connection;

}
// DkLANClientManager --------------------------------------------------------------------
DkLANClientManager::DkLANClientManager(const QString& title, QObject* parent, quint16 udpServerPortRangeStart, quint16 udpServerPortRangeEnd) : DkClientManager(title, parent) {
	server = new DkLANTcpServer(this, udpServerPortRangeStart, udpServerPortRangeEnd);
	connect(server, SIGNAL(serverReiceivedNewConnection(const QHostAddress&, quint16, const QString&)), this, SLOT(startConnection(const QHostAddress&, quint16, const QString&)));
	connect(server, SIGNAL(serverReiceivedNewConnection(int)), this, SLOT(newConnection(int)));
	connect(server, SIGNAL(sendStopSynchronizationToAll()), this, SLOT(sendStopSynchronizationToAll()));
}

DkLANClientManager::~DkLANClientManager() {
}

QList<DkPeer*> DkLANClientManager::getPeerList() {
	QList<DkPeer*> list;
	foreach(DkPeer* peer, mPeerList.getPeerList()) {
		
		if (!peer)
			continue;

		if (peer->showInMenu)
			list.push_back(peer);
	}
	return list;
}

void DkLANClientManager::startConnection(const QHostAddress& address, quint16 port, const QString& clientName) {
	qDebug() << "DkLANClientManager::startConnection: connecting to:" << address << ":" << port << "    line:" << __LINE__;  
	if (mPeerList.alreadyConnectedTo(address, port)) {
		qDebug() << "already connected";
		return;
	}

	for (int i=0; i < mStartUpConnections.size(); i++) {
		DkConnection* suConnection = mStartUpConnections.at(i);
		if (suConnection->peerAddress() == address && suConnection->peerPort() == port) {
			qDebug() << "already trying to connect to this client";
			return;
		}
	}

	DkLANConnection* connection = createConnection();
	connection->connectionCreated = true;
	connection->setClientName(clientName);
	connection->connectToHost(address, port);
	connection->setIAmServer(false);

	mStartUpConnections.append(connection);

	if (connection->waitForConnected(10))
		qDebug() << "Connected to " << address << ":" << port ;

	connection->sendGreetingMessage(mCurrentTitle);
	connection->setShowInMenu(true);
}

void DkLANClientManager::connectionReadyForUse(quint16 peerServerPort, const QString& title, DkConnection* dkconnection) {
	DkLANConnection* connection = dynamic_cast<DkLANConnection*>(dkconnection); // TODO???? darf ich das
	//qDebug() << "connection ready for use" << connection->peerPort() << " with title:" << title << " peerServerPort:" << peerServerPort << "  showInMenu: " << connection->getShowInMenu();

	mNewPeerId++;
	DkPeer* peer = new DkPeer(connection->peerPort(), mNewPeerId, connection->peerAddress(), peerServerPort, title, connection, false, connection->getClientName(), connection->getShowInMenu(), this);
	connection->setPeerId(mNewPeerId);
	mPeerList.addPeer(peer); 

	//qDebug() << "--------------------";
	//qDebug() << "current peer list:";
	//peerList.print();
	//qDebug() << "--------------------";

}



void DkLANClientManager::connectionSentNewTitle(DkConnection* connection, const QString& newTitle) {
	mPeerList.setTitle(connection->getPeerId(), newTitle);

	// propagate this message
	QList<DkPeer*> syncPeerList = mPeerList.getSynchronizedPeers();
	foreach (DkPeer* peer, syncPeerList) {
		
		if (!peer)
			continue;

		if (peer->peerId != connection->getPeerId())
			peer->connection->sendNewTitleMessage(newTitle);
	}
}


void DkLANClientManager::connectionSynchronized(QList<quint16> synchronizedPeersOfOtherClient, DkConnection* connection) {
	//qDebug() << "Connection synchronized with:" << connection->getPeerPort();
	mPeerList.setSynchronized(connection->getPeerId(), true);
	mPeerList.setShowInMenu(connection->getPeerId(), true);
	emit synchronizedPeersListChanged(mPeerList.getSynchronizedPeerServerPorts());
	emit updateConnectionSignal(mPeerList.getActivePeers());

	// ignore synchronized clients of other connection

	// add to last seen for whitelisting
	DkPeer* syncedPeer = mPeerList.getPeerById(connection->getPeerId());
	if (!syncedPeer)
		return;
	Settings::param().sync().recentSyncNames << syncedPeer->clientName;
	Settings::param().sync().recentLastSeen.insert(syncedPeer->clientName, QDateTime::currentDateTime());
	qDebug() << "added " << syncedPeer->clientName << " to recently seen list";
}

void DkLANClientManager::connectionStopSynchronized(DkConnection* connection) {
	//qDebug() << "Connection no longer synchronized with: " << connection->getPeerPort();
	mPeerList.setSynchronized(connection->getPeerId(), false);
	mPeerList.setShowInMenu(connection->getPeerId(), false);
	//qDebug() << "--------------------";
	//qDebug() << "current peer list:";
	//peerList.print();
	//qDebug() << "--------------------";
	emit synchronizedPeersListChanged(mPeerList.getSynchronizedPeerServerPorts());
	emit updateConnectionSignal(mPeerList.getActivePeers());
}


void DkLANClientManager::synchronizeWith(quint16 peerId) {
	//qDebug() << "DkClientManager::synchronizeWith  peerId:" << peerId;

	if (server->isListening()) {
		DkPeer* newServer = mPeerList.getPeerById(peerId);
		foreach (DkPeer* peer, mPeerList.getSynchronizedPeers()) {

			if (!peer)
				continue;

			connect(this,SIGNAL(sendSwitchServerMessage(const QHostAddress&, quint16)), peer->connection, SLOT(sendSwitchServerMessage(const QHostAddress&, quint16)));
			emit sendSwitchServerMessage(newServer->hostAddress, newServer->localServerPort);
			disconnect(this,SIGNAL(sendSwitchServerMessage(const QHostAddress&, quint16)), peer->connection, SLOT(sendSwitchServerMessage(const QHostAddress&, quint16)));
			mPeerList.setShowInMenu(peer->peerId, false);

			connect(this,SIGNAL(sendGoodByeMessage()), peer->connection, SLOT(sendNewGoodbyeMessage()));
			emit sendGoodByeMessage();
			disconnect(this,SIGNAL(sendGoodByeMessage()), peer->connection, SLOT(sendNewGoodbyeMessage()));

			//connect(this,SIGNAL(sendDisableSynchronizeMessage()), peer.connection, SLOT(sendStopSynchronizeMessage()));
			//emit sendDisableSynchronizeMessage();
			//peerList.setSynchronized(peer.peerId, false);
			//disconnect(this,SIGNAL(sendDisableSynchronizeMessage()), peer.connection, SLOT(sendStopSynchronizeMessage()));
		}

		server->startServer(false);
	}


	mPeerList.setSynchronized(peerId, true); // will be reset if other client does not response within 1 sec
	emit synchronizedPeersListChanged(mPeerList.getSynchronizedPeerServerPorts());


	//qDebug() << "--------------------";
	//qDebug() << "current peer list:";
	//peerList.print();
	//qDebug() << "--------------------";

	DkPeer* peer = mPeerList.getPeerById(peerId);
	if (!peer || peer->connection == 0) {
		qDebug() << "TcpClient: synchronizeWith: connection is null";
		return;
	}

	qDebug() << "DkLANClientManager synchronizing with: " << peerId;
	connect(this,SIGNAL(sendSynchronizeMessage()), peer->connection, SLOT(sendStartSynchronizeMessage()));
	emit sendSynchronizeMessage();
	disconnect(this,SIGNAL(sendSynchronizeMessage()), peer->connection, SLOT(sendStartSynchronizeMessage()));

}


void DkLANClientManager::stopSynchronizeWith(quint16 peerId) {
	
	//qDebug() << "stop synchronize with:" << peerId;

	// disconnect all
	if (peerId == USHRT_MAX) {
		qDebug() << "stopSynchronize with all synchronized peers";
		QList<DkPeer*> synchronizedPeers = mPeerList.getSynchronizedPeers();
		foreach (DkPeer* peer, synchronizedPeers) {
			
			if (!peer)
				continue;

			connect(this,SIGNAL(sendDisableSynchronizeMessage()), peer->connection, SLOT(sendStopSynchronizeMessage()));
			emit sendDisableSynchronizeMessage();
			mPeerList.setSynchronized(peer->peerId, false);
			disconnect(this,SIGNAL(sendDisableSynchronizeMessage()), peer->connection, SLOT(sendStopSynchronizeMessage()));
		}
	}
	else {
		DkPeer* peer = mPeerList.getPeerById(peerId);
		if (!peer || peer->connection == 0 ) {
			qDebug() << "peer or its connection is null ...  are you sure this peerId exists?";
			return;
		}
		connect(this,SIGNAL(sendDisableSynchronizeMessage()), peer->connection, SLOT(sendStopSynchronizeMessage()));
		emit sendDisableSynchronizeMessage();
		disconnect(this,SIGNAL(sendDisableSynchronizeMessage()), peer->connection, SLOT(sendStopSynchronizeMessage()));

		mPeerList.setSynchronized(peer->peerId, false);
		if (server->isListening()) // i am server
			mPeerList.setShowInMenu(peerId, false);
	}


	emit synchronizedPeersListChanged(mPeerList.getSynchronizedPeerServerPorts());
	emit updateConnectionSignal(mPeerList.getActivePeers());	
}

void DkLANClientManager::connectionReceivedTransformation(DkConnection* connection, QTransform transform, QTransform imgTransform, QPointF canvasSize) {
	emit receivedTransformation(transform, imgTransform, canvasSize);

	// propagate this message
	QList<DkPeer*> syncPeerList = mPeerList.getSynchronizedPeers();
	foreach (DkPeer* peer, syncPeerList) {
		if (peer && peer->peerId != connection->getPeerId())
			peer->connection->sendNewTransformMessage(transform, imgTransform, canvasSize);
	}

}

void DkLANClientManager::connectionReceivedPosition(DkConnection* connection, QRect rect, bool opacity, bool overlaid) {
	emit receivedPosition(rect, opacity, overlaid);

	// propagate this message
	QList<DkPeer*> syncPeerList = mPeerList.getSynchronizedPeers();
	foreach (DkPeer* peer, syncPeerList) {
		if (peer && peer->peerId != connection->getPeerId())
			peer->connection->sendNewPositionMessage(rect, opacity, overlaid);
	}

}

void DkLANClientManager::connectionReceivedNewFile(DkConnection* connection, qint16 op, const QString& filename) {
	emit receivedNewFile(op, filename);

	// propagate this message
	QList<DkPeer*> syncPeerList = mPeerList.getSynchronizedPeers();
	foreach (DkPeer* peer, syncPeerList) {
		if (peer && peer->peerId != connection->getPeerId())
			peer->connection->sendNewFileMessage(op, filename);
	}

}


void DkLANClientManager::connectionReceivedUpcomingImage(DkConnection*, const QString& imageTitle) {
	//qDebug() << "Connection will receive Image with title: " << imageTitle;
	emit sendInfoSignal("receiving image:\n" + imageTitle, 2000);
}


void DkLANClientManager::connectionReceivedNewImage(DkConnection* connection, const QImage& image, const QString& title) {
	//qDebug() << "DkTcpNetworkClient:: connection Received New Image";
	emit receivedImage(image);
	emit receivedImageTitle(title + " - ");
	//qDebug() << "received title: " << title;

	// propagate this message
	QList<DkPeer*> syncPeerList = mPeerList.getSynchronizedPeers();
	foreach (DkPeer* peer, syncPeerList) {
		if (peer && peer->peerId != connection->getPeerId()) {
			DkLANConnection* con = dynamic_cast<DkLANConnection*>(peer->connection); // TODO???? darf ich das
			connect(this,SIGNAL(sendNewImageMessage(QImage, const QString&)), con, SLOT(sendNewImageMessage(QImage, const QString&)));
			emit sendNewImageMessage(image, title);
			connect(this,SIGNAL(sendNewImageMessage(QImage, const QString&)), con, SLOT(sendNewImageMessage(QImage, const QString&)));
		}
	}

}

void DkLANClientManager::connectionReceivedSwitchServer(DkConnection* connection, const QHostAddress& address, quint16 port) {
	//qDebug() << "DkLANClientManager::connectionReceivedSwitchServer:" << address << ":" << port;
	if (!mPeerList.alreadyConnectedTo(address, port))
		return;

	DkPeer* peer = mPeerList.getPeerByAddress(address, port);
	if (!peer) {
		qDebug() << "DkLANClientManager::connectionSwitchServer: other peer not known";
		return;
	}
	connect(this,SIGNAL(sendSynchronizeMessage()), peer->connection, SLOT(sendStartSynchronizeMessage()));
	emit sendSynchronizeMessage();
	disconnect(this,SIGNAL(sendSynchronizeMessage()), peer->connection, SLOT(sendStartSynchronizeMessage()));

	connect(this,SIGNAL(sendDisableSynchronizeMessage()), connection, SLOT(sendStopSynchronizeMessage()));
	emit sendDisableSynchronizeMessage();
	mPeerList.setSynchronized(peer->peerId, false);
	if (server->isListening()) // i am server
		mPeerList.setShowInMenu(connection->getPeerId(), false);
	disconnect(this,SIGNAL(sendDisableSynchronizeMessage()), connection, SLOT(sendStopSynchronizeMessage()));

}

void DkLANClientManager::sendTitle(const QString& newTitle) {
	this->mCurrentTitle = newTitle;

	if (server->isListening()) { // only send title if instance is server
		QList<DkPeer*> peers = mPeerList.getPeerList();
		foreach (DkPeer* peer , peers) {
			
			if (!peer)
				continue;
			
			connect(this,SIGNAL(sendNewTitleMessage(const QString&)), peer->connection, SLOT(sendNewTitleMessage(const QString&)));
			emit sendNewTitleMessage(newTitle);
			disconnect(this,SIGNAL(sendNewTitleMessage(const QString&)), peer->connection, SLOT(sendNewTitleMessage(const QString&)));
		}
	}
}

void DkLANClientManager::sendNewImage(QImage image, const QString& title) {
	//qDebug() << "sending new image";
	QList<DkPeer*> synchronizedPeers = mPeerList.getSynchronizedPeers();
	foreach (DkPeer* peer , synchronizedPeers) {
		
		if (!peer)
			continue;

		DkLANConnection* connection = dynamic_cast<DkLANConnection*>(peer->connection);
		connect(this,SIGNAL(sendNewUpcomingImageMessage(const QString&)), connection, SLOT(sendNewUpcomingImageMessage(const QString&)));
		emit sendNewUpcomingImageMessage(title);
		disconnect(this,SIGNAL(sendNewUpcomingImageMessage(const QString&)), connection, SLOT(sendNewUpcomingImageMessage(const QString&)));

		connect(this,SIGNAL(sendNewImageMessage(QImage, const QString&)), connection, SLOT(sendNewImageMessage(QImage, const QString&)));
		emit sendNewImageMessage(image, title);
		disconnect(this,SIGNAL(sendNewImageMessage(QImage, const QString&)), connection, SLOT(sendNewImageMessage(QImage, const QString&)));
	}
}


void DkLANClientManager::startServer(bool flag) {
	if (!flag) {
		foreach (DkPeer* peer, mPeerList.getPeerList()) {
			
			if (!peer)
				continue;

			peer->connection->sendNewGoodbyeMessage();
		}
	}
	server->startServer(flag);
#ifdef WITH_UPNP
	qDebug() << "server address:" << server->serverAddress() << " port:" << server->serverPort();
	if (flag) {
		emit serverPortChanged(server->serverPort());
	} else {
		emit serverPortChanged(0);
	}
#endif // WITH_UPNP
}


void DkLANClientManager::sendStopSynchronizationToAll() {
	foreach (DkPeer* peer, mPeerList.getSynchronizedPeers()) {
		
		if (!peer)
			continue;
		
		connect(this,SIGNAL(sendDisableSynchronizeMessage()), peer->connection, SLOT(sendStopSynchronizeMessage()));
		emit sendDisableSynchronizeMessage();
		mPeerList.setSynchronized(peer->peerId, false);
		disconnect(this,SIGNAL(sendDisableSynchronizeMessage()), peer->connection, SLOT(sendStopSynchronizeMessage()));	
	}
	emit synchronizedPeersListChanged(mPeerList.getSynchronizedPeerServerPorts());
	emit updateConnectionSignal(mPeerList.getActivePeers());	

	// second for loop needed for disconnect message
	qDebug() << "disconnecting connections:";
	foreach (DkPeer* peer, mPeerList.getSynchronizedPeers()) {
		
		if (!peer)
			continue;

		qDebug() << peer->connection->disconnect();
		mPeerList.removePeer(peer->peerId);
	}		
}

DkLANConnection* DkLANClientManager::createConnection() {
	DkLANConnection* connection = new DkLANConnection();
	connectConnection(connection);
	mStartUpConnections.append(connection);

	return connection;
}

void DkLANClientManager::connectConnection(DkConnection* connection) {
	DkClientManager::connectConnection(connection);
	connect(connection, SIGNAL(connectionNewImage(DkConnection*, const QImage&, const QString&)), this, SLOT(connectionReceivedNewImage(DkConnection*, const QImage&, const QString&)));
	connect(connection, SIGNAL(connectionUpcomingImage(DkConnection*, const QString&)), this, SLOT(connectionReceivedUpcomingImage(DkConnection*, const QString&)));
	connect(connection, SIGNAL(connectionSwitchServer(DkConnection*, const QHostAddress&, quint16)), this, SLOT(connectionReceivedSwitchServer(DkConnection*, const QHostAddress&, quint16)));
}

// DkRemoteControllClientManager --------------------------------------------------------------------
DkRCClientManager::DkRCClientManager(const QString& title, QObject* parent /* = 0 */) : DkLANClientManager(title, parent, rc_udp_port, rc_udp_port) {
	connect(server, SIGNAL(sendStopSynchronizationToAll()), this, SLOT(sendStopSynchronizationToAll()));
}

QList<DkPeer*> DkRCClientManager::getPeerList() {
	QList<DkPeer*> list;
	foreach(DkPeer* peer, mPeerList.getPeerList()) {
		
		if (peer && permissionList.value(peer->peerId) && peer->connection->connectionCreated)
			list.push_back(peer);
	}
	return list;
}

void DkRCClientManager::synchronizeWith(quint16 peerId) {
	//qDebug() << "DkCRemoteControllientManager::synchronizeWith  peerId:" << peerId;

	mPeerList.setSynchronized(peerId, true); // will be reset if other client does not response within 1 sec

	qDebug() << "DkRCClientManager: peer list:" << __FILE__ << __FUNCTION__;
	qDebug() << "--------- peerList -------";
	mPeerList.print();
	qDebug() << "--------- peerList end----";

	DkPeer* peer = mPeerList.getPeerById(peerId);
	if (!peer || peer->connection == 0) {
		qDebug() << "TcpClient: synchronizeWith: connection is null";
		return;
	}

	//qDebug() << "synchronizing with: " << peerId;
	connect(this,SIGNAL(sendSynchronizeMessage()), peer->connection, SLOT(sendStartSynchronizeMessage()));
	emit sendSynchronizeMessage();
	disconnect(this,SIGNAL(sendSynchronizeMessage()), peer->connection, SLOT(sendStartSynchronizeMessage()));

	emit synchronizedPeersListChanged(mPeerList.getSynchronizedPeerServerPorts());
}

void DkRCClientManager::connectionSynchronized(QList<quint16> synchronizedPeersOfOtherClient, DkConnection* connection) {
	DkPeer* peer = mPeerList.getPeerById(connection->getPeerId());
	if (!peer)
		return;

	//qDebug() << "Connection synchronized with:" << connection->getPeerPort();
	mPeerList.setSynchronized(connection->getPeerId(), true);
	mPeerList.setShowInMenu(connection->getPeerId(), true);
	emit synchronizedPeersListChanged(mPeerList.getSynchronizedPeerServerPorts());
	emit updateConnectionSignal(mPeerList.getActivePeers());

	// ignore synchronized clients of other connection

	// add to last seen for whitelisting
	Settings::param().sync().recentSyncNames << peer->clientName;
	Settings::param().sync().recentLastSeen.insert(peer->clientName, QDateTime::currentDateTime());

}



void DkRCClientManager::connectionReadyForUse(quint16 peerServerPort, const QString& title, DkConnection* dkconnection) {
	//DkLANClientManager::connectionReadyForUse(peerServerPort, title, dkconnection);
	//peerList.print();
	//DkPeer peer = peerList.getPeerByAddress(dkconnection->peerAddress(), peerServerPort);
	//permissionList.insert(peer.peerId, false);

	DkRCConnection* connection = dynamic_cast<DkRCConnection*>(dkconnection); // TODO???? darf ich das
	//qDebug() << "connection ready for use" << connection->peerPort() << " with title:" << title << " peerServerPort:" << peerServerPort << "  showInMenu: " << connection->getShowInMenu();

	mNewPeerId++;
	DkPeer* peer = new DkPeer(connection->peerPort(), mNewPeerId, connection->peerAddress(), peerServerPort, title, connection, false, connection->getClientName(), connection->getShowInMenu(), this);
	connection->setPeerId(mNewPeerId);
	mPeerList.addPeer(peer); 

	//qDebug() << "peerList:";
	//peerList.print();

	//if (!server->isListening())
		connection->sendAskForPermission(); // TODO: do i have to ask for permission?
}

void DkRCClientManager::connectionReceivedPermission(DkConnection* connection, bool allowedToConnect) {
	permissionList.insert(connection->getPeerId(), allowedToConnect);
}

void DkRCClientManager::connectionReceivedRCType(DkConnection*, int mode) {
	qDebug() << "connection received new remote control mode: " << mode;
	emit(connectedReceivedNewMode(mode));
}

void DkRCClientManager::connectionReceivedGoodBye(DkConnection* connection) {
	emit connectedReceivedNewMode(DkSettings::sync_mode_default);
	DkClientManager::connectionReceivedGoodBye(connection);
}

DkRCConnection* DkRCClientManager::createConnection() {
	DkRCConnection* connection = new DkRCConnection();
	connectConnection(connection);
	return connection;
}

void DkRCClientManager::connectConnection(DkConnection* connection) {
	DkLANClientManager::connectConnection(connection);
	connect(connection, SIGNAL(connectionNewPermission(DkConnection*, bool)), this, SLOT(connectionReceivedPermission(DkConnection*, bool)));
	connect(connection, SIGNAL(connectionNewRCType(DkConnection*, int)), this, SLOT(connectionReceivedRCType(DkConnection*, int)));
}

void DkRCClientManager::sendNewMode(int mode) {
	
	//qDebug() << "sending new image";
	QList<DkPeer*> synchronizedPeers = mPeerList.getSynchronizedPeers();
	foreach (DkPeer* peer , synchronizedPeers) {
		
		if (!peer)
			continue;
		
		DkRCConnection* connection = dynamic_cast<DkRCConnection*>(peer->connection);
		connect(this,SIGNAL(sendNewModeMessage(int)), connection, SLOT(sendRCType(int)));
		emit sendNewModeMessage(mode);
		disconnect(this,SIGNAL(sendNewModeMessage(int)), connection, SLOT(sendRCType(int)));
	}
}


// DkLocalTcpServer --------------------------------------------------------------------
DkLocalTcpServer::DkLocalTcpServer(QObject* parent) : QTcpServer(parent) {
	this->startPort = local_tcp_port_start;
	this->endPort = local_tcp_port_end;

	for (int i = startPort; i < endPort; i++) {
		if (listen(QHostAddress::LocalHost, (quint16)i)) {
			break;
		}
	}
	qDebug() << "TCP Listening on port " << this->serverPort();
}

void DkLocalTcpServer::incomingConnection (qintptr socketDescriptor)  {
	emit serverReiceivedNewConnection((int)socketDescriptor);
	//qDebug() << "Server: NEW CONNECTION AVAIABLE";
}

// DkLANTcpServer --------------------------------------------------------------------

DkLANTcpServer::DkLANTcpServer( QObject* parent, quint16 udpServerPortRangeStart, quint16 updServerPortRangeEnd) : QTcpServer(parent) {
	udpSocket = new DkLANUdpSocket(udpServerPortRangeStart, updServerPortRangeEnd, this);
	connect(udpSocket, SIGNAL(udpSocketNewServerOnline(const QHostAddress&, quint16, const QString&)), this, SLOT(udpNewServerFound(const QHostAddress&, quint16, const QString&)));
	connect(this, SIGNAL(sendNewClientBroadcast()), udpSocket, SLOT(sendNewClientBroadcast()));
	emit sendNewClientBroadcast();
}

void DkLANTcpServer::startServer(bool flag) {
	qDebug() << "DkLANTcpServer::startServer start: " << flag;
	if (flag) {
		listen(QHostAddress::Any);
		qDebug() << "DkLANTcpServer listening on:" << this->serverPort();
		udpSocket->startBroadcast(this->serverPort());
	} else {
		emit(sendStopSynchronizationToAll());
		this->close();
		qDebug() << "DkLANTcpServer stopped listening";
		udpSocket->stopBroadcast();
	}
}

void DkLANTcpServer::udpNewServerFound(const QHostAddress& address , quint16 port , const QString& clientName) {
	emit(serverReiceivedNewConnection(address , port , clientName));
}

void DkLANTcpServer::incomingConnection (qintptr socketDescriptor)  {
	emit serverReiceivedNewConnection((int)socketDescriptor);
}

// DkUpdSocket --------------------------------------------------------------------
DkLANUdpSocket::DkLANUdpSocket( quint16 startPort, quint16 endPort , QObject* parent ) : QUdpSocket(parent) {
	this->mStartPort = startPort;
	this->mEndPort = endPort;

	for (mServerPort = startPort; mServerPort <= endPort; mServerPort++) {
		if (bind(QHostAddress::Any, mServerPort))
			break;
	}

	qDebug() << "UpdBroadcastserver listening on " << mServerPort;
	connect(this, SIGNAL(readyRead()), this, SLOT(readBroadcast()));

	
	checkLocalIpAddresses();

	mBroadcasting = false;
}

void DkLANUdpSocket::startBroadcast(quint16 tcpServerPort) {
	this->mTcpServerPort = tcpServerPort;

	sendBroadcast(); // send first broadcast 
	this->mBroadcastTimer = new QTimer;
	mBroadcastTimer->setInterval(10000);
	connect(mBroadcastTimer, SIGNAL(timeout()), this, SLOT(sendBroadcast()));
	mBroadcastTimer->start();
	mBroadcasting = true;
}

void DkLANUdpSocket::stopBroadcast() {
	mBroadcastTimer->stop();
	mBroadcasting = false;
}

void DkLANUdpSocket::sendBroadcast() {
	//qDebug() << "sending broadcast";
	QByteArray datagram;
	datagram.append(QHostInfo::localHostName());
	datagram.append("@");
	datagram.append(QByteArray::number(mTcpServerPort));
	QList<QNetworkInterface> networkInterfaces = QNetworkInterface::allInterfaces();
	for (quint16 port = mStartPort; port <= mEndPort; port++) {
		for (QList<QNetworkInterface>::iterator networkInterfacesItr = networkInterfaces.begin(); networkInterfacesItr != networkInterfaces.end(); networkInterfacesItr++) {
			QList<QNetworkAddressEntry> entires = networkInterfacesItr->addressEntries();
			for (QList<QNetworkAddressEntry>::iterator itr = entires.begin(); itr != entires.end(); itr++) {
				if (itr->broadcast().isNull())
					continue;
				writeDatagram(datagram.data(), datagram.size(), itr->broadcast(), port);
			}
		}
	}


}

void DkLANUdpSocket::sendNewClientBroadcast() {
	QByteArray datagram;
	datagram.append("newClient");
	datagram.append("@");
	datagram.append(QByteArray::number(0));

	for (quint16 port = mStartPort; port <= mEndPort; port++)  {
		writeDatagram(datagram.data(), datagram.size(), QHostAddress::Broadcast, port);
	}
	qDebug() << "sent broadcast:" << datagram << "--- " << 0;
}

void DkLANUdpSocket::readBroadcast() {
	while(this->hasPendingDatagrams()) {
		QHostAddress senderIP;
		quint16 senderPort;
		QByteArray datagram;
		datagram.resize((int)this->pendingDatagramSize());
		if (readDatagram(datagram.data(), datagram.size(), &senderIP, &senderPort) == -1)
			continue;

		QList<QByteArray> list = datagram.split('@');
		if (list.size() != 2)
			continue;

		quint16 senderServerPort = (quint16)list.at(1).toInt();

		if (isLocalHostAddress(senderIP)) // ignore connections from localhost
			continue;

		if (list.at(0) == "newClient" && senderServerPort == 0 && mBroadcasting) { // new Client broadcast, answer with broadcast if instance is server
			sendBroadcast();
			return;
		}

		//qDebug() << "Broadcast received from" << senderIP << ":" << senderPort << " :"<< list.at(0) << "@" << senderServerPort;
		emit(udpSocketNewServerOnline(senderIP, senderServerPort, list.at(0)));

	}
}

bool DkLANUdpSocket::isLocalHostAddress(const QHostAddress & address) {
	if (mLocalIpAddresses.empty())
		checkLocalIpAddresses();
	foreach(QHostAddress localAddress, mLocalIpAddresses) {
		//qDebug() << "comparing addresses: " << localAddress << " or " << address.toIPv4Address() << " vs " << localAddress.toIPv4Address() << (address.toIPv4Address() == localAddress.toIPv4Address());
		if (address.toIPv4Address() == localAddress.toIPv4Address()) {
			return true;
		}
	}
	return false;
}

void DkLANUdpSocket::checkLocalIpAddresses() {
	mLocalIpAddresses.clear();
	QList<QHostAddress> localAddresses = QNetworkInterface::allAddresses();
	for (int i = 0; i < localAddresses.size(); i++) {
		if (localAddresses.at(i).toIPv4Address()) {
			//qDebug() << "adding local address:" << localAddresses.at(i);
			mLocalIpAddresses << localAddresses.at(i);
		}
	}
}
// DkPeer --------------------------------------------------------------------

//DkPeer::DkPeer(QObject* parent) : QObject(parent) {
//	this->peerId = 0;
//	this->localServerPort = 0;
//	this->hostAddress = QHostAddress();
//	this->peerServerPort= 0;
//	this->title = "empty Peer";
//	this->connection = 0;
//	this->sychronized = false;
//	this->timer = new QTimer(this);
//	this->hasChangedRecently = false;
//	this->clientName = "";
//	//connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeout()));
//	connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeout()), Qt::UniqueConnection);
//}

DkPeer::DkPeer(quint16 port, quint16 peerId, const QHostAddress& hostAddress, quint16 peerServerPort, const QString& title, DkConnection* connection, bool sychronized, const QString& clientName, bool showInMenu, QObject* parent) : QObject(parent) {
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
	this->hasChangedRecently = false;
	//connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeout()));
	connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeout()), Qt::UniqueConnection);
}

//DkPeer::DkPeer(const DkPeer& peer) {
//	this->peerId = peer.peerId;
//	this->localServerPort = peer.localServerPort;
//	this->peerServerPort = peer.peerServerPort;
//	this->sychronized = peer.sychronized;
//	this->title = peer.title;
//	this->connection = peer.connection;	// that's dangerous!!
//	this->hasChangedRecently = peer.hasChangedRecently;
//	this->timer = new QTimer(this); 
//	this->hostAddress = peer.hostAddress;
//	this->clientName = peer.clientName;
//	this->showInMenu = peer.showInMenu;
//
//	connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeout()), Qt::UniqueConnection);
//	//connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeout()));
//}

DkPeer::~DkPeer() {

	// don't (see copy constructor)
	//if (connection)
	//	delete connection;
	//if (timer)
	//	delete timer;

}

void DkPeer::setSynchronized(bool flag) {
	sychronized = flag;
	hasChangedRecently = true;
	connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeout()), Qt::UniqueConnection);
	timer->start(4000);
};

bool DkPeer::operator==(const DkPeer& peer) const {

	return localServerPort == peer.localServerPort && sychronized == peer.sychronized && title == peer.title && hostAddress == peer.hostAddress;
}

//DkPeer& DkPeer::operator=(const DkPeer& peer) {
//	this->peerId = peer.peerId;
//	this->localServerPort = peer.localServerPort;
//	this->peerServerPort = peer.peerServerPort;
//	this->sychronized = peer.sychronized;
//	this->title = peer.title;
//	this->connection = peer.connection;
//	this->hasChangedRecently = peer.hasChangedRecently;
//	this->timer = peer.timer; 
//	this->hostAddress = peer.hostAddress;
//	this->clientName = peer.clientName;
//	this->showInMenu = peer.showInMenu;
//
//	connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeout()), Qt::UniqueConnection);
//
//	return *this;
//}



// DkPeerList --------------------------------------------------------------------
DkPeerList::DkPeerList() {
	// do nothing
}

bool DkPeerList::addPeer(DkPeer* peer) {
	
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

bool DkPeerList::removePeer(quint16 peerId) {
	if (!peerList.contains(peerId))
		return false;
	else {
		peerList.remove(peerId);
		return true;
	}
}

bool DkPeerList::setSynchronized(quint16 peerId, bool synchronized) {
		
	if (!peerList.contains(peerId))
		return false;
	DkPeer* peer = peerList.value(peerId);
	peer->setSynchronized(synchronized);
	//peerList.replace(peerId, peer);

	return true;
}

bool DkPeerList::setTitle(quint16 peerId, const QString& title) {
	if (!peerList.contains(peerId))
		return false;
	DkPeer* peer = peerList.value(peerId);
	peer->title = title;
	//peerList.replace(peerId, peer);

	return true;
}

bool DkPeerList::setShowInMenu(quint16 peerId, bool showInMenu) {
	if (!peerList.contains(peerId))
		return false;
	DkPeer* peer = peerList.value(peerId);
	peer->showInMenu = showInMenu;
	//peerList.replace(peerId, peer);

	return true;
}

QList<DkPeer*> DkPeerList::getSynchronizedPeers() const {
	QList<DkPeer*> sychronizedPeers;
	foreach(DkPeer* peer, peerList) {
		if (peer->isSynchronized())
			sychronizedPeers.push_back(peer);
	}
	return sychronizedPeers;
}

QList<DkPeer*> DkPeerList::getPeerList() {
	return peerList.values();
}

QList<quint16> DkPeerList::getSynchronizedPeerServerPorts() const {
	QList<quint16> sychronizedPeerServerPorts;
	foreach(DkPeer* peer, peerList) {
		if (peer->isSynchronized())
			sychronizedPeerServerPorts.push_back(peer->peerServerPort);
	}
	return sychronizedPeerServerPorts;
}

QList<DkPeer*> DkPeerList::getActivePeers() const {
	QList<DkPeer*> activePeers;
	foreach(DkPeer* peer, peerList) {
		if (peer->isActive())
			activePeers.push_back(peer);
	}
	return activePeers;
}

DkPeer* DkPeerList::getPeerByServerport(quint16 port) const {
	foreach (DkPeer* peer, peerList) {
		if (peer->peerServerPort == port)
			return peer;
	}
	return 0;
}

bool DkPeerList::alreadyConnectedTo(const QHostAddress& address, quint16 port) const {
	foreach (DkPeer* peer, peerList) {
		if (peer->hostAddress == address && peer->localServerPort == port ) // TODO: wieso localserver port ... aber es funkt
			return true;
	}
	return false;
}

DkPeer* DkPeerList::getPeerById(quint16 id) {
	return peerList.value(id, 0);
}

DkPeer* DkPeerList::getPeerByAddress(const QHostAddress& address, quint16 port) const {
	foreach (DkPeer* peer, peerList) {
		if (peer->hostAddress == address && peer->localServerPort == port)
			return peer;
	}
	return 0; // should not happen
}

void DkPeerList::print() const {
	foreach (DkPeer* peer, peerList) {
		
		if (!peer)
			continue;
		
		qDebug() << peer->peerId << " " << peer->clientName << " " << peer->hostAddress << " serverPort:" << peer->peerServerPort << 
			" localPort:" << peer->localServerPort << " " << peer->title << " sync:" << peer->isSynchronized() << " menu:" << peer->showInMenu << " connection:" << peer->connection;
	}
}

// DkManagerThread  --------------------------------------------------------------------
DkManagerThread::DkManagerThread(DkNoMacs* parent) {
	this->parent = parent;
}

void DkManagerThread::connectClient() {

	if (!parent)
		return;

	// this is definitely not elegant
	// we connect on the thread (don't know exactly why)
	// however, the issues we get is some nasty includes here...
	DkViewPort* vp = parent->viewport();

	connect(this, SIGNAL(clientInitializedSignal()), parent, SLOT(clientInitialized()));
	connect(this, SIGNAL(syncWithSignal(quint16)), clientManager, SLOT(synchronizeWith(quint16)));
	connect(this, SIGNAL(stopSyncWithSignal(quint16)), clientManager, SLOT(stopSynchronizeWith(quint16)));

	// TCP communication
	connect(vp, SIGNAL(sendTransformSignal(QTransform, QTransform, QPointF)), clientManager, SLOT(sendTransform(QTransform, QTransform, QPointF)));
	connect(parent, SIGNAL(sendPositionSignal(QRect, bool)), clientManager, SLOT(sendPosition(QRect, bool)));
	connect(parent, SIGNAL(synchronizeRemoteControl(quint16)), clientManager, SLOT(synchronizeWith(quint16)));
	connect(parent, SIGNAL(synchronizeWithServerPortSignal(quint16)), clientManager, SLOT(synchronizeWithServerPort(quint16)));

	connect(parent, SIGNAL(sendTitleSignal(const QString&)), clientManager, SLOT(sendTitle(const QString&)));
	connect(vp, SIGNAL(sendNewFileSignal(qint16, const QString&)), clientManager, SLOT(sendNewFile(qint16, const QString&)));
	connect(clientManager, SIGNAL(receivedNewFile(qint16, const QString&)), vp, SLOT(tcpLoadFile(qint16, const QString&)));
	connect(clientManager, SIGNAL(updateConnectionSignal(QList<DkPeer*>)), vp, SLOT(tcpShowConnections(QList<DkPeer*>)));

	connect(clientManager, SIGNAL(receivedTransformation(QTransform, QTransform, QPointF)), vp, SLOT(tcpSetTransforms(QTransform, QTransform, QPointF)));
	connect(clientManager, SIGNAL(receivedPosition(QRect, bool, bool)), parent, SLOT(tcpSetWindowRect(QRect, bool, bool)));
}

void DkManagerThread::run() {
	
	QString title = "no title";
	if (parent) title = parent->windowTitle();

	mutex.lock();
	createClient(title);
	connectClient();
	mutex.unlock();

	if (parent) title = parent->windowTitle();
	clientManager->sendTitle(title);	// if title is added before title slot is connected...
	emit clientInitializedSignal();

	exec();

	// call the destructor from the thread -> it was created here!
	if (clientManager) {
		delete clientManager;
		clientManager = 0;
	}

	qDebug() << "quitting in da thread...";
}

void DkManagerThread::quit() {
	
	qDebug() << "quitting thread...";
	sendGoodByeToAll();
	
	QThread::quit();
}

// DkLocalMangagerThread --------------------------------------------------------------------
DkLocalManagerThread::DkLocalManagerThread(DkNoMacs* parent) : DkManagerThread(parent) {
	// nothing todo here yet
	clientManager = 0;
}

void DkLocalManagerThread::connectClient() {

	// just for local client
	connect(parent, SIGNAL(sendArrangeSignal(bool)), clientManager, SLOT(sendArrangeInstances(bool)));
	connect(parent, SIGNAL(sendQuitLocalClientsSignal()), clientManager, SLOT(sendQuitMessageToPeers()));
	
	// this connection to parent is only needed for the local client (synchronize all instances)
	connect(parent, SIGNAL(synchronizeWithSignal(quint16)), clientManager, SLOT(synchronizeWith(quint16)));
	DkManagerThread::connectClient();
}

void DkLocalManagerThread::createClient(const QString& title) {

	DkTimer dt;
	if (clientManager)
		delete clientManager;

	// remember: if we set this as parent, we get a warning (different threads)
	// but: take a look at a line which should be about 40 lines from here : )
	clientManager = new DkLocalClientManager(title, 0);

	qDebug() << "client created in: " << dt.getTotal();	// takes 1 sec in the main thread
}

// DkLanManagerThread --------------------------------------------------------------------
DkLanManagerThread::DkLanManagerThread(DkNoMacs* parent) : DkManagerThread(parent) {
	
	clientManager = 0;
}

void DkLanManagerThread::connectClient() {

	connect(parent->viewport(), SIGNAL(sendImageSignal(QImage, const QString&)), clientManager, SLOT(sendNewImage(QImage, const QString&)));
	connect(clientManager, SIGNAL(receivedImage(const QImage &)), parent->viewport(), SLOT(loadImage(const QImage&)));
	connect(clientManager, SIGNAL(receivedImageTitle(const QString&)), parent, SLOT(setWindowTitle(const QString&)));
	connect(this, SIGNAL(startServerSignal(bool)), clientManager, SLOT(startServer(bool)));
	connect(this, SIGNAL(goodByeToAllSignal()), clientManager, SLOT(sendGoodByeToAll()));

	// TODO: uncomment OR do a better signaling here...
	//connect(clientManager, SIGNAL(sendInfoSignal(const QString&, int)), parent->viewport()->getController(), SLOT(setInfo(const QString&, int)));

#ifdef WITH_UPNP
	qRegisterMetaType<QHostAddress>("QHostAddress");
	connect(upnpControlPoint.data(), SIGNAL(newLANNomacsFound(const QHostAddress&, quint16, const QString&)), clientManager, SLOT(startConnection(const QHostAddress&, quint16, const QString&)), Qt::QueuedConnection);
	connect(clientManager, SIGNAL(serverPortChanged(quint16)), upnpDeviceHost.data(), SLOT(tcpServerPortChanged(quint16)), Qt::QueuedConnection);
	
#endif // WITH_UPNP


	DkManagerThread::connectClient();
}

void DkLanManagerThread::createClient(const QString& title) {

	if (clientManager)
		delete clientManager;

	clientManager = new DkLANClientManager(title);
}

// DkRCManagerThread --------------------------------------------------------------------
DkRCManagerThread::DkRCManagerThread(DkNoMacs* parent) : DkLanManagerThread(parent) {
	clientManager = 0;
}

void DkRCManagerThread::createClient(const QString& title) {
	if (clientManager)
		delete clientManager;

	clientManager = new DkRCClientManager(title);

}

void DkRCManagerThread::connectClient() {
	// not sure if we need something here

	//connect(parent->mViewport(), SIGNAL(sendImageSignal(QImage, const QString&)), clientManager, SLOT(sendNewImage(QImage, const QString&)));
	//connect(clientManager, SIGNAL(receivedImage(const QImage&)), parent->mViewport(), SLOT(loadImage(const QImage&)));
	//connect(clientManager, SIGNAL(sendInfoSignal(const QString&, int)), parent->mViewport()->getController(), SLOT(setInfo(const QString&, int)));
	//connect(clientManager, SIGNAL(receivedImageTitle(const QString&)), parent, SLOT(setWindowTitle(const QString&)));
	//connect(this, SIGNAL(startServerSignal(bool)), clientManager, SLOT(startServer(bool)));
	connect(this, SIGNAL(newModeSignal(int)), clientManager, SLOT(sendNewMode(int)));
	connect(parent, SIGNAL(stopSynchronizeWithSignal()), clientManager, SLOT(stopSynchronizeWith()));
	connect(clientManager, SIGNAL(connectedReceivedNewMode(int)), parent, SLOT(tcpChangeSyncMode(int)));

	DkLanManagerThread::connectClient();

#ifdef WITH_UPNP
	 //disconnect signals made by lan manager thread
	disconnect(upnpControlPoint.data(), SIGNAL(newLANNomacsFound(const QHostAddress&, quint16, const QString&)), clientManager, SLOT(startConnection(const QHostAddress&, quint16, const QString&)));
	disconnect(clientManager, SIGNAL(serverPortChanged(quint16)), upnpDeviceHost.data(), SLOT(tcpServerPortChanged(quint16)));
	connect(upnpControlPoint.data(), SIGNAL(newRCNomacsFound(const QHostAddress&, quint16, const QString&)), clientManager, SLOT(startConnection(const QHostAddress&, quint16, const QString&)), Qt::QueuedConnection);
	connect(clientManager, SIGNAL(serverPortChanged(quint16)), upnpDeviceHost.data(), SLOT(wlServerPortChanged(quint16)), Qt::QueuedConnection);
#endif // WITH_UPNP

}

void DkRCManagerThread::sendNewMode(int mode) {
	newModeSignal(mode);
}

}
