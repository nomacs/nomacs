/*******************************************************************************************************
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2016 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2016 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2016 Florian Kleber <florian@nomacs.org>

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

 related links:
 [1] http://www.nomacs.org/
 [2] https://github.com/nomacs/
 [3] http://download.nomacs.org
 *******************************************************************************************************/

#include "DkUpdater.h"

#include "DkSettings.h"
#include "DkTimer.h"
#include "DkUtils.h"

#pragma warning(push, 0)	// no warnings from includes
#include <QVector>
#include <QDebug>
#include <QXmlStreamReader>
#include <QNetworkProxyQuery>
#include <QFile>
#include <QCoreApplication>
#include <QMessageBox>
#include <QApplication>
#include <QPushButton>
#include <QFileInfo>
#include <QNetworkCookieJar>
#include <QDir>
#include <QProcess>
#include <QStandardPaths>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#pragma warning(pop)

namespace nmc {

// DkPackage --------------------------------------------------------------------
DkPackage::DkPackage(const QString& name, const QString& version) {
	mName = name;
	mVersion = version;
}

bool DkPackage::isEmpty() const {
	return mName.isEmpty();
}

bool DkPackage::operator==(const DkPackage& o) const {

	return mName == o.name();
}

QString DkPackage::version() const {
	return mVersion;
}

QString DkPackage::name() const {
	return mName;
}

// DkXmlUpdateChecker --------------------------------------------------------------------
DkXmlUpdateChecker::DkXmlUpdateChecker() {
}

QVector<DkPackage> DkXmlUpdateChecker::updatesAvailable(QXmlStreamReader& localXml, QXmlStreamReader& remoteXml) const {

	QVector<DkPackage> localPackages = parse(localXml);
	QVector<DkPackage> remotePackages = parse(remoteXml);
	QVector<DkPackage> updatePackages;

	for (const DkPackage& p : localPackages) {

		int idx = remotePackages.indexOf(p);

		if (idx != -1) {
			bool isEqual = remotePackages[idx].version() == p.version();
			qDebug() << "checking" << p.name() << "v" << p.version();

			if (!isEqual)	// we assume that the remote is _always_ newer than the local version
				updatePackages.append(remotePackages[idx]);
			else
				qDebug() << "up-to-date";
		}
		else
			qDebug() << "I could not find" << p.name() << "in the repository";
	}

	if (localPackages.empty() || remotePackages.empty())
		qDebug() << "WARNING: I could not find any packages. local (" << localPackages.size() << ") remote (" << remotePackages.size() << ")";

	return updatePackages;
}

QVector<DkPackage> DkXmlUpdateChecker::parse(QXmlStreamReader& reader) const {

	QVector<DkPackage> packages;
	QString pName;

	while (!reader.atEnd()) {

		// e.g. <Name>nomacs</Name>
		if (reader.tokenType() == QXmlStreamReader::StartElement && reader.qualifiedName() == "Name") {
			reader.readNext();
			pName = reader.text().toString();
		}
		// e.g. <Version>3.0.0-3</Version>
		else if (reader.tokenType() == QXmlStreamReader::StartElement && reader.qualifiedName() == "Version") {
			reader.readNext();

			if (!pName.isEmpty()) {
				packages.append(DkPackage(pName, reader.text().toString()));
				pName = "";	// reset
			}
			else {
				qWarning() << "version: " << reader.text().toString() << "without a valid package name detected";
			}
		}

		reader.readNext();
	}

	return packages;
}


// DkInstallUpdater --------------------------------------------------------------------
DkInstallUpdater::DkInstallUpdater(QObject* parent) : QObject(parent) {

}

void DkInstallUpdater::checkForUpdates(bool silent) {

	mSilent = silent;

	Settings::param().sync().lastUpdateCheck = QDate::currentDate();
	Settings::param().save();

	QUrl url ("http://download.nomacs.org/repository/Updates.xml");

	// this is crucial since every item creates it's own http thread
	if (!mManager) {
		mManager = new QNetworkAccessManager(this);
		connect(mManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
	}

	// the proxy settings take > 2 sec on Win7
	// that is why proxy settings are only set
	// for manual updates
	if (!silent) {
		DkTimer dt;
		QNetworkProxyQuery npq(url);
		QList<QNetworkProxy> listOfProxies = QNetworkProxyFactory::systemProxyForQuery(npq);

		if (!listOfProxies.empty() && listOfProxies[0].hostName() != "") {
			mManager->setProxy(listOfProxies[0]);
		}
		qDebug() << "checking for proxy takes: " << dt;
	}

	mManager->get(QNetworkRequest(url));
	qDebug() << "checking updates at: " << url;

}

void DkInstallUpdater::replyFinished(QNetworkReply* reply) {

	QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
	qDebug() << "--------------------------";

	if (!redirect.isEmpty()) {
		//qDebug() << "redirecting: " << redirect;
		checkForUpdates(false);
		reply->close();
		return;
	}

	if (!reply->isFinished()) {
		qDebug() << "reply not finished...";
		reply->close();		
		return;
	}

	if (reply->error() != QNetworkReply::NoError) {
		qDebug() << "could not check for updates: " << reply->errorString();
		reply->close();
		return;
	}

	QFile componentsXml(QCoreApplication::applicationDirPath() + "/../components.xml");

	if (!componentsXml.exists()) {
		qDebug() << "Sorry, " << componentsXml.fileName() << "does not exist";
		return;
	}

	componentsXml.open(QIODevice::ReadOnly);
	QXmlStreamReader localReader(componentsXml.readAll());
	QXmlStreamReader remoteReader(reply);

	DkXmlUpdateChecker checker;
	QVector<DkPackage> newPackages = checker.updatesAvailable(localReader, remoteReader);
	if (!newPackages.empty()) {

		bool update = true;

		if (mSilent) {	// ask user before updating if a silent check was performed
			QString msg = tr("There are new packages available: ") + "\n";

			for (const DkPackage& p : newPackages)
				msg += "\t" + p.name() + " " + p.version() + "\n";

			QMessageBox* msgBox = new QMessageBox(
				QMessageBox::Information, 
				tr("Updates Available"), 
				msg, 
				QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
				QApplication::activeWindow());

			msgBox->button(QMessageBox::Yes)->setText(tr("&Upgrade"));
			msgBox->button(QMessageBox::Cancel)->setText(tr("Remind Me &Later"));
			msgBox->button(QMessageBox::No)->setText(tr("&Skip this Version"));

			int result = msgBox->exec();

			update = result == QMessageBox::Accepted || result == QMessageBox::Yes;

			if (result == QMessageBox::No)	// do not show again
				Settings::param().sync().updateDialogShown = true;	

			msgBox->deleteLater();
		}

		if (update)
			updateNomacs("--updater");
	}
	else if (!mSilent) {
		QMessageBox::information(QApplication::activeWindow(), tr("nomacs Updates"), tr("nomacs is up-to-date"));
	}
	else
		qDebug() << "nomacs is up-to-date...";
	qDebug() << "--------------------------";
}

bool DkInstallUpdater::updateNomacs(const QString& cmd) {

	QFileInfo updater(QCoreApplication::applicationDirPath() + "/../maintenancetool.exe");

	if (!updater.exists()) {
		qDebug() << "Sorry, " << updater.absoluteFilePath() << "does not exist";
		return false;
	}

#ifdef Q_OS_WIN

	// diem: 14.12.2015 - NOTE we need this win API command only to fix a qt installer bug: https://bugreports.qt.io/browse/QTIFW-746
	// hence after updating the installer (> 2.0.1) we can safely fall back to the Qt cmd....
	std::wstring upath = DkUtils::qStringToStdWString(updater.absoluteFilePath());
	std::wstring cmdW = DkUtils::qStringToStdWString(cmd);

	HINSTANCE h = ShellExecuteW( NULL, 
		L"runas",  
		upath.data(),  
		cmdW.data(),     
		NULL,                        // default dir 
		SW_SHOWNORMAL  
		); 

	return h != NULL;
#else
	QStringList args;
	args << cmd;

	return QProcess::startDetached(updater.absoluteFilePath(), args);
#endif
}

// DkUpdater  --------------------------------------------------------------------

DkUpdater::DkUpdater(QObject* parent) : QObject(parent) {

	silent = true;
	mCookie = new QNetworkCookieJar(this);
	mAccessManagerSetup.setCookieJar(mCookie);
	connect(&mAccessManagerSetup, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinishedSlot(QNetworkReply*)));
	mUpdateAborted = false;
}

void DkUpdater::checkForUpdates() {

	Settings::param().sync().lastUpdateCheck = QDate::currentDate();
	Settings::param().save();

#ifdef Q_OS_WIN
	QUrl url ("http://www.nomacs.org/version_win_stable");
#elif defined Q_OS_LINUX
	QUrl url ("http://www.nomacs.org/version_linux");
#elif defined Q_OS_MAC
	QUrl url ("http://www.nomacs.org/version_mac_stable");
#else
	QUrl url ("http://www.nomacs.org/version");
#endif

	// the proxy settings take > 2 sec on Win7
	// that is why proxy settings are only set
	// for manual updates
	if (!silent) {
		DkTimer dt;
		QNetworkProxyQuery npq(QUrl("http://www.google.com"));
		QList<QNetworkProxy> listOfProxies = QNetworkProxyFactory::systemProxyForQuery(npq);

		if (!listOfProxies.empty() && listOfProxies[0].hostName() != "") {
			mAccessManagerSetup.setProxy(listOfProxies[0]);
			mAccessManagerVersion.setProxy(listOfProxies[0]);
		}
		qDebug() << "checking for proxy takes: " << dt;
	}

	qDebug() << "checking for updates";
	connect(&mAccessManagerVersion, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));	
	QNetworkRequest request = QNetworkRequest(url);
	request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
	mReply = mAccessManagerVersion.get(QNetworkRequest(url));
	connect(mReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(replyError(QNetworkReply::NetworkError)));
}

