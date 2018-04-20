#include "DkOcr.h"

#include <QtGui/QPainter>
#include <QDir>

#include <vector>

#include "genericvector.h"
#include "strngs.h"
#include <QtWidgets/QMessageBox>
#include <QCoreApplication>
#include <tesseract/baseapi.h>

Ocr::TesseractApi::TesseractApi() {
	api = nullptr;
}

Ocr::TesseractApi::~TesseractApi() {
	if (api) {
		api->End();
		delete api;
	}
}

bool Ocr::TesseractApi::initialize(const std::vector<std::string>& ll) {

	if (api) {
		api->End();
		delete api;
	}

	if(!api)
		api = new tesseract::TessBaseAPI();

	std::string langConcat;
	if (ll.size() > 0) // concat languages
	{
		auto iter = ll.begin();
		langConcat = *iter;

		for (; iter != ll.end(); ++iter) {
			langConcat += "+" + *iter;
		}
	}
	else
	{
		langConcat = "eng"; // default
	}

	//api->GetAvailableLanguagesAsVector()

	QString languagePath = (QCoreApplication::applicationDirPath() + "/plugins/tessdata");
	std::string languagePath_cstr = languagePath.toStdString();
	const char* language_cstr = langConcat.c_str();

	if (api->Init(languagePath_cstr.c_str(), language_cstr, tesseract::OcrEngineMode::OEM_TESSERACT_ONLY)) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", QString("Could not load language files from: ") + languagePath + " (https://github.com/tesseract-ocr/tessdata)");
		messageBox.setFixedSize(500, 200);
		return false;
	}

	GenericVector<STRING> languages;
	api->GetAvailableLanguagesAsVector(&languages);

	for (int index = 0; index < languages.size(); ++index) {
		STRING& string = languages[index];
		QString str(string.string());
		availableLanguages.push_back(str);
	}

	return true;
}

QString	Ocr::TesseractApi::runOcr(QImage& image) {
	api->SetImage(image.bits(), image.width(), image.height(), image.bytesPerLine() / image.width(), image.bytesPerLine());
	char* rectext = api->GetUTF8Text();
	qDebug("text: %s", rectext);

	QVector<QRect> rects;
	tesseract::PageIterator *iter = api->AnalyseLayout();
	while (iter->Next(tesseract::RIL_WORD)) {
		int left, top, right, bottom;

		iter->BoundingBox(
			tesseract::RIL_WORD,
			&left, &top, &right, &bottom
		);

		rects.push_back(QRect(left, top, right - left, bottom - top));
	}

	QPixmap px = QPixmap::fromImage(image);
	QPainter painter(&px);
	QPen penHLines(QColor("#0e5a77"), 2, Qt::DotLine, Qt::FlatCap, Qt::RoundJoin);
	painter.setPen(penHLines);

	painter.drawRects(rects);
	image = px.toImage();

	auto text = QString(rectext);
	delete[] rectext;

	return text;
}

QList<QString> Ocr::TesseractApi::getAvailableLanguages()
{
	return availableLanguages;
}

QString Ocr::testOcr(QImage& image)
{
	tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
	// Initialize tesseract-ocr with English, without specifying tessdata path

	if (api->Init((QDir::currentPath() + "/plugins").toStdString().c_str(), "eng")) {
		qCritical("Could not initialize tesseract");
	}

	//Pix* thresholded_image = api->GetThresholdedImage();

	api->SetImage(image.bits(), image.width(), image.height(), image.bytesPerLine() / image.width(), image.bytesPerLine());
	char* rectext = api->GetUTF8Text();
	qDebug("text: %s", rectext);

	QVector<QRect> rects;
	tesseract::PageIterator *iter = api->AnalyseLayout();
	while (iter->Next(tesseract::RIL_WORD)) {
		int left, top, right, bottom;

		iter->BoundingBox(
			tesseract::RIL_WORD,
			&left, &top, &right, &bottom
			);
		
		rects.push_back(QRect(left, top, right-left, bottom-top));
	}

	QPixmap px = QPixmap::fromImage(image);
	QPainter painter(&px);
	QPen penHLines(QColor("#0e5a77"), 2, Qt::DotLine, Qt::FlatCap, Qt::RoundJoin);
	painter.setPen(penHLines);

	painter.drawRects(rects);
	image = px.toImage();

	auto text = QString(rectext);

	api->End();
	delete[] rectext;

	return text;
}