/*******************************************************************************************************
 DkDocAnalysisPlugin.cpp
 Created on:	14.07.2013

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2013 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2013 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2013 Florian Kleber <florian@nomacs.org>

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

#include "DkDocAnalysisPlugin.h"
#include "DkViewPort.h"

namespace nmc {

/*-----------------------------------DkDocAnalysisPlugin ---------------------------------------------*/

DkSettings::Display& DkSettings::display = DkSettings::getDisplaySettings();
DkSettings::Global& DkSettings::global = DkSettings::getGlobalSettings();

/**
*	Constructor
**/
DkDocAnalysisPlugin::DkDocAnalysisPlugin() {

	viewport = 0;
}

/**
*	Destructor
**/
DkDocAnalysisPlugin::~DkDocAnalysisPlugin() {

	qDebug() << "[DOCUMENT ANALYSIS PLUGIN] deleted...";

	if (viewport) {
		viewport->deleteLater();
		viewport = 0;
	}
}

/**
* Returns unique ID for the generated dll
**/
QString DkDocAnalysisPlugin::pluginID() const {
	return PLUGIN_ID;
};


/**
* Returns plugin name
**/
QString DkDocAnalysisPlugin::pluginName() const {

   return "Document Analysis";
};

/**
* Returns long description
**/
QString DkDocAnalysisPlugin::pluginDescription() const {

   return "<b>Created by:</b> Daniel Fischl<br><b>Modified:</b> Oct 2014<br><b>Description:</b> Tools to analyze images of documents, like text line detection, region cutting, distance measurement.";
};

/**
* Returns descriptive image
**/
QImage DkDocAnalysisPlugin::pluginDescriptionImage() const {

   return QImage(":/nomacsPluginDocAnalysis/img/description.png");
};

/**
* Returns plugin version
**/
QString DkDocAnalysisPlugin::pluginVersion() const {

   return PLUGIN_VERSION;
};

/**
* Returns unique IDs for every plugin in this dll
* plugin can have more the one functionality that are triggered in the menu
* runID differes from pluginID
* viewport plugins can have only one runID and one functionality bound to it 
**/
QStringList DkDocAnalysisPlugin::runID() const {

	//GUID without hyphens generated at http://www.guidgenerator.com/
	return QStringList() << PLUGIN_ID;
};

/**
* Returns plugin name for every run ID
* @param run ID
**/
QString DkDocAnalysisPlugin::pluginMenuName(const QString &runID) const {

	if (runID==PLUGIN_ID) return "Document Analysis";
	return "Wrong GUID!";
};

/**
* Returns short description for status tip for every ID
* @param plugin ID
**/
QString DkDocAnalysisPlugin::pluginStatusTip(const QString &runID) const {

	if (runID==PLUGIN_ID) return "Tools for Document Analysis";
	return "Wrong GUID!";
};

/**
* Main function: runs plugin based on its ID
* @param run ID
* @param current image in the Nomacs viewport
**/
QImage DkDocAnalysisPlugin::runPlugin(const QString &runID, const QImage &image) const {
	
	//for a viewport plugin runID and image are null
	if (viewport) {

		DkDocAnalysisViewPort* docAnalysisViewport = dynamic_cast<DkDocAnalysisViewPort*>(viewport);

		QImage retImg = QImage();
		if (!docAnalysisViewport->isCanceled()) retImg = docAnalysisViewport->getPaintedImage();

		viewport->setVisible(false);

		return retImg;
	}
	
	return image;
};



/**
* returns paintViewPort
**/
DkPluginViewPort* DkDocAnalysisPlugin::getViewPort() {

	if (!viewport) {
		DkDocAnalysisViewPort* vp = new DkDocAnalysisViewPort();
		vp->setMainWindow(getMainWidnow());

		viewport = vp;

		// signal for saving magic cut
		connect(viewport, SIGNAL(saveMagicCutRequest(QImage, int, int, int, int)), this, SLOT(saveMagicCut(QImage, int, int, int, int)));
		connect(this, SIGNAL(magicCutSavedSignal(bool)), viewport, SLOT(magicCutSaved(bool)));
	}
	return viewport;
}

/**
* sets the viewport pointer to NULL after the viewport is destroyed
**/
void DkDocAnalysisPlugin::viewportDestroyed() {

	viewport = 0;
}

void DkDocAnalysisPlugin::deleteViewPort() {

	if (viewport) {
		viewport->deleteLater();
		viewport = 0;
	}
}

