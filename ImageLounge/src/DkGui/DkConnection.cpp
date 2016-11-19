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

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QBuffer>
#include <QByteArray>
#include <QTimer>
#include <QHostInfo>
#include <QThread>
#include <QDebug>
#pragma warning(pop)		// no warnings from includes - end

namespace nmc {

// DkConnection --------------------------------------------------------------------

DkConnection::DkConnection(QObject* parent) : QTcpSocket(parent) {
	
	mNumBytesForCurrentDataType = -1;
	mIsGreetingMessageSent = false;	
	mIsSynchronizeMessageSent = false;
	connectionCreated = false;
	mSynchronizedTimer = new QTimer(this);

	connect(mSynchronizedTimer, SIGNAL(timeout()), this, SLOT(synchronizedTimerTimeout()));
	connect(this, SIGNAL(readyRead()), this, SLOT(processReadyRead()));

	setReadBufferSize(MaxBufferSize);
}

void DkConnection::setTitle(const QString& newTitle) {
	mCurrentTitle = newTitle;
}

void DkConnection::sendStartSynchronizeMessage() {
	//qDebug() << "sending Synchronize Message to " << this->peerName() << ":" << this->peerPort();
	if (mIsSynchronizeMessageSent == false) // initialize sync message, not the response
		mSynchronizedTimer->start(1000);

	QByteArray ba;
	QDataStream ds(&ba, QIODevice::ReadWrite);
	ds << quint16(mSynchronizedPeersServerPorts.size());
	for (int i = 0; i < mSynchronizedPeersServerPorts.size(); i++) {
		qDebug() << "mSynchronizedPeersServerPorts: " << mSynchronizedPeersServerPorts[i];
		ds << mSynchronizedPeersServerPorts[i];
	}
	//QByteArray data = "SYNCHRONIZE" + SeparatorToken + QByteArray::number(synchronize.size()) + SeparatorToken + synchronize;
	QByteArray data = "STARTSYNCHRONIZE";
	data.append(SeparatorToken).append(QByteArray::number(ba.size())).append(SeparatorToken).append(ba);
	qDebug() << "sending startsynchronize:" << data;
	if (write(data) == data.size())
		mIsSynchronizeMessageSent = true;
}

void DkConnection::sendStopSynchronizeMessage() {
	if (mState == Synchronized) { // only send message if connection is synchronized
		//qDebug() << "sending disable synchronize Message to " << this->peerName() << ":" << this->peerPort();
		QByteArray synchronize = "disable synchronizing";
		//QByteArray data = "DISABLESYNCHRONIZE" + SeparatorToken + QByteArray::number(synchronize.size()) + SeparatorToken + synchronize;
		QByteArray data = "STOPSYNCHRONIZE";
		data.append(SeparatorToken).append(QByteArray::number(synchronize.size())).append(SeparatorToken).append(synchronize);
		if (write(data) == data.size())
			mIsSynchronizeMessageSent = false;
		mState=ReadyForUse;
	}
}

void DkConnection::sendNewTitleMessage(const QString& newtitle) {
	mCurrentTitle = newtitle;
	//qDebug() << "sending new Title (\"" << newtitle << "\") Message to " << this->peerName() << ":" << this->peerPort();

	QByteArray newTitleBA=newtitle.toUtf8();
	//QByteArray data = "NEWTITLE" + SeparatorToken + QByteArray::number(newTitleBA.size()) + SeparatorToken + newTitleBA;
	QByteArray data = "NEWTITLE";
	data.append(SeparatorToken).append(QByteArray::number(newTitleBA.size())).append(SeparatorToken).append(newTitleBA);
	write(data);
}

void DkConnection::sendNewPositionMessage(QRect position, bool opacity, bool overlaid) {
	//qDebug() << "sending new Position to " << this->peerName() << ":" << this->peerPort();
	QByteArray ba;
	QDataStream ds(&ba, QIODevice::ReadWrite);
	ds << position;
	ds << opacity;
	ds << overlaid;

	//QByteArray data = "NEWTITLE" + SeparatorToken + QByteArray::number(ba.size()) + SeparatorToken + ba;
	QByteArray data = "NEWPOSITION";
	data.append(SeparatorToken).append(QByteArray::number(ba.size())).append(SeparatorToken).append(ba);
	write(data);
}

void DkConnection::sendNewTransformMessage(QTransform transform, QTransform imgTransform, QPointF canvasSize) {
	//qDebug() << "sending new Transform Message to " << this->peerName() << ":" << this->peerPort();
	QByteArray ba;
	QDataStream ds(&ba, QIODevice::ReadWrite);
	ds << transform;
	ds << imgTransform;
	ds << canvasSize;

	//QByteArray data = "NEWTRANSFORM" + SeparatorToken + QByteArray::number(ba.size()) + SeparatorToken + ba;
	QByteArray data = "NEWTRANSFORM";
	data.append(SeparatorToken).append(QByteArray::number(ba.size())).append(SeparatorToken).append(ba);
	write(data);
}

void DkConnection::sendNewFileMessage(qint16 op, const QString& filename) {
	//qDebug() << "sending new File Message to " << this->peerName() << ":" << this->peerPort();
	QByteArray ba;
	QDataStream ds(&ba, QIODevice::ReadWrite);
	ds << op;
	ds << filename;
	QByteArray data = "NEWFILE";
	data.append(SeparatorToken).append(QByteArray::number(ba.size())).append(SeparatorToken).append(ba);
	write(data);
};

void DkConnection::sendNewGoodbyeMessage() {
	//qDebug() << "sending good bye to " << peerName() << ":" << this->peerPort();

	QByteArray ba = "GoodBye";  // scherz?
	QByteArray data = "GOODBYE";
	data.append(SeparatorToken).append(QByteArray::number(ba.size())).append(SeparatorToken).append(ba);
	write(data);
	waitForBytesWritten();
}

void DkConnection::synchronizedPeersListChanged(QList<quint16> newList) {
	mSynchronizedPeersServerPorts = newList;
}

bool DkConnection::readProtocolHeader() {
	QByteArray greetingBA = QByteArray("GREETING").append(SeparatorToken);
	QByteArray synchronizeBA = QByteArray("STARTSYNCHRONIZE").append(SeparatorToken);
	QByteArray disableSynchronizeBA = QByteArray("STOPSYNCHRONIZE").append(SeparatorToken);
	QByteArray newtitleBA = QByteArray("NEWTITLE").append(SeparatorToken);
	QByteArray newtransformBA = QByteArray("NEWTRANSFORM").append(SeparatorToken);
	QByteArray newpositionBA = QByteArray("NEWPOSITION").append(SeparatorToken);
	QByteArray newFileBA = QByteArray("NEWFILE").append(SeparatorToken);
	QByteArray goodbyeBA = QByteArray("GOODBYE").append(SeparatorToken);

	if (mBuffer == greetingBA) {
		//qDebug() << "Greeting received from:" << this->peerAddress() << ":" << this->peerPort();
		mCurrentDataType = Greeting;
	} else if (mBuffer == synchronizeBA) {
		//qDebug() << "Synchronize received from:" << this->peerAddress() << ":" << this->peerPort();
		mCurrentDataType = startSynchronize;
	} else if (mBuffer == disableSynchronizeBA) {
		//qDebug() << "StopSynchronize received from:" << this->peerAddress() << ":" << this->peerPort();
		mCurrentDataType = stopSynchronize;
	} else if (mBuffer == newtitleBA) {
		//qDebug() << "New Title received from:" << this->peerAddress() << ":" << this->peerPort();
		mCurrentDataType = newTitle;
	} else if (mBuffer == newtransformBA) {
		//qDebug() << "New Transform received from:" << this->peerAddress() << ":" << this->peerPort();
		mCurrentDataType = newTransform;
	} else if (mBuffer == newpositionBA) {
		//qDebug() << "New Position received from:" << this->peerAddress() << ":" << this->peerPort();
		mCurrentDataType = newPosition;
	} else if (mBuffer == newFileBA) {
		//qDebug() << "New File received from:" << this->peerAddress() << ":" << this->peerPort();
		mCurrentDataType = newFile;
	} else if (mBuffer == goodbyeBA) {
		//qDebug() << "Goodbye received from:" << this->peerAddress() << ":" << this->peerPort();
		mCurrentDataType = GoodBye;
	} else {
		qDebug() << QString(mBuffer);
		qDebug() << "Undefined received from:" << this->peerAddress() << ":" << this->peerPort();
		mCurrentDataType = Undefined;
		//abort();
		//return false;
		return true;
	}

	mBuffer.clear();
	mNumBytesForCurrentDataType = dataLengthForCurrentDataType();
	return true;
}

int DkConnection::readDataIntoBuffer(int maxSize) {
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

bool DkConnection::hasEnoughData() {
	if (mNumBytesForCurrentDataType <= 0) {
		mNumBytesForCurrentDataType = dataLengthForCurrentDataType();
	}
	
	//qDebug() << "numBytesForCurrentDataType:" << numBytesForCurrentDataType;
	//qDebug() << "bytesAvailable:" << bytesAvailable();
	//qDebug() << "buffer size:" << buffer.size();
	
	if (bytesAvailable() < mNumBytesForCurrentDataType || mNumBytesForCurrentDataType <= 0) {
		return false;
	}

	return true;
}

int DkConnection::dataLengthForCurrentDataType() {
	if (bytesAvailable() <= 0 || readDataIntoBuffer() <= 0 || !mBuffer.endsWith(SeparatorToken))
		return 0;

	mBuffer.chop(1);
	int number = mBuffer.toInt();
	mBuffer.clear();
	return number;
}

void DkConnection::processReadyRead() {
	if (readDataIntoBuffer() <= 0)
		return;
	if (!readProtocolHeader())
		return;
	checkState();

	readWhileBytesAvailable();
}

void DkConnection::checkState() {
	
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

	if (mState==ReadyForUse && mCurrentDataType == startSynchronize) {
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

		//qDebug() << "other client is sychronized with: ";
		for (int i=0; i < numberOfSynchronizedPeers; i++) {
			quint16 peerId;
			ds >> peerId;
			synchronizedPeersOfOtherInstance.push_back(peerId);
			//qDebug() << peerId;
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

	if (mState==Synchronized && mCurrentDataType == stopSynchronize) {
		mState=ReadyForUse;
		this->mIsSynchronizeMessageSent=false;
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
		//qDebug() << "received GoodBye from " << peerAddress() << ":" << peerPort();
		emit connectionGoodBye(this);
		mCurrentDataType = Undefined;
		mNumBytesForCurrentDataType = 0;
		mBuffer.clear();
		abort();
		return;
	}
}

void DkConnection::readWhileBytesAvailable() {
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

bool DkConnection::readDataTypeIntoBuffer() {
	mBuffer = read(mNumBytesForCurrentDataType);
	if (mBuffer.size() != mNumBytesForCurrentDataType) {
		abort();
		return false;
	}
	return true;

}

void DkConnection::processData() {
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
		break;}
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
		break;}
	case newFile: {
		if (mState == Synchronized) {
			qint16 op;
			QString filename;

			QDataStream dsTransform(mBuffer);
			dsTransform >> op;
			dsTransform >> filename;
			emit connectionNewFile(this, op, filename);
		}
		break;}
	default:
		break;
	}

	mCurrentDataType = Undefined;
	mNumBytesForCurrentDataType = 0;
	mBuffer.clear();
}

void DkConnection::synchronizedTimerTimeout() {
	mSynchronizedTimer->stop();
	emit connectionStopSynchronize(this);
}

// DkLocalConnection --------------------------------------------------------------------
DkLocalConnection::DkLocalConnection(QObject* parent/* =0 */) : DkConnection(parent) {
}


void DkLocalConnection::processReadyRead() {
	if (mCurrentLocalDataType == Quit) { // long message (copied from lan connection) -> does this work here correctly?
		readWhileBytesAvailable();
		return;
	}

	//if (readDataIntoBuffer() <= 0)
	//	return;
	//if (!readProtocolHeader())
	//	return;

	DkConnection::processReadyRead();
}

void DkLocalConnection::processData() {
	switch (mCurrentLocalDataType) {
	case Quit:
		emit connectionQuitReceived();
		break;
	}
	
	DkConnection::processData();
}

bool DkLocalConnection::readProtocolHeader() {
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


void DkLocalConnection::sendGreetingMessage(const QString& currentTitle) {
	
	mCurrentTitle = currentTitle;
	QByteArray ba;
	QDataStream ds(&ba, QIODevice::ReadWrite);
	ds << mLocalTcpServerPort;
	ds << mCurrentTitle;

	//qDebug() << "title: " << mCurrentTitle;
	//qDebug() << "local tcp: " << mLocalTcpServerPort;
	//qDebug() << "peer id: " << mPeerId;

	QByteArray data = "GREETING";
	data.append(SeparatorToken);
	data.append(QByteArray::number(ba.size()));
	data.append(SeparatorToken);
	data.append(ba);

	//qDebug() << "greeting message: " << data;

	if (write(data) == data.size()) {
		mIsGreetingMessageSent = true;
	}

}

void DkLocalConnection::readGreetingMessage() {
	QString title;
	QDataStream ds(mBuffer);
	ds >> this->mPeerServerPort;
	ds >> title;

	//qDebug() << "emitting readyForUse";
	emit connectionReadyForUse(mPeerServerPort, title, this);
}

void DkLocalConnection::sendQuitMessage() {
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



// DkLANConnection --------------------------------------------------------------------
DkLANConnection::DkLANConnection(QObject* parent /* = 0 */) : DkConnection(parent) {
}

void DkLANConnection::sendNewUpcomingImageMessage(const QString& imageTitle) {
	if (!mAllowImage)
		return;

	QString title = imageTitle;
	if (title == "")
		title = "nomacs - ImageLounge";

	QByteArray ba;
	QDataStream ds(&ba, QIODevice::ReadWrite);
	ds << title;

	QByteArray data = "UPCOMINGIMAGE";
	data.append(SeparatorToken).append(QByteArray::number(ba.size())).append(SeparatorToken).append(ba);
	write(data);
};


void DkLANConnection::sendNewImageMessage(const QImage& image, const QString& imageTitle) {
	if (!mAllowImage)
		return;

	QString title = imageTitle;

	if (title == "")
		title = "nomacs - ImageLounge";

	QByteArray ba;
	QDataStream ds(&ba, QIODevice::ReadWrite);
	ds << title;

	QByteArray imageBA;
	QBuffer buffer(&imageBA);
	buffer.open(QIODevice::WriteOnly);
	
	if (image.hasAlphaChannel())
		image.save(&buffer, "TIF");
	else
		image.save(&buffer, "JPG", 100);	// fastest way
	buffer.close();

	ds << imageBA;


	//qDebug() << "imag size: " << (float)buffer.size()/1000000.0f;
	//qDebug() << "my stacksize is: " << (float)this->thread()->stackSize()/1000000.0f;

	try {
		QByteArray data = "NEWIMAGE";
		data.append(SeparatorToken).append(QByteArray::number(ba.size())).append(SeparatorToken).append(ba);
		write(data);
	} 
	catch(...) {
		QString imageSize;
		imageSize.setNum(buffer.size() / 1000000);
		QString msg = "sorry, I could not send the image\n " + imageSize + " MB is too much for me...";
		qDebug() << msg;
		emit connectionShowStatusMessage(this, msg);
	}
};

void DkLANConnection::sendSwitchServerMessage(const QHostAddress& address, quint16 port) {
	//qDebug() << "sending switch server message";
	QByteArray ba;
	QDataStream ds(&ba, QIODevice::ReadWrite);
	ds << address;
	ds << port;	

	QByteArray data = "SWITCHSERVER";
	data.append(SeparatorToken).append(QByteArray::number(ba.size())).append(SeparatorToken).append(ba);
	write(data);
}

void DkLANConnection::sendGreetingMessage(const QString& currentTitle) {
	
	mCurrentTitle = currentTitle;
	//qDebug() << "DKLANConnection::sendGreetingMessage to " << this->peerName() << ":" << this->peerPort() << " with title: " << currentTitle;
	QByteArray ba;
	QDataStream ds(&ba, QIODevice::ReadWrite);
	ds << QHostInfo::localHostName();
	ds << DkSettingsManager::param().sync().allowFile;
	ds << DkSettingsManager::param().sync().allowImage;
	ds << DkSettingsManager::param().sync().allowPosition;
	ds << DkSettingsManager::param().sync().allowTransformation;

	if (mIAmServer) 
		ds << currentTitle;
	else
		ds << " ";

	//QByteArray data = "GREETING" + SeparatorToken + QByteArray::number(ba.size()) + SeparatorToken + ba;
	QByteArray data = "GREETING";
	data.append(SeparatorToken);
	data.append(QByteArray::number(ba.size()));
	data.append(SeparatorToken);
	data.append(ba);
	if (write(data) == data.size())
		mIsGreetingMessageSent = true;
}

void DkLANConnection::readGreetingMessage() {
	QString title;

	if (!mIAmServer) { // server controls which actions are allowed 
		
		QDataStream ds(mBuffer);
		ds >> mClientName;
		ds >> mAllowFile;
		ds >> mAllowImage;
		ds >> mAllowPosition;
		ds >> mAllowTransformation;
		ds >> title;		
	} else {
		QDataStream ds(mBuffer); // only read clientname
		ds >> mClientName;

		mAllowFile = DkSettingsManager::param().sync().allowFile;
		mAllowImage = DkSettingsManager::param().sync().allowImage;
		mAllowPosition = DkSettingsManager::param().sync().allowPosition;
		mAllowTransformation = DkSettingsManager::param().sync().allowTransformation;
		title = "";
	}

	//qDebug() << "emitting readyForUse";
	emit connectionReadyForUse(mPeerServerPort, title, this);
}

bool DkLANConnection::readProtocolHeader() {
	//qDebug() << "DkLANConnection::readProtocolHeader";
	QByteArray newImageBA = QByteArray("NEWIMAGE").append(SeparatorToken);
	QByteArray upcomingImageBA = QByteArray("UPCOMINGIMAGE").append(SeparatorToken);
	QByteArray switchServerBA = QByteArray("SWITCHSERVER").append(SeparatorToken);

	if (mBuffer == newImageBA) {
		//qDebug() << "New Image received from:" << this->peerAddress() << ":" << this->peerPort();
		mCurrentLanDataType = newImage;
	} else if (mBuffer == upcomingImageBA) {
		//qDebug() << "Upcoming Image received from:" << this->peerAddress() << ":" << this->peerPort();
		mCurrentLanDataType = upcomingImage;
	} else if (mBuffer == switchServerBA) {
		//qDebug() << "Switch Server received from:" << this->peerAddress() << ":" << this->peerPort();
		mCurrentLanDataType = switchServer;
	} else {
		return DkConnection::readProtocolHeader();
	}

	mBuffer.clear();
	mNumBytesForCurrentDataType = dataLengthForCurrentDataType();
	return true;
}

void DkLANConnection::processReadyRead() {

	if (mCurrentLanDataType == newImage) { // long message
		readWhileBytesAvailable();
		return;
	}

	//qDebug() << __FUNCTION__ << " " << __LINE__ << " ###################################################### buffer.size:" << buffer.size();
	//if (buffer.size() == 0) {
	//	if (readDataIntoBuffer() <= 0)
	//		return;
	//}
	//if (!readProtocolHeader())
	//	return;

	//checkState();
	DkConnection::processReadyRead();
}

void DkLANConnection::readWhileBytesAvailable() {
	//qDebug() << "DKLANConnection:" << __FUNCTION__ << " line:" << __LINE__;
	do {
		if (mCurrentDataType == DkConnection::Undefined && mCurrentLanDataType == Undefined) {
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


void  DkLANConnection::processData() {
	switch (mCurrentLanDataType) {
	case newImage: 
			if (mState == Synchronized) {
				
				QString title;
				QByteArray imageBA;
				QDataStream ds(mBuffer);
				ds >> title;
				ds >> imageBA;
				QImage image;
				image.loadFromData(imageBA);
				emit connectionNewImage(this, image, title);
				//qDebug() << "emitted receivedNewImage";
			}
			break;

	case upcomingImage:
			if (mState == Synchronized) {
				//QString imageTitle = QString::fromUtf8(buffer);
				QString imageTitle;
				QDataStream dsUpcomingImage(mBuffer);
				dsUpcomingImage >> imageTitle;
				emit connectionUpcomingImage(this, imageTitle);
			}
			break;
	case switchServer:
		  if (mState == Synchronized) {
			  QHostAddress address;
			  quint16 port;
			  QDataStream ds(mBuffer);
			  ds >> address;
			  ds >> port;
			  emit connectionSwitchServer(this, address, port);
			  //qDebug() << "switch server received: " << address << ":" << port;
		  }
		  break;
	case Undefined:
	default: 
		DkConnection::processData();
	}
		
	mCurrentLanDataType = Undefined;
	mCurrentDataType = DkConnection::Undefined;
	mNumBytesForCurrentDataType = 0;
	mBuffer.clear();
}

void DkLANConnection::sendNewPositionMessage(const QRect& position, bool opacity, bool overlaid) {
	if(!mAllowPosition)
		return;

	DkConnection::sendNewPositionMessage(position, opacity, overlaid);
}

void DkLANConnection::sendNewTransformMessage(const QTransform& transform, const QTransform& imgTransform, const QPointF& canvasSize) {
	if (!mAllowTransformation)
		return;

	DkConnection::sendNewTransformMessage(transform, imgTransform, canvasSize);
}

void DkLANConnection::sendNewFileMessage(qint16 op, const QString& filename) {
	if (!mAllowFile)
		return;

	DkConnection::sendNewFileMessage(op, filename);
}

// DkRemoteControlConnection --------------------------------------------------------------------
DkRCConnection::DkRCConnection(QObject* parent /* = 0 */) : DkLANConnection(parent) {
	currentRemoteControlDataType = Undefined;
}

void DkRCConnection::readGreetingMessage() {
	DkLANConnection::readGreetingMessage();
	mAllowFile = true;
	mAllowImage = true;
	mAllowPosition = true;
	mAllowTransformation = true;
	//sendAskForPermission(); // if here to many messages are sent ... wait until readyforuse in network.cpp
}

bool DkRCConnection::readProtocolHeader() {
	//qDebug() << __FUNCTION__ << " " << __LINE__;
	QByteArray newPermissionBA = QByteArray("PERMISSION").append(SeparatorToken);
	QByteArray newAskPermissionBA = QByteArray("ASKPERMISSION").append(SeparatorToken);
	QByteArray newRCType = QByteArray("RCTYPE").append(SeparatorToken);

	if (mBuffer == newPermissionBA) {
		//qDebug() << "New Permission received from:" << this->peerAddress() << ":" << this->peerPort();
		currentRemoteControlDataType = newPermission;
	} else if (mBuffer == newAskPermissionBA) {
		//qDebug() << "New Ask Permission received from:" << this->peerAddress() << ":" << this->peerPort();
		currentRemoteControlDataType = newAskPermission;
	} else if (mBuffer == newRCType) {
		//qDebug() << "New RCType received from:" << this->peerAddress() << ":" << this->peerPort();
		currentRemoteControlDataType = newRcType;
	} else {
		return DkLANConnection::readProtocolHeader();
	}

	mBuffer.clear();
	mNumBytesForCurrentDataType = dataLengthForCurrentDataType();
	return true;
}

void DkRCConnection::processReadyRead() {
	//if (currentRemoteControlDataType != Undefined) { // long message
	//	readWhileBytesAvailable();
	//	return;
	//}

	//qDebug() << __FUNCTION__ << " " << __LINE__ << " ###################################################### buffer.size:" << buffer.size();
	//if (readDataIntoBuffer() <= 0)
	//	return;
	//if (!readProtocolHeader())
	//	return;

	//checkState();
	DkLANConnection::processReadyRead();
}

void DkRCConnection::readWhileBytesAvailable() {
	//qDebug() << __FUNCTION__ << " " << __LINE__;
	do {
		if (mCurrentDataType == DkConnection::Undefined && mCurrentLanDataType == DkLANConnection::Undefined && currentRemoteControlDataType == DkRCConnection::Undefined) {
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

void DkRCConnection::processData() {
	switch (currentRemoteControlDataType) {
	case newPermission: {
			bool allowedToConnect;
			QString dummy;
			QDataStream ds(mBuffer);
			ds >> allowedToConnect;
			ds >> dummy;
			emit connectionNewPermission(this, allowedToConnect);
			//qDebug() << "emitted connectionNewPermission: allowedToConnect:" << allowedToConnect;
			}
		break;
	case newAskPermission:  {
		QString dummy;
		QDataStream ds(mBuffer);
		ds >> dummy;
		//qDebug() << "askPermission processed ... sending Permission";
		sendPermission();
		}
		break;
	case newRcType: {
		int type;
		QDataStream ds(mBuffer);
		ds >> type;
		emit connectionNewRCType(this, type);
		}
	case Undefined:
	default: 
		DkLANConnection::processData();
	}

	currentRemoteControlDataType = DkRCConnection::Undefined;
	mCurrentLanDataType = DkLANConnection::Undefined;
	mCurrentDataType = DkConnection::Undefined;
	mNumBytesForCurrentDataType = 0;
	mBuffer.clear();
}

void DkRCConnection::sendAskForPermission() {
	//qDebug() << "sending askForPermission to " << this->peerName() << ":" << this->peerPort();

	QByteArray ba;
	QDataStream ds(&ba, QIODevice::ReadWrite);
	ds << "dummyMessage";

	QByteArray data = "ASKPERMISSION";
	data.append(SeparatorToken).append(QByteArray::number(ba.size())).append(SeparatorToken).append(ba);
	write(data);
	this->waitForBytesWritten();
}

void DkRCConnection::sendPermission() {
	qDebug() << "printing whitelist\n--------------------";
	for (int i = 0; i < DkSettingsManager::param().sync().syncWhiteList.size(); i++)
		qDebug() << DkSettingsManager::param().sync().syncWhiteList.at(i);
	qDebug() << "--------------------";
	qDebug() << "current client Name:" << getClientName();
	qDebug() << "sending Permission to " << this->peerName() << ":" << this->peerPort() << "      value:" << DkSettingsManager::param().sync().syncWhiteList.contains(getClientName());
	

	QByteArray ba;
	QDataStream ds(&ba, QIODevice::ReadWrite);
	ds << (DkSettingsManager::param().sync().syncWhiteList.contains(getClientName()) != 0);
	ds << "dummyText";
	QByteArray data = "PERMISSION";
	data.append(SeparatorToken).append(QByteArray::number(ba.size())).append(SeparatorToken).append(ba);
	write(data);
	this->waitForBytesWritten();
}

void DkRCConnection::sendRCType(int type) {
	QByteArray ba;
	QDataStream ds(&ba, QIODevice::ReadWrite);
	ds << type;
	QByteArray data = "RCTYPE";
	data.append(SeparatorToken).append(QByteArray::number(ba.size())).append(SeparatorToken).append(ba);
	write(data);
	this->waitForBytesWritten();
}

bool DkRCConnection::allowedToSynchronize() {
	if (!DkSettingsManager::param().sync().syncWhiteList.contains(getClientName())) {
		qDebug() << "Peer " << getClientName() << " is not allowed to synchronize (not in whitelist)";
		qDebug() << "printing whitelist:";
		for(int i=0; i<DkSettingsManager::param().sync().syncWhiteList.size();i++ )
			qDebug() << DkSettingsManager::param().sync().syncWhiteList.at(i);

		//disconnect immediately
		sendStopSynchronizeMessage();
		return false;
	} else
		return true;
}

}
