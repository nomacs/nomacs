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

namespace nmc {

// DkFilePreview --------------------------------------------------------------------
DkFilePreview::DkFilePreview(QWidget* parent, Qt::WindowFlags flags) : DkWidget(parent, flags) {

	this->parent = parent;
	orientation = Qt::Vertical;

	init();
	//setStyleSheet("QToolTip{border: 0px; border-radius: 21px; color: white; background-color: red;}"); //" + DkUtils::colorToString(bgCol) + ";}");

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
	//fileLabel = new DkGradientLabel(this);

	worldMatrix = QTransform();

	moveImageTimer = new QTimer(this);
	moveImageTimer->setInterval(5);	// reduce cpu utilization
	connect(moveImageTimer, SIGNAL(timeout()), this, SLOT(moveImages()));

	leftGradient = (orientation == Qt::Horizontal) ? QLinearGradient(QPoint(0, 0), QPoint(borderTrigger, 0)) : QLinearGradient(QPoint(0, 0), QPoint(0, borderTrigger));
	rightGradient = (orientation == Qt::Horizontal) ? QLinearGradient(QPoint(width()-borderTrigger, 0), QPoint(width(), 0)) : QLinearGradient(QPoint(0, height()-borderTrigger), QPoint(0, height()));
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

	initOrientations();
}

void DkFilePreview::initOrientations() {

	if (orientation == Qt::Horizontal) {
		setMaximumHeight(minHeight);
		setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
		borderTrigger = (float)width()*winPercent;
		leftGradient = QLinearGradient(QPoint(0, 0), QPoint(borderTrigger, 0));
		rightGradient = QLinearGradient(QPoint(width()-borderTrigger, 0), QPoint(width(), 0));
	}
	else {
		setMaximumWidth(minHeight);
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
		borderTrigger = (float)height()*winPercent;
		leftGradient = QLinearGradient(QPoint(0, 0), QPoint(0, borderTrigger));
		rightGradient = QLinearGradient(QPoint(0, height()-borderTrigger), QPoint(0, height()));
	}

	leftGradient.setColorAt(1, Qt::white);
	leftGradient.setColorAt(0, Qt::black);
	rightGradient.setColorAt(1, Qt::black);
	rightGradient.setColorAt(0, Qt::white);
}

void DkFilePreview::paintEvent(QPaintEvent* event) {

	//if (selected != -1)
	//	resize(parent->width(), minHeight+fileLabel->height());	// catch parent resize...

	if (minHeight != DkSettings::display.thumbSize + yOffset) {

		xOffset = qCeil(DkSettings::display.thumbSize*0.1f);
		yOffset = qCeil(DkSettings::display.thumbSize*0.1f);

		minHeight = DkSettings::display.thumbSize + yOffset;
		
		if (orientation == Qt::Horizontal)
			setMaximumHeight(minHeight);
		else
			setMaximumWidth(minHeight);

		//if (fileLabel->height() >= height() && fileLabel->isVisible())
		//	fileLabel->hide();

	}
	//minHeight = DkSettings::DisplaySettings::thumbSize + yOffset;
	//resize(parent->width(), minHeight);

	//qDebug() << "size: " << size();

	QPainter painter(this);
	painter.setBackground(bgCol);

	painter.setPen(Qt::NoPen);
	painter.setBrush(bgCol);
	QRect r = QRect(QPoint(), this->size());
	painter.drawRect(r);

	painter.setWorldTransform(worldMatrix);
	painter.setWorldMatrixEnabled(true);

	if (thumbs.empty()) {
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

	for (int idx = 0; idx < thumbs.size(); idx++) {

		QSharedPointer<DkThumbNailT> thumb = thumbs.at(idx)->getThumb();

		if (thumb->hasImage() == DkThumbNail::exists_not) {
			thumbRects.push_back(QRectF());
			continue;
		}

		QImage img;
		if (thumb->hasImage() == DkThumbNail::loaded)
			img = thumb->getImage();

		QPointF anchor = orientation == Qt::Horizontal ? bufferDim.topRight() : bufferDim.bottomLeft();
		QRectF r = !img.isNull() ? QRectF(anchor, img.size()) : QRectF(anchor, QSize(DkSettings::display.thumbSize, DkSettings::display.thumbSize));
		if (orientation == Qt::Horizontal && height()-yOffset < r.height())
			r.setSize(QSizeF(qFloor(r.width()*(float)(height()-yOffset)/r.height()), height()-yOffset));
		else if (orientation == Qt::Vertical && width()-yOffset < r.width())
			r.setSize(QSizeF(width()-yOffset, qFloor(r.height()*(float)(width()-yOffset)/r.width())));

		// check if the size is still valid
		if (r.width() < 1 || r.height() < 1) 
			continue;

		// center vertically
		if (orientation == Qt::Horizontal)
			r.moveCenter(QPoint(qFloor(r.center().x()), height()/2));
		else
			r.moveCenter(QPoint(width()/2, qFloor(r.center().y())));

		// update the buffer dim
		if (orientation == Qt::Horizontal)
			bufferDim.setRight(qFloor(bufferDim.right() + r.width()) + cvCeil(xOffset/2.0f));
		else
			bufferDim.setBottom(qFloor(bufferDim.bottom() + r.height()) + cvCeil(xOffset/2.0f));
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
	float oldOp = painter->opacity();
	
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
	float oldOp = painter->opacity();

	// draw
	QRectF cr = r;
	cr.setSize(QSize(cr.width()+1, cr.height()+1));
	cr.moveCenter(cr.center() + QPointF(-1,-1));

	QPen cPen(DkSettings::display.highlightColor, 1);
	painter->setBrush(QColor(0,0,0,0));
	painter->setOpacity(1.0);
	painter->setPen(cPen);
	painter->drawRect(cr);

	painter->setOpacity(0.5);
	cr.setSize(QSize(cr.width()+2, cr.height()+2));
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

//void DkFilePreview::createCurrentImg(const QImage& img) {
//
//	QRectF r = img.rect();
//	if (height()-yOffset < r.height())
//		r.setSize(QSizeF(qRound(r.width()*(float)(height()-yOffset)/r.height()), height()-yOffset));
//
//	QPixmap glow = DkImage::colorizePixmap(QPixmap::fromImage(img), DkSettings::display.highlightColor, 1.0f);
//
//	currentImg = QPixmap(r.width()+4, r.height()+4);
//#if QT_VERSION < QT_VERSION_CHECK(4, 8, 0)
//	currentImg.fill(qRgba(0,0,0,0));	// sets alpha wrong
//#else
//	currentImg.fill(QColor(0,0,0,0));	// introduced in Qt 4.8
//#endif
//	//currentImg = QPixmap::fromImage(img);
//
//	QPainter painter(&currentImg);
//	painter.setRenderHint(QPainter::SmoothPixmapTransform);
//	// create border
//	QRectF sr = r;
//	sr.setSize(sr.size()+QSize(2, 2));
//	sr.moveCenter(QRectF(currentImg.rect()).center());
//	painter.setOpacity(0.8);
//	painter.drawPixmap(sr, glow, QRect(QPoint(), img.size()));
//
//	sr.setSize(sr.size()+QSize(2, 2));
//	sr.moveCenter(QRectF(currentImg.rect()).center());
//	painter.setOpacity(0.3);
//	painter.drawPixmap(sr, glow, QRect(QPoint(), img.size()));
//
//	sr = r;
//	sr.moveCenter(QRectF(currentImg.rect()).center());
//	painter.setOpacity(1.0);
//	painter.drawImage(sr, img, QRect(QPoint(), img.size()));
//}

void DkFilePreview::resizeEvent(QResizeEvent *event) {

	if (event->size() == event->oldSize() && 
		(orientation == Qt::Horizontal && this->width() == parent->width()  ||
		orientation == Qt::Vertical && this->height() == parent->height())) {
	
			qDebug() << "parent size: " << parent->height();
			return;
	}

	//minHeight = DkSettings::display.thumbSize + yOffset;

	//if (orientation == Qt::Horizontal) {
	//	setMinimumHeight(1);
	//	setMaximumHeight(minHeight);
	//	resize(parent->width(), event->size().height());
	//}
	//else {
	//	setMinimumWidth(1);
	//	setMaximumWidth(minHeight);
	//	resize(parent->height(), event->size().width());
	//}

	if (currentFileIdx >= 0 && isVisible()) {
		scrollToCurrentImage = true;
		moveImageTimer->start();
	}

	// now update...
	borderTrigger = (orientation == Qt::Horizontal) ? (float)width()*winPercent : (float)height()*winPercent;
	leftGradient.setFinalStop((orientation == Qt::Horizontal) ? QPoint(borderTrigger, 0) : QPoint(0, borderTrigger));
	rightGradient.setStart((orientation == Qt::Horizontal) ? QPoint(width()-borderTrigger, 0) : QPoint(0, height()-borderTrigger));
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
		mouseTrace += fabs(QPointF(lastMousePos - event->pos()).manhattanLength());
		return;
	}

	float eventPos = orientation == Qt::Horizontal ? event->pos().x() : event->pos().y();
	float lastMousePosC = orientation == Qt::Horizontal ? lastMousePos.x() : lastMousePos.y();
	int limit = orientation == Qt::Horizontal ? width() : height();

	if (event->buttons() == Qt::MiddleButton) {

		float enterPosC = orientation == Qt::Horizontal ? enterPos.x() : enterPos.y();
		float dx = std::fabs((float)(enterPosC - eventPos))*0.015;
		dx = std::exp(dx);

		if (enterPosC - eventPos < 0)
			dx = -dx;

		currentDx = dx;	// update dx
		return;
	}

	int mouseDir = eventPos - lastMousePosC;

	if (event->buttons() == Qt::LeftButton) {
		currentDx = mouseDir;
		lastMousePos = event->pos();
		selected = -1;
		setCursor(Qt::ClosedHandCursor);
		scrollToCurrentImage = false;
		moveImages();
		return;
	}

	unsetCursor();

	int ndx = limit - eventPos;
	int pdx = eventPos;

	bool left = pdx < ndx;
	float dx = (left) ? pdx : ndx;

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

				if (selected <= thumbs.size() && selected >= 0) {
					QSharedPointer<DkThumbNailT> thumb = thumbs.at(selected)->getThumb();
					//selectedImg = DkImage::colorizePixmap(QPixmap::fromImage(thumb->getImage()), DkSettings::display.highlightColor, 0.3f);

					// important: setText shows the label - if you then hide it here again you'll get a stack overflow
					//if (fileLabel->height() < height())
					//	fileLabel->setText(thumbs.at(selected).getFile().fileName(), -1);
					setToolTip(thumb->getFile().fileName());
					setStatusTip(thumb->getFile().fileName());
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

			if (idx < thumbs.size() && worldMatrix.mapRect(thumbRects.at(idx)).contains(event->pos())) {
				if (thumbs.at(idx)->isFromZip()) emit changeFileSignal(idx - currentFileIdx);
				else emit loadFileSignal(thumbs.at(idx)->file());
			}
		}
	}
	else
		unsetCursor();

}

void DkFilePreview::wheelEvent(QWheelEvent *event) {

	if (event->modifiers() == Qt::CTRL) {

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
	else
		emit changeFileSignal((event->delta() > 0) ? -1 : 1);
}

void DkFilePreview::leaveEvent(QEvent *event) {

	selected = -1;
	if (!scrollToCurrentImage) {
		moveImageTimer->stop();
		qDebug() << "stopping timer (leaveEvent)";
	}
	//fileLabel->hide();
	update();
}

void DkFilePreview::moveImages() {

	if (!isVisible()) {
		moveImageTimer->stop();
		return;
	}

	int limit = orientation == Qt::Horizontal ? width() : height();
	float center = orientation == Qt::Horizontal ? newFileRect.center().x() : newFileRect.center().y();

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

	float translation = orientation == Qt::Horizontal ? worldMatrix.dx() : worldMatrix.dy();
	float bufferPos = orientation == Qt::Horizontal ? bufferDim.right() : bufferDim.bottom();

	// do not scroll out of the thumbs
	if (translation >= limit*0.5 && currentDx > 0 || translation <= -(bufferPos-limit*0.5+xOffset) && currentDx < 0)
		return;

	// set the last step to match the center of the screen...	(nicer if user scrolls very fast)
	if (translation < limit*0.5 && currentDx > 0 && translation+currentDx > limit*0.5 && currentDx > 0)
		currentDx = limit*0.5-translation;
	else if (translation > -(bufferPos-limit*0.5+xOffset) && translation+currentDx <= -(bufferPos-limit*0.5+xOffset) && currentDx < 0)
		currentDx = -(bufferPos-limit*0.5+xOffset+worldMatrix.dx());

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

	int tIdx = -1;

	for (int idx = 0; idx < thumbs.size(); idx++) {
		if (thumbs.at(idx)->file().absoluteFilePath() == cImage->file().absoluteFilePath()) {
			tIdx = idx;
			break;
		}
	}

	if (tIdx == currentFileIdx)
		return;

	currentFileIdx = tIdx;
	if (currentFileIdx >= 0)
		scrollToCurrentImage = true;
	update();

}

void DkFilePreview::updateThumbs(QVector<QSharedPointer<DkImageContainerT> > thumbs) {

	this->thumbs = thumbs;

	for (int idx = 0; idx < thumbs.size(); idx++) {
		if (thumbs.at(idx)->isSelected()) {
			currentFileIdx = idx;
			break;
		}
	}

	update();
}

void DkFilePreview::setVisible(bool visible) {

	DkWidget::setVisible(visible);
}

// DkThumbLabel --------------------------------------------------------------------
DkThumbLabel::DkThumbLabel(QSharedPointer<DkThumbNailT> thumb, QGraphicsItem* parent) : QGraphicsObject(parent), text(this) {

	thumbInitialized = false;
	fetchingThumb = false;
	isHovered = false;

	//imgLabel = new QLabel(this);
	//imgLabel->setFocusPolicy(Qt::NoFocus);
	//imgLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
	//imgLabel->setScaledContents(true);
	//imgLabel->setFixedSize(10,10);
	//setStyleSheet("QLabel{background: transparent;}");
	setThumb(thumb);

	//setFlag(QGraphicsItem::ItemIsSelectable, false);

#if QT_VERSION < 0x050000
	setAcceptsHoverEvents(true);
#else
	setAcceptHoverEvents(true);
#endif
}

DkThumbLabel::~DkThumbLabel() {}

void DkThumbLabel::setThumb(QSharedPointer<DkThumbNailT> thumb) {

	this->thumb = thumb;

	if (thumb.isNull())
		return;

	connect(thumb.data(), SIGNAL(thumbLoadedSignal()), this, SLOT(updateLabel()));
	//setStatusTip(thumb->getFile().fileName());
	//setToolTip(thumb->getFile().fileName());

	// style dummy
	noImagePen.setColor(QColor(150,150,150));
	noImageBrush = QColor(100,100,100,50);

	QColor col = DkSettings::display.highlightColor;
	col.setAlpha(90);
	selectBrush = col;
	selectPen.setColor(DkSettings::display.highlightColor);
	//selectPen.setWidth(2);
}

QPixmap DkThumbLabel::pixmap() const {

	return icon.pixmap();
}

QRectF DkThumbLabel::boundingRect() const {

	if (icon.pixmap().isNull())
		return QRectF(QPoint(1,1), QSize(DkSettings::display.thumbPreviewSize-2, DkSettings::display.thumbPreviewSize-2));

	// TODO: add toggle
	//if (true) {
	//	QRectF ri = icon.boundingRect();
	//	QRectF rt = text.boundingRect();
	//	rt.moveTopLeft(text.pos());

	//	return ri.united(rt);
	//}

	return icon.boundingRect();
}

QPainterPath DkThumbLabel::shape() const {

	QPainterPath path;

	if (icon.pixmap().isNull()) {
		path.addRect(boundingRect());
		return path;
	}
	else
		path = icon.shape();

	//if (false)
	//	path.addPath(text.shape());


	return path;
}

void DkThumbLabel::updateLabel() {

	if (thumb.isNull())
		return;

	QPixmap pm;

	if (!thumb->getImage().isNull()) {

		pm = QPixmap::fromImage(thumb->getImage());

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
		icon.setPixmap(pm);
		setFlag(ItemIsSelectable, true);
		icon.setFlag(ItemIsSelectable, true);
		//QFlags<enum> f;
	}

	// update label
	text.setPos(0, icon.boundingRect().bottom());
	text.setDefaultTextColor(QColor(255,255,255));
	text.setTextWidth(icon.boundingRect().width()-5);
	QFont font;
	font.setBold(false);
	font.setPixelSize(10);
	text.setFont(font);
	text.setPlainText(thumb->getFile().fileName());
	text.hide();

	prepareGeometryChange();
	updateSize();
}

void DkThumbLabel::updateSize() {


	if (icon.pixmap().isNull())
		return;

	prepareGeometryChange();

	// resize pixmap label
	int maxSize = qMax(icon.pixmap().width(), icon.pixmap().height());
	int ps = DkSettings::display.thumbPreviewSize;

	if ((float)ps/maxSize != scale()) {
		setScale((float)ps/maxSize);
		moveBy(-(icon.pixmap().width()*scale()-ps)*0.5f, -(icon.pixmap().height()*scale()-ps)*0.5f);
	}

	//update();
}	

void DkThumbLabel::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {

	if (thumb.isNull())
		return;

	if (event->buttons() == Qt::LeftButton && event->modifiers() == Qt::ControlModifier) {
		QString exe = QApplication::applicationFilePath();
		QStringList args;
		args.append(thumb->getFile().absoluteFilePath());

		if (objectName() == "DkNoMacsFrameless")
			args.append("1");	

		QProcess::startDetached(exe, args);
	}
	else {
		QFileInfo file = thumb->getFile();
		qDebug() << "trying to load: " << file.absoluteFilePath();
		emit loadFileSignal(file);
	}
}

void DkThumbLabel::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {

	isHovered = true;
	emit showFileSignal(thumb->getFile());
	update();
}

void DkThumbLabel::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {

	isHovered = false;
	emit showFileSignal(QFileInfo());
	update();
}

void DkThumbLabel::setVisible(bool visible) {

	icon.setVisible(visible);
	text.setVisible(visible);
}

void DkThumbLabel::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {

	if (!fetchingThumb && thumb->hasImage() == DkThumbNail::not_loaded && 
		DkSettings::resources.numThumbsLoading < DkSettings::resources.maxThumbsLoading*2) {
			thumb->fetchThumb();
			fetchingThumb = true;
	}
	else if (!thumbInitialized && (thumb->hasImage() == DkThumbNail::loaded || thumb->hasImage() == DkThumbNail::exists_not)) {
		updateLabel();
		thumbInitialized = true;
		return;		// exit - otherwise we get paint errors
	}

	//if (!pixmap().isNull()) {
	//	painter->setRenderHint(QPainter::SmoothPixmapTransform);
	//	painter->drawPixmap(boundingRect(), pixmap(), QRectF(QPoint(), pixmap().size()));
	//}
	if (icon.pixmap().isNull() && thumb->hasImage() == DkThumbNail::exists_not) {
		painter->setPen(noImagePen);
		painter->setBrush(noImageBrush);
		painter->drawRect(boundingRect());
	}
	else if (icon.pixmap().isNull()) {
		QColor c = DkSettings::display.highlightColor;
		c.setAlpha(30);
		painter->setPen(noImagePen);
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

	painter->setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);
	icon.paint(painter, &noSelOption, widget);
	//text.paint(painter, &noSelOption, widget);

	// debug
	//painter->drawRect(text.boundingRect());

	// render hovered
	if (isHovered) {
		painter->setBrush(QColor(255,255,255,60));
		painter->setPen(noImagePen);
		//painter->setPen(Qt::NoPen);
		painter->drawRect(boundingRect());
	}

	// render selected
	if (isSelected()) {
		painter->setBrush(selectBrush);
		painter->setPen(selectPen);
		painter->drawRect(boundingRect());
	}

}

// DkThumbWidget --------------------------------------------------------------------
DkThumbScene::DkThumbScene(QWidget* parent /* = 0 */) : QGraphicsScene(parent) {

	setObjectName("DkThumbWidget");

	xOffset = 0;
	numCols = 0;
	numRows = 0;
	firstLayout = true;

	setBackgroundBrush(DkSettings::slideShow.backgroundColor);
}

void DkThumbScene::updateLayout() {

	if (thumbLabels.empty())
		return;

	QSize pSize;

	if (!views().empty())
		pSize = QSize(views().first()->viewport()->size());

	int oldNumCols = numCols;
	int oldNumRows = numRows;

	xOffset = qCeil(DkSettings::display.thumbPreviewSize*0.1f);
	numCols = qMax(qFloor(((float)pSize.width()-xOffset)/(DkSettings::display.thumbPreviewSize + xOffset)), 1);
	numCols = qMin(thumbLabels.size(), numCols);
	numRows = qCeil((float)thumbLabels.size()/numCols);
	int rIdx = 0;

	qDebug() << "num rows x num cols: " << numCols*numRows;
	qDebug() << " thumb labels size: " << thumbLabels.size();

	int tso = DkSettings::display.thumbPreviewSize+xOffset;
	setSceneRect(0, 0, numCols*tso+xOffset, numRows*tso+xOffset);
	//int fileIdx = thumbPool->getCurrentFileIdx();

	DkTimer dt;
	int cYOffset = xOffset;

	for (int rIdx = 0; rIdx < numRows; rIdx++) {

		int cXOffset = xOffset;

		for (int cIdx = 0; cIdx < numCols; cIdx++) {

			int tIdx = rIdx*numCols+cIdx;

			if (tIdx < 0 || tIdx >= thumbLabels.size())
				break;

			DkThumbLabel* cLabel = thumbLabels.at(tIdx);
			cLabel->setPos(cXOffset, cYOffset);
			cLabel->updateSize();

			//if (tIdx == fileIdx)
			//	cLabel->ensureVisible();

			//cLabel->show();

			cXOffset += DkSettings::display.thumbPreviewSize + xOffset;
		}

		// update ypos
		cYOffset += DkSettings::display.thumbPreviewSize + xOffset;	// 20 for label 
	}

	qDebug() << "moving takes: " << dt.getTotal();

	for (int idx = 0; idx < thumbLabels.size(); idx++) {

		//if (thumbs.at(idx)->isSelected()) {
		//	thumbLabels.at(idx)->ensureVisible();
		//	thumbLabels.at(idx)->setSelected(true);	// not working here?!
		//}
		if (thumbLabels.at(idx)->isSelected())
			thumbLabels.at(idx)->ensureVisible();
	}

	//update();

	//if (verticalScrollBar()->isVisible())
	//	verticalScrollBar()->update();

	firstLayout = false;
}

void DkThumbScene::updateThumbs(QVector<QSharedPointer<DkImageContainerT> > thumbs) {

	this->thumbs = thumbs;
	updateThumbLabels();
}

void DkThumbScene::updateThumbLabels() {

	qDebug() << "updating thumb labels...";

	QWidget* p = reinterpret_cast<QWidget*>(parent());
	if (p && !p->isVisible())
		return;

	DkTimer dt;

	clear();	// deletes the thumbLabels
	qDebug() << "clearing viewport: " << dt.getTotal();
	thumbLabels.clear();
	thumbsNotLoaded.clear();

	qDebug() << "clearing labels takes: " << dt.getTotal();

	for (int idx = 0; idx < thumbs.size(); idx++) {
		DkThumbLabel* thumb = new DkThumbLabel(thumbs.at(idx)->getThumb());
		connect(thumb, SIGNAL(loadFileSignal(QFileInfo&)), this, SLOT(loadFile(QFileInfo&)));
		connect(thumb, SIGNAL(showFileSignal(const QFileInfo&)), this, SLOT(showFile(const QFileInfo&)));
		connect(thumbs.at(idx).data(), SIGNAL(thumbLoadedSignal()), this, SIGNAL(thumbLoadedSignal()));

		//if (lastThumb)
		//	lastThumb->setBrother(thumb);

		//thumb->show();
		addItem(thumb);
		thumbLabels.append(thumb);
		//thumbsNotLoaded.append(thumb);
	}

	showFile(QFileInfo());

	qDebug() << "creating labels takes: " << dt.getTotal();

	if (!thumbs.empty())
		updateLayout();

	qDebug() << "initializing labels takes: " << dt.getTotal();
}

void DkThumbScene::showFile(const QFileInfo& file) {

	if (file.absoluteFilePath() == QDir::currentPath() || file.absoluteFilePath().isEmpty())
		emit statusInfoSignal(tr("%1 Images").arg(QString::number(thumbLabels.size())));
	else
		emit statusInfoSignal(file.fileName());
}

void DkThumbScene::toggleSquaredThumbs(bool squares) {

	DkSettings::display.displaySquaredThumbs = squares;

	for (int idx = 0; idx < thumbLabels.size(); idx++)
		thumbLabels.at(idx)->updateLabel();

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

	int newSize = DkSettings::display.thumbPreviewSize * dx;
	qDebug() << "delta: " << dx;
	qDebug() << "newsize: " << newSize;

	if (newSize > 6 && newSize <= 160) {
		DkSettings::display.thumbPreviewSize = newSize;
		updateLayout();
	}
}

void DkThumbScene::loadFile(QFileInfo& file) {
	emit loadFileSignal(file);
}

void DkThumbScene::selectAllThumbs(bool selected) {

	qDebug() << "selecting...";
	selectThumbs(selected);
}

void DkThumbScene::selectThumbs(bool selected /* = true */, int from /* = 0 */, int to /* = -1 */) {

	if (to == -1)
		to = thumbLabels.size();

	for (int idx = from; idx < to && idx < thumbLabels.size(); idx++) {
		thumbLabels.at(idx)->setSelected(selected);
	}

}

QList<QUrl> DkThumbScene::getSelectedUrls() const {

	QList<QUrl> urls;

	for (int idx = 0; idx < thumbLabels.size(); idx++) {

		if (thumbLabels.at(idx)->isSelected()) {
			urls.append("file:///" + thumbLabels.at(idx)->getThumb()->getFile().absoluteFilePath());
		}
	}

	return urls;
}

// DkThumbView --------------------------------------------------------------------
DkThumbsView::DkThumbsView(DkThumbScene* scene, QWidget* parent /* = 0 */) : QGraphicsView(scene, parent) {

	this->scene = scene;
	connect(scene, SIGNAL(thumbLoadedSignal()), this, SLOT(fetchThumbs()));

	//setDragMode(QGraphicsView::RubberBandDrag);

	setResizeAnchor(QGraphicsView::AnchorUnderMouse);
	setAcceptDrops(true);

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
		//itemClicked = itemAt(event->pos()) != 0;
		//qDebug() << "item clicked: " << itemClicked;
	}

	QGraphicsView::mousePressEvent(event);
}

void DkThumbsView::mouseMoveEvent(QMouseEvent *event) {

	if (event->buttons() == Qt::LeftButton) {

		int dist = QPointF(event->pos()-mousePos).manhattanLength();

		if (dist > QApplication::startDragDistance()) {

			QList<QUrl> urls = scene->getSelectedUrls();

			QMimeData* mimeData = new QMimeData;

			if (!urls.empty()) {
				mimeData->setUrls(urls);
				QDrag* drag = new QDrag(this);
				drag->setMimeData(mimeData);
				Qt::DropAction dropAction = drag->exec(Qt::CopyAction);
			}
		}
	}

	QGraphicsView::mouseMoveEvent(event);
}

void DkThumbsView::mouseReleaseEvent(QMouseEvent *event) {

	QGraphicsView::mouseReleaseEvent(event);
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
		if (DkImageLoader::isValid(file))
			event->acceptProposedAction();
		else if (file.isDir())
			event->acceptProposedAction();
	}

	//QGraphicsView::dragEnterEvent(event);
}

void DkThumbsView::dragMoveEvent(QDragMoveEvent *event) {

	if (event->source() == this)
		event->acceptProposedAction();
	else if (event->mimeData()->hasUrls()) {
		QUrl url = event->mimeData()->urls().at(0);
		url = url.toLocalFile();

		QFileInfo file = QFileInfo(url.toString());

		// just accept image files
		if (DkImageLoader::isValid(file))
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
		QUrl url = event->mimeData()->urls().at(0);
		qDebug() << "dropping: " << url;
		url = url.toLocalFile();

		QFileInfo file = QFileInfo(url.toString());

		emit updateDirSignal(file);
	}

	QGraphicsView::dropEvent(event);

	qDebug() << "drop event...";
}

void DkThumbsView::fetchThumbs() {

	int maxThreads = DkSettings::resources.maxThumbsLoading*2;

	// don't do anything if it is loading anyway
	if (DkSettings::resources.numThumbsLoading)
		return;


	//bool firstReached = false;

	QList<QGraphicsItem*> items = scene->items(mapToScene(viewport()->rect()).boundingRect(), Qt::IntersectsItemShape);

	qDebug() << mapToScene(viewport()->rect()).boundingRect() << " number of items: " << items.size();

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

	thumbsScene = new DkThumbScene(this);
	//thumbsView->setContentsMargins(0,0,0,0);

	view = new DkThumbsView(thumbsScene, this);
	connect(view, SIGNAL(updateDirSignal(QFileInfo)), this, SIGNAL(updateDirSignal(QFileInfo)));

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0,0,0,0);
	layout->addWidget(view);
	setLayout(layout);