/**
* function for saving a magic cut.
* Saves the image using it's original name and appending the x- and y-Coordinates to the name.
* Additional metadata to where the cut is located in the original image
* is written in the Comment string of the image.
* @param saveImg The image to be saved
* @param xCoord The x-coordinate of the upper left corner of the bounding box of the cut
* @param yCoord The y-coordinate of the upper left corner of the bounding box of the but
* @param height The height of the bounding box (y-extent)
* @param width The width of the bounding box (x-extent)
* \sa magicCutSavedSignal(bool) 
**/
void DkDocAnalysisPlugin::saveMagicCut(QImage saveImage, int xCoord, int yCoord, int height, int width/* QString saveNameAppendix*/) {

	qDebug() << "saving...";

	DkImageLoader* loader;
	DkNoMacs* nmcWin;
	QMainWindow* win = getMainWidnow();
	if (win) {

		// this should usually work - since we are a nomacs plugin
		nmcWin = dynamic_cast<DkNoMacs*>(getMainWidnow());

		if (nmcWin) {

			DkViewPort* vp = nmcWin->viewport();

			if (vp) {
				loader = vp->getImageLoader();
			}

		}
	}
	
	QString selectedFilter;
	QString saveName;
	QFileInfo saveFile;

	QString saveNameAppendix = QString("_%1").arg(xCoord);
	saveNameAppendix.append(QString("_%1").arg(yCoord));

	if (loader->hasFile()) {
		saveFile = loader->file();
		saveName = saveFile.fileName();

		qDebug() << "saveName: " << saveName; //.toStdString();
		
		qDebug() << "save dir: " << loader->getSaveDir();

		if (loader->getSaveDir() != saveFile.absoluteDir())
			saveFile = QFileInfo(loader->getSaveDir(), saveName);

		int filterIdx = -1;

		QStringList sF = DkSettings::saveFilters;
		//qDebug() << sF;

		QRegExp exp = QRegExp("*." + saveFile.suffix() + "*", Qt::CaseInsensitive);
		exp.setPatternSyntax(QRegExp::Wildcard);

		for (int idx = 0; idx < sF.size(); idx++) {

			//qDebug() << exp;
			//qDebug() << saveFile.suffix();
			//qDebug() << sF.at(idx);

			if (exp.exactMatch(sF.at(idx))) {
				selectedFilter = sF.at(idx);
				filterIdx = idx;
				break;
			}
		}

		if (filterIdx == -1)
			saveName.remove("." + saveFile.suffix());
	}

	// note: basename removes the whole file name from the first dot...
	QString savePath = (!selectedFilter.isEmpty()) ? saveFile.absoluteFilePath() : QFileInfo(saveFile.absoluteDir(), saveName).absoluteFilePath();

	savePath.insert(savePath.length()-saveFile.completeSuffix().length()-1, saveNameAppendix);

	QString fileName = QFileDialog::getSaveFileName(viewport, tr("Save Magic Cut"),
		savePath, DkSettings::saveFilters.join(";;"), &selectedFilter);

	//qDebug() << "selected Filter: " << selectedFilter;

	if (fileName.isEmpty())
		return;


	QString ext = QFileInfo(fileName).suffix();

	if (!ext.isEmpty() && !selectedFilter.contains(ext)) {

		QStringList sF = DkSettings::saveFilters;

		for (int idx = 0; idx < sF.size(); idx++) {

			if (sF.at(idx).contains(ext)) {
				selectedFilter = sF.at(idx);
				break;
			}
		}
	}

	// TODO: if more than one file is opened -> open new threads
	QFileInfo sFile = QFileInfo(fileName);
	int compression = -1;	// default value

	//if (saveDialog->selectedNameFilter().contains("jpg")) {
	if (selectedFilter.contains(QRegExp("(jpg|jpeg|j2k|jp2|jpf|jpx)", Qt::CaseInsensitive))) {

		if (!jpgDialog)
			jpgDialog = new DkCompressDialog(nmcWin);

		if (selectedFilter.contains(QRegExp("(j2k|jp2|jpf|jpx)")))
			jpgDialog->setDialogMode(DkCompressDialog::j2k_dialog);
		else
			jpgDialog->setDialogMode(DkCompressDialog::jpg_dialog);

		jpgDialog->imageHasAlpha(saveImage.hasAlphaChannel());
		//jpgDialog->show();
		if (!jpgDialog->exec())
			return;

		compression = jpgDialog->getCompression();


		if (saveImage.hasAlphaChannel()) {

			QRect imgRect = QRect(QPoint(), saveImage.size());
			QImage tmpImg = QImage(saveImage.size(), QImage::Format_RGB32);
			QPainter painter(&tmpImg);
			painter.fillRect(imgRect, jpgDialog->getBackgroundColor());
			painter.drawImage(imgRect, saveImage, imgRect);

			saveImage = tmpImg;
		}

	//	qDebug() << "returned: " << ret;
	}

	if (selectedFilter.contains("webp")) {

		if (!jpgDialog)
			jpgDialog = 0; // new DkCompressDialog(nmcWin);

		jpgDialog->setDialogMode(DkCompressDialog::webp_dialog);

		jpgDialog->setImage(&saveImage);

		if (!jpgDialog->exec())
			return;

		compression = jpgDialog->getCompression();
	}

	//if (saveDialog->selectedNameFilter().contains("tif")) {
	if (selectedFilter.contains("tif")) {
		
		if (!tifDialog)
			tifDialog = 0; //new DkTifDialog(nmcWin);

		if (!tifDialog->exec())
			return;

		compression = tifDialog->getCompression();
	}

	QImageWriter* imgWriter = new QImageWriter(sFile.absoluteFilePath());
	imgWriter->setCompression(compression);
	imgWriter->setQuality(compression);
	
	if (imgWriter->supportsOption(QImageIOHandler::Description)) {
		QString comment = QString(saveName);
		comment.append(QString("; %1").arg(xCoord));
		comment.append(QString("; %1").arg(yCoord));
		comment.append(QString("; %1").arg(height));
		comment.append(QString("; %1").arg(width));

		imgWriter->setText("Comment", comment);
	}
	
	bool saved = imgWriter->write(saveImage);
	//imgWriter->setFileName(QFileInfo().absoluteFilePath());
	delete imgWriter;


	emit magicCutSavedSignal(saved);
	//bool saved = sImg.save(filePath, 0, compression);
	//qDebug() << "jpg compression: " << compression;
}

void DkDocAnalysisPlugin::contextMenuEvent(QContextMenuEvent *event) {

	// ignore context menu event if any editing is active
	if( ((DkDocAnalysisViewPort *)viewport)->editingActive() )
		event->ignore();
	else
		return;
		//QWidget::contextMenuEvent(event);
		//return; //DkNoMacs::contextMenuEvent(event);
}



/* macro for exporting plugin */
Q_EXPORT_PLUGIN2("com.nomacs.ImageLounge.DkDocAnalysisPlugin/1.0", DkDocAnalysisPlugin)


/*-----------------------------------DkDocAnalysisViewPort ---------------------------------------------*/

DkDocAnalysisViewPort::DkDocAnalysisViewPort(QWidget* parent, Qt::WindowFlags flags) : DkPluginViewPort(parent, flags) {
	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);
	init();
}

DkDocAnalysisViewPort::~DkDocAnalysisViewPort() {
	qDebug() << "[DOCUMENT ANALYSIS VIEWPORT] deleted...";
	
	// acitive deletion since the MainWindow takes ownership...
	// if we have issues with this, we could disconnect all signals between viewport and toolbar too
	// however, then we have lot's of toolbars in memory if the user opens the plugin again and again
	if (magicCut) {
		delete magicCut;
	}
	if (lineDetection) {
		delete lineDetection;
	}
	if (lineDetectionDialog) {
		delete lineDetectionDialog;
	}
	if (docAnalysisToolbar) {
		delete docAnalysisToolbar;
		docAnalysisToolbar = 0;
	}
	setMouseTracking(false);
	setFocusPolicy(Qt::NoFocus);
}