void DkUpdater::replyFinished(QNetworkReply* reply) {

	if (reply->error())
		return;

	QString replyData = reply->readAll();

	QStringList sl = replyData.split('\n', QString::SkipEmptyParts);

	QString version, x64, x86, url, mac, XPx86;
	for(int i = 0; i < sl.length();i++) {
		QStringList values = sl[i].split(" ");
		if (values[0] == "version") 
			version = values[1];
		else if (values[0] == "x64")
			x64 = values[1];
		else if (values[0] == "XPx86")
			XPx86 = values[1];
		else if (values[0] == "x86")
			x86 = values[1];
		else if (values[0] == "mac")
			mac = values[1];
	}


#if _MSC_VER == 1600
	url = XPx86;	// for WinXP packages
#elif defined _WIN64
	url = x64;
#elif _WIN32
	url = x86;
#elif defined Q_OS_MAC
	url = mac;
#endif 

	qDebug() << "version:" << version;
	qDebug() << "x64:" << x64;
	qDebug() << "x86:" << x86;
	qDebug() << "mac:" << mac;

	if ((!version.isEmpty() && !x64.isEmpty()) || !x86.isEmpty()) {		
		QStringList cVersion = QApplication::applicationVersion().split('.');
		QStringList nVersion = version.split('.');

		if (cVersion.size() < 3 || nVersion.size() < 3) {
			qDebug() << "sorry, I could not parse the version number...";

			if (!silent)
				emit showUpdaterMessage(tr("sorry, I could not check for newer versions"), tr("updates"));

			return;
		}

		if (nVersion[0].toInt() > cVersion[0].toInt()  ||	// major release
			(nVersion[0].toInt() == cVersion[0].toInt() &&	// major release
				nVersion[1].toInt() > cVersion[1].toInt())  ||	// minor release
			(nVersion[0].toInt() == cVersion[0].toInt() &&	// major release
				nVersion[1].toInt() == cVersion[1].toInt()) &&	// minor release
			nVersion[2].toInt() >  cVersion[2].toInt()) {	// minor-minor release

			QString msg = tr("A new version") + " (" + sl[0] + ") " + tr("is available");
			msg = msg + "<br>" + tr("Do you want to download and install it now?");
			msg = msg + "<br>" + tr("For more information see ") + " <a href=\"http://www.nomacs.org\">http://www.nomacs.org</a>";
			mNomacsSetupUrl = url;
			mSetupVersion = version;
			qDebug() << "nomacs setup url:" << mNomacsSetupUrl;

			if (!url.isEmpty())
				emit displayUpdateDialog(msg, tr("updates")); 
		}
		else if (!silent)
			emit showUpdaterMessage(tr("nomacs is up-to-date"), tr("updates"));
	}

}

