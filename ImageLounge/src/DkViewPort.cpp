/*******************************************************************************************************
 DkViewPort.cpp
 Created on:	05.05.2011
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2012 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2012 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2012 Florian Kleber <florian@nomacs.org>

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

#include "DkViewPort.h"

namespace nmc {

// DkControlWidget --------------------------------------------------------------------
DkControlWidget::DkControlWidget(DkViewPort *parent, Qt::WFlags flags) : QWidget(parent, flags) {

	viewport = parent;

	rating = -1;
	
	// cropping
	editRect = new DkEditableRect(QRectF(), this);

	// thumbnails, metadata
	filePreview = new DkFilePreview(this, flags);
	metaDataInfo = new DkMetaDataInfo(this);
	overviewWindow = new DkOverview(this);
	player = new DkPlayer(this);
	addActions(player->getActions().toList());

	// file info - overview
	fileInfoLabel = new DkFileInfoLabel(this);
	ratingLabel = new DkRatingLabelBg(2, this, flags);
	addActions(ratingLabel->getActions().toList());		// register actions

	// delayed info
	delayedInfo = new DkDelayedMessage();
	delayedSpinner = new DkDelayedInfo(0);

	// info labels
	spinnerLabel = new DkAnimationLabel(":/nomacs/img/loading.gif", this);
	centerLabel = new DkLabelBg(this, "");
	bottomLabel = new DkLabelBg(this, "");
	bottomLeftLabel = new DkLabelBg(this, "");

	// wheel label
	QPixmap wp = QPixmap(":/nomacs/img/thumbs-move.png");
	wheelButton = new QLabel(this);
	wheelButton->setAttribute(Qt::WA_TransparentForMouseEvents);
	wheelButton->setPixmap(wp);
	wheelButton->adjustSize();
	wheelButton->hide();

	// image histogram
	histogram = new DkHistogram(this);

	init();
	connectWidgets();
}

void DkControlWidget::init() {

	//// debug: show invisible widgets
	//setStyleSheet("QWidget{background-color: QColor(0,0,0,20); border: 1px solid #000000;}");
	setFocusPolicy(Qt::StrongFocus);
	setFocus(Qt::TabFocusReason);
	setMouseTracking(true);
	
	// connect widgets with their settings
	filePreview->setDisplaySettings(&DkSettings::App::showFilePreview);
	metaDataInfo->setDisplaySettings(&DkSettings::App::showMetaData);
	fileInfoLabel->setDisplaySettings(&DkSettings::App::showFileInfoLabel);
	player->setDisplaySettings(&DkSettings::App::showPlayer);
	histogram->setDisplaySettings(&DkSettings::App::showHistogram);

	// some adjustments
	bottomLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	bottomLeftLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	ratingLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	centerLabel->setAlignment(Qt::AlignCenter);
	overviewWindow->setContentsMargins(10, 10, 0, 0);
	editRect->setMaximumSize(16777215, 16777215);		// max widget size, why is it a 24 bit int??
	editRect->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	spinnerLabel->halfSize();

	// dummy
	QWidget* dw = new QWidget();
	dw->setMouseTracking(true);
	QBoxLayout* dLayout = new QBoxLayout(QBoxLayout::LeftToRight, dw);
	dLayout->setContentsMargins(0,0,0,0);
	dLayout->addWidget(bottomLabel);
	dLayout->addStretch();

	// zoom widget
	QWidget* bw = new QWidget();
	bw->setMouseTracking(true);
	bw->setMinimumHeight(40);
	bw->setMaximumHeight(80);
	QBoxLayout* zLayout = new QBoxLayout(QBoxLayout::TopToBottom, bw);
	zLayout->setContentsMargins(0,0,0,0);
	zLayout->setSpacing(0);
	zLayout->addWidget(bottomLabel);
	zLayout->addWidget(bottomLeftLabel);
	zLayout->addStretch();

	// left column widget
	QWidget* leftWidget = new QWidget();
	leftWidget->setMouseTracking(true);
	QBoxLayout* ulLayout = new QBoxLayout(QBoxLayout::TopToBottom, leftWidget);
	ulLayout->setContentsMargins(0,0,0,0);
	ulLayout->setSpacing(0);
	ulLayout->addWidget(overviewWindow);
	ulLayout->addStretch();
	ulLayout->addWidget(bw);
	ulLayout->addWidget(dw);

	// center column
	QWidget* cW = new QWidget();
	cW->setMouseTracking(true);
	QBoxLayout* cwLayout = new QBoxLayout(QBoxLayout::LeftToRight, cW);
	cwLayout->setContentsMargins(0,0,0,0);
	cwLayout->addStretch();
	cwLayout->addWidget(centerLabel);
	cwLayout->addWidget(spinnerLabel);
	cwLayout->addStretch();

	// center player horizontally
	QWidget* cP = new QWidget();
	cP->setMouseTracking(true);
	QBoxLayout* cpLayout = new QBoxLayout(QBoxLayout::LeftToRight, cP);
	cpLayout->setContentsMargins(0,0,0,0);
	cpLayout->addWidget(player);

	// center column
	QWidget* center = new QWidget();
	center->setMouseTracking(true);
	QBoxLayout* cLayout = new QBoxLayout(QBoxLayout::TopToBottom, center);
	cLayout->setContentsMargins(0,0,0,0);
	cLayout->addStretch();
	cLayout->addWidget(cW);
	cLayout->addStretch();
	cLayout->addWidget(cP);
	
	// rating widget
	QWidget* rw = new QWidget();
	rw->setMouseTracking(true);
	rw->setMinimumSize(0,0);
	QBoxLayout* rLayout = new QBoxLayout(QBoxLayout::RightToLeft, rw);
	rLayout->setContentsMargins(0,0,0,17);
	rLayout->addWidget(ratingLabel);
	rLayout->addStretch();

	// file info
	QWidget* fw = new QWidget();
	fw->setContentsMargins(0,0,0,30);
	fw->setMouseTracking(true);
	fw->setMinimumSize(0,0);
	QBoxLayout* rwLayout = new QBoxLayout(QBoxLayout::RightToLeft, fw);
	rwLayout->setContentsMargins(0,0,0,0);
	rwLayout->addWidget(fileInfoLabel);
	rwLayout->addStretch();

	// right column
	QWidget* hw = new QWidget();
	hw->setContentsMargins(0,10,10,0);
	hw->setMouseTracking(true);
	QBoxLayout* hwLayout = new QBoxLayout(QBoxLayout::RightToLeft, hw);
	hwLayout->setContentsMargins(0,0,0,0);
	hwLayout->addWidget(histogram);
	hwLayout->addStretch();

	// right column
	QWidget* rightWidget = new QWidget();
	rightWidget->setMouseTracking(true);
	QBoxLayout* lrLayout = new QBoxLayout(QBoxLayout::TopToBottom, rightWidget);
	lrLayout->setContentsMargins(0,0,0,0);
	lrLayout->addWidget(hw);
	lrLayout->addStretch();
	lrLayout->addWidget(fw);
	lrLayout->addWidget(rw);

	// init both main widgets
	hudWidget = new QWidget(this);
	hudWidget->setMouseTracking(true);
	editWidget = new QWidget(this);
	editWidget->setMouseTracking(true);
	editWidget->hide();

	// global controller layout
	QGridLayout* hudLayout = new QGridLayout(hudWidget);
	hudLayout->setContentsMargins(0,0,0,0);
	hudLayout->setSpacing(0);

	// add elements
	hudLayout->addWidget(filePreview, top, left, 1, hor_pos_end);
	hudLayout->addWidget(metaDataInfo, bottom, left, 1, hor_pos_end);
	hudLayout->addWidget(leftWidget, ver_center, left, 1, 1);
	hudLayout->addWidget(center, ver_center, hor_center, 1, 1);
	hudLayout->addWidget(rightWidget, ver_center, right, 1, 1);
		
	// we need to put everything into extra widgets (which are exclusive) in order to handle the mouse events correctly
	QHBoxLayout* editLayout = new QHBoxLayout(editWidget);
	editLayout->setContentsMargins(0,0,0,0);
	editLayout->addWidget(editRect);

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0,0,0,0);
	layout->addWidget(hudWidget);
	layout->addWidget(editWidget);

	//// TODO: remove...
	//centerLabel->setText("ich bin richtig...", -1);
	//bottomLeftLabel->setText("topLeft label...", -1);
	//spinnerLabel->show();
	
	show();
	qDebug() << "controller initialized...";
}

void DkControlWidget::connectWidgets() {

	if (!viewport)
		return;

	DkImageLoader* loader = viewport->getImageLoader();

	if (loader) {
		qDebug() << "loader slots connected";

		connect(loader, SIGNAL(updateDirSignal(QFileInfo, bool)), filePreview, SLOT(updateDir(QFileInfo, bool)));
		connect(loader, SIGNAL(updateFileSignal(QFileInfo, QSize)), metaDataInfo, SLOT(setFileInfo(QFileInfo, QSize)));
		connect(loader, SIGNAL(updateFileSignal(QFileInfo, QSize, bool)), this, SLOT(setFileInfo(QFileInfo, QSize, bool)));

		connect(loader, SIGNAL(updateInfoSignal(QString, int, int)), this, SLOT(setInfo(QString, int, int)));
		connect(loader, SIGNAL(updateInfoSignalDelayed(QString, bool, int)), this, SLOT(setInfoDelayed(QString, bool, int)));
		connect(loader, SIGNAL(updateSpinnerSignalDelayed(bool, int)), this, SLOT(setSpinnerDelayed(bool, int)));

		connect(loader, SIGNAL(setPlayer(bool)), player, SLOT(play(bool)));
	}

	// overview
	connect(filePreview, SIGNAL(loadFileSignal(QFileInfo)), viewport, SLOT(loadFile(QFileInfo)));
	connect(overviewWindow, SIGNAL(moveViewSignal(QPointF)), viewport, SLOT(moveView(QPointF)));
	connect(overviewWindow, SIGNAL(sendTransformSignal()), viewport, SLOT(tcpSynchronize()));

	// waiting
	connect(delayedInfo, SIGNAL(infoSignal(QString, int)), this, SLOT(setInfo(QString, int)));
	connect(delayedSpinner, SIGNAL(infoSignal(int)), this, SLOT(setSpinner(int)));
	
	// rating
	connect(fileInfoLabel->getRatingLabel(), SIGNAL(newRatingSignal(int)), this, SLOT(updateRating(int)));
	connect(ratingLabel, SIGNAL(newRatingSignal(int)), this, SLOT(updateRating(int)));
	connect(ratingLabel, SIGNAL(newRatingSignal(int)), metaDataInfo, SLOT(setRating(int)));

	// playing
	connect(player, SIGNAL(previousSignal(bool)), viewport, SLOT(loadPrevFile(bool)));
	connect(player, SIGNAL(nextSignal(bool)), viewport, SLOT(loadNextFile(bool)));

	// cropping
	connect(editRect, SIGNAL(enterPressedSignal(DkRotatingRect)), viewport, SLOT(cropImage(DkRotatingRect)));
}

void DkControlWidget::update() {

	overviewWindow->update();

	QWidget::update();
}

void DkControlWidget::showWidgetsSettings() {

	if (viewport->getImage().isNull()) {
		showPreview(false);
		showMetaData(false);
		showFileInfo(false);
		showPlayer(false);
		overviewWindow->hide();
		showHistogram(false);
	}

	qDebug() << "current app mode: " << DkSettings::App::currentAppMode;

	showPreview(filePreview->getCurrentDisplaySetting());
	showMetaData(metaDataInfo->getCurrentDisplaySetting());
	showFileInfo(fileInfoLabel->getCurrentDisplaySetting());
	showPlayer(player->getCurrentDisplaySetting());
	showHistogram(histogram->getCurrentDisplaySetting());
}

void DkControlWidget::showPreview(bool visible) {

	if (!filePreview)
		return;

	if (visible && !filePreview->isVisible())
		filePreview->show();
	else if (!visible && filePreview->isVisible())
		filePreview->hide();
}

void DkControlWidget::showMetaData(bool visible) {

	qDebug() << "[DkMetaData] showing: " << visible;

	if (!metaDataInfo)
		return;

	if (visible && !metaDataInfo->isVisible()) {
		metaDataInfo->show();
		qDebug() << "showing metadata...";
	}
	else if (!visible && metaDataInfo->isVisible())
		metaDataInfo->hide();
}

void DkControlWidget::showFileInfo(bool visible) {

	if (!fileInfoLabel)
		return;

	if (visible && !fileInfoLabel->isVisible()) {
		fileInfoLabel->show();
		ratingLabel->block(fileInfoLabel->isVisible());
	}
	else if (!visible && fileInfoLabel->isVisible()) {
		fileInfoLabel->hide();
		ratingLabel->block(false);
	}
}

void DkControlWidget::showPlayer(bool visible) {

	if (!player)
		return;

	if (visible)
		player->show();
	else
		player->hide();
}

void DkControlWidget::showOverview(bool visible) {

	if (!overviewWindow)
		return;

	// viewport decides whether to show overview or not
	DkSettings::App::showOverview.setBit(DkSettings::App::currentAppMode, visible);

	if (visible && !overviewWindow->isVisible()) {		
		viewport->update();
	}
	else if (!visible && overviewWindow->isVisible()) {
		overviewWindow->hide();
	}

}

void DkControlWidget::showCrop(bool visible) {

	if (visible && !editWidget->isVisible()) {
		editWidget->show();
		hudWidget->hide();

		editRect->reset();
		editRect->show();
	}
	else if (!visible && editWidget->isVisible()) {
		editWidget->hide();
		editRect->hide();
		hudWidget->show();

		// ok, this is really nasty... however, the fileInfo layout is destroyed otherwise
		if (fileInfoLabel->isVisible()) {
			fileInfoLabel->setVisible(false);
			showFileInfo(true);
		}
	}

}

void DkControlWidget::showHistogram(bool visible) {
	
	if (!histogram)
		return;

	if (visible && !histogram->isVisible()) {
		histogram->show();
		if(!viewport->getImage().isNull()) histogram->drawHistogram(viewport->getImage());
		else  histogram->clearHistogram();
	}
	else if (!visible && histogram->isVisible()) {
		histogram->hide();
	}

}

void DkControlWidget::setFileInfo(QFileInfo fileInfo, QSize size, bool edited) {

	qDebug() << "file info set...";

	//// TODO: this is a fast fix
	//// if this thread uses the static metadata object 
	//// nomacs crashes when images are loaded fast (2 threads try to access DkMetaData simultaneously)
	//// currently we need to read the metadata twice (not nice either)
	DkImageLoader::imgMetaData.setFileName(fileInfo);

	QString dateString = QString::fromStdString(DkImageLoader::imgMetaData.getExifValue("DateTimeOriginal"));
	fileInfoLabel->updateInfo(fileInfo, dateString, DkImageLoader::imgMetaData.getRating());
	fileInfoLabel->setEdited(edited);
	updateRating(DkImageLoader::imgMetaData.getRating());
}

void DkControlWidget::setInfo(QString msg, int time, int location) {

	if (location == center_label && centerLabel)
		centerLabel->setText(msg, time);
	else if (location == bottom_left_label && bottomLabel)
		bottomLabel->setText(msg, time);
	else if (location == top_left_label && bottomLeftLabel)
		bottomLeftLabel->setText(msg, time);

	update();
}

void DkControlWidget::setInfoDelayed(QString msg, bool start, int delayTime) {

	if (!centerLabel)
		return;

	if (start)
		delayedInfo->setInfo(msg, delayTime);
	else
		delayedInfo->stop();

}

void DkControlWidget::setSpinner(int time) {

	if (spinnerLabel)
		spinnerLabel->showTimed(time);
}

void DkControlWidget::setSpinnerDelayed(bool start, int time) {

	if (!spinnerLabel) 
		return;

	if (start)
		delayedSpinner->setInfo(time);
	else
		delayedSpinner->stop();
}


void DkControlWidget::stopLabels() {

	centerLabel->stop();
	bottomLabel->stop();
	//topLeftLabel->stop();
	spinnerLabel->stop();

	showCrop(false);
}

void DkControlWidget::settingsChanged() {

	if (fileInfoLabel && fileInfoLabel->isVisible()) {
		showFileInfo(false);	// just a hack but all states are preserved this way
		showFileInfo(true);
	}

}

void DkControlWidget::updateRating(int rating) {

	this->rating = rating;

	ratingLabel->setRating(rating);

	if (fileInfoLabel)
		fileInfoLabel->updateRating(rating);
}

void DkControlWidget::imageLoaded(bool loaded) {

	showWidgetsSettings();
}

void DkControlWidget::setFullScreen(bool fullscreen) {

	showWidgetsSettings();

	if (fullscreen &&!player->isVisible())
		player->show(3000);		
}

// DkControlWidget - Events --------------------------------------------------------------------
void DkControlWidget::mousePressEvent(QMouseEvent *event) {

	qDebug() << "has mouse tracking: " << hasMouseTracking();

	enterPos = event->pos();

	if (filePreview && filePreview->isVisible() && event->buttons() == Qt::MiddleButton) {

		QTimer* mImgTimer = filePreview->getMoveImageTimer();
		mImgTimer->start(1);

		// show icon
		wheelButton->move(event->pos().x()-16, event->pos().y()-16);
		wheelButton->show();
	}

	QWidget::mousePressEvent(event);
}

void DkControlWidget::mouseReleaseEvent(QMouseEvent *event) {

	if (filePreview && filePreview->isVisible()) {
		filePreview->setCurrentDx(0);
		QTimer* mImgTimer = filePreview->getMoveImageTimer();
		mImgTimer->stop();
		wheelButton->hide();
	}

	QWidget::mouseReleaseEvent(event);
}

void DkControlWidget::mouseMoveEvent(QMouseEvent *event) {

	// scroll thumbs preview
	if (filePreview && filePreview->isVisible() && event->buttons() == Qt::MiddleButton) {
		
		float dx = std::fabs(enterPos.x() - event->pos().x())*0.015f;
		dx = std::exp(dx);
		if (enterPos.x() - event->pos().x() < 0)
			dx = -dx;

		filePreview->setCurrentDx(dx);	// update dx
	}

	QWidget::mouseMoveEvent(event);
}



void DkControlWidget::keyPressEvent(QKeyEvent *event) {
	
	// conflicting with ESC in fullscreen
	//int mode = DkSettings::AppSettings::currentAppMode;
	//if (event->key() == Qt::Key_Escape && 
	//	(mode == DkSettings::mode_default || mode == DkSettings::mode_frameless || mode == DkSettings::mode_contrast)) {
	//	if (filePreview->isVisible()) {
	//		filePreview->hide();
	//	}
	//	if (metaDataInfo->isVisible()) {
	//		metaDataInfo->hide();
	//	}
	//}

	QWidget::keyPressEvent(event);
}

void DkControlWidget::keyReleaseEvent(QKeyEvent *event) {

	QWidget::keyReleaseEvent(event);
}


// DkBaseViewport --------------------------------------------------------------------
DkBaseViewPort::DkBaseViewPort(QWidget *parent, Qt::WFlags flags) : QGraphicsView(parent) {
	
	this->parent = parent;
	viewportRect = QRect(0, 0, width(), height());
	worldMatrix.reset();
	imgMatrix.reset();

	blockZooming = false;
	altMod = DkSettings::Global::altMod;
	ctrlMod = DkSettings::Global::ctrlMod;

	zoomTimer = new QTimer(this);
	zoomTimer->setSingleShot(true);
	connect(zoomTimer, SIGNAL(timeout()), this, SLOT(stopBlockZooming()));

	setObjectName(QString::fromUtf8("DkBaseViewPort"));

	setStyleSheet("QGraphicsView { border-style: none; background: QLinearGradient(x1: 0, y1: 0.7, x2: 0, y2: 1, stop: 0 #edeff9, stop: 1 #d9dbe4);}" );
	setMouseTracking(true);

}

DkBaseViewPort::~DkBaseViewPort() {

	release();
}

void DkBaseViewPort::release() {
}

// zoom - pan --------------------------------------------------------------------
void DkBaseViewPort::resetView() {

	worldMatrix.reset();
	changeCursor();

	update();
}

void DkBaseViewPort::fullView() {

	worldMatrix.reset();
	zoom(1.0f/imgMatrix.m11());
	changeCursor();
	
	update();
}

void DkBaseViewPort::shiftLeft() {

	float delta = 2*width()/(100.0*worldMatrix.m11());
	moveView(QPointF(delta,0));
}

void DkBaseViewPort::shiftRight() {

	float delta = -2*width()/(100.0*worldMatrix.m11());
	moveView(QPointF(delta,0));
}

void DkBaseViewPort::shiftUp() {

	float delta = 2*height()/(100.0*worldMatrix.m11());
	moveView(QPointF(0,delta));
}

void DkBaseViewPort::shiftDown() {

	float delta = -2*height()/(100.0*worldMatrix.m11());
	moveView(QPointF(0,delta));
}

void DkBaseViewPort::moveView(QPointF delta) {

	QRectF imgWorldRect = worldMatrix.mapRect(imgViewRect);
	if (imgWorldRect.width() < this->width())
		delta.setX(0);
	if (imgWorldRect.height() < this->height())
		delta.setY(0);

	worldMatrix.translate(delta.x(), delta.y());
	controlImagePosition();
	update();
}


void DkBaseViewPort::zoomIn() {

	zoom(1.1f);
}

void DkBaseViewPort::zoomOut() {

	zoom(0.9f);
}

void DkBaseViewPort::zoom(float factor, QPointF center) {

	if (imgQt.isNull())
		return;

	//factor/=5;//-0.1 <-> 0.1
	//factor+=1;//0.9 <-> 1.1

	//limit zoom out ---
	if (worldMatrix.m11()*factor < 0.1 && factor < 1)
		return;

	//if (worldMatrix.m11()*factor < 1) {
	//	resetView();
	//	return;
	//}

	// reset view & block if we pass the 'image fit to screen' on zoom out
	if (worldMatrix.m11() > 1 && worldMatrix.m11()*factor < 1) {

		blockZooming = true;
		zoomTimer->start(500);
		resetView();
		return;
	}

	// reset view if we pass the 'image fit to screen' on zoom in
	if (worldMatrix.m11() < 1 && worldMatrix.m11()*factor > 1) {

		resetView();
		return;
	}

	//limit zoom in ---
	if (worldMatrix.m11()*imgMatrix.m11() > 50 && factor > 1)
		return;

	// if no center assigned: zoom in at the image center
	if (center.x() == -1 || center.y() == -1)
		center = imgViewRect.center();

	//inverse the transform
	int a, b;
	worldMatrix.inverted().map(center.x(), center.y(), &a, &b);

	worldMatrix.translate(a-factor*a, b-factor*b);
	worldMatrix.scale(factor, factor);

	controlImagePosition();
	changeCursor();

	update();
}

void DkBaseViewPort::stopBlockZooming() {
	blockZooming = false;
}

// set image --------------------------------------------------------------------
#ifdef WITH_OPENCV
void DkBaseViewPort::setImage(cv::Mat newImg) {

	//if (this->img.size() != newImg.size()) {
	//	// do we really need a deep copy here?

	// TODO: be careful with the format!
	imgQt = QImage(newImg.data, newImg.cols, newImg.rows, newImg.step, QImage::Format_RGB888);
	//}

	this->imgRect = QRect(0, 0, newImg.cols, newImg.rows);


	if (!DkSettings::Display::keepZoom)
		worldMatrix.reset();

	updateImageMatrix();

	if (DkSettings::Display::keepZoom)
		centerImage();

	update();
}
#endif

void DkBaseViewPort::setImage(QImage newImg) {

	imgQt = newImg;
	QRectF oldImgRect = imgRect;
	this->imgRect = QRectF(0, 0, newImg.width(), newImg.height());
	imgPyramid.clear();

	emit enableNoImageSignal(!imgQt.isNull());

	if (!DkSettings::Display::keepZoom || imgRect != oldImgRect)
		worldMatrix.reset();							

	updateImageMatrix();
	update();
}

QImage DkBaseViewPort::getImage() {

	return imgQt;
}

QRectF DkBaseViewPort::getImageViewRect() {
	
	return worldMatrix.mapRect(imgViewRect);
}

void DkBaseViewPort::unloadImage() {
}

// events --------------------------------------------------------------------
void DkBaseViewPort::paintEvent(QPaintEvent* event) {

	QPainter painter(viewport());

	if (!imgQt.isNull()) {
		painter.setWorldTransform(worldMatrix);

		if (imgMatrix.m11()*worldMatrix.m11() <= (float)DkSettings::Display::interpolateZoomLevel/100.0f)
			painter.setRenderHint(QPainter::SmoothPixmapTransform);

		draw(&painter);

		//Now disable matrixWorld for overlay display
		painter.setWorldMatrixEnabled(false);
	}

	painter.end();

	// propagate
	QGraphicsView::paintEvent(event);

}

void DkBaseViewPort::resizeEvent(QResizeEvent *event) {

	viewportRect = QRect(0, 0, width(), height());

	// do we still need that??
	QSize newSize = imgQt.size();
	newSize.scale(event->size(), Qt::IgnoreAspectRatio);

	newSize = (event->size()-newSize)/2;
	move(newSize.width(), newSize.height());

	updateImageMatrix();
	centerImage();
	changeCursor();

	return QGraphicsView::resizeEvent(event);
}

// key events --------------------------------------------------------------------
void DkBaseViewPort::keyPressEvent(QKeyEvent* event) {

	QWidget::keyPressEvent(event);
}

void DkBaseViewPort::keyReleaseEvent(QKeyEvent* event) {

#ifdef DK_DLL
	if (!event->isAutoRepeat())
		emit keyReleaseSignal(event);	// make key presses available
		//emit enableNoImageSignal(true);
#endif

	QWidget::keyReleaseEvent(event);
}

// mouse events --------------------------------------------------------------------
void DkBaseViewPort::mousePressEvent(QMouseEvent *event) {

	// ok, start panning
	if (worldMatrix.m11() > 1 && !imageInside() && event->buttons() == Qt::LeftButton) {
		setCursor(Qt::ClosedHandCursor);
		posGrab = event->pos();
	}

	QWidget::mousePressEvent(event);
}

void DkBaseViewPort::mouseReleaseEvent(QMouseEvent *event) {

	if (worldMatrix.m11() > 1 && !imageInside())
		setCursor(Qt::OpenHandCursor);

	QWidget::mouseReleaseEvent(event);
}

void DkBaseViewPort::mouseDoubleClickEvent(QMouseEvent *event) {

	QCoreApplication::sendEvent(parent, event);
}

void DkBaseViewPort::mouseMoveEvent(QMouseEvent *event) {

	if (worldMatrix.m11() > 1 && event->buttons() == Qt::LeftButton) {

		QPointF cPos = event->pos();
		QPointF dxy = (cPos - posGrab);
		posGrab = cPos;
		moveView(dxy/worldMatrix.m11());
	}
	if (event->buttons() != Qt::LeftButton && event->buttons() != Qt::RightButton) {

		if (event->modifiers() == ctrlMod && event->modifiers() != altMod)
			setCursor(Qt::CrossCursor);
		else if (worldMatrix.m11() > 1 && !imageInside())
			setCursor(Qt::OpenHandCursor);
		else
			unsetCursor();

	}


	QWidget::mouseMoveEvent(event);
}

void DkBaseViewPort::wheelEvent(QWheelEvent *event) {

	float factor = -event->delta();
	if (DkSettings::Display::invertZoom) factor *= -1.0f;

	factor /= -1200.0f;
	factor += 1.0f;

	zoom( factor, event->pos());
}

void DkBaseViewPort::contextMenuEvent(QContextMenuEvent *event) {

	// send this event to my parent...
	QWidget::contextMenuEvent(event);
}

// protected functions --------------------------------------------------------------------
void DkBaseViewPort::draw(QPainter *painter) {

	//QImage imgDraw = getScaledImage(imgMatrix.m11()*worldMatrix.m11());
	//painter->drawImage(imgViewRect, imgDraw, QRect(QPoint(), imgDraw.size()));
	if (parent->isFullScreen()) {
		painter->setWorldMatrixEnabled(false);
		painter->fillRect(QRect(QPoint(), size()), DkSettings::SlideShow::backgroundColor);
		painter->setWorldMatrixEnabled(true);
	}

	painter->drawImage(imgViewRect, imgQt, imgRect);

}

bool DkBaseViewPort::imageInside() {

	return viewportRect.contains(worldMatrix.mapRect(imgViewRect));
}

void DkBaseViewPort::updateImageMatrix() {

	if (imgQt.isNull())
		return;

	QRectF oldImgRect = imgViewRect;
	QTransform oldImgMatrix = imgMatrix;

	imgMatrix.reset();

	// if the image is smaller or zoom is active: paint the image as is
	if (!viewportRect.contains(imgRect))
		imgMatrix = getScaledImageMatrix();
	else {
		imgMatrix.translate((float)(width()-imgQt.width())*0.5f, (float)(height()-imgQt.height())*0.5f);
		imgMatrix.scale(1.0f, 1.0f);
	}

	imgViewRect = imgMatrix.mapRect(imgRect);

	// update world matrix
	if (worldMatrix.m11() != 1) {

		float scaleFactor = oldImgMatrix.m11()/imgMatrix.m11();
		double dx = oldImgRect.x()/scaleFactor-imgViewRect.x();
		double dy = oldImgRect.y()/scaleFactor-imgViewRect.y();

		worldMatrix.scale(scaleFactor, scaleFactor);
		worldMatrix.translate(dx, dy);
	}
}

QTransform DkBaseViewPort::getScaledImageMatrix() {

	// the image resizes as we zoom
	float ratioImg = imgRect.width()/imgRect.height();
	float ratioWin = (float)this->width()/(float)this->height();

	QTransform imgMatrix;
	float s;
	if (imgRect.width() == 0 || imgRect.height() == 0)
		s = 1.0f;
	else
		s = (ratioImg > ratioWin) ? (float)width()/imgRect.width() : (float)height()/imgRect.height();

	imgMatrix.scale(s, s);

	QRectF imgViewRect = imgMatrix.mapRect(imgRect);
	imgMatrix.translate((width()-imgViewRect.width())*0.5f/s, (height()-imgViewRect.height())*0.5f/s);

	return imgMatrix;
}

QImage DkBaseViewPort::getScaledImage(float factor) {

// this function does not help anything if we cannot interpolate with OpenCV
#ifndef WITH_OPENCV
	return imgQt;
#endif

	if (factor > 0.5f)
		return imgQt;

	int divisor = DkMath::getNextPowerOfTwoDivisior(factor);

	//if (divisor < 2)
	//	return imgQt;

	// is the image cached already?
	if (imgPyramid.contains(divisor))
		return imgPyramid.value(divisor);

	QSize newSize = imgQt.size()*1.0f/(float)divisor;

	//// caching should not consume more than 30 MB
	//if (newSize.width()*newSize.height() > 30 * 2^20)
	//	return imgQt;

#ifdef WITH_OPENCV

	Mat resizeImage = DkImage::qImage2Mat(imgQt);

	// is the image convertible?
	if (resizeImage.empty())
		return imgQt;

	Mat tmp;
	cv::resize(resizeImage, tmp, cv::Size(newSize.width(), newSize.height()), 0, 0, CV_INTER_AREA);
	resizeImage = tmp;
	QImage iplImg = DkImage::mat2QImage(resizeImage);

	imgPyramid.insert(divisor, iplImg);

	return iplImg;
#endif

	return imgQt;
}

void DkBaseViewPort::controlImagePosition(float lb, float ub) {
	// dummy method
}

void DkBaseViewPort::centerImage() {

	QRectF imgWorldRect = worldMatrix.mapRect(imgViewRect);
	float dx, dy;

	// if black border - center the image
	if (imgWorldRect.width() < (float)width()) {
		dx = ((float)width()-imgWorldRect.width())*0.5f-imgViewRect.x()*worldMatrix.m11();
		dx = (dx-worldMatrix.dx())/worldMatrix.m11();
		worldMatrix.translate(dx, 0);
	}
	else if (imgWorldRect.left() > 0)
		worldMatrix.translate(-imgWorldRect.left()/worldMatrix.m11(), 0);
	else if (imgWorldRect.right() < width())
		worldMatrix.translate((width()-imgWorldRect.right())/worldMatrix.m11(), 0);

	if (imgWorldRect.height() < height()) {
		dy = (height()-imgWorldRect.height())*0.5f-imgViewRect.y()*worldMatrix.m22();
		dy = (dy-worldMatrix.dy())/worldMatrix.m22();
		worldMatrix.translate(0, dy);
	}
	else if (imgWorldRect.top() > 0) {
		worldMatrix.translate(0, -imgWorldRect.top()/worldMatrix.m22());
	}
	else if (imgWorldRect.bottom() < height()) {
		worldMatrix.translate(0, (height()-imgWorldRect.bottom())/worldMatrix.m22());
	}
}

void DkBaseViewPort::changeCursor() {

	if (worldMatrix.m11() > 1 && !imageInside())
		setCursor(Qt::OpenHandCursor);
	else
		unsetCursor();
}

// DkViewPort --------------------------------------------------------------------
DkViewPort::DkViewPort(QWidget *parent, Qt::WFlags flags) : DkBaseViewPort(parent) {

	testLoaded = false;
	thumbLoaded = false;
	visibleStatusbar = false;

	imgBg = QImage();
	imgBg.load(":/nomacs/img/nomacs-bg.png");

	loader = 0;
			
	setAcceptDrops(true);
	setObjectName(QString::fromUtf8("DkViewPort"));

	//no border
	setStyleSheet( "QGraphicsView { border-style: none; background: QLinearGradient(x1: 0, y1: 0.7, x2: 0, y2: 1, stop: 0 #edeff9, stop: 1 #d9dbe4); }" );
	setMouseTracking (true);//receive mouse event everytime
	
	loader = new DkImageLoader();

	controller = new DkControlWidget(this, flags);
	controller->show();

	controller->getOverview()->setTransforms(&worldMatrix, &imgMatrix);
	controller->getEditRect()->setWorldTransform(&worldMatrix);
	controller->getEditRect()->setImageTransform(&imgMatrix);
	controller->getEditRect()->setImageRect(&imgViewRect);

	connect(loader, SIGNAL(updateImageSignal()), this, SLOT(updateImage()), Qt::QueuedConnection);
	connect(loader, SIGNAL(fileNotLoadedSignal(QFileInfo)), this, SLOT(fileNotLoaded(QFileInfo)));
	connect(this, SIGNAL(enableNoImageSignal(bool)), controller, SLOT(imageLoaded(bool)));
	
	qDebug() << "viewer created...";

	//// >DIR:  [7.4.2011 diem]
	//trayIcon = new QSystemTrayIcon(this->windowIcon(), this);

	// TODO:
	// one could blur the canvas if a transparent GUI is present
	// what we would need: QGraphicsBlurEffect...
	// render all widgets to the alpha channel (bw)
	// pre-render the viewport to that image... apply blur
	// and then render the blurred image after the widget is rendered...
	// performance?!

}

DkViewPort::~DkViewPort() {

	release();
}

void DkViewPort::release() {

	if (loader) delete loader;
	loader = 0;
}

#ifdef WITH_OPENCV
void DkViewPort::setImage(cv::Mat newImg) {

	//if (this->img.size() != newImg.size()) {
	//	// do we really need a deep copy here?

	// TODO: be careful with the format!
	imgQt = QImage(newImg.data, newImg.cols, newImg.rows, newImg.step, QImage::Format_RGB888);
	//}

	this->imgRect = QRect(0, 0, newImg.cols, newImg.rows);


	if (!DkSettings::Display::keepZoom)
		worldMatrix.reset();

	updateImageMatrix();

	if (DkSettings::Display::keepZoom)
		centerImage();

	controller->stopLabels();

	update();
}
#endif

void DkViewPort::updateImage() {

	// should not happen -> the loader should send this signal
	if (!loader)
		return;

	if (loader->hasImage())
		setImage(loader->getImage());
}

void DkViewPort::setImage(QImage newImg) {

	DkTimer dt;

	if (!thumbLoaded) { 
		qDebug() << "saving image matrix...";
		oldImgViewRect = imgViewRect;
		oldWorldMatrix = worldMatrix;
		oldImgMatrix = imgMatrix;
	}

	imgPyramid.clear();

	controller->getOverview()->setImage(QImage());	// clear overview

	imgQt = newImg;
	this->imgRect = QRectF(0, 0, newImg.width(), newImg.height());

	emit enableNoImageSignal(!imgQt.isNull());

	//qDebug() << "new image (viewport) loaded,  size: " << newImg.size() << "channel: " << imgQt.format();

	if (!DkSettings::Display::keepZoom || imgRect != oldImgRect)
		worldMatrix.reset();
	else {
		imgViewRect = oldImgViewRect;
		imgMatrix = oldImgMatrix;
		worldMatrix = oldWorldMatrix;
	}

	updateImageMatrix();

	controller->getPlayer()->startTimer();
	controller->getOverview()->setImage(imgQt);
	//// TODO: this is a fast fix
	//// if this thread uses the static metadata object 
	//// nomacs crashes when images are loaded fast (2 threads try to access DkMetaData simultaneously)
	//// currently we need to read the metadata twice (not nice either)
	DkImageLoader::imgMetaData.setFileName(loader->getFile());	controller->stopLabels();

	thumbLoaded = false;
	thumbFile = QFileInfo();
	oldImgRect = imgRect;

	update();

	// draw a histogram from the image -> does nothing if the histogram is invisible
	if (controller->getHistogram()) controller->getHistogram()->drawHistogram(newImg);
	qDebug() << "setting the image took me: " << QString::fromStdString(dt.getTotal());

}

void DkViewPort::setThumbImage(QImage newImg) {

	if (!thumbLoaded) { 
		qDebug() << "saving image matrix...";
		oldImgViewRect = imgViewRect;
		oldWorldMatrix = worldMatrix;
		oldImgMatrix = imgMatrix;
	}

	DkTimer dt;
	imgPyramid.clear();

	imgQt = newImg;
	QRectF oldImgRect = imgRect;
	this->imgRect = QRectF(0, 0, newImg.width(), newImg.height());

	emit enableNoImageSignal(true);

	if (!DkSettings::Display::keepZoom || imgRect != oldImgRect)
		worldMatrix.reset();							

	updateImageMatrix();
	
	controller->getOverview()->setImage(imgQt);
	controller->stopLabels();

	//// TODO: this is a fast fix
	//// if this thread uses the static metadata object 
	//// nomacs crashes when images are loaded fast (2 threads try to access DkMetaData simultaneously)
	//// currently we need to read the metadata twice (not nice either)
	//DkImageLoader::imgMetaData.setFileName(loader->getFile());

	//controller->updateRating(DkImageLoader::imgMetaData.getRating());

	//if (controller->getFileInfoLabel()->isVisible()) {
	//	QString dateString = QString::fromStdString(DkImageLoader::imgMetaData.getExifValue("DateTimeOriginal"));
	//	controller->getFileInfoLabel()->updateInfo(loader->getFile(), dateString, DkImageLoader::imgMetaData.getRating());
	//}

	thumbLoaded = true;

	update();
	qDebug() << "setting the image took me: " << QString::fromStdString(dt.getTotal());
}

void DkViewPort::tcpSendImage() {

	controller->setInfo("sending image...", 3000, DkControlWidget::center_label);

	if (loader)
		sendImageSignal(imgQt, loader->fileName());
	else
		sendImageSignal(imgQt, "nomacs - Image Lounge");
}

void DkViewPort::fileNotLoaded(QFileInfo file) {

	qDebug() << "starting timer over again...";

	// things todo if a file was not loaded...
	controller->getPlayer()->startTimer();
}

QPoint DkViewPort::newCenter(QSize s) {

	QSize dxy = (s - parent->size())/2;
	QPoint newPos = parent->pos() - QPoint(dxy.width(), dxy.height());

	return newPos;
}

void DkViewPort::zoom(float factor, QPointF center) {

	if (imgQt.isNull() || blockZooming)
		return;

	//factor/=5;//-0.1 <-> 0.1
	//factor+=1;//0.9 <-> 1.1

	//limit zoom out ---
	if (worldMatrix.m11()*factor < 0.1f && factor < 1)
		return;

	// reset view & block if we pass the 'image fit to screen' on zoom out
	if (worldMatrix.m11() > 1 && worldMatrix.m11()*factor < 1) {

		blockZooming = true;
		zoomTimer->start(500);
		resetView();
		return;
	}

	// reset view if we pass the 'image fit to screen' on zoom in
	if (worldMatrix.m11() < 1 && worldMatrix.m11()*factor > 1) {
		
		resetView();
		return;
	}

	//limit zoom in ---
	if (worldMatrix.m11()*imgMatrix.m11() > 50 && factor > 1)
		return;

	bool blackBorder = false;

	// if no center assigned: zoom in at the image center
	if (center.x() == -1 || center.y() == -1)
		center = imgViewRect.center();
	else {

		// if black border - do not zoom to the mouse coordinate
		if ((float)imgViewRect.width()*(worldMatrix.m11()*factor) < (float)width()) {
			center.setX(imgViewRect.center().x());
			blackBorder = true;
		}
		if (((float)imgViewRect.height()*worldMatrix.m11()*factor) < (float)height()) {
			center.setY(imgViewRect.center().y());
			blackBorder = true;
		}
	}

	//inverse the transform
	int a, b;
	worldMatrix.inverted().map(center.x(), center.y(), &a, &b);

	worldMatrix.translate(a-factor*a, b-factor*b);
	worldMatrix.scale(factor, factor);
	
	controlImagePosition();
	if (blackBorder && factor < 1) centerImage();	// TODO: geht auch schöner
	showZoom();
	changeCursor();

	controller->update();	// why do we need to update the controller manually?
	update();

	if (qApp->keyboardModifiers() == altMod && (hasFocus() || controller->hasFocus()))
		tcpSynchronize();
	
}

void DkViewPort::resetView() {

	worldMatrix.reset();
	showZoom();
	changeCursor();

	update();

	if (qApp->keyboardModifiers() == altMod && (hasFocus() || controller->hasFocus()))
		tcpSynchronize();
}

void DkViewPort::fullView() {

	worldMatrix.reset();
	zoom(1.0f/imgMatrix.m11());
	showZoom();
	changeCursor();
	update();
}

void DkViewPort::showZoom() {

	QString zoomStr;
	zoomStr.sprintf("%.1f%%", imgMatrix.m11()*worldMatrix.m11()*100);
	controller->setInfo(zoomStr, 3000, DkControlWidget::bottom_left_label);
}

void DkViewPort::toggleResetMatrix() {

	DkSettings::Display::keepZoom = !DkSettings::Display::keepZoom;
}

void DkViewPort::controlImagePosition(float lb, float ub) {

	QRectF imgRectWorld = worldMatrix.mapRect(imgViewRect);

	if (lb == -1)	lb = viewportRect.width()/2;
	if (ub == -1)	ub = viewportRect.height()/2;

	if (imgRectWorld.left() > lb && imgRectWorld.width() > width())
		worldMatrix.translate((lb-imgRectWorld.left())/worldMatrix.m11(), 0);

	if (imgRectWorld.top() > ub && imgRectWorld.height() > height())
		worldMatrix.translate(0, (ub-imgRectWorld.top())/worldMatrix.m11());

	if (imgRectWorld.right() < lb && imgRectWorld.width() > width())
		worldMatrix.translate((lb-imgRectWorld.right())/worldMatrix.m11(), 0);

	if (imgRectWorld.bottom() < ub && imgRectWorld.height() > height())
		worldMatrix.translate(0, (ub-imgRectWorld.bottom())/worldMatrix.m11());

}

void DkViewPort::updateImageMatrix() {

	if (imgQt.isNull())
		return;

	QRectF oldImgRect = imgViewRect;
	QTransform oldImgMatrix = imgMatrix;

	imgMatrix.reset();

	// if the image is smaller or zoom is active: paint the image as is
	if (!viewportRect.contains(imgRect.toRect()))
		imgMatrix = getScaledImageMatrix();
	else {
		imgMatrix.translate((float)(getMainGeometry().width()-imgQt.width())*0.5f, (float)(getMainGeometry().height()-imgQt.height())*0.5f);
		imgMatrix.scale(1.0f, 1.0f);
	}

	imgViewRect = imgMatrix.mapRect(imgRect);

	// update world matrix
	if (worldMatrix.m11() != 1) {

		float scaleFactor = oldImgMatrix.m11()/imgMatrix.m11();
		double dx = oldImgRect.x()/scaleFactor-imgViewRect.x();
		double dy = oldImgRect.y()/scaleFactor-imgViewRect.y();

		worldMatrix.scale(scaleFactor, scaleFactor);
		worldMatrix.translate(dx, dy);
	}
}

void DkViewPort::tcpSetTransforms(QTransform newWorldMatrix, QTransform newImgMatrix, QPointF canvasSize) {

	// ok relative transform
	if (canvasSize.isNull()) {
		moveView(QPointF(newWorldMatrix.dx(), newWorldMatrix.dy())/worldMatrix.m11());
	}
	else {
		worldMatrix = newWorldMatrix;
		imgMatrix = newImgMatrix;
		updateImageMatrix();

		QPointF imgPos = QPointF(canvasSize.x()*imgQt.width(), canvasSize.y()*imgQt.height());

		// go to screen coordinates
		imgPos = imgMatrix.map(imgPos);

		// go to world coordinates
		imgPos = worldMatrix.map(imgPos);

		// compute difference to current viewport center - in world coordinates
		imgPos = QPointF(width()*0.5f, height()*0.5f) - imgPos;

		// back to screen coordinates
		float s = worldMatrix.m11();
		worldMatrix.translate(imgPos.x()/s, imgPos.y()/s);
	}

	update();
}

void DkViewPort::tcpSetWindowRect(QRect rect) {
	this->setGeometry(rect);
}

void DkViewPort::tcpSynchronize(QTransform relativeMatrix) {
	
	if (relativeMatrix.isIdentity()) {
		QPointF size = QPointF(geometry().width()/2.0f, geometry().height()/2.0f);
		size = worldMatrix.inverted().map(size);
		size = imgMatrix.inverted().map(size);
		size = QPointF(size.x()/(float)imgQt.width(), size.y()/(float)imgQt.height());

		emit sendTransformSignal(worldMatrix, imgMatrix, size);
	}
	else {
		emit sendTransformSignal(relativeMatrix, QTransform(), QPointF());
	}
}

void DkViewPort::tcpShowConnections(QList<DkPeer> peers) {

	QString newPeers;

	for (int idx = 0; idx < peers.size(); idx++) {
		
		DkPeer cp = peers.at(idx);

		if (cp.getSynchronized() && newPeers.isEmpty()) {
			newPeers = tr("connected with: ");
			emit newClientConnectedSignal();
		}
		else if (newPeers.isEmpty())
			newPeers = tr("disconnected with: ");
		
		newPeers.append("\n\t");

		if (!cp.clientName.isEmpty())
			newPeers.append(cp.clientName);
		if (!cp.clientName.isEmpty() && !cp.title.isEmpty())
			newPeers.append(": ");
		if (!cp.title.isEmpty())
			newPeers.append(cp.title);
	}

	controller->setInfo(newPeers);
	update();
}

void DkViewPort::paintEvent(QPaintEvent* event) {

	QPainter painter(viewport());

	if (!imgQt.isNull()) {
		painter.setWorldTransform(worldMatrix);

		if (imgMatrix.m11()*worldMatrix.m11() <= (float)DkSettings::Display::interpolateZoomLevel/100.0f)
			painter.setRenderHint(QPainter::SmoothPixmapTransform);
		
		draw(&painter);
		//Now disable matrixWorld for overlay display
		painter.setWorldMatrixEnabled(false);
	}
	else
		drawBackground(&painter);

	//in mode zoom/panning
	if (worldMatrix.m11() > 1 && !imageInside() && 
		DkSettings::App::showOverview.testBit(DkSettings::App::currentAppMode)) {

		if (!controller->getOverview()->isVisible())
			controller->getOverview()->show();
	}
	else if (controller->getOverview()->isVisible())
		controller->getOverview()->hide();

	painter.end();

	// propagate
	QGraphicsView::paintEvent(event);

}

// drawing functions --------------------------------------------------------------------
void DkViewPort::drawBackground(QPainter *painter) {
	
	painter->setRenderHint(QPainter::SmoothPixmapTransform);

	// fit to viewport
	QSize s = imgBg.size();
	if (s.width() > (float)(size().width()*0.5))
		s = s*((size().width()*0.5)/s.width());

	if (s.height() > size().height()*0.6)
		s = s*((size().height()*0.6)/s.height());

	QRect bgRect(QPoint(size().width()-s.width()-size().width()*0.05, size().height()-s.height()-size().height()*0.05), s);

	painter->drawImage(bgRect, imgBg, QRect(QPoint(), imgBg.size()));
}

void DkViewPort::drawPolygon(QPainter *painter, QPolygon *polygon) {

	QPoint lastPoint;

	for (int idx = 0; idx < polygon->size(); idx++) {

		QPoint p = polygon->at(idx);

		if (!lastPoint.isNull())
			painter->drawLine(p, lastPoint);

		lastPoint = p;
	}

}

// event listeners --------------------------------------------------------------------
void DkViewPort::resizeEvent(QResizeEvent *event) {

	viewportRect = QRect(0, 0, width(), height());

	// >DIR: diem - bug if zoom factor is large and window becomes small
	updateImageMatrix();
	centerImage();
	changeCursor();

	controller->getOverview()->setViewPortRect(geometry());
	
	controller->resize(width(), height());
	qDebug() << "controller geometry: " << controller->geometry();

	return QGraphicsView::resizeEvent(event);
}

// mouse events --------------------------------------------------------------------
bool DkViewPort::event(QEvent *event) {

	// ok obviously QGraphicsView eats all mouse events -> so we simply redirect these to QWidget in order to get them delivered here
	if (event->type() == QEvent::MouseButtonPress || 
		event->type() == QEvent::MouseButtonDblClick || 
		event->type() == QEvent::MouseButtonRelease || 
		event->type() == QEvent::MouseMove || 
		event->type() == QEvent::Wheel || 
		event->type() == QEvent::KeyPress || 
		event->type() == QEvent::KeyRelease) {

		//qDebug() << "redirecting event...";
		// mouse eventss that double are now fixed, since the viewport is now overlayed by the controller
		return QWidget::event(event);
	}
	else
		return DkBaseViewPort::event(event);
	
}

void DkViewPort::mousePressEvent(QMouseEvent *event) {

	// ok, start panning
	if (worldMatrix.m11() > 1 && !imageInside() && event->buttons() == Qt::LeftButton) {
		setCursor(Qt::ClosedHandCursor);
		posGrab = event->pos();
	}
	
	// should be sent to QWidget?!
	DkBaseViewPort::mousePressEvent(event);
}

void DkViewPort::mouseReleaseEvent(QMouseEvent *event) {
	
	DkBaseViewPort::mouseReleaseEvent(event);
}

void DkViewPort::mouseMoveEvent(QMouseEvent *event) {

	//qDebug() << "mouse move (DkViewPort)";
	//changeCursor();

	if (visibleStatusbar)
		getPixelInfo(event->pos());

	if (worldMatrix.m11() > 1 && event->buttons() == Qt::LeftButton) {

		QPointF cPos = event->pos();
		QPointF dxy = (cPos - posGrab);
		posGrab = cPos;
		moveView(dxy/worldMatrix.m11());

		// with shift also a hotkey for fast switching...
		if ((DkSettings::Sync::syncAbsoluteTransform &&
			event->modifiers() == (altMod | Qt::ShiftModifier)) || 
			(!DkSettings::Sync::syncAbsoluteTransform &&
			event->modifiers() == (altMod))) {
			
			if (dxy.x() != 0 || dxy.y() != 0) {
				QTransform relTransform;
				relTransform.translate(dxy.x(), dxy.y());
				tcpSynchronize(relTransform);
			}
		}
		else if (event->modifiers() == altMod)
			tcpSynchronize();
	}

	// send to parent
	DkBaseViewPort::mouseMoveEvent(event);
}

void DkViewPort::wheelEvent(QWheelEvent *event) {

	if (event->modifiers() == ctrlMod) {

		// TODO: think how we can make this fast too
		if (event->delta() > 0)
			loadNextFile();
		else
			loadPrevFile();
	}
	else 
		DkBaseViewPort::wheelEvent(event);

	if (event->modifiers() == altMod)
		tcpSynchronize();

}

void DkViewPort::setFullScreen(bool fullScreen) {

	controller->setFullScreen(fullScreen);
	toggleLena();
}

void DkViewPort::getPixelInfo(const QPoint& pos) {

	if (imgQt.isNull())
		return;

	QPointF imgPos = worldMatrix.inverted().map(QPointF(pos));
	imgPos = imgMatrix.inverted().map(imgPos);

	QPoint xy = imgPos.toPoint();

	if (xy.x() < 0 || xy.y() < 0 || xy.x() >= imgQt.width() || xy.y() >= imgQt.height())
		return;

	QColor col = imgQt.pixel(xy);
	
	QString msg = "<font color='grey'>x: " % QString::number(xy.x()) % " y: " % QString::number(xy.y()) % "</font>"
		" | r: " % QString::number(col.red()) % " g: " % QString::number(col.green()) % " b: " % QString::number(col.blue());

	if (imgQt.hasAlphaChannel())
		msg = msg % " a: " % QString::number(col.alpha());

	emit statusInfoSignal(msg);

}

// edit image --------------------------------------------------------------------
void DkViewPort::rotateCW() {

	if (loader != 0)
		loader->rotateImage(90);

}

void DkViewPort::rotateCCW() {

	if (loader != 0)
		loader->rotateImage(-90);

}

void DkViewPort::rotate180() {

	if (loader != 0)
		loader->rotateImage(180);

}

// file handling --------------------------------------------------------------------
void DkViewPort::loadLena() {

	bool ok;
	QString text = QInputDialog::getText(this, tr("Lena"), tr("A remarkable woman"), QLineEdit::Normal, 0, &ok);

	// pass phrase
	if (ok && !text.isEmpty() && text == "lena") {
		testLoaded = true;
		toggleLena();
	}
	else if (!ok) {
		QMessageBox warningDialog(parent);
		warningDialog.setIcon(QMessageBox::Warning);
		warningDialog.setText(tr("you cannot cancel this"));
		warningDialog.exec();
		loadLena();
	}
	else {
		QApplication::beep();
		
		if (text.isEmpty())
			controller->setInfo(tr("did you understand the brainteaser?"));
		else
			controller->setInfo(tr("%1 is wrong...").arg(text));
	}
}

void DkViewPort::toggleLena() {

	if (!testLoaded)
		return;

	if (loader) {
		if (parent->isFullScreen())
			loader->load(QFileInfo(":/nomacs/img/lena-full.jpg"));
		else
			loader->load(QFileInfo(":/nomacs/img/lena.jpg"));
	}
}

void DkViewPort::settingsChanged() {

	reloadFile();

	altMod = DkSettings::Global::altMod;
	ctrlMod = DkSettings::Global::ctrlMod;

	controller->settingsChanged();
}

void DkViewPort::setEditedImage(QImage newImg) {

	QFileInfo file = loader->getFile();
	unloadImage();
	setImage(newImg);
	loader->setImage(newImg, file);

	// TODO: contrast viewport does not add * 

	// TODO: add functions such as save file on unload
}

void DkViewPort::unloadImage() {

	int rating = controller->getRating();

	// TODO: if image is not saved... ask user?! -> resize & crop
	if (!imgQt.isNull() && loader && rating != -1 && rating != loader->getMetaData().getRating()) {
		qDebug() << "old rating: " << loader->getMetaData().getRating();
		loader->saveRating(rating);
	}
	else
		qDebug() << "there is no need to save the rating (metadata rating: " << loader->getMetaData().getRating() << "my rating: " << rating << ")";
	
	if (loader) loader->clearPath();	// tell loader that the image is not the display image anymore
}

void DkViewPort::loadFile(QFileInfo file, bool silent) {

	testLoaded = false;
	unloadImage();

	if (loader && file.isDir()) {
		QDir dir = QDir(file.absoluteFilePath());
		loader->setDir(dir);
	} else if (loader)
		loader->load(file, silent);
}

void DkViewPort::reloadFile() {

	unloadImage();

	if (loader) {
		loader->changeFile(0, false, true);	// silent loading, but force loading

		if (controller->getFilePreview())
			controller->getFilePreview()->updateDir(loader->getFile(), true);
	}
}

void DkViewPort::loadNextFile(bool silent) {

	// this function is (more or less) deprecated -> just needed since we cannot distinguish between action triggered & action repeated
	unloadImage();

	if (loader && !testLoaded)
		loader->changeFile(1, silent || (parent->isFullScreen() && DkSettings::SlideShow::silentFullscreen));

	// alt mod
	if (qApp->keyboardModifiers() == altMod && (hasFocus() || controller->hasFocus())) {
		emit sendNewFileSignal(1);
		qDebug() << "emitting load next";
	}
}

void DkViewPort::loadPrevFile(bool silent) {

	unloadImage();

	if (loader && !testLoaded)
		loader->changeFile(-1, silent || (parent->isFullScreen() && DkSettings::SlideShow::silentFullscreen));

	if (qApp->keyboardModifiers() == altMod && (hasFocus() || controller->hasFocus()))
		emit sendNewFileSignal(-1);
}

void DkViewPort::loadPrevFileFast(bool silent) {

	loadFileFast(-1, silent);
}

void DkViewPort::loadNextFileFast(bool silent) {

	loadFileFast(1, silent);
}


void DkViewPort::loadFileFast(int skipIdx, bool silent) {

	silent |= (parent->isFullScreen() && DkSettings::SlideShow::silentFullscreen);

	if (DkSettings::Resources::fastThumbnailPreview) {

		QImage thumb;
		QFileInfo thumbFile;

		if (loader && !testLoaded) {

			thumbFile = loader->getChangedFileInfo(skipIdx, silent);
			qDebug() << thumbFile.fileName();

			QFile f((thumbFile.isSymLink()) ? thumbFile.symLinkTarget() : thumbFile.absoluteFilePath());

			// directly load images < 150 KB
			if (f.exists() && f.size() > 0 && f.size() < 150*1024) {
				unloadImage();
				loader->loadFile(thumbFile, silent, DkImageLoader::cache_disable_update);
			}
			// load full file if cached
			else if (loader->isCached(thumbFile)) {
				unloadImage();
				loader->load(thumbFile, silent, DkImageLoader::cache_disable_update);	// disable cacher on fast load
			}
			else {
				thumb = loader->loadThumb(thumbFile, silent);

				if (thumbFile.exists())
					this->thumbFile = thumbFile;
			}
		}

		if (!thumb.isNull()) {
			unloadImage();
			setThumbImage(thumb);
		}

		QCoreApplication::sendPostedEvents();
	}
	else if (loader && !testLoaded) {
		unloadImage();
		loader->changeFile(skipIdx, silent);
	}

	if (qApp->keyboardModifiers() == altMod && (hasFocus() || controller->hasFocus()))
		emit sendNewFileSignal(skipIdx);
}

void DkViewPort::loadFullFile(bool silent) {

	if (thumbFile.exists()) {
		unloadImage();	// TODO: unload image clears the image -> makes an empty file
		loader->load(thumbFile, silent || (parent->isFullScreen() && DkSettings::SlideShow::silentFullscreen));
	}
	
	if (loader)
		loader->updateCacheIndex();
}

void DkViewPort::loadFirst() {

	unloadImage();

	if (loader && !testLoaded)
		loader->firstFile();

	if (qApp->keyboardModifiers() == altMod && (hasFocus() || controller->hasFocus()))
		emit sendNewFileSignal(SHRT_MIN);
}

void DkViewPort::loadLast() {

	unloadImage();

	if (loader && !testLoaded)
		loader->lastFile();

	if (qApp->keyboardModifiers() == altMod && (hasFocus() || controller->hasFocus()))
		emit sendNewFileSignal(SHRT_MAX);

}

void DkViewPort::loadSkipPrev10() {

	unloadImage();

	if (loader && !testLoaded)
		loader->changeFile(-DkSettings::Global::skipImgs, (parent->isFullScreen() && DkSettings::SlideShow::silentFullscreen));

	if (qApp->keyboardModifiers() == altMod && (hasFocus() || controller->hasFocus()))
		emit sendNewFileSignal(-DkSettings::Global::skipImgs);
}

void DkViewPort::loadSkipNext10() {

	unloadImage();

	if (loader && !testLoaded)
		loader->changeFile(DkSettings::Global::skipImgs, (parent->isFullScreen() && DkSettings::SlideShow::silentFullscreen));

	if (qApp->keyboardModifiers() == altMod && (hasFocus() || controller->hasFocus()))
		emit sendNewFileSignal(DkSettings::Global::skipImgs);
}

void DkViewPort::tcpLoadFile(qint16 idx, QString filename) {

	qDebug() << "I got a file request??";

	if (filename.isEmpty()) {

		// change the file idx according to my brother
		switch (idx) {
			case SHRT_MIN:
				loadFirst();
				break;
			case SHRT_MAX:
				loadLast();
				break;
			case 1:
				loadNextFile();
				break;
			case -1:
				loadPrevFile();
				break;
			default:
				if (loader) loader->loadFileAt(idx);
		}
	}
	else 
		loadFile(QFileInfo(filename));

}

DkImageLoader* DkViewPort::getImageLoader() {

	return loader;
}

DkControlWidget* DkViewPort::getController() {
	
	return controller;
}

void DkViewPort::cropImage(DkRotatingRect rect) {

	QTransform tForm; 
	QPointF cImgSize;

	rect.getTransform(tForm, cImgSize);

	if (cImgSize.x() < 0.5f || cImgSize.y() < 0.5f) {
		controller->setInfo(tr("I cannot crop an image that has 0 px, sorry."));
		return;
	}

	qDebug() << cImgSize;

	QImage img = QImage(cImgSize.x(), cImgSize.y(), QImage::Format_ARGB32);
	img.fill(QColor(0,0,0,0).rgba());

	// render the image into the new coordinate system
	QPainter painter(&img);
	painter.setWorldTransform(tForm);
	painter.drawImage(QRect(QPoint(), imgQt.size()), imgQt, QRect(QPoint(), imgQt.size()));
	painter.end();

	setEditedImage(img);
	
	//imgQt = img;
	update();

	qDebug() << "cropping...";
}

void DkViewPort::printImage() {

	QPrinter printer;

	QPrintDialog *dialog = new QPrintDialog(&printer, this);
	dialog->setWindowTitle(tr("Print Document"));
	if (!imgQt.isNull())
		dialog->addEnabledOption(QAbstractPrintDialog::PrintSelection);
	if (dialog->exec() != QDialog::Accepted)
		return;

	qDebug() << printer.pageRect(QPrinter::Inch).height() << " x " << printer.pageRect(QPrinter::Inch).width();

	// TODO: not that stupid...
	QPainter painter(&printer);
	QRect rect = painter.viewport();
	QSize size = imgQt.size();
	size.scale(rect.size(), Qt::KeepAspectRatio);
	painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
	painter.setWindow(imgQt.rect());
	painter.drawImage(0, 0, imgQt);

	painter.end();
}

// DkViewPortFrameless --------------------------------------------------------------------
DkViewPortFrameless::DkViewPortFrameless(QWidget *parent, Qt::WFlags flags) : DkViewPort(parent) {
	
	setAttribute(Qt::WA_TranslucentBackground, true);
	setCursor(Qt::OpenHandCursor);

	imgBg.load(":/nomacs/img/splash-screen.png");

	mainScreen = geometry();

	//show();

	// TODO: just set the left - upper - lower offset for all labels (according to viewRect)
	// always set the size to be full screen -> bad for OS that are not able to show transparent frames!!
}

DkViewPortFrameless::~DkViewPortFrameless() {

	release();
}

void DkViewPortFrameless::release() {

	DkViewPort::release();
}

void DkViewPortFrameless::addStartActions(QAction* startAction, QIcon* startIcon) {

	startActions.append(startAction);
	startIcons.append(startIcon);
}

void DkViewPortFrameless::setImage(QImage newImg) {

	DkViewPort::setImage(newImg);
}

void DkViewPortFrameless::zoom(float factor, QPointF center) {

	if (imgQt.isNull() || blockZooming)
		return;

	//limit zoom out ---
	if (worldMatrix.m11()*factor <= 0.1 && factor < 1)
		return;

	//if (worldMatrix.m11()*factor < 1) {
	//	resetView();
	//	return;
	//}

	// reset view & block if we pass the 'image fit to screen' on zoom out
	if (worldMatrix.m11() > 1 && worldMatrix.m11()*factor < 1) {

		blockZooming = true;
		zoomTimer->start(500);
		//resetView();
		//return;
	}

	//limit zoom in ---
	if (worldMatrix.m11()*imgMatrix.m11() > 50 && factor > 1)
		return;

	QRectF viewRect = worldMatrix.mapRect(imgViewRect);

	// if no center assigned: zoom in at the image center
	if (center.x() == -1 || center.y() == -1)
		center = viewRect.center();

	
	//if (factor < 1) {
	//	
	//	QRectF imgWorldRect = worldMatrix.mapRect(imgViewRect);
	//	float ipl = imgViewRect.width()/imgWorldRect.width();	// size ratio
	//	center = (imgViewRect.center() - imgWorldRect.center()) + imgViewRect.center();
	//}

	if (center.x() < viewRect.left())			center.setX(viewRect.left());
	else if (center.x() > viewRect.right())		center.setX(viewRect.right());
	if (center.y() < viewRect.top())			center.setY(viewRect.top());
	else if (center.y() > viewRect.bottom())	center.setY(viewRect.bottom());

	//inverse the transform
	int a, b;
	worldMatrix.inverted().map(center.x(), center.y(), &a, &b);

	worldMatrix.translate(a-factor*a, b-factor*b);
	worldMatrix.scale(factor, factor);

	controlImagePosition();
	showZoom();
	changeCursor();

	update();

	if (qApp->keyboardModifiers() == altMod && (hasFocus() || controller->hasFocus()))
		tcpSynchronize();

}

void DkViewPortFrameless::resetView() {

	// maybe we can delete this function...
	DkViewPort::resetView();
}

void DkViewPortFrameless::paintEvent(QPaintEvent* event) {

	if (!parent->isFullScreen()) {

		QPainter painter(viewport());
		painter.setWorldTransform(worldMatrix);
		drawFrame(&painter);
		painter.end();
	}

	DkViewPort::paintEvent(event);
}

void DkViewPortFrameless::draw(QPainter *painter) {
	
	if (parent->isFullScreen()) {
		QColor col = QColor(0,0,0);
		col.setAlpha(150);
		painter->setWorldMatrixEnabled(false);
		painter->fillRect(QRect(QPoint(), size()), col);
		painter->setWorldMatrixEnabled(true);
	}

	painter->drawImage(imgViewRect, imgQt, imgRect);
	//DkViewPort::draw(painter);
}

void DkViewPortFrameless::drawBackground(QPainter *painter) {
	
	painter->setWorldTransform(imgMatrix);
	painter->setRenderHint(QPainter::SmoothPixmapTransform);
	painter->setBrush(QColor(127, 144, 144, 200));
	painter->setPen(QColor(100, 100, 100, 255));

	QRectF initialRect = mainScreen;
	QPointF oldCenter = initialRect.center();

	QTransform cT;
	cT.scale(400/initialRect.width(), 400/initialRect.width());
	initialRect = cT.mapRect(initialRect);
	initialRect.moveCenter(oldCenter);

	// fit to viewport
	QSize s = imgBg.size();

	QRectF bgRect(QPoint(), s);
	bgRect.moveCenter(initialRect.center());//moveTopLeft(QPointF(size().width(), size().height())*0.5 - initialRect.bottomRight()*0.5);

	//painter->drawRect(initialRect);
	painter->drawImage(bgRect, imgBg, QRect(QPoint(), imgBg.size()));

	if (startActions.isEmpty())
		return;

	// first time?
	if (startActionsRects.isEmpty()) {
		float margin = 40;
		float iconSizeMargin = (initialRect.width()-3*margin)/startActions.size();
		QSize iconSize = QSize(iconSizeMargin - margin, iconSizeMargin - margin);
		QPointF offset = QPointF(bgRect.left() + 50, initialRect.center().y()+iconSizeMargin*0.25f);

		for (int idx = 0; idx < startActions.size(); idx++) {

			QRectF iconRect = QRectF(offset, iconSize);
			QPixmap ci = startIcons[idx] ? startIcons[idx]->pixmap(iconSize) : startActions[idx]->icon().pixmap(iconSize);
			startActionsRects.push_back(iconRect);
			startActionsIcons.push_back(ci);

			offset.setX(offset.x() + margin + iconSize.width());
		}
	}

	// draw start actions
	for (int idx = 0; idx < startActions.size(); idx++) {
		
		if (startIcons[idx])
			painter->drawPixmap(startActionsRects[idx], startActionsIcons[idx], QRect(QPoint(), startActionsIcons[idx].size()));
		else
			painter->drawPixmap(startActionsRects[idx], startActionsIcons[idx], QRect(QPoint(), startActionsIcons[idx].size()));
		
		QRectF tmpRect = startActionsRects[idx];
		QString text = startActions[idx]->text().replace("&", "");
		tmpRect.moveTop(tmpRect.top()+tmpRect.height()+10);
		painter->drawText(tmpRect, text);
	}
}

void DkViewPortFrameless::drawFrame(QPainter* painter) {

	// TODO: replace hasAlphaChannel with has alphaBorder
	if (!imgQt.isNull() && imgQt.hasAlphaChannel())
		return;

	painter->setBrush(QColor(255, 255, 255, 200));
	painter->setPen(QColor(100, 100, 100, 255));

	QRectF frameRect;

	float fs = qMin(imgViewRect.width(), imgViewRect.height())*0.1f;

	// looks pretty bad if the frame is too small
	if (fs < 4)
		return;

	frameRect = imgViewRect;
	frameRect.setSize(frameRect.size() + QSize(fs, fs));
	frameRect.moveCenter(imgViewRect.center());

	painter->drawRect(frameRect);
}

void DkViewPortFrameless::mousePressEvent(QMouseEvent *event) {
	
	// move the window - todo: NOT full screen, window inside...
	setCursor(Qt::ClosedHandCursor);
	posGrab = event->pos();

	DkViewPort::mousePressEvent(event);
}

void DkViewPortFrameless::mouseReleaseEvent(QMouseEvent *event) {
	
	if (imgQt.isNull()) {

		qDebug() << "mouse released";
		QPointF pos = imgMatrix.inverted().map(event->pos());

		for (int idx = 0; idx < startActionsRects.size(); idx++) {

			if (startActionsRects[idx].contains(pos)) {
				qDebug() << "toggle..." << idx;
				
				startActions[idx]->trigger();
				break;
			}
		}
	}

	setCursor(Qt::OpenHandCursor);
	DkViewPort::mouseReleaseEvent(event);
}


void DkViewPortFrameless::mouseMoveEvent(QMouseEvent *event) {
	
	if (imgQt.isNull()) {

		QPointF pos = imgMatrix.inverted().map(event->pos());

		int idx;
		for (idx = 0; idx < startActionsRects.size(); idx++) {

			if (startActionsRects[idx].contains(pos)) {
				setCursor(Qt::PointingHandCursor);
				break;
			}
		}

		// TODO: change if closed hand cursor is present...
		if (idx == startActionsRects.size())
			setCursor(Qt::OpenHandCursor);
	}

	if (visibleStatusbar)
		getPixelInfo(event->pos());

	if (event->buttons() == Qt::LeftButton) {

		QPointF cPos = event->pos();
		QPointF dxy = (cPos - posGrab);
		posGrab = cPos;
		moveView(dxy/worldMatrix.m11());
	}

	//// scroll thumbs preview
	//if (filePreview && filePreview->isVisible() && event->buttons() == Qt::MiddleButton) {

	//	float dx = std::fabs(enterPos.x() - event->pos().x())*0.015;
	//	dx = std::exp(dx);
	//	if (enterPos.x() - event->pos().x() < 0)
	//		dx = -dx;

	//	filePreview->setCurrentDx(dx);	// update dx
	//}

	QGraphicsView::mouseMoveEvent(event);
}

void DkViewPortFrameless::resizeEvent(QResizeEvent *event) {

	DkViewPort::resizeEvent(event);

	// controller should only be on the main screen...
	QDesktopWidget* dw = QApplication::desktop();
	controller->setGeometry(dw->screenGeometry());
	qDebug() << "controller resized to: " << controller->geometry();
}

void DkViewPortFrameless::moveView(QPointF delta) {

	// if no zoom is present -> the translation is like a move window
	if (worldMatrix.m11() == 1.0f) {
		float s = imgMatrix.m11();
		imgMatrix.translate(delta.x()/s, delta.y()/s);
		imgViewRect = imgMatrix.mapRect(imgRect);
	}
	else
		worldMatrix.translate(delta.x(), delta.y());

	controlImagePosition();
	update();
}


void DkViewPortFrameless::controlImagePosition(float lb, float ub) {
	// dummy method
}

void DkViewPortFrameless::centerImage() {

}

void DkViewPortFrameless::updateImageMatrix() {

	if (imgQt.isNull())
		return;

	QRectF oldImgRect = imgViewRect;
	QTransform oldImgMatrix = imgMatrix;

	imgMatrix.reset();

	// if the image is smaller or zoom is active: paint the image as is
	if (!getMainGeometry().contains(imgRect.toRect()))
		imgMatrix = getScaledImageMatrix();
	else {

		QPointF p = (imgViewRect.isEmpty()) ? getMainGeometry().center() : imgViewRect.center();
		p -= imgQt.rect().center();
		imgMatrix.translate(p.x()-1, p.y()-1);	// -1 is needed due to float -> int
		imgMatrix.scale(1.0f, 1.0f);
	}

	imgViewRect = imgMatrix.mapRect(imgRect);

	// update world matrix
	if (worldMatrix.m11() != 1) {

		float scaleFactor = oldImgMatrix.m11()/imgMatrix.m11();
		double dx = oldImgRect.x()/scaleFactor-imgViewRect.x();
		double dy = oldImgRect.y()/scaleFactor-imgViewRect.y();

		worldMatrix.scale(scaleFactor, scaleFactor);
		worldMatrix.translate(dx, dy);
	}
}

QTransform DkViewPortFrameless::getScaledImageMatrix() {

	QRectF initialRect = mainScreen;
	QPointF oldCenter = imgViewRect.isEmpty() ? initialRect.center() : imgViewRect.center();
	qDebug() << "initial rect: " << initialRect;

	QTransform cT;
	cT.scale(800/initialRect.width(), 800/initialRect.width());
	cT.translate(initialRect.center().x(), initialRect.center().y());
	initialRect = cT.mapRect(initialRect);
	initialRect.moveCenter(oldCenter);

	// the image resizes as we zoom
	float ratioImg = imgRect.width()/imgRect.height();
	float ratioWin = initialRect.width()/initialRect.height();

	QTransform imgMatrix;
	float s;
	if (imgRect.width() == 0 || imgRect.height() == 0)
		s = 1.0f;
	else
		s = (ratioImg > ratioWin) ? initialRect.width()/imgRect.width() : initialRect.height()/imgRect.height();

	imgMatrix.scale(s, s);

	QRectF imgViewRect = imgMatrix.mapRect(imgRect);
	QSizeF sDiff = (initialRect.size() - imgViewRect.size())*0.5f/s;
	imgMatrix.translate(initialRect.left()/s+sDiff.width(), initialRect.top()/s+sDiff.height());

	return imgMatrix;
}

DkViewPortContrast::DkViewPortContrast(QWidget *parent, Qt::WFlags flags) : DkViewPort(parent) {

	isColorPickerActive = false;
	activeChannel = 0;
	
	colorTable = QVector<QRgb>(256);
	for (int i = 0; i < colorTable.size(); i++) 
		colorTable[i] = qRgb(i, i, i);
	
	drawFalseColorImg = false;

}

DkViewPortContrast::~DkViewPortContrast() {

	release();
}

void DkViewPortContrast::release() {

	DkViewPort::release();
}

void DkViewPortContrast::changeChannel(int channel) {

	if (channel < 0 || channel >= imgs.size())
		return;

	if (!imgQt.isNull()) {

		falseColorImg = imgs[channel];
		falseColorImg.setColorTable(colorTable);
		drawFalseColorImg = true;

		update();

		drawImageHistogram();
	}

}


void DkViewPortContrast::changeColorTable(QGradientStops stops) {

	
	qreal pos = stops.at(0).first;
	qreal fac;

	qreal actPos, leftStop, rightStop;
	QColor tmp;

	int rLeft, gLeft, bLeft, rRight, gRight, bRight;
	int rAct, gAct, bAct;

	// At least one stop has to be set:
	tmp = stops.at(0).second;
	tmp.getRgb(&rLeft, &gLeft, &bLeft);
	leftStop = stops.at(0).first;

	// If just one stop is set, we can speed things up:
	if (stops.size() == 1) {
		for (int i = 0; i < colorTable.size(); i++)
			colorTable[i] = qRgb(rLeft, gLeft, bLeft);
	}
	// Otherwise interpolate:
	else {

		int rightStopIdx = 1;
		tmp = stops.at(rightStopIdx).second;
		tmp.getRgb(&rRight, &gRight, &bRight);
		rightStop = stops.at(rightStopIdx).first;
	
		for (int i = 0; i < colorTable.size(); i++) {
			actPos = (qreal) i / colorTable.size();

			if (actPos > rightStop) {
				leftStop = rightStop;	

				rLeft = rRight;
				gLeft = gRight;
				bLeft = bRight;

				if (stops.size() > rightStopIdx + 1) {
					rightStopIdx++;
					rightStop = stops.at(rightStopIdx).first;
					tmp = stops.at(rightStopIdx).second;
					tmp.getRgb(&rRight, &gRight, &bRight);
				}

			}
		
			if (actPos <= leftStop)
				colorTable[i] = qRgb(rLeft, gLeft, bLeft);
			else if (actPos >= rightStop)
				colorTable[i] = qRgb(rRight, gRight, bRight);
			else {
				fac = (actPos - leftStop) / (rightStop - leftStop);
				rAct = qRound(rLeft + (rRight - rLeft) * fac);
				gAct = qRound(gLeft + (gRight - gLeft) * fac);
				bAct = qRound(bLeft + (bRight - bLeft) * fac);
				colorTable[i] = qRgb(rAct, gAct, bAct);
			}	
		}
	}


	falseColorImg.setColorTable(colorTable);
	
	update();
	
}

void DkViewPortContrast::draw(QPainter *painter) {

	if (parent->isFullScreen()) {
		painter->setWorldMatrixEnabled(false);
		painter->fillRect(QRect(QPoint(), size()), DkSettings::SlideShow::backgroundColor);
		painter->setWorldMatrixEnabled(true);
	}

	if (drawFalseColorImg)
		painter->drawImage(imgViewRect, falseColorImg, imgRect);
	else
		painter->drawImage(imgViewRect, imgQt, imgRect);

}

void DkViewPortContrast::setImage(QImage newImg) {

	DkViewPort::setImage(newImg);

	if (imgQt.format() == QImage::Format_Indexed8) {
		int format = imgQt.format();
		imgs = QVector<QImage>(1);
		imgs[0] = imgQt;
		activeChannel = 0;
	}

#ifdef WITH_OPENCV

	else {	
					
			imgs = QVector<QImage>(4);
			vector<Mat> planes;
			
			Mat imgUC3 = DkImage::qImage2Mat(imgQt);
			//int format = imgQt.format();
			//if (format == QImage::Format_RGB888)
			//	imgUC3 = Mat(imgQt.height(), imgQt.width(), CV_8UC3, (uchar*)imgQt.bits(), imgQt.bytesPerLine());
			//else
			//	imgUC3 = Mat(imgQt.height(), imgQt.width(), CV_8UC4, (uchar*)imgQt.bits(), imgQt.bytesPerLine());
			split(imgUC3, planes);
			// Store the 3 channels in a QImage Vector.
			//Be aware that OpenCV 'swaps' the rgb triplet, hence process it in a descending way:
			int idx = 1;
			for (int i = 2; i >= 0; i--) {

				// dirty hack
				if (i >= (int)planes.size()) i = 0;
				imgs[idx] = QImage((const unsigned char*)planes[i].data, planes[i].cols, planes[i].rows, planes[i].step,  QImage::Format_Indexed8);
				imgs[idx] = imgs[idx].copy();
				idx++;

			}
			// The first element in the vector contains the gray scale 'average' of the 3 channels:
			Mat grayMat;
			cv::cvtColor(imgUC3, grayMat, CV_BGR2GRAY);
			imgs[0] = QImage((const unsigned char*)grayMat.data, grayMat.cols, grayMat.rows, grayMat.step,  QImage::Format_Indexed8);
			imgs[0] = imgs[0].copy();
			planes.clear();

	}
#else

	else {
		drawFalseColorImg = false;
		emit imageModeSet(mode_invalid_format);	
		return;
	}

#endif
	
	
	falseColorImg = imgs[activeChannel];
	falseColorImg.setColorTable(colorTable);
	
	// images with valid color table return img.isGrayScale() false...
	if (imgs.size() == 1) 
		emit imageModeSet(mode_gray);
	else
		emit imageModeSet(mode_rgb);

	update();

	
}

void DkViewPortContrast::pickColor() {

	isColorPickerActive = true;
	this->setCursor(Qt::CrossCursor);

}

void DkViewPortContrast::enableTF(bool enable) {

	drawFalseColorImg = enable;
	update();

	drawImageHistogram();

}

void DkViewPortContrast::mousePressEvent(QMouseEvent *event) {


	if (isColorPickerActive) {

		QPointF imgPos = worldMatrix.inverted().map(event->pos());
		imgPos = imgMatrix.inverted().map(imgPos);

		QPoint xy = imgPos.toPoint();

		bool isPointValid = true;

		if (xy.x() < 0 || xy.y() < 0 || xy.x() >= imgQt.width() || xy.y() >= imgQt.height())
			isPointValid = false;

		if (isPointValid) {

			int colorIdx = imgs[activeChannel].pixelIndex(xy);
			qreal normedPos = (qreal) colorIdx / 255;
			emit tFSliderAdded(normedPos);

		}

		unsetCursor();
		isColorPickerActive = false;

	} 
	else
		DkViewPort::mousePressEvent(event);

}

void DkViewPortContrast::keyPressEvent(QKeyEvent* event) {

	if ((event->key() == Qt::Key_Escape) && isColorPickerActive) {
		unsetCursor();
		isColorPickerActive = false;
		update();
		return;
	}
	else
		DkViewPort::keyPressEvent(event);
}

QImage DkViewPortContrast::getImage() {

	if (drawFalseColorImg)
		return falseColorImg;
	else
		return imgQt;

}

// in contrast mode: if the histogram widget is visible redraw the histogram from the selected image channel data
void DkViewPortContrast::drawImageHistogram() {

	if (controller->getHistogram() && controller->getHistogram()->isVisible()) {
		if(drawFalseColorImg) controller->getHistogram()->drawHistogram(falseColorImg);
		else controller->getHistogram()->drawHistogram(imgQt);
	}

}

// custom events --------------------------------------------------------------------
//QEvent::Type DkInfoEvent::infoEventType = static_cast<QEvent::Type>(QEvent::registerEventType());
//QEvent::Type DkLoadImageEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());


}