void DkDocAnalysisViewPort::init() {
	
	cancelTriggered = false;
	isOutside = false;
	defaultCursor = Qt::ArrowCursor;
	setCursor(defaultCursor);
	
	editMode = mode_default;
	showBottomLines = false;
	showTopLines = false;
	
	docAnalysisToolbar = new DkDocAnalysisToolBar(tr("Document Analysis Toolbar"), this);

	// connect signals from toolbar to viewport
	connect(docAnalysisToolbar, SIGNAL(measureDistanceRequest(bool)), this, SLOT(pickDistancePoint(bool)));
	connect(docAnalysisToolbar, SIGNAL(pickSeedpointRequest(bool)),  this, SLOT(pickSeedpoint(bool)));
	connect(docAnalysisToolbar, SIGNAL(clearSelectionSignal()), this, SLOT(clearMagicCut()));
	connect(docAnalysisToolbar, SIGNAL(toleranceChanged(int)), this, SLOT(setMagicCutTolerance(int)));
	connect(docAnalysisToolbar, SIGNAL(openCutDialogSignal()), this, SLOT(openMagicCutDialog()));
	connect(docAnalysisToolbar, SIGNAL(detectLinesSignal()), this, SLOT(openLineDetectionDialog()));
	connect(docAnalysisToolbar, SIGNAL(showBottomTextLinesSignal(bool)), this, SLOT(showBottomTextLines(bool)));
	connect(docAnalysisToolbar, SIGNAL(showTopTextLinesSignal(bool)), this, SLOT(showTopTextLines(bool)));
	connect(this, SIGNAL(cancelPickSeedpointRequest()), docAnalysisToolbar, SLOT(pickSeedpointCanceled()));
	connect(this, SIGNAL(cancelDistanceMeasureRequest()), docAnalysisToolbar, SLOT(measureDistanceCanceled()));
	connect(this, SIGNAL(enableSaveCutSignal(bool)), docAnalysisToolbar, SLOT(enableButtonSaveCut(bool)));
	connect(this, SIGNAL(enableShowTextLinesSignal(bool)), docAnalysisToolbar, SLOT(enableButtonShowTextLines(bool)));
	connect(this, SIGNAL(toggleBottomTextLinesButtonSignal(bool)), docAnalysisToolbar, SLOT(toggleBottomTextLinesButton(bool)));
	connect(this, SIGNAL(toggleTopTextLinesButtonSignal(bool)), docAnalysisToolbar, SLOT(toggleTopTextLinesButton(bool)));
	connect(this, SIGNAL(startDistanceMeasureRequest()), docAnalysisToolbar, SLOT(measureDistanceStarted()));
	connect(this, SIGNAL(startPickSeedpointRequest()), docAnalysisToolbar, SLOT(pickSeedpointStarted()));

	// the magic cut tool
	magicCut = new DkMagicCut();
	magicCutDialog = 0;
	// regular update of contours
	QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateAnimatedContours()));
    timer->start(850);
	// the distance tool
	distance = new DkDistanceMeasure();
	// the line detection tool
	lineDetection = new DkLineDetection();
	lineDetectionDialog = 0;


	DkPluginViewPort::init();
}


void DkDocAnalysisViewPort::mouseMoveEvent(QMouseEvent *event) {

	if (editingActive()) {
		if(parent()) {
			DkBaseViewPort* viewport = dynamic_cast<DkBaseViewPort*>(parent());
			
			if(viewport) {
		
				if(QRectF(QPointF(), viewport->getImage().size()).contains(mapToImage(event->posF()))) {
					
					switch(editMode) {

					case mode_pickSeedpoint: {
						this->setCursor(Qt::PointingHandCursor);
						break;
					}
					case mode_pickDistance: {
						if(distance->hasStartPoint() && !distance->hastStartAndEndPoint()) {
							this->setCursor(Qt::BlankCursor);
						} else {
							this->setCursor(Qt::CrossCursor);
						}
						//imgPos = worldMatrix.inverted().map(event->pos());
						//imgPos = imgMatrix.inverted().map(imgPos);
						QPointF imgPos;
						imgPos = mapToImage(event->posF());
						distance->setCurPoint(imgPos.toPoint());
						update();
						break;
					}
					default:
						break;
					}
					
					/*if (isOutside) {
						paths.append(QPainterPath());
						paths.last().moveTo(mapToImage(event->posF()));
						pathsPen.append(pen);
					}
					else {
						QPointF point = mapToImage(event->posF());
						paths.last().lineTo(point);
						update();
					}*/
					isOutside = false;
				}
				else isOutside = true;
			}
		}
	} else {
		// propagate mouse event (for panning)
		this->unsetCursor();
		QWidget::mouseMoveEvent(event);
	}
}

void DkDocAnalysisViewPort::mouseDoubleClickEvent(QMouseEvent *event) {

	// if any editing operation is going on then do not allow full screen
	if (editingActive() || editingDrawingActive() )
		return;
	else
		QWidget::mouseDoubleClickEvent(event);
}

void DkDocAnalysisViewPort::mouseReleaseEvent(QMouseEvent *event) {

	QPointF imgPos;
	QPoint xy;
	imgPos = mapToImage(event->posF());
	xy = imgPos.toPoint();

	if(parent()) {
		DkBaseViewPort* viewport = dynamic_cast<DkBaseViewPort*>(parent());

		if(viewport) {
			// check if point is within image
			if(QRectF(QPointF(), viewport->getImage().size()).contains(imgPos)) {

				switch(editMode) {

				case mode_pickSeedpoint:
					//imgPos = worldMatrix.inverted().map(event->pos());
					//imgPos = imgMatrix.inverted().map(imgPos);
					
					if (event->button() == Qt::LeftButton)
						if(!magicCut->magicwand(xy)) {
								QString tooLargeAreaString = QString("Selected area is too big");
								QMessageBox tooLargeAreaDialog(this);
								tooLargeAreaDialog.setWindowTitle("Area too big");
								tooLargeAreaDialog.setIcon(QMessageBox::Information);
								tooLargeAreaDialog.setText(tooLargeAreaString);
								tooLargeAreaDialog.show();
								tooLargeAreaDialog.exec();
						}
					if (event->button() == Qt::RightButton) {
						magicCut->resetRegionMask(xy);
					}
					update();
					// check if the save-button has to be enabled or disabled
					if(magicCut->hasContours())
						emit enableSaveCutSignal(true);
					else
						emit enableSaveCutSignal(false);

					this->setCursor(Qt::PointingHandCursor);
					break;

				case mode_pickDistance: 

					if(distance->hastStartAndEndPoint()) {
						distance->resetPoints();
					}

					distance->setPoint(xy);
					update();

					/*if(distance->hastStartAndEndPoint())
					{
						QString distanceString = QString("Distance: %1 cm (%2 inch)").arg(distance->getDistanceInCm()).arg(distance->getDistanceInInch());
						QMessageBox distanceDialog(this);
						distanceDialog.setWindowTitle("Distance");
						distanceDialog.setIcon(QMessageBox::Information);
						distanceDialog.setText(distanceString);
						distanceDialog.show();
						distanceDialog.exec();

						distance->resetPoints();
						// go back to default mode
						stopEditing();
					}*/
		
					this->setCursor(Qt::CrossCursor);
					break;
				}
			}
		}
	}
}

