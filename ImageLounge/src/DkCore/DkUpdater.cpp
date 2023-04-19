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
 [1] https://nomacs.org/
 [2] https://github.com/nomacs/
 [3] http://download.nomacs.org
 *******************************************************************************************************/

#include "DkUpdater.h"

#include "DkSettings.h"
#include "DkTimer.h"
#include "DkUtils.h"

#pragma warning(push, 0) // no warnings from includes
#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QNetworkCookieJar>
#include <QNetworkProxyQuery>
#include <QProcess>
#include <QPushButton>
#include <QStandardPaths>
#include <QStringView>
#include <QVector>
#include <QXmlStreamReader>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#pragma warning(pop)

namespace nmc
{

// DkPackage --------------------------------------------------------------------
DkPackage::DkPackage(const QString &name, const QString &version)
{
    mName = name;
    mVersion = version;
}

bool DkPackage::isEmpty() const
{
    return mName.isEmpty();
}

bool DkPackage::operator==(const DkPackage &o) const
{
    return mName == o.name();
}

QString DkPackage::version() const
{
    return mVersion;
}

QString DkPackage::name() const
{
    return mName;
}

// DkXmlUpdateChecker --------------------------------------------------------------------
DkXmlUpdateChecker::DkXmlUpdateChecker()
{
}

QVector<DkPackage> DkXmlUpdateChecker::updatesAvailable(QXmlStreamReader &localXml, QXmlStreamReader &remoteXml) const
{
    QVector<DkPackage> localPackages = parse(localXml);
    QVector<DkPackage> remotePackages = parse(remoteXml);
    QVector<DkPackage> updatePackages;

    for (const DkPackage &p : localPackages) {
        int idx = remotePackages.indexOf(p);

        if (idx != -1) {
            bool isEqual = remotePackages[idx].version() == p.version();
            qDebug() << "checking" << p.name() << "v" << p.version();

            if (!isEqual) // we assume that the remote is _always_ newer than the local version
                updatePackages.append(remotePackages[idx]);
            else
                qDebug() << "up-to-date";
        } else
            qDebug() << "I could not find" << p.name() << "in the repository";
    }

    if (localPackages.empty() || remotePackages.empty())
        qDebug() << "WARNING: I could not find any packages. local (" << localPackages.size() << ") remote (" << remotePackages.size() << ")";

    return updatePackages;
}

QVector<DkPackage> DkXmlUpdateChecker::parse(QXmlStreamReader &reader) const
{
    QVector<DkPackage> packages;
    QString pName;

    while (!reader.atEnd()) {
        // e.g. <Name>nomacs</Name>
        if (reader.tokenType() == QXmlStreamReader::StartElement && reader.qualifiedName().toString() == "Name") {
            reader.readNext();
            pName = reader.text().toString();
        }
        // e.g. <Version>3.0.0-3</Version>
        else if (reader.tokenType() == QXmlStreamReader::StartElement && reader.qualifiedName().toString() == "Version") {
            reader.readNext();

            if (!pName.isEmpty()) {
                packages.append(DkPackage(pName, reader.text().toString()));
                pName = ""; // reset
            } else {
                qWarning() << "version: " << reader.text().toString() << "without a valid package name detected";
            }
        }

        reader.readNext();
    }

    return packages;
}

// DkUpdater  --------------------------------------------------------------------
DkUpdater::DkUpdater(QObject *parent)
    : QObject(parent)
{
    silent = true;
    mCookie = new QNetworkCookieJar(this);
    mAccessManagerSetup.setCookieJar(mCookie);
    connect(&mAccessManagerSetup, SIGNAL(finished(QNetworkReply *)), this, SLOT(downloadFinishedSlot(QNetworkReply *)));
    mUpdateAborted = false;
}

void DkUpdater::checkForUpdates()
{
    if (DkSettingsManager::param().sync().disableUpdateInteraction) {
        QMessageBox::critical(DkUtils::getMainWindow(),
                              tr("Updates Disabled"),
                              tr("nomacs updates are disabled.\nPlease contact your system administrator for further information."),
                              QMessageBox::Ok);
        return;
    }

    DkSettingsManager::param().sync().lastUpdateCheck = QDate::currentDate();
    DkSettingsManager::param().save();

#ifdef Q_OS_WIN
    QUrl url("http://nomacs.org/version/version_win_stable");
#elif defined Q_OS_LINUX
    QUrl url("http://nomacs.org/version/version_linux");
#elif defined Q_OS_MAC
    QUrl url("http://nomacs.org/version/version_mac_stable");
#else
    QUrl url("http://nomacs.org/version/version");
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
    connect(&mAccessManagerVersion, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyFinished(QNetworkReply *)));
    mReply = mAccessManagerVersion.get(QNetworkRequest(url));
    connect(mReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(replyError(QNetworkReply::NetworkError)));
}

