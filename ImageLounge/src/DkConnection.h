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

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QTcpSocket>
#include <QRect>
#include <QTransform>
#include <QHostAddress>
#include <QImage>
#pragma warning(pop)		// no warnings from includes - end

#ifdef QT_NO_DEBUG_OUTPUT
#pragma warning(disable: 4127)		// no 'conditional expression is constant' if qDebug() messages are removed
#endif

// Qt defines
class QTimer;

namespace nmc {

static const int MaxBufferSize = 102400000;
static const char SeparatorToken = '<';

class DkConnection : public QTcpSocket {
	Q_OBJECT;

	public:
		DkConnection(QObject* parent=0);
		~DkConnection() {
			//qDebug() << "connection destructed...";
		};

		void release() {
			sendNewGoodbyeMessage();
		};

		quint16 getPeerPort() { return mPortOfPeer;};
		quint16 getPeerId() {return mPeerId;};
		void setPeerId(quint16 peerId) { mPeerId = peerId;};
		void setTitle(QString newTitle);

		bool connectionCreated;

	signals:
		void connectionReadyForUse(quint16 peerServerPort, QString title, DkConnection* connection) const;
		void connectionStartSynchronize(QList<quint16> synchronizedPeersOfOtherClient, DkConnection* connection) const;
		void connectionStopSynchronize(DkConnection* connection) const;
		void connectionTitleHasChanged(DkConnection* connection, QString newTitle) const;
		void connectionNewPosition(DkConnection* connection, QRect position, bool opacity, bool overlaid) const;
		void connectionNewTransform(DkConnection* connection, QTransform transform, QTransform imgTransform, QPointF canvasSize) const;
		void connectionNewFile(DkConnection* connection, qint16 op, QString filename) const;
		void connectionGoodBye(DkConnection* connection) const;
		void connectionShowStatusMessage(DkConnection* connection, QString msg) const;

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
		virtual bool allowedToSynchronize() {return true;};

		ConnectionState mState = WaitingForGreeting; 
		DataType mCurrentDataType = Undefined; 
		QByteArray mBuffer;
		QString mCurrentTitle;
		int mNumBytesForCurrentDataType = 0;
		quint16 mPortOfPeer = 0;  
		quint16 mPeerServerPort = 0;
		bool mIsGreetingMessageSent = false;
		bool mIsSynchronizeMessageSent = false;

	protected slots:
		virtual void processReadyRead();

	private slots:
		void synchronizedTimerTimeout();

	private:

		QTimer* mSynchronizedTimer;
		QList<quint16> mSynchronizedPeersServerPorts;
		quint16 mPeerId;
};

class DkLocalConnection : public DkConnection {
	Q_OBJECT;

	public:
		DkLocalConnection(QObject* parent=0);

		quint16 getLocalTcpServerPort() { return mLocalTcpServerPort;};
		void setLocalTcpServerPort(quint16 localTcpServerPort) { mLocalTcpServerPort = localTcpServerPort;};
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

		quint16 mLocalTcpServerPort;
		LocalDataType mCurrentLocalDataType;
};

class DkLANConnection : public DkConnection {
	Q_OBJECT;

	public:
		DkLANConnection(QObject* parent = 0) ;

		QString getClientName() { return mClientName;};
		void setClientName(QString clientName) { this->mClientName = clientName;} ;

		bool getShowInMenu() {return mShowInMenu;};
		void setShowInMenu(bool flag) {mShowInMenu = flag;};
		void sendGreetingMessage(QString currentTitle);
		bool getIAmServer() {return mIAmServer;};
		void setIAmServer(bool iAmServer) { this->mIAmServer = iAmServer;};

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
		LANDataType mCurrentLanDataType = Undefined;
		bool mAllowTransformation = false;
		bool mAllowPosition = false;
		bool mAllowFile = false;
		bool mAllowImage = false;

	private:

		QString mClientName;
		bool mShowInMenu = false;
		bool mIAmServer = true;
};


class DkRCConnection : public DkLANConnection {
	Q_OBJECT

	public:
		DkRCConnection(QObject* parent = 0);
		
	signals:
		void connectionNewPermission(DkConnection*, bool) const;
		void connectionNewRCType(DkConnection*, int) const ;

	public slots:
		void sendAskForPermission();
		void sendPermission();
		void sendRCType(int type);

	protected slots:
		virtual void processReadyRead();
		

	protected:
		virtual void readGreetingMessage();
		virtual bool readProtocolHeader();
		virtual void processData();
		virtual void readWhileBytesAvailable();
		virtual bool allowedToSynchronize();

		enum RemoteControlDataType {
			newPermission = 11,
			newAskPermission,
			newRcType,
			Undefined
		};

		RemoteControlDataType currentRemoteControlDataType;

	private:

};

};