void DkDocAnalysisViewPort::keyPressEvent(QKeyEvent* event) {

	if ((event->key() == Qt::Key_Escape) && (editMode != mode_default)) {
		
		stopEditing();	
	}
	else if (event->key() == Qt::Key_Return) {
		// use Alt + Enter for MagicCut to distinguish from ordinary cut (which is only Enter)
		if(event->modifiers() == Qt::AltModifier && !event->isAutoRepeat()) {

			openMagicCutDialog();
		}
	}
	else if (editMode == mode_pickDistance && event->key() == Qt::Key_Shift) {
		// if shift is held, perform snapping for distance measure tool
		distance->setSnapping(true);
		if(!event->isAutoRepeat()) {
			if(distance->hasStartPoint()) {
				// update immediately
				distance->setCurPoint(distance->getCurPoint());
			}
		}
		update();
	}
	else {
		//DkViewPort::keyPressEvent(event);
		// propagate event
		QWidget::keyPressEvent(event);
	}

}

void DkDocAnalysisViewPort::keyReleaseEvent(QKeyEvent *event) {
	distance->setSnapping(false);
}

/**
* Function to check if nomacs is in any of the editing modes (pick seedpoint/color/distance) 
* @returns true, if any editing function is active (!= mode_default)
* \sa editModes
**/
bool DkDocAnalysisViewPort::editingActive() {
		if(editMode == mode_default)
			return false;
		else 
			return true;
}

/**
* Check if any drawing specific to the editing operations is active
* @returns true if contour drawing or distance drawing is currently active
**/
bool DkDocAnalysisViewPort::editingDrawingActive() {

	if(magicCut)
		return magicCut->hasContours();
	if(distance)
		return distance->hasStartPoint();

	return false;
}

/**
* Stops the current editing and sends corresponding signals to the Tool Bar
* \sa cancelDistanceMeasureRequest() DkMagicCutToolBar::measureDistanceCanceled()
* \sa cancelPickSeedpointRequest() DkMagicCutToolBar::pickSeedpointCanceled()
**/
void DkDocAnalysisViewPort::stopEditing() {

	switch(editMode) {
	case mode_pickDistance:
		emit cancelDistanceMeasureRequest();
		distance->resetPoints();
		break;
	case mode_pickSeedpoint:
		emit cancelPickSeedpointRequest();
		break;
	}
	editMode = mode_default;
	this->unsetCursor();
	update();
}

void DkDocAnalysisViewPort::paintEvent(QPaintEvent *event) {
	
	QPainter painter(this);
	
	if (worldMatrix)
		painter.setWorldTransform((*imgMatrix) * (*worldMatrix));	// >DIR: using both matrices allows for correct resizing [16.10.2013 markus]

	if(parent()) {
		DkBaseViewPort* viewport = dynamic_cast<DkBaseViewPort*>(parent());
		
		// show the bottom text lines if toggled
		if (showBottomLines) {
			QImage botTextLines = lineDetection->getBottomLines();
			painter.drawImage(botTextLines.rect(), botTextLines);
		}
		if (showTopLines) {
			QImage topTextLines = lineDetection->getTopLines();
			painter.drawImage(topTextLines.rect(), topTextLines);
		}


		// draw Contours
		if (magicCut->hasContours()) {	
			drawContours(&painter);
		}

		// draw distance line
		if (editMode == mode_pickDistance) {
			drawDistanceLine(&painter);
		}
	}

	painter.end();

	DkPluginViewPort::paintEvent(event);
}

/**
* Slot - A timed function, called regularly to generate an animated effect
* for the drawn contours of the magic cut areas
* \sa DkMagicCut DkMagicCut::updateAnimateContours()
**/
void DkDocAnalysisViewPort::updateAnimatedContours() {

	magicCut->updateAnimateContours();
	update();
}

/**
* Draws the contours of selected regions (made using the magic cut tool)
* @param painter The painter to use
* \sa DkMagicCut
**/ 
void DkDocAnalysisViewPort::drawContours(QPainter *painter) {

	QPen pen = painter->pen();
	painter->setPen(magicCut->getContourPen());
	painter->drawPath(magicCut->getContourPath());
	painter->setPen(pen);
}


/**
* Draws lines and points referring to the distance measure tool
* @param painter The painter to use
* \sa DkDistanceMeasure
**/
void DkDocAnalysisViewPort::drawDistanceLine(QPainter *painter) {

	// return if no start point yet
	if(!distance->hasStartPoint()) return;

	if (distance->getCurPoint().isNull()) {
		distance->setCurPoint(distance->getStartPoint());
	}

	QPoint point = distance->getStartPoint();
	point = imgMatrix->map(point);
	
	// special handling of drawing cross - to avoid zooming of cross lines
	QPointF startPointMapped = worldMatrix->map(point);
	QPointF crossTransP1 = startPointMapped;
	crossTransP1.setY(crossTransP1.y() + 7);
	QPointF crossTransP2 = startPointMapped;
	crossTransP2.setY(crossTransP2.y() - 7);
	painter->setWorldMatrixEnabled(false);
	painter->drawLine(crossTransP1, crossTransP2);
	painter->setWorldMatrixEnabled(true);
	crossTransP1 = startPointMapped;
	crossTransP1.setX(crossTransP1.x() + 7);
	crossTransP2 = startPointMapped;
	crossTransP2.setX(crossTransP2.x() - 7);
	painter->setWorldMatrixEnabled(false);
	painter->drawLine(crossTransP1, crossTransP2);
	painter->setWorldMatrixEnabled(true);
	
	// draw the line
	painter->drawLine(distance->getStartPoint(), distance->getCurPoint());
	// draw the text containing the current distance

	QPoint point_end = distance->getCurPoint();
	point_end = imgMatrix->map(point_end);

	QString dist_text = QString::number(distance->getDistanceInCm(), 'f', 2) + " cm";
	QPoint pos_text = QPoint(point_end.x(), point_end.y());

	QFont font = painter->font();
	font.setPointSizeF(12);

	painter->setFont(font);

	QFontMetricsF fm(font);
	QRectF rect = fm.boundingRect(dist_text);
	QPointF transP = worldMatrix->map(pos_text);

	if (point_end.x() < point.x())
		transP.setX(transP.x());
	else
		transP.setX(transP.x() - rect.width());

	if (point_end.y() > point.y()) 
		transP.setY(transP.y() + rect.height());
	else
		transP.setY(transP.y());
	
	painter->setWorldMatrixEnabled(false);
	painter->drawText(transP.x(), transP.y(), dist_text);
	painter->setWorldMatrixEnabled(true);

	point = distance->getCurPoint();
	point = imgMatrix->map(point);

	// special handling of drawing cross - to avoid zooming of cross lines
	QPointF endPointMapped = worldMatrix->map(point);
	crossTransP1 = endPointMapped;
	crossTransP1.setY(crossTransP1.y() + 7);
	crossTransP2 = endPointMapped;
	crossTransP2.setY(crossTransP2.y() - 7);
	painter->setWorldMatrixEnabled(false);
	painter->drawLine(crossTransP1, crossTransP2);
	painter->setWorldMatrixEnabled(true);
	crossTransP1 = endPointMapped;
	crossTransP1.setX(crossTransP1.x() + 7);
	crossTransP2 = endPointMapped;
	crossTransP2.setX(crossTransP2.x() - 7);
	painter->setWorldMatrixEnabled(false);
	painter->drawLine(crossTransP1, crossTransP2);
	painter->setWorldMatrixEnabled(true);

	/*if(distance->hastStartAndEndPoint()) {
		point = distance->getEndPoint();
		point = imgMatrix.map(point);
		painter->drawLine(point.x(), point.y()+3, point.x(), point.y()-3);
		painter->drawLine(point.x()-3, point.y(), point.x()+3, point.y());
	}*/


}

