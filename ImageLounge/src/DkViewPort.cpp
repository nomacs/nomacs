/*******************************************************************************************************
 DkViewPort.cpp
 Created on:	05.05.2011
 
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

#include "DkViewPort.h"

#include "DkControlWidget.h"
#include "DkImageLoader.h"
#include "DkWidgets.h"
#include "DkSettings.h"
#include "DkNetwork.h"
#include "DkThumbsWidgets.h"		// needed in the connects -> shall we move them to mController?
#include "DkMetaDataWidgets.h"
#include "DkToolbars.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QClipboard>
#include <QShortcut>
#include <QMovie>
#include <QMimeData>
#include <QAction>
#include <QApplication>
#include <QVBoxLayout>
#include <QDragLeaveEvent>
#include <QDrag>
#include <QInputDialog>
#include <QMessageBox>
#include <QDesktopWidget>
#include <qmath.h>
#pragma warning(pop)		// no warnings from includes - end

namespace nmc {

// DkViewPort --------------------------------------------------------------------
DkViewPort::DkViewPort(QWidget *parent, Qt::WindowFlags flags) : DkBaseViewPort(parent) {

	qRegisterMetaType<QSharedPointer<DkImageContainerT> >( "QSharedPointer<DkImageContainerT>");
	qRegisterMetaType<QSharedPointer<DkImageContainerT> >( "QSharedPointer<nmc::DkImageContainerT>");
	qRegisterMetaType<QVector<QSharedPointer<DkImageContainerT> > >( "QVector<QSharedPointer<DkImageContainerT> >");

	mImgBg.load(":/nomacs/img/nomacs-bg.png");

	mRepeatZoomTimer->setInterval(20);
	connect(mRepeatZoomTimer, SIGNAL(timeout()), this, SLOT(repeatZoom()));

	mFadeTimer->setInterval(5);
	connect(mFadeTimer, SIGNAL(timeout()), this, SLOT(animateFade()));

	//no border
	setMouseTracking (true);//receive mouse event everytime
	
	mPaintLayout = new QVBoxLayout(this);
	mPaintLayout->setContentsMargins(0,0,0,0);

	createShortcuts();

	mController = new DkControlWidget(this, flags);
	//mController->show();

	mLoader = QSharedPointer<DkImageLoader>(new DkImageLoader());
	connectLoader(mLoader);

	mController->getOverview()->setTransforms(&mWorldMatrix, &mImgMatrix);
	mController->getCropWidget()->setWorldTransform(&mWorldMatrix);
	mController->getCropWidget()->setImageTransform(&mImgMatrix);
	mController->getCropWidget()->setImageRect(&mImgViewRect);

	connect(this, SIGNAL(enableNoImageSignal(bool)), mController, SLOT(imageLoaded(bool)));
	connect(&mImgStorage, SIGNAL(infoSignal(QString)), this, SIGNAL(infoSignal(QString)));
	
	qDebug() << "viewer created...";

	// TODO:
	// one could blur the canvas if a transparent GUI is present
	// what we would need: QGraphicsBlurEffect...
	// render all widgets to the alpha channel (bw)
	// pre-render the mViewport to that image... apply blur
	// and then render the blurred image after the widget is rendered...
	// performance?!
	
}

DkViewPort::~DkViewPort() {

	release();
}

void DkViewPort::release() {
}

void DkViewPort::createShortcuts() {

	//DkBaseViewPort::createShortcuts();

	mShortcuts.resize(scf_end);

	// TODO: make actions!!

	//// files
	//mShortcuts[sc_first_file] = new QShortcut(shortcut_first_file, this);
	//connect(mShortcuts[sc_first_file], SIGNAL(activated()), this, SLOT(loadFirst()));
	//mShortcuts[sc_last_file] = new QShortcut(shortcut_last_file, this);
	//connect(mShortcuts[sc_last_file], SIGNAL(activated()), this, SLOT(loadLast()));

	//mShortcuts[sc_skip_prev] = new QShortcut(shortcut_skip_prev, this);
	//mShortcuts[sc_skip_prev]->setContext(Qt::WidgetWithChildrenShortcut);
	//connect(mShortcuts[sc_skip_prev], SIGNAL(activated()), this, SLOT(loadSkipPrev10()));
	//mShortcuts[sc_skip_next] = new QShortcut(shortcut_skip_next, this);
	//mShortcuts[sc_skip_next]->setContext(Qt::WidgetWithChildrenShortcut);
	//connect(mShortcuts[sc_skip_next], SIGNAL(activated()), this, SLOT(loadSkipNext10()));
	//
	//mShortcuts[sc_first_sync] = new QShortcut(shortcut_first_file_sync, this);
	//connect(mShortcuts[sc_first_sync], SIGNAL(activated()), this, SLOT(loadFirst()));

	//mShortcuts[sc_last_sync] = new QShortcut(shortcut_last_file_sync, this);
	//connect(mShortcuts[sc_last_sync], SIGNAL(activated()), this, SLOT(loadLast()));

	//mShortcuts[sc_next_sync] = new QShortcut(shortcut_next_file_sync, this);
	//connect(mShortcuts[sc_next_sync], SIGNAL(activated()), this, SLOT(loadNextFileFast()));

	//mShortcuts[sc_prev_sync] = new QShortcut(shortcut_prev_file_sync, this);
	//connect(mShortcuts[sc_prev_sync], SIGNAL(activated()), this, SLOT(loadPrevFileFast()));

	//mShortcuts[sc_delete_silent] = new QShortcut(shortcut_delete_silent, this);

	//for (int idx = 0; idx < mShortcuts.size(); idx++) {
	//	// assign widget shortcuts to all of them
	//	mShortcuts[idx]->setContext(Qt::WidgetWithChildrenShortcut);
	//}


}

void DkViewPort::setPaintWidget(QWidget* widget, bool removeWidget) {

	if(!removeWidget) {
		mPaintLayout->addWidget(widget);
		//pluginImageWasApplied = false;
	} else {
		mPaintLayout->removeWidget(widget);
		//widget->deleteLater();
	}
	
	//mController->raise();
	
}

#ifdef WITH_OPENCV
void DkViewPort::setImage(cv::Mat newImg) {

	QImage imgQt = DkImage::mat2QImage(newImg);
	setImage(imgQt);
}
#endif

void DkViewPort::updateImage(QSharedPointer<DkImageContainerT> image, bool loaded) {

	// things todo if a file was not loaded...
	if (!loaded) {
		mController->getPlayer()->startTimer();
		return;
	}

	// should not happen -> the mLoader should send this signal
	if (!mLoader)
		return;

	if (mLoader->hasImage())
		setImage(mLoader->getImage());
}

void DkViewPort::loadImage(QImage newImg) {

	// delete current information
	if (mLoader) {
		if (!unloadImage(true))
			return;	// user canceled

		mLoader->setImage(newImg);
		setImage(newImg);

		// save to temp folder
		mLoader->saveTempFile(newImg);
	}
}

void DkViewPort::loadImage(QSharedPointer<DkImageContainerT> img) {

	if (mLoader) {

		if (!unloadImage(true))
			return;

		if (img->hasImage()) {
			mLoader->setCurrentImage(img);
			setImage(img->image());
		}
		mLoader->load(img);
	}

}

void DkViewPort::setImage(QImage newImg) {

	DkTimer dt;

	emit movieLoadedSignal(false);

	stopMovie();	// just to be sure

	//imgPyramid.clear();

	mController->getOverview()->setImage(QImage());	// clear overview

	mImgStorage.setImage(newImg);
	mImgRect = QRectF(0, 0, newImg.width(), newImg.height());

	if (mLoader->hasMovie() && !mLoader->isEdited())
		loadMovie();

	emit enableNoImageSignal(!newImg.isNull());

	//qDebug() << "new image (mViewport) loaded,  size: " << newImg.size() << "channel: " << imgQt.format();
	//qDebug() << "keep zoom is always: " << (DkSettings::display.keepZoom == DkSettings::zoom_always_keep);

	if (!DkSettings::slideShow.moveSpeed && (DkSettings::display.keepZoom == DkSettings::zoom_never_keep || 
		(DkSettings::display.keepZoom == DkSettings::zoom_keep_same_size && mOldImgRect != mImgRect)) ||
		 mOldImgRect.isEmpty())
		mWorldMatrix.reset();

	updateImageMatrix();		

	// if image is not inside, we'll align it at the top left border
	if (!mViewportRect.intersects(mWorldMatrix.mapRect(mImgViewRect))) {
		mWorldMatrix.translate(-mWorldMatrix.dx(), -mWorldMatrix.dy());
		centerImage();
	}

	mController->getPlayer()->startTimer();
	mController->getOverview()->setImage(newImg);	// TODO: maybe we could make use of the image pyramid here
	mController->stopLabels();

	mOldImgRect = mImgRect;
	
	// init fading
	if (DkSettings::display.fadeSec && (mController->getPlayer()->isPlaying() || parentWidget() && parentWidget()->isFullScreen())) {
		mFadeTimer->start();
		mFadeTime.start();
	}
	else
		mFadeOpacity = 0.0f;

	update();

	// draw a histogram from the image -> does nothing if the histogram is invisible
	if (mController->getHistogram()) mController->getHistogram()->drawHistogram(newImg);
	if (DkSettings::sync.syncMode == DkSettings::sync_mode_remote_display)
		tcpSendImage(true);

	emit newImageSignal(&newImg);
	emit zoomSignal((float)(mWorldMatrix.m11()*mImgMatrix.m11()*100));
}

void DkViewPort::setThumbImage(QImage newImg) {
	
	DkTimer dt;
	//imgPyramid.clear();

	mImgStorage.setImage(newImg);
	QRectF oldImgRect = mImgRect;
	this->mImgRect = QRectF(0, 0, newImg.width(), newImg.height());

	emit enableNoImageSignal(true);

	if (!DkSettings::display.keepZoom || mImgRect != oldImgRect)
		mWorldMatrix.reset();							

	updateImageMatrix();
	
	mController->getOverview()->setImage(newImg);
	mController->stopLabels();

	update();

	qDebug() << "setting the image took me: " << dt.getTotal();
}

void DkViewPort::tcpSendImage(bool silent) {

	if (!silent)
		mController->setInfo("sending image...", 3000, DkControlWidget::center_label);

	if (mLoader)
		emit sendImageSignal(mImgStorage.getImage(), mLoader->fileName());
	else
		emit sendImageSignal(mImgStorage.getImage(), "nomacs - Image Lounge");
}

void DkViewPort::zoom(float factor, QPointF center) {

	if (mImgStorage.getImage().isNull() || mBlockZooming)
		return;

	//factor/=5;//-0.1 <-> 0.1
	//factor+=1;//0.9 <-> 1.1

	//limit zoom out ---
	if (mWorldMatrix.m11()*factor < mMinZoom && factor < 1)
		return;

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

	bool blackBorder = false;

	// if no center assigned: zoom in at the image center
	if (center.x() == -1 || center.y() == -1)
		center = mImgViewRect.center();
	else {

		// if black border - do not zoom to the mouse coordinate
		if ((float)mImgViewRect.width()*(mWorldMatrix.m11()*factor) < (float)width()) {
			center.setX(mImgViewRect.center().x());
			blackBorder = true;
		}
		if (((float)mImgViewRect.height()*mWorldMatrix.m11()*factor) < (float)height()) {
			center.setY(mImgViewRect.center().y());
			blackBorder = true;
		}
	}

	//inverse the transform
	int a, b;
	mWorldMatrix.inverted().map((int)center.x(), (int)center.y(), &a, &b);

	mWorldMatrix.translate(a-factor*a, b-factor*b);
	mWorldMatrix.scale(factor, factor);
	
	controlImagePosition();
	if (blackBorder && factor < 1) centerImage();	// TODO: geht auch schöner
	showZoom();
	changeCursor();

	mController->update();	// why do we need to update the mController manually?
	update();

	tcpSynchronize();

	emit zoomSignal((float)(mWorldMatrix.m11()*mImgMatrix.m11()*100));
	
}

void DkViewPort::zoomTo(float zoomLevel, const QPoint&) {

	mWorldMatrix.reset();
	zoom(zoomLevel/(float)mImgMatrix.m11());
}

void DkViewPort::resetView() {

	mWorldMatrix.reset();
	showZoom();
	changeCursor();

	update();

	tcpSynchronize();
}

void DkViewPort::zoomToFit() {

	QSizeF imgSize = mImgStorage.getImage().size();
	QSizeF winSize = size();

	float zoomLevel = (float)qMin(winSize.width()/imgSize.width(), winSize.height()/imgSize.height());
	zoomTo(zoomLevel);
}

void DkViewPort::fullView() {

	mWorldMatrix.reset();
	zoom(1.0f/(float)mImgMatrix.m11());
	showZoom();
	changeCursor();
	update();
	if (this->visibleRegion().isEmpty()) qDebug() << "empty region...";
}

void DkViewPort::showZoom() {

	QString zoomStr;
	zoomStr.sprintf("%.1f%%", mImgMatrix.m11()*mWorldMatrix.m11()*100);
	
	if (!mController->getZoomWidget()->isVisible())
		mController->setInfo(zoomStr, 3000, DkControlWidget::bottom_left_label);
}

void DkViewPort::repeatZoom() {

	qDebug() << "repeating...";
	if (DkSettings::display.invertZoom && QApplication::mouseButtons() == Qt::XButton1 ||
		!DkSettings::display.invertZoom && QApplication::mouseButtons() == Qt::XButton2)
		zoom(1.1f);
	else if (!DkSettings::display.invertZoom && QApplication::mouseButtons() == Qt::XButton1 ||
		DkSettings::display.invertZoom && QApplication::mouseButtons() == Qt::XButton2)
		zoom(0.9f);
	else
		mRepeatZoomTimer->stop();	// safety if we don't catch the release

}

void DkViewPort::toggleResetMatrix() {

	DkSettings::display.keepZoom = !DkSettings::display.keepZoom;
}

void DkViewPort::updateImageMatrix() {

	if (mImgStorage.getImage().isNull())
		return;

	QRectF oldImgRect = mImgViewRect;
	QTransform oldImgMatrix = mImgMatrix;

	mImgMatrix.reset();

	// if the image is smaller or zoom is active: paint the image as is
	if (!mViewportRect.contains(mImgRect.toRect()))
		mImgMatrix = getScaledImageMatrix();
	else {
		mImgMatrix.translate((float)(getMainGeometry().width()-mImgStorage.getImage().width())*0.5f, (float)(getMainGeometry().height()-mImgStorage.getImage().height())*0.5f);
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

void DkViewPort::tcpSetTransforms(QTransform newWorldMatrix, QTransform newImgMatrix, QPointF canvasSize) {

	// ok relative transform
	if (canvasSize.isNull()) {
		moveView(QPointF(newWorldMatrix.dx(), newWorldMatrix.dy())/mWorldMatrix.m11());
	}
	else {
		mWorldMatrix = newWorldMatrix;
		mImgMatrix = newImgMatrix;
		updateImageMatrix();

		QPointF imgPos = QPointF(canvasSize.x()*mImgStorage.getImage().width(), canvasSize.y()*mImgStorage.getImage().height());

		// go to screen coordinates
		imgPos = mImgMatrix.map(imgPos);

		// go to world coordinates
		imgPos = mWorldMatrix.map(imgPos);

		// compute difference to current mViewport center - in world coordinates
		imgPos = QPointF(width()*0.5f, height()*0.5f) - imgPos;

		// back to screen coordinates
		float s = (float)mWorldMatrix.m11();
		mWorldMatrix.translate(imgPos.x()/s, imgPos.y()/s);
	}

	update();
}

void DkViewPort::tcpSetWindowRect(QRect rect) {
	this->setGeometry(rect);
}

void DkViewPort::tcpSynchronize(QTransform relativeMatrix) {
	
	if (!relativeMatrix.isIdentity())
		emit sendTransformSignal(relativeMatrix, QTransform(), QPointF());

	// check if we need a synchronization
	if ((qApp->keyboardModifiers() == mAltMod ||
		DkSettings::sync.syncMode != DkSettings::sync_mode_default || DkSettings::sync.syncActions) &&
		(hasFocus() || mController->hasFocus())) {
		QPointF size = QPointF(geometry().width()/2.0f, geometry().height()/2.0f);
		size = mWorldMatrix.inverted().map(size);
		size = mImgMatrix.inverted().map(size);
		size = QPointF(size.x()/(float)mImgStorage.getImage().width(), size.y()/(float)mImgStorage.getImage().height());

		emit sendTransformSignal(mWorldMatrix, mImgMatrix, size);
	}
}

void DkViewPort::tcpForceSynchronize() {

	int oldMode = DkSettings::sync.syncMode;
	DkSettings::sync.syncMode = DkSettings::sync_mode_remote_display;
	tcpSynchronize();
	DkSettings::sync.syncMode = oldMode;
}

void DkViewPort::tcpShowConnections(QList<DkPeer*> peers) {

	QString newPeers;

	for (int idx = 0; idx < peers.size(); idx++) {
		
		DkPeer* cp = peers.at(idx);

		if (cp->isSynchronized() && newPeers.isEmpty()) {
			newPeers = tr("connected with: ");
			emit newClientConnectedSignal(true, cp->isLocal());
		}
		else if (newPeers.isEmpty()) {
			newPeers = tr("disconnected with: ");
			emit newClientConnectedSignal(false, cp->isLocal());
		}

		qDebug() << "cp address..." << cp->hostAddress;

		newPeers.append("\n\t");

		if (!cp->clientName.isEmpty())
			newPeers.append(cp->clientName);
		if (!cp->clientName.isEmpty() && !cp->title.isEmpty())
			newPeers.append(": ");
		if (!cp->title.isEmpty())
			newPeers.append(cp->title);
	}

	mController->setInfo(newPeers);
	update();
}

void DkViewPort::paintEvent(QPaintEvent* event) {

	QPainter painter(viewport());

	if (mImgStorage.hasImage()) {
		painter.setWorldTransform(mWorldMatrix);

		// don't interpolate if we are forced to, at 100% or we exceed the maximal interpolation level
		if (!mForceFastRendering && // force?
			fabs(mImgMatrix.m11()*mWorldMatrix.m11()-1.0f) > FLT_EPSILON && // @100% ?
			mImgMatrix.m11()*mWorldMatrix.m11() <= (float)DkSettings::display.interpolateZoomLevel/100.0f) {	// > max zoom level
			painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);
		}

		if (mDissolveImage) {
			QImage imgQt = mImgStorage.getImage();
			DkImage::addToImage(imgQt, 255);
			mImgStorage.setImage(imgQt);
			qDebug() << "added to image...";
		}

		// TODO: if fading is active we interpolate with background instead of the other image
		draw(&painter, 1.0f-mFadeOpacity);

		if (/*mFadeTimer->isActive() && */!mFadeBuffer.isNull()) {
			float oldOp = (float)painter.opacity();
			painter.setOpacity(mFadeOpacity);
			painter.drawImage(mFadeImgViewRect, mFadeBuffer, mFadeBuffer.rect());
			painter.setOpacity(oldOp);
		}

		//Now disable matrixWorld for overlay display
		painter.setWorldMatrixEnabled(false);
	}
	else
		drawBackground(&painter);

	// this was the auto-show function of the zoom widget
	//DkZoomWidget* zw = mController->getZoomWidget();

	////in mode zoom/panning
	//if (worldMatrix.m11() > 1 && !imageInside() && 
	//	DkSettings::app.showOverview.testBit(DkSettings::app.currentAppMode)) {

	//	if (!zw->isVisible())
	//		zw->setVisible(true, true);
	//}
	//else if (zw->isVisible() && zw->isAutoHide())
	//	mController->getZoomWidget()->hide();

	painter.end();

	//qDebug() << "painting main widget...";

	// propagate
	QGraphicsView::paintEvent(event);

	// NOTE: never ever do this
	// here it is just for fun!
	if (mDissolveImage)
		update();
}

