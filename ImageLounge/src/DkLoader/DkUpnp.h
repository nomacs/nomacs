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

#ifdef WITH_UPNP

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QDebug>
#include <QFile>
#include <QUrl>
#include <QHostAddress>
#include <QCoreApplication>
#include <QNetworkInterface>
#include <QUuid>
#include <QDir>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPointer>
#include <QNetworkAccessManager>
#include <QImage>

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

#include <HUpnpAV/HAvControlPoint>
#include <HUpnpAv/HRendererConnectionManager>
#include <HUpnpAv/HMediaRendererDeviceConfiguration>
#include <HUpnpAv/HAvDeviceModelCreator>
#include <HUpnpAv/HRendererConnection>
#include <HUpnpAv/HSeekInfo>
#include <HUpnpAv/HProtocolInfo>
#pragma warning(pop)		// no warnings from includes - end

#ifdef QT_NO_DEBUG_OUTPUT
#pragma warning(disable: 4127)		// no 'conditional expression is constant' if qDebug() messages are removed
#endif

namespace nmc {

	class DkUpnpDeviceHost : public Herqq::Upnp::HDeviceHost {
		Q_OBJECT
		public:
			DkUpnpDeviceHost();
			virtual ~DkUpnpDeviceHost();
			bool startDevicehost(QString pathToConfig);
			void stopDevicehost(); 
		public slots:
			void tcpServerPortChanged(quint16 port);
			void wlServerPortChanged(quint16 port);

		private:
			quint16 tcpServerPort, wlServerPort;
			QString serviceXMLPath;
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

// MediaRenderer --------------------------------------------------------------------

	class DkUpnpRendererConnection : public Herqq::Upnp::Av::HRendererConnection {
		Q_OBJECT
		public:
			DkUpnpRendererConnection();
		signals:
			void newImage(QImage img);

	protected:
		virtual qint32 doPlay(const QString &speed) { qDebug() << "doPlay: speed:" << speed; return Herqq::Upnp::UpnpErrorCode::UpnpSuccess;};
		virtual qint32 doStop() { qDebug() << "doStop";return Herqq::Upnp::UpnpErrorCode::UpnpSuccess;};
		virtual qint32 doSeek(const Herqq::Upnp::Av::HSeekInfo&) { qDebug() << "doSeek";return Herqq::Upnp::UpnpErrorCode::UpnpSuccess;};
		virtual qint32 doNext() { qDebug() << "doNext";return Herqq::Upnp::UpnpErrorCode::UpnpSuccess;};
		virtual qint32 doPrevious() { qDebug() << "doPrevious";return Herqq::Upnp::UpnpErrorCode::UpnpSuccess;};
		virtual qint32 doSetResource(const QUrl &resourceUri, Herqq::Upnp::Av::HObject *cdsMetadata=0);
		virtual qint32 doSetNextResource(const QUrl &resourceUri, Herqq::Upnp::Av::HObject * = 0) { qDebug() << "doSetNextResource url:" << resourceUri;return Herqq::Upnp::UpnpErrorCode::UpnpSuccess;};
		virtual qint32 doSelectPreset(const QString &presetName) { qDebug() << "doSelectPreset preset:" << presetName;return Herqq::Upnp::UpnpErrorCode::UpnpSuccess;};
	private slots:
		void finished();
	private:
		QPointer<QNetworkReply> curResource;
		QNetworkAccessManager accessManager;
	};

	class DkUpnpRendererConnectionManager : public Herqq::Upnp::Av::HRendererConnectionManager {
		Q_OBJECT
		protected:
			virtual Herqq::Upnp::Av::HRendererConnection* doCreate(Herqq::Upnp::Av::HAbstractConnectionManagerService* service, Herqq::Upnp::Av::HConnectionInfo* cinfo);

		private slots:
			void conNewImage(QImage img) {emit newImage(img);};
		private:
			Herqq::Upnp::Av::HRendererConnection* rendererConnection;
		signals:
			void newImage(QImage img);

	};

	class DkUpnpRendererDeviceHost : public Herqq::Upnp::HDeviceHost {
		Q_OBJECT
		public:
			DkUpnpRendererDeviceHost () {};
			virtual ~DkUpnpRendererDeviceHost () {qDebug() << "deleting Devicehost!";};
			bool startDevicehost(QString pathToConfig);
			void stopDevicehost(); 
		signals:
			void newImage(QImage);
			private slots:
				void cmNewImage(QImage img) {emit newImage(img);};
		private:
			DkUpnpRendererConnectionManager* connectionManager;
	};

};

#endif //#ifdef WITH_UPNP