QImage DkDocAnalysisViewPort::getPaintedImage() {

	if(parent()) {
		DkBaseViewPort* viewport = dynamic_cast<DkBaseViewPort*>(parent());
		if (viewport) {
			QImage img = viewport->getImage();
			return img;
		}
	}
	
	return QImage();
}

void DkDocAnalysisViewPort::setMainWindow(QMainWindow* win) {
	this->win = win;

	QSharedPointer<DkMetaDataT> metadata;
	QImage image;
	// >DIR: OK, let's get the current image metadata [21.10.2014 markus]
	// all ifs are to be 100% save : )
	// TODO: add error dialogs if we cannot retrieve metadata...
	if (win) {

		// this should usually work - since we are a nomacs plugin
		DkNoMacs* nmcWin = dynamic_cast<DkNoMacs*>(win);

		if (nmcWin) {

			DkViewPort* vp = nmcWin->viewport();

			if (vp) {
				DkImageLoader* loader = vp->getImageLoader();
				
				if (loader) {
					QSharedPointer<DkImageContainerT> imgC = loader->getCurrentImage();
					
					if (imgC) {

						metadata = imgC->getMetaData();
						image = imgC->image();
					}
				}
			}

		}
	}

	if (metadata) {
		qDebug() << "metadata received, the image has: " << metadata->getResolution() << " dpi";
	} else {
		qDebug() << "WARNING: I could not retrieve the image metadata...";
		QMessageBox dialog(this);
		dialog.setIcon(QMessageBox::Warning);
		dialog.setText(tr("Could not retrieve image metadata"));
		dialog.show();
		dialog.exec();
	}
	distance->setMetaData(metadata);

	if (!image.isNull()) {
		cv::Mat img = DkImage::qImage2Mat(image);
		// set image for magic cut
		magicCut->setImage(img, imgMatrix);
		// disable the save region button
		emit enableSaveCutSignal(false);
		// the line detection part
		lineDetection->setImage(img);
		if(lineDetectionDialog) {
			lineDetectionDialog->setDefaultConfiguration();
		}
		// disable the display text lines button
		showBottomTextLines(false);
		showTopTextLines(false);
		emit enableShowTextLinesSignal(false);
	}
}

bool DkDocAnalysisViewPort::isCanceled() {
	return cancelTriggered;
}

void DkDocAnalysisViewPort::setVisible(bool visible) {
	
	if (docAnalysisToolbar)
		emit showToolbar(docAnalysisToolbar, visible);

	DkPluginViewPort::setVisible(visible);
}


/**
* Starts/ends the distance points picking mode.
* Cancels any other active modes.
* @param pick start or end the mode
* \sa cancelPickSeedpointRequest() DkMagicCutToolBar::pickSeedpointCanceled()
**/
void DkDocAnalysisViewPort::pickDistancePoint(bool pick) {

	switch(editMode) {
	case mode_pickSeedpoint:
		emit cancelPickSeedpointRequest();
		break;
	}

	if(pick) {
		editMode = mode_pickDistance;
		distance->resetPoints();
		this->setCursor(Qt::CrossCursor);
	} else
		editMode = mode_default;	
}

/**
* Starts the distance points picking mode if not yet active
* Cancels any other active modes.
* \sa cancelPickSeedpointRequest() DkMagicCutToolBar::pickSeedpointCanceled(), startDistanceMeasureRequest()
**/
void DkDocAnalysisViewPort::pickDistancePoint() {

	switch(editMode) {
	case mode_pickDistance:
		return;
	case mode_pickSeedpoint:
		emit cancelPickSeedpointRequest();
		break;
	}

	editMode = mode_pickDistance;
	emit startDistanceMeasureRequest();
	distance->resetPoints();
	this->setCursor(Qt::CrossCursor);
}


/**
* Starts/ends the seed points picking mode for region selection.
* Cancels any other active modes.
* @param pick start or end the mode
* \sa cancelDistanceMeasureRequest() DkMagicCutToolBar::measureDistanceCanceled()
**/
void DkDocAnalysisViewPort::pickSeedpoint(bool pick) {

	switch(editMode) {
	case mode_pickDistance:
		emit cancelDistanceMeasureRequest();
		distance->resetPoints();
		break;
	}

	if(pick) {
		editMode = mode_pickSeedpoint;
		this->setCursor(Qt::PointingHandCursor);
	} else
		editMode = mode_default;	
}

/**
* Starts the seed points pickcing picking mode if not yet active.
* Cancels any other active modes.
* @param pick start or end the mode
* \sa cancelDistanceMeasureRequest() DkMagicCutToolBar::measureDistanceCanceled(), startPickSeedpointRequest()
**/
void DkDocAnalysisViewPort::pickSeedpoint() {

	switch(editMode) {
	case mode_pickSeedpoint:
		return;
	case mode_pickDistance:
		emit cancelDistanceMeasureRequest();
		distance->resetPoints();
		break;
	}

	editMode = mode_pickSeedpoint;
	emit startPickSeedpointRequest();
	this->setCursor(Qt::PointingHandCursor);
}