// drawing functions --------------------------------------------------------------------
void DkViewPort::drawBackground(QPainter *painter) {
	
	painter->setRenderHint(QPainter::SmoothPixmapTransform);

	// fit to mViewport
	QSize s = mImgBg.size();
	if (s.width() > (float)(size().width()*0.5))
		s = s*((size().width()*0.5)/s.width());

	if (s.height() > size().height()*0.6)
		s = s*((size().height()*0.6)/s.height());

	QRect bgRect(QPoint(qRound(size().width()-s.width()-size().width()*0.05), qRound(size().height()-s.height()-size().height()*0.05)), s);

	painter->drawImage(bgRect, mImgBg, QRect(QPoint(), mImgBg.size()));
}

void DkViewPort::loadMovie() {

	if (!mLoader)
		return;

	if (mMovie)
		mMovie->stop();

	mMovie = QSharedPointer<QMovie>(new QMovie(mLoader->filePath()));
	connect(mMovie.data(), SIGNAL(frameChanged(int)), this, SLOT(update()));
	mMovie->start();

	emit movieLoadedSignal(true);
}

void DkViewPort::pauseMovie(bool pause) {

	if (!mMovie)
		return;

	mMovie->setPaused(pause);
}

void DkViewPort::nextMovieFrame() {

	if (!mMovie)
		return;

	mMovie->jumpToNextFrame();
	update();
}

