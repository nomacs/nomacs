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
}

bool DkUpnpDeviceHost::startDevicehost(QString pathToConfig, quint16 tcpServerPort, quint16 wlServerPort) {
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
	qDebug() << "im getTCPServerURL";
	qDebug() << "sending tcpServerPort:" << tcpServerPort;
	outArgs->setValue("tcpServerPort", tcpServerPort);
	return Herqq::Upnp::UpnpSuccess;
}

qint32 DkUpnpService::getWhitelistServerURL(const Herqq::Upnp::HActionArguments& inArgs, Herqq::Upnp::HActionArguments* outArgs) {
	qDebug() << "im getWhitelistServerURL";
	outArgs->setValue("whiteListServerPort", wlServerPort);
	return Herqq::Upnp::UpnpSuccess;
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
	return true;
}



void DkUpnpControlPoint::rootDeviceOnline(Herqq::Upnp::HClientDevice* clientDevice) {
	qDebug() << "rootDeviceOnline:" << clientDevice->info().deviceType().toString();
	qDebug() << "manufacturer:" << clientDevice->info().manufacturer();
	if(clientDevice->info().manufacturer()=="nomacs") {
		qDebug() << "nomacs found!!!";
		//qDebug() << "description:" << clientDevice->description();
		
		qDebug() << "locations:";
		QList<QUrl> locations = clientDevice->locations(Herqq::Upnp::AbsoluteUrl);
		QUrl url;
		for (auto itr = locations.begin(); itr != locations.end(); itr++) {
			url = *itr;
		}
		if(url.isEmpty()) {
			qDebug() << "url is empty, aborting";
			return;
		}

		Herqq::Upnp::HClientServices services = clientDevice->services();
		for(int i = 0; i < (int) services.size(); i++) {
			Herqq::Upnp::HClientService* service = services.at(i); 
			qDebug() << "service " << i << ":" << service->description() ;
			Herqq::Upnp::HClientActions actions = service->actions();
			Herqq::Upnp::HActionArguments aas;
			connect(actions.value("getTCPServerURL"), SIGNAL(invokeComplete(Herqq::Upnp::HClientAction*, const Herqq::Upnp::HClientActionOp&)), this, SLOT(invokeComplete(Herqq::Upnp::HClientAction*, const Herqq::Upnp::HClientActionOp&)));
			Herqq::Upnp::HClientActionOp cao;
			cao = actions.value("getTCPServerURL")->beginInvoke(aas);
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
	Herqq::Upnp::HActionArguments arguments = clientActionOp.outputArguments();
	int port = arguments.get("tcpServerPort").value().toInt();
	
	qDebug() << "im invoke complete, serverport:" << port;
	qDebug() << "locations:";
	QList<QUrl> locations = clientAction->parentService()->parentDevice()->locations();
	QUrl url;
	for (auto itr = locations.begin(); itr != locations.end(); itr++) {
		url = *itr;
	}
	if(url.isEmpty()) {
		qDebug() << "url is empty, aborting";
		return;
	}
	qDebug() << "im invoke script: ip:" << url.host();
	QHostAddress address = QHostAddress(url.host());
	emit newNomacsFound(address, port, "");

}

void DkUpnpControlPoint::rootDeviceOffline(Herqq::Upnp::HClientDevice* clientDevice) {
	qDebug() << "rootDeviceOffline:" << clientDevice->info().deviceType().toString();

	controlPoint->removeRootDevice(clientDevice);
}



}