void DkUpdater::replyFinished(QNetworkReply *reply)
{
    if (reply->error())
        return;

    QString replyData = reply->readAll();

    QStringList sl = replyData.split('\n', Qt::SkipEmptyParts);

    QString version, x64, x86, url, mac, XPx86;
    for (int i = 0; i < sl.length(); i++) {
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
    url = XPx86; // for WinXP packages
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
                emit showUpdaterMessage(tr("sorry, I could not check for newer versions"), tr("Updates"));

            return;
        }

        if (nVersion[0].toInt() > cVersion[0].toInt() || // major release
            (nVersion[0].toInt() == cVersion[0].toInt() && // major release
             nVersion[1].toInt() > cVersion[1].toInt())
            || // minor release
            (nVersion[0].toInt() == cVersion[0].toInt() && // major release
             nVersion[1].toInt() == cVersion[1].toInt() && // minor release
             nVersion[2].toInt() > cVersion[2].toInt())) { // minor-minor release

            QString msg = tr("A new version") + " (" + sl[0] + ") " + tr("is available");
            msg = msg + "<br>" + tr("Do you want to download and install it now?");
            msg = msg + "<br>" + tr("For more information see ") + " <a href=\"https://nomacs.org\">https://nomacs.org</a>";
            mNomacsSetupUrl = url;
            mSetupVersion = version;
            qDebug() << "nomacs setup url:" << mNomacsSetupUrl;

            if (!url.isEmpty())
                emit displayUpdateDialog(msg, tr("updates"));
        } else if (!silent)
            emit showUpdaterMessage(tr("nomacs is up-to-date"), tr("updates"));
    }
}

void DkUpdater::startDownload(QUrl downloadUrl)
{
    if (downloadUrl.isEmpty())
        emit showUpdaterMessage(tr("sorry, unable to download the new version"), tr("updates"));

    qDebug() << "-----------------------------------------------------";
    qDebug() << "starting to download update from " << downloadUrl;

    // updateAborted = false;	// reset - it may have been canceled before
    QNetworkRequest req(downloadUrl);
    req.setRawHeader("User-Agent", "Auto-Updater");
    mReply = mAccessManagerSetup.get(req);
    connect(mReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(updateDownloadProgress(qint64, qint64)));
}

void DkUpdater::downloadFinishedSlot(QNetworkReply *data)
{
    QUrl redirect = data->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (!redirect.isEmpty()) {
        qDebug() << "redirecting: " << redirect;
        startDownload(redirect);
        return;
    }

    if (!mUpdateAborted) {
        QString basename = "nomacs-setup";
        QString extension = ".msi";
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
            qDebug() << "Could not open " << QFileInfo(file).absoluteFilePath() << "for writing";
            return;
        }

        file.write(data->readAll());
        qDebug() << "saved new version: "
                 << " " << QFileInfo(file).absoluteFilePath();

        file.close();

        DkSettingsManager::param().global().setupVersion = mSetupVersion;
        DkSettingsManager::param().global().setupPath = absoluteFilePath;
        DkSettingsManager::param().save();

        emit downloadFinished(absoluteFilePath);
    }
    mUpdateAborted = false;
    qDebug() << "downloadFinishedSlot complete";
}

void DkUpdater::performUpdate()
{
    if (mNomacsSetupUrl.isEmpty())
        qDebug() << "unable to perform update because the nomacsSetupUrl is empty";
    else
        startDownload(mNomacsSetupUrl);
}

void DkUpdater::cancelUpdate()
{
    qDebug() << "abort update";
    mUpdateAborted = true;
    mReply->abort();
}

