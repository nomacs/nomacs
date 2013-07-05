/*******************************************************************************************************
 DkConnection.cpp
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

#include "DkConnection.h"

namespace nmc {

// DkConnection --------------------------------------------------------------------

DkConnection::DkConnection(QObject* parent) : QTcpSocket(parent) {
	state = WaitingForGreeting;
	currentDataType = Undefined;
	numBytesForCurrentDataType = -1;
	isGreetingMessageSent = false;	
	isSynchronizeMessageSent = false;
	this->synchronizedTimer = new QTimer(this);
	connect(synchronizedTimer, SIGNAL(timeout()), this, SLOT(synchronizedTimerTimeout()));

	QObject::connect(this, SIGNAL(readyRead()), this, SLOT(processReadyRead()));
	this->setReadBufferSize(MaxBufferSize);
}

void DkConnection::setTitle(QString newTitle) {
	this->currentTitle = newTitle;
}

void DkConnection::sendStartSynchronizeMessage() {
	//qDebug() << "sending Synchronize Message to " << this->peerName() << ":" << this->peerPort();
	if (isSynchronizeMessageSent == false) // initialize sync message, not the response
		synchronizedTimer->start(1000);

	QByteArray ba;
	QDataStream ds(&ba, QIODevice::ReadWrite);
	ds << quint16(synchronizedPeersServerPorts.size());
	for (int i=0; i < synchronizedPeersServerPorts.size();i++)
		ds << synchronizedPeersServerPorts[i];
	//QByteArray data = "SYNCHRONIZE" + SeparatorToken + QByteArray::number(synchronize.size()) + SeparatorToken + synchronize;
	QByteArray data = "STARTSYNCHRONIZE";
	data.append(SeparatorToken).append(QByteArray::number(ba.size())).append(SeparatorToken).append(ba);
	if (write(data) == data.size())
		isSynchronizeMessageSent = true;
}

void DkConnection::sendStopSynchronizeMessage() {
	if (state == Synchronized) { // only send message if connection is synchronized
		//qDebug() << "sending disable synchronize Message to " << this->peerName() << ":" << this->peerPort();
		QByteArray synchronize = "disable synchronizing";
		//QByteArray data = "DISABLESYNCHRONIZE" + SeparatorToken + QByteArray::number(synchronize.size()) + SeparatorToken + synchronize;
		QByteArray data = "STOPSYNCHRONIZE";
		data.append(SeparatorToken).append(QByteArray::number(synchronize.size())).append(SeparatorToken).append(synchronize);
		if (write(data) == data.size())
			isSynchronizeMessageSent = false;
		state=ReadyForUse;
	}
}

void DkConnection::sendNewTitleMessage(QString newtitle) {
	this->currentTitle = newtitle;
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

void DkConnection::sendNewFileMessage(qint16 op , QString filename) {
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
	qDebug() << "sending good bye to " << this->peerName() << ":" << this->peerPort();

	QByteArray ba = "GoodBye"; 
	QByteArray data = "GOODBYE";
	data.append(SeparatorToken).append(QByteArray::number(ba.size())).append(SeparatorToken).append(ba);
	write(data);
	this->waitForBytesWritten();
}

void DkConnection::synchronizedPeersListChanged(QList<quint16> newList) {
	this->synchronizedPeersServerPorts = newList;
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

	if (buffer == greetingBA) {
		//qDebug() << "Greeting received from:" << this->peerAddress() << ":" << this->peerPort();
		currentDataType = Greeting;
	} else if (buffer == synchronizeBA) {
		//qDebug() << "Synchronize received from:" << this->peerAddress() << ":" << this->peerPort();
		currentDataType = startSynchronize;
	} else if (buffer == disableSynchronizeBA) {
		//qDebug() << "Disable synchronize received from:" << this->peerAddress() << ":" << this->peerPort();
		currentDataType = stopSynchronize;
	} else if (buffer == newtitleBA) {
		//qDebug() << "New Title received from:" << this->peerAddress() << ":" << this->peerPort();
		currentDataType = newTitle;
	} else if (buffer == newtransformBA) {
		//qDebug() << "New Transform received from:" << this->peerAddress() << ":" << this->peerPort();
		currentDataType = newTransform;
	} else if (buffer == newpositionBA) {
		//qDebug() << "New Position received from:" << this->peerAddress() << ":" << this->peerPort();
		currentDataType = newPosition;
	} else if (buffer == newFileBA) {
		//qDebug() << "New File received from:" << this->peerAddress() << ":" << this->peerPort();
		currentDataType = newFile;
	} else if (buffer == goodbyeBA) {
		//qDebug() << "Goodbye received from:" << this->peerAddress() << ":" << this->peerPort();
		currentDataType = GoodBye;
	} else {
		//qDebug() << "Undefined received from:" << this->peerAddress() << ":" << this->peerPort();
		currentDataType = Undefined;
		//abort();
		//return false;
		return true;
	}

	buffer.clear();
	numBytesForCurrentDataType = dataLengthForCurrentDataType();
	return true;
}

int DkConnection::readDataIntoBuffer(int maxSize) {
	if (maxSize > MaxBufferSize)
		return 0;

	int numBytesBeforeRead = buffer.size();
	if (numBytesBeforeRead == MaxBufferSize) {
		qDebug() << "DkConnection::readDataIntoBuffer: Connection aborted";
		abort();
		return 0;
	}

	while (bytesAvailable() > 0 && buffer.size() < maxSize) {
		buffer.append(read(1));
		if (buffer.endsWith(SeparatorToken))
			break;
	}
	return buffer.size() - numBytesBeforeRead;
}

bool DkConnection::hasEnoughData() {
	if (numBytesForCurrentDataType <= 0) {
		numBytesForCurrentDataType = dataLengthForCurrentDataType();
	}
	
	//qDebug() << "numBytesForCurrentDataType:" << numBytesForCurrentDataType;
	//qDebug() << "bytesAvailable:" << bytesAvailable();
	//qDebug() << "buffer size:" << buffer.size();
	
	if (bytesAvailable() < numBytesForCurrentDataType || numBytesForCurrentDataType <= 0) {
		return false;
	}

	return true;
}

int DkConnection::dataLengthForCurrentDataType() {
	if (bytesAvailable() <= 0 || readDataIntoBuffer() <= 0 || !buffer.endsWith(SeparatorToken))
		return 0;

	buffer.chop(1);
	int number = buffer.toInt();
	buffer.clear();
	return number;
}

void DkConnection::processReadyRead() {
	//qDebug() << __FUNCTION__ << " " << __LINE__ << " ###################################################### buffer.size:" << buffer.size();
	if (readDataIntoBuffer() <= 0)
		return;
	if (!readProtocolHeader())
		return;
	checkState();

	readWhileBytesAvailable();
}

void DkConnection::checkState() {
	if (state == WaitingForGreeting) {
		if (currentDataType != Greeting) {
			abort();
			return;
		}

		if (!hasEnoughData())
			return;

		buffer = read(numBytesForCurrentDataType);
		if (buffer.size() != numBytesForCurrentDataType) {
			abort();
			return;
		}

		if (!isValid()) {
			abort();
			return;
		}

		if (!isGreetingMessageSent)
			sendGreetingMessage(currentTitle);

		state = ReadyForUse;
		portOfPeer = peerPort(); // save peer port ... otherwise connections where this instance is server can not be removed from peerList

		readGreetingMessage();

		buffer.clear();
		numBytesForCurrentDataType = 0;
		currentDataType = Undefined;
		return;
	}

	if (state==ReadyForUse && currentDataType == startSynchronize) {
		if (!hasEnoughData())
			return;

		buffer = read(numBytesForCurrentDataType);
		if (buffer.size() != numBytesForCurrentDataType) {
			abort();
			return;
		}

		QDataStream ds(buffer);
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
		currentDataType = Undefined;
		numBytesForCurrentDataType = 0;
		buffer.clear();

		if (!isValid()) {
			abort();
			return;
		}

		state = Synchronized;
		if (!isSynchronizeMessageSent)
			sendStartSynchronizeMessage();

		synchronizedTimer->stop();
		//qDebug() << "emitting Synchronized";
		emit connectionStartSynchronize(synchronizedPeersOfOtherInstance, this);
		return;
	}

	if (state==Synchronized && currentDataType == stopSynchronize) {
		state=ReadyForUse;
		this->isSynchronizeMessageSent=false;
		emit connectionStopSynchronize(this);
		buffer = read(numBytesForCurrentDataType);
		if (buffer.size() != numBytesForCurrentDataType) {
			abort();
			return;
		}

		currentDataType = Undefined;
		numBytesForCurrentDataType = 0;
		buffer.clear();

		return;
	}

	if (currentDataType == GoodBye) {
		qDebug() << "received GoodBye";
		emit connectionGoodBye(this);
		currentDataType = Undefined;
		numBytesForCurrentDataType = 0;
		buffer.clear();
		abort();
		return;
	}

}

void DkConnection::readWhileBytesAvailable() {
	do {
		if (currentDataType == Undefined) {
			readDataIntoBuffer();
			if (!readProtocolHeader())
				return;
			checkState();
		}
		if (!hasEnoughData()) {
			return;
		}

		buffer = read(numBytesForCurrentDataType);
		if (buffer.size() != numBytesForCurrentDataType) {
			abort();
			return;
		}
		processData();

	} while (bytesAvailable() > 0);
}

bool DkConnection::readDataTypeIntoBuffer() {
	buffer = read(numBytesForCurrentDataType);
	if (buffer.size() != numBytesForCurrentDataType) {
		abort();
		return false;
	}
	return true;

}

void DkConnection::processData() {
	switch (currentDataType) {
	case newTitle:
		emit connectionTitleHasChanged(this, QString::fromUtf8(buffer));
		break;
	case newPosition: {
		if (state == Synchronized) {
			QRect rect;
			bool opacity;
			bool overlaid;
			QDataStream ds(buffer);
			ds >> rect;
			ds >> opacity;
			ds >> overlaid;
			emit connectionNewPosition(this, rect, opacity, overlaid);
		}
		break;}
	case newTransform: {
		if (state == Synchronized) {
			QTransform transform;
			QTransform imgTransform;
			QPointF canvasSize;
			QDataStream dsTransform(buffer);
			dsTransform >> transform;
			dsTransform >> imgTransform;
			dsTransform >> canvasSize;
			emit connectionNewTransform(this, transform, imgTransform, canvasSize);
		}
		break;}
	case newFile: {
		if (state == Synchronized) {
			qint16 op;
			QString filename;

			QDataStream dsTransform(buffer);
			dsTransform >> op;
			dsTransform >> filename;
			emit connectionNewFile(this, op, filename);
		}
		break;}
	default:
		break;
	}

	currentDataType = Undefined;
	numBytesForCurrentDataType = 0;
	buffer.clear();
}

void DkConnection::synchronizedTimerTimeout() {
	synchronizedTimer->stop();
	emit connectionStopSynchronize(this);
}

// DkLocalConnection --------------------------------------------------------------------
DkLocalConnection::DkLocalConnection(QObject* parent/* =0 */) {
	this->currentLocalDataType = Undefined;
}


