#include "tesseractsettingsdialog.h"

#include <QGridLayout>
#include <QTableWidget>

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QtCore/qcoreapplication.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>
#include "downloadmanager.h"
#include <QPushButton>
#include <QHeaderView>
#include "QDownloadButton.h"
#include <QCheckBox>
#include <complex>

/*namespace
{
	QJsonDocument httpgetjson(const QUrl& url)
	{
		
	//	request.redi
		//request.setHeader(QNetworkRequest::ContentTypeHeader, "application / json");

		QNetworkAccessManager nam;
		QNetworkReply *reply = nam.get(request);
		while (!reply->isFinished())
			qApp->processEvents();
		
		QByteArray response_data = reply->readAll();
		qDebug() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
		qDebug() << response_data;
		

		QJsonDocument json = QJsonDocument::fromJson(response_data);
		reply->deleteLater();

		return json;
	}
}*/

TesseractSettingsDialog::TesseractSettingsDialog(QWidget * parent, const QStringList& selectedLanguages) : QDialog(parent) {
	mLanguageFileList = { "afr", "amh", "ara", "asm", "aze", "yrl",
		"bel", "ben", "bod", "bos", "bre", "bul", "cat",
		"ceb", "ces", "sim", "ert", "tra", "ert", "chr",
		"cos", "cym", "dan", "rak", "deu", "rak", "div",
		"dzo", "ell", "eng", "enm", "epo", "equ", "est",
		"eus", "fao", "fas", "fil", "fin", "fra", "frk",
		"frm", "fry", "gla", "gle", "glg", "grc", "guj",
		"hat", "heb", "hin", "hrv", "hun", "hye", "iku",
		"ind", "isl", "ita", "old", "jav", "jpn", "ert",
		"kan", "kat", "old", "kaz", "khm", "kir", "kor",
		"ert", "kur", "ara", "lao", "lat", "lav", "lit",
		"ltz", "mal", "mar", "mkd", "mlt", "mon", "mri",
		"msa", "mya", "nep", "nld", "nor", "oci", "ori",
		"osd", "pan", "pol", "por", "pus", "que", "ron",
		"rus", "san", "sin", "slk", "rak", "slv", "snd",
		"spa", "old", "sqi", "srp", "atn", "sun", "swa",
		"swe", "syr", "tam", "tat", "tel", "tgk", "tgl",
		"tha", "tir", "ton", "tur", "uig", "ukr", "urd",
		"uzb", "yrl", "vie", "yid", "yor" };

	const QString baseUrl = "https://raw.githubusercontent.com/tesseract-ocr/tessdata/master/";
	const QString dataPath = (QCoreApplication::applicationDirPath() + "/plugins/tessdata/");

	auto* label = new QLabel(tr("Available Languages:"));

	auto* mainLayout = new QVBoxLayout();
	mainLayout->addWidget(label);

	mLangTable = new QTableWidget();
	mLangTable->setColumnCount(3);
	mLangTable->setRowCount(mLanguageFileList.length());
	mLangTable->horizontalHeader()->hide();
	mLangTable->verticalHeader()->hide();
	mLangTable->setSelectionMode(QTableWidget::SelectionMode::NoSelection);

	{
		int i = 0;
		for (auto it = mLanguageFileList.begin(); it != mLanguageFileList.end(); ++it, ++i)
		{
			auto* model = mLangTable->model();
			mLangTable->setItem(i, 0, new QTableWidgetItem(*it));

			QDownloadButton* btn = new QDownloadButton();
			QCheckBox* cbx = new QCheckBox();
			cbx->setDisabled(true);
			mLangTable->setIndexWidget(model->index(i, 2), cbx);

			connect(btn, &QDownloadButton::downloadFinishedSignal, this, [this, cbx]() {
				cbx->setDisabled(false);
				//cbx->setChecked(true);
			});

			if (selectedLanguages.contains(*it))
				cbx->setChecked(true);

			btn->init(QUrl(baseUrl + *it + ".traineddata"), dataPath + *it + ".traineddata");
			mLangTable->setIndexWidget(model->index(i, 1), btn);
		}
	}

	mainLayout->addWidget(mLangTable);
	setLayout(mainLayout);

	// github.com -> SSL HANDSHAKE ERROR :(
	/*auto json = ::httpgetjson(QUrl("https://api.github.com/repos/tesseract-ocr/tessdata/contents")).array();
	{
	for(auto it = json.begin(); it != json.end(); ++it)
	{
	auto e = (*it).toObject();
	qDebug() << e.value("name");
	}
	}*/

	//QNetworkRequest request(QUrl("https://api.github.com/repos/tesseract-ocr/tessdata/contents"));

	/*QNetworkRequest request(QUrl("https://raw.githubusercontent.com/tesseract-ocr/tessdata/master/eng"));
	request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, QVariant(true));
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QVariant(QNetworkRequest::NoLessSafeRedirectPolicy));*/

	/*QSslConfiguration sslConfiguration = request.sslConfiguration();
	//sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
	//sslConfiguration.setProtocol(QSsl::AnyProtocol);
	QList<QSslCertificate> caList = sslConfiguration.caCertificates();
	caList.append(QSslCertificate("cacert.pem")); //Root CA
	sslConfiguration.setCaCertificates(caList);
	request.setSslConfiguration(sslConfiguration); */

	/*auto* nam = new QNetworkAccessManager();
	reply = nam->get(request);
	connect(reply, SIGNAL(downloadProgress(qint64, qint64)), SLOT(downloadProgress(qint64, qint64)));
	connect(reply, SIGNAL(finished()), SLOT(downloadFinished()));
	connect(reply, SIGNAL(readyRead()), SLOT(downloadReadyRead()));
	connect(reply, SIGNAL(sslErrors(QNetworkReply*, QList<QSslError>)), this, SLOT(sslErrorOccured(QNetworkReply*, QList<QSslError>)));*/


	//DownloadManager* dm = new DownloadManager(this);
	//dm->append(QUrl("https://api.github.com/repos/tesseract-ocr/tessdata/contents"));

	//QNetworkAccessManager nam;
	//connect(&nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinishedSlot(QNetworkReply*)));
	
}

void TesseractSettingsDialog::init()
{
	
}

void TesseractSettingsDialog::test()
{
	qDebug() << "test";
}

void TesseractSettingsDialog::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	qDebug() << bytesReceived << " " << bytesTotal;
}

void TesseractSettingsDialog::downloadFinished()
{
	if (reply->error())
		qDebug() << reply->errorString();

	const QList<QSslError> errors;
	reply->sslErrors(errors);

	qDebug() << "finished";
}

void TesseractSettingsDialog::downloadReadyRead()
{
	qDebug() << "finished";
}

void TesseractSettingsDialog::sslErrorOccured(QNetworkReply*, QList<QSslError>)
{
	qDebug() << "sslerrors";
}

void TesseractSettingsDialog::closeEvent(QCloseEvent *event)
{
	QStringList selectedLanguages;

	auto* model = mLangTable->model();

	int i = 0;
	while (mLangTable->item(i, 0) != nullptr)
	{
		auto name = mLangTable->item(i, 0)->text();
		auto state = reinterpret_cast<QCheckBox*>(mLangTable->cellWidget(i, 2))->checkState(); //reinterpret_cast<QCheckBox*>(mLangTable->item(0, 2))->checkState();

		if(state == Qt::CheckState::Checked)
		{
			selectedLanguages.push_back(name);
		}

		++i;
	}
	
	emit closeSignal(selectedLanguages);
	QDialog::closeEvent(event);
}

TesseractSettingsDialog::~TesseractSettingsDialog() {
	
}