void DkViewPort::previousMovieFrame() {

	if (!mMovie)
		return;

	
	int fn = mMovie->currentFrameNumber()-1;
	if (fn == -1)
		fn = mMovie->frameCount()-1;
	//qDebug() << "retrieving frame: " << fn;
	
	while(mMovie->currentFrameNumber() != fn)
		mMovie->jumpToNextFrame();

	//// the subsequent thing is not working if the movie is paused
	//bool success = movie->jumpToFrame(movie->currentFrameNumber()-1);
	update();
}

void DkViewPort::stopMovie() {

	if (!mMovie)
		return;		
	
	mMovie->stop();
	mMovie = QSharedPointer<QMovie>();
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

	mViewportRect = QRect(0, 0, width(), height());

	// >DIR: diem - bug if zoom factor is large and window becomes small
	updateImageMatrix();
	centerImage();
	changeCursor();

	mController->getOverview()->setViewPortRect(geometry());
	
	mController->resize(width(), height());
	qDebug() << "mController geometry: " << mController->geometry();

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
		event->type() == QEvent::KeyRelease || 
		event->type() == QEvent::DragEnter ||

		event->type() == QEvent::Drop) {

		//qDebug() << "redirecting event...";
		// mouse events that double are now fixed, since the mViewport is now overlayed by the mController
		return QWidget::event(event);
	}
	else
		return DkBaseViewPort::event(event);
	
}

void DkViewPort::dragLeaveEvent(QDragLeaveEvent *event) {

	qDebug() << "";
	event->accept();	
}

void DkViewPort::mousePressEvent(QMouseEvent *event) {

	// if zoom on wheel, the additional keys should be used for switching files
	if (DkSettings::global.zoomOnWheel) {
		if(event->buttons() == Qt::XButton1)
			loadPrevFileFast();
		else if(event->buttons() == Qt::XButton2)
			loadNextFileFast();
	} 
	else if(event->buttons() == Qt::XButton1 || event->buttons() == Qt::XButton2) {
		repeatZoom();
		mRepeatZoomTimer->start();
	}
	
	// ok, start panning
	if (mWorldMatrix.m11() > 1 && !imageInside() && event->buttons() == Qt::LeftButton) {
		setCursor(Qt::ClosedHandCursor);
		mPosGrab = event->pos();
	}
	
	// keep in mind if the gesture was started in the mViewport
	// this fixes issues if some HUD widgets or child widgets
	// do not implement mouse events correctly
	if (event->buttons() == Qt::LeftButton)
		mGestureStarted = true;
	else
		mGestureStarted = false;

	// should be sent to QWidget?!
	DkBaseViewPort::mousePressEvent(event);
}

void DkViewPort::mouseReleaseEvent(QMouseEvent *event) {
	
	mRepeatZoomTimer->stop();

	int sa = swipeRecognition(event->pos(), mPosGrab.toPoint());
	QPoint pos = mapToImage(event->pos());

	if (imageInside() && mGestureStarted) {
		swipeAction(sa);
	}

	// needed for scientific projects...
	if (pos.x() != -1 && pos.y() != -1)
		emit mouseClickSignal(event, pos);

	mGestureStarted = false;

	DkBaseViewPort::mouseReleaseEvent(event);
}

void DkViewPort::mouseMoveEvent(QMouseEvent *event) {

	//qDebug() << "mouse move (DkViewPort)";
	//changeCursor();
	mCurrentPixelPos = event->pos();

	if (mVisibleStatusbar)
		getPixelInfo(event->pos());

	if (mWorldMatrix.m11() > 1 && event->buttons() == Qt::LeftButton) {

		QPointF cPos = event->pos();
		QPointF dxy = (cPos - mPosGrab);
		mPosGrab = cPos;
		moveView(dxy/mWorldMatrix.m11());

		// with shift also a hotkey for fast switching...
		if ((DkSettings::sync.syncAbsoluteTransform &&
			event->modifiers() == (mAltMod | Qt::ShiftModifier)) || 
			(!DkSettings::sync.syncAbsoluteTransform &&
			event->modifiers() == (mAltMod))) {
			
			if (dxy.x() != 0 || dxy.y() != 0) {
				QTransform relTransform;
				relTransform.translate(dxy.x(), dxy.y());
				tcpSynchronize(relTransform);
			}
		}
		tcpSynchronize();
	}

	int dist = QPoint(event->pos()-mPosGrab.toPoint()).manhattanLength();
	
	if (event->buttons() == Qt::LeftButton 
		&& dist > QApplication::startDragDistance()
		&& imageInside()
		&& !getImage().isNull()
		&& mLoader
		&& !QApplication::widgetAt(event->globalPos())) {	// is NULL if the mouse leaves the window

			qDebug() << mLoader->filePath();

			// TODO: check if we do it correct (network locations that are not mounted)
			QUrl fileUrl = QUrl::fromLocalFile(mLoader->filePath());

			QList<QUrl> urls;
			urls.append(fileUrl);

			// who deletes me?
			QMimeData* mimeData = new QMimeData();

			if (QFileInfo(mLoader->filePath()).exists() && !mLoader->isEdited())
				mimeData->setUrls(urls);
			else if (!getImage().isNull())
				mimeData->setImageData(getImage());

			QDrag* drag = new QDrag(this);
			drag->setMimeData(mimeData);
			drag->exec(Qt::CopyAction);
			qDebug() << "creating drag: " << fileUrl;
	}

	// send to parent
	DkBaseViewPort::mouseMoveEvent(event);
}

