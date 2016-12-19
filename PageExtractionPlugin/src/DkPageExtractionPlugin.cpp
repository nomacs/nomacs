/*******************************************************************************************************
 DkPageExtractionPlugin.cpp

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2015 Markus Diem

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

 *******************************************************************************************************/

#include "DkPageExtractionPlugin.h"
#include "DkPageSegmentation.h"

#include "DkImageStorage.h"
#include "DkMetaData.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#include <QDebug>
#include <QUuid>
#include <QDateTime>
#include <QDir>

#include <QXmlStreamReader>
#pragma warning(pop)		// no warnings from includes - end

namespace nmp {

/**
*	Constructor
**/
DkPageExtractionPlugin::DkPageExtractionPlugin(QObject* parent) : QObject(parent) {

	// create run IDs
	QVector<QString> runIds;
	runIds.resize(id_end);

	for (int idx = 0; idx < id_end; idx++)
		runIds[idx] = QUuid::createUuid().toString();
	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);
		
	menuNames[id_crop_to_page] = tr("Crop to Page");
	menuNames[id_crop_to_metadata] = tr("Crop to Metadata");
	menuNames[id_draw_to_page] = tr("Draw to Page");
	menuNames[id_eval_page] = tr("Evaluate Page");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_crop_to_page] = tr("Finds a page in a document image and then crops the image to that page.");
	statusTips[id_crop_to_metadata] = tr("Finds a page in a document image and then saves the coordinates to the XMP metadata.");
	statusTips[id_draw_to_page] = tr("Finds a page in a document image and then draws the found document boundaries.");
	statusTips[id_eval_page] = tr("Loads GT and computes the Jaccard index.");
	mMenuStatusTips = statusTips.toList();

	QFileInfo resPath(QDir("D:/dmrz/numerical-results/"), "results-" + QDateTime::currentDateTime().toString("yyyy-MM-dd HH-mm-ss") + ".txt");
	mResultPath = resPath.absoluteFilePath();
}

/**
*	Destructor
**/
DkPageExtractionPlugin::~DkPageExtractionPlugin() {
}


/**
* Returns unique ID for the generated dll
**/
QString DkPageExtractionPlugin::id() const {

	return PLUGIN_ID;
};


/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage DkPageExtractionPlugin::image() const {

	return QImage(":/PageExtractionPlugin/img/page-extraction.png");
};

/**
* Returns plugin version for every ID
* @param plugin ID
**/
QString DkPageExtractionPlugin::version() const {

	return PLUGIN_VERSION;
};

QList<QAction*> DkPageExtractionPlugin::createActions(QWidget* parent) {

	if (mActions.empty()) {

		for (int idx = 0; idx < id_end; idx++) {
			QAction* ca = new QAction(mMenuNames[idx], parent);
			ca->setObjectName(mMenuNames[idx]);
			ca->setStatusTip(mMenuStatusTips[idx]);
			ca->setData(mRunIDs[idx]);	// runID needed for calling function runPlugin()
			mActions.append(ca);
		}
	}

	return mActions;
}

QList<QAction*> DkPageExtractionPlugin::pluginActions() const {

	return mActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> DkPageExtractionPlugin::runPlugin(
	const QString &runID, 
	QSharedPointer<nmc::DkImageContainer> imgC, 
	const nmc::DkSaveInfo& saveInfo, 
	QSharedPointer<nmc::DkBatchInfo>& batchInfo) const {

	if (!mRunIDs.contains(runID) || !imgC)
		return imgC;

	cv::Mat img = nmc::DkImage::qImage2Mat(imgC->image());

	// run the page segmentation
	DkPageSegmentation segM(img);
	segM.compute();
	segM.filterDuplicates();

	// crop image
	if(runID == mRunIDs[id_crop_to_page]) {
		imgC->setImage(segM.getCropped(imgC->image()), tr("Page Cropped"));
	}
	// save to metadata
	if(runID == mRunIDs[id_crop_to_metadata]) {
		
		if (segM.getRects().empty())
			imgC = QSharedPointer<nmc::DkImageContainer>();	// notify parent
		else {
			nmc::DkRotatingRect rect = segM.getMaxRect().toRotatingRect();
			
			QSharedPointer<nmc::DkMetaDataT> m = imgC->getMetaData();
			m->saveRectToXMP(rect, imgC->image().size());
		}
	}
	// draw rectangles to the image
	else if(runID == mRunIDs[id_draw_to_page]) {
		
		QImage dImg = imgC->image();
		segM.draw(dImg);
		imgC->setImage(dImg, tr("Page Annotated"));
	}
	else if (runID == mRunIDs[id_eval_page]) {

		QImage dImg = imgC->image();

		QPolygonF gt = readGT(imgC->filePath());
		
		QPen pen(QColor(100, 200, 50));
		pen.setWidth(10);
		QPainter p(&dImg);
		p.setPen(pen);
		p.drawPolygon(gt);
		p.end();

		segM.draw(dImg);
		imgC->setImage(dImg, tr("Result vs GT"));

		double ji = jaccardIndex(imgC->image().size(), gt, segM.getMaxRect().toPolygon());

		QString data = imgC->fileName() + ", " + QString::number(ji) + "\n";
		qDebug() << data;

		QFile file(mResultPath);
		file.open(QIODevice::WriteOnly | QIODevice::Append);
		QTextStream stream(&file);
		stream << data;
		qInfo() << "results written to" << mResultPath;

	}

	// wrong runID? - do nothing
	return imgC;
}

QPolygonF DkPageExtractionPlugin::readGT(const QString& imgPath) const {

	QFileInfo imgInfo(imgPath);

	QFileInfo xmlFileI(imgInfo.absolutePath(), imgInfo.baseName() + ".xml");

	if (!xmlFileI.exists()) {
		qWarning() << "no xml file found: " << xmlFileI.absoluteFilePath();
		return QPolygonF();
	}
	QFile xmlFile(xmlFileI.absoluteFilePath());
	if (!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qWarning() << "could not load" << xmlFileI.absoluteFilePath();
		return QPolygonF();
	}

	QXmlStreamReader xmlReader(&xmlFile);
	QPolygonF rect;

	while (!xmlReader.atEnd() && !xmlReader.hasError()) {

		QString tag = xmlReader.qualifiedName().toString();

		if (xmlReader.tokenType() == QXmlStreamReader::StartElement && tag == "dmrz") {
			
			for (int idx = 0; idx < 4; idx++) {

				QPoint p;
				p.setX(xmlReader.attributes().value("x" + QString::number(idx)).toInt());
				p.setY(xmlReader.attributes().value("y" + QString::number(idx)).toInt());
				rect << p;
			}
		}
		xmlReader.readNext();
	}

	return rect;
}

double DkPageExtractionPlugin::jaccardIndex(const QSize & imgSize, const QPolygonF & gt, const QPolygonF & computed) const {
	
	cv::Mat gtImg = nmc::DkImage::qImage2Mat(drawPoly(imgSize, gt));
	cv::Mat evImg = nmc::DkImage::qImage2Mat(drawPoly(imgSize, computed));

	double and = cv::sum(gtImg & evImg)[0];
	double or = cv::sum(gtImg | evImg)[0];

	return and/or;
}

QImage DkPageExtractionPlugin::drawPoly(const QSize & imgSize, const QPolygonF & poly) const {

	QImage img(imgSize, QImage::Format_RGB888);
	img.fill(QColor(0, 0, 0));

	QPainter pg(&img);
	pg.setBrush(QColor(255, 255, 255));
	pg.drawPolygon(poly);

	return img;
}


};