/**
* Opens the dialog to save the magic cut, if anything is selected.
* \sa DkMagicCutDialog DkMagicCut
**/
void DkDocAnalysisViewPort::openMagicCutDialog() {

	DkBaseViewPort* viewport = dynamic_cast<DkBaseViewPort*>(parent());
	if (viewport->getImage().isNull()) return;

	// make sure that at least one region is outlined
	if (!magicCut->hasContours()) {
		/*QString msg = tr("Please select at least one region before saving.\n");
		QMessageBox errorDialog(this);
		errorDialog.setWindowTitle("Information");
		errorDialog.setIcon(QMessageBox::Information);
		errorDialog.setText(msg);
		errorDialog.show();
	    errorDialog.exec();*/
		return;
	}

	if (!magicCutDialog) {
		magicCutDialog = new DkMagicCutDialog(magicCut, this, 0);
		connect(magicCutDialog, SIGNAL(savePressed(QImage, int, int, int, int)), this, SLOT(saveMagicCutPressed(QImage, int, int, int, int)));
	}

	bool done = magicCutDialog->exec();

}

/**
* Sets the changed magic cut tolerance in the tool to the new value.
* @param tol The new color tolerance value
* \sa DkMagicCut::setTolerance(int)
**/
void DkDocAnalysisViewPort::setMagicCutTolerance(int tol) {

	magicCut->setTolerance(tol);
}

/**
* Clears all selected regions and resets the region mask in the magic cut tool.
* \sa DkMagicCut::resetRegionMask()
**/
void DkDocAnalysisViewPort::clearMagicCut() {

	magicCut->resetRegionMask();
	emit enableSaveCutSignal(false);
}

/**
* Emits a signal that the magic cut should be saved.
* \a saveMagicCutRequest(QImage, int, int, int, int) DkNoMacs::saveMagicCut(QImage, int, int, int, int)
**/
void DkDocAnalysisViewPort::saveMagicCutPressed(QImage saveImg, int xCoord, int yCoord, int height, int width) {

	//std::cout << "SAVE MAGIC CUT: " << nameAppendix.toStdString() << std::endl;
	emit saveMagicCutRequest(saveImg, xCoord, yCoord, height, width);
}

/**
* Called after the magic cut has been saved, resets masks, displays error message when needed.
* @param saved true if successfully saved, false otherwise
* \sa DkMagicCut::resetRegionMask()
**/
void DkDocAnalysisViewPort::magicCutSaved(bool saved) {

	if(saved) {
		magicCut->resetRegionMask();
		emit enableSaveCutSignal(false);
	}
	else {

		QString msg = tr("Sorry, the magic cut could not be saved\n");
		QMessageBox errorDialog(this);
		errorDialog.setWindowTitle("Error saving magic cut");
		errorDialog.setIcon(QMessageBox::Critical);
		errorDialog.setText(msg);
		errorDialog.show();
	    errorDialog.exec();
	}
}

/**
* Opens the line detection configuration dialog and automatically displays the bottom text lines
* when lines were detected
* \sa DkLineDetectionDialog DkLineDetection
**/
void DkDocAnalysisViewPort::openLineDetectionDialog() {

	DkBaseViewPort* viewport = dynamic_cast<DkBaseViewPort*>(parent());
	if (viewport->getImage().isNull()) return;

	if (!lineDetectionDialog) {
		lineDetectionDialog = new DkLineDetectionDialog(lineDetection, this, 0);
		//connect(magicCutDialog, SIGNAL(savePressed(QImage, QString)), this, SLOT(saveMagicCutPressed(QImage, QString)));
	}

	bool done = lineDetectionDialog->exec();

	if(lineDetection->hasTextLines()) {
		emit enableShowTextLinesSignal(true);
		showBottomTextLines(true);
		
	}
}

/**
* Sets the flag to indicate that the bottom text lines are shown - used for rendering
* @param show Bottom text lines shall be shown/hidden
**/
void DkDocAnalysisViewPort::showBottomTextLines(bool show) {
	showBottomLines = show;
	emit toggleBottomTextLinesButtonSignal(show);
	update();
}

/**
* Sets the flag to indicate that the top text lines are shown - used for rendering
* @param show Top text lines shall be shown/hidden
**/
void DkDocAnalysisViewPort::showTopTextLines(bool show) {
	showTopLines = show;
	emit toggleTopTextLinesButtonSignal(show);
	update();
}

/**
* Sets the flag to indicate that the bottom text lines are shown or not, depending on the
* previous state
**/
void DkDocAnalysisViewPort::showBottomTextLines() {
	showBottomLines = !showBottomLines;
	emit toggleBottomTextLinesButtonSignal(showBottomLines);
	update();
}

/**
* Sets the flag to indicate that the top text lines are shown or not, depending on the
* previous state
**/
void DkDocAnalysisViewPort::showTopTextLines() {
	showTopLines = !showTopLines;
	emit toggleTopTextLinesButtonSignal(showTopLines);
	update();
}



/*-----------------------------------DkDocAnalysisToolBar ---------------------------------------------*/
DkDocAnalysisToolBar::DkDocAnalysisToolBar(const QString & title, QWidget * parent /* = 0 */) : QToolBar(title, parent) {

	createIcons();
	createLayout();
	QMetaObject::connectSlotsByName(this);

	if (DkSettings::display.smallIcons)
		setIconSize(QSize(16, 16));
	else
		setIconSize(QSize(32, 32));

	if (DkSettings::display.toolbarGradient) {

		QColor hCol = DkSettings::display.highlightColor;
		hCol.setAlpha(80);

		setStyleSheet(
			//QString("QToolBar {border-bottom: 1px solid #b6bccc;") +
			QString("QToolBar {border: none; background: QLinearGradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #edeff9, stop: 1 #bebfc7); spacing: 3px; padding: 3px;}")
			+ QString("QToolBar::separator {background: #656565; width: 1px; height: 1px; margin: 3px;}")
			//+ QString("QToolButton:disabled{background-color: rgba(0,0,0,10);}")
			+ QString("QToolButton:hover{border: none; background-color: rgba(255,255,255,80);} QToolButton:pressed{margin: 0px; border: none; background-color: " + DkUtils::colorToString(hCol) + ";}")
			);
	}
	else
		setStyleSheet("QToolBar{spacing: 3px; padding: 3px;}");

	qDebug() << "[DOCANALYSIS TOOLBAR] created...";
}

