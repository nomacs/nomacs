#ifndef DK_OCR_H
#define DK_OCR_H

#include <baseapi.h> //Tesseract

#include <QtGui/QImage>

namespace Ocr {

	class TesseractApi {
	private:
		tesseract::TessBaseAPI* api;
		QList<QString> availableLanguages;

	public:
		TesseractApi();
		~TesseractApi();
		bool initialize(const std::vector<std::string>& languages); // initialize api
		QString runOcr(QImage& image);
		QList<QString> getAvailableLanguages();
	};

	QString testOcr(QImage& image);
}

#endif