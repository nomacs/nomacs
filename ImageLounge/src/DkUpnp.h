/**************************************************
 * 	DkUpnp.h
 *
 *	Created on:	18.03.2014
 * 	    Author:	Markus Diem
 *				Stefan Fiel
 *				Angelika Garz
 * 				Florian Kleber
 *     Company:	Vienna University of Technology
 **************************************************/

#pragma once

#include <QDebug>
#include <QFile>
#include <QUrl>
#include <QHostAddress>
#include <QCoreApplication>
#include <QNetworkInterface>
#include <QUuid>
#include <QDir>

#include <HUpnpCore/HServerDevice>
#include <HUpnpCore/HServerService>
#include <HUpnpCore/HDeviceConfiguration>
#include <HUpnpCore/HControlPoint>
#include <HUpnpCore/HDeviceModelCreator>
#include <HUpnpCore/HDeviceInfo>
#include <HUpnpCore/HServiceInfo>
#include <HUpnpCore/HDeviceHostConfiguration>
#include <HUpnpCore/HDeviceHost>
#include <HUpnpCore/HActionArgument>
#include <HUpnpCore/HServerStateVariable>
#include <HUpnpCore/HServiceId>

#include <HUpnpCore/HActionInvoke>
#include <HUpnpCore/HClientService>
#include <HUpnpCore/HClientStateVariable>
#include <HUpnpCore/HClientAction>
#include <HUpnpCore/HClientActionOp>
#include <HUpnpCore/HAsyncOp>
#include <HUpnpCore/HControlPointConfiguration>

namespace nmc {

	class DkUpnpDeviceHost : public Herqq::Upnp::HDeviceHost {
		Q_OBJECT
		public:
			DkUpnpDeviceHost();
			virtual ~DkUpnpDeviceHost() {qDebug() << "deleting Devicehost!";};
			bool startDevicehost(QString pathToConfig);
			void stopDevicehost(); 
		public slots:
			void tcpServerPortChanged(quint16 port);
			void wlServerPortChanged(quint16 port);

		private:
			quint16 tcpServerPort, wlServerPort;
	};

	class DkUpnpServer : public Herqq::Upnp::HServerDevice {
		public:
			DkUpnpServer() {};
			virtual ~DkUpnpServer() {};
	};

	class DkUpnpService : public Herqq::Upnp::HServerService {
		Q_OBJECT
		public:
			DkUpnpService();
			virtual ~DkUpnpService() {};

			Q_INVOKABLE qint32 getTCPServerURL(const Herqq::Upnp::HActionArguments& inArgs, Herqq::Upnp::HActionArguments* outArgs);
			Q_INVOKABLE qint32 getWhiteListServerURL(const Herqq::Upnp::HActionArguments& inArgs, Herqq::Upnp::HActionArguments* outArgs);
	};

	class DkUpnpDeviceModelCreator : public Herqq::Upnp::HDeviceModelCreator {
		public:	
			DkUpnpDeviceModelCreator();
			virtual DkUpnpServer* createDevice(const Herqq::Upnp::HDeviceInfo& info) const;
			virtual DkUpnpService* createService(const Herqq::Upnp::HServiceInfo& serviceInfo, const Herqq::Upnp::HDeviceInfo& deviceInfo) const;
			virtual DkUpnpDeviceModelCreator* newInstance() const {return new DkUpnpDeviceModelCreator();};
		private:
	};

	class DkUpnpControlPoint : public QObject {
		Q_OBJECT
		public:
			DkUpnpControlPoint() { cpIsStarted=false; };
			virtual ~DkUpnpControlPoint();
			bool init();
			bool isStarted();
		signals:
			void newLANNomacsFound(QHostAddress address, quint16 port, QString name);
			void newRCNomacsFound(QHostAddress address, quint16 port, QString name);

		private slots:
			void rootDeviceOnline(Herqq::Upnp::HClientDevice*);
			void rootDeviceOffline(Herqq::Upnp::HClientDevice*);
			void invokeComplete(Herqq::Upnp::HClientAction* clientAction, const Herqq::Upnp::HClientActionOp& clientActionOp);
			void tcpValueChanged(const Herqq::Upnp::HClientStateVariable *source, const Herqq::Upnp::HStateVariableEvent &event);
			void wlValueChanged(const Herqq::Upnp::HClientStateVariable *source, const Herqq::Upnp::HStateVariableEvent &event);
		private:
			bool isLocalHostAddress(const QHostAddress address);
			bool cpIsStarted;
			Herqq::Upnp::HControlPoint* controlPoint;
			QList<QHostAddress> localIpAddresses;
	};



};
