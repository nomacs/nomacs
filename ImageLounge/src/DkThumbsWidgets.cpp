/*******************************************************************************************************
 DkThumbsWidgets.cpp
 Created on:	18.09.2014
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2014 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2014 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2014 Florian Kleber <florian@nomacs.org>

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

#include "DkThumbsWidgets.h"
#include "DkThumbs.h"
#include "DkTimer.h"
#include "DkImageContainer.h"
#include "DkImageStorage.h"
#include "DkSettings.h"
#include "DkImageLoader.h"
#include "DkActionManager.h"
#include "DkUtils.h"
#include "DkMessageBox.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QTimer>
#include <QAction>
#include <QMenu>
#include <qmath.h>
#include <QResizeEvent>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include <QUrl>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QStyleOptionGraphicsItem>
#include <QToolBar>
#include <QToolButton>
#include <QLineEdit>
#include <QClipboard>
#include <QMessageBox>
#include <QInputDialog>
#include <QMimeData>
#pragma warning(pop)		// no warnings from includes - end

namespace nmc {

// DkFilePreview --------------------------------------------------------------------
DkFilePreview::DkFilePreview(QWidget* parent, Qt::WindowFlags flags) : DkWidget(parent, flags) {

	orientation = Qt::Horizontal;
	windowPosition = pos_north;

	init();
	//setStyleSheet("QToolTip{border: 0px; border-radius: 21px; color: white; background-color: red;}"); //" + DkUtils::colorToString(mBgCol) + ";}");

	loadSettings();
	initOrientations();

	createContextMenu();
}

void DkFilePreview::init() {

	setObjectName("DkFilePreview");
	setMouseTracking(true);	//receive mouse event everytime
	
	//thumbsLoader = 0;

	xOffset = qRound(DkSettings::display.thumbSize*0.1f);
	yOffset = qRound(DkSettings::display.thumbSize*0.1f);

	qDebug() << "x offset: " << xOffset;

	currentDx = 0;
	currentFileIdx = -1;
	oldFileIdx = -1;
	mouseTrace = 0;
	scrollToCurrentImage = false;
	isPainted = false;

	winPercent = 0.1f;
	borderTrigger = (orientation == Qt::Horizontal) ? (float)width()*winPercent : (float)height()*winPercent;

	worldMatrix = QTransform();

	moveImageTimer = new QTimer(this);
	moveImageTimer->setInterval(5);	// reduce cpu utilization
	connect(moveImageTimer, SIGNAL(timeout()), this, SLOT(moveImages()));

	int borderTriggerI = qRound(borderTrigger);
	leftGradient = (orientation == Qt::Horizontal) ? QLinearGradient(QPoint(0, 0), QPoint(borderTriggerI, 0)) : QLinearGradient(QPoint(0, 0), QPoint(0, borderTriggerI));
	rightGradient = (orientation == Qt::Horizontal) ? QLinearGradient(QPoint(width()-borderTriggerI, 0), QPoint(width(), 0)) : QLinearGradient(QPoint(0, height()-borderTriggerI), QPoint(0, height()));
	leftGradient.setColorAt(1, Qt::white);
	leftGradient.setColorAt(0, Qt::black);
	rightGradient.setColorAt(1, Qt::black);
	rightGradient.setColorAt(0, Qt::white);

	minHeight = DkSettings::display.thumbSize + yOffset;
	//resize(parent->width(), minHeight);
	
	selected = -1;

	// wheel label
	QPixmap wp = QPixmap(":/nomacs/img/thumbs-move.png");
	wheelButton = new QLabel(this);
	wheelButton->setAttribute(Qt::WA_TransparentForMouseEvents);
	wheelButton->setPixmap(wp);
	wheelButton->hide();
}

void DkFilePreview::initOrientations() {

	if (windowPosition == pos_north || windowPosition == pos_south || windowPosition == pos_dock_hor)
		orientation = Qt::Horizontal;
	else if (windowPosition == pos_east || windowPosition == pos_west || windowPosition == pos_dock_ver)
		orientation = Qt::Vertical;

	if (windowPosition == pos_dock_ver || windowPosition == pos_dock_hor)
		minHeight = max_thumb_size;
	else
		minHeight = DkSettings::display.thumbSize;

	if (orientation == Qt::Horizontal) {

		setMinimumSize(20, 20);
		setMaximumSize(QWIDGETSIZE_MAX, minHeight);
		setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
		borderTrigger = (float)width()*winPercent;
		int borderTriggerI = qRound(borderTrigger);
		leftGradient = QLinearGradient(QPoint(0, 0), QPoint(borderTriggerI, 0));
		rightGradient = QLinearGradient(QPoint(width()-borderTriggerI, 0), QPoint(width(), 0));
	}
	else {

		setMinimumSize(20, 20);
		setMaximumSize(minHeight, QWIDGETSIZE_MAX);
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
		borderTrigger = (float)height()*winPercent;
		int borderTriggerI = qRound(borderTrigger);
		leftGradient = QLinearGradient(QPoint(0, 0), QPoint(0, borderTriggerI));
		rightGradient = QLinearGradient(QPoint(0, height()-borderTriggerI), QPoint(0, height()));
	}

	leftGradient.setColorAt(1, Qt::white);
	leftGradient.setColorAt(0, Qt::black);
	rightGradient.setColorAt(1, Qt::black);
	rightGradient.setColorAt(0, Qt::white);

	worldMatrix.reset();
	currentDx = 0;
	scrollToCurrentImage = true;
	update();

}

void DkFilePreview::loadSettings() {

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup(objectName());
	windowPosition = settings.value("windowPosition", windowPosition).toInt();
	settings.endGroup();

}

void DkFilePreview::saveSettings() {

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup(objectName());
	settings.setValue("windowPosition", windowPosition);
	settings.endGroup();
}

void DkFilePreview::createContextMenu() {

	contextMenuActions.resize(cm_end - 1);	// -1 because we just need to know of one dock widget

	contextMenuActions[cm_pos_west] = new QAction(tr("Show Left"), this);
	contextMenuActions[cm_pos_west]->setStatusTip(tr("Shows the Thumbnail Bar on the Left"));
	connect(contextMenuActions[cm_pos_west], SIGNAL(triggered()), this, SLOT(newPosition()));

	contextMenuActions[cm_pos_north] = new QAction(tr("Show Top"), this);
	contextMenuActions[cm_pos_north]->setStatusTip(tr("Shows the Thumbnail Bar at the Top"));
	connect(contextMenuActions[cm_pos_north], SIGNAL(triggered()), this, SLOT(newPosition()));

	contextMenuActions[cm_pos_east] = new QAction(tr("Show Right"), this);
	contextMenuActions[cm_pos_east]->setStatusTip(tr("Shows the Thumbnail Bar on the Right"));
	connect(contextMenuActions[cm_pos_east], SIGNAL(triggered()), this, SLOT(newPosition()));

	contextMenuActions[cm_pos_south] = new QAction(tr("Show Bottom"), this);
	contextMenuActions[cm_pos_south]->setStatusTip(tr("Shows the Thumbnail Bar at the Bottom"));
	connect(contextMenuActions[cm_pos_south], SIGNAL(triggered()), this, SLOT(newPosition()));

	contextMenuActions[cm_pos_dock_hor] = new QAction(tr("Undock"), this);
	contextMenuActions[cm_pos_dock_hor]->setStatusTip(tr("Undock the thumbnails"));
	connect(contextMenuActions[cm_pos_dock_hor], SIGNAL(triggered()), this, SLOT(newPosition()));

	contextMenu = new QMenu(tr("File Preview Menu"), this);
	contextMenu->addActions(contextMenuActions.toList());
}



void DkFilePreview::paintEvent(QPaintEvent*) {

	//if (selected != -1)
	//	resize(parent->width(), minHeight+fileLabel->height());	// catch parent resize...

	if (minHeight != DkSettings::display.thumbSize + yOffset && windowPosition != pos_dock_hor && windowPosition != pos_dock_ver) {

		xOffset = qCeil(DkSettings::display.thumbSize*0.1f);
		yOffset = qCeil(DkSettings::display.thumbSize*0.1f);

		minHeight = DkSettings::display.thumbSize + yOffset;
		
		if (orientation == Qt::Horizontal)
			setMaximumSize(QWIDGETSIZE_MAX, minHeight);
		else
			setMaximumSize(minHeight, QWIDGETSIZE_MAX);

		//if (fileLabel->height() >= height() && fileLabel->isVisible())
		//	fileLabel->hide();

	}
	//minHeight = DkSettings::DisplaySettings::thumbSize + yOffset;
	//resize(parent->width(), minHeight);

	//qDebug() << "size: " << size();

	QPainter painter(this);
	painter.setBackground(mBgCol);

	painter.setPen(Qt::NoPen);
	painter.setBrush(mBgCol);
	
	if (windowPosition != pos_dock_hor && windowPosition != pos_dock_ver) {
		QRect r = QRect(QPoint(), this->size());
		painter.drawRect(r);
	}

	painter.setWorldTransform(worldMatrix);
	painter.setWorldMatrixEnabled(true);

	if (mThumbs.empty()) {
		thumbRects.clear();
		return;
	}

	painter.setRenderHint(QPainter::SmoothPixmapTransform);
	drawThumbs(&painter);

	if (currentFileIdx != oldFileIdx && currentFileIdx >= 0) {
		oldFileIdx = currentFileIdx;
		scrollToCurrentImage = true;
		moveImageTimer->start();
	}
	isPainted = true;

}

void DkFilePreview::drawThumbs(QPainter* painter) {

	//qDebug() << "drawing thumbs: " << worldMatrix.dx();

	bufferDim = (orientation == Qt::Horizontal) ? QRectF(QPointF(0, yOffset/2), QSize(xOffset, 0)) : QRectF(QPointF(yOffset/2, 0), QSize(0, xOffset));
	thumbRects.clear();

	DkTimer dt;

	// mouse over effect
	QPoint p = worldMatrix.inverted().map(mapFromGlobal(QCursor::pos()));

	for (int idx = 0; idx < mThumbs.size(); idx++) {

		QSharedPointer<DkThumbNailT> thumb = mThumbs.at(idx)->getThumb();

		if (thumb->hasImage() == DkThumbNail::exists_not) {
			thumbRects.push_back(QRectF());
			continue;
		}

		QImage img;
		if (thumb->hasImage() == DkThumbNail::loaded)
			img = thumb->getImage();

		QPointF anchor = orientation == Qt::Horizontal ? bufferDim.topRight() : bufferDim.bottomLeft();
		QRectF r = !img.isNull() ? QRectF(anchor, img.size()) : QRectF(anchor, QSize(DkSettings::display.thumbSize, DkSettings::display.thumbSize));
		if (orientation == Qt::Horizontal && height()-yOffset < r.height()*2)
			r.setSize(QSizeF(qFloor(r.width()*(float)(height()-yOffset)/r.height()), height()-yOffset));
		else if (orientation == Qt::Vertical && width()-yOffset < r.width()*2)
			r.setSize(QSizeF(width()-yOffset, qFloor(r.height()*(float)(width()-yOffset)/r.width())));

		// check if the size is still valid
		if (r.width() < 1 || r.height() < 1) 
			continue;	// this brings us in serious problems with the selection

		// center vertically
		if (orientation == Qt::Horizontal)
			r.moveCenter(QPoint(qFloor(r.center().x()), height()/2));
		else
			r.moveCenter(QPoint(width()/2, qFloor(r.center().y())));

		// update the buffer dim
		if (orientation == Qt::Horizontal)
			bufferDim.setRight(qFloor(bufferDim.right() + r.width()) + qCeil(xOffset/2.0f));
		else
			bufferDim.setBottom(qFloor(bufferDim.bottom() + r.height()) + qCeil(xOffset/2.0f));
		thumbRects.push_back(r);

		QRectF imgWorldRect = worldMatrix.mapRect(r);

		// update file rect for move to current file timer
		if (scrollToCurrentImage && idx == currentFileIdx)
			newFileRect = imgWorldRect;

		// is the current image within the canvas?
		if (orientation == Qt::Horizontal && imgWorldRect.right() < 0 || orientation == Qt::Vertical && imgWorldRect.bottom() < 0)
			continue;
		if ((orientation == Qt::Horizontal && imgWorldRect.left() > width() || orientation == Qt::Vertical && imgWorldRect.top() > height()) && scrollToCurrentImage) 
			continue;
		else if (orientation == Qt::Horizontal && imgWorldRect.left() > width() || orientation == Qt::Vertical && imgWorldRect.top() > height())
			break;

		if (thumb->hasImage() == DkThumbNail::not_loaded && 
			DkSettings::resources.numThumbsLoading < DkSettings::resources.maxThumbsLoading) {
				thumb->fetchThumb();
				connect(thumb.data(), SIGNAL(thumbLoadedSignal()), this, SLOT(update()));
		}

		bool isLeftGradient = (orientation == Qt::Horizontal && worldMatrix.dx() < 0 && imgWorldRect.left() < leftGradient.finalStop().x()) ||
			(orientation == Qt::Vertical && worldMatrix.dy() < 0 && imgWorldRect.top() < leftGradient.finalStop().y());
		bool isRightGradient = orientation == Qt::Horizontal && imgWorldRect.right() > rightGradient.start().x() ||
			orientation == Qt::Vertical && imgWorldRect.bottom() > rightGradient.start().y();
		// show that there are more images...
		if (isLeftGradient && !img.isNull())
			drawFadeOut(leftGradient, imgWorldRect, &img);
		if (isRightGradient && !img.isNull())
			drawFadeOut(rightGradient, imgWorldRect, &img);

		if (!img.isNull())
			painter->drawImage(r, img, QRect(QPoint(), img.size()));
		else 
			drawNoImgEffect(painter, r);
				
		if (idx == currentFileIdx)
			drawCurrentImgEffect(painter, r);
		else if (idx == selected && r.contains(p))
			drawSelectedEffect(painter, r);


		//painter->fillRect(QRect(0,0,200, 110), leftGradient);
	}
}

void DkFilePreview::drawNoImgEffect(QPainter* painter, const QRectF& r) {

	QBrush oldBrush = painter->brush();
	QPen oldPen = painter->pen();

	QPen noImgPen(DkSettings::display.bgColor);
	painter->setPen(noImgPen);
	painter->setBrush(QColor(0,0,0,0));
	painter->drawRect(r);
	painter->setPen(oldPen);
	painter->setBrush(oldBrush);
}

void DkFilePreview::drawSelectedEffect(QPainter* painter, const QRectF& r) {

	QBrush oldBrush = painter->brush();
	float oldOp = (float)painter->opacity();
	
	// drawing
	painter->setOpacity(0.4);
	painter->setBrush(DkSettings::display.highlightColor);
	painter->drawRect(r);
	
	// reset painter
	painter->setOpacity(oldOp);
	painter->setBrush(oldBrush);
}

void DkFilePreview::drawCurrentImgEffect(QPainter* painter, const QRectF& r) {

	QPen oldPen = painter->pen();
	QBrush oldBrush = painter->brush();
	float oldOp = (float)painter->opacity();

	// draw
	QRectF cr = r;
	cr.setSize(QSize((int)cr.width()+1, (int)cr.height()+1));
	cr.moveCenter(cr.center() + QPointF(-1,-1));

	QPen cPen(DkSettings::display.highlightColor, 1);
	painter->setBrush(QColor(0,0,0,0));
	painter->setOpacity(1.0);
	painter->setPen(cPen);
	painter->drawRect(cr);

	painter->setOpacity(0.5);
	cr.setSize(QSize((int)cr.width()+2, (int)cr.height()+2));
	cr.moveCenter(cr.center() + QPointF(-1,-1));
	painter->drawRect(cr);

	painter->setBrush(oldBrush);
	painter->setOpacity(oldOp);
	painter->setPen(oldPen);
}

void DkFilePreview::drawFadeOut(QLinearGradient gradient, QRectF imgRect, QImage *img) {

	if (img && img->format() == QImage::Format_Indexed8)
		return;

	// compute current scaling
	QPointF scale(img->width()/imgRect.width(), img->height()/imgRect.height());
	QTransform wm;
	wm.scale(scale.x(), scale.y());
	
	if (orientation == Qt::Horizontal)
		wm.translate(-imgRect.left(), 0);
	else
		wm.translate(0, -imgRect.top());

	QLinearGradient imgGradient = gradient;
	
	if (orientation == Qt::Horizontal) {
		imgGradient.setStart(wm.map(gradient.start()).x(), 0);
		imgGradient.setFinalStop(wm.map(gradient.finalStop()).x(), 0);
	}
	else {
		imgGradient.setStart(0, wm.map(gradient.start()).y());
		imgGradient.setFinalStop(0, wm.map(gradient.finalStop()).y());
	}

	QImage mask = *img;
	QPainter painter(&mask);
	painter.fillRect(img->rect(), Qt::black);
	painter.fillRect(img->rect(), imgGradient);
	painter.end();

	img->setAlphaChannel(mask);
}

void DkFilePreview::resizeEvent(QResizeEvent *event) {

	QWidget* pw = qobject_cast<QWidget*>(parent());

	if (event->size() == event->oldSize() && 
		(orientation == Qt::Horizontal && pw && this->width() == pw->width()  ||
		orientation == Qt::Vertical && pw && this->height() == pw->height())) {
	
			qDebug() << "parent size: " << pw->height();
			return;
	}

	if (currentFileIdx >= 0 && isVisible()) {
		scrollToCurrentImage = true;
		moveImageTimer->start();
	}

	// now update...
	borderTrigger = (orientation == Qt::Horizontal) ? (float)width()*winPercent : (float)height()*winPercent;
	int borderTriggerI = qRound(borderTrigger);
	leftGradient.setFinalStop((orientation == Qt::Horizontal) ? QPoint(borderTriggerI, 0) : QPoint(0, borderTriggerI));
	rightGradient.setStart((orientation == Qt::Horizontal) ? QPoint(width()-borderTriggerI, 0) : QPoint(0, height()-borderTriggerI));
	rightGradient.setFinalStop((orientation == Qt::Horizontal) ?  QPoint(width(), 0) : QPoint(0, height()));

	qDebug() << "file preview size: " << event->size();

	//update();
	QWidget::resizeEvent(event);

}

void DkFilePreview::mouseMoveEvent(QMouseEvent *event) {

	if (lastMousePos.isNull()) {
		lastMousePos = event->pos();
		QWidget::mouseMoveEvent(event);
		return;
	}

	if (mouseTrace < 21) {
		mouseTrace += qRound(fabs(QPointF(lastMousePos - event->pos()).manhattanLength()));
		return;
	}

	float eventPos = orientation == Qt::Horizontal ? (float)event->pos().x() : (float)event->pos().y();
	float lastMousePosC = orientation == Qt::Horizontal ? (float)lastMousePos.x() : (float)lastMousePos.y();
	int limit = orientation == Qt::Horizontal ? width() : height();

	if (event->buttons() == Qt::MiddleButton) {

		float enterPosC = orientation == Qt::Horizontal ? (float)enterPos.x() : (float)enterPos.y();
		float dx = std::fabs((float)(enterPosC - eventPos))*0.015f;
		dx = std::exp(dx);

		if (enterPosC - eventPos < 0)
			dx = -dx;

		currentDx = dx;	// update dx
		return;
	}

	int mouseDir = qRound(eventPos - lastMousePosC);

	if (event->buttons() == Qt::LeftButton) {
		currentDx = (float)mouseDir;
		lastMousePos = event->pos();
		selected = -1;
		setCursor(Qt::ClosedHandCursor);
		scrollToCurrentImage = false;
		moveImages();
		return;
	}

	unsetCursor();

	int ndx = limit - qRound(eventPos);
	int pdx = qRound(eventPos);

	bool left = pdx < ndx;
	float dx = (left) ? (float)pdx : (float)ndx;

	if (dx < borderTrigger && (mouseDir < 0 && left || mouseDir > 0 && !left)) {
		dx = std::exp((borderTrigger - dx)/borderTrigger*3);
		currentDx = (left) ? dx : -dx;

		scrollToCurrentImage = false;
		moveImageTimer->start();
	}
	else if (dx > borderTrigger && !scrollToCurrentImage)
		moveImageTimer->stop();

	// select the current thumbnail
	if (dx > borderTrigger*0.5) {

		int oldSelection = selected;
		selected = -1;

		// find out where the mouse is
		for (int idx = 0; idx < thumbRects.size(); idx++) {

			if (worldMatrix.mapRect(thumbRects.at(idx)).contains(event->pos())) {
				selected = idx;

				if (selected <= mThumbs.size() && selected >= 0) {
					QSharedPointer<DkThumbNailT> thumb = mThumbs.at(selected)->getThumb();
					//selectedImg = DkImage::colorizePixmap(QPixmap::fromImage(thumb->getImage()), DkSettings::display.highlightColor, 0.3f);

					// important: setText shows the label - if you then hide it here again you'll get a stack overflow
					//if (fileLabel->height() < height())
					//	fileLabel->setText(thumbs.at(selected).getFile().fileName(), -1);
					QFileInfo fileInfo(thumb->getFilePath());
					QString toolTipInfo = tr("Name: ") + fileInfo.fileName() + 
						"\n" + tr("Size: ") + DkUtils::readableByte((float)fileInfo.size()) + 
						"\n" + tr("Created: ") + fileInfo.created().toString(Qt::SystemLocaleDate);
					setToolTip(toolTipInfo);
					setStatusTip(fileInfo.fileName());
				}
				break;
			}
		}

		if (selected != -1 || selected != oldSelection)
			update();
		//else if (selected == -1)
		//	fileLabel->hide();
	}
	else
		selected = -1;

	if (selected == -1)
		setToolTip(tr("CTRL+Zoom resizes the thumbnails"));


	lastMousePos = event->pos();

	//QWidget::mouseMoveEvent(event);
}

void DkFilePreview::mousePressEvent(QMouseEvent *event) {

	if (event->buttons() == Qt::LeftButton) {
		mouseTrace = 0;
	}
	else if (event->buttons() == Qt::MiddleButton) {

		enterPos = event->pos();
		qDebug() << "stop scrolling (middle button)";
		scrollToCurrentImage = false;
		moveImageTimer->start();

		// show icon
		wheelButton->move(event->pos().x()-16, event->pos().y()-16);
		wheelButton->show();
	}

}

void DkFilePreview::mouseReleaseEvent(QMouseEvent *event) {

	currentDx = 0;
	moveImageTimer->stop();
	wheelButton->hide();
	qDebug() << "stopping image timer (mouse release)";

	if (mouseTrace < 20) {

		// find out where the mouse did click
		for (int idx = 0; idx < thumbRects.size(); idx++) {

			if (idx < mThumbs.size() && worldMatrix.mapRect(thumbRects.at(idx)).contains(event->pos())) {
				if (mThumbs.at(idx)->isFromZip()) 
					emit changeFileSignal(idx - currentFileIdx);
				else 
					emit loadFileSignal(mThumbs.at(idx)->filePath());
			}
		}
	}
	else
		unsetCursor();

}

void DkFilePreview::wheelEvent(QWheelEvent *event) {

	if (event->modifiers() == Qt::CTRL && windowPosition != pos_dock_hor && windowPosition != pos_dock_ver) {

		int newSize = DkSettings::display.thumbSize;
		newSize += qRound(event->delta()*0.05f);

		// make sure it is even
		if (qRound(newSize*0.5f) != newSize*0.5f)
			newSize++;

		if (newSize < 8)
			newSize = 8;
		else if (newSize > 160)
			newSize = 160;

		if (newSize != DkSettings::display.thumbSize) {
			DkSettings::display.thumbSize = newSize;
			update();
		}
	}
	else {
		
		int fc = (event->delta() > 0) ? -1 : 1;
		
		if (!DkSettings::resources.waitForLastImg) {
			currentFileIdx += fc;
			scrollToCurrentImage = true;
		}
		emit changeFileSignal(fc);
	}
}

void DkFilePreview::leaveEvent(QEvent*) {

	selected = -1;
	if (!scrollToCurrentImage) {
		moveImageTimer->stop();
		qDebug() << "stopping timer (leaveEvent)";
	}
	//fileLabel->hide();
	update();
}

void DkFilePreview::contextMenuEvent(QContextMenuEvent *event) {

	contextMenu->exec(event->globalPos());
	event->accept();

	DkWidget::contextMenuEvent(event);
}

void DkFilePreview::newPosition() {

	QAction* sender = static_cast<QAction*>(QObject::sender());

	if (!sender)
		return;

	int pos = 0;
	Qt::Orientation orient = Qt::Horizontal;

	if (sender == contextMenuActions[cm_pos_west]) {
		pos = pos_west;
		orient = Qt::Vertical;
	}
	else if (sender == contextMenuActions[cm_pos_east]) {
		pos = pos_east;
		orient = Qt::Vertical;
	}
	else if (sender == contextMenuActions[cm_pos_north]) {
		pos = pos_north;
		orient = Qt::Horizontal;
	}
	else if (sender == contextMenuActions[cm_pos_south]) {
		pos = pos_south;
		orient = Qt::Horizontal;
	}
	else if (sender == contextMenuActions[cm_pos_dock_hor]) {
		pos = pos_dock_hor;
		orient = Qt::Horizontal;
	}

	// don't apply twice
	if (windowPosition == pos || pos == pos_dock_hor && windowPosition == pos_dock_ver)
		return;

	windowPosition = pos;
	orientation = orient;
	initOrientations();
	emit positionChangeSignal(windowPosition);

	hide();
	show();

	//emit showThumbsDockSignal(true);
}

void DkFilePreview::moveImages() {

	if (!isVisible()) {
		moveImageTimer->stop();
		return;
	}

	int limit = orientation == Qt::Horizontal ? width() : height();
	float center = orientation == Qt::Horizontal ? (float)newFileRect.center().x() : (float)newFileRect.center().y();

	if (scrollToCurrentImage) {
		float cDist = limit/2.0f - center;

		if (fabs(cDist) < limit) {
			currentDx = sqrt(fabs(cDist))/1.3f;
			if (cDist < 0) currentDx *= -1.0f;
		}
		else
			currentDx = cDist/4.0f;

		if (fabs(currentDx) < 2)
			currentDx = (currentDx < 0) ? -2.0f : 2.0f;

		// end position
		if (fabs(cDist) <= 2) {
			currentDx = limit/2.0f-center;
			moveImageTimer->stop();
			scrollToCurrentImage = false;
		}
		else
			isPainted = false;
	}

	float translation	= orientation == Qt::Horizontal ? (float)worldMatrix.dx() : (float)worldMatrix.dy();
	float bufferPos		= orientation == Qt::Horizontal ? (float)bufferDim.right() : (float)bufferDim.bottom();

	// do not scroll out of the thumbs
	if (translation >= limit*0.5 && currentDx > 0 || translation <= -(bufferPos-limit*0.5+xOffset) && currentDx < 0)
		return;

	// set the last step to match the center of the screen...	(nicer if user scrolls very fast)
	if (translation < limit*0.5 && currentDx > 0 && translation+currentDx > limit*0.5 && currentDx > 0)
		currentDx = limit*0.5f-translation;
	else if (translation > -(bufferPos-limit*0.5+xOffset) && translation+currentDx <= -(bufferPos-limit*0.5+xOffset) && currentDx < 0)
		currentDx = -(bufferPos-limit*0.5f+xOffset+(float)worldMatrix.dx());

	//qDebug() << "currentDx: " << currentDx;
	if (orientation == Qt::Horizontal)
		worldMatrix.translate(currentDx, 0);
	else
		worldMatrix.translate(0, currentDx);
	//qDebug() << "dx: " << worldMatrix.dx();
	update();
}

void DkFilePreview::updateFileIdx(int idx) {

	if (idx == currentFileIdx)
		return;

	currentFileIdx = idx;
	if (currentFileIdx >= 0)
		scrollToCurrentImage = true;
	update();
}

void DkFilePreview::setFileInfo(QSharedPointer<DkImageContainerT> cImage) {

	if (!cImage)
		return;

	int tIdx = -1;

	for (int idx = 0; idx < mThumbs.size(); idx++) {
		if (mThumbs.at(idx)->filePath() == cImage->filePath()) {
			tIdx = idx;
			break;
		}
	}

	//// don't know why we needed this statement
	//// however, if we break here, the file preview
	//// might not update correctly
	//if (tIdx == currentFileIdx) {
	//	return;
	//}

	currentFileIdx = tIdx;
	if (currentFileIdx >= 0)
		scrollToCurrentImage = true;
	update();

}

void DkFilePreview::updateThumbs(QVector<QSharedPointer<DkImageContainerT> > thumbs) {

	this->mThumbs = thumbs;

	for (int idx = 0; idx < thumbs.size(); idx++) {
		if (thumbs.at(idx)->isSelected()) {
			currentFileIdx = idx;
			break;
		}
	}

	update();
}

void DkFilePreview::setVisible(bool visible) {

	emit showThumbsDockSignal(visible);

	DkWidget::setVisible(visible);
}

// DkThumbLabel --------------------------------------------------------------------
DkThumbLabel::DkThumbLabel(QSharedPointer<DkThumbNailT> thumb, QGraphicsItem* parent) : QGraphicsObject(parent), mText(this) {

	mThumbInitialized = false;
	mFetchingThumb = false;
	mIsHovered = false;

	//imgLabel = new QLabel(this);
	//imgLabel->setFocusPolicy(Qt::NoFocus);
	//imgLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
	//imgLabel->setScaledContents(true);
	//imgLabel->setFixedSize(10,10);
	//setStyleSheet("QLabel{background: transparent;}");
	setThumb(thumb);
	setFlag(ItemIsSelectable, true);
	//setFlag(ItemIsMovable, true);	// uncomment this - it's fun : )

	//setFlag(QGraphicsItem::ItemIsSelectable, false);

#if QT_VERSION < 0x050000
	setAcceptsHoverEvents(true);
#else
	setAcceptHoverEvents(true);
#endif
}

DkThumbLabel::~DkThumbLabel() {}

void DkThumbLabel::setThumb(QSharedPointer<DkThumbNailT> thumb) {

	this->mThumb = thumb;

	if (thumb.isNull())
		return;

	connect(thumb.data(), SIGNAL(thumbLoadedSignal()), this, SLOT(updateLabel()));
	//setStatusTip(thumb->getFile().fileName());
	QFileInfo fileInfo(thumb->getFilePath());
	QString toolTipInfo = tr("Name: ") + fileInfo.fileName() + 
		"\n" + tr("Size: ") + DkUtils::readableByte((float)fileInfo.size()) + 
		"\n" + tr("Created: ") + fileInfo.created().toString(Qt::SystemLocaleDate);

	setToolTip(toolTipInfo);

	// style dummy
	mNoImagePen.setColor(QColor(150,150,150));
	mNoImageBrush = QColor(100,100,100,50);

	QColor col = DkSettings::display.highlightColor;
	col.setAlpha(90);
	mSelectBrush = col;
	mSelectPen.setColor(DkSettings::display.highlightColor);
	//selectPen.setWidth(2);
}

QPixmap DkThumbLabel::pixmap() const {

	return mIcon.pixmap();
}

QRectF DkThumbLabel::boundingRect() const {

	return QRectF(QPoint(0,0), QSize(DkSettings::display.thumbPreviewSize, DkSettings::display.thumbPreviewSize));
}

QPainterPath DkThumbLabel::shape() const {

	QPainterPath path;

	path.addRect(boundingRect());
	return path;
}

void DkThumbLabel::updateLabel() {

	if (mThumb.isNull())
		return;

	QPixmap pm;

	if (!mThumb->getImage().isNull()) {

		pm = QPixmap::fromImage(mThumb->getImage());

		if (DkSettings::display.displaySquaredThumbs) {
			QRect r(QPoint(), pm.size());

			if (r.width() > r.height()) {
				r.setX(qFloor((r.width()-r.height())*0.5f));
				r.setWidth(r.height());
			}
			else {
				r.setY(qFloor((r.height()-r.width())*0.5f));
				r.setHeight(r.width());
			}

			pm = pm.copy(r);
		}
	}
	else
		qDebug() << "update called on empty thumb label!";

	if (!pm.isNull()) {
		mIcon.setTransformationMode(Qt::SmoothTransformation);
		mIcon.setPixmap(pm);
		mIcon.setFlag(ItemIsSelectable, true);
		//QFlags<enum> f;
	}
	if (pm.isNull())
		setFlag(ItemIsSelectable, false);	// if we cannot load it -> disable selection

	// update label
	mText.setPos(0, pm.height());
	mText.setDefaultTextColor(QColor(255,255,255));
	//text.setTextWidth(icon.boundingRect().width());
	QFont font;
	font.setBold(false);
	font.setPixelSize(10);
	mText.setFont(font);
	mText.setPlainText(QFileInfo(mThumb->getFilePath()).fileName());
	mText.hide();

	prepareGeometryChange();
	updateSize();
}

void DkThumbLabel::updateSize() {

	if (mIcon.pixmap().isNull())
		return;

	prepareGeometryChange();

	// resize pixmap label
	int maxSize = qMax(mIcon.pixmap().width(), mIcon.pixmap().height());
	int ps = DkSettings::display.thumbPreviewSize;

	if ((float)ps/maxSize != mIcon.scale()) {
		mIcon.setScale(1.0f);
		mIcon.setPos(0,0);

		mIcon.setScale((float)ps/maxSize);
		mIcon.moveBy((ps-mIcon.pixmap().width()*mIcon.scale())*0.5f, (ps-mIcon.pixmap().height()*mIcon.scale())*0.5);
	}

	//update();
}	

void DkThumbLabel::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {

	if (mThumb.isNull())
		return;

	if (event->buttons() == Qt::LeftButton && event->modifiers() == Qt::ControlModifier) {
		QString exe = QApplication::applicationFilePath();
		QStringList args;
		args.append(mThumb->getFilePath());

		if (objectName() == "DkNoMacsFrameless")
			args.append("1");	

		QProcess::startDetached(exe, args);
	}
	else {
		qDebug() << "trying to load: " << mThumb->getFilePath();
		emit loadFileSignal(mThumb->getFilePath());
	}
}

void DkThumbLabel::hoverEnterEvent(QGraphicsSceneHoverEvent*) {

	mIsHovered = true;
	emit showFileSignal(mThumb->getFilePath());
	update();
}

void DkThumbLabel::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {

	mIsHovered = false;
	emit showFileSignal();
	update();
}

void DkThumbLabel::setVisible(bool visible) {

	mIcon.setVisible(visible);
	mText.setVisible(visible);
}

void DkThumbLabel::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	
	if (!mFetchingThumb && mThumb->hasImage() == DkThumbNail::not_loaded && 
		DkSettings::resources.numThumbsLoading < DkSettings::resources.maxThumbsLoading*2) {
			mThumb->fetchThumb();
			mFetchingThumb = true;
	}
	else if (!mThumbInitialized && (mThumb->hasImage() == DkThumbNail::loaded || mThumb->hasImage() == DkThumbNail::exists_not)) {
		updateLabel();
		mThumbInitialized = true;
		return;		// exit - otherwise we get paint errors
	}

	//if (!pixmap().isNull()) {
	//	painter->setRenderHint(QPainter::SmoothPixmapTransform);
	//	painter->drawPixmap(boundingRect(), pixmap(), QRectF(QPoint(), pixmap().size()));
	//}
	if (mIcon.pixmap().isNull() && mThumb->hasImage() == DkThumbNail::exists_not) {
		painter->setPen(mNoImagePen);
		painter->setBrush(mNoImageBrush);
		painter->drawRect(boundingRect());
	}
	else if (mIcon.pixmap().isNull()) {
		QColor c = DkSettings::display.highlightColor;
		c.setAlpha(30);
		painter->setPen(mNoImagePen);
		painter->setBrush(c);

		QRectF r = boundingRect();
		painter->drawRect(r);
	}

	// this is the Qt idea of how to fix the dashed border:
	// http://www.qtcentre.org/threads/23087-How-to-hide-the-dashed-frame-outside-the-QGraphicsItem
	// I don't think it's beautiful...
	QStyleOptionGraphicsItem noSelOption;
	if (option) {
		noSelOption = *option;
		noSelOption.state &= ~QStyle::State_Selected;
	}

	//painter->setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);

	QTransform mt = painter->worldTransform();
	QTransform it = mt;
	it.translate(mIcon.pos().x(), mIcon.pos().y());
	it.scale(mIcon.scale(), mIcon.scale());

	painter->setTransform(it);
	mIcon.paint(painter, &noSelOption, widget);
	painter->setTransform(mt);

	// draw text
	if (boundingRect().width() > 50 && DkSettings::display.showThumbLabel) {
		
		QTransform tt = mt;
		tt.translate(0, boundingRect().height()-mText.boundingRect().height());

		QRectF r = mText.boundingRect();
		r.setWidth(boundingRect().width());
		painter->setPen(Qt::NoPen);
		painter->setWorldTransform(tt);
		painter->setBrush(DkSettings::display.bgColorWidget);
		painter->drawRect(r);
		mText.paint(painter, &noSelOption, widget);
		painter->setWorldTransform(mt);
	}

	// render hovered
	if (mIsHovered) {
		painter->setBrush(QColor(255,255,255,60));
		painter->setPen(mNoImagePen);
		//painter->setPen(Qt::NoPen);
		painter->drawRect(boundingRect());
	}

	// render selected
	if (isSelected()) {
		painter->setBrush(mSelectBrush);
		painter->setPen(mSelectPen);
		painter->drawRect(boundingRect());
	}

}

// DkThumbWidget --------------------------------------------------------------------
DkThumbScene::DkThumbScene(QWidget* parent /* = 0 */) : QGraphicsScene(parent) {

	setObjectName("DkThumbWidget");

}

