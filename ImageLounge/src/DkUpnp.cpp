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
	for (Herqq::Upnp::HServerDevices::iterator itr = devices.begin(); itr != devices.end(); itr++) {
		Herqq::Upnp::HServerService* service =  (*itr)->serviceById(Herqq::Upnp::HServiceId("urn:nomacs-org:service:nomacsService:1"));
		if (service)
			service->stateVariables().value("tcpServerPort")->setValue(port);
	}	

}

void DkUpnpDeviceHost::wlServerPortChanged(quint16 port) {
	qDebug() << "DkUpnpDeviceHost: setting wl port" << port;
	this->wlServerPort = port;
	Herqq::Upnp::HServerDevices devices = rootDevices();
	for (Herqq::Upnp::HServerDevices::iterator itr = devices.begin(); itr != devices.end(); itr++) {
		Herqq::Upnp::HServerService* service =  (*itr)->serviceById(Herqq::Upnp::HServiceId("urn:nomacs-org:service:nomacsService:1"));
		if(service)
			service->stateVariables().value("whiteListServerPort")->setValue(port);
	}
}

bool DkUpnpDeviceHost::startDevicehost(QString pathToConfig) {
	qDebug() << "pathToConfig:" << pathToConfig;
	qDebug() << "starting DeviceHost";
	QFile f(pathToConfig);
	if (!f.exists()) {
		qDebug() << "DkUpnpDeviceHost: config file not found";
		return false;
	}

	QUuid uuid = QUuid::createUuid();
	QString uuidString = uuid.toString();
	uuidString.replace("{","");
	uuidString.replace("}","");
	QString newXMLpath = QDir::tempPath() + QDir::separator() + uuidString + ".xml";
	

	QByteArray fileData;
	f.open(QIODevice::ReadOnly);
	//f.seek(0);
	fileData = f.readAll();
	f.close();
	QString fileText(fileData);
	fileText.replace("insert-new-uuid-here", uuidString);
#ifdef WIN32
	fileText.replace("nomacs-service.xml", QDir::temp().dirName()+"/nomacs-service.xml");
#else
	fileText.replace("nomacs-service.xml", "/nomacs-service.xml");
#endif // WIN32

	
	
	QFile newXMLfile(newXMLpath);
	newXMLfile.open(QIODevice::WriteOnly);
	newXMLfile.write(fileText.toUtf8());
	qDebug() << "writing file:" << newXMLpath;
	
	newXMLfile.close();

	QFileInfo fileInfo = QFileInfo(f);
	QFile serviceXML(fileInfo.absolutePath() + QDir::separator() + "nomacs-service.xml");
	if (!serviceXML.exists())
		qDebug() << "nomacs-service.xml file does not exist";
	QString newServiceXMLPath = QDir::tempPath()+ QDir::separator() + "nomacs-service.xml";
	if(!QFile::exists(newServiceXMLPath)) {
		if (!serviceXML.copy(newServiceXMLPath))
			qDebug() << "unable to copy nomacs-service.xml to " << newServiceXMLPath << ", perhaps files already exists";
	}
	QFile newServiceXMLFile(QDir::tempPath()+ QDir::separator() + "nomacs-service.xml");
	serviceXML.close();
	DkUpnpDeviceModelCreator creator;

	Herqq::Upnp::HDeviceHostConfiguration hostConfig;
	hostConfig.setDeviceModelCreator(creator);
	
	Herqq::Upnp::HDeviceConfiguration config;
	//config.setPathToDeviceDescription(pathToConfig);
	config.setPathToDeviceDescription(newXMLpath);
	
	hostConfig.add(config);
	
	bool retVal = init(hostConfig);
	if (!retVal) {
		qDebug() << "error while initializing device host:\n" << errorDescription();
	}

	newXMLfile.remove();
	newServiceXMLFile.remove();
	return retVal;
}


void DkUpnpDeviceHost::stopDevicehost() {
	qDebug() << "DkUpnpDeviceHost: stopping DeviceHost";
	quit();
}

