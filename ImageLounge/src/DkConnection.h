/*******************************************************************************************************
 DkConnection.h
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

#pragma once

#include <QtNetwork/QTcpSocket>
#include <QBuffer>
#include <QByteArray>
#include <QImage>
#include <QTransform>
#include <QRect>
#include <QTimer>
#include <QHostAddress>
#include <QHostInfo>
#include <QThread>
#include "DkSettings.h"

namespace nmc {

static const int MaxBufferSize = 102400000;
static const char SeparatorToken = '<';


class DkConnection : public QTcpSocket {
	Q_OBJECT;

	public:
		DkConnection(QObject* parent=0);
		~DkConnection() {
			qDebug() << "connection destructed...";
			//sendNewGoodbyeMessage();
		};

		void release() {
			qDebug() << "connection destructed...";
			sendNewGoodbyeMessage();
		};

		quint16 getPeerPort() { return portOfPeer;};
		quint16 getPeerId() {return peerId;};
		void setPeerId(quint16 peerId) { this->peerId = peerId;};
		void setTitle(QString newTitle);

	signals:
		void connectionReadyForUse(quint16 peerServerPort, QString title, DkConnection* connection);
		void connectionStartSynchronize(QList<quint16> synchronizedPeersOfOtherClient, DkConnection* connection);
		void connectionStopSynchronize(DkConnection* connection);
		void connectionTitleHasChanged(DkConnection* connection, QString newTitle);
		void connectionNewPosition(DkConnection* connection, QRect position, bool opacity, bool overlaid);
		void connectionNewTransform(DkConnection* connection, QTransform transform, QTransform imgTransform, QPointF canvasSize);
		void connectionNewFile(DkConnection* connection, qint16 op, QString filename);
		void connectionGoodBye(DkConnection* connection);
		void connectionShowStatusMessage(DkConnection* connection, QString msg);

	public slots:
		virtual void sendGreetingMessage(QString currenTitle) = 0;
		void sendStartSynchronizeMessage();
		void sendStopSynchronizeMessage();
		void sendNewTitleMessage(QString newtitle);
		virtual void sendNewPositionMessage(QRect position, bool opacity, bool overlaid);
		virtual void sendNewTransformMessage(QTransform transform, QTransform imgTransform, QPointF canvasSize);
		virtual void sendNewFileMessage(qint16 op , QString filename);
		void sendNewGoodbyeMessage();
		void synchronizedPeersListChanged(QList<quint16> newList);


	protected:
		enum ConnectionState {
			WaitingForGreeting,
			ReadyForUse,
			Synchronized
		};
		enum DataType {
			Greeting,
			startSynchronize,
			stopSynchronize,
			newTitle,
			newPosition,
			newTransform,
			newFile,
			GoodBye,
			Undefined
		};

		virtual bool readProtocolHeader();
		virtual void checkState();
		int readDataIntoBuffer(int maxSize = MaxBufferSize);
		bool readDataTypeIntoBuffer();
		virtual void processData();
		virtual void readWhileBytesAvailable();
		virtual void readGreetingMessage() = 0;
		bool hasEnoughData();
		int dataLengthForCurrentDataType();

		ConnectionState state; 
		DataType currentDataType; 
		QByteArray buffer;
		QString currentTitle;
		int numBytesForCurrentDataType;
		quint16 portOfPeer;  
		quint16 peerServerPort;
		bool isGreetingMessageSent;
		bool isSynchronizeMessageSent;

	protected slots:
		virtual void processReadyRead();

	private slots:
		void synchronizedTimerTimeout();

	private:

		QTimer* synchronizedTimer;
		QList<quint16> synchronizedPeersServerPorts;
		quint16 peerId;
};

class DkLocalConnection : public DkConnection {
	Q_OBJECT;

	public:
		DkLocalConnection(QObject* parent=0);

		quint16 getLocalTcpServerPort() { return localTcpServerPort;};
		void setLocalTcpServerPort(quint16 localTcpServerPort) { this->localTcpServerPort = localTcpServerPort;};
		void sendGreetingMessage(QString currentTitle);
		

	signals:
		void connectionQuitReceived();

	protected slots:
		void processReadyRead();
		void processData();
		void sendQuitMessage();

	protected:
		enum LocalDataType {
			Quit,
			Undefined
		};

	private:
		bool readProtocolHeader();
		void readGreetingMessage();
		void readQuitMessage();
		quint16 localTcpServerPort;
		LocalDataType currentLocalDataType;

};

class DkLANConnection : public DkConnection {
	Q_OBJECT;

	public:
		DkLANConnection(QObject* parent = 0) ;

		QString getClientName() { return clientName;};
		void setClientName(QString clientName) { this->clientName = clientName;} ;

		bool getShowInMenu() {return showInMenu;};
		void setShowInMenu(bool flag) {showInMenu = flag;};
		void sendGreetingMessage(QString currentTitle);
		bool getIAmServer() {return iAmServer;};
		void setIAmServer(bool iAmServer) { this->iAmServer = iAmServer;};

	signals:	
		void connectionNewImage(DkConnection* connection, QImage image, QString title);
		void connectionUpcomingImage(DkConnection* connection, QString imageTitle);
		void connectionSwitchServer(DkConnection* connection, QHostAddress address, quint16 port);

	protected slots:
		void processReadyRead();

	public slots:
		void sendNewImageMessage(QImage image, QString title);
		void sendNewUpcomingImageMessage(QString imageTitle);
		void sendNewPositionMessage(QRect position, bool opacity, bool overlaid);
		void sendNewTransformMessage(QTransform transform, QTransform imgTransform, QPointF canvasSize);
		void sendNewFileMessage(qint16 op , QString filename);
		void sendSwitchServerMessage(QHostAddress address, quint16 port);


	protected:
		virtual void readGreetingMessage();
		virtual bool readProtocolHeader();
		virtual void processData();
		virtual void readWhileBytesAvailable();

		enum LANDataType {
			upcomingImage = 9,
			newImage,
			switchServer,
			Undefined
		};
		LANDataType currentLanDataType;
		bool allowTransformation;
		bool allowPosition;
		bool allowFile;
		bool allowImage;

	private:
		

		QString clientName;
		bool showInMenu;


		bool iAmServer;
};


class DkRemoteControlConnection : public DkLANConnection {
	Q_OBJECT

	public:
		DkRemoteControlConnection(QObject* parent = 0);

	signals:
		void connectionNewPermission(DkConnection*, bool);

	public slots:
		void sendAskForPermission();
		void sendPermission();

	protected slots:
		virtual void processReadyRead();
		

	protected:
		virtual void readGreetingMessage();
		virtual bool readProtocolHeader();
		virtual void processData();
		virtual void readWhileBytesAvailable();

		enum RemoteControlDataType {
			newPermission = 11,
			newAskPermission,
			Undefined
		};
		RemoteControlDataType currentRemoteControlDataType;

	private:

};

};

