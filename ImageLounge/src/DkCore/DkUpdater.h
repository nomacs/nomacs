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

#pragma once

#pragma warning(push, 0) // no warnings from includes
#include <QNetworkReply>
#include <QObject>
#include <QString>
#pragma warning(pop)

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

// Qt defines
class QXmlStreamReader;
class QNetworkAccessManager;

namespace nmc
{

// nomacs defines
class DkPackage
{
public:
    DkPackage(const QString &name = "", const QString &version = "");

    bool isEmpty() const;
    bool operator==(const DkPackage &o) const;

    QString name() const;
    QString version() const;

protected:
    QString mName;
    QString mVersion;
};

class DkXmlUpdateChecker
{
public:
    DkXmlUpdateChecker();

    QVector<DkPackage> updatesAvailable(QXmlStreamReader &localXml, QXmlStreamReader &remoteXml) const;

protected:
    QVector<DkPackage> parse(QXmlStreamReader &reader) const;
};

class DllCoreExport DkUpdater : public QObject
{
    Q_OBJECT

public:
    bool silent;

    DkUpdater(QObject *parent = nullptr);

public slots:
    void checkForUpdates();
    void replyFinished(QNetworkReply *);
    void replyError(QNetworkReply::NetworkError);
    void performUpdate();
    void downloadFinishedSlot(QNetworkReply *data);
    void updateDownloadProgress(qint64 received, qint64 total)
    {
        emit downloadProgress(received, total);
    };
    void cancelUpdate();

signals:
    void displayUpdateDialog(const QString &msg, const QString &title) const;
    void showUpdaterMessage(const QString &msg, const QString &title) const;
    void downloadFinished(const QString &filePath) const;
    void downloadProgress(qint64, qint64) const;

protected:
    void startDownload(QUrl downloadUrl);

    QNetworkAccessManager mAccessManagerVersion;
    QNetworkAccessManager mAccessManagerSetup;

    QNetworkReply *mReply = nullptr;
    QNetworkCookieJar *mCookie = nullptr;

    QUrl mNomacsSetupUrl;
    QString mSetupVersion;
    bool mUpdateAborted = false;
};

class DllCoreExport DkTranslationUpdater : public QObject
{
    Q_OBJECT

public:
    DkTranslationUpdater(bool silent = false, QObject *parent = nullptr);
    bool silent;

public slots:
    virtual void checkForUpdates();
    virtual void replyFinished(QNetworkReply *);
    void updateDownloadProgress(qint64 received, qint64 total);
    void updateDownloadProgressQt(qint64 received, qint64 total);
    void cancelUpdate();

signals:
    void translationUpdated();
    void showUpdaterMessage(const QString &, const QString &);
    void downloadProgress(qint64, qint64);
    void downloadFinished();

private:
    bool isRemoteFileNewer(QDateTime lastModifiedRemote, const QString &localTranslationName);
    bool updateAborted, updateAbortedQt;

    qint64 mTotal, mTotalQt, mReceived, mReceivedQt;
    QNetworkAccessManager mAccessManager;
    QNetworkReply *mReply = nullptr;
    QNetworkReply *mReplyQt = nullptr;
};

}