void DkThumbScene::updateLayout() {

	if (mThumbLabels.empty())
		return;

	QSize pSize;

	if (!views().empty())
		pSize = QSize(views().first()->viewport()->size());

	mXOffset = qCeil(DkSettings::display.thumbPreviewSize*0.1f);
	mNumCols = qMax(qFloor(((float)pSize.width()-mXOffset)/(DkSettings::display.thumbPreviewSize + mXOffset)), 1);
	mNumCols = qMin(mThumbLabels.size(), mNumCols);
	mNumRows = qCeil((float)mThumbLabels.size()/mNumCols);

	qDebug() << "num rows x num cols: " << mNumCols*mNumRows;
	qDebug() << " thumb labels size: " << mThumbLabels.size();

	int tso = DkSettings::display.thumbPreviewSize+mXOffset;
	setSceneRect(0, 0, mNumCols*tso+mXOffset, mNumRows*tso+mXOffset);
	//int fileIdx = thumbPool->getCurrentFileIdx();

	DkTimer dt;
	int cYOffset = mXOffset;

	for (int rIdx = 0; rIdx < mNumRows; rIdx++) {

		int cXOffset = mXOffset;

		for (int cIdx = 0; cIdx < mNumCols; cIdx++) {

			int tIdx = rIdx*mNumCols+cIdx;

			if (tIdx < 0 || tIdx >= mThumbLabels.size())
				break;

			DkThumbLabel* cLabel = mThumbLabels.at(tIdx);
			cLabel->setPos(cXOffset, cYOffset);
			cLabel->updateSize();

			//if (tIdx == fileIdx)
			//	cLabel->ensureVisible();

			//cLabel->show();

			cXOffset += DkSettings::display.thumbPreviewSize + mXOffset;
		}

		// update ypos
		cYOffset += DkSettings::display.thumbPreviewSize + mXOffset;	// 20 for label 
	}

	qDebug() << "moving takes: " << dt.getTotal();

	for (int idx = 0; idx < mThumbLabels.size(); idx++) {

		//if (thumbs.at(idx)->isSelected()) {
		//	thumbLabels.at(idx)->ensureVisible();
		//	thumbLabels.at(idx)->setSelected(true);	// not working here?!
		//}
		if (mThumbLabels.at(idx)->isSelected())
			mThumbLabels.at(idx)->ensureVisible();
	}

	//update();

	//if (verticalScrollBar()->isVisible())
	//	verticalScrollBar()->update();

	mFirstLayout = false;
}