DkDocAnalysisToolBar::~DkDocAnalysisToolBar() {

	qDebug() << "[DOCANALYSIS TOOLBAR] deleted...";
}

/**
* Enables/Disables All the actions which require a loaded image
**/
void DkDocAnalysisToolBar::enableNoImageActions(bool enable) {

	actions[linedetection_action]->setEnabled(enable);
	actions[distance_action]->setEnabled(enable);
	actions[magic_action]->setEnabled(enable);
}

void DkDocAnalysisToolBar::createIcons() {

	// create icons
	icons.resize(icons_end);

	icons[linedetection_icon] = QIcon(":/nomacsPluginDocAnalysis/img/detect_lines.png");
	icons[showbottomlines_icon] = QIcon(":/nomacsPluginDocAnalysis/img/lower_lines.png");
	icons[showtoplines_icon] = QIcon(":/nomacsPluginDocAnalysis/img/upper_lines.png");
	icons[distance_icon] = QIcon(":/nomacsPluginDocAnalysis/img/distance.png");
	icons[magic_icon] = QIcon(":/nomacsPluginDocAnalysis/img/magic_wand.png");
	icons[savecut_icon] = QIcon(":/nomacsPluginDocAnalysis/img/save_cut.png");
	icons[clearselection_icon] = QIcon(":/nomacsPluginDocAnalysis/img/reset_cut.png");


	if (!DkSettings::display.defaultIconColor) {
		// now colorize all icons
		for (int idx = 0; idx < icons.size(); idx++) {

			icons[idx].addPixmap(DkImage::colorizePixmap(icons[idx].pixmap(100, QIcon::Normal, QIcon::On), DkSettings::display.iconColor), QIcon::Normal, QIcon::On);
			icons[idx].addPixmap(DkImage::colorizePixmap(icons[idx].pixmap(100, QIcon::Normal, QIcon::Off), DkSettings::display.iconColor), QIcon::Normal, QIcon::Off);
		}
	}
}

void DkDocAnalysisToolBar::createLayout() {

	/*QList<QKeySequence> enterSc;
	enterSc.append(QKeySequence(Qt::Key_Enter));
	enterSc.append(QKeySequence(Qt::Key_Return));*/
	actions.resize(actions_end);

	QAction* linedetectionAction = new QAction(icons[linedetection_icon], tr("Detect text lines"), this);
	linedetectionAction->setShortcut(Qt::SHIFT + Qt::Key_D);
	linedetectionAction->setStatusTip(tr("Opens dialog to configure and start the detection of text lines for the current image"));
	linedetectionAction->setCheckable(false);
	linedetectionAction->setChecked(false);
	linedetectionAction->setWhatsThis(tr("alwaysenabled")); // set flag to always make this icon clickable
	linedetectionAction->setObjectName("linedetectionAction");
	actions[linedetection_action] = linedetectionAction;

	QAction* showbottomlinesAction = new QAction(icons[showbottomlines_icon], tr("Show detected bottom text lines"), this);
	showbottomlinesAction->setShortcut(Qt::SHIFT + Qt::Key_L);
	showbottomlinesAction->setStatusTip(tr("Displays the previously calculated lower text lines for the image"));
	showbottomlinesAction->setCheckable(true);
	showbottomlinesAction->setChecked(false);
	showbottomlinesAction->setEnabled(false);
	showbottomlinesAction->setWhatsThis(tr("alwaysenabled")); // set flag to always make this icon clickable
	showbottomlinesAction->setObjectName("showbottomlinesAction");
	actions[showbottomlines_action] = showbottomlinesAction;

	QAction* showtoplinesAction = new QAction(icons[showtoplines_icon], tr("Show detected top text lines"), this);
	showtoplinesAction->setShortcut(Qt::SHIFT + Qt::Key_U);
	showtoplinesAction->setStatusTip(tr("Displays the previously calculated upper text lines for the image"));
	showtoplinesAction->setCheckable(true);
	showtoplinesAction->setChecked(false);
	showtoplinesAction->setEnabled(false);
	showtoplinesAction->setWhatsThis(tr("alwaysenabled")); // set flag to always make this icon clickable
	showtoplinesAction->setObjectName("showtoplinesAction");
	actions[showtoplines_action] = showtoplinesAction;

	QAction* distanceAction = new QAction(icons[distance_icon], tr("Measure distance"), this);
	distanceAction->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_D);
	distanceAction->setStatusTip(tr("Measure the distance between 2 points (in cm)"));
	distanceAction->setCheckable(true);
	distanceAction->setChecked(false);
	distanceAction->setWhatsThis(tr("alwaysenabled")); // set flag to always make this icon clickable
	distanceAction->setObjectName("distanceAction");
	actions[distance_action] = distanceAction;

	QAction* magicAction = new QAction(icons[magic_icon], tr("Select region"), this);
	magicAction->setShortcut(Qt::SHIFT + Qt::Key_S);
	magicAction->setStatusTip(tr("Select regions with similar color"));
	magicAction->setCheckable(true);
	magicAction->setChecked(false);
	magicAction->setWhatsThis(tr("alwaysenabled")); // set flag to always make this icon clickable
	magicAction->setObjectName("magicAction");
	actions[magic_action] = magicAction;

	QAction* savecutAction = new QAction(icons[savecut_icon], tr("Save the selected region"), this);
	savecutAction->setStatusTip(tr("Open the Save Dialog for saving the currently selected region"));
	savecutAction->setCheckable(false);
	savecutAction->setChecked(false);
	savecutAction->setEnabled(false);
	savecutAction->setWhatsThis(tr("alwaysenabled")); // set flag to always make this icon clickable
	savecutAction->setObjectName("savecutAction");
	actions[savecut_action] = savecutAction;

	QAction* clearselectionAction = new QAction(icons[clearselection_icon], tr("Clear selection"), this);
	clearselectionAction->setShortcut(Qt::SHIFT + Qt::Key_C);
	clearselectionAction->setStatusTip(tr("Clear the current selection"));
	clearselectionAction->setCheckable(false);
	clearselectionAction->setChecked(false);
	clearselectionAction->setEnabled(false);
	clearselectionAction->setWhatsThis(tr("alwaysenabled")); // set flag to always make this icon clickable
	clearselectionAction->setObjectName("clearselectionAction");
	actions[clearselection_action] = clearselectionAction;

	QLabel *lbl_tolerance = new QLabel(tr("Tolerance:"));
	lbl_tolerance->setStatusTip(tr("Set the maximum color difference tolerance for flood-filling"));
	lbl_tolerance->setWhatsThis(tr("alwaysenabled")); // set flag to always make this icon clickable

	QSpinBox *toleranceBox = new QSpinBox();
	toleranceBox->setStatusTip(tr("Set the maximum color difference tolerance for flood-filling"));
	toleranceBox->setRange(0, 255);
	toleranceBox->setValue(40);
	toleranceBox->setWhatsThis(tr("alwaysenabled"));
	toleranceBox->setObjectName("toleranceBox");

	// actually add things to interface
	addAction(linedetectionAction);
	addAction(showbottomlinesAction);
	addAction(showtoplinesAction);
	addSeparator();
	addAction(distanceAction);
	addSeparator();
	addAction(magicAction);
	addAction(savecutAction);
	addAction(clearselectionAction);
	addWidget(lbl_tolerance);
	addWidget(toleranceBox);
}

