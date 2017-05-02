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

#pragma  once

#define local_tcp_port_start 45454
#define local_tcp_port_end 45484
#define lan_udp_port_start 28566
#define lan_udp_port_end 28576
#define rc_udp_port 28565

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QTcpServer>
#include <QUdpSocket>
#include <QNetworkReply>
#include <QThread>
#include <QMutex>
#include <QSharedPointer>
#pragma warning(pop)		// no warnings from includes - end

#include "DkConnection.h"

namespace nmc {

// nomacs defines
class DkLocalTcpServer;
class DkLANTcpServer;
class DkLANUdpSocket;

class DkPeer : public QObject{
	Q_OBJECT;

public:
	//DkPeer(QObject* parent = 0);
	DkPeer(quint16 port, quint16 peerId, const QHostAddress& hostAddress, quint16 peerServerPort, const QString& title, DkConnection* connection, bool sychronized = false, const QString& clientName="", bool showInMenu = false, QObject* parent = NULL);

	//DkPeer(const DkPeer& peer);
	~DkPeer();

	bool operator==(const DkPeer& peer) const;
	//DkPeer& operator=(const DkPeer& peer);

	bool isActive() {return hasChangedRecently;};
	void setSynchronized(bool flag);
	bool isSynchronized() {return sychronized;};
	bool isLocal() {
		return hostAddress == QHostAddress::LocalHost;
	};

	quint16 peerId;
	quint16 localServerPort;
	quint16 peerServerPort;
	QHostAddress hostAddress;
	QString clientName;
	QString title;
	DkConnection* connection;
	QTimer* timer;
	bool showInMenu;

signals:
	void sendGoodByeMessage();
	private slots:
	void timerTimeout() {hasChangedRecently = false;};

private:
	bool hasChangedRecently;
	bool sychronized;
};

class DkPeerList {

public:
	DkPeerList();
	bool addPeer(DkPeer* peer);
	bool removePeer(quint16 peerId);
	bool setSynchronized(quint16 peerId, bool synchronized);
	bool setTitle(quint16 peerId, const QString& title);
	bool setShowInMenu(quint16 peerId, bool showInMenu);
	QList<DkPeer*> getPeerList();
	DkPeer* getPeerById(quint16 id);
	DkPeer* getPeerByAddress(const QHostAddress& address, quint16 port) const;

	QList<DkPeer*> getSynchronizedPeers() const;
	QList<quint16> getSynchronizedPeerServerPorts() const;
	QList<DkPeer*> getActivePeers() const;

	DkPeer* getPeerByServerport(quint16 port) const;
	bool alreadyConnectedTo(const QHostAddress& address, quint16 port) const;
	void print() const;

private:
	QMultiHash<quint16, DkPeer*> peerList;
};

class DkClientManager : public QThread {
	Q_OBJECT;
	public:
		DkClientManager(const QString& title, QObject* parent = 0);
		~DkClientManager();
		virtual	QList<DkPeer*> getPeerList() = 0;

	signals:
		void receivedTransformation(QTransform transform, QTransform imgTransform, QPointF canvasSize);
		void receivedPosition(QRect position, bool opacity, bool overlaid);
		void receivedNewFile(qint16 op, const QString& filename);
		void receivedImage(const QImage& image);
		void receivedImageTitle(const QString& title);
		void sendInfoSignal(const QString& msg, int time = 3000);
		void sendGreetingMessage(const QString& title);
		void sendSynchronizeMessage();
		void sendDisableSynchronizeMessage();
		void sendNewTitleMessage(const QString& newtitle);
		void sendNewPositionMessage(QRect position, bool opacity, bool overlaid);
		void sendNewTransformMessage(QTransform transform, QTransform imgTransform, QPointF canvasSize);
		void sendNewFileMessage(qint16 op, const QString& filename);
		void sendNewImageMessage(QImage image, const QString& title);
		void sendNewUpcomingImageMessage(const QString& imageTitle);
		void sendGoodByeMessage();
		void synchronizedPeersListChanged(QList<quint16> newList);
		void updateConnectionSignal(QList<DkPeer*> peers);
		
		void receivedQuit();

	public slots:
		virtual void synchronizeWith(quint16 peerId) = 0;
		virtual void synchronizeWithServerPort(quint16 port) = 0;
		virtual void stopSynchronizeWith(quint16 peerId) = 0;
		virtual void sendTitle(const QString& newTitle);
		void sendTransform(QTransform transform, QTransform imgTransform, QPointF canvasSize);
		void sendPosition(QRect newRect, bool overlaid);