void DkViewPort::wheelEvent(QWheelEvent *event) {

	//if (event->modifiers() & ctrlMod)
	//	qDebug() << "CTRL modifier";
	//if (event->modifiers() & altMod)
	//	qDebug() << "ALT modifier";

	if ((!DkSettings::global.zoomOnWheel && event->modifiers() != mCtrlMod) || 
		(DkSettings::global.zoomOnWheel && (event->modifiers() & mCtrlMod || (event->orientation() == Qt::Horizontal && !(event->modifiers() & mAltMod))))) {

		if (event->delta() < 0)
			loadNextFileFast();
		else
			loadPrevFileFast();
	}
	else 
		DkBaseViewPort::wheelEvent(event);

	tcpSynchronize();

}

#if QT_VERSION < 0x050000
#ifndef QT_NO_GESTURES
int DkViewPort::swipeRecognition(QNativeGestureEvent* event) {
	

	if (mPosGrab.isNull()) {
		mPosGrab = event->position;
		return no_swipe;
	}

	return swipeRecognition(event->position, mPosGrab.toPoint());
}
#endif
#endif

int DkViewPort::swipeRecognition(QPoint start, QPoint end) {

	DkVector vec((float)(start.x()-end.x()), (float)(start.y()-end.y()));

	if (fabs(vec.norm()) < 100) {
		qDebug() << "ignoring, too small: " << vec.norm();
		return no_swipe;
	}

	double angle = DkMath::normAngleRad(vec.angle(DkVector(0,1)), 0.0, CV_PI);
	bool horizontal = false;

	if (angle > CV_PI*0.3 && angle < CV_PI*0.6)
		horizontal = true;
	else if (angle < 0.2*CV_PI || angle > 0.8*CV_PI)
		horizontal = false;
	else
		return no_swipe;	// angles ~45° are not accepted

	QPoint startPos = QWidget::mapFromGlobal(end);
	qDebug() << "vec: " << vec.x << ", " << vec.y;

	if (horizontal) {

		if (vec.x < 0)
			return next_image;
		else
			return prev_image;

	}
	// upper part of the canvas is thumbs
	else if (!horizontal && startPos.y() < height()*0.5f) {

		// downward gesture is opening
		if (vec.y > 0)
			return open_thumbs;
		else
			return close_thumbs;
	}
	// lower part of the canvas is thumbs
	else if (!horizontal && startPos.y() > height()*0.5f) {

		// upward gesture is opening
		if (vec.y < 0)
			return open_metadata;
		else
			return close_metadata;
	}

	return no_swipe;
}

void DkViewPort::swipeAction(int swipeGesture) {

	switch (swipeGesture) {
	case next_image:
		loadNextFileFast();
		break;
	case prev_image:
		loadPrevFileFast();
		break;
	case open_thumbs:
		mController->showPreview(true);
		break;
	case close_thumbs:
		mController->showPreview(false);
		break;
	case open_metadata:
		mController->showMetaData(true);
		break;
	case close_metadata:
		mController->showMetaData(false);
		break;
	default:
		break;
	}

}

void DkViewPort::setFullScreen(bool fullScreen) {

	mController->setFullScreen(fullScreen);
	toggleLena();
}

QPoint DkViewPort::mapToImage(const QPoint& windowPos) const {

	QPointF imgPos = mWorldMatrix.inverted().map(QPointF(windowPos));
	imgPos = mImgMatrix.inverted().map(imgPos);

	QPoint xy(qFloor(imgPos.x()), qFloor(imgPos.y()));

	if (xy.x() < 0 || xy.y() < 0 || xy.x() >= mImgStorage.getImageConst().width() || xy.y() >= mImgStorage.getImageConst().height())
		return QPoint(-1,-1);

	return xy;
}

void DkViewPort::getPixelInfo(const QPoint& pos) {

	if (mImgStorage.getImage().isNull())
		return;

	QPoint xy = mapToImage(pos);

	if (xy.x() == -1 || xy.y() == -1)
		return;

	QColor col = mImgStorage.getImage().pixel(xy);
	
	QString msg = "<font color=#555555>x: " + QString::number(xy.x()) + " y: " + QString::number(xy.y()) + "</font>"
		" | r: " + QString::number(col.red()) + " g: " + QString::number(col.green()) + " b: " + QString::number(col.blue());

	if (mImgStorage.getImage().hasAlphaChannel())
		msg += " a: " + QString::number(col.alpha());

	msg += " | <font color=#555555>" + col.name().toUpper() + "</font>";

	emit statusInfoSignal(msg, 0);// status_pixel_info); TODO: fix include
}

QString DkViewPort::getCurrentPixelHexValue() {

	if (mImgStorage.getImage().isNull() || mCurrentPixelPos.isNull())
		return QString();

	QPointF imgPos = mWorldMatrix.inverted().map(QPointF(mCurrentPixelPos));
	imgPos = mImgMatrix.inverted().map(imgPos);

	QPoint xy(qFloor(imgPos.x()), qFloor(imgPos.y()));

	if (xy.x() < 0 || xy.y() < 0 || xy.x() >= mImgStorage.getImage().width() || xy.y() >= mImgStorage.getImage().height())
		return QString();

	QColor col = mImgStorage.getImage().pixel(xy);
	
	return col.name().toUpper().remove(0,1);
}

// Copy & Paste --------------------------------------------------------
void DkViewPort::copyPixelColorValue() {

	if (getImage().isNull())
		return;

	QMimeData* mimeData = new QMimeData;

	if (!getImage().isNull())
		mimeData->setText(getCurrentPixelHexValue());

	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setMimeData(mimeData);
}

void DkViewPort::copyImage() {

	qDebug() << "copying...";

	if (getImage().isNull() || !mLoader)
		return;

	QUrl fileUrl = QUrl("file:///" + mLoader->filePath());

	QList<QUrl> urls;
	urls.append(fileUrl);

	QMimeData* mimeData = new QMimeData;

	if (QFileInfo(mLoader->filePath()).exists() && !mLoader->isEdited())
		mimeData->setUrls(urls);
	else if (!getImage().isNull())
		mimeData->setImageData(getImage());

	mimeData->setText(mLoader->filePath());

	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setMimeData(mimeData);

	qDebug() << "copying: " << fileUrl;
}

void DkViewPort::copyImageBuffer() {

	qDebug() << "copying...";

	if (getImage().isNull())
		return;

	QMimeData* mimeData = new QMimeData;

	if (!getImage().isNull())
		mimeData->setImageData(getImage());

	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setMimeData(mimeData);
}

void DkViewPort::animateFade() {

	mFadeOpacity = 1.0f-(float)mFadeTime.getTotalTime()/DkSettings::display.fadeSec;
	
	if (mFadeOpacity <= 0) {
		mFadeBuffer = QImage();
		mFadeTimer->stop();
		mFadeOpacity = 0;
	}

	qDebug() << "new opacity: " << mFadeOpacity;

	update();
}

void DkViewPort::togglePattern(bool show) {

	mController->setInfo((show) ? tr("Transparency Pattern Enabled") : tr("Transparency Pattern Disabled"));

	DkBaseViewPort::togglePattern(show);
}

// edit image --------------------------------------------------------------------
void DkViewPort::rotateCW() {

	applyPluginChanges();

	if (mLoader != 0)
		mLoader->rotateImage(90);

}

void DkViewPort::rotateCCW() {

	applyPluginChanges();

	if (mLoader != 0)
		mLoader->rotateImage(-90);

}

void DkViewPort::rotate180() {

	applyPluginChanges();

	if (mLoader != 0)
		mLoader->rotateImage(180);

}

// file handling --------------------------------------------------------------------
void DkViewPort::loadLena() {

	bool ok;
	QString text = QInputDialog::getText(this, tr("Lena"), tr("A remarkable woman"), QLineEdit::Normal, 0, &ok);

	// pass phrase
	if (ok && !text.isEmpty() && text == "lena") {
		mTestLoaded = true;
		toggleLena();
	}
	else if (!ok) {
		QMessageBox warningDialog(QApplication::activeWindow());
		warningDialog.setIcon(QMessageBox::Warning);
		warningDialog.setText(tr("you cannot cancel this"));
		warningDialog.exec();
		loadLena();
	}
	else {
		QApplication::beep();
		
		if (text.isEmpty())
			mController->setInfo(tr("did you understand the brainteaser?"));
		else
			mController->setInfo(tr("%1 is wrong...").arg(text));
	}
}