void DkUpdater::replyError(QNetworkReply::NetworkError)
{
    if (!silent)
        emit showUpdaterMessage(tr("Unable to connect to server ... please try again later"), tr("updates"));
}

// DkTranslationUpdater --------------------------------------------------------------------
DkTranslationUpdater::DkTranslationUpdater(bool silent, QObject *parent)
    : QObject(parent)
{
    this->silent = silent;
    connect(&mAccessManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyFinished(QNetworkReply *)));

    updateAborted = false;
    updateAbortedQt = false;
}

void DkTranslationUpdater::checkForUpdates()
{
    if (DkSettingsManager::param().sync().disableUpdateInteraction) {
        QMessageBox::critical(DkUtils::getMainWindow(),
                              tr("Updates Disabled"),
                              tr("nomacs updates are disabled.\nPlease contact your system administrator for further information."),
                              QMessageBox::Ok);
        return;
    }

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

    QUrl url("http://nomacs.org/translations/" + DkSettingsManager::param().global().language + "/nomacs_" + DkSettingsManager::param().global().language
             + ".qm");
    qInfo() << "checking for new translations at " << url;
    mReply = mAccessManager.get(QNetworkRequest(url));
    connect(mReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(updateDownloadProgress(qint64, qint64)));

    url = QUrl("http://nomacs.org/translations/qt/qt_" + DkSettingsManager::param().global().language + ".qm");
    qDebug() << "checking for new translations at " << url;
    mReplyQt = mAccessManager.get(QNetworkRequest(url));
    connect(mReplyQt, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(updateDownloadProgressQt(qint64, qint64)));
}

void DkTranslationUpdater::replyFinished(QNetworkReply *reply)
{
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

    QDir storageLocation = DkUtils::getTranslationPath();
    QString translationName =
        qtTranslation ? "qt_" + DkSettingsManager::param().global().language + ".qm" : "nomacs_" + DkSettingsManager::param().global().language + ".qm";

    if (isRemoteFileNewer(lastModifiedRemote, translationName)) {
        QString basename = qtTranslation ? "qt_" + DkSettingsManager::param().global().language : "nomacs_" + DkSettingsManager::param().global().language;
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
            qInfo() << "File already exists - overwriting";
        }

        QFile file(absoluteFilePath);
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "Could not open " << QFileInfo(file).absoluteFilePath() << "for writing";
            return;
        }

        file.write(reply->readAll());
        qDebug() << "saved new translation: "
                 << " " << QFileInfo(file).absoluteFilePath();

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

void DkTranslationUpdater::updateDownloadProgress(qint64 received, qint64 total)
{
    if (total == -1) // if file does not exist
        return;

    QDateTime lastModifiedRemote = mReply->header(QNetworkRequest::LastModifiedHeader).toDateTime();

    QString translationName = "nomacs_" + DkSettingsManager::param().global().language + ".qm";
    qDebug() << "isRemoteFileNewer:" << isRemoteFileNewer(lastModifiedRemote, translationName);
    if (!isRemoteFileNewer(lastModifiedRemote, translationName)) {
        updateAborted = true;
        this->mTotal = 0;
        this->mReceived = 0;
        mReply->abort();
        return;
    }

    this->mReceived = received;
    this->mTotal = total;
    qDebug() << "total:" << total;
    emit downloadProgress(this->mReceived + this->mReceivedQt, this->mTotal + this->mTotalQt);
}

void DkTranslationUpdater::updateDownloadProgressQt(qint64 received, qint64 total)
{
    if (total == -1) // if file does not exist
        return;

    QDateTime lastModifiedRemote = mReplyQt->header(QNetworkRequest::LastModifiedHeader).toDateTime();
    QString translationName = "qt_" + DkSettingsManager::param().global().language + ".qm";
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

bool DkTranslationUpdater::isRemoteFileNewer(QDateTime lastModifiedRemote, const QString &localTranslationName)
{
    if (!lastModifiedRemote.isValid())
        return false;

    QString trPath = DkUtils::getTranslationPath();
    QFileInfo trFile(trPath, localTranslationName);

    return !trFile.exists() || (QFileInfo(trFile).lastModified() < lastModifiedRemote);
}

void DkTranslationUpdater::cancelUpdate()
{
    mReply->abort();
    mReplyQt->abort();
    updateAborted = true;
    updateAbortedQt = true;
}

}
