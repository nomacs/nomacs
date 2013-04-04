/*******************************************************************************************************
 DkNetwork.cpp
 Created on:	20.07.2011
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2012 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2012 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2012 Florian Kleber <florian@nomacs.org>

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

namespace nmc {

// DkClientManager --------------------------------------------------------------------
DkClientManager::DkClientManager(QString title, QObject* parent) {
	newPeerId = 0;
	this->currentTitle = title;
	qRegisterMetaType<QList<quint16> >("QList<quint16>");
	qRegisterMetaType<QList<DkPeer> >("QList<DkPeer>");
}

DkClientManager::~DkClientManager() {
	
	sendGoodByeToAll();	// does nothing...
};


void DkClientManager::connectionReadyForUse(quint16 peerServerPort, QString title, DkConnection* connection) {
	qDebug() << "connection ready for use" << connection->peerPort() << " with title:" << title << " peerServerPort:" << peerServerPort;

	newPeerId++;
	DkPeer peer = DkPeer(connection->peerPort(), newPeerId, connection->peerAddress(), peerServerPort, title, connection, false);
	connection->setPeerId(newPeerId);
	peerList.addPeer(peer); 

	//connect(this,SIGNAL(sendNewTitleMessage(QString)), peer.connection, SLOT(sendNewTitleMessage(QString)));
	//emit sendNewTitleMessage(title);
	//disconnect(this,SIGNAL(sendNewTitleMessage(QString)), peer.connection, SLOT(sendNewTitleMessage(QString)));
	sendTitle(currentTitle);

}

void DkClientManager::disconnected() {

	if (DkConnection *connection = qobject_cast<DkConnection *>(sender())) {		
		removeConnection(connection);
	}


}

void DkClientManager::removeConnection(DkConnection* connection) {
	peerList.setSynchronized(connection->getPeerId(), false);
	emit synchronizedPeersListChanged(peerList.getSynchronizedPeerServerPorts());
	emit updateConnectionSignal(peerList.getActivePeers());	

	qDebug() << "connection Disconnected:" << connection->getPeerPort();
	peerList.removePeer(connection->getPeerId());

	//qDebug() << "--------------------";
	//qDebug() << "current peer list:";
	//peerList.print();
	//qDebug() << "--------------------";
	emit synchronizedPeersListChanged(peerList.getSynchronizedPeerServerPorts());
	emit updateConnectionSignal(peerList.getActivePeers());
}

void DkClientManager::connectionSentNewTitle(DkConnection* connection, QString newTitle) {
	peerList.setTitle(connection->getPeerId(), newTitle);
}

void DkClientManager::connectionReceivedTransformation(DkConnection* connection, QTransform transform, QTransform imgTransform, QPointF canvasSize) {
	emit receivedTransformation(transform, imgTransform, canvasSize);
}

void DkClientManager::connectionReceivedPosition(DkConnection* connection, QRect rect, bool opacity, bool overlaid) {
	emit receivedPosition(rect, opacity, overlaid);
}

void DkClientManager::connectionReceivedNewFile(DkConnection* connection, qint16 op, QString filename) {
	emit receivedNewFile(op, filename);
}

void DkClientManager::connectionReceivedGoodBye(DkConnection* connection) {
	peerList.removePeer(connection->getPeerId());
	qDebug() << "goodbye received from " << connection->getPeerId();

	emit synchronizedPeersListChanged(peerList.getSynchronizedPeerServerPorts());
	emit updateConnectionSignal(peerList.getActivePeers());
}

void DkClientManager::connectionShowStatusMessage(DkConnection* connection, QString msg) {
	emit sendInfoSignal(msg, 2000);
}

void DkClientManager::sendTitle(QString newTitle) {
	this->currentTitle = newTitle;

	QList<DkPeer> peers = peerList.getPeerList();
	foreach (DkPeer peer , peers) {
		connect(this,SIGNAL(sendNewTitleMessage(QString)), peer.connection, SLOT(sendNewTitleMessage(QString)));
		emit sendNewTitleMessage(newTitle);
		disconnect(this,SIGNAL(sendNewTitleMessage(QString)), peer.connection, SLOT(sendNewTitleMessage(QString)));
	}

}

void DkClientManager::sendTransform(QTransform transform, QTransform imgTransform, QPointF canvasSize) {
	QList<DkPeer> synchronizedPeers = peerList.getSynchronizedPeers();
	foreach (DkPeer peer , synchronizedPeers) {
		connect(this,SIGNAL(sendNewTransformMessage(QTransform, QTransform, QPointF)), peer.connection, SLOT(sendNewTransformMessage(QTransform, QTransform, QPointF)));
		emit sendNewTransformMessage(transform, imgTransform, canvasSize);
		disconnect(this,SIGNAL(sendNewTransformMessage(QTransform, QTransform, QPointF)), peer.connection, SLOT(sendNewTransformMessage(QTransform, QTransform, QPointF)));
	}
}

void DkClientManager::sendPosition(QRect newRect, bool overlaid) {
	QList<DkPeer> synchronizedPeers = peerList.getSynchronizedPeers();
	foreach (DkPeer peer , synchronizedPeers) {
		connect(this,SIGNAL(sendNewPositionMessage(QRect, bool, bool)), peer.connection, SLOT(sendNewPositionMessage(QRect, bool, bool)));
		emit sendNewPositionMessage(newRect, true, overlaid);
		disconnect(this,SIGNAL(sendNewPositionMessage(QRect, bool, bool)), peer.connection, SLOT(sendNewPositionMessage(QRect, bool, bool)));
	}
}

void DkClientManager::sendNewFile(qint16 op, QString filename) {
	QList<DkPeer> synchronizedPeers = peerList.getSynchronizedPeers();
	foreach (DkPeer peer , synchronizedPeers) {
		connect(this,SIGNAL(sendNewFileMessage(qint16, QString)), peer.connection, SLOT(sendNewFileMessage(qint16, QString)));
		emit sendNewFileMessage(op, filename);
		disconnect(this,SIGNAL(sendNewFileMessage(qint16, QString)), peer.connection, SLOT(sendNewFileMessage(qint16, QString)));
	}
}

void DkClientManager::newConnection( int socketDescriptor ) {
	DkConnection* connection = createConnection();
	connection->setSocketDescriptor(socketDescriptor);
	connection->setTitle(currentTitle);
	startUpConnections.append(connection);
	qDebug() << "new Connection " << connection->peerPort();
}

void DkClientManager::connectConnection(DkConnection* connection) {
	qRegisterMetaType<QList<quint16> >("QList<quint16>");
	connect(connection, SIGNAL(connectionReadyForUse(quint16, QString, DkConnection*)), this, SLOT(connectionReadyForUse(quint16, QString , DkConnection*)));
	connect(connection, SIGNAL(connectionStopSynchronize(DkConnection*)), this, SLOT(connectionStopSynchronized(DkConnection*)));
	connect(connection, SIGNAL(connectionStartSynchronize(QList<quint16>,DkConnection*)), this, SLOT(connectionSynchronized(QList<quint16>,DkConnection*)));
	connect(connection, SIGNAL(disconnected()), this, SLOT(disconnected()));
	connect(connection, SIGNAL(connectionTitleHasChanged(DkConnection*, QString)), this, SLOT(connectionSentNewTitle(DkConnection*, QString)));
	connect(connection, SIGNAL(connectionNewPosition(DkConnection*, QRect, bool, bool)), this, SLOT(connectionReceivedPosition(DkConnection*, QRect, bool, bool)));
	connect(connection, SIGNAL(connectionNewTransform(DkConnection*, QTransform, QTransform, QPointF)), this, SLOT(connectionReceivedTransformation(DkConnection*, QTransform, QTransform, QPointF)));
	connect(connection, SIGNAL(connectionNewFile(DkConnection*, qint16, QString)), this, SLOT(connectionReceivedNewFile(DkConnection*, qint16, QString)));
	connect(connection, SIGNAL(connectionGoodBye(DkConnection*)), this, SLOT(connectionReceivedGoodBye(DkConnection*)));
	connect(connection, SIGNAL(connectionShowStatusMessage(DkConnection*, QString)), this, SLOT(connectionShowStatusMessage(DkConnection*, QString)));

	connection->synchronizedPeersListChanged(peerList.getSynchronizedPeerServerPorts());
}

void DkClientManager::sendGoodByeToAll() {
	
	foreach (DkPeer peer, peerList.getPeerList()) {
		
		connect(this,SIGNAL(sendGoodByeMessage()), peer.connection, SLOT(sendNewGoodbyeMessage()));
		emit sendGoodByeMessage();
		disconnect(this,SIGNAL(sendGoodByeMessage()), peer.connection, SLOT(sendNewGoodbyeMessage()));
	}
}

// DkLocalClientManager --------------------------------------------------------------------

DkLocalClientManager::DkLocalClientManager(QString title, QObject* parent ) : DkClientManager(title, parent) {
	server = new DkLocalTcpServer();
	connect(server, SIGNAL(serverReiceivedNewConnection(int)), this, SLOT(newConnection(int)));
	searchForOtherClients();
	//QFuture<void> future = QtConcurrent::run(this, &DkLocalClientManager::searchForOtherClients);
}

QList<DkPeer> DkLocalClientManager::getPeerList() {
	return peerList.getPeerList();
}

quint16 DkLocalClientManager::getServerPort() {
	return server->serverPort();
}

void DkLocalClientManager::synchronizeWithServerPort(quint16 port) {
	qDebug() << "DkClientManager::synchronizeWithServerPort port:" << port;
	DkPeer peer = peerList.getPeerByServerport(port);
	synchronizeWith(peer.peerId);
}

void DkLocalClientManager::searchForOtherClients() {
		
	for (int i = server->startPort; i <= server->endPort; i++) {
		if (i == server->serverPort())
			continue;
		//qDebug() << "search For other clients on port:" << i;
		DkConnection* connection = createConnection();
		connection->connectToHost(QHostAddress::LocalHost,i);

		if (connection->waitForConnected(10))
			qDebug() << "Connected to " << i ;

		connection->sendGreetingMessage(currentTitle);
		startUpConnections.append(connection);
	}
	
}

void DkLocalClientManager::run() {
	
	//exec();
}

void DkLocalClientManager::connectionSynchronized(QList<quint16> synchronizedPeersOfOtherClient, DkConnection* connection) {
	qDebug() << "Connection synchronized with:" << connection->getPeerPort();
	peerList.setSynchronized(connection->getPeerId(), true);
	emit synchronizedPeersListChanged(peerList.getSynchronizedPeerServerPorts());

	emit updateConnectionSignal(peerList.getActivePeers());
	
	for (int i=0; i < synchronizedPeersOfOtherClient.size(); i++) {
		if (synchronizedPeersOfOtherClient[i]!=server->serverPort()) {
			
			DkPeer peer = peerList.getPeerByServerport(synchronizedPeersOfOtherClient[i]);
			if (peer.getSynchronized())
				continue;

			connect(this,SIGNAL(sendSynchronizeMessage()), peer.connection, SLOT(sendStartSynchronizeMessage()));
			emit sendSynchronizeMessage();
			disconnect(this,SIGNAL(sendSynchronizeMessage()), peer.connection, SLOT(sendStartSynchronizeMessage()));				

		}
	}
	//qDebug() << "--------------------";
	//qDebug() << "current peer list:";
	//peerList.print();
	//qDebug() << "--------------------";
}

void DkLocalClientManager::connectionStopSynchronized(DkConnection* connection) {
	qDebug() << "Connection no longer synchronized with: " << connection->getPeerPort();
	peerList.setSynchronized(connection->getPeerId(), false);

	//qDebug() << "--------------------";
	//qDebug() << "current peer list:";
	//peerList.print();
	//qDebug() << "--------------------";
	emit synchronizedPeersListChanged(peerList.getSynchronizedPeerServerPorts());
	emit updateConnectionSignal(peerList.getActivePeers());
}


void DkLocalClientManager::synchronizeWith(quint16 peerId) {
	qDebug() << "DkClientManager::synchronizeWith  peerId:" << peerId;
	QList<DkPeer> peers = peerList.getPeerList();

	peerList.setSynchronized(peerId, true); // will be reset if other client does not response within 1 sec
	emit synchronizedPeersListChanged(peerList.getSynchronizedPeerServerPorts());

	DkPeer peer = peerList.getPeerById(peerId);
	if (peer.connection == 0) {
		qDebug() << "TcpClient: synchronizeWith: connection is null";
		return;
	}

	qDebug() << "synchronizing with: " << peerId;


	connect(this,SIGNAL(sendSynchronizeMessage()), peer.connection, SLOT(sendStartSynchronizeMessage()));
	emit sendSynchronizeMessage();
	disconnect(this,SIGNAL(sendSynchronizeMessage()), peer.connection, SLOT(sendStartSynchronizeMessage()));

}

void DkLocalClientManager::stopSynchronizeWith(quint16 peerId) {
	QList<DkPeer> synchronizedPeers = peerList.getSynchronizedPeers();
	foreach (DkPeer peer , synchronizedPeers) {
		connect(this,SIGNAL(sendDisableSynchronizeMessage()), peer.connection, SLOT(sendStopSynchronizeMessage()));
		emit sendDisableSynchronizeMessage();
		peerList.setSynchronized(peer.peerId, false);
		disconnect(this,SIGNAL(sendDisableSynchronizeMessage()), peer.connection, SLOT(sendStopSynchronizeMessage()));
	}

	emit synchronizedPeersListChanged(peerList.getSynchronizedPeerServerPorts());
	emit updateConnectionSignal(peerList.getActivePeers());	
}

void DkLocalClientManager::sendArrangeInstances(bool overlaid) {
	int screen = QApplication::desktop()->screenNumber(QApplication::activeWindow());
	const QRect screenGeometry =  QApplication::desktop()->availableGeometry(screen);
	int connectedInstances = peerList.getSynchronizedPeers().size() + 1 ; // +1 because of this instance itself
	if (connectedInstances == 1)
		return;
	int instancesPerRow = (connectedInstances == 2 || connectedInstances == 4) ? 2 : 3;

	int rows = ceil((float)connectedInstances / (float)instancesPerRow);
	int width = screenGeometry.width() / instancesPerRow;
	int height = screenGeometry.height() / rows;

	int curX = screenGeometry.topLeft().x();
	int curY = screenGeometry.topLeft().y();
	emit receivedPosition(QRect(curX, curY, width, height), false, overlaid);
	curX += width;
	int count = 1;
	foreach (DkPeer peer, peerList.getSynchronizedPeers()) {
		QRect newPosition = QRect(curX, curY, width, height);
		connect(this,SIGNAL(sendNewPositionMessage(QRect, bool, bool)), peer.connection, SLOT(sendNewPositionMessage(QRect, bool, bool)));
		emit sendNewPositionMessage(newPosition, false, overlaid);
		disconnect(this,SIGNAL(sendNewPositionMessage(QRect, bool, bool)), peer.connection, SLOT(sendNewPositionMessage(QRect, bool, bool)));
		
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
	DkLocalConnection* connection = new DkLocalConnection();
	connection->setLocalTcpServerPort(server->serverPort());
	connection->setTitle(currentTitle);
	connectConnection(connection);
	connect(this, SIGNAL(synchronizedPeersListChanged(QList<quint16>)), connection, SLOT(synchronizedPeersListChanged(QList<quint16>)));
	connect(this, SIGNAL(sendQuitMessage()), connection, SLOT(sendQuitMessage()));
	connect(connection, SIGNAL(connectionQuitReceived()), this, SLOT(connectionReceivedQuit()));
	return connection;

}
// DkLANClientManager --------------------------------------------------------------------
DkLANClientManager::DkLANClientManager(QString title, QObject* parent) : DkClientManager(title, parent) {
	server = new DkLANTcpServer();
	connect(server, SIGNAL(serverReiceivedNewConnection(QHostAddress, quint16, QString)), this, SLOT(startConnection(QHostAddress, quint16, QString)));
	connect(server, SIGNAL(serverReiceivedNewConnection(int)), this, SLOT(newConnection(int)));
	connect(server, SIGNAL(sendStopSynchronizationToAll()), this, SLOT(sendStopSynchronizationToAll()));
}

QList<DkPeer> DkLANClientManager::getPeerList() {
	QList<DkPeer> list;
	foreach(DkPeer peer, peerList.getPeerList()) {
		if (peer.showInMenu)
			list.push_back(peer);
	}
	return list;
}

void DkLANClientManager::startConnection(QHostAddress address, quint16 port, QString clientName) {
	//qDebug() << "DkLANClientManager::startConnection: connecting to:" << address << ":" << port;
	if (peerList.alreadyConnectedTo(address, port)) {
		//qDebug() << "already connected";
		return;
	}


	DkLANConnection* connection = createConnection();
	connection->setClientName(clientName);
	connection->connectToHost(address, port);
	connection->setIAmServer(false);

	if (connection->waitForConnected(10))
		qDebug() << "Connected to " << address << ":" << port ;

	connection->sendGreetingMessage(currentTitle);
	connection->setShowInMenu(true);
}

void DkLANClientManager::connectionReadyForUse(quint16 peerServerPort, QString title, DkConnection* dkconnection) {
	DkLANConnection* connection = dynamic_cast<DkLANConnection*>(dkconnection); // TODO???? darf ich das
	//qDebug() << "connection ready for use" << connection->peerPort() << " with title:" << title << " peerServerPort:" << peerServerPort << "  showInMenu: " << connection->getShowInMenu();

	newPeerId++;
	DkPeer peer = DkPeer(connection->peerPort(), newPeerId, connection->peerAddress(), peerServerPort, title, connection, false, connection->getClientName(), connection->getShowInMenu());
	connection->setPeerId(newPeerId);
	peerList.addPeer(peer); 

	//qDebug() << "--------------------";
	//qDebug() << "current peer list:";
	//peerList.print();
	//qDebug() << "--------------------";

}



void DkLANClientManager::connectionSentNewTitle(DkConnection* connection, QString newTitle) {
	peerList.setTitle(connection->getPeerId(), newTitle);

	// propagate this message
	QList<DkPeer> syncPeerList = peerList.getSynchronizedPeers();
	foreach (DkPeer peer, syncPeerList) {
		if (peer.peerId != connection->getPeerId())
			peer.connection->sendNewTitleMessage(newTitle);
	}

}


void DkLANClientManager::connectionSynchronized(QList<quint16> synchronizedPeersOfOtherClient, DkConnection* connection) {
	//qDebug() << "Connection synchronized with:" << connection->getPeerPort();
	peerList.setSynchronized(connection->getPeerId(), true);
	peerList.setShowInMenu(connection->getPeerId(), true);
	emit synchronizedPeersListChanged(peerList.getSynchronizedPeerServerPorts());
	emit updateConnectionSignal(peerList.getActivePeers());

	// ignore synchronized clients of other connection
}

void DkLANClientManager::connectionStopSynchronized(DkConnection* connection) {
	qDebug() << "Connection no longer synchronized with: " << connection->getPeerPort();
	peerList.setSynchronized(connection->getPeerId(), false);
	peerList.setShowInMenu(connection->getPeerId(), false);
	//qDebug() << "--------------------";
	//qDebug() << "current peer list:";
	//peerList.print();
	//qDebug() << "--------------------";
	emit synchronizedPeersListChanged(peerList.getSynchronizedPeerServerPorts());
	emit updateConnectionSignal(peerList.getActivePeers());
}


void DkLANClientManager::synchronizeWith(quint16 peerId) {
	//qDebug() << "DkClientManager::synchronizeWith  peerId:" << peerId;

	if (server->isListening()) {
		DkPeer newServer = peerList.getPeerById(peerId);
		foreach (DkPeer peer, peerList.getSynchronizedPeers()) {

			connect(this,SIGNAL(sendSwitchServerMessage(QHostAddress, quint16)), peer.connection, SLOT(sendSwitchServerMessage(QHostAddress, quint16)));
			emit sendSwitchServerMessage(newServer.hostAddress, newServer.localServerPort);
			disconnect(this,SIGNAL(sendSwitchServerMessage(QHostAddress, quint16)), peer.connection, SLOT(sendSwitchServerMessage(QHostAddress, quint16)));
			peerList.setShowInMenu(peer.peerId, false);

			connect(this,SIGNAL(sendGoodByeMessage()), peer.connection, SLOT(sendNewGoodbyeMessage()));
			emit sendGoodByeMessage();
			disconnect(this,SIGNAL(sendGoodByeMessage()), peer.connection, SLOT(sendNewGoodbyeMessage()));

			//connect(this,SIGNAL(sendDisableSynchronizeMessage()), peer.connection, SLOT(sendStopSynchronizeMessage()));
			//emit sendDisableSynchronizeMessage();
			//peerList.setSynchronized(peer.peerId, false);
			//disconnect(this,SIGNAL(sendDisableSynchronizeMessage()), peer.connection, SLOT(sendStopSynchronizeMessage()));
		}

		server->startServer(false);
	}


	peerList.setSynchronized(peerId, true); // will be reset if other client does not response within 1 sec
	emit synchronizedPeersListChanged(peerList.getSynchronizedPeerServerPorts());

	DkPeer peer = peerList.getPeerById(peerId);
	if (peer.connection == 0) {
		qDebug() << "TcpClient: synchronizeWith: connection is null";
		return;
	}

	qDebug() << "synchronizing with: " << peerId;
	connect(this,SIGNAL(sendSynchronizeMessage()), peer.connection, SLOT(sendStartSynchronizeMessage()));
	emit sendSynchronizeMessage();
	disconnect(this,SIGNAL(sendSynchronizeMessage()), peer.connection, SLOT(sendStartSynchronizeMessage()));

}


void DkLANClientManager::stopSynchronizeWith(quint16 peerId) {
	DkPeer peer = peerList.getPeerById(peerId);
	connect(this,SIGNAL(sendDisableSynchronizeMessage()), peer.connection, SLOT(sendStopSynchronizeMessage()));
	emit sendDisableSynchronizeMessage();
	disconnect(this,SIGNAL(sendDisableSynchronizeMessage()), peer.connection, SLOT(sendStopSynchronizeMessage()));

	peerList.setSynchronized(peer.peerId, false);
	if (server->isListening()) // i am server
		peerList.setShowInMenu(peerId, false);

	emit synchronizedPeersListChanged(peerList.getSynchronizedPeerServerPorts());
	emit updateConnectionSignal(peerList.getActivePeers());	
}

void DkLANClientManager::connectionReceivedTransformation(DkConnection* connection, QTransform transform, QTransform imgTransform, QPointF canvasSize) {
	emit receivedTransformation(transform, imgTransform, canvasSize);

	// propagate this message
	QList<DkPeer> syncPeerList = peerList.getSynchronizedPeers();
	foreach (DkPeer peer, syncPeerList) {
		if (peer.peerId != connection->getPeerId())
			peer.connection->sendNewTransformMessage(transform, imgTransform, canvasSize);
	}

}

void DkLANClientManager::connectionReceivedPosition(DkConnection* connection, QRect rect, bool opacity, bool overlaid) {
	emit receivedPosition(rect, opacity, overlaid);

	// propagate this message
	QList<DkPeer> syncPeerList = peerList.getSynchronizedPeers();
	foreach (DkPeer peer, syncPeerList) {
		if (peer.peerId != connection->getPeerId())
			peer.connection->sendNewPositionMessage(rect, opacity, overlaid);
	}

}

void DkLANClientManager::connectionReceivedNewFile(DkConnection* connection, qint16 op, QString filename) {
	emit receivedNewFile(op, filename);

	// propagate this message
	QList<DkPeer> syncPeerList = peerList.getSynchronizedPeers();
	foreach (DkPeer peer, syncPeerList) {
		if (peer.peerId != connection->getPeerId())
			peer.connection->sendNewFileMessage(op, filename);
	}

}


void DkLANClientManager::connectionReceivedUpcomingImage(DkConnection* connection, QString imageTitle) {
	//qDebug() << "Connection will receive Image with title: " << imageTitle;
	emit sendInfoSignal("receiving image:\n" + imageTitle, 2000);
}


void DkLANClientManager::connectionReceivedNewImage(DkConnection* connection, QImage image, QString title) {
	//qDebug() << "DkTcpNetworkClient:: connection Received New Image";
	emit receivedImage(image);
	emit receivedImageTitle(title + " - ");
	//qDebug() << "received title: " << title;

	// propagate this message
	QList<DkPeer> syncPeerList = peerList.getSynchronizedPeers();
	foreach (DkPeer peer, syncPeerList) {
		if (peer.peerId != connection->getPeerId()) {
			DkLANConnection* connection = dynamic_cast<DkLANConnection*>(peer.connection); // TODO???? darf ich das
			connect(this,SIGNAL(sendNewImageMessage(QImage, QString)), connection, SLOT(sendNewImageMessage(QImage, QString)));
			emit sendNewImageMessage(image, title);
			connect(this,SIGNAL(sendNewImageMessage(QImage, QString)), connection, SLOT(sendNewImageMessage(QImage, QString)));
		}
	}

}

void DkLANClientManager::connectionReceivedSwitchServer(DkConnection* connection, QHostAddress address, quint16 port) {
	//qDebug() << "DkLANClientManager::connectionReceivedSwitchServer:" << address << ":" << port;
	if (!peerList.alreadyConnectedTo(address, port))
		return;

	DkPeer peer = peerList.getPeerByAddress(address, port);
	connect(this,SIGNAL(sendSynchronizeMessage()), peer.connection, SLOT(sendStartSynchronizeMessage()));
	emit sendSynchronizeMessage();
	disconnect(this,SIGNAL(sendSynchronizeMessage()), peer.connection, SLOT(sendStartSynchronizeMessage()));

	connect(this,SIGNAL(sendDisableSynchronizeMessage()), connection, SLOT(sendStopSynchronizeMessage()));
	emit sendDisableSynchronizeMessage();
	peerList.setSynchronized(peer.peerId, false);
	if (server->isListening()) // i am server
		peerList.setShowInMenu(connection->getPeerId(), false);
	disconnect(this,SIGNAL(sendDisableSynchronizeMessage()), connection, SLOT(sendStopSynchronizeMessage()));

}

void DkLANClientManager::sendTitle(QString newTitle) {
	this->currentTitle = newTitle;

	if (server->isListening()) { // only send title if instance is server
		QList<DkPeer> peers = peerList.getPeerList();
		foreach (DkPeer peer , peers) {
			connect(this,SIGNAL(sendNewTitleMessage(QString)), peer.connection, SLOT(sendNewTitleMessage(QString)));
			emit sendNewTitleMessage(newTitle);
			disconnect(this,SIGNAL(sendNewTitleMessage(QString)), peer.connection, SLOT(sendNewTitleMessage(QString)));
		}
	}
}

void DkLANClientManager::sendNewImage(QImage image, QString title) {
	//qDebug() << "sending new image";
	QList<DkPeer> synchronizedPeers = peerList.getSynchronizedPeers();
	foreach (DkPeer peer , synchronizedPeers) {
		DkLANConnection* connection = dynamic_cast<DkLANConnection*>(peer.connection);
		connect(this,SIGNAL(sendNewUpcomingImageMessage(QString)), connection, SLOT(sendNewUpcomingImageMessage(QString)));
		emit sendNewUpcomingImageMessage(title);
		disconnect(this,SIGNAL(sendNewUpcomingImageMessage(QString)), connection, SLOT(sendNewUpcomingImageMessage(QString)));

		connect(this,SIGNAL(sendNewImageMessage(QImage, QString)), connection, SLOT(sendNewImageMessage(QImage, QString)));
		emit sendNewImageMessage(image, title);
		disconnect(this,SIGNAL(sendNewImageMessage(QImage, QString)), connection, SLOT(sendNewImageMessage(QImage, QString)));
	}
}

void DkLANClientManager::startServer(bool flag) {
	if (!flag) {
		foreach (DkPeer peer, peerList.getPeerList()) {
			peer.connection->sendNewGoodbyeMessage();
		}
	}
	server->startServer(flag);
}


void DkLANClientManager::sendStopSynchronizationToAll() {
	foreach (DkPeer peer, peerList.getSynchronizedPeers()) {
		connect(this,SIGNAL(sendDisableSynchronizeMessage()), peer.connection, SLOT(sendStopSynchronizeMessage()));
		emit sendDisableSynchronizeMessage();
		peerList.setSynchronized(peer.peerId, false);
		disconnect(this,SIGNAL(sendDisableSynchronizeMessage()), peer.connection, SLOT(sendStopSynchronizeMessage()));	
	}
	emit synchronizedPeersListChanged(peerList.getSynchronizedPeerServerPorts());
	emit updateConnectionSignal(peerList.getActivePeers());	

	// second for loop needed for disconnect message
	qDebug() << "disconnecting connections:";
	foreach (DkPeer peer, peerList.getSynchronizedPeers()) {
		qDebug() << peer.connection->disconnect();
		peerList.removePeer(peer.peerId);
	}		
}

DkLANConnection* DkLANClientManager::createConnection() {
	DkLANConnection* connection = new DkLANConnection();
	connectConnection(connection);
	connect(connection, SIGNAL(connectionNewImage(DkConnection*, QImage, QString)), this, SLOT(connectionReceivedNewImage(DkConnection*, QImage, QString)));
	connect(connection, SIGNAL(connectionUpcomingImage(DkConnection*, QString)), this, SLOT(connectionReceivedUpcomingImage(DkConnection*, QString)));
	connect(connection, SIGNAL(connectionSwitchServer(DkConnection*, QHostAddress, quint16)), this, SLOT(connectionReceivedSwitchServer(DkConnection*, QHostAddress, quint16)));
	return connection;
}


// DkLocalTcpServer --------------------------------------------------------------------
DkLocalTcpServer::DkLocalTcpServer(QObject* parent) : QTcpServer(parent) {
	this->startPort = 45454;
	this->endPort = 45484;

	for (int i = startPort; i < endPort; i++) {
		if (listen(QHostAddress::LocalHost, i)) {
			break;
		}
	}
	qDebug() << "TCP Listening on port " << this->serverPort();
}

void DkLocalTcpServer::incomingConnection ( int socketDescriptor )  {
	emit serverReiceivedNewConnection(socketDescriptor);
	qDebug() << "Server: NEW CONNECTION AVAIABLE";
}

// DkLANTcpServer --------------------------------------------------------------------

DkLANTcpServer::DkLANTcpServer( QObject* parent) : QTcpServer(parent) {
	udpSocket = new DkLANUdpSocket();
	connect(udpSocket, SIGNAL(udpSocketNewServerOnline(QHostAddress, quint16, QString)), this, SLOT(udpNewServerFound(QHostAddress, quint16, QString)));
	connect(this, SIGNAL(sendNewClientBroadcast()), udpSocket, SLOT(sendNewClientBroadcast()));
	emit sendNewClientBroadcast();
}

void DkLANTcpServer::startServer(bool flag) {
	qDebug() << "DkLANTcpServer::startServer start: " << flag;
	if (flag) {
		listen(QHostAddress::Any);
		qDebug() << "DkTcpNetworkServer listening on:" << this->serverPort();
		udpSocket->startBroadcast(this->serverPort());
	} else {
		emit(sendStopSynchronizationToAll());
		this->close();
		qDebug() << "DkTcpNetworkServer stopped listening";
		udpSocket->stopBroadcast();
	}
}

void DkLANTcpServer::udpNewServerFound(QHostAddress address , quint16 port , QString clientName) {
	emit(serverReiceivedNewConnection(address , port , clientName));
}

void DkLANTcpServer::incomingConnection ( int socketDescriptor )  {
	emit serverReiceivedNewConnection(socketDescriptor);
}

// DkUpdSocket --------------------------------------------------------------------

DkLANUdpSocket::DkLANUdpSocket( quint16 startPort, quint16 endPort , QObject* parent ) : QUdpSocket(parent) {
	this->startPort = startPort;
	this->endPort = endPort;

	for (serverPort = startPort; serverPort <= endPort; serverPort++) {
		if (bind(QHostAddress::Any, serverPort))
			break;
	}

	qDebug() << "UpdBroadcastserver listening on " << serverPort;
	connect(this, SIGNAL(readyRead()), this, SLOT(readBroadcast()));

	localIpAddresses.clear();
	foreach (QNetworkInterface interface, QNetworkInterface::allInterfaces()) {
		foreach (QNetworkAddressEntry entry, interface.addressEntries()) {
			localIpAddresses << entry.ip();
		}
	}
	broadcasting = false;
}

	void DkLANUdpSocket::startBroadcast(quint16 tcpServerPort) {
	this->tcpServerPort = tcpServerPort;

	sendBroadcast(); // send first broadcast 
	this->broadcastTimer = new QTimer;
	broadcastTimer->setInterval(10000);
	connect(broadcastTimer, SIGNAL(timeout()), this, SLOT(sendBroadcast()));
	broadcastTimer->start();
	broadcasting = true;
}

void DkLANUdpSocket::stopBroadcast() {
	broadcastTimer->stop();
	broadcasting = false;
}

void DkLANUdpSocket::sendBroadcast() {
	QByteArray datagram;
	datagram.append(QHostInfo::localHostName());
	datagram.append("@");
	datagram.append(QByteArray::number(tcpServerPort));
	// datagram.append(serverport) + clientname

	
	for (quint16 port = startPort; port <= endPort; port++) {
		foreach (QNetworkInterface interface, QNetworkInterface::allInterfaces()) {
			foreach (QNetworkAddressEntry entry, interface.addressEntries()) {
				if (entry.broadcast().isNull())
					continue;
				writeDatagram(datagram.data(), datagram.size(), entry.broadcast(), port);
			}
		}
	}


}

void DkLANUdpSocket::sendNewClientBroadcast() {
	QByteArray datagram;
	datagram.append("newClient");
	datagram.append("@");
	datagram.append(QByteArray::number(0));

	for (quint16 port = startPort; port < endPort; port++) 
		writeDatagram(datagram.data(), datagram.size(), QHostAddress::Broadcast, port);
	qDebug() << "sent broadcast:" << datagram << "--- " << 0;
}

void DkLANUdpSocket::readBroadcast() {
	while(this->hasPendingDatagrams()) {
		QHostAddress senderIP;
		quint16 senderPort;
		QByteArray datagram;
		datagram.resize(this->pendingDatagramSize());
		if (readDatagram(datagram.data(), datagram.size(), &senderIP, &senderPort) == -1)
			continue;

		QList<QByteArray> list = datagram.split('@');
		if (list.size() != 2)
			continue;

		quint16 senderServerPort = list.at(1).toInt();

		if (isLocalHostAddress(senderIP)) // ignore connections from localhost
			continue;

		if (list.at(0) == "newClient" && senderServerPort == 0 && broadcasting) { // new Client broadcast, answer with broadcast if instance is server
			sendBroadcast();
		}

		qDebug() << "Broadcast received from" << senderIP << ":" << senderPort << " :"<< list.at(0) << "@" << senderServerPort;
		emit(udpSocketNewServerOnline(senderIP, senderServerPort, list.at(0)));

	}
}

bool DkLANUdpSocket::isLocalHostAddress(const QHostAddress &address) {
	foreach (QHostAddress localAddress, localIpAddresses) {
		if (address == localAddress)
			return true;
	}
	return false;
}


// DkPeer --------------------------------------------------------------------

DkPeer::DkPeer() {
	this->peerId = 0;
	this->localServerPort = 0;
	this->hostAddress = QHostAddress();
	this->peerServerPort= 0;
	this->title = "empty Peer";
	this->connection = 0;
	this->sychronized = false;
	this->timer = new QTimer(this);
	this->hasChangedRecently = false;
	this->clientName = "";
	//connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeout()));
	connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeout()), Qt::UniqueConnection);
}

DkPeer::DkPeer(quint16 port, quint16 peerId, QHostAddress hostAddress, quint16 peerServerPort, QString title, DkConnection* connection, bool sychronized, QString clientName, bool showInMenu) {
	this->peerId = peerId;
	this->localServerPort = port;
	this->peerServerPort = peerServerPort;
	this->hostAddress = hostAddress;
	this->title = title;
	this->sychronized = sychronized;
	this->connection = connection;
	this->timer = new QTimer;
	timer->setSingleShot(true);
	this->clientName = clientName;
	this->showInMenu = showInMenu;
	this->hasChangedRecently = false;
	//connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeout()));
	connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeout()), Qt::UniqueConnection);
}

DkPeer::DkPeer(const DkPeer& peer) {
	this->peerId = peer.peerId;
	this->localServerPort = peer.localServerPort;
	this->peerServerPort = peer.peerServerPort;
	this->sychronized = peer.sychronized;
	this->title = peer.title;
	this->connection = peer.connection;	// that's dangerous!!
	this->hasChangedRecently = peer.hasChangedRecently;
	this->timer = peer.timer; 
	this->hostAddress = peer.hostAddress;
	this->clientName = peer.clientName;
	this->showInMenu = peer.showInMenu;

	connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeout()), Qt::UniqueConnection);
	//connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeout()));
}

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

// DkPeerList --------------------------------------------------------------------
DkPeerList::DkPeerList() {
	// do nothing
}

bool DkPeerList::addPeer(DkPeer peer) {
	
	if (peerList.contains(peer.peerId))
		return false;
	else {
		peerList.insert(peer.peerId, peer);
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
	DkPeer peer = peerList.value(peerId);
	peer.setSynchronized(synchronized);
	peerList.replace(peerId, peer);

	return true;
}

bool DkPeerList::setTitle(quint16 peerId, QString title) {
	if (!peerList.contains(peerId))
		return false;
	DkPeer peer = peerList.value(peerId);
	peer.title = title;
	peerList.replace(peerId, peer);

	return true;
}

bool DkPeerList::setShowInMenu(quint16 peerId, bool showInMenu) {
	if (!peerList.contains(peerId))
		return false;
	DkPeer peer = peerList.value(peerId);
	peer.showInMenu = showInMenu;
	peerList.replace(peerId, peer);

	return true;
}

QList<DkPeer> DkPeerList::getSynchronizedPeers() {
	QList<DkPeer> sychronizedPeers;
	foreach(DkPeer peer, peerList) {
		if (peer.getSynchronized())
			sychronizedPeers.push_back(peer);
	}
	return sychronizedPeers;
}

QList<DkPeer> DkPeerList::getPeerList() {
	return peerList.values();
}

QList<quint16> DkPeerList::getSynchronizedPeerServerPorts() {
	QList<quint16> sychronizedPeerServerPorts;
	foreach(DkPeer peer, peerList) {
		if (peer.getSynchronized())
			sychronizedPeerServerPorts.push_back(peer.peerServerPort);
	}
	return sychronizedPeerServerPorts;
}

QList<DkPeer> DkPeerList::getActivePeers() {
	QList<DkPeer> activePeers;
	foreach(DkPeer peer, peerList) {
		if (peer.isActive())
			activePeers.push_back(peer);
	}
	return activePeers;
}

DkPeer DkPeerList::getPeerByServerport(quint16 port) {
	foreach (DkPeer peer, peerList) {
		if (peer.peerServerPort == port)
			return peer;
	}
	return DkPeer();
}

bool DkPeerList::alreadyConnectedTo(QHostAddress address, quint16 port) {
	foreach (DkPeer peer, peerList) {
		if (peer.hostAddress == address && peer.localServerPort == port ) // TODO: wieso localserver port ... aber es funkt
			return true;
	}
	return false;
}

DkPeer DkPeerList::getPeerById(quint16 id) {
	return peerList.value(id, DkPeer());
}

DkPeer DkPeerList::getPeerByAddress(QHostAddress address, quint16 port) {
	foreach (DkPeer peer, peerList) {
		if (peer.hostAddress == address && peer.localServerPort == port)
			return peer;
	}
	return DkPeer(); // can not happen
}

void DkPeerList::print() {
	foreach (DkPeer peer, peerList) {
		qDebug() << peer.peerId << 	" " << peer.clientName << " " << peer.hostAddress << " " << peer.peerServerPort << 
			" " << peer.localServerPort << " " << peer.title << " sync:" << peer.getSynchronized() << " menu:" << peer.showInMenu;
	}
}

// DkUpdater  --------------------------------------------------------------------

DkUpdater::DkUpdater() {
	silent = true;
	cookie = new QNetworkCookieJar();
	accessManagerSetup.setCookieJar(cookie);
	connect(&accessManagerSetup, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinishedSlot(QNetworkReply*)));
	updateAborted = false;
}

void DkUpdater::checkForUpdated() {

	DkSettings::Sync::lastUpdateCheck = QDate::currentDate();

	DkSettings settings;
	settings.save();

#ifdef Q_WS_WIN
	QUrl url ("http://www.nomacs.org/version_win_stable");
#elif defined Q_WS_X11
	QUrl url ("http://www.nomacs.org/version_linux");
#elif defined Q_WS_MAC
	QUrl url ("http://www.nomacs.org/version_mac_stable");
#else
	QUrl url ("http://www.nomacs.org/version");
#endif

	qDebug() << "checking for updates";
	connect(&accessManagerVersion, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));	
	QNetworkRequest request = QNetworkRequest(url);
	request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
	reply = accessManagerVersion.get(QNetworkRequest(url));
}

void DkUpdater::replyFinished(QNetworkReply* reply) {
	if (reply->error())
		return;

	QString replyData = reply->readAll();

	QStringList sl = replyData.split('\n', QString::SkipEmptyParts);

	QString version, x64, x86, url, mac;
	for(int i = 0; i < sl.length();i++) {
		QStringList values = sl[i].split(" ");
		if (values[0] == "version") 
			version = values[1];
		else if (values[0] == "x64")
			x64 = values[1];
		else if (values[0] == "x86")
			x86 = values[1];
		else if (values[0] == "mac")
			mac = values[1];
	}


#if _MSC_VER == 1600
	url = XPx86;	// for WinXP packages
#elif _WIN32
	url = x86;
#elif defined _WIN64
	url = x64;
#elif defined Q_WS_MAC
	url = mac;
#endif 

	qDebug() << "version:" << version;
	qDebug() << "x64:" << x64;
	qDebug() << "x86:" << x86;
	qDebug() << "mac:" << mac;

	if (!version.isEmpty() && !x64.isEmpty() || !x86.isEmpty()) {		
		QStringList cVersion = QApplication::applicationVersion().split('.');
		QStringList nVersion = version.split('.');

		if (cVersion.size() < 3 || nVersion.size() < 3) {
			qDebug() << "sorry, I could not parse the version number...";

			if (!silent)
				emit showUpdaterMessage(tr("sorry, I could not check for newer versions"), tr("updates"));

			return;
		}
		
		if (nVersion[0].toInt() > cVersion[0].toInt()  ||	// major release
			nVersion[0].toInt() == cVersion[0].toInt() &&	// major release
			nVersion[1].toInt() > cVersion[1].toInt()  ||	// minor release
			nVersion[0].toInt() == cVersion[0].toInt() &&	// major release
			nVersion[1].toInt() == cVersion[1].toInt() &&	// minor release
			nVersion[2].toInt() >  cVersion[2].toInt()) {	// minor-minor release
		
			QString msg = tr("A new version") % " (" % sl[0] % ") " % tr("is available");
			msg = msg % "<br>" % tr("Do you want to download and install it now?");
			msg = msg % "<br>" % tr("For more information see ") + " <a href=\"http://www.nomacs.org\">http://www.nomacs.org</a>";
			nomacsSetupUrl = url;
			setupVersion = version;
			qDebug() << "nomacs setup url:" << nomacsSetupUrl;

			if (!url.isEmpty())
				emit displayUpdateDialog(msg, tr("updates")); 
		}
		else if (!silent)
			emit showUpdaterMessage(tr("nomacs is up-to-date"), tr("updates"));

	}
	
}

void DkUpdater::startDownload(QUrl downloadUrl) {
	if (downloadUrl.isEmpty())
		emit showUpdaterMessage(tr("sorry, unable to download the new version"), tr("updates"));

	qDebug() << "-----------------------------------------------------";
	qDebug() << "starting to download update from " << downloadUrl ;
	
	//updateAborted = false;	// reset - it may have been canceled before
	QNetworkRequest req(downloadUrl);
	req.setRawHeader("User-Agent", " ");
	reply = accessManagerSetup.get(req);
	connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(updateDownloadProgress(qint64, qint64)));
}

void DkUpdater::downloadFinishedSlot(QNetworkReply* data) {
	QUrl redirect = data->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
	if (!redirect.isEmpty() ) {
		qDebug() << "redirecting: " << redirect;
		startDownload(redirect);
		return;
	}

	if (!updateAborted) {
		QString basename = "nomacs-setup";
		QString extension = ".exe";
		QString absoluteFilePath = QDir::tempPath() + "/" + basename + extension;
		if (QFile::exists(absoluteFilePath)) {
			qDebug() << "File already exists - searching for new name";
			// already exists, don't overwrite
			int i = 0;
			while (QFile::exists(absoluteFilePath)) {
				absoluteFilePath = QDir::tempPath() + "/" + basename + "-" + QString::number(i) + extension;
				++i;
			}
		}

		QFile file(absoluteFilePath);
		if (!file.open(QIODevice::WriteOnly)) {
			qDebug()  << "Could not open " << QFileInfo(file).absoluteFilePath() << "for writing";
			return;
		}

		file.write(data->readAll());
		qDebug() << "saved new version: " << " " << QFileInfo(file).absoluteFilePath();

		file.close();

		DkSettings::Global::setupVersion = setupVersion;
		DkSettings::Global::setupPath = absoluteFilePath;

		DkSettings settings;
		settings.save();

		emit downloadFinished(absoluteFilePath);
	}
	updateAborted = false;
	qDebug() << "downloadFinishedSlot complete";
}

void DkUpdater::performUpdate() {
	if(nomacsSetupUrl.isEmpty())
		qDebug() << "unable to perform update because the nomacsSetupUrl is empty";
	else
		startDownload(nomacsSetupUrl);
}

void DkUpdater::cancelUpdate()  {
	qDebug() << "abort update";
	updateAborted = true; 
	reply->abort(); 
}

}

// that's a bit nasty
#include "DkNoMacs.h"
#include "DkViewPort.h"

namespace nmc {

// DkManagerThread  --------------------------------------------------------------------
DkManagerThread::DkManagerThread(DkNoMacs* parent) {
	this->parent = parent;
}

void DkManagerThread::connectClient() {

	if (!parent)
		return;

	DkViewPort* vp = parent->viewport();

	connect(this, SIGNAL(clientInitializedSignal()), parent, SLOT(clientInitialized()));
	connect(this, SIGNAL(syncWithSignal(quint16)), clientManager, SLOT(synchronizeWith(quint16)));
	connect(this, SIGNAL(stopSyncWithSignal(quint16)), clientManager, SLOT(stopSynchronizeWith(quint16)));

	// TCP communication
	connect(vp, SIGNAL(sendTransformSignal(QTransform, QTransform, QPointF)), clientManager, SLOT(sendTransform(QTransform, QTransform, QPointF)));
	connect(parent, SIGNAL(sendPositionSignal(QRect, bool)), clientManager, SLOT(sendPosition(QRect, bool)));
	connect(parent, SIGNAL(synchronizeWithSignal(quint16)), clientManager, SLOT(synchronizeWith(quint16)));
	connect(parent, SIGNAL(synchronizeWithServerPortSignal(quint16)), clientManager, SLOT(synchronizeWithServerPort(quint16)));

	connect(parent, SIGNAL(sendTitleSignal(QString)), clientManager, SLOT(sendTitle(QString)));
	connect(vp, SIGNAL(sendNewFileSignal(qint16, QString)), clientManager, SLOT(sendNewFile(qint16, QString)));
	connect(clientManager, SIGNAL(receivedNewFile(qint16, QString)), vp, SLOT(tcpLoadFile(qint16, QString)));
	connect(clientManager, SIGNAL(updateConnectionSignal(QList<DkPeer>)), vp, SLOT(tcpShowConnections(QList<DkPeer>)));

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
	DkManagerThread::connectClient();
}

void DkLocalManagerThread::createClient(QString title) {

	if (clientManager)
		delete clientManager;

	clientManager = new DkLocalClientManager(title);
}

// DkLanManagerThread --------------------------------------------------------------------
DkLanManagerThread::DkLanManagerThread(DkNoMacs* parent) : DkManagerThread(parent) {
	
	clientManager = 0;
}

void DkLanManagerThread::connectClient() {

	connect(parent->viewport(), SIGNAL(sendImageSignal(QImage, QString)), clientManager, SLOT(sendNewImage(QImage, QString)));
	connect(clientManager, SIGNAL(receivedImage(QImage)), parent->viewport(), SLOT(setImage(QImage)));
	connect(clientManager, SIGNAL(sendInfoSignal(QString, int)), parent->viewport()->getController(), SLOT(setInfo(QString, int)));
	connect(clientManager, SIGNAL(receivedImageTitle(QString)), parent, SLOT(setWindowTitle(QString)));
	connect(this, SIGNAL(startServerSignal(bool)), clientManager, SLOT(startServer(bool)));

	DkManagerThread::connectClient();
}

void DkLanManagerThread::createClient(QString title) {

	if (clientManager)
		delete clientManager;

	clientManager = new DkLANClientManager(title);
}


}