void DkUpdater::startDownload(QUrl downloadUrl) {

	if (downloadUrl.isEmpty())
		emit showUpdaterMessage(tr("sorry, unable to download the new version"), tr("updates"));

	qDebug() << "-----------------------------------------------------";
	qDebug() << "starting to download update from " << downloadUrl ;

	//updateAborted = false;	// reset - it may have been canceled before
	QNetworkRequest req(downloadUrl);
	req.setRawHeader("User-Agent", "Auto-Updater");
	mReply = mAccessManagerSetup.get(req);
	connect(mReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(updateDownloadProgress(qint64, qint64)));
}

void DkUpdater::downloadFinishedSlot(QNetworkReply* data) {
	QUrl redirect = data->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
	if (!redirect.isEmpty() ) {
		qDebug() << "redirecting: " << redirect;
		startDownload(redirect);
		return;
	}

	if (!mUpdateAborted) {
		QString basename = "nomacs-setup";
		QString extension = ".exe";
		QString absoluteFilePath = QDir::tempPath() + "/" + basename + extension;
		if (QFile::exists(absoluteFilePath)) {
			qDebug() << "File already exists - searching for new name";
			// already exists, don't overwrite
			int i = 0;
			while (QFile::exists(absoluteFilePath)) {
				absoluteFilePath = QDir::tempPath() + "/" + basename + "-" + QString::number(i) + extension;
				++i;
			}
		}

		QFile file(absoluteFilePath);
		if (!file.open(QIODevice::WriteOnly)) {
			qDebug()  << "Could not open " << QFileInfo(file).absoluteFilePath() << "for writing";
			return;
		}

		file.write(data->readAll());
		qDebug() << "saved new version: " << " " << QFileInfo(file).absoluteFilePath();

		file.close();

		Settings::param().global().setupVersion = mSetupVersion;
		Settings::param().global().setupPath = absoluteFilePath;
		Settings::param().save();

		emit downloadFinished(absoluteFilePath);
	}
	mUpdateAborted = false;
	qDebug() << "downloadFinishedSlot complete";
}