void DkViewPort::toggleDissolve() {

	qDebug() << "dissolving: " << mDissolveImage;
	mDissolveImage = !mDissolveImage;

	update();
}

void DkViewPort::toggleLena() {

	if (!mTestLoaded)
		return;

	if (mLoader) {
		if (parentWidget() && parentWidget()->isFullScreen())
			mLoader->load(":/nomacs/img/lena-full.jpg");
		else
			mLoader->load(":/nomacs/img/lena.jpg");
	}
}

void DkViewPort::settingsChanged() {

	reloadFile();

	mAltMod = DkSettings::global.altMod;
	mCtrlMod = DkSettings::global.ctrlMod;

	mController->settingsChanged();
}

void DkViewPort::setEditedImage(QImage newImg) {

	if (newImg.isNull()) {
		mController->setInfo(tr("Attempted to set NULL image"));	// not sure if users understand that
		return;
	}

	QSharedPointer<DkImageContainerT> imgC = mLoader->getCurrentImage();
	imgC->setImage(newImg);
	unloadImage(false);
	mLoader->setImage(imgC);
	qDebug() << "mLoader gets this size: " << newImg.size();

	// TODO: contrast mViewport does not add * 

	// TODO: add functions such as save file on unload
}

void DkViewPort::setEditedImage(QSharedPointer<DkImageContainerT> img) {

	if (!img) {
		mController->setInfo(tr("Attempted to set NULL image"));	// not sure if users understand that
		return;
	}

	unloadImage(false);
	mLoader->setImage(img);
}

void DkViewPort::applyPluginChanges() {

	// TODO: that's dangerous! - remove -> pluginManager is singelton soon anyway
	//DkNoMacs* noMacs = dynamic_cast<DkNoMacs*>(parentWidget());

	//if (!noMacs) 
	//	return;

	//if(!noMacs->getCurrRunningPlugin().isEmpty()) 
	//	noMacs->applyPluginChanges(true, false);
}

bool DkViewPort::unloadImage(bool fileChange) {

	// TODO: we have to check here - why loading is not stopped by applyPluginChanges()
	/*if (!pluginImageWasApplied)*/ applyPluginChanges(); //prevent recursion
	
	if (DkSettings::display.fadeSec && (mController->getPlayer()->isPlaying() || parentWidget() && parentWidget()->isFullScreen())) {
		mFadeBuffer = mImgStorage.getImage((float)(mImgMatrix.m11()*mWorldMatrix.m11()));
		mFadeImgViewRect = mImgViewRect;
		mFadeImgRect = mImgRect;
		mFadeOpacity = 1.0f;
	}

	int success = true;
	if (fileChange)
		success = mLoader->unloadFile();		// returns false if the user cancels
	
	if (mMovie && success) {
		mMovie->stop();
		mMovie = QSharedPointer<QMovie>();
	}

	return success != 0;
}

void DkViewPort::deactivate() {
	
	setImage(QImage());
}

void DkViewPort::loadFile(const QString& filePath) {

	if (!unloadImage())
		return;

	mTestLoaded = false;

	if (mLoader && !filePath.isEmpty() && QFileInfo(filePath).isDir()) {
		mLoader->setDir(filePath);
	} 
	else if (mLoader)
		mLoader->load(filePath);

	qDebug() << "sync mode: " << (DkSettings::sync.syncMode == DkSettings::sync_mode_remote_display);
	if ((qApp->keyboardModifiers() == mAltMod || DkSettings::sync.syncMode == DkSettings::sync_mode_remote_display) && (hasFocus() || mController->hasFocus()) && mLoader->hasFile())
		tcpLoadFile(0, filePath);
}

void DkViewPort::reloadFile() {

	if (mLoader) {

		if (unloadImage())
			mLoader->reloadImage();
	}
}

void DkViewPort::loadFile(int skipIdx) {

	if (!unloadImage())
		return;

	if (mLoader && !mTestLoaded)
		mLoader->changeFile(skipIdx);

	// alt mod
	if ((qApp->keyboardModifiers() == mAltMod || DkSettings::sync.syncMode == DkSettings::sync_mode_remote_display || DkSettings::sync.syncActions) && (hasFocus() || mController->hasFocus())) {
		emit sendNewFileSignal((qint16)skipIdx);
		qDebug() << "emitting load next";
	}
}

void DkViewPort::loadPrevFileFast() {

	loadFileFast(-1);
}

void DkViewPort::loadNextFileFast() {

	loadFileFast(1);
}


void DkViewPort::loadFileFast(int skipIdx) {

	if (!unloadImage())
		return;

	if (!((qApp->keyboardModifiers() == mAltMod || DkSettings::sync.syncActions) &&
		DkSettings::sync.syncMode == DkSettings::sync_mode_remote_control)) {
		QApplication::sendPostedEvents();

		int sIdx = skipIdx;
		QSharedPointer<DkImageContainerT> lastImg;

		for (int idx = 0; idx < mLoader->getImages().size(); idx++) {

			QSharedPointer<DkImageContainerT> imgC = mLoader->getSkippedImage(sIdx);

			if (!imgC)
				break;

			mLoader->setCurrentImage(imgC);

			if (imgC && imgC->getLoadState() != DkImageContainer::exists_not) {
				mLoader->load(imgC);
				break;
			}
			else if (lastImg == imgC) {
				sIdx += skipIdx;	// get me out of endless loops (self referencing shortcuts)
			}
			else {
				qDebug() << "image does not exist - skipping";
			}

			lastImg = imgC;
		}
	}	

	if (((qApp->keyboardModifiers() == mAltMod || DkSettings::sync.syncActions) &&
		DkSettings::sync.syncMode != DkSettings::sync_mode_remote_display) && 
		(hasFocus() || 
		mController->hasFocus())) {
		emit sendNewFileSignal((qint16)skipIdx);
		QCoreApplication::sendPostedEvents();
	}

	//skipImageTimer->start(50);	// load full image in 50 ms if there is not a fast load again
}

void DkViewPort::loadFirst() {

	if (!unloadImage())
		return;

	if (mLoader && !mTestLoaded)
		mLoader->firstFile();

	if ((qApp->keyboardModifiers() == mAltMod || DkSettings::sync.syncMode == DkSettings::sync_mode_remote_display || DkSettings::sync.syncActions) && (hasFocus() || mController->hasFocus()))
		emit sendNewFileSignal(SHRT_MIN);
}

void DkViewPort::loadLast() {

	if (!unloadImage())
		return;

	if (mLoader && !mTestLoaded)
		mLoader->lastFile();

	if ((qApp->keyboardModifiers() == mAltMod || DkSettings::sync.syncMode == DkSettings::sync_mode_remote_display || DkSettings::sync.syncActions) && (hasFocus() || mController->hasFocus()))
		emit sendNewFileSignal(SHRT_MAX);

}

void DkViewPort::loadSkipPrev10() {

	loadFileFast(-DkSettings::global.skipImgs);
	//unloadImage();

	//if (mLoader && !testLoaded)
	//	mLoader->changeFile(-DkSettings::global.skipImgs, (parent->isFullScreen() && DkSettings::slideShow.silentFullscreen));

	if (qApp->keyboardModifiers() == mAltMod && (hasFocus() || mController->hasFocus()))
		emit sendNewFileSignal((qint16)-DkSettings::global.skipImgs);
}

void DkViewPort::loadSkipNext10() {

	loadFileFast(DkSettings::global.skipImgs);
	//unloadImage();

	//if (mLoader && !testLoaded)
	//	mLoader->changeFile(DkSettings::global.skipImgs, (parent->isFullScreen() && DkSettings::slideShow.silentFullscreen));

	if (qApp->keyboardModifiers() == mAltMod && (hasFocus() || mController->hasFocus()))
		emit sendNewFileSignal((qint16)DkSettings::global.skipImgs);
}

void DkViewPort::tcpLoadFile(qint16 idx, QString filename) {

	qDebug() << "I got a file request??";

	// some hack: set the mode to default in order to prevent loops (if both are auto connected)
	// should be mostly harmless
	//int oldMode = DkSettings::sync.syncMode;
	//DkSettings::sync.syncMode = DkSettings::sync_mode_receiveing_command;

	if (filename.isEmpty()) {

		// change the file idx according to my brother
		switch (idx) {
			case SHRT_MIN:
				loadFirst();
				break;
			case SHRT_MAX:
				loadLast();
				break;
			//case 1:
			//	loadNextFileFast();
			//	break;
			//case -1:
			//	loadPrevFileFast();
			//	break;
			default:
				loadFileFast(idx);
				//if (mLoader) mLoader->loadFileAt(idx);
		}
	}
	else 
		loadFile(filename);

	qDebug() << "loading file: " << filename;

	//DkSettings::sync.syncMode = oldMode;
}

//DkImageLoader* DkViewPort::getImageLoader() {
//
//	return mLoader;
//}

