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
#include "DkActionManager.h"
#include "DkSettings.h"
#include "DkUtils.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QCoreApplication>
#include <QTimer>
#include <QMovie>
#include <QShortcut>
#include <QDebug>
#include <QTimer>
#include <QSvgRenderer>
#include <QMainWindow>

// gestures
#include <QSwipeGesture>

#if QT_VERSION < 0x050000
// native gestures
#ifndef QT_NO_GESTURES
#include "extern/qevent_p.h"
#endif
#endif
#pragma warning(pop)		// no warnings from includes - end

#include <float.h>

namespace nmc {
	
// DkBaseViewport --------------------------------------------------------------------
DkBaseViewPort::DkBaseViewPort(QWidget *parent) : QGraphicsView(parent) {

	grabGesture(Qt::PanGesture);
	grabGesture(Qt::PinchGesture);
	grabGesture(Qt::SwipeGesture);
	setAttribute(Qt::WA_AcceptTouchEvents);

	mForceFastRendering = false;
	mViewportRect = QRect(0, 0, width(), height());
	mWorldMatrix.reset();
	mImgMatrix.reset();

	mPanControl = QPointF(-1.0f, -1.0f);
	mMinZoom = 0.01f;
	mMaxZoom = 50;

	mBlockZooming = false;
	mAltMod = DkSettings::global.altMod;
	mCtrlMod = DkSettings::global.ctrlMod;

	mZoomTimer = new QTimer(this);
	mZoomTimer->setSingleShot(true);
	connect(mZoomTimer, SIGNAL(timeout()), this, SLOT(stopBlockZooming()));
	connect(&mImgStorage, SIGNAL(imageUpdated()), this, SLOT(update()));

	mPattern.setTexture(QPixmap(":/nomacs/img/tp-pattern.png"));

	if (DkSettings::display.useDefaultColor) {

		if (DkSettings::display.toolbarGradient)
			setObjectName("DkBaseViewPortGradient");
		else
			setObjectName("DkBaseViewPortDefaultColor");		
	}
	else
		setObjectName("DkBaseViewPort");


	setMouseTracking(true);

	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	setMinimumSize(10, 10);

	createShortcuts();
}

DkBaseViewPort::~DkBaseViewPort() {

	release();
}

void DkBaseViewPort::createShortcuts() {

	mShortcuts.resize(sc_end);

	// panning
	mShortcuts[sc_pan_left] = new QShortcut(shortcut_panning_left, this);
	connect(mShortcuts[sc_pan_left], SIGNAL(activated()), this, SLOT(shiftLeft()));
	mShortcuts[sc_pan_right] = new QShortcut(shortcut_panning_right, this);
	connect(mShortcuts[sc_pan_right], SIGNAL(activated()), this, SLOT(shiftRight()));
	mShortcuts[sc_pan_up] = new QShortcut(shortcut_panning_up, this);
	connect(mShortcuts[sc_pan_up], SIGNAL(activated()), this, SLOT(shiftUp()));
	mShortcuts[sc_pan_down] = new QShortcut(shortcut_panning_down, this);
	connect(mShortcuts[sc_pan_down], SIGNAL(activated()), this, SLOT(shiftDown()));

	//// zoom
	//mShortcuts[sc_zoom_in] = new QShortcut(shortcut_zoom_in, this);
	////connect(mShortcuts[sc_zoom_in], SIGNAL(activated()), this, SLOT(zoomIn()));
	//mShortcuts[sc_zoom_out] = new QShortcut(shortcut_zoom_out, this);
	//connect(mShortcuts[sc_zoom_out], SIGNAL(activated()), this, SLOT(zoomOut()));
	//mShortcuts[sc_zoom_in_alt] = new QShortcut(shortcut_zoom_in_alt, this);
	//connect(mShortcuts[sc_zoom_in_alt], SIGNAL(activated()), this, SLOT(zoomIn()));
	//mShortcuts[sc_zoom_out_alt] = new QShortcut(shortcut_zoom_out_alt, this);
	//connect(mShortcuts[sc_zoom_out_alt], SIGNAL(activated()), this, SLOT(zoomOut()));

	for (int idx = 0; idx < mShortcuts.size(); idx++) {
		// assign widget shortcuts to all of them
		mShortcuts[idx]->setContext(Qt::WidgetWithChildrenShortcut);
	}

}

void DkBaseViewPort::zoomConstraints(float minZoom, float maxZoom) {

	mMinZoom = minZoom;
	mMaxZoom = maxZoom;
}

void DkBaseViewPort::release() {
}

QWidget* DkBaseViewPort::parentWidget() const {

	return qobject_cast<QWidget*>(parent());
}

// zoom - pan --------------------------------------------------------------------
void DkBaseViewPort::resetView() {

	mWorldMatrix.reset();
	changeCursor();

	update();
}

void DkBaseViewPort::fullView() {

	mWorldMatrix.reset();
	zoom(1.0f/(float)mImgMatrix.m11());
	changeCursor();

	update();
}

void DkBaseViewPort::togglePattern(bool show) {

	DkSettings::display.tpPattern = show;
	update();
}

void DkBaseViewPort::shiftLeft() {

	float delta = 2*width()/(100.0f*(float)mWorldMatrix.m11());
	moveView(QPointF(delta,0));
}

void DkBaseViewPort::shiftRight() {

	float delta = -2*width()/(100.0f*(float)mWorldMatrix.m11());
	moveView(QPointF(delta,0));
}

void DkBaseViewPort::shiftUp() {

	float delta = 2*height()/(100.0f*(float)mWorldMatrix.m11());
	moveView(QPointF(0,delta));
}

void DkBaseViewPort::shiftDown() {

	float delta = -2*height()/(100.0f*(float)mWorldMatrix.m11());
	moveView(QPointF(0,delta));
}

void DkBaseViewPort::moveView(const QPointF& delta) {

	QPointF lDelta = delta;
	QRectF imgWorldRect = mWorldMatrix.mapRect(mImgViewRect);
	if (imgWorldRect.width() < width())
		lDelta.setX(0);
	if (imgWorldRect.height() < height())
		lDelta.setY(0);

	mWorldMatrix.translate(lDelta.x(), lDelta.y());
	controlImagePosition();
	update();
}


void DkBaseViewPort::zoomIn() {

	zoom(1.5f);
}

void DkBaseViewPort::zoomOut() {

	zoom(0.5f);
}

void DkBaseViewPort::zoom(float factor, QPointF center) {

	if (mImgStorage.getImage().isNull())
		return;

	//factor/=5;//-0.1 <-> 0.1
	//factor+=1;//0.9 <-> 1.1

	//limit zoom out ---
	if (mWorldMatrix.m11()*factor < mMinZoom && factor < 1)
		return;

	//if (worldMatrix.m11()*factor < 1) {
	//	resetView();
	//	return;
	//}

	// reset view & block if we pass the 'image fit to screen' on zoom out
	if (mWorldMatrix.m11() > 1 && mWorldMatrix.m11()*factor < 1) {

		mBlockZooming = true;
		mZoomTimer->start(500);
		resetView();
		return;
	}

	// reset view if we pass the 'image fit to screen' on zoom in
	if (mWorldMatrix.m11() < 1 && mWorldMatrix.m11()*factor > 1) {

		resetView();
		return;
	}

	//limit zoom in ---
	if (mWorldMatrix.m11()*mImgMatrix.m11() > mMaxZoom && factor > 1)
		return;

	// if no center assigned: zoom in at the image center
	if (center.x() == -1 || center.y() == -1)
		center = mImgViewRect.center();

	//inverse the transform
	int a, b;
	mWorldMatrix.inverted().map(qRound(center.x()), qRound(center.y()), &a, &b);

	mWorldMatrix.translate(a-factor*a, b-factor*b);
	mWorldMatrix.scale(factor, factor);

	controlImagePosition();
	changeCursor();

	update();
}

void DkBaseViewPort::stopBlockZooming() {
	mBlockZooming = false;
}

// set image --------------------------------------------------------------------
#ifdef WITH_OPENCV
void DkBaseViewPort::setImage(cv::Mat newImg) {

	QImage imgQt = DkImage::mat2QImage(newImg);
	setImage(imgQt);
}
#endif

void DkBaseViewPort::setImage(QImage newImg) {

	mImgStorage.setImage(newImg);
	QRectF oldImgRect = mImgRect;
	mImgRect = QRectF(QPoint(), getImageSize());
	
	emit enableNoImageSignal(!newImg.isNull());

	if (!DkSettings::display.keepZoom || mImgRect != oldImgRect)
		mWorldMatrix.reset();							

	updateImageMatrix();
	update();
	emit newImageSignal(&newImg);
}

QImage DkBaseViewPort::getImage() const {
	
	if (mMovie && mMovie->isValid())
		return mMovie->currentImage();

	return mImgStorage.getImageConst();
}

QSize DkBaseViewPort::getImageSize() const {

	if (mSvg) {
		qDebug() << "win: " << size() << "svg:" << mSvg->defaultSize() << "scaled:" << mSvg->defaultSize().scaled(size(), Qt::KeepAspectRatio);
		return mSvg->defaultSize().scaled(size(), Qt::KeepAspectRatio);
	}

	return mImgStorage.getImageConst().size();
}

QRectF DkBaseViewPort::getImageViewRect() const {

	return mWorldMatrix.mapRect(mImgViewRect);
}

QImage DkBaseViewPort::getCurrentImageRegion() {

	QRectF viewRect = QRectF(QPoint(), size());
	viewRect = mWorldMatrix.inverted().mapRect(viewRect);
	viewRect = mImgMatrix.inverted().mapRect(viewRect);

	QImage imgR(viewRect.size().toSize(), QImage::Format_ARGB32);
	imgR.fill(0);

	QPainter painter(&imgR);
	painter.drawImage(imgR.rect(), mImgStorage.getImage(), viewRect.toRect());
	painter.end();

	return imgR;
}

bool DkBaseViewPort::unloadImage(bool) {

	return true;
}

// events --------------------------------------------------------------------
void DkBaseViewPort::paintEvent(QPaintEvent* event) {

	QPainter painter(viewport());

	qDebug() << "painting...";
	if (mImgStorage.hasImage()) {
		painter.setWorldTransform(mWorldMatrix);

		// don't interpolate if we are forced to, at 100% or we exceed the maximal interpolation level
		if (!mForceFastRendering && // force?
			fabs(mImgMatrix.m11()*mWorldMatrix.m11()-1.0f) > FLT_EPSILON && // @100% ?
			mImgMatrix.m11()*mWorldMatrix.m11() <= (float)DkSettings::display.interpolateZoomLevel/100.0f) {	// > max zoom level
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

	mViewportRect = QRect(0, 0, event->size().width(), event->size().height());

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
	float cZoom = (float)event->argument;
#else
	float cZoom = 0;	// ignore on other os
#endif

	switch (event->gestureType) {
	case  QNativeGestureEvent::Zoom:

		if (mLastZoom != 0 && mStartZoom != 0) {
			float scale = cZoom-mLastZoom;
			scale /= 100;	// tested on surface 2 - is pretty handy like this...

			if (fabs(scale) > FLT_EPSILON) {
				zoom(1.0f+scale, event->position-QWidget::mapToGlobal(pos()));
				mLastZoom = cZoom;
			}
		}
		else if (mStartZoom == 0)
			mStartZoom = cZoom;
		else if (mLastZoom == 0)
			mLastZoom = cZoom;



		qDebug() << "zooming: " << cZoom << " pos: " << event->position << " angle: " << event->angle;
		break;
	case QNativeGestureEvent::Pan:

		//if (!cZoom)	// sometimes a pan gesture is triggered at the end of a pinch gesture
		mSwipeGesture = swipeRecognition(event);

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
		mPosGrab = event->position;
		mLastZoom = cZoom;
		mStartZoom = cZoom;
		mSwipeGesture = no_swipe;
		qDebug() << "beginning";
		break;
	case QNativeGestureEvent::GestureEnd:

		swipeAction(mSwipeGesture);

		mPosGrab = QPoint();
		mLastZoom = 0;
		mStartZoom = 0;
		qDebug() << "ending...";
		break;
	default:
		return false;	// ignored type
	}
#else
	Q_UNUSED(event);
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
	else if (QPinchGesture *pinch = static_cast<QPinchGesture*>(event->gesture(Qt::PinchGesture))) {

#if QT_VERSION >= 0x050000
		float scale = (float)pinch->lastScaleFactor();

		if (fabs(scale-1.0f) > FLT_EPSILON) {
			zoom(scale, mapFromGlobal(pinch->centerPoint().toPoint()));
		}
#endif
		qDebug() << "[Qt] pinching...";
	}
	else if (QGesture *pan = event->gesture(Qt::PanGesture)) {

		qDebug() << "panning...";
	}
	else
		return false;

	return true;
}

// key events --------------------------------------------------------------------
void DkBaseViewPort::keyPressEvent(QKeyEvent* event) {

	// we want to change the behaviour on auto-repeat - so we cannot use QShortcuts here...
	if (event->key() == shortcut_zoom_in || event->key() == shortcut_zoom_in_alt) {
		zoom(event->isAutoRepeat() ? 1.1f : 1.5f);
	}
	if (event->key() == shortcut_zoom_out || event->key() == shortcut_zoom_out_alt) {
		zoom(event->isAutoRepeat() ? 0.9f : 0.5f);
	}

	qDebug() << "keypress event: " << event->key() << "sc" << shortcut_zoom_in;

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
	if (mWorldMatrix.m11() > 1 && !imageInside() && event->buttons() == Qt::LeftButton) {
		setCursor(Qt::ClosedHandCursor);
	}

	mPosGrab = event->pos();

	QWidget::mousePressEvent(event);
}

void DkBaseViewPort::mouseReleaseEvent(QMouseEvent *event) {

	if (mWorldMatrix.m11() > 1 && !imageInside())
		setCursor(Qt::OpenHandCursor);

	QWidget::mouseReleaseEvent(event);
}

void DkBaseViewPort::mouseDoubleClickEvent(QMouseEvent *event) {

	QCoreApplication::sendEvent(parentWidget(), event);
}

void DkBaseViewPort::mouseMoveEvent(QMouseEvent *event) {

	if (mWorldMatrix.m11() > 1 && event->buttons() == Qt::LeftButton) {

		QPointF cPos = event->pos();
		QPointF dxy = (cPos - mPosGrab);
		mPosGrab = cPos;
		moveView(dxy/mWorldMatrix.m11());
	}
	if (event->buttons() != Qt::LeftButton && event->buttons() != Qt::RightButton) {

		if (event->modifiers() == mCtrlMod && event->modifiers() != mAltMod) {
			setCursor(Qt::CrossCursor);
			emit showStatusBar(true, false);
		}
		else if (mWorldMatrix.m11() > 1 && !imageInside())
			setCursor(Qt::OpenHandCursor);
		else {

			if (!DkSettings::app.showStatusBar)
				emit showStatusBar(false, false);

			if (cursor().shape() != Qt::ArrowCursor)
				unsetCursor();
		}

	}


	QWidget::mouseMoveEvent(event);
}

void DkBaseViewPort::wheelEvent(QWheelEvent *event) {

	float factor = (float)-event->delta();
	if (DkSettings::display.invertZoom) factor *= -1.0f;

	factor /= -1200.0f;
	factor += 1.0f;

	//qDebug() << "zoom factor..." << factor;
	zoom(factor, event->pos());
}

void DkBaseViewPort::contextMenuEvent(QContextMenuEvent *event) {

	qDebug() << "mViewport event...";

	// send this event to my parent...
	QWidget::contextMenuEvent(event);
}

// protected functions --------------------------------------------------------------------
void DkBaseViewPort::draw(QPainter *painter, float opacity) {

	if (parentWidget() && DkActionManager::instance().getMainWindow()->isFullScreen()) {
		painter->setWorldMatrixEnabled(false);
		painter->fillRect(QRect(QPoint(), size()), DkSettings::slideShow.backgroundColor);
		painter->setWorldMatrixEnabled(true);
	}

	if (backgroundBrush() != Qt::NoBrush) {
		painter->setWorldMatrixEnabled(false);
		painter->fillRect(QRect(QPoint(), size()), backgroundBrush());
		painter->setWorldMatrixEnabled(true);
	}

	QImage imgQt = mImgStorage.getImage((float)(mImgMatrix.m11()*mWorldMatrix.m11()));

	// opacity == 1.0f -> do not show pattern if we crossfade two images
	if (DkSettings::display.tpPattern && imgQt.hasAlphaChannel() && opacity == 1.0f) {

		// don't scale the pattern...
		QTransform scaleIv;
		scaleIv.scale(mWorldMatrix.m11(), mWorldMatrix.m22());
		mPattern.setTransform(scaleIv.inverted());

		painter->setPen(QPen(Qt::NoPen));	// no border
		painter->setBrush(mPattern);
		painter->drawRect(mImgViewRect);
	}

	float oldOp = (float)painter->opacity();
	painter->setOpacity(opacity);

	if (mSvg && mSvg->isValid()) {
		mSvg->render(painter, mImgViewRect);
	}
	else if (mMovie && mMovie->isValid())
		painter->drawPixmap(mImgViewRect, mMovie->currentPixmap(), mMovie->frameRect());
	else
		painter->drawImage(mImgViewRect, imgQt, imgQt.rect());

	painter->setOpacity(oldOp);

	//qDebug() << "view rect: " << imgStorage.getImage().size()*imgMatrix.m11()*worldMatrix.m11() << " img rect: " << imgQt.size();
}

bool DkBaseViewPort::imageInside() const {

	return mWorldMatrix.m11() <= 1.0f || mViewportRect.contains(mWorldMatrix.mapRect(mImgViewRect));
}

void DkBaseViewPort::updateImageMatrix() {

	if (mImgStorage.getImage().isNull())
		return;

	QRectF oldImgRect = mImgViewRect;
	QTransform oldImgMatrix = mImgMatrix;

	mImgMatrix.reset();

	QSize imgSize = getImageSize();

	// if the image is smaller or zoom is active: paint the image as is
	if (!mViewportRect.contains(mImgRect))
		mImgMatrix = getScaledImageMatrix();
	else {
		mImgMatrix.translate((float)(width()-imgSize.width())*0.5f, (float)(height()-imgSize.height())*0.5f);
		mImgMatrix.scale(1.0f, 1.0f);
	}

	mImgViewRect = mImgMatrix.mapRect(mImgRect);

	// update world matrix
	if (mWorldMatrix.m11() != 1) {

		float scaleFactor = (float)(oldImgMatrix.m11()/mImgMatrix.m11());
		double dx = oldImgRect.x()/scaleFactor-mImgViewRect.x();
		double dy = oldImgRect.y()/scaleFactor-mImgViewRect.y();

		mWorldMatrix.scale(scaleFactor, scaleFactor);
		mWorldMatrix.translate(dx, dy);
	}
}

QTransform DkBaseViewPort::getScaledImageMatrix() const {
	return getScaledImageMatrix(size());
}

QTransform DkBaseViewPort::getScaledImageMatrix(const QSize& size) const {

	// the image resizes as we zoom
	float ratioImg = (float)mImgRect.width()/(float)mImgRect.height();
	float ratioWin = (float)size.width()/(float)size.height();

	QTransform imgMatrix;
	float s;
	if (mImgRect.width() == 0 || mImgRect.height() == 0)
		s = 1.0f;
	else
		s = (ratioImg > ratioWin) ? (float)size.width()/(float)mImgRect.width() : (float)size.height()/(float)mImgRect.height();

	imgMatrix.scale(s, s);

	QRectF imgViewRect = imgMatrix.mapRect(mImgRect);
	imgMatrix.translate((size.width()-imgViewRect.width())*0.5f/s, (size.height()-imgViewRect.height())*0.5f/s);

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

	QRectF imgRectWorld = mWorldMatrix.mapRect(mImgViewRect);

	if (lb == -1 && ub == -1 && mPanControl.x() != -1 && mPanControl.y() != -1) {
		lb = (float)mPanControl.x(); 
		ub = (float)mPanControl.y();
	}
	else {

		// default behavior
		if (lb == -1)	lb = (float)mViewportRect.width()/2.0f;
		if (ub == -1)	ub = (float)mViewportRect.height()/2.0f;
	}

	if (imgRectWorld.left() > lb && imgRectWorld.width() > width())
		mWorldMatrix.translate((lb-imgRectWorld.left())/mWorldMatrix.m11(), 0);

	if (imgRectWorld.top() > ub && imgRectWorld.height() > height())
		mWorldMatrix.translate(0, (ub-imgRectWorld.top())/mWorldMatrix.m11());

	if (imgRectWorld.right() < width()-lb && imgRectWorld.width() > width())
		mWorldMatrix.translate(((width()-lb)-imgRectWorld.right())/mWorldMatrix.m11(), 0);

	if (imgRectWorld.bottom() < height()-ub && imgRectWorld.height() > height())
		mWorldMatrix.translate(0, ((height()-ub)-imgRectWorld.bottom())/mWorldMatrix.m11());
}

void DkBaseViewPort::centerImage() {

	QRectF imgWorldRect = mWorldMatrix.mapRect(mImgViewRect);
	float dx, dy;

	// if black border - center the image
	if (imgWorldRect.width() < (float)width()) {
		dx = (float)((width()-imgWorldRect.width())*0.5f-mImgViewRect.x()*mWorldMatrix.m11());
		dx = (dx-(float)mWorldMatrix.dx())/(float)mWorldMatrix.m11();
		mWorldMatrix.translate(dx, 0);
	}
	else if (imgWorldRect.left() > 0)
		mWorldMatrix.translate(-imgWorldRect.left()/mWorldMatrix.m11(), 0);
	else if (imgWorldRect.right() < width())
		mWorldMatrix.translate((width()-imgWorldRect.right())/mWorldMatrix.m11(), 0);

	if (imgWorldRect.height() < height()) {
		dy = (float)((height()-imgWorldRect.height())*0.5f-mImgViewRect.y()*mWorldMatrix.m22());
		dy = (float)((dy-mWorldMatrix.dy())/mWorldMatrix.m22());
		mWorldMatrix.translate(0, dy);
	}
	else if (imgWorldRect.top() > 0) {
		mWorldMatrix.translate(0, -imgWorldRect.top()/mWorldMatrix.m22());
	}
	else if (imgWorldRect.bottom() < height()) {
		mWorldMatrix.translate(0, (height()-imgWorldRect.bottom())/mWorldMatrix.m22());
	}
}

void DkBaseViewPort::changeCursor() {

	if (mWorldMatrix.m11() > 1 && !imageInside())
		setCursor(Qt::OpenHandCursor);
	else
		unsetCursor();
}

void DkBaseViewPort::setBackgroundBrush(const QBrush &brush) {

	QGraphicsView::setBackgroundBrush(brush);
}

}