void DkLocalConnection::processReadyRead() {
	if (currentLocalDataType == Quit) { // long message (copied from lan connection) -> does this work here correctly?
		readWhileBytesAvailable();
		return;
	}

	if (readDataIntoBuffer() <= 0)
		return;
	if (!readProtocolHeader())
		return;

	DkConnection::processReadyRead();
}

void DkLocalConnection::processData() {
	switch (currentLocalDataType) {
	case Quit:
		emit connectionQuitReceived();
		break;
	}
	
	DkConnection::processData();
}

bool DkLocalConnection::readProtocolHeader() {
	QByteArray quitBA = QByteArray("QUIT").append(SeparatorToken);

	if (buffer == quitBA) {
		currentLocalDataType = Quit;
	} else {
		return DkConnection::readProtocolHeader();
	}

	buffer.clear();
	numBytesForCurrentDataType = dataLengthForCurrentDataType();
	return true;
}


void DkLocalConnection::sendGreetingMessage(QString currentTitle) {
	this->currentTitle = currentTitle;
	//qDebug() << "sending Greeting Message to " << this->peerName() << ":" << this->peerPort() << " with title: " << currentTitle;
	QByteArray ba;
	QDataStream ds(&ba, QIODevice::ReadWrite);
	ds << localTcpServerPort;
	ds << currentTitle;

	//QByteArray data = "GREETING" + SeparatorToken + QByteArray::number(ba.size()) + SeparatorToken + ba;
	QByteArray data = "GREETING";
	data.append(SeparatorToken);
	data.append(QByteArray::number(ba.size()));
	data.append(SeparatorToken);
	data.append(ba);

	if (write(data) == data.size()) {
		isGreetingMessageSent = true;
	}

}