void DkViewPort::setImageLoader(QSharedPointer<DkImageLoader> newLoader) {
	
	mLoader = newLoader;
	connectLoader(newLoader);

	if (mLoader)
		mLoader->activate();
}

void DkViewPort::connectLoader(QSharedPointer<DkImageLoader> loader, bool connectSignals) {

	if (!loader)
		return;

	if (connectSignals) {
		//connect(mLoader.data(), SIGNAL(imageLoadedSignal(QSharedPointer<DkImageContainerT>, bool)), this, SLOT(updateImage(QSharedPointer<DkImageContainerT>, bool)), Qt::UniqueConnection);
		connect(loader.data(), SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), this, SLOT(updateImage(QSharedPointer<DkImageContainerT>)), Qt::UniqueConnection);

		connect(loader.data(), SIGNAL(updateDirSignal(QVector<QSharedPointer<DkImageContainerT> >)), mController->getFilePreview(), SLOT(updateThumbs(QVector<QSharedPointer<DkImageContainerT> >)), Qt::UniqueConnection);
		connect(loader.data(), SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), mController->getFilePreview(), SLOT(setFileInfo(QSharedPointer<DkImageContainerT>)), Qt::UniqueConnection);
		connect(loader.data(), SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), mController->getMetaDataWidget(), SLOT(updateMetaData(QSharedPointer<DkImageContainerT>)), Qt::UniqueConnection);
		connect(loader.data(), SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), mController, SLOT(setFileInfo(QSharedPointer<DkImageContainerT>)), Qt::UniqueConnection);

		connect(loader.data(), SIGNAL(showInfoSignal(QString, int, int)), mController, SLOT(setInfo(QString, int, int)), Qt::UniqueConnection);
		connect(loader.data(), SIGNAL(updateInfoSignalDelayed(QString, bool, int)), mController, SLOT(setInfoDelayed(QString, bool, int)), Qt::UniqueConnection);
		connect(loader.data(), SIGNAL(updateSpinnerSignalDelayed(bool, int)), mController, SLOT(setSpinnerDelayed(bool, int)), Qt::UniqueConnection);

		connect(loader.data(), SIGNAL(setPlayer(bool)), mController->getPlayer(), SLOT(play(bool)), Qt::UniqueConnection);

		connect(loader.data(), SIGNAL(updateDirSignal(QVector<QSharedPointer<DkImageContainerT> >)), mController->getScroller(), SLOT(updateDir(QVector<QSharedPointer<DkImageContainerT> >)), Qt::UniqueConnection);
		connect(loader.data(), SIGNAL(imageUpdatedSignal(int)), mController->getScroller(), SLOT(updateFile(int)), Qt::UniqueConnection);
		connect(mController->getScroller(), SIGNAL(valueChanged(int)), loader.data(), SLOT(loadFileAt(int)));

		// not sure if this is elegant?!
		connect(mShortcuts[sc_delete_silent], SIGNAL(activated()), loader.data(), SLOT(deleteFile()), Qt::UniqueConnection);
	}
	else {
		//connect(mLoader.data(), SIGNAL(imageLoadedSignal(QSharedPointer<DkImageContainerT>, bool)), this, SLOT(updateImage(QSharedPointer<DkImageContainerT>, bool)), Qt::UniqueConnection);
		disconnect(loader.data(), SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), this, SLOT(updateImage(QSharedPointer<DkImageContainerT>)));

		disconnect(loader.data(), SIGNAL(updateDirSignal(QVector<QSharedPointer<DkImageContainerT> >)), mController->getFilePreview(), SLOT(updateThumbs(QVector<QSharedPointer<DkImageContainerT> >)));
		disconnect(loader.data(), SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), mController->getFilePreview(), SLOT(setFileInfo(QSharedPointer<DkImageContainerT>)));
		disconnect(loader.data(), SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), mController->getMetaDataWidget(), SLOT(updateMetaData(QSharedPointer<DkImageContainerT>)));
		disconnect(loader.data(), SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), mController, SLOT(setFileInfo(QSharedPointer<DkImageContainerT>)));

		disconnect(loader.data(), SIGNAL(showInfoSignal(QString, int, int)), mController, SLOT(setInfo(QString, int, int)));
		disconnect(loader.data(), SIGNAL(updateInfoSignalDelayed(QString, bool, int)), mController, SLOT(setInfoDelayed(QString, bool, int)));
		disconnect(loader.data(), SIGNAL(updateSpinnerSignalDelayed(bool, int)), mController, SLOT(setSpinnerDelayed(bool, int)));

		disconnect(loader.data(), SIGNAL(setPlayer(bool)), mController->getPlayer(), SLOT(play(bool)));

		disconnect(loader.data(), SIGNAL(updateDirSignal(QVector<QSharedPointer<DkImageContainerT> >)), mController->getScroller(), SLOT(updateDir(QVector<QSharedPointer<DkImageContainerT> >)));
		disconnect(loader.data(), SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), mController->getScroller(), SLOT(updateFile(QSharedPointer<DkImageContainerT>)));
		
		// not sure if this is elegant?!
		disconnect(mShortcuts[sc_delete_silent], SIGNAL(activated()), loader.data(), SLOT(deleteFile()));
	}
}


DkControlWidget* DkViewPort::getController() {
	
	return mController;
}

void DkViewPort::cropImage(const DkRotatingRect& rect, const QColor& bgCol) {

	QTransform tForm; 
	QPointF cImgSize;

	rect.getTransform(tForm, cImgSize);

	if (cImgSize.x() < 0.5f || cImgSize.y() < 0.5f) {
		mController->setInfo(tr("I cannot crop an image that has 0 px, sorry."));
		return;
	}

	qDebug() << cImgSize;

	double angle = DkMath::normAngleRad(rect.getAngle(), 0, CV_PI*0.5);
	double minD = qMin(abs(angle), abs(angle-CV_PI*0.5));

	QImage img = QImage(qRound(cImgSize.x()), qRound(cImgSize.y()), QImage::Format_ARGB32);
	img.fill(bgCol.rgba());

	// render the image into the new coordinate system
	QPainter painter(&img);
	painter.setWorldTransform(tForm);
	
	// for rotated rects we want perfect anti-aliasing
	if (minD > FLT_EPSILON)
		painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);
	
	painter.drawImage(QRect(QPoint(), getImage().size()), getImage(), QRect(QPoint(), getImage().size()));
	painter.end();

	QSharedPointer<DkImageContainerT> imgC = mLoader->getCurrentImage();
	imgC->setImage(img);
	setEditedImage(imgC);
	
	//imgQt = img;
	//update();

	qDebug() << "cropping...";
}

// DkViewPortFrameless --------------------------------------------------------------------
DkViewPortFrameless::DkViewPortFrameless(QWidget *parent, Qt::WindowFlags flags) : DkViewPort(parent, flags) {
	
#ifdef Q_WS_MAC
	parent->setAttribute(Qt::WA_MacNoShadow);
#endif

	setAttribute(Qt::WA_TranslucentBackground, true);
	mImgBg.load(":/nomacs/img/splash-screen.png");

	mMainScreen = geometry();

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

	mStartActions.append(startAction);
	mStartIcons.append(startIcon);
}

void DkViewPortFrameless::setImage(QImage newImg) {

	DkViewPort::setImage(newImg);
}

void DkViewPortFrameless::zoom(float factor, QPointF center) {

	if (!mImgStorage.hasImage() || mBlockZooming)
		return;

	//limit zoom out ---
	if (mWorldMatrix.m11()*factor <= mMinZoom && factor < 1)
		return;

	//if (worldMatrix.m11()*factor < 1) {
	//	resetView();
	//	return;
	//}

	// reset view & block if we pass the 'image fit to screen' on zoom out
	if (mWorldMatrix.m11() > 1 && mWorldMatrix.m11()*factor < 1) {

		mBlockZooming = true;
		mZoomTimer->start(500);
		//resetView();
		//return;
	}

	//limit zoom in ---
	if (mWorldMatrix.m11()*mImgMatrix.m11() > mMaxZoom && factor > 1)
		return;

	QRectF viewRect = mWorldMatrix.mapRect(mImgViewRect);

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
	mWorldMatrix.inverted().map(qRound(center.x()), qRound(center.y()), &a, &b);

	mWorldMatrix.translate(a-factor*a, b-factor*b);
	mWorldMatrix.scale(factor, factor);

	controlImagePosition();
	showZoom();
	changeCursor();

	update();

	tcpSynchronize();
	emit zoomSignal((float)(mWorldMatrix.m11()*mImgMatrix.m11()*100));
}

void DkViewPortFrameless::resetView() {

	// maybe we can delete this function...
	DkViewPort::resetView();
}

void DkViewPortFrameless::paintEvent(QPaintEvent* event) {

	if (parentWidget() && !parentWidget()->isFullScreen()) {

		QPainter painter(viewport());
		painter.setWorldTransform(mWorldMatrix);
		drawFrame(&painter);
		painter.end();
	}

	DkViewPort::paintEvent(event);
}

