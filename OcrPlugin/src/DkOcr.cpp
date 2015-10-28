#include "DkOcr.h"

#include <baseapi.h> //Tesseract
#include <iostream>
#include <QtGui/QPainter>

#include "DkSettingsDialog.h"

void Ocr::testOcr(QImage& image)
{
	FindDialog fd;
	fd.show();

	std::cout << "testOcr" << std::endl;

	tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
	// Initialize tesseract-ocr with English, without specifying tessdata path
	if (api->Init("E:/dev/tesseract", "eng")) {
		std::cout <<  "Could not initialize tesseract" << std::endl;
	}

	Pix* thresholded_image = api->GetThresholdedImage();

	api->SetImage(image.bits(), image.width(), image.height(), image.bytesPerLine() / image.width(), image.bytesPerLine());
	char* rectext = api->GetUTF8Text();

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

	std::cout << "read text: " << rectext << std::endl;
	std::cout << "drew " << rects.size() << " Rects" << std::endl;
	api->End();
	delete[] rectext;
}