void DkLocalConnection::readGreetingMessage() {
	QString title;
	QDataStream ds(buffer);
	ds >> this->peerServerPort;
	ds >> title;

	//qDebug() << "emitting readyForUse";
	emit connectionReadyForUse(peerServerPort, title, this);
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
		isGreetingMessageSent = true;
	}
}



// DkLANConnection --------------------------------------------------------------------
DkLANConnection::DkLANConnection(QObject* parent /* = 0 */) : DkConnection(parent) {
	iAmServer = true;
	showInMenu = false;
	currentLanDataType = Undefined;
}

void DkLANConnection::sendNewUpcomingImageMessage(QString image) {
	if (!allowImage)
		return;

	if (image == "")
		image = "nomacs - ImageLounge";

	QByteArray ba;
	QDataStream ds(&ba, QIODevice::ReadWrite);
	ds << image;

	QByteArray data = "UPCOMINGIMAGE";
	data.append(SeparatorToken).append(QByteArray::number(ba.size())).append(SeparatorToken).append(ba);
	write(data);
};


void DkLANConnection::sendNewImageMessage(QImage image, QString title) {
	if (!allowImage)
		return;

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

void DkLANConnection::sendSwitchServerMessage(QHostAddress address, quint16 port) {
	//qDebug() << "sending switch server message";
	QByteArray ba;
	QDataStream ds(&ba, QIODevice::ReadWrite);
	ds << address;
	ds << port;	

	QByteArray data = "SWITCHSERVER";
	data.append(SeparatorToken).append(QByteArray::number(ba.size())).append(SeparatorToken).append(ba);
	write(data);
}

void DkLANConnection::sendGreetingMessage(QString currentTitle) {
	this->currentTitle = currentTitle;
	//qDebug() << "DKLANConnection::sendGreetingMessage to " << this->peerName() << ":" << this->peerPort() << " with title: " << currentTitle;
	QByteArray ba;
	QDataStream ds(&ba, QIODevice::ReadWrite);
	ds << QHostInfo::localHostName();
	ds << DkSettings::Sync::allowFile;
	ds << DkSettings::Sync::allowImage;
	ds << DkSettings::Sync::allowPosition;
	ds << DkSettings::Sync::allowTransformation;

	if (iAmServer) 
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
		isGreetingMessageSent = true;
}

void DkLANConnection::readGreetingMessage() {
	QString title;

	if (!iAmServer) { // server controls which actions are allowed 
		
		QDataStream ds(buffer);
		ds >> clientName;
		ds >> allowFile;
		ds >> allowImage;
		ds >> allowPosition;
		ds >> allowTransformation;
		ds >> title;		
	} else {
		QDataStream ds(buffer); // only read clientname
		ds >> clientName;

		allowFile = DkSettings::Sync::allowFile;
		allowImage = DkSettings::Sync::allowImage;
		allowPosition = DkSettings::Sync::allowPosition;
		allowTransformation = DkSettings::Sync::allowTransformation;
		title = "";
	}

	//qDebug() << "emitting readyForUse";
	emit connectionReadyForUse(peerServerPort, title, this);
}

bool DkLANConnection::readProtocolHeader() {
	//qDebug() << "DkLANConnection::readProtocolHeader";
	QByteArray newImageBA = QByteArray("NEWIMAGE").append(SeparatorToken);
	QByteArray upcomingImageBA = QByteArray("UPCOMINGIMAGE").append(SeparatorToken);
	QByteArray switchServerBA = QByteArray("SWITCHSERVER").append(SeparatorToken);

	if (buffer == newImageBA) {
		//qDebug() << "New Image received from:" << this->peerAddress() << ":" << this->peerPort();
		currentLanDataType = newImage;
	} else if (buffer == upcomingImageBA) {
		//qDebug() << "Upcoming Image received from:" << this->peerAddress() << ":" << this->peerPort();
		currentLanDataType = upcomingImage;
	} else if (buffer == switchServerBA) {
		//qDebug() << "Switch Server received from:" << this->peerAddress() << ":" << this->peerPort();
		currentLanDataType = switchServer;
	} else {
		return DkConnection::readProtocolHeader();
	}

	buffer.clear();
	numBytesForCurrentDataType = dataLengthForCurrentDataType();
	return true;
}

void DkLANConnection::processReadyRead() {

	if (currentLanDataType == newImage) { // long message
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
		if (currentDataType == DkConnection::Undefined && currentLanDataType == Undefined) {
			readDataIntoBuffer();
			if (!readProtocolHeader())
				return;
			checkState();
		}
		if (!hasEnoughData()) {
			return;
		}

		buffer = read(numBytesForCurrentDataType);
		if (buffer.size() != numBytesForCurrentDataType) {
			abort();
			return;
		}
		processData();
	} while (bytesAvailable() > 0);
}


void  DkLANConnection::processData() {
	switch (currentLanDataType) {
	case newImage: 
			if (state == Synchronized) {
				
				QString title;
				QByteArray imageBA;
				QDataStream ds(buffer);
				ds >> title;
				ds >> imageBA;
				QImage image;
				image.loadFromData(imageBA);
				emit connectionNewImage(this, image, title);
				//qDebug() << "emitted receivedNewImage";
			}
			break;

	case upcomingImage:
			if (state == Synchronized) {
				//QString imageTitle = QString::fromUtf8(buffer);
				QString imageTitle;
				QDataStream dsUpcomingImage(buffer);
				dsUpcomingImage >> imageTitle;
				emit connectionUpcomingImage(this, imageTitle);
			}
			break;
	case switchServer:
		  if (state == Synchronized) {
			  QHostAddress address;
			  quint16 port;
			  QDataStream ds(buffer);
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
		
	currentLanDataType = Undefined;
	currentDataType = DkConnection::Undefined;
	numBytesForCurrentDataType = 0;
	buffer.clear();
}

void DkLANConnection::sendNewPositionMessage(QRect position, bool opacity, bool overlaid) {
	if(!allowPosition)
		return;

	DkConnection::sendNewPositionMessage(position, opacity, overlaid);
}

void DkLANConnection::sendNewTransformMessage(QTransform transform, QTransform imgTransform, QPointF canvasSize) {
	if (!allowTransformation)
		return;

	DkConnection::sendNewTransformMessage(transform, imgTransform, canvasSize);
}

void DkLANConnection::sendNewFileMessage(qint16 op , QString filename) {
	if (!allowFile)
		return;

	DkConnection::sendNewFileMessage(op, filename);
}

// DkRemoteControlConnection --------------------------------------------------------------------
DkRCConnection::DkRCConnection(QObject* parent /* = 0 */) : DkLANConnection(parent) {
	currentRemoteControlDataType = Undefined;
	rcType = remoteUndefined;
}

void DkRCConnection::readGreetingMessage() {
	DkLANConnection::readGreetingMessage();
	allowFile = true;
	allowImage = true;
	allowPosition = true;
	allowTransformation = true;
	//sendAskForPermission(); // if here to many messages are sent ... wait until readyforuse in network.cpp
}

bool DkRCConnection::readProtocolHeader() {
	//qDebug() << __FUNCTION__ << " " << __LINE__;
	QByteArray newPermissionBA = QByteArray("PERMISSION").append(SeparatorToken);
	QByteArray newAskPermissionBA = QByteArray("ASKPERMISSION").append(SeparatorToken);
	QByteArray newRCType = QByteArray("RCTYPE").append(SeparatorToken);

	if (buffer == newPermissionBA) {
		qDebug() << "New Permission received from:" << this->peerAddress() << ":" << this->peerPort();
		currentRemoteControlDataType = newPermission;
	} else if (buffer == newAskPermissionBA) {
		qDebug() << "New Ask Permission received from:" << this->peerAddress() << ":" << this->peerPort();
		currentRemoteControlDataType = newAskPermission;
	} else if (buffer == newRCType) {
		qDebug() << "New RCType received from:" << this->peerAddress() << ":" << this->peerPort();
		currentRemoteControlDataType = newRcType;
	} else {
		return DkLANConnection::readProtocolHeader();
	}

	buffer.clear();
	numBytesForCurrentDataType = dataLengthForCurrentDataType();
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
		if (currentDataType == DkConnection::Undefined && currentLanDataType == DkLANConnection::Undefined && currentRemoteControlDataType == DkRCConnection::Undefined) {
			readDataIntoBuffer();
			if (!readProtocolHeader())
				return;
			checkState();
		}

		if (!hasEnoughData()) {
			return;
		}
		buffer = read(numBytesForCurrentDataType);
		if (buffer.size() != numBytesForCurrentDataType) {
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
			QDataStream ds(buffer);
			ds >> allowedToConnect;
			ds >> dummy;
			emit connectionNewPermission(this, allowedToConnect);
			qDebug() << "emitted connectionNewPermission: allowedToConnect:" << allowedToConnect;
			}
		break;
	case newAskPermission:  {
		QString dummy;
		QDataStream ds(buffer);
		ds >> dummy;
		qDebug() << "askPermission processed ... sending Permission";
		sendPermission();
		}
		break;
	case newRcType: {
		int tmp;
		QDataStream ds(buffer);
		ds >> tmp;
		rcType = static_cast<RemoteControlType>(tmp);
		emit connectionNewRCType(this, rcType);
		}
	case Undefined:
	default: 
		DkLANConnection::processData();
	}

	currentRemoteControlDataType = DkRCConnection::Undefined;
	currentLanDataType = DkLANConnection::Undefined;
	currentDataType = DkConnection::Undefined;
	numBytesForCurrentDataType = 0;
	buffer.clear();
}

void DkRCConnection::sendAskForPermission() {
	qDebug() << "sending askForPermission to " << this->peerName() << ":" << this->peerPort();

	QByteArray ba;
	QDataStream ds(&ba, QIODevice::ReadWrite);
	ds << "dummyMessage";

	QByteArray data = "ASKPERMISSION";
	qDebug() << "ba.size:" << ba.size();
	data.append(SeparatorToken).append(QByteArray::number(ba.size())).append(SeparatorToken).append(ba);
	write(data);
	this->waitForBytesWritten();
}

void DkRCConnection::sendPermission() {
	qDebug() << "printing whitelist\n--------------------";
	for (int i = 0; i < DkSettings::Sync::syncWhiteList.size(); i++)
		qDebug() << DkSettings::Sync::syncWhiteList.at(i);
	qDebug() << "--------------------";
	qDebug() << "current client Name:" << getClientName();
	qDebug() << "sending Permission to " << this->peerName() << ":" << this->peerPort() << "      value:" << DkSettings::Sync::syncWhiteList.contains(getClientName());

	QByteArray ba;
	QDataStream ds(&ba, QIODevice::ReadWrite);
	ds << DkSettings::Sync::syncWhiteList.contains(getClientName());
	ds << "dummyText";
	QByteArray data = "PERMISSION";
	data.append(SeparatorToken).append(QByteArray::number(ba.size())).append(SeparatorToken).append(ba);
	write(data);
	this->waitForBytesWritten();
}

void DkRCConnection::sendRCType(RemoteControlType type) {
	QByteArray ba;
	QDataStream ds(&ba, QIODevice::ReadWrite);
	ds << type;
	QByteArray data = "RCTYPE";
	data.append(SeparatorToken).append(QByteArray::number(ba.size())).append(SeparatorToken).append(ba);
	write(data);
	this->waitForBytesWritten();
}


}