void DkUpdater::performUpdate() {
	if(mNomacsSetupUrl.isEmpty())
		qDebug() << "unable to perform update because the nomacsSetupUrl is empty";
	else
		startDownload(mNomacsSetupUrl);
}

void DkUpdater::cancelUpdate()  {
	qDebug() << "abort update";
	mUpdateAborted = true; 
	mReply->abort(); 
}

void DkUpdater::replyError(QNetworkReply::NetworkError) {
	if (!silent)
		emit showUpdaterMessage(tr("Unable to connect to server ... please try again later"), tr("updates"));
}

// DkTranslationUpdater --------------------------------------------------------------------
DkTranslationUpdater::DkTranslationUpdater(bool silent, QObject* parent) : QObject(parent) {

	this->silent = silent;
	connect(&mAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

	updateAborted = false;
	updateAbortedQt = false;
}

void DkTranslationUpdater::checkForUpdates() {

	mTotal = -1;
	mTotalQt = -1;
	mReceived = 0;
	mReceivedQt = 0;
	updateAborted = false;
	updateAbortedQt = false;

	// that line takes 2 secs on win7!
	QNetworkProxyQuery npq(QUrl("http://www.google.com"));
	QList<QNetworkProxy> listOfProxies = QNetworkProxyFactory::systemProxyForQuery(npq);
	if (!listOfProxies.empty() && listOfProxies[0].hostName() != "") {
		mAccessManager.setProxy(listOfProxies[0]);
	}

	QUrl url ("http://www.nomacs.org/translations/" + Settings::param().global().language + "/nomacs_" + Settings::param().global().language + ".qm");
	qDebug() << "checking for new translations at " << url;
	QNetworkRequest request = QNetworkRequest(url);
	request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
	mReply = mAccessManager.get(QNetworkRequest(url));
	connect(mReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(updateDownloadProgress(qint64, qint64)));

	url=QUrl("http://www.nomacs.org/translations/qt/qt_" + Settings::param().global().language + ".qm");
	qDebug() << "checking for new translations at " << url;
	request = QNetworkRequest(url);
	request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
	mReplyQt = mAccessManager.get(QNetworkRequest(url));
	connect(mReplyQt, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(updateDownloadProgressQt(qint64, qint64)));
}

void DkTranslationUpdater::replyFinished(QNetworkReply* reply) {

	bool qtTranslation = false;
	if (reply->url().toString().contains("qt_"))
		qtTranslation = true;

	if (updateAbortedQt && updateAborted) {
		emit downloadFinished();
		return;
	}

	if (reply->error() == QNetworkReply::OperationCanceledError)
		return;

	if (reply->error()) {
		qDebug() << "network reply error : url: " << reply->url();
		if (!qtTranslation && !silent)
			emit showUpdaterMessage(tr("Unable to download translation"), tr("update")); 
		return;
	}

	QDateTime lastModifiedRemote = reply->header(QNetworkRequest::LastModifiedHeader).toDateTime();


#ifdef  Q_OS_WIN
	QDir storageLocation;
	if (Settings::param().isPortable()) {
		storageLocation = QDir(QCoreApplication::applicationDirPath());
		storageLocation.cd("translations");
	}
	else
		storageLocation = QDir(QDir::home().absolutePath() + "/AppData/Roaming/nomacs/translations");
#else
#if QT_VERSION >= 0x050000
	QDir storageLocation(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/data/" + QCoreApplication::organizationName() + "/" + QCoreApplication::applicationName());
#else
	QDir storageLocation(QDesktopServices::storageLocation(QDesktopServices::DataLocation)+"/translations/");
#endif

#endif //  Q_OS_WIN

	QString translationName = qtTranslation ? "qt_"+ Settings::param().global().language + ".qm" : "nomacs_"+ Settings::param().global().language + ".qm";

	if (isRemoteFileNewer(lastModifiedRemote, translationName)) {
		QString basename = qtTranslation ? "qt_" + Settings::param().global().language : "nomacs_" + Settings::param().global().language;
		QString extension = ".qm";

		if (!storageLocation.exists()) {
			if (!storageLocation.mkpath(storageLocation.absolutePath())) {
				qDebug() << "unable to create storage location ... aborting";
				if (!qtTranslation && !silent)
					emit showUpdaterMessage(tr("Unable to update translation"), tr("update")); 
				return;
			}
		}

		QString absoluteFilePath = storageLocation.absolutePath() + "/" + basename + extension;
		if (QFile::exists(absoluteFilePath)) {
			qDebug() << "File already exists - overwriting";
		}

		QFile file(absoluteFilePath);
		if (!file.open(QIODevice::WriteOnly)) {
			qDebug()  << "Could not open " << QFileInfo(file).absoluteFilePath() << "for writing";
			return;
		}

		file.write(reply->readAll());
		qDebug() << "saved new translation: " << " " << QFileInfo(file).absoluteFilePath();

		file.close();

		if (!qtTranslation && !silent)
			emit showUpdaterMessage(tr("Translation updated"), tr("update")); 
		qDebug() << "translation updated";
	} else {
		qDebug() << "no newer translations available";
		if (!silent)
			emit showUpdaterMessage(tr("No newer translations found"), tr("update")); 
	}
	if (reply->isFinished() && mReplyQt->isFinished()) {
		qDebug() << "emitting downloadFinished";
		emit downloadFinished();
	}

}

void DkTranslationUpdater::updateDownloadProgress(qint64 received, qint64 total) {
	if (total == -1)  // if file does not exist 
		return;

	QDateTime lastModifiedRemote = mReply->header(QNetworkRequest::LastModifiedHeader).toDateTime();
	QString translationName = "nomacs_"+ Settings::param().global().language + ".qm";
	qDebug() << "isRemoteFileNewer:" << isRemoteFileNewer(lastModifiedRemote, translationName);
	if (!isRemoteFileNewer(lastModifiedRemote, translationName)) {
		updateAborted = true;
		this->mTotal = 0;
		this->mReceived = 0;
		mReply->abort();
		return;
	}

	this->mReceived = received;
	this->mTotal  = total;
	qDebug() << "total:" << total;
	emit downloadProgress(this->mReceived + this->mReceivedQt, this->mTotal + this->mTotalQt); 
}

void DkTranslationUpdater::updateDownloadProgressQt(qint64 received, qint64 total) {
	if (total == -1)  // if file does not exist 
		return;

	QDateTime lastModifiedRemote = mReplyQt->header(QNetworkRequest::LastModifiedHeader).toDateTime();
	QString translationName = "qt_"+ Settings::param().global().language + ".qm";
	qDebug() << "isRemoteFileNewer:" << isRemoteFileNewer(lastModifiedRemote, translationName);
	if (!isRemoteFileNewer(lastModifiedRemote, translationName)) {
		updateAbortedQt = true;
		this->mTotalQt = 0;
		this->mReceivedQt = 0;
		mReplyQt->abort();
		return;
	}

	this->mReceivedQt = received;
	this->mTotalQt = total;
	qDebug() << "totalQt:" << mTotalQt;
	emit downloadProgress(this->mReceived + this->mReceivedQt, this->mTotal + this->mTotalQt); 
}

bool DkTranslationUpdater::isRemoteFileNewer(QDateTime lastModifiedRemote, const QString& localTranslationName) {
	if (!lastModifiedRemote.isValid())
		return false;

#ifdef  Q_OS_WIN
	QDir storageLocation;
	if (Settings::param().isPortable()) {
		storageLocation = QDir(QCoreApplication::applicationDirPath());
		storageLocation.cd("translations");
	}
	else
		storageLocation = QDir(QDir::home().absolutePath() + "/AppData/Roaming/nomacs/translations");
#else
#if QT_VERSION >= 0x050000
	QDir storageLocation(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/data/" + QCoreApplication::organizationName() + "/" + QCoreApplication::applicationName());
#else
	QDir storageLocation(QDesktopServices::storageLocation(QDesktopServices::DataLocation)+"/translations/");
#endif

#endif //  Q_OS_WIN
	QFile userTranslation(storageLocation.absoluteFilePath(localTranslationName));
	//qDebug() << "local: " << QFileInfo(userTranslation).lastModified()  << "  remote: " << lastModifiedRemote << " bool: " << (QFileInfo(userTranslation).lastModified() < lastModifiedRemote);
	//qDebug() << "userTranslation exists:" << userTranslation.exists();
	return !userTranslation.exists() || (QFileInfo(userTranslation).lastModified() < lastModifiedRemote);
}

void DkTranslationUpdater::cancelUpdate() {
	mReply->abort(); 
	mReplyQt->abort();
	updateAborted = true;
	updateAbortedQt = true;
}

}