	createActions();
}

void DkThumbScrollWidget::addContextMenuActions(const QVector<QAction*>& actions, QString menuTitle) {

	parentActions = actions;

	if (!menuTitle.isEmpty()) {
		QMenu* m = contextMenu->addMenu(menuTitle);
		m->addActions(parentActions.toList());
	}
	else {
		contextMenu->addSeparator();
		contextMenu->addActions(parentActions.toList());
	}

}

void DkThumbScrollWidget::createActions() {

	actions.resize(actions_end);

	actions[select_all] = new QAction(tr("Select &All"), this);
	actions[select_all]->setShortcut(QKeySequence::SelectAll);
	actions[select_all]->setCheckable(true);
	connect(actions[select_all], SIGNAL(triggered(bool)), thumbsScene, SLOT(selectAllThumbs(bool)));

	actions[zoom_in] = new QAction(tr("Zoom &In"), this);
	actions[zoom_in]->setShortcut(Qt::CTRL + Qt::Key_Plus);
	connect(actions[zoom_in], SIGNAL(triggered()), thumbsScene, SLOT(increaseThumbs()));

	actions[zoom_out] = new QAction(tr("Zoom &Out"), this);
	actions[zoom_out]->setShortcut(Qt::CTRL + Qt::Key_Minus);
	connect(actions[zoom_out], SIGNAL(triggered()), thumbsScene, SLOT(decreaseThumbs()));

	actions[display_squares] = new QAction(tr("Display &Squares"), this);
	actions[display_squares]->setCheckable(true);
	actions[display_squares]->setChecked(DkSettings::display.displaySquaredThumbs);
	connect(actions[display_squares], SIGNAL(triggered(bool)), thumbsScene, SLOT(toggleSquaredThumbs(bool)));

	contextMenu = new QMenu(tr("Thumb"), this);
	for (int idx = 0; idx < actions.size(); idx++) {

		actions[idx]->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		contextMenu->addAction(actions.at(idx));
	}

	addActions(actions.toList());
}

void DkThumbScrollWidget::updateThumbs(QVector<QSharedPointer<DkImageContainerT> > thumbs) {

	thumbsScene->updateThumbs(thumbs);
}

void DkThumbScrollWidget::setDir(QFileInfo file) {

	if (isVisible())
		emit updateDirSignal(file);
}

void DkThumbScrollWidget::setVisible(bool visible) {

	DkWidget::setVisible(visible);

	if (visible)
		thumbsScene->updateThumbLabels();

	qDebug() << "showing thumb scroll widget...";
}

void DkThumbScrollWidget::resizeEvent(QResizeEvent *event) {

	if (event->oldSize().width() != event->size().width() && isVisible())
		thumbsScene->updateLayout();

	DkWidget::resizeEvent(event);

}

void DkThumbScrollWidget::contextMenuEvent(QContextMenuEvent *event) {

	//if (!event->isAccepted())
	contextMenu->exec(event->globalPos());
	event->accept();

	//QGraphicsView::contextMenuEvent(event);
}



}