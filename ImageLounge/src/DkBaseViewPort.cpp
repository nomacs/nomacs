/*******************************************************************************************************
 DkBaseViewPort.cpp
 Created on:	03.07.2013
 
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

#include "DkBaseViewPort.h"

#include "DkSettings.h"
#include "DkUtils.h"

namespace nmc {
	
// DkBaseViewport --------------------------------------------------------------------
DkBaseViewPort::DkBaseViewPort(QWidget *parent, Qt::WindowFlags flags) : QGraphicsView(parent) {

	grabGesture(Qt::PanGesture);
	grabGesture(Qt::PinchGesture);
	grabGesture(Qt::SwipeGesture);
	setAttribute(Qt::WA_AcceptTouchEvents);

	forceFastRendering = false;
	this->parent = parent;
	viewportRect = QRect(0, 0, width(), height());
	worldMatrix.reset();
	imgMatrix.reset();
	movie = 0;

	panControl = QPointF(-1.0f, -1.0f);
	minZoom = 0.01f;
	maxZoom = 50;

	blockZooming = false;
	altMod = DkSettings::global.altMod;
	ctrlMod = DkSettings::global.ctrlMod;

	zoomTimer = new QTimer(this);
	zoomTimer->setSingleShot(true);
	connect(zoomTimer, SIGNAL(timeout()), this, SLOT(stopBlockZooming()));
	connect(&imgStorage, SIGNAL(imageUpdated()), this, SLOT(update()));

	pattern.setTexture(QPixmap(":/nomacs/img/tp-pattern.png"));

	setObjectName(QString::fromUtf8("DkBaseViewPort"));

	if (DkSettings::display.useDefaultColor) {

		if (DkSettings::display.toolbarGradient)
			setStyleSheet("QGraphicsView{border-style: none; background: QLinearGradient(x1: 0, y1: 0.7, x2: 0, y2: 1, stop: 0 #edeff9, stop: 1 #d9dbe4);}" );
		else
			setStyleSheet("QGraphicsView{border-style: none; background-color: " + DkUtils::colorToString(QPalette().color(QPalette::Window)) + ";}" );		
	}
	else
		setStyleSheet("QGraphicsView{border-style: none; background-color: " + DkUtils::colorToString(DkSettings::display.bgColor) + ";}" );

	setMouseTracking(true);

	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	setMinimumSize(10, 10);

	createShortcuts();
}

DkBaseViewPort::~DkBaseViewPort() {

	release();
}

void DkBaseViewPort::createShortcuts() {

	shortcuts.resize(sc_end);

	// panning
	shortcuts[sc_pan_left] = new QShortcut(shortcut_panning_left, this);
	connect(shortcuts[sc_pan_left], SIGNAL(activated()), this, SLOT(shiftLeft()));
	shortcuts[sc_pan_right] = new QShortcut(shortcut_panning_right, this);
	connect(shortcuts[sc_pan_right], SIGNAL(activated()), this, SLOT(shiftRight()));
	shortcuts[sc_pan_up] = new QShortcut(shortcut_panning_up, this);
	connect(shortcuts[sc_pan_up], SIGNAL(activated()), this, SLOT(shiftUp()));
	shortcuts[sc_pan_down] = new QShortcut(shortcut_panning_down, this);
	connect(shortcuts[sc_pan_down], SIGNAL(activated()), this, SLOT(shiftDown()));

	// zoom
	shortcuts[sc_zoom_in] = new QShortcut(shortcut_zoom_in, this);
	connect(shortcuts[sc_zoom_in], SIGNAL(activated()), this, SLOT(zoomIn()));
	shortcuts[sc_zoom_out] = new QShortcut(shortcut_zoom_out, this);
	connect(shortcuts[sc_zoom_out], SIGNAL(activated()), this, SLOT(zoomOut()));
	shortcuts[sc_zoom_in_alt] = new QShortcut(shortcut_zoom_in_alt, this);
	connect(shortcuts[sc_zoom_in_alt], SIGNAL(activated()), this, SLOT(zoomIn()));
	shortcuts[sc_zoom_out_alt] = new QShortcut(shortcut_zoom_out_alt, this);
	connect(shortcuts[sc_zoom_out_alt], SIGNAL(activated()), this, SLOT(zoomOut()));

	for (int idx = 0; idx < shortcuts.size(); idx++) {
		// assign widget shortcuts to all of them
		shortcuts[idx]->setContext(Qt::WidgetWithChildrenShortcut);
	}

}

void DkBaseViewPort::zoomConstraints(float minZoom, float maxZoom) {

	this->minZoom = minZoom;
	this->maxZoom = maxZoom;
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

void DkBaseViewPort::togglePattern(bool show) {

	DkSettings::display.tpPattern = show;
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

	if (imgStorage.getImage().isNull())
		return;

	//factor/=5;//-0.1 <-> 0.1
	//factor+=1;//0.9 <-> 1.1

	//limit zoom out ---
	if (worldMatrix.m11()*factor < minZoom && factor < 1)
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
	if (worldMatrix.m11()*imgMatrix.m11() > maxZoom && factor > 1)
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

	QImage imgQt = DkImage::mat2QImage(newImg);
	setImage(imgQt);
}
#endif

void DkBaseViewPort::setImage(QImage newImg) {

	imgStorage.setImage(newImg);
	QRectF oldImgRect = imgRect;
	this->imgRect = QRectF(0, 0, newImg.width(), newImg.height());

	emit enableNoImageSignal(!newImg.isNull());

	if (!DkSettings::display.keepZoom || imgRect != oldImgRect)
		worldMatrix.reset();							

	updateImageMatrix();
	update();
}

QImage DkBaseViewPort::getImage() {

	if (movie)
		return movie->currentImage();

	return imgStorage.getImage();
}

QRectF DkBaseViewPort::getImageViewRect() {

	return worldMatrix.mapRect(imgViewRect);
}

QImage DkBaseViewPort::getCurrentImageRegion() {

	QRectF viewRect = QRectF(QPoint(), size());
	viewRect = worldMatrix.inverted().mapRect(viewRect);
	viewRect = imgMatrix.inverted().mapRect(viewRect);

	QImage imgR(viewRect.size().toSize(), QImage::Format_ARGB32);
	imgR.fill(0);

	QPainter painter(&imgR);
	painter.drawImage(imgR.rect(), imgStorage.getImage(), viewRect.toRect());
	painter.end();

	return imgR;
}

bool DkBaseViewPort::unloadImage(bool fileChange) {

	return true;
}

// events --------------------------------------------------------------------
void DkBaseViewPort::paintEvent(QPaintEvent* event) {

	QPainter painter(viewport());

	qDebug() << "painting...";
	if (imgStorage.hasImage()) {
		painter.setWorldTransform(worldMatrix);

		// don't interpolate if we are forced to, at 100% or we exceed the maximal interpolation level
		if (!forceFastRendering && // force?
			fabs(imgMatrix.m11()*worldMatrix.m11()-1.0f) > FLT_EPSILON && // @100% ?
			imgMatrix.m11()*worldMatrix.m11() <= (float)DkSettings::display.interpolateZoomLevel/100.0f) {	// > max zoom level
				painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);
		}

		draw(&painter);

		//Now disable matrixWorld for overlay display
		painter.setWorldMatrixEnabled(false);
		qDebug() << "&& storage is not empty...";
	}

	painter.end();

	emit imageUpdated();	// TODO: delay timer is important here!

	// propagate
	QGraphicsView::paintEvent(event);

}

void DkBaseViewPort::resizeEvent(QResizeEvent *event) {

	if (event->oldSize() == event->size())
		return;

	viewportRect = QRect(0, 0, event->size().width(), event->size().height());

	qDebug() << "new size: " << event->size();
	//// do we still need that??
	//QSize newSize = imgStorage.getImage().size();
	//newSize.scale(event->size(), Qt::IgnoreAspectRatio);

	//newSize = (event->size()-newSize)/2;
	//move(newSize.width(), newSize.height());

	updateImageMatrix();
	centerImage();
	changeCursor();

	return QGraphicsView::resizeEvent(event);
}

bool DkBaseViewPort::event(QEvent *event) {

#ifndef QT_NO_GESTURES	// for now we block all gestures on systems except for windows
	if (event->type() == QEvent::NativeGesture)
		return nativeGestureEvent(static_cast<QNativeGestureEvent*>(event));
	else if (event->type() == QEvent::Gesture)
		return gestureEvent(static_cast<QGestureEvent*>(event));
#endif

	//if (event->type() == QEvent::Paint)
	//	return QGraphicsView::event(event);
	//else
	return QGraphicsView::event(event);

	//qDebug() << "event caught..." << event->type();


}

#ifndef QT_NO_GESTURES
bool DkBaseViewPort::nativeGestureEvent(QNativeGestureEvent* event) {

	qDebug() << "native gesture...";

#if QT_VERSION < 0x050000

#ifdef WIN32
	float cZoom = event->argument;
#else
	float cZoom = 0;	// ignore on other os
#endif

	switch (event->gestureType) {
	case  QNativeGestureEvent::Zoom:

		if (lastZoom != 0 && startZoom != 0) {
			float scale = (cZoom-lastZoom)/startZoom;

			if (fabs(scale) > FLT_EPSILON) {
				zoom(1.0f+scale, event->position-QWidget::mapToGlobal(pos()));
				lastZoom = cZoom;
			}
		}
		else if (startZoom == 0)
			startZoom = cZoom;
		else if (lastZoom == 0)
			lastZoom = cZoom;



		qDebug() << "zooming: " << cZoom << " pos: " << event->position << " angle: " << event->angle;
		break;
	case QNativeGestureEvent::Pan:

		//if (!cZoom)	// sometimes a pan gesture is triggered at the end of a pinch gesture
		swipeGesture = swipeRecognition(event);

		qDebug() << "panning....";
		break;
	case QNativeGestureEvent::Rotate:
		qDebug() << "rotating: " << event->angle;
	case QNativeGestureEvent::Swipe:
		qDebug() << "SWIPPING..........................";
		break;
		//if (event->gestureType == QNativeGestureEvent::Pan) {
		//	QPoint dxy = event->position-lastPos;
		//	if (dxy.y() != 0)
		//		verticalScrollBar()->setValue(verticalScrollBar()->value()-dxy.y());
		//}
	case QNativeGestureEvent::GestureBegin:
		posGrab = event->position;
		lastZoom = cZoom;
		startZoom = cZoom;
		swipeGesture = no_swipe;
		qDebug() << "beginning";
		break;
	case QNativeGestureEvent::GestureEnd:

		swipeAction(swipeGesture);

		posGrab = QPoint();
		lastZoom = 0;
		startZoom = 0;
		qDebug() << "ending...";
		break;
	default:
		return false;	// ignored type
	}
#endif

	return true;
}
#endif


bool DkBaseViewPort::gestureEvent(QGestureEvent* event) {

	if (QGesture *swipeG = event->gesture(Qt::SwipeGesture)) {
		QSwipeGesture *swipe = static_cast<QSwipeGesture *>(swipeG);

		// thanks qt documentation : )
		if (swipe->state() == Qt::GestureFinished) {
			if (swipe->horizontalDirection() == QSwipeGesture::Left
				|| swipe->verticalDirection() == QSwipeGesture::Up)
				qDebug() << "here comes the previous image function...";
			else
				qDebug() << "here comes the next image function...";
		}
		qDebug() << "swiping...";
	}
	else if (QGesture *pan = event->gesture(Qt::PanGesture)) {

		qDebug() << "panning...";
	}
	else if (QGesture *pinch = event->gesture(Qt::PinchGesture)) {

		qDebug() << "pinching...";
	}
	else
		return false;

	return true;
}

// key events --------------------------------------------------------------------
void DkBaseViewPort::keyPressEvent(QKeyEvent* event) {

	QWidget::keyPressEvent(event);
}

void DkBaseViewPort::keyReleaseEvent(QKeyEvent* event) {

#ifdef DK_DLL_EXPORT
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

		if (event->modifiers() == ctrlMod && event->modifiers() != altMod) {
			setCursor(Qt::CrossCursor);
			emit showStatusBar(true, false);
		}
		else if (worldMatrix.m11() > 1 && !imageInside())
			setCursor(Qt::OpenHandCursor);
		else {

			if (!DkSettings::app.showStatusBar)
				emit showStatusBar(false, false);

			unsetCursor();
		}

	}


	QWidget::mouseMoveEvent(event);
}

void DkBaseViewPort::wheelEvent(QWheelEvent *event) {

	float factor = -event->delta();
	if (DkSettings::display.invertZoom) factor *= -1.0f;

	factor /= -1200.0f;
	factor += 1.0f;

	qDebug() << "zoom factor..." << factor;
	zoom(factor, event->pos());
}

void DkBaseViewPort::contextMenuEvent(QContextMenuEvent *event) {

	qDebug() << "viewport event...";

	// send this event to my parent...
	QWidget::contextMenuEvent(event);
}

// protected functions --------------------------------------------------------------------
void DkBaseViewPort::draw(QPainter *painter) {

	//QImage imgDraw = getScaledImage(imgMatrix.m11()*worldMatrix.m11());
	//painter->drawImage(imgViewRect, imgDraw, QRect(QPoint(), imgDraw.size()));
	if (parent && parent->isFullScreen()) {
		painter->setWorldMatrixEnabled(false);
		painter->fillRect(QRect(QPoint(), size()), DkSettings::slideShow.backgroundColor);
		painter->setWorldMatrixEnabled(true);
	}

	if (backgroundBrush() != Qt::NoBrush) {
		painter->setWorldMatrixEnabled(false);
		painter->fillRect(QRect(QPoint(), size()), backgroundBrush());
		painter->setWorldMatrixEnabled(true);
	}

	QImage imgQt = imgStorage.getImage(imgMatrix.m11()*worldMatrix.m11());

	if (DkSettings::display.tpPattern && imgQt.hasAlphaChannel()) {

		// don't scale the pattern...
		QTransform scaleIv;
		scaleIv.scale(worldMatrix.m11(), worldMatrix.m22());
		pattern.setTransform(scaleIv.inverted());

		painter->setPen(QPen(Qt::NoPen));	// no border
		painter->setBrush(pattern);
		painter->drawRect(imgViewRect);
	}

	if (!movie || !movie->isValid())
		painter->drawImage(imgViewRect, imgQt, imgQt.rect());
	else {
		painter->drawPixmap(imgViewRect, movie->currentPixmap(), movie->frameRect());
	}
	//qDebug() << "view rect: " << imgStorage.getImage().size()*imgMatrix.m11()*worldMatrix.m11() << " img rect: " << imgQt.size();
}

bool DkBaseViewPort::imageInside() {

	return worldMatrix.m11() <= 1.0f || viewportRect.contains(worldMatrix.mapRect(imgViewRect));
}

void DkBaseViewPort::updateImageMatrix() {

	if (imgStorage.getImage().isNull())
		return;

	QRectF oldImgRect = imgViewRect;
	QTransform oldImgMatrix = imgMatrix;

	imgMatrix.reset();

	// if the image is smaller or zoom is active: paint the image as is
	if (!viewportRect.contains(imgRect))
		imgMatrix = getScaledImageMatrix();
	else {
		imgMatrix.translate((float)(width()-imgStorage.getImage().width())*0.5f, (float)(height()-imgStorage.getImage().height())*0.5f);
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

//QImage DkBaseViewPort::getScaledImage(float factor) {
//
//// this function does not help anything if we cannot interpolate with OpenCV
//#ifndef WITH_OPENCV
//	return imgQt;
//#endif
//
//	if (factor > 0.5f)
//		return imgQt;
//
//	int divisor = DkMath::getNextPowerOfTwoDivisior(factor);
//
//	//if (divisor < 2)
//	//	return imgQt;
//
//	// is the image cached already?
//	if (imgPyramid.contains(divisor))
//		return imgPyramid.value(divisor);
//
//	QSize newSize = imgQt.size()*1.0f/(float)divisor;
//
//	//// caching should not consume more than 30 MB
//	//if (newSize.width()*newSize.height() > 30 * 2^20)
//	//	return imgQt;
//
//#ifdef WITH_OPENCV
//
//	Mat resizeImage = DkImage::qImage2Mat(imgQt);
//
//	// is the image convertible?
//	if (resizeImage.empty())
//		return imgQt;
//
//	Mat tmp;
//	cv::resize(resizeImage, tmp, cv::Size(newSize.width(), newSize.height()), 0, 0, CV_INTER_AREA);
//	resizeImage = tmp;
//	QImage iplImg = DkImage::mat2QImage(resizeImage);
//
//	imgPyramid.insert(divisor, iplImg);
//
//	return iplImg;
//#endif
//
//	return imgQt;
//}

void DkBaseViewPort::controlImagePosition(float lb, float ub) {

	QRectF imgRectWorld = worldMatrix.mapRect(imgViewRect);

	if (lb == -1 && ub == -1 && panControl.x() != -1 && panControl.y() != -1) {
		lb = panControl.x(); 
		ub = panControl.y();
	}
	else {

		// default behavior
		if (lb == -1)	lb = viewportRect.width()/2;
		if (ub == -1)	ub = viewportRect.height()/2;
	}

	if (imgRectWorld.left() > lb && imgRectWorld.width() > width())
		worldMatrix.translate((lb-imgRectWorld.left())/worldMatrix.m11(), 0);

	if (imgRectWorld.top() > ub && imgRectWorld.height() > height())
		worldMatrix.translate(0, (ub-imgRectWorld.top())/worldMatrix.m11());

	if (imgRectWorld.right() < width()-lb && imgRectWorld.width() > width())
		worldMatrix.translate(((width()-lb)-imgRectWorld.right())/worldMatrix.m11(), 0);

	if (imgRectWorld.bottom() < height()-ub && imgRectWorld.height() > height())
		worldMatrix.translate(0, ((height()-ub)-imgRectWorld.bottom())/worldMatrix.m11());
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

void DkBaseViewPort::setBackgroundBrush(const QBrush &brush) {

	QGraphicsView::setBackgroundBrush(brush);
}

}