		void sendNewFile(qint16 op, const QString& filename);
		virtual void sendNewImage(QImage, const QString&) {}; // dummy
		void sendGoodByeToAll();

	protected slots:
		void newConnection( int socketDescriptor );
		virtual void connectionReadyForUse(quint16 peerId, const QString& title, DkConnection* connection);
		virtual void connectionSynchronized(QList<quint16> synchronizedPeersOfOtherClient, DkConnection* connection) = 0;
		virtual void connectionStopSynchronized(DkConnection* connection) = 0;
		virtual void connectionSentNewTitle(DkConnection* connection, const QString& newTitle);
		virtual void connectionReceivedTransformation(DkConnection* connection, const QTransform& transform, const QTransform& imgTransform, const QPointF& canvasSize);
		virtual void connectionReceivedPosition(DkConnection* connection, const QRect& rect, bool opacity, bool overlaid);
		virtual void connectionReceivedNewFile(DkConnection* connection, qint16 op, const QString& filename);
		virtual void connectionReceivedGoodBye(DkConnection* connection);
		void connectionShowStatusMessage(DkConnection* connection, const QString& msg);
		void disconnected();

	protected:
		void removeConnection(DkConnection* connection);
		void connectConnection(DkConnection* connection);
		virtual DkConnection* createConnection() = 0;

		DkPeerList mPeerList;
		QString mCurrentTitle;
		quint16 mNewPeerId;
		QList<DkConnection*> mStartUpConnections;

};

class DkLocalClientManager : public DkClientManager {
	Q_OBJECT;
	public:
		DkLocalClientManager(const QString& title, QObject* parent = 0);
		QList<DkPeer*> getPeerList();
		quint16 getServerPort();
		void run();

	signals:
		void receivedQuit();
		void sendQuitMessage();

	public slots:
		void stopSynchronizeWith(quint16 peerId);
		void synchronizeWithServerPort(quint16 port);
		void synchronizeWith(quint16 peerId);
		void sendArrangeInstances(bool overlaid);
		void sendQuitMessageToPeers();

	private slots:
		void connectionSynchronized(QList<quint16> synchronizedPeersOfOtherClient, DkConnection* connection);
		virtual void connectionStopSynchronized(DkConnection* connection);
		void connectionReceivedQuit(); 

	private:
		DkLocalConnection* createConnection();
		void searchForOtherClients();

		DkLocalTcpServer* server;
};



class DkLANClientManager : public DkClientManager {
	Q_OBJECT;
	public:
		DkLANClientManager(const QString& title, QObject* parent = 0, quint16 updServerPortRangeStart = lan_udp_port_start, quint16 udpServerPortRangeEnd = lan_udp_port_end);
		virtual ~DkLANClientManager(); 
		virtual QList<DkPeer*> getPeerList();

	signals:
		void sendSwitchServerMessage(const QHostAddress& address, quint16 port);
		void serverPortChanged(quint16 port);

	public slots:
		void sendTitle(const QString& newTitle);
		virtual void synchronizeWithServerPort(quint16) {}; // dummy
		void stopSynchronizeWith(quint16 peerId = USHRT_MAX);
		void startServer(bool flag);
		void sendNewImage(QImage image, const QString& title);
		void synchronizeWith(quint16 peerId);

	protected:
		void connectConnection(DkConnection* connection);
		
		DkLANTcpServer* server;

	protected slots:
		virtual void connectionReadyForUse(quint16 peerServerPort, const QString& title, DkConnection* connection);

	private slots:
		void connectionReceivedNewImage(DkConnection* connection, const QImage& image, const QString& title);
		void startConnection(const QHostAddress& address, quint16 port, const QString& clientName);
		void sendStopSynchronizationToAll();
		
		virtual void connectionSynchronized(QList<quint16> synchronizedPeersOfOtherClient, DkConnection* connection);
		virtual void connectionStopSynchronized(DkConnection* connection);
		void connectionSentNewTitle(DkConnection* connection, const QString& newTitle);
		void connectionReceivedTransformation(DkConnection* connection, QTransform transform, QTransform imgTransform, QPointF canvasSize);
		void connectionReceivedPosition(DkConnection* connection, QRect rect, bool opacity, bool overlaid);
		void connectionReceivedNewFile(DkConnection* connection, qint16 op, const QString& filename);
		void connectionReceivedUpcomingImage(DkConnection* connection, const QString& imageTitle);
		void connectionReceivedSwitchServer(DkConnection* connection, const QHostAddress& address, quint16 port);
	private:
		virtual DkLANConnection* createConnection();

};

class DkLocalTcpServer : public QTcpServer {
	Q_OBJECT;
	public:
		DkLocalTcpServer(QObject* parent = 0);

