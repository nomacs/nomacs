#pragma once

#include <QDialog>
#include <QLabel>
#include <QtNetwork/QNetworkReply>
#include <QtWidgets/QTableWidget>

class TesseractSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	TesseractSettingsDialog(QWidget *parent = Q_NULLPTR, const QStringList& selectedLanguages = {});
	~TesseractSettingsDialog();
	void init();

	void closeEvent(QCloseEvent *event) override;

private:
	QPushButton *findButton;
	QPushButton *moreButton;
	QWidget *extension;
	QNetworkReply* reply;
	QList<QString> mLanguageFileList;
	QTableWidget* mLangTable;

signals:
	void closeSignal(QStringList);

private slots:
	void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
	void downloadFinished();
	void downloadReadyRead();
	void sslErrorOccured(QNetworkReply*, QList<QSslError>);
	void test();
};