void DkViewPortFrameless::draw(QPainter *painter, float) {
	
	if (parentWidget() && parentWidget()->isFullScreen()) {
		QColor col = QColor(0,0,0);
		col.setAlpha(150);
		painter->setWorldMatrixEnabled(false);
		painter->fillRect(QRect(QPoint(), size()), col);
		painter->setWorldMatrixEnabled(true);
	}

	if (!mMovie || !mMovie->isValid()) {
		QImage imgQt = mImgStorage.getImage((float)(mImgMatrix.m11()*mWorldMatrix.m11()));

		if (DkSettings::display.tpPattern && imgQt.hasAlphaChannel()) {

			// don't scale the pattern...
			QTransform scaleIv;
			scaleIv.scale(mWorldMatrix.m11(), mWorldMatrix.m22());
			mPattern.setTransform(scaleIv.inverted());

			painter->setPen(QPen(Qt::NoPen));	// no border
			painter->setBrush(mPattern);
			painter->drawRect(mImgViewRect);
		}

		painter->drawImage(mImgViewRect, imgQt, QRect(QPoint(), imgQt.size()));
	}
	else {
		painter->drawPixmap(mImgViewRect, mMovie->currentPixmap(), mMovie->frameRect());
	}

}

void DkViewPortFrameless::drawBackground(QPainter *painter) {
	
	painter->setWorldTransform(mImgMatrix);
	painter->setRenderHint(QPainter::SmoothPixmapTransform);
	painter->setBrush(QColor(127, 144, 144, 200));
	painter->setPen(QColor(100, 100, 100, 255));

	QRectF initialRect = mMainScreen;
	QPointF oldCenter = initialRect.center();

	QTransform cT;
	cT.scale(400/initialRect.width(), 400/initialRect.width());
	initialRect = cT.mapRect(initialRect);
	initialRect.moveCenter(oldCenter);

	// fit to mViewport
	QSize s = mImgBg.size();

	QRectF bgRect(QPoint(), s);
	bgRect.moveCenter(initialRect.center());//moveTopLeft(QPointF(size().width(), size().height())*0.5 - initialRect.bottomRight()*0.5);

	//painter->drawRect(initialRect);
	painter->drawImage(bgRect, mImgBg, QRect(QPoint(), mImgBg.size()));

	if (mStartActions.isEmpty())
		return;

	// first time?
	if (mStartActionsRects.isEmpty()) {
		float margin = 40;
		float iconSizeMargin = (float)((initialRect.width()-3*margin)/mStartActions.size());
		QSize iconSize = QSize(qRound(iconSizeMargin - margin), qRound(iconSizeMargin - margin));
		QPointF offset = QPointF(bgRect.left() + 50, initialRect.center().y()+iconSizeMargin*0.25f);

		for (int idx = 0; idx < mStartActions.size(); idx++) {

			QRectF iconRect = QRectF(offset, iconSize);
			QPixmap ci = mStartIcons[idx] ? mStartIcons[idx]->pixmap(iconSize) : mStartActions[idx]->icon().pixmap(iconSize);
			mStartActionsRects.push_back(iconRect);
			mStartActionsIcons.push_back(ci);

			offset.setX(offset.x() + margin + iconSize.width());
		}
	}

	// draw start actions
	for (int idx = 0; idx < mStartActions.size(); idx++) {
		
		if (mStartIcons[idx])
			painter->drawPixmap(mStartActionsRects[idx], mStartActionsIcons[idx], QRect(QPoint(), mStartActionsIcons[idx].size()));
		else
			painter->drawPixmap(mStartActionsRects[idx], mStartActionsIcons[idx], QRect(QPoint(), mStartActionsIcons[idx].size()));
		
		QRectF tmpRect = mStartActionsRects[idx];
		QString text = mStartActions[idx]->text().replace("&", "");
		tmpRect.moveTop(tmpRect.bottom()+10);
		painter->drawText(tmpRect, text);
	}

	QString infoText = tr("Press F10 to exit Frameless view");
	QRectF tmpRect(bgRect.left()+50, bgRect.bottom()-60, bgRect.width()-100, 20);
	painter->drawText(tmpRect, infoText);
}

void DkViewPortFrameless::drawFrame(QPainter* painter) {

	// TODO: replace hasAlphaChannel with has alphaBorder
	if (mImgStorage.hasImage() && mImgStorage.getImage().hasAlphaChannel() || !DkSettings::display.showBorder)
		return;

	painter->setBrush(QColor(255, 255, 255, 200));
	painter->setPen(QColor(100, 100, 100, 255));

	QRectF frameRect;

	float fs = qMin((float)mImgViewRect.width(), (float)mImgViewRect.height())*0.1f;

	// looks pretty bad if the frame is too small
	if (fs < 4)
		return;

	frameRect = mImgViewRect;
	frameRect.setSize(frameRect.size() + QSize(qRound(fs), qRound(fs)));
	frameRect.moveCenter(mImgViewRect.center());

	painter->drawRect(frameRect);
}

void DkViewPortFrameless::mousePressEvent(QMouseEvent *event) {
	
	// move the window - todo: NOT full screen, window inside...
	setCursor(Qt::ClosedHandCursor);
	mPosGrab = event->pos();

	DkViewPort::mousePressEvent(event);
}

void DkViewPortFrameless::mouseReleaseEvent(QMouseEvent *event) {
	
	if (!mImgStorage.hasImage()) {

		qDebug() << "mouse released";
		QPointF pos = mImgMatrix.inverted().map(event->pos());

		for (int idx = 0; idx < mStartActionsRects.size(); idx++) {

			if (mStartActionsRects[idx].contains(pos)) {
				qDebug() << "toggle..." << idx;
				
				mStartActions[idx]->trigger();
				break;
			}
		}
	}

	unsetCursor();
	//setCursor(Qt::OpenHandCursor);
	DkViewPort::mouseReleaseEvent(event);
}


void DkViewPortFrameless::mouseMoveEvent(QMouseEvent *event) {
	
	if (!mImgStorage.hasImage()) {

		QPointF pos = mImgMatrix.inverted().map(event->pos());

		int idx;
		for (idx = 0; idx < mStartActionsRects.size(); idx++) {

			if (mStartActionsRects[idx].contains(pos)) {
				setCursor(Qt::PointingHandCursor);
				break;
			}
		}

		//// TODO: change if closed hand cursor is present...
		//if (idx == startActionsRects.size())
		//	setCursor(Qt::OpenHandCursor);
	}

	if (mVisibleStatusbar)
		getPixelInfo(event->pos());

	if (event->buttons() == Qt::LeftButton) {

		QPointF cPos = event->pos();
		QPointF dxy = (cPos - mPosGrab);
		mPosGrab = cPos;
		moveView(dxy/mWorldMatrix.m11());
	}

	//// scroll thumbs preview
	//if (filePreview && filePreview->isVisible() && event->mButtons() == Qt::MiddleButton) {

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

	// mController should only be on the main screen...
	QDesktopWidget* dw = QApplication::desktop();
	mController->setGeometry(dw->screenGeometry());
	qDebug() << "mController resized to: " << mController->geometry();
}

void DkViewPortFrameless::moveView(QPointF delta) {

	// if no zoom is present -> the translation is like a move window
	if (mWorldMatrix.m11() == 1.0f) {
		float s = (float)mImgMatrix.m11();
		mImgMatrix.translate(delta.x()/s, delta.y()/s);
		mImgViewRect = mImgMatrix.mapRect(mImgRect);
	}
	else
		mWorldMatrix.translate(delta.x(), delta.y());

	controlImagePosition();
	update();
}


void DkViewPortFrameless::controlImagePosition(float, float) {
	// dummy method
}

void DkViewPortFrameless::centerImage() {

}