void DkThumbScene::updateThumbs(QVector<QSharedPointer<DkImageContainerT> > thumbs) {

	this->mThumbs = thumbs;
	updateThumbLabels();
}

void DkThumbScene::updateThumbLabels() {

	qDebug() << "updating thumb labels...";

	DkTimer dt;

	blockSignals(true);	// do not emit selection changed while clearing!
	clear();	// deletes the thumbLabels
	blockSignals(false);

	qDebug() << "clearing mViewport: " << dt.getTotal();
	mThumbLabels.clear();

	qDebug() << "clearing labels takes: " << dt.getTotal();

	for (int idx = 0; idx < mThumbs.size(); idx++) {
		DkThumbLabel* thumb = new DkThumbLabel(mThumbs.at(idx)->getThumb());
		connect(thumb, SIGNAL(loadFileSignal(const QString&)), this, SLOT(loadFile(const QString&)));
		connect(thumb, SIGNAL(showFileSignal(const QString&)), this, SLOT(showFile(const QString&)));
		connect(mThumbs.at(idx).data(), SIGNAL(thumbLoadedSignal()), this, SIGNAL(thumbLoadedSignal()));

		//thumb->show();
		addItem(thumb);
		mThumbLabels.append(thumb);
		//thumbsNotLoaded.append(thumb);
	}

	showFile();

	qDebug() << "creating labels takes: " << dt.getTotal();

	if (!mThumbs.empty())
		updateLayout();

	emit selectionChanged();

	qDebug() << "initializing labels takes: " << dt.getTotal();
}