/**
* Make the DocAnalyis toolbar visible
**/
void DkDocAnalysisToolBar::setVisible(bool visible) {

	qDebug() << "[DOCANALYSIS TOOLBAR] set visible: " << visible;

	QToolBar::setVisible(visible);
}

/**
* Called when the detect lines tool icon is clicked.
* Emits signal (a request) to open the configuration dialog for detecting text lines.
* \sa detectLinesSignal() DkDocAnalysisViewPort::openLineDetectionDialog() DkLineDetection
**/
void DkDocAnalysisToolBar::on_linedetectionAction_triggered() {
	emit detectLinesSignal();
}

/**
* Called when the show bottom text lines tool icon is pressed.
* Willd display the previously detected bottom text lines if available.
* Emits signal (a request) to display/hide the lines.
* \sa showBottomTextLinesSignal(bool) DkDocAnalysisViewPort::showBottomTextLines(bool show) DkLineDetection
**/
void DkDocAnalysisToolBar::on_showbottomlinesAction_triggered() {
	emit showBottomTextLinesSignal(actions[showbottomlines_action]->isChecked());
}

/**
* Called when the show top text lines tool icon is pressed.
* Willd display the previously detected top text lines if available.
* Emits signal (a request) to display/hide the lines.
* \sa showTopTextLinesSignal(bool) DkDocAnalysisViewPort::showBottomTextLines(bool show) DkLineDetection
**/
void DkDocAnalysisToolBar::on_showtoplinesAction_triggered() {
	emit showTopTextLinesSignal(actions[showtoplines_action]->isChecked());
}

/**
* Called when the measure distance tool icon is clicked.
* Emits signal (a request) to start/end this tool.
* \sa measureDistanceRequest(bool) DkDocAnalysisViewPort::pickDistancePoint(bool pick) DkDistanceMeasure
**/
void DkDocAnalysisToolBar::on_distanceAction_toggled(bool checked) {
	emit measureDistanceRequest(actions[distance_action]->isChecked());
}

/**
* Called when the magic cut tool icon is clicked.
* Emits signal (a request) to start/end this tool.
* \sa pickSeedpointRequest(bool) DkDocAnalysisViewPort::pickSeedpoint(bool pick) DkMagicCut
**/
void DkDocAnalysisToolBar::on_magicAction_toggled(bool checked) {
	emit pickSeedpointRequest(actions[magic_action]->isChecked());
}

/**
* Called when the currently selected magic cut areas shall be saved (click on the icon)
* \sa openCutDialogSignal() DkDocAnalysisViewPort::openMagicCutDialog() DkMagicCutDialog
**/
void DkDocAnalysisToolBar::on_savecutAction_triggered() {
	emit openCutDialogSignal();
}

/**
* Called when the clear magic cut selection tool icon is clicked.
* Emits signal (a request) to clear all selected magic cut regions.
* \sa clearSelectionSignal() DkDocAnalysisViewPort::clearMagicCut() DkMagicCut
**/
void DkDocAnalysisToolBar::on_clearselectionAction_triggered() {
	emit clearSelectionSignal();
}

/**
* Called when the tolerance value has changed within the toolbar.
* @param value The new tolerance value
* \sa toeranceChanged(int) DkDocAnalysisViewPort::setMagicCutTolerance(int) DkMagicCut::magicwand(QPoint)
**/
void DkDocAnalysisToolBar::on_toleranceBox_valueChanged(int value) {
	emit toleranceChanged(value);
}


/**
* Slot - called when the user canceles during picking a seedpoint in the magic cut tool.
* Untoggles the corresponding tool icon.
**/
void DkDocAnalysisToolBar::pickSeedpointCanceled() {
	actions[magic_action]->setChecked(false);
}

/**
* Slot - called when the user starts the picking seedpoints for magic cut tool (e.g. via shortcut).
* Toggles the corresponding tool icon.
**/
void DkDocAnalysisToolBar::pickSeedpointStarted() {
	actions[magic_action]->setChecked(true);
}

/**
* Slot - called when the user canceles during picking a distance measure point.
* Untoggles the corresponding tool icon.
**/
void DkDocAnalysisToolBar::measureDistanceCanceled() {
	actions[distance_action]->setChecked(false);
}

/**
* Slot - called when the user requests to measure the distance (e.g. via shortcut).
* Toggles the corresponding tool icon.
**/
void DkDocAnalysisToolBar::measureDistanceStarted() {
	actions[distance_action]->setChecked(true);
}

/**
* Slot - called when the icon should be enabled or disabled (e.g. nothing selected, no image, ...)
**/
void DkDocAnalysisToolBar::enableButtonSaveCut(bool enable) {
	actions[savecut_action]->setEnabled(enable);
	// also enable the clear selection, since something has been selected
	actions[clearselection_action]->setEnabled(enable);
}

/**
* Slot - called when the icon should be enabled or disabled (e.g. no image, no text lines detected...)
**/
void DkDocAnalysisToolBar::enableButtonShowTextLines(bool enable) {
	actions[showbottomlines_action]->setEnabled(enable);
	actions[showtoplines_action]->setEnabled(enable);
}

/**
* Slot - called when after calculation of text lines the bottom text lines are automatically
* displayed to also toggle the corresponding icon
**/
void DkDocAnalysisToolBar::toggleBottomTextLinesButton(bool toggle) {
	actions[showbottomlines_action]->setChecked(toggle);
}

/**
* Slot - called when after calculation of text lines the top text lines are automatically
* enabled, in case they have been enabled in a previous text line calculation
**/
void DkDocAnalysisToolBar::toggleTopTextLinesButton(bool toggle) {
	actions[showtoplines_action]->setChecked(toggle);
}


};