void DkViewPortFrameless::updateImageMatrix() {

	if (!mImgStorage.hasImage())
		return;

	QRectF oldImgRect = mImgViewRect;
	QTransform oldImgMatrix = mImgMatrix;

	mImgMatrix.reset();

	// if the image is smaller or zoom is active: paint the image as is
	if (!getMainGeometry().contains(mImgRect.toRect()))
		mImgMatrix = getScaledImageMatrix();
	else {

		QPointF p = (mImgViewRect.isEmpty()) ? getMainGeometry().center() : mImgViewRect.center();
		p -= mImgStorage.getImage().rect().center();
		mImgMatrix.translate(p.x()-1, p.y()-1);	// -1 is needed due to float -> int
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

QTransform DkViewPortFrameless::getScaledImageMatrix() {

	QRectF initialRect = mMainScreen;
	QPointF oldCenter = mImgViewRect.isEmpty() ? initialRect.center() : mImgViewRect.center();
	qDebug() << "initial rect: " << initialRect;

	QTransform cT;
	cT.scale(800/initialRect.width(), 800/initialRect.width());
	cT.translate(initialRect.center().x(), initialRect.center().y());
	initialRect = cT.mapRect(initialRect);
	initialRect.moveCenter(oldCenter);

	// the image resizes as we zoom
	float ratioImg = (float)(mImgRect.width()/mImgRect.height());
	float ratioWin = (float)(initialRect.width()/initialRect.height());

	QTransform imgMatrix;
	float s;
	if (mImgRect.width() == 0 || mImgRect.height() == 0)
		s = 1.0f;
	else
		s = (ratioImg > ratioWin) ? (float)(initialRect.width()/mImgRect.width()) : (float)(initialRect.height()/mImgRect.height());

	imgMatrix.scale(s, s);

	QRectF imgViewRect = imgMatrix.mapRect(mImgRect);
	QSizeF sDiff = (initialRect.size() - imgViewRect.size())*0.5f/s;
	imgMatrix.translate(initialRect.left()/s+sDiff.width(), initialRect.top()/s+sDiff.height());

	return imgMatrix;
}

DkViewPortContrast::DkViewPortContrast(QWidget *parent, Qt::WindowFlags flags) : DkViewPort(parent, flags) {

	mColorTable = QVector<QRgb>(256);
	for (int i = 0; i < mColorTable.size(); i++) 
		mColorTable[i] = qRgb(i, i, i);

}

DkViewPortContrast::~DkViewPortContrast() {

	release();
}

void DkViewPortContrast::release() {

	DkViewPort::release();
}

void DkViewPortContrast::changeChannel(int channel) {

	if (channel < 0 || channel >= mImgs.size())
		return;

	if (mImgStorage.hasImage()) {

		mFalseColorImg = mImgs[channel];
		mFalseColorImg.setColorTable(mColorTable);
		mDrawFalseColorImg = true;

		update();

		drawImageHistogram();
	}

}


void DkViewPortContrast::changeColorTable(QGradientStops stops) {
	
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
		for (int i = 0; i < mColorTable.size(); i++)
			mColorTable[i] = qRgb(rLeft, gLeft, bLeft);
	}
	// Otherwise interpolate:
	else {

		int rightStopIdx = 1;
		tmp = stops.at(rightStopIdx).second;
		tmp.getRgb(&rRight, &gRight, &bRight);
		rightStop = stops.at(rightStopIdx).first;
	
		for (int i = 0; i < mColorTable.size(); i++) {
			actPos = (qreal) i / mColorTable.size();

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
				mColorTable[i] = qRgb(rLeft, gLeft, bLeft);
			else if (actPos >= rightStop)
				mColorTable[i] = qRgb(rRight, gRight, bRight);
			else {
				fac = (actPos - leftStop) / (rightStop - leftStop);
				rAct = qRound(rLeft + (rRight - rLeft) * fac);
				gAct = qRound(gLeft + (gRight - gLeft) * fac);
				bAct = qRound(bLeft + (bRight - bLeft) * fac);
				mColorTable[i] = qRgb(rAct, gAct, bAct);
			}	
		}
	}


	mFalseColorImg.setColorTable(mColorTable);
	
	update();
	
}

void DkViewPortContrast::draw(QPainter *painter, float) {

	if (parentWidget() && parentWidget()->isFullScreen()) {
		painter->setWorldMatrixEnabled(false);
		painter->fillRect(QRect(QPoint(), size()), DkSettings::slideShow.backgroundColor);
		painter->setWorldMatrixEnabled(true);
	}

	QImage imgQt = mImgStorage.getImage((float)(mImgMatrix.m11()*mWorldMatrix.m11()));

	if (DkSettings::display.tpPattern && imgQt.hasAlphaChannel()) {

		// don't scale the pattern...
		QTransform scaleIv;
		scaleIv.scale(mWorldMatrix.m11(), mWorldMatrix.m22());
		mPattern.setTransform(scaleIv.inverted());

		painter->setPen(QPen(Qt::NoPen));	// no border
		painter->setBrush(mPattern);
		painter->drawRect(mImgViewRect);
	}

	if (mDrawFalseColorImg)
		painter->drawImage(mImgViewRect, mFalseColorImg, mImgRect);		// TODO: add storage class for falseColorImg
	else 
		painter->drawImage(mImgViewRect, imgQt, QRect(QPoint(), imgQt.size()));

}

void DkViewPortContrast::setImage(QImage newImg) {

	DkViewPort::setImage(newImg);

	if (newImg.isNull())
		return;

	if (mImgStorage.getImage().format() == QImage::Format_Indexed8) {
		mImgs = QVector<QImage>(1);
		mImgs[0] = mImgStorage.getImage();
		mActiveChannel = 0;
	}

#ifdef WITH_OPENCV

	else {	
					
			mImgs = QVector<QImage>(4);
			std::vector<cv::Mat> planes;
			
			cv::Mat imgUC3 = DkImage::qImage2Mat(mImgStorage.getImage());
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
				mImgs[idx] = QImage((const unsigned char*)planes[i].data, (int)planes[i].cols, (int)planes[i].rows, (int)planes[i].step,  QImage::Format_Indexed8);
				mImgs[idx] = mImgs[idx].copy();
				idx++;

			}
			// The first element in the vector contains the gray scale 'average' of the 3 channels:
			cv::Mat grayMat;
			cv::cvtColor(imgUC3, grayMat, CV_BGR2GRAY);
			mImgs[0] = QImage((const unsigned char*)grayMat.data, (int)grayMat.cols, (int)grayMat.rows, (int)grayMat.step,  QImage::Format_Indexed8);
			mImgs[0] = mImgs[0].copy();
			planes.clear();

	}
#else

	else {
		mDrawFalseColorImg = false;
		emit imageModeSet(mode_invalid_format);	
		return;
	}

#endif
	
	mFalseColorImg = mImgs[mActiveChannel];
	mFalseColorImg.setColorTable(mColorTable);
	
	// images with valid color table return img.isGrayScale() false...
	if (mImgs.size() == 1) 
		emit imageModeSet(mode_gray);
	else
		emit imageModeSet(mode_rgb);

	update();

	
}

void DkViewPortContrast::pickColor(bool enable) {

	mIsColorPickerActive = enable;
	this->setCursor(Qt::CrossCursor);

}

void DkViewPortContrast::enableTF(bool enable) {

	mDrawFalseColorImg = enable;
	update();

	drawImageHistogram();

}

void DkViewPortContrast::mousePressEvent(QMouseEvent *event) {

	if (!mIsColorPickerActive)
		DkViewPort::mousePressEvent(event);	// just propagate events, if the color picker is not active
}

void DkViewPortContrast::mouseMoveEvent(QMouseEvent *event) {

	if (!mIsColorPickerActive)
		DkViewPort::mouseMoveEvent(event); // just propagate events, if the color picker is not active
	else if (mVisibleStatusbar)
		getPixelInfo(event->pos());
}

void DkViewPortContrast::mouseReleaseEvent(QMouseEvent *event) {

	if (mIsColorPickerActive) {

		QPointF imgPos = mWorldMatrix.inverted().map(event->pos());
		imgPos = mImgMatrix.inverted().map(imgPos);

		QPoint xy = imgPos.toPoint();

		bool isPointValid = true;

		if (xy.x() < 0 || xy.y() < 0 || xy.x() >= mImgStorage.getImage().width() || xy.y() >= mImgStorage.getImage().height())
			isPointValid = false;

		if (isPointValid) {

			int colorIdx = mImgs[mActiveChannel].pixelIndex(xy);
			qreal normedPos = (qreal) colorIdx / 255;
			emit tFSliderAdded(normedPos);
		}

		//unsetCursor();
		//isColorPickerActive = false;
	} 
	else
		DkViewPort::mouseReleaseEvent(event);
}

void DkViewPortContrast::keyPressEvent(QKeyEvent* event) {

	if ((event->key() == Qt::Key_Escape) && mIsColorPickerActive) {
		unsetCursor();
		mIsColorPickerActive = false;
		update();
		return;
	}
	else
		DkViewPort::keyPressEvent(event);
}

QImage DkViewPortContrast::getImage() {

	if (mDrawFalseColorImg)
		return mFalseColorImg;
	else
		return mImgStorage.getImage();

}

// in contrast mode: if the histogram widget is visible redraw the histogram from the selected image channel data
void DkViewPortContrast::drawImageHistogram() {

	if (mController->getHistogram() && mController->getHistogram()->isVisible()) {
		if(mDrawFalseColorImg) mController->getHistogram()->drawHistogram(mFalseColorImg);
		else mController->getHistogram()->drawHistogram(mImgStorage.getImage());
	}

}

// custom events --------------------------------------------------------------------
//QEvent::Type DkInfoEvent::infoEventType = static_cast<QEvent::Type>(QEvent::registerEventType());
//QEvent::Type DkLoadImageEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());


}