		quint16 startPort;
		quint16 endPort;
		

	signals:
		void serverReiceivedNewConnection(int DkDescriptor);


	protected:
		void incomingConnection(qintptr socketDescriptor) override;
};

class DkLANTcpServer : public QTcpServer {
	Q_OBJECT;
	public:
		DkLANTcpServer(QObject* parent = 0, quint16 updServerPortRangeStart = lan_udp_port_start, quint16 udpServerPortRangeEnd = lan_udp_port_end);

	signals:
		void serverReiceivedNewConnection(const QHostAddress& address , quint16 port , const QString& clientName);
		void serverReiceivedNewConnection(int DkDescriptor);
		void sendStopSynchronizationToAll();
		void sendNewClientBroadcast();

	public slots:
		void startServer(bool flag);

	private slots:
		void udpNewServerFound(const QHostAddress& address , quint16 port , const QString& clientName);
	
	protected:
		void incomingConnection(qintptr socketDescriptor) override;		// f* updating this interface just costs 3hrs
		DkLANUdpSocket* udpSocket;

	private:

};

class DkLANUdpSocket : public QUdpSocket {
	Q_OBJECT;

	public:
		DkLANUdpSocket(quint16 startPort = lan_udp_port_start, quint16 endPort = lan_udp_port_end, QObject* parent = 0);
		void startBroadcast(quint16 tcpServerPort);
		void stopBroadcast();
		
	
	signals:
		void udpSocketNewServerOnline(const QHostAddress& address, quint16 port, const QString& clientName);
	
	public slots:
		void sendBroadcast();
		void sendNewClientBroadcast();

	private slots:
		void readBroadcast();

	private:
		bool isLocalHostAddress(const QHostAddress & address);
		void checkLocalIpAddresses();
		quint16 mStartPort;
		quint16 mEndPort;
		quint16 mServerPort;
		quint16 mTcpServerPort;
		QList<QHostAddress> mLocalIpAddresses;
		QTimer* mBroadcastTimer = 0;
		bool mBroadcasting = false;
};

class DkNoMacs;

class DkManagerThread : public QThread {
	Q_OBJECT

public:
	DkManagerThread(DkNoMacs *parent = 0);
	~DkManagerThread() {
		
		if (clientManager) 
			delete clientManager;
		clientManager = 0;
	};
	
	virtual void connectClient();
	void run();

	QList<DkPeer*> getPeerList() {

		if (!clientManager)
			return QList<DkPeer*>();

		QMutexLocker locker(&mutex);
		return clientManager->getPeerList();	// critical section
	};

signals:
	void clientInitializedSignal();
	void syncWithSignal(quint16);
	void stopSyncWithSignal(quint16);
	void goodByeToAllSignal();

public slots:
	void synchronizeWith(quint16 peerId) {

		emit syncWithSignal(peerId);
	};
	void stopSynchronizeWith(quint16 peerId) {
		emit stopSyncWithSignal(peerId);
	};
	void sendGoodByeToAll() {
		emit goodByeToAllSignal();
	};

	void quit();

protected:
	virtual void createClient(const QString& title) = 0;
	
	DkClientManager* clientManager;
	DkNoMacs *parent;
	QMutex mutex;
};

class DkLocalManagerThread : public DkManagerThread {
	Q_OBJECT

public:
	DkLocalManagerThread(DkNoMacs* parent);
	void connectClient();

	qint16 getServerPort() {
		//TODO: da kann der hund begraben sein...
		QMutexLocker locker(&mutex);
		return dynamic_cast<DkLocalClientManager*>(clientManager)->getServerPort();
	};

protected:
	void createClient(const QString& title);

};

class DkLanManagerThread : public DkManagerThread {
	Q_OBJECT

public:
	DkLanManagerThread(DkNoMacs* parent);
	virtual void connectClient();

signals:
	void startServerSignal(bool start);

public slots:
	virtual void startServer(bool start) {
		// re-send
		emit startServerSignal(start);
	};

protected:
	void createClient(const QString& title);

};

};