void DkThumbScene::setImageLoader(QSharedPointer<DkImageLoader> loader) {
	
	connectLoader(this->mLoader, false);		// disconnect
	this->mLoader = loader;
	connectLoader(loader);
}

void DkThumbScene::connectLoader(QSharedPointer<DkImageLoader> loader, bool connectSignals) {

	if (!loader)
		return;

	if (connectSignals) {
		connect(loader.data(), SIGNAL(updateDirSignal(QVector<QSharedPointer<DkImageContainerT> >)), this, SLOT(updateThumbs(QVector<QSharedPointer<DkImageContainerT> >)), Qt::UniqueConnection);
	}
	else {
		disconnect(loader.data(), SIGNAL(updateDirSignal(QVector<QSharedPointer<DkImageContainerT> >)), this, SLOT(updateThumbs(QVector<QSharedPointer<DkImageContainerT> >)));
	}
}

void DkThumbScene::showFile(const QString& filePath) {

	if (filePath == QDir::currentPath() || filePath.isEmpty())
		emit statusInfoSignal(tr("%1 Images").arg(QString::number(mThumbLabels.size())));
	else
		emit statusInfoSignal(QFileInfo(filePath).fileName());
}

void DkThumbScene::ensureVisible(QSharedPointer<DkImageContainerT> img) const {

	if (!img)
		return;

	for (DkThumbLabel* label : mThumbLabels) {

		if (label->getThumb()->getFilePath() == img->filePath()) {
			label->ensureVisible();
			break;
		}
	}

}

