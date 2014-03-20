/**************************************************
 * 	DkUpnp.cpp
 *
 *	Created on:	18.03.2014
 * 	    Author:	Markus Diem
 *				Stefan Fiel
 * 				Florian Kleber
 *     Company:	Vienna University of Technology
 **************************************************/


#include "DkUpnp.h"

namespace nmc{

// DkUpnpDeviceHost --------------------------------------------------------------------
DkUpnpDeviceHost::DkUpnpDeviceHost() {
	tcpServerPort = 0;
	wlServerPort = 0;
}

void DkUpnpDeviceHost::tcpServerPortChanged(quint16 port) {
	qDebug() << "DkUpnpDeviceHost: setting tcp port" << port;
	this->tcpServerPort = port;
	Herqq::Upnp::HServerDevices devices = rootDevices();
	for (auto itr = devices.begin(); itr != devices.end(); itr++) {
		Herqq::Upnp::HServerService* service =  (*itr)->serviceById(Herqq::Upnp::HServiceId("urn:nomacs-org:service:nomacsService:1"));
		if (service)
			service->stateVariables().value("tcpServerPort")->setValue(port);
	}	

}

void DkUpnpDeviceHost::wlServerPortChanged(quint16 port) {
	qDebug() << "DkUpnpDeviceHost: setting wl port" << port;
	this->wlServerPort = port;
	Herqq::Upnp::HServerDevices devices = rootDevices();
	for (auto itr = devices.begin(); itr != devices.end(); itr++) {
		Herqq::Upnp::HServerService* service =  (*itr)->serviceById(Herqq::Upnp::HServiceId("urn:nomacs-org:service:nomacsService:1"));
		if(service)
			service->stateVariables().value("whiteListServerPort")->setValue(port);
	}
}

bool DkUpnpDeviceHost::startDevicehost(QString pathToConfig) {
	qDebug() << "starting DeviceHost";
	QFile f(pathToConfig);
	if (!f.exists()) {
		qDebug() << "DkUpnpDeviceHost: config file not found";
		return false;
	}
	DkUpnpDeviceModelCreator creator(tcpServerPort, wlServerPort);

	Herqq::Upnp::HDeviceHostConfiguration hostConfig;
	hostConfig.setDeviceModelCreator(creator);
	
	Herqq::Upnp::HDeviceConfiguration config;
	config.setPathToDeviceDescription(pathToConfig);
	
	hostConfig.add(config);
	
	bool retVal = init(hostConfig);
	if (!retVal) {
		qDebug() << "error while initializing device host:\n" << errorDescription();
	}
	return retVal;
}


void DkUpnpDeviceHost::stopDevicehost() {
	qDebug() << "DkUpnpDeviceHost: stopping DeviceHost";
	quit();
}

// DkUpnpServer --------------------------------------------------------------------	
DkUpnpDeviceModelCreator::DkUpnpDeviceModelCreator(quint16 tcpServerPort, quint16 wlServerPort) {
	this->tcpServerPort = tcpServerPort;
	this->wlServerPort = wlServerPort;
	qDebug() << "DkUpnpDeviceModelCreator port:" << this->tcpServerPort;
}

DkUpnpServer* DkUpnpDeviceModelCreator::createDevice(const Herqq::Upnp::HDeviceInfo& info) const {
	qDebug() << "DkUpnpDeviceModelCreator: creating device: " << info.deviceType().toString();
	if (info.deviceType().toString() == "urn:nomacs-org:device:nomacs:1") {
		return new DkUpnpServer();
	}
	qDebug() << "DkUpnpDeviceModelCreator: unable to create Device. deviceType not supported";
	return 0;
}

// DkUpnpService --------------------------------------------------------------------
DkUpnpService::DkUpnpService(quint16 tcpServerPort, quint16 wlServerPort) {
	this->tcpServerPort = tcpServerPort;
	this->wlServerPort = wlServerPort;
	qDebug() << "DkUpnpService port:" << tcpServerPort;
}

DkUpnpService* DkUpnpDeviceModelCreator::createService(const Herqq::Upnp::HServiceInfo& serviceInfo, const Herqq::Upnp::HDeviceInfo& deviceInfo) const {
	qDebug() << "DkUpnpDeviceModelCreator: creating service: " << serviceInfo.serviceType().toString();
	if (serviceInfo.serviceType().toString() == "urn:nomacs-org:service:nomacsService:1") {
		qDebug() << "creating new service port:" << tcpServerPort;
		return new DkUpnpService(tcpServerPort, wlServerPort);
	}
	qDebug() << "DkUpnpDeviceModelCreator: unable to create Service. serviceType not supported";
	return 0;
}

// DkUpnpService --------------------------------------------------------------------
qint32 DkUpnpService::getTCPServerURL(const Herqq::Upnp::HActionArguments& inArgs, Herqq::Upnp::HActionArguments* outArgs) {
	qDebug() << "im getTCPServerURL: sending " << tcpServerPort;
	int port = stateVariables().value("tcpServerPort")->value().toInt();
	qDebug() << "port: " << port;
	outArgs->setValue("tcpServerPort", port);
	return Herqq::Upnp::UpnpSuccess;
}

qint32 DkUpnpService::getWhitelistServerURL(const Herqq::Upnp::HActionArguments& inArgs, Herqq::Upnp::HActionArguments* outArgs) {
	qDebug() << "im getWhitelistServerURL: sending" << wlServerPort;
	outArgs->setValue("whiteListServerPort", wlServerPort);
	return Herqq::Upnp::UpnpSuccess;
}

void DkUpnpService::setTcpServerPort(quint16 port) {
	qDebug() << "tcp server port set";
	tcpServerPort = port;
	stateVariables().value("tcpServerPort")->setValue(port);
}

void DkUpnpService::setWlServerPort(quint16 port) {
	qDebug() << "wl server port set";
	wlServerPort = port;
	stateVariables().value("whiteListServerPort")->setValue(port);
}

// DkUpnpControlPoint --------------------------------------------------------------------
bool DkUpnpControlPoint::init() {
	controlPoint = new Herqq::Upnp::HControlPoint(this);
	connect(controlPoint, SIGNAL(rootDeviceOnline(Herqq::Upnp::HClientDevice*)), this, SLOT(rootDeviceOnline(Herqq::Upnp::HClientDevice*)));
	connect(controlPoint, SIGNAL(rootDeviceOffline(Herqq::Upnp::HClientDevice*)), this, SLOT(rootDeviceOffline(Herqq::Upnp::HClientDevice*)));

	if (!controlPoint->init()) {
		qDebug() << "cannot init controlPoint";
		return false;
	}

	localIpAddresses.clear();
	QList<QNetworkInterface> networkInterfaces = QNetworkInterface::allInterfaces();
	for (QList<QNetworkInterface>::iterator networkInterfacesItr = networkInterfaces.begin(); networkInterfacesItr != networkInterfaces.end(); networkInterfacesItr++) {
		QList<QNetworkAddressEntry> entires = networkInterfacesItr->addressEntries();
		for (QList<QNetworkAddressEntry>::iterator itr = entires.begin(); itr != entires.end(); itr++) {
			localIpAddresses << itr->ip();
		}
	}

	return true;
}



void DkUpnpControlPoint::rootDeviceOnline(Herqq::Upnp::HClientDevice* clientDevice) {
	qDebug() << "rootDeviceOnline:" << clientDevice->info().deviceType().toString();
	qDebug() << "manufacturer:" << clientDevice->info().manufacturer();
	if(clientDevice->info().manufacturer()=="nomacs") {
		qDebug() << "nomacs found!!!";
		//qDebug() << "description:" << clientDevice->description();
		
		QList<QUrl> locations = clientDevice->locations(Herqq::Upnp::AbsoluteUrl);
		QUrl url;
		for (auto itr = locations.begin(); itr != locations.end(); itr++) {
			url = *itr;
		}
		if(url.isEmpty()) {
			qDebug() << "url is empty, aborting";
			return;
		}
		QHostAddress host = QHostAddress(url.host());
		if(isLocalHostAddress(host)) {
			//qDebug() << "is local address ... aborting";
			return;
		}


		Herqq::Upnp::HClientServices services = clientDevice->services();
		for(int i = 0; i < (int) services.size(); i++) {
			Herqq::Upnp::HClientService* service = services.at(i); 
			qDebug() << "service " << i << ":" << service->description() ;
			Herqq::Upnp::HClientActions actions = service->actions();
			Herqq::Upnp::HActionArguments aas;
			Herqq::Upnp::HClientActionOp cao;

			// ask for LAN server
			connect(actions.value("getTCPServerURL"), SIGNAL(invokeComplete(Herqq::Upnp::HClientAction*, const Herqq::Upnp::HClientActionOp&)), this, SLOT(invokeComplete(Herqq::Upnp::HClientAction*, const Herqq::Upnp::HClientActionOp&)));
			cao = actions.value("getTCPServerURL")->beginInvoke(aas);

			// ask for RC server
			connect(actions.value("getWhitelistServerURL"), SIGNAL(invokeComplete(Herqq::Upnp::HClientAction*, const Herqq::Upnp::HClientActionOp&)), this, SLOT(invokeComplete(Herqq::Upnp::HClientAction*, const Herqq::Upnp::HClientActionOp&)));
			cao = actions.value("getWhitelistServerURL")->beginInvoke(aas);
		}

		//emit newNomacsFound(url.host(), )

		//Herqq::Upnp::HClientServices services = clientDevice->services();
		//for(int i = 0; i < (int) services.size(); i++) {
		//	Herqq::Upnp::HClientService* service = services.at(i); 
		//	qDebug() << "service " << i << ":" << service->description() ;
		//	Herqq::Upnp::HClientActions actions = service->actions();
		//	QList<QString> keys = actions.keys();
		//	for (int j = 0; j < (int) keys.size(); j++) {
		//		qDebug() << "action " << j << ": " << keys.at(j);
		//	}
		//	Herqq::Upnp::HActionArgument aa("newTargetValue", Herqq::Upnp::HStateVariableInfo("Target", Herqq::Upnp::HUpnpDataTypes::boolean, Herqq::Upnp::HInclusionRequirement::InclusionOptional));
		//	aa.setValue(true);
		//	Herqq::Upnp::HActionArguments aas;
		//	aas.append(aa);
		//	actions.value("SetTarget")->beginInvoke(aas);
		//}
	}
}

void DkUpnpControlPoint::invokeComplete(Herqq::Upnp::HClientAction* clientAction, const Herqq::Upnp::HClientActionOp& clientActionOp) {
	qDebug() << "im invoke Complete";

	QList<QUrl> locations = clientAction->parentService()->parentDevice()->locations();
	QUrl url;
	for (auto itr = locations.begin(); itr != locations.end(); itr++) {
		url = *itr;
	}
	if(url.isEmpty()) {
		qDebug() << "url is empty, aborting";
		return;
	}
	QHostAddress address = QHostAddress(url.host());
	if(isLocalHostAddress(address)) {
		//qDebug() << "is local address ... aborting";
		return;
	}



	Herqq::Upnp::HActionArguments arguments = clientActionOp.outputArguments();
	int port = arguments.get("tcpServerPort").value().toInt();
	if (port > 0 ) {
		qDebug() << "emitting newLANNomacsFound:" << address << " port:" << port;
		quint16 quintPort = port;
		emit newLANNomacsFound(address, quintPort, "");
		//QCoreApplication::sendPostedEvents();
	}
	port = arguments.get("whitelistServerPort").value().toInt();
	if (port > 0 ) {
		QList<QUrl> locations = clientAction->parentService()->parentDevice()->locations();
		QUrl url;
		for (auto itr = locations.begin(); itr != locations.end(); itr++) {
			url = *itr;
		}
		if(url.isEmpty()) {
			qDebug() << "url is empty, aborting";
			return;
		}
		QHostAddress address = QHostAddress(url.host());
		emit newRCNomacsFound(address, port, "");
	}
}

void DkUpnpControlPoint::rootDeviceOffline(Herqq::Upnp::HClientDevice* clientDevice) {
	qDebug() << "rootDeviceOffline:" << clientDevice->info().deviceType().toString();

	controlPoint->removeRootDevice(clientDevice);
}

bool DkUpnpControlPoint::isLocalHostAddress(const QHostAddress address) {
	foreach (QHostAddress localAddress, localIpAddresses) {
		if (address == localAddress)
			return true;
	}
	return false;
}


}
