#include "QDownloadButton.h"

#include <QBoxLayout>
#include <QDebug>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#include <QtGui/qguiapplication.h>

QDownloadButton::QDownloadButton(QWidget * parent) : QWidget(parent) {

}

void QDownloadButton::init(const QUrl& url, const QString& file) {
	setUrl(url);
	setFile(file);

	if (QFile::exists(file))
	{
		qDebug() << file << " already exists";
		emit downloadFinishedSignal();
	}

	mLayout = new QBoxLayout(QBoxLayout::LeftToRight);
	mLayout->setMargin(0);
	mBtn = new QPushButton("download");

	mLayout->addWidget(mBtn);
	setLayout(mLayout);

	connect(mBtn, SIGNAL(clicked()), this, SLOT(startDownload()));
}

void QDownloadButton::setUrl(const QUrl& url) {
	mUrl = url;
}

void QDownloadButton::setFile(const QString& file) {
	mOutputfile = new QFile(file);
}

void QDownloadButton::startDownload() {
	qDebug() << "start Download";

	//mLayout->removeWidget(mBtn);
	//mLayout->takeAt(0);
	mBtn->setVisible(false);

	mProgressbar = new QProgressBar(this);
	mProgressbar->setTextVisible(false);
	//mProgressbar->setFormat("test");
	mLayout->addWidget(mProgressbar);

	if (!mOutputfile->open(QIODevice::WriteOnly))
	{
		qDebug() << "file error";
	}

	QNetworkRequest request(mUrl);
	auto* nam = new QNetworkAccessManager();
	mReply = nam->get(request);
	connect(mReply, SIGNAL(downloadProgress(qint64, qint64)), SLOT(downloadProgress(qint64, qint64)));
	connect(mReply, SIGNAL(finished()), SLOT(downloadFinished()));
	connect(mReply, SIGNAL(readyRead()), SLOT(downloadReadyRead()));

	downloadTime.start();
	mLayout->update();
}

void QDownloadButton::downloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
	
	if (!bytesTotal) {
		qWarning() << "total bytes is zero...";
		return;
	}
	
	qDebug() << "bytes received/total" << bytesReceived << " " << bytesTotal;

	double speed = bytesReceived * 1000.0 / downloadTime.elapsed();
	QString unit;
	if (speed < 1024) {
		unit = "bytes/sec";
	}
	else if (speed < 1024 * 1024) {
		speed /= 1024;
		unit = "kB/s";
	}
	else {
		speed /= 1024 * 1024;
		unit = "MB/s";
	}

	mProgressbar->setValue(bytesReceived * 100 / bytesTotal);
	//mProgressbar->setFormat(QString::fromLatin1("%1 %2").arg(speed, 3, 'f', 1).arg(unit));
	mProgressbar->update();
	//qApp->processEvents();
}

void QDownloadButton::downloadFinished() {
	if (mReply->error())
		qDebug() << mReply->errorString();

	qDebug() << "finished";

	mOutputfile->close();
	emit downloadFinishedSignal();
}

void QDownloadButton::downloadReadyRead() {
	qDebug() << "finished";
	mOutputfile->write(mReply->readAll());
}

QDownloadButton::~QDownloadButton() {
	/*delete mOutputfile;
	delete mProgressbar;
	delete mBtn;
	delete mLayout;
	delete mReply;*/
}