// DkUpnpServer --------------------------------------------------------------------	
DkUpnpDeviceModelCreator::DkUpnpDeviceModelCreator() {
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
DkUpnpService::DkUpnpService() {
	// do nothing
}

DkUpnpService* DkUpnpDeviceModelCreator::createService(const Herqq::Upnp::HServiceInfo& serviceInfo, const Herqq::Upnp::HDeviceInfo& deviceInfo) const {
	qDebug() << "DkUpnpDeviceModelCreator: creating service: " << serviceInfo.serviceType().toString();
	if (serviceInfo.serviceType().toString() == "urn:nomacs-org:service:nomacsService:1") {
		return new DkUpnpService();
	}
	qDebug() << "DkUpnpDeviceModelCreator: unable to create Service. serviceType not supported";
	return 0;
}

// DkUpnpService --------------------------------------------------------------------
qint32 DkUpnpService::getTCPServerURL(const Herqq::Upnp::HActionArguments& inArgs, Herqq::Upnp::HActionArguments* outArgs) {
	int port = stateVariables().value("tcpServerPort")->value().toInt();
	qDebug() << "DkUpnpService::getTCPServerURL sending port:" << port;
	outArgs->setValue("tcpServerPort", port);
	return Herqq::Upnp::UpnpSuccess;
}

qint32 DkUpnpService::getWhiteListServerURL(const Herqq::Upnp::HActionArguments& inArgs, Herqq::Upnp::HActionArguments* outArgs) {
	int port = stateVariables().value("whiteListServerPort")->value().toInt();
	qDebug() << "DkUpnpService::getWhiteListServerURL sending port:" << port;
	outArgs->setValue("whiteListServerPort", port);
	return Herqq::Upnp::UpnpSuccess;
}

// DkUpnpControlPoint --------------------------------------------------------------------
DkUpnpControlPoint::~DkUpnpControlPoint() {
	if(controlPoint) {
		delete controlPoint;
		controlPoint = 0;
	}
}

bool DkUpnpControlPoint::init() {
	//Herqq::Upnp::SetLoggingLevel(Herqq::Upnp::Debug);
	localIpAddresses.clear();
	QList<QNetworkInterface> networkInterfaces = QNetworkInterface::allInterfaces();
	for (QList<QNetworkInterface>::iterator networkInterfacesItr = networkInterfaces.begin(); networkInterfacesItr != networkInterfaces.end(); networkInterfacesItr++) {
		if (networkInterfacesItr->flags() & QNetworkInterface::IsUp) {
			QList<QNetworkAddressEntry> entires = networkInterfacesItr->addressEntries();
			for (QList<QNetworkAddressEntry>::iterator itr = entires.begin(); itr != entires.end(); itr++) {
				if (itr->ip() != QHostAddress::LocalHost) {
					localIpAddresses << itr->ip();
				}
			}
		}
	}

	//foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
	//	if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost)) {
	//		qDebug() << address.toString();
	//		localIpAddresses << address;
	//		qDebug() << "add:" << address;
	//	}
	//}

	Herqq::Upnp::HControlPointConfiguration config;
	config.setNetworkAddressesToUse(localIpAddresses);

	controlPoint = new Herqq::Upnp::HControlPoint(/*config,*/ this);
	
	connect(controlPoint, SIGNAL(rootDeviceOnline(Herqq::Upnp::HClientDevice*)), this, SLOT(rootDeviceOnline(Herqq::Upnp::HClientDevice*)));
	connect(controlPoint, SIGNAL(rootDeviceOffline(Herqq::Upnp::HClientDevice*)), this, SLOT(rootDeviceOffline(Herqq::Upnp::HClientDevice*)));

	if (!controlPoint->init()) {
		qDebug() << "cannot init controlPoint";
		return false;
	}

	cpIsStarted = true;
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
		for (QList<QUrl>::iterator itr = locations.begin(); itr != locations.end(); itr++) {
			url = *itr;
		}
		if(url.isEmpty()) {
			qDebug() << "url is empty, aborting";
			return;
		}
		QHostAddress host = QHostAddress(url.host());
		if(isLocalHostAddress(host)) {
			qDebug() << "is local address ... aborting";
			return;
		}

		Herqq::Upnp::HClientService* service = clientDevice->serviceById(Herqq::Upnp::HServiceId("urn:nomacs-org:service:nomacsService:1"));
		if (!service) {
			qDebug() << "nomacs service is empty ... aborting";
			return;
		}

		Herqq::Upnp::HClientActions actions = service->actions();
		qDebug() << "service:" << service->description();
		Herqq::Upnp::HActionArguments aas;
		Herqq::Upnp::HClientActionOp cao;

		// ask for LAN server
		if (!actions.value("getTCPServerURL")) {
			qDebug() << "action.value(getTCPServerURL) is null ... aborting";
			return;
		}
		connect(actions.value("getTCPServerURL"), SIGNAL(invokeComplete(Herqq::Upnp::HClientAction*, const Herqq::Upnp::HClientActionOp&)), this, SLOT(invokeComplete(Herqq::Upnp::HClientAction*, const Herqq::Upnp::HClientActionOp&)));
		cao = actions.value("getTCPServerURL")->beginInvoke(aas);

		// ask for RC server
		if (!actions.value("getWhiteListServerURL")) {
			qDebug() << "action.value(getWhiteListServerURL) is null ... aborting";
			return;
		}
		connect(actions.value("getWhiteListServerURL"), SIGNAL(invokeComplete(Herqq::Upnp::HClientAction*, const Herqq::Upnp::HClientActionOp&)), this, SLOT(invokeComplete(Herqq::Upnp::HClientAction*, const Herqq::Upnp::HClientActionOp&)));
		cao = actions.value("getWhiteListServerURL")->beginInvoke(aas);


		connect(service->stateVariables().value("tcpServerPort"), SIGNAL(valueChanged(const Herqq::Upnp::HClientStateVariable*, const Herqq::Upnp::HStateVariableEvent&)), this, SLOT(tcpValueChanged(const Herqq::Upnp::HClientStateVariable*, const Herqq::Upnp::HStateVariableEvent&)));
		connect(service->stateVariables().value("whiteListServerPort"), SIGNAL(valueChanged(const Herqq::Upnp::HClientStateVariable*, const Herqq::Upnp::HStateVariableEvent&)), this, SLOT(wlValueChanged(const Herqq::Upnp::HClientStateVariable*, const Herqq::Upnp::HStateVariableEvent&)));

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
	for (QList<QUrl>::iterator itr = locations.begin(); itr != locations.end(); itr++) {
		url = *itr;
	}
	if(url.isEmpty()) {
		qDebug() << "url is empty, aborting";
		return;
	}
	QHostAddress address = QHostAddress(url.host());
	if(isLocalHostAddress(address)) {
		qDebug() << "is local address ... aborting";
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
	port = arguments.get("whiteListServerPort").value().toInt();
	if (port > 0 ) {
		QList<QUrl> locations = clientAction->parentService()->parentDevice()->locations();
		QUrl url;
		for (QList<QUrl>::iterator itr = locations.begin(); itr != locations.end(); itr++) {
			url = *itr;
		}
		if(url.isEmpty()) {
			qDebug() << "url is empty, aborting";
			return;
		}
		QHostAddress address = QHostAddress(url.host());
		qDebug() << "emitting newRCNomacsFound:" << address << " port:" << port;
		emit newRCNomacsFound(address, port, "");
	}
}

void DkUpnpControlPoint::rootDeviceOffline(Herqq::Upnp::HClientDevice* clientDevice) {
	qDebug() << "rootDeviceOffline:" << clientDevice->info().deviceType().toString();

	controlPoint->removeRootDevice(clientDevice);
}

void DkUpnpControlPoint::tcpValueChanged(const Herqq::Upnp::HClientStateVariable *source, const Herqq::Upnp::HStateVariableEvent &event) {
	qDebug() << "im tcp value changed";
	QList<QUrl> locations = source->parentService()->parentDevice()->locations();
	QUrl url;
	for (QList<QUrl>::iterator itr = locations.begin(); itr != locations.end(); itr++) {
		url = *itr;
	}
	if(url.isEmpty()) {
		qDebug() << "url is empty, aborting";
		return;
	}
	QHostAddress address = QHostAddress(url.host());
	if(isLocalHostAddress(address)) {
		qDebug() << "is local address ... aborting";
		return;
	}

	quint16 port = source->value().toInt();
	if (port == 0)
		return;
	qDebug() << "emitting newLANNomacsFound:" << address << " port:" << port;
	emit newLANNomacsFound(address, port, "");
}

void DkUpnpControlPoint::wlValueChanged(const Herqq::Upnp::HClientStateVariable *source, const Herqq::Upnp::HStateVariableEvent &event) {
	qDebug() << "im wl value changed";
	QList<QUrl> locations = source->parentService()->parentDevice()->locations();
	QUrl url;
	for (QList<QUrl>::iterator itr = locations.begin(); itr != locations.end(); itr++) {
		url = *itr;
	}
	if(url.isEmpty()) {
		qDebug() << "url is empty, aborting";
		return;
	}
	QHostAddress address = QHostAddress(url.host());
	if(isLocalHostAddress(address)) {
		qDebug() << "is local address ... aborting";
		return;
	}

	quint16 port = source->value().toInt();
	if (port == 0)
		return;
	qDebug() << "emitting newRCNomacsFound:" << address << " port:" << port;
	emit newRCNomacsFound(address, port, "");

}

bool DkUpnpControlPoint::isLocalHostAddress(const QHostAddress address) {
	foreach (QHostAddress localAddress, localIpAddresses) {
		if (address == localAddress)
			return true;
	}
	return false;
}

bool DkUpnpControlPoint::isStarted() {
	return cpIsStarted;	
}

// DkUpnpRendererConnectionManager --------------------------------------------------------------------
Herqq::Upnp::Av::HRendererConnection* DkUpnpRendererConnectionManager::doCreate(Herqq::Upnp::Av::HAbstractConnectionManagerService* service, Herqq::Upnp::Av::HConnectionInfo* cinfo) {
	QString contentFormat = cinfo->protocolInfo().contentFormat();
	qDebug() << "content:" << contentFormat;
	rendererConnection = new DkUpnpRendererConnection();
	connect(rendererConnection, SIGNAL(newImage(QImage)), this, SLOT(conNewImage(QImage)));
	return rendererConnection;
}

// DkUpnpRendererDeviceHost
bool DkUpnpRendererDeviceHost::startDevicehost(QString pathToConfig) {
	//Herqq::Upnp::SetLoggingLevel(Herqq::Upnp::Debug);
	qDebug() << "starting DeviceHost";
	QFile f(pathToConfig);
	qDebug() << "pathToConfig:" << pathToConfig;
	if (!f.exists()) {
		qDebug() << "DkUpnpRendererDeviceHost: config file not found";
		return false;
	}

	QUuid uuid = QUuid::createUuid();
	QString uuidString = uuid.toString();
	uuidString.replace("{","");
	uuidString.replace("}","");
	QString newXMLpath = QDir::tempPath() + QDir::separator() + uuidString + ".xml";

	QByteArray fileData;
	f.open(QIODevice::ReadOnly);
	fileData = f.readAll();
	QString fileText(fileData);
	fileText.replace("insert-new-uuid-here", uuidString);
#ifdef WIN32
	fileText.replace("nomacs_avtransport_scpd.xml", QDir::temp().dirName()+"/nomacs_avtransport_scpd.xml");
	fileText.replace("nomacs_connectionmanager_sink_scpd.xml", QDir::temp().dirName()+"/nomacs_connectionmanager_sink_scpd.xml");
	fileText.replace("nomacs_renderingcontrol_scpd.xml", QDir::temp().dirName()+"/nomacs_renderingcontrol_scpd.xml");
#else
	fileText.replace("nomacs_avtransport_scpd.xml", QDir::temp().dirName()+"/nomacs_avtransport_scpd.xml");
	fileText.replace("nomacs_connectionmanager_sink_scpd.xml", QDir::temp().dirName()+"/nomacs_connectionmanager_sink_scpd.xml");
	fileText.replace("nomacs_renderingcontrol_scpd.xml", QDir::temp().dirName()+"/nomacs_renderingcontrol_scpd.xml");
#endif // WIN32

	f.seek(0);
	QFile newXMLfile(newXMLpath);
	newXMLfile.open(QIODevice::WriteOnly);
	newXMLfile.write(fileText.toUtf8());
	f.close();
	newXMLfile.close();

	QFileInfo fileInfo = QFileInfo(f);
	QFile avtransportXML(fileInfo.absolutePath() + QDir::separator() + "nomacs_avtransport_scpd.xml");
	QFile connectionManagerXML(fileInfo.absolutePath() + QDir::separator() + "nomacs_connectionmanager_sink_scpd.xml");
	QFile renderingcontrolXML(fileInfo.absolutePath() + QDir::separator() + "nomacs_renderingcontrol_scpd.xml");

	QString newAVtransportXML = QDir::tempPath()+ QDir::separator() + "nomacs_avtransport_scpd.xml";
	if (!QFile::exists(newAVtransportXML)) {
		if(!avtransportXML.copy(newAVtransportXML))
			qDebug() << "unable to copy nomacs_avtransport_scpd.xml";
	}
	QString newConnectionManagerXML = QDir::tempPath()+ QDir::separator() + "nomacs_connectionmanager_sink_scpd.xml";
	if (!QFile::exists(newConnectionManagerXML)) {
		if(!connectionManagerXML.copy(newConnectionManagerXML))
			qDebug() << "unable to copy nomacs_connectionmanager_sink_scpd.xml";
	}
	QString newRenderingControlXML = QDir::tempPath()+ QDir::separator() + "nomacs_renderingcontrol_scpd.xml";
	if (!QFile::exists(newRenderingControlXML)) {
		if(!renderingcontrolXML.copy(newRenderingControlXML))
			qDebug() << "unable to copy nomacs_renderingcontrol_scpd.xml";
	}


	Herqq::Upnp::Av::HMediaRendererDeviceConfiguration mediaRendererConfig;
	connectionManager = new DkUpnpRendererConnectionManager();
	connect(connectionManager, SIGNAL(newImage(QImage)), this, SLOT(cmNewImage(QImage)));
	mediaRendererConfig.setRendererConnectionManager(connectionManager, true);

	Herqq::Upnp::Av::HAvDeviceModelCreator avCreator;
	avCreator.setMediaRendererConfiguration(mediaRendererConfig);

	Herqq::Upnp::HDeviceConfiguration config;
	config.setPathToDeviceDescription(newXMLpath);

	Herqq::Upnp::HDeviceHostConfiguration hostConfig;
	hostConfig.setDeviceModelCreator(avCreator);
	hostConfig.add(config);


	bool retVal = init(hostConfig);
	if (!retVal) {
		qDebug() << "error while initializing device host:\n" << errorDescription();
	}

	QFile::remove(newAVtransportXML);
	QFile::remove(newConnectionManagerXML);
	QFile::remove(newRenderingControlXML);
	return retVal;
}


void DkUpnpRendererDeviceHost::stopDevicehost() {
	qDebug() << "DkUpnpRenderDeviceHost: stopping DeviceHost";
	quit();
}

DkUpnpRendererConnection::DkUpnpRendererConnection() {
}

qint32 DkUpnpRendererConnection::doSetResource(const QUrl &resourceUri, Herqq::Upnp::Av::HObject *cdsMetadata/* =0 */) {
	qDebug() << "doSetResource url:" << resourceUri;
	QNetworkRequest req(resourceUri);
	curResource = accessManager.get(req);
	bool ok = connect(curResource, SIGNAL(finished()), this, SLOT(finished()));
	
	return Herqq::Upnp::UpnpErrorCode::UpnpSuccess;
}

void DkUpnpRendererConnection::finished() {
	qDebug() << "finished";
	QByteArray currentData = curResource->readAll();
	QImage img;
	img.loadFromData(currentData);
	emit newImage(img);
}

}
