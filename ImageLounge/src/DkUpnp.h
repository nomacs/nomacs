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

#include <HUpnpCore/HActionInvoke>
#include <HUpnpCore/HClientService>
#include <HUpnpCore/HClientAction>
#include <HUpnpCore/HClientActionOp>
#include <HUpnpCore/HAsyncOp>

namespace nmc {

	class DkUpnpDeviceHost : public Herqq::Upnp::HDeviceHost {
		public:
			DkUpnpDeviceHost();
			bool startDevicehost(QString pathToConfig, quint16 tcpServerPort, quint16 wlServerPort);
			void stopDevicehost(); 

	};

	class DkUpnpServer : public Herqq::Upnp::HServerDevice {
		public:
			DkUpnpServer() {};
			virtual ~DkUpnpServer() {};
	};

	class DkUpnpService : public Herqq::Upnp::HServerService {
		Q_OBJECT
		public:
			DkUpnpService(quint16 tcpServerPort, quint16 wlServerPort);
			virtual ~DkUpnpService() {};

			Q_INVOKABLE qint32 getTCPServerURL(const Herqq::Upnp::HActionArguments& inArgs, Herqq::Upnp::HActionArguments* outArgs);
			Q_INVOKABLE qint32 getWhitelistServerURL(const Herqq::Upnp::HActionArguments& inArgs, Herqq::Upnp::HActionArguments* outArgs);

		private:
			quint16 tcpServerPort;
			quint16 wlServerPort;
	};

	class DkUpnpDeviceModelCreator : public Herqq::Upnp::HDeviceModelCreator {
		public:	
			//DkUpnpDeviceModelCreator() {};
			DkUpnpDeviceModelCreator(quint16 tcpServerPort, quint16 wlServerPort);
			virtual DkUpnpServer* createDevice(const Herqq::Upnp::HDeviceInfo& info) const;
			virtual DkUpnpService* createService(const Herqq::Upnp::HServiceInfo& serviceInfo, const Herqq::Upnp::HDeviceInfo& deviceInfo) const;
			virtual DkUpnpDeviceModelCreator* newInstance() const {return new DkUpnpDeviceModelCreator(tcpServerPort, wlServerPort);};
		private:
			quint16 tcpServerPort;
			quint16 wlServerPort;
	};

	class DkUpnpControlPoint : public QObject {
		Q_OBJECT
		public:
			DkUpnpControlPoint() { init();};
			virtual ~DkUpnpControlPoint() {}; // TODO!!
			bool init();

		signals:
			void newNomacsFound(QHostAddress address, quint16 port, QString name);

		private slots:
			void rootDeviceOnline(Herqq::Upnp::HClientDevice*);
			void rootDeviceOffline(Herqq::Upnp::HClientDevice*);
			void invokeComplete(Herqq::Upnp::HClientAction* clientAction, const Herqq::Upnp::HClientActionOp& clientActionOp);

		private:
			Herqq::Upnp::HControlPoint* controlPoint;
	};



};
