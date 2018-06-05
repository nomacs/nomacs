#pragma once

#include <QtWidgets/QWidget>
#include <QPushButton>
#include <QtWidgets/qprogressbar.h>
#include <QtNetwork/QNetworkReply>
#include <QFile>
#include <QTime>

class QFile;

class QDownloadButton : public QWidget {
	Q_OBJECT

public:
	QDownloadButton(QWidget *parent = Q_NULLPTR);
	~QDownloadButton();
	void init(const QUrl& url, const QString& file);
	void setUrl(const QUrl& url);
	void setFile(const QString& file);

private:
	QPushButton *mBtn;
	QProgressBar *mProgressbar;
	QLayout* mLayout;
	QUrl mUrl;
	QNetworkReply* mReply;
	QTime downloadTime;
	QFile* mOutputfile;

signals:
	void downloadFinishedSignal();

private slots:
	void startDownload();
	
	void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
	void downloadFinished();
	void downloadReadyRead();
};