void DkThumbScene::toggleThumbLabels(bool show) {

	DkSettings::display.showThumbLabel = show;

	for (int idx = 0; idx < mThumbLabels.size(); idx++)
		mThumbLabels.at(idx)->updateLabel();

	//// well, that's not too beautiful
	//if (DkSettings::display.displaySquaredThumbs)
	//	updateLayout();
}

void DkThumbScene::toggleSquaredThumbs(bool squares) {

	DkSettings::display.displaySquaredThumbs = squares;

	for (int idx = 0; idx < mThumbLabels.size(); idx++)
		mThumbLabels.at(idx)->updateLabel();

	// well, that's not too beautiful
	if (DkSettings::display.displaySquaredThumbs)
		updateLayout();
}

void DkThumbScene::increaseThumbs() {

	resizeThumbs(1.2f);
}

void DkThumbScene::decreaseThumbs() {

	resizeThumbs(0.8f);
}

void DkThumbScene::resizeThumbs(float dx) {

	if (dx < 0)
		dx += 2.0f;

	int newSize = qRound(DkSettings::display.thumbPreviewSize * dx);
	qDebug() << "delta: " << dx;
	qDebug() << "newsize: " << newSize;

	if (newSize > 6 && newSize <= 160) {
		DkSettings::display.thumbPreviewSize = newSize;
		updateLayout();
	}
}

