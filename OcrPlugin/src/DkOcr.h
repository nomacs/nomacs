#ifndef DK_OCR_H
#define DK_OCR_H

#include <baseapi.h> //Tesseract

#include <QtGui/QImage>

class QFileInfo;

namespace Ocr {

	class TesseractApi {
	private:
		tesseract::TessBaseAPI* api;
		QList<QString> availableLanguages;
		QString mTessdataPath;

	public:
		TesseractApi();
		~TesseractApi();
		bool initialize(const QString& mode, const QStringList& languages, const QString& config = ""); // initialize api
		QString runOcr(QImage& image);
		QList<QString> getAvailableLanguages();
		void readConfigFile(const QFileInfo& configFilePath);
		void getAvailableLanguages(QList<QString>& qlanguages);
	};

	QString testOcr(QImage& image);
	
}

#endif