void DkThumbScene::loadFile(const QString& filePath) const {
	emit loadFileSignal(filePath);
}

void DkThumbScene::selectAllThumbs(bool selected) {

	qDebug() << "selecting...";
	selectThumbs(selected);
}

void DkThumbScene::selectThumbs(bool selected /* = true */, int from /* = 0 */, int to /* = -1 */) {

	if (mThumbLabels.empty())
		return;

	if (to == -1)
		to = mThumbLabels.size()-1;

	if (from > to) {
		int tmp = to;
		to = from;
		from = tmp;
	}

	blockSignals(true);
	for (int idx = from; idx <= to && idx < mThumbLabels.size(); idx++) {
		mThumbLabels.at(idx)->setSelected(selected);
	}
	blockSignals(false);
	emit selectionChanged();
}

void DkThumbScene::copySelected() const {

	QStringList fileList = getSelectedFiles();

	if (fileList.empty())
		return;

	QMimeData* mimeData = new QMimeData();

	if (!fileList.empty()) {

		QList<QUrl> urls;
		for (QString cStr : fileList)
			urls.append(QUrl::fromLocalFile(cStr));
		mimeData->setUrls(urls);
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setMimeData(mimeData);
	}
}

void DkThumbScene::pasteImages() const {

	copyImages(QApplication::clipboard()->mimeData());
}

void DkThumbScene::copyImages(const QMimeData* mimeData) const {

	if (!mimeData || !mimeData->hasUrls() || !mLoader)
		return;

	QDir dir = mLoader->getDirPath();

	for (QUrl url : mimeData->urls()) {

		QFileInfo fileInfo = DkUtils::urlToLocalFile(url);
		QFile file(fileInfo.absoluteFilePath());
		QString newFilePath = QFileInfo(dir, fileInfo.fileName()).absoluteFilePath();

		// ignore existing silently
		if (QFileInfo(newFilePath).exists())
			continue;

		if (!file.copy(newFilePath)) {
			int answer = QMessageBox::critical(qApp->activeWindow(), tr("Error"), tr("Sorry, I cannot copy %1 to %2")
				.arg(fileInfo.absoluteFilePath(), newFilePath), QMessageBox::Ok | QMessageBox::Cancel);

			if (answer == QMessageBox::Cancel) {
				break;
			}
		}
	}

}

void DkThumbScene::deleteSelected() const {

	QStringList fileList = getSelectedFiles();

	if (fileList.empty())
		return;

	QString question;

#if defined(WIN32) || defined(W_OS_LINUX)
	question = tr("Shall I move %1 file(s) to trash?").arg(fileList.size());
#else
	question = tr("Are you sure you want to permanently delete %1 file(s)?").arg(fileList.size());
#endif

	DkMessageBox* msgBox = new DkMessageBox(
		QMessageBox::Question, 
		tr("Delete File"), 
		question, 
		(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel), 
		qApp->activeWindow());

	msgBox->setDefaultButton(QMessageBox::Yes);
	msgBox->setObjectName("deleteThumbFileDialog");

	int answer = msgBox->exec();

	if (answer == QMessageBox::Yes || answer == QMessageBox::Accepted) {
		
		if (mLoader && fileList.size() > 100)	// saves CPU
			mLoader->deactivate();

		for (const QString& fString : fileList) {

			QString fName = QFileInfo(fString).fileName();
			qDebug() << "deleting: " << fString;

			if (!DkUtils::moveToTrash(fString)) {
				answer = QMessageBox::critical(qApp->activeWindow(), tr("Error"), tr("Sorry, I cannot delete:\n%1").arg(fName), QMessageBox::Ok | QMessageBox::Cancel);

				if (answer == QMessageBox::Cancel) {
					break;
				}
			}
		}

		if (mLoader && fileList.size() > 100)	// saves CPU
			mLoader->activate();

		if (mLoader)
			mLoader->directoryChanged(mLoader->getDirPath());
	}
}

void DkThumbScene::renameSelected() const {

	QStringList fileList = getSelectedFiles();

	if (fileList.empty())
		return;

	bool ok;
	QString newFileName = QInputDialog::getText(qApp->activeWindow(), tr("Rename File(s)"),
		tr("New Filename:"), QLineEdit::Normal,
		"", &ok);
	
	if (ok && !newFileName.isEmpty()) {

		for (int idx = 0; idx < fileList.size(); idx++) {

			QFileInfo fileInfo = fileList.at(idx);
			QFile file(fileInfo.absoluteFilePath());
			QString pattern = (fileList.size() == 1) ? newFileName + ".<old>" : newFileName + "<d:3>.<old>";	// no index if just 1 file was added
			DkFileNameConverter converter(fileInfo.fileName(), pattern, idx);
			QFileInfo newFileInfo(fileInfo.dir(), converter.getConvertedFileName());
			if (!file.rename(newFileInfo.absoluteFilePath())) {
				
				int answer = QMessageBox::critical(qApp->activeWindow(), tr("Error"), tr("Sorry, I cannot rename: %1 to %2")
					.arg(fileInfo.fileName(), newFileInfo.fileName()), QMessageBox::Ok | QMessageBox::Cancel);

				if (answer == QMessageBox::Cancel) {
					break;
				}
			}
		}
	}
}

QStringList DkThumbScene::getSelectedFiles() const {

	QStringList fileList;

	for (int idx = 0; idx < mThumbLabels.size(); idx++) {

		if (mThumbLabels.at(idx) && mThumbLabels.at(idx)->isSelected()) {
			fileList.append(mThumbLabels.at(idx)->getThumb()->getFilePath());
		}
	}

	return fileList;
}

int DkThumbScene::findThumb(DkThumbLabel* thumb) const {

	int thumbIdx = -1;

	for (int idx = 0; idx < mThumbLabels.size(); idx++) {
		if (thumb == mThumbLabels.at(idx)) {
			thumbIdx = idx;
			break;
		}
	}

	return thumbIdx;
}

bool DkThumbScene::allThumbsSelected() const {

	for (DkThumbLabel* label : mThumbLabels)
		if (label->flags() & QGraphicsItem::ItemIsSelectable && !label->isSelected())
			return false;

	return true;
}

// DkThumbView --------------------------------------------------------------------
DkThumbsView::DkThumbsView(DkThumbScene* scene, QWidget* parent /* = 0 */) : QGraphicsView(scene, parent) {

	setObjectName("DkThumbsView");
	this->scene = scene;
	connect(scene, SIGNAL(thumbLoadedSignal()), this, SLOT(fetchThumbs()));

	//setDragMode(QGraphicsView::RubberBandDrag);

	setResizeAnchor(QGraphicsView::AnchorUnderMouse);
	setAcceptDrops(true);

	lastShiftIdx = -1;
}

void DkThumbsView::wheelEvent(QWheelEvent *event) {

	if (event->modifiers() == Qt::ControlModifier) {
		scene->resizeThumbs(event->delta()/100.0f);
	}
	else if (event->modifiers() == Qt::NoModifier) {

		if (verticalScrollBar()->isVisible()) {
			verticalScrollBar()->setValue(verticalScrollBar()->value()-event->delta());
			//fetchThumbs();
			//scene->update();
			//scene->invalidate(scene->sceneRect());
		}
	}

	//QWidget::wheelEvent(event);
}

void DkThumbsView::mousePressEvent(QMouseEvent *event) {

	if (event->buttons() == Qt::LeftButton) {
		mousePos = event->pos();
	}

	qDebug() << "mouse pressed";

#if QT_VERSION < 0x050000
	DkThumbLabel* itemClicked = static_cast<DkThumbLabel*>(scene->itemAt(mapToScene(event->pos())));
#else
	DkThumbLabel* itemClicked = static_cast<DkThumbLabel*>(scene->itemAt(mapToScene(event->pos()), QTransform()));
#endif

	// this is a bit of a hack
	// what we want to achieve: if the user is selecting with e.g. shift or ctrl 
	// and he clicks (unintentionally) into the background - the selection would be lost
	// otherwise so we just don't propagate this event
	if (itemClicked || event->modifiers() == Qt::NoModifier)	
		QGraphicsView::mousePressEvent(event);
}

void DkThumbsView::mouseMoveEvent(QMouseEvent *event) {

	if (event->buttons() == Qt::LeftButton) {

		int dist = qRound(QPointF(event->pos()-mousePos).manhattanLength());

		if (dist > QApplication::startDragDistance()) {

			QStringList fileList = scene->getSelectedFiles();

			QMimeData* mimeData = new QMimeData;

			if (!fileList.empty()) {

				QList<QUrl> urls;
				for (QString fStr : fileList)
					urls.append(QUrl::fromLocalFile(fStr));

				mimeData->setUrls(urls);
				QDrag* drag = new QDrag(this);
				drag->setMimeData(mimeData);
				drag->exec(Qt::CopyAction);
			}
		}
	}

	QGraphicsView::mouseMoveEvent(event);
}

void DkThumbsView::mouseReleaseEvent(QMouseEvent *event) {
	
	QGraphicsView::mouseReleaseEvent(event);
	
#if QT_VERSION < 0x050000
	DkThumbLabel* itemClicked = static_cast<DkThumbLabel*>(scene->itemAt(mapToScene(event->pos())));
#else
	DkThumbLabel* itemClicked = static_cast<DkThumbLabel*>(scene->itemAt(mapToScene(event->pos()), QTransform()));
#endif

	if (lastShiftIdx != -1 && event->modifiers() & Qt::ShiftModifier && itemClicked != 0) {
		scene->selectThumbs(true, lastShiftIdx, scene->findThumb(itemClicked));
		qDebug() << "selecting... with SHIFT from: " << lastShiftIdx << " to: " << scene->findThumb(itemClicked);
	}
	else if (itemClicked != 0) {
		lastShiftIdx = scene->findThumb(itemClicked);
		qDebug() << "starting shift: " << lastShiftIdx;
	}
	else
		lastShiftIdx = -1;

}

void DkThumbsView::dragEnterEvent(QDragEnterEvent *event) {

	qDebug() << event->source() << " I am: " << this;

	if (event->source() == this)
		event->acceptProposedAction();
	else if (event->mimeData()->hasUrls()) {
		QUrl url = event->mimeData()->urls().at(0);
		url = url.toLocalFile();

		QFileInfo file = QFileInfo(url.toString());

		// just accept image files
		if (DkUtils::isValid(file))
			event->acceptProposedAction();
		else if (file.isDir())
			event->acceptProposedAction();
	}

	//QGraphicsView::dragEnterEvent(event);
}

void DkThumbsView::dragMoveEvent(QDragMoveEvent *event) {
//
//	qDebug() << event->source() << " I am: " << this;
//
//	if (event->source() == this)
//		event->acceptProposedAction();
//	else if (event->mimeData()->hasUrls()) {
//		QUrl url = event->mimeData()->urls().at(0);
//		url = url.toLocalFile();
//
//		QFileInfo file = QFileInfo(url.toString());
//
//		// just accept image files
//		if (DkImageLoader::isValid(file))
//			event->acceptProposedAction();
//		else if (file.isDir())
//			event->acceptProposedAction();
//	}
//
//	//QGraphicsView::dragEnterEvent(event);
//}

	if (event->source() == this)
		event->acceptProposedAction();
	else if (event->mimeData()->hasUrls()) {
		QUrl url = event->mimeData()->urls().at(0);
		url = url.toLocalFile();
//		QUrl url = event->mimeData()->urls().at(0);
//		url = url.toLocalFile();
//
//		QFileInfo file = QFileInfo(url.toString());
//
//		// just accept image files
//		if (DkImageLoader::isValid(file))
//			event->acceptProposedAction();
//		else if (file.isDir())
//			event->acceptProposedAction();
//	}
//
//	//QGraphicsView::dragMoveEvent(event);
//}

		QFileInfo file = QFileInfo(url.toString());
//
//	if (event->source() == this) {
//		event->accept();
//		return;
//	}
//
//	if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
//		QUrl url = event->mimeData()->urls().at(0);
//		qDebug() << "dropping: " << url;
//		url = url.toLocalFile();
//
//		QFileInfo file = QFileInfo(url.toString());
//		QDir newDir = file.isDir() ? file.absoluteFilePath() : file.absolutePath();
//
//		emit updateDirSignal(newDir);
//	}
//
//	QGraphicsView::dropEvent(event);
//
//	qDebug() << "drop event...";
//}

		// just accept image files
		if (DkUtils::isValid(file))
			event->acceptProposedAction();
		else if (file.isDir())
			event->acceptProposedAction();
	}

	//QGraphicsView::dragMoveEvent(event);
}

void DkThumbsView::dropEvent(QDropEvent *event) {

	if (event->source() == this) {
		event->accept();
		return;
	}

	if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
		
		if (event->mimeData()->urls().size() > 1) {
			scene->copyImages(event->mimeData());
			return;
		}

		QUrl url = event->mimeData()->urls().at(0);
		qDebug() << "dropping: " << url;
		url = url.toLocalFile();

		QFileInfo file = QFileInfo(url.toString());
		QString newDir = (file.isDir()) ? file.absoluteFilePath() : file.absolutePath();

		emit updateDirSignal(newDir);
	}

	QGraphicsView::dropEvent(event);

	qDebug() << "drop event...";
}

void DkThumbsView::fetchThumbs() {

	int maxThreads = DkSettings::resources.maxThumbsLoading*2;

	// don't do anything if it is loading anyway
	if (DkSettings::resources.numThumbsLoading/* > maxThreads*/) {
		//qDebug() << "rejected because num thumbs loading: " << 
		return;
	}

	qDebug() << "currently loading: " << DkSettings::resources.numThumbsLoading << " thumbs";

	//bool firstReached = false;

	QList<QGraphicsItem*> items = scene->items(mapToScene(viewport()->rect()).boundingRect(), Qt::IntersectsItemShape);

	//qDebug() << mapToScene(mViewport()->rect()).boundingRect() << " number of items: " << items.size();

	for (int idx = 0; idx < items.size(); idx++) {

		if (!maxThreads)
			break;

		DkThumbLabel* th = dynamic_cast<DkThumbLabel*>(items.at(idx));

		if (!th) {
			qDebug() << "not a thumb label...";
			continue;
		}

		if (th->pixmap().isNull()) {
			th->update();
			maxThreads--;
		}
		//else if (!thumbLabels.at(idx)->pixmap().isNull())
		//	firstReached = true;
	}

}

// DkThumbScrollWidget --------------------------------------------------------------------
DkThumbScrollWidget::DkThumbScrollWidget(QWidget* parent /* = 0 */, Qt::WindowFlags flags /* = 0 */) : DkWidget(parent, flags) {

	setObjectName("DkThumbScrollWidget");
	setContentsMargins(0,0,0,0);

	mThumbsScene = new DkThumbScene(this);
	//thumbsView->setContentsMargins(0,0,0,0);

	mView = new DkThumbsView(mThumbsScene, this);
	mView->setFocusPolicy(Qt::StrongFocus);

	createActions();
	createToolbar();

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0,0,0,0);
	layout->setSpacing(0);
	layout->addWidget(mToolbar);
	layout->addWidget(mView);
	setLayout(layout);

	enableSelectionActions();
}

void DkThumbScrollWidget::createToolbar() {

	mToolbar = new QToolBar(tr("Thumb Preview Toolbar"), this);

	if (DkSettings::display.smallIcons)
		mToolbar->setIconSize(QSize(16, 16));
	else
		mToolbar->setIconSize(QSize(32, 32));

	qDebug() << mToolbar->styleSheet();

	if (DkSettings::display.toolbarGradient) {
		mToolbar->setObjectName("toolBarWithGradient");
	}

	DkActionManager& am = DkActionManager::instance();
	mToolbar->addAction(am.action(DkActionManager::preview_zoom_in));
	mToolbar->addAction(am.action(DkActionManager::preview_zoom_out));
	mToolbar->addAction(am.action(DkActionManager::preview_display_squares));
	mToolbar->addAction(am.action(DkActionManager::preview_show_labels));
	mToolbar->addSeparator();
	mToolbar->addAction(am.action(DkActionManager::preview_copy));
	mToolbar->addAction(am.action(DkActionManager::preview_paste));
	mToolbar->addAction(am.action(DkActionManager::preview_rename));
	mToolbar->addAction(am.action(DkActionManager::preview_delete));
	mToolbar->addSeparator();
	mToolbar->addAction(am.action(DkActionManager::preview_batch));

	// add sorting
	QString menuTitle = tr("&Sort");
	QMenu* m = mContextMenu->addMenu(menuTitle);
	m->addActions(am.sortActions().toList());

	QToolButton* toolButton = new QToolButton(this);
	toolButton->setObjectName("DkThumbToolButton");
	toolButton->setMenu(m);
	toolButton->setAccessibleName(menuTitle);
	toolButton->setText(menuTitle);
	QPixmap pm(":/nomacs/img/sort.png");

	if (!DkSettings::display.defaultIconColor || DkSettings::app.privateMode)
		pm = DkImage::colorizePixmap(pm, DkSettings::display.iconColor);

	toolButton->setIcon(pm);
	toolButton->setPopupMode(QToolButton::InstantPopup);
	mToolbar->addWidget(toolButton);

	// filter edit
	mFilterEdit = new QLineEdit("", this);
	mFilterEdit->setPlaceholderText(tr("Filter Files (Ctrl + F)"));
	mFilterEdit->setMaximumWidth(250);

	// right align search filters
	QWidget* spacer = new QWidget(this);
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	mToolbar->addWidget(spacer);
	mToolbar->addWidget(mFilterEdit);
}

void DkThumbScrollWidget::createActions() {

	// create context menu
	mContextMenu = new QMenu(tr("Thumb"), this);
	QVector<QAction*> actions = DkActionManager::instance().previewActions();
	for (int idx = 0; idx < actions.size(); idx++) {

		mContextMenu->addAction(actions.at(idx));

		if (idx == DkActionManager::preview_show_labels)
			mContextMenu->addSeparator();
	}

	DkActionManager& am = DkActionManager::instance();
	//addActions(am.allActions().toList());
	addActions(am.previewActions().toList());
}

void DkThumbScrollWidget::batchProcessFiles() const {

	QStringList fileList = mThumbsScene->getSelectedFiles();
	emit batchProcessFilesSignal(fileList);
}

void DkThumbScrollWidget::updateThumbs(QVector<QSharedPointer<DkImageContainerT> > thumbs) {

	mThumbsScene->updateThumbs(thumbs);
}

void DkThumbScrollWidget::clear() {

	mThumbsScene->updateThumbs(QVector<QSharedPointer<DkImageContainerT> > ());
}

void DkThumbScrollWidget::setDir(const QString& dirPath) {

	if (isVisible())
		emit updateDirSignal(dirPath);
}

void DkThumbScrollWidget::setVisible(bool visible) {

	connectToActions(visible);

	DkWidget::setVisible(visible);

	if (visible) {
		mThumbsScene->updateThumbLabels();
		mFilterEdit->setText("");
		qDebug() << "mShowing thumb scroll widget...";
	}
}

void DkThumbScrollWidget::connectToActions(bool activate) {
	
	DkActionManager& am = DkActionManager::instance();
	for (QAction* a : am.previewActions())
		a->setEnabled(activate);
	
	if (activate) {
		connect(am.action(DkActionManager::preview_select_all), SIGNAL(triggered(bool)), mThumbsScene, SLOT(selectAllThumbs(bool)));
		connect(am.action(DkActionManager::preview_zoom_in), SIGNAL(triggered()), mThumbsScene, SLOT(increaseThumbs()));
		connect(am.action(DkActionManager::preview_zoom_out), SIGNAL(triggered()), mThumbsScene, SLOT(decreaseThumbs()));
		connect(am.action(DkActionManager::preview_display_squares), SIGNAL(triggered(bool)), mThumbsScene, SLOT(toggleSquaredThumbs(bool)));
		connect(am.action(DkActionManager::preview_show_labels), SIGNAL(triggered(bool)), mThumbsScene, SLOT(toggleThumbLabels(bool)));
		connect(am.action(DkActionManager::preview_filter), SIGNAL(triggered()), this, SLOT(setFilterFocus()));
		connect(am.action(DkActionManager::preview_delete), SIGNAL(triggered()), mThumbsScene, SLOT(deleteSelected()));
		connect(am.action(DkActionManager::preview_copy), SIGNAL(triggered()), mThumbsScene, SLOT(copySelected()));
		connect(am.action(DkActionManager::preview_paste), SIGNAL(triggered()), mThumbsScene, SLOT(pasteImages()));
		connect(am.action(DkActionManager::preview_rename), SIGNAL(triggered()), mThumbsScene, SLOT(renameSelected()));
		connect(am.action(DkActionManager::preview_batch), SIGNAL(triggered()), this, SLOT(batchProcessFiles()));

		connect(mFilterEdit, SIGNAL(textChanged(const QString&)), this, SIGNAL(filterChangedSignal(const QString&)));
		connect(mView, SIGNAL(updateDirSignal(const QString&)), this, SIGNAL(updateDirSignal(const QString&)));
		connect(mThumbsScene, SIGNAL(selectionChanged()), this, SLOT(enableSelectionActions()));
	}
	else {
		disconnect(am.action(DkActionManager::preview_select_all), SIGNAL(triggered(bool)), mThumbsScene, SLOT(selectAllThumbs(bool)));
		disconnect(am.action(DkActionManager::preview_zoom_in), SIGNAL(triggered()), mThumbsScene, SLOT(increaseThumbs()));
		disconnect(am.action(DkActionManager::preview_zoom_out), SIGNAL(triggered()), mThumbsScene, SLOT(decreaseThumbs()));
		disconnect(am.action(DkActionManager::preview_display_squares), SIGNAL(triggered(bool)), mThumbsScene, SLOT(toggleSquaredThumbs(bool)));
		disconnect(am.action(DkActionManager::preview_show_labels), SIGNAL(triggered(bool)), mThumbsScene, SLOT(toggleThumbLabels(bool)));
		disconnect(am.action(DkActionManager::preview_filter), SIGNAL(triggered()), this, SLOT(setFilterFocus()));
		disconnect(am.action(DkActionManager::preview_delete), SIGNAL(triggered()), mThumbsScene, SLOT(deleteSelected()));
		disconnect(am.action(DkActionManager::preview_copy), SIGNAL(triggered()), mThumbsScene, SLOT(copySelected()));
		disconnect(am.action(DkActionManager::preview_paste), SIGNAL(triggered()), mThumbsScene, SLOT(pasteImages()));
		disconnect(am.action(DkActionManager::preview_rename), SIGNAL(triggered()), mThumbsScene, SLOT(renameSelected()));
		disconnect(am.action(DkActionManager::preview_batch), SIGNAL(triggered()), this, SLOT(batchProcessFiles()));

		disconnect(mFilterEdit, SIGNAL(textChanged(const QString&)), this, SIGNAL(filterChangedSignal(const QString&)));
		disconnect(mView, SIGNAL(updateDirSignal(const QString&)), this, SIGNAL(updateDirSignal(const QString&)));
		disconnect(mThumbsScene, SIGNAL(selectionChanged()), this, SLOT(enableSelectionActions()));

	}
}

void DkThumbScrollWidget::setFilterFocus() const {

	mFilterEdit->setFocus(Qt::MouseFocusReason);
	qDebug() << "focus set...";
}

void DkThumbScrollWidget::resizeEvent(QResizeEvent *event) {

	if (event->oldSize().width() != event->size().width() && isVisible())
		mThumbsScene->updateLayout();

	DkWidget::resizeEvent(event);

}

void DkThumbScrollWidget::contextMenuEvent(QContextMenuEvent *event) {

	//if (!event->isAccepted())
	mContextMenu->exec(event->globalPos());
	event->accept();

	//QGraphicsView::contextMenuEvent(event);
}

void DkThumbScrollWidget::enableSelectionActions() {

	bool enable = !mThumbsScene->getSelectedFiles().isEmpty();

	DkActionManager& am = DkActionManager::instance();
	am.action(DkActionManager::preview_copy)->setEnabled(enable);
	am.action(DkActionManager::preview_rename)->setEnabled(enable);
	am.action(DkActionManager::preview_delete)->setEnabled(enable);
	am.action(DkActionManager::preview_batch)->setEnabled(enable);

	am.action(DkActionManager::preview_select_all)->setChecked(mThumbsScene->allThumbsSelected());
}

}
