/*******************************************************************************************************
 DkWidgets.cpp
 Created on:	17.05.2011
 
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

#include "DkWidgets.h"

#include "DkNoMacs.h"

namespace nmc {

DkWidget::DkWidget(QWidget* parent, Qt::WFlags flags) : QWidget(parent, flags) {
	init();
}

void DkWidget::init() {

	setMouseTracking(true);

	bgCol = (DkSettings::App::appMode == DkSettings::mode_frameless) ?
		DkSettings::Display::bgColorFrameless :
		DkSettings::Display::bgColorWidget;
	
	showing = false;
	hiding = false;
	blocked = false;
	displaySettingsBits = 0;
	opacityEffect = 0;

	// painter problems if the widget is a child of another that has the same graphicseffect
	// widget starts on hide
	opacityEffect = new QGraphicsOpacityEffect(this);
	opacityEffect->setOpacity(0);
	opacityEffect->setEnabled(false);
	setGraphicsEffect(opacityEffect);

	setVisible(false);
}

void DkWidget::show() {

	// here is a strange problem if you add a DkWidget to another DkWidget -> painters crash
	if (!blocked && !showing) {
		hiding = false;
		showing = true;
		setVisible(true);
		animateOpacityUp();
	}
}

void DkWidget::hide() {

	if (!hiding) {
		hiding = true;
		showing = false;
		animateOpacityDown();

		// set display bit here too -> since the final call to setVisible takes a few seconds
		if (displaySettingsBits && displaySettingsBits->size() > DkSettings::App::currentAppMode) {
			displaySettingsBits->setBit(DkSettings::App::currentAppMode, false);
		}
	}
}

void DkWidget::setVisible(bool visible) {
	
	if (blocked) {
		QWidget::setVisible(false);
		return;
	}

	if (visible && !isVisible() && !showing)
		opacityEffect->setOpacity(100);

	QWidget::setVisible(visible);
	emit visibleSignal(visible);	// if this gets slow -> put it into hide() or show()

	if (displaySettingsBits && displaySettingsBits->size() > DkSettings::App::currentAppMode) {
		displaySettingsBits->setBit(DkSettings::App::currentAppMode, visible);
	}
}

void DkWidget::animateOpacityUp() {

	if (!showing)
		return;

	opacityEffect->setEnabled(true);
	if (opacityEffect->opacity() >= 1.0f || !showing) {
		opacityEffect->setOpacity(1.0f);
		showing = false;
		opacityEffect->setEnabled(false);
		return;
	}

	QTimer::singleShot(20, this, SLOT(animateOpacityUp()));
	opacityEffect->setOpacity(opacityEffect->opacity()+0.05);
}

void DkWidget::animateOpacityDown() {

	if (!hiding)
		return;

	opacityEffect->setEnabled(true);
	if (opacityEffect->opacity() <= 0.0f) {
		opacityEffect->setOpacity(0.0f);
		hiding = false;
		setVisible(false);	// finally hide the widget
		opacityEffect->setEnabled(false);
		return;
	}

	QTimer::singleShot(20, this, SLOT(animateOpacityDown()));
	opacityEffect->setOpacity(opacityEffect->opacity()-0.05);
}

// DkFilePreview --------------------------------------------------------------------
DkFilePreview::DkFilePreview(QWidget* parent, Qt::WFlags flags) : DkWidget(parent, flags) {

	this->parent = parent;
	init();
}

void DkFilePreview::init() {

	setObjectName("DkFilePreview");
	setMouseTracking (true);	//receive mouse event everytime
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	
	thumbsLoader = 0;

	xOffset = qRound(DkSettings::Display::thumbSize*0.1f);
	yOffset = qRound(DkSettings::Display::thumbSize*0.1f);

	qDebug() << "x offset: " << xOffset;

	currentDx = 0;
	currentFileIdx = 0;
	oldFileIdx = 0;
	mouseTrace = 0;
	scrollToCurrentImage = false;

	winPercent = 0.1f;
	borderTrigger = (float)width()*winPercent;
	fileLabel = new DkGradientLabel(this);

	worldMatrix = QTransform();

	moveImageTimer = new QTimer(this);
	connect(moveImageTimer, SIGNAL(timeout()), this, SLOT(moveImages()));
	
	leftGradient = QLinearGradient(QPoint(0, 0), QPoint(borderTrigger, 0));
	rightGradient = QLinearGradient(QPoint(width()-borderTrigger, 0), QPoint(width(), 0));
	leftGradient.setColorAt(1, Qt::white);
	leftGradient.setColorAt(0, Qt::black);
	rightGradient.setColorAt(1, Qt::black);
	rightGradient.setColorAt(0, Qt::white);

	minHeight = DkSettings::Display::thumbSize + yOffset;
	resize(parent->width(), minHeight);
	setMaximumHeight(minHeight);

	selected = -1;

	// load a default image
	QImageReader imageReader(":/nomacs/img/dummy-img.png");
	float fw = (float)DkSettings::Display::thumbSize/(float)imageReader.size().width();
	QSize newSize = QSize(imageReader.size().width()*fw, imageReader.size().height()*fw);
	imageReader.setScaledSize(newSize);
	stubImg = imageReader.read();

	// wheel label
	QPixmap wp = QPixmap(":/nomacs/img/thumbs-move.png");
	wheelButton = new QLabel(this);
	wheelButton->setAttribute(Qt::WA_TransparentForMouseEvents);
	wheelButton->setPixmap(wp);
	wheelButton->hide();

}

void DkFilePreview::paintEvent(QPaintEvent* event) {

	//if (selected != -1)
	//	resize(parent->width(), minHeight+fileLabel->height());	// catch parent resize...

	if (minHeight != DkSettings::Display::thumbSize + yOffset) {

		xOffset = qCeil(DkSettings::Display::thumbSize*0.1f);
		yOffset = qCeil(DkSettings::Display::thumbSize*0.1f);
		
		minHeight = DkSettings::Display::thumbSize + yOffset;
		setMaximumHeight(minHeight);

		if (fileLabel->height() >= height() && fileLabel->isVisible())
			fileLabel->hide();

	}
	//minHeight = DkSettings::DisplaySettings::thumbSize + yOffset;
	//resize(parent->width(), minHeight);

	QPainter painter(this);
	painter.setBackground(bgCol);
	
	painter.setPen(Qt::NoPen);
	painter.setBrush(bgCol);
	QRect r = QRect(QPoint(), this->size());
	painter.drawRect(r);

	painter.setWorldTransform(worldMatrix);
	painter.setWorldMatrixEnabled(true);

	// TODO: paint dummies
	if (thumbs.empty())
		return;

	painter.setRenderHint(QPainter::SmoothPixmapTransform);
	drawThumbs(&painter);

	if (currentFileIdx != oldFileIdx) {
		oldFileIdx = currentFileIdx;
		moveImageTimer->start(1);
	}
}

void DkFilePreview::drawThumbs(QPainter* painter) {

	bufferDim = QRectF(QPointF(0, yOffset/2), QSize(xOffset, 0));
	thumbRects.clear();

	// update stub??
	if (stubImg.width() != DkSettings::Display::thumbSize) {
		// load a default image
		QImageReader imageReader(":/nomacs/img/dummy-img.png");
		float fw = (float)DkSettings::Display::thumbSize/(float)imageReader.size().width();
		QSize newSize = QSize(imageReader.size().width()*fw, imageReader.size().height()*fw);
		imageReader.setScaledSize(newSize);
		stubImg = imageReader.read();
	}

	DkTimer dt;

	for (unsigned int idx = 0; idx < thumbs.size(); idx++) {

		DkThumbNail thumb = thumbs.at(idx);
		
		if (thumb.hasImage() == DkThumbNail::exists_not) {
			thumbRects.push_back(QRectF());
			continue;
		}

		QImage img = (thumb.hasImage() == DkThumbNail::loaded) ? thumb.getImage().copy() : stubImg;
		
		QRectF r = QRectF(bufferDim.topRight(), img.size());
		if (height()-yOffset < r.height())
			r.setSize(QSizeF(qRound(r.width()*(float)(height()-yOffset)/r.height()), height()-yOffset));

		// check if the size is still valid
		if (r.width() < 1 || r.height() < 1) 
			continue;

		// center vertically
		r.moveCenter(QPoint(qRound(r.center().x()), height()/2));

		//if (idx == selected)
		//	qDebug() << "rect: " << r;

		// update the buffer dim
		bufferDim.setRight(qRound(bufferDim.right() + r.width()) + cvCeil(xOffset/2.0f));
		thumbRects.push_back(r);

		QRectF imgWorldRect = worldMatrix.mapRect(r);
		
		// update file rect for move to current file timer
		if (scrollToCurrentImage && idx == currentFileIdx)
			newFileRect = imgWorldRect;

		// is the current image within the canvas?
		if (imgWorldRect.right() < 0)
			continue;
		if (imgWorldRect.left() > width() && scrollToCurrentImage) 
			continue;
		else if (imgWorldRect.left() > width())
			break;

		// load the thumb!
		if (thumb.hasImage() == DkThumbNail::not_loaded && currentFileIdx == oldFileIdx)
			thumbsLoader->setLoadLimits(idx-10, idx+10);

		////imgWorldRect = worldMatrix.mapRect(r);
		//QRectF debugR = worldMatrix.inverted().mapRect(imgWorldRect);
		//painter->drawRect(debugR);

		bool isLeftGradient = worldMatrix.dx() < 0 && imgWorldRect.left() < leftGradient.finalStop().x();
		bool isRightGradient = imgWorldRect.right() > rightGradient.start().x();

		// create effect before gradient (otherwise the effect might be transparent : )
		if (idx == currentFileIdx && (currentImgGlow.isNull() || currentFileIdx != oldFileIdx || currentImgGlow.size() != img.size()))
			createCurrentImgEffect(img.copy(), DkSettings::Display::highlightColor);

		// show that there are more images...
		if (isLeftGradient)
			drawFadeOut(leftGradient, imgWorldRect, &img);
		if (isRightGradient)
			drawFadeOut(rightGradient, imgWorldRect, &img);
		
		if (idx == selected && !selectionGlow.isNull()) {
			painter->drawPixmap(r, selectionGlow, QRect(QPoint(), img.size()));
			painter->setOpacity(0.8);
			painter->drawImage(r, img, QRect(QPoint(), img.size()));
			painter->setOpacity(1.0f);
		}
		else if (idx == currentFileIdx) {

			// create border
			QRectF sr = r;
			sr.setSize(sr.size()+QSize(2, 2));
			sr.moveCenter(r.center());
			painter->setOpacity(0.8);
			painter->drawPixmap(sr, currentImgGlow, QRect(QPoint(), img.size()));

			sr.setSize(sr.size()+QSize(2, 2));
			sr.moveCenter(r.center());
			painter->setOpacity(0.3);
			painter->drawPixmap(sr, currentImgGlow, QRect(QPoint(), img.size()));

			painter->setOpacity(1.0);
			painter->drawImage(r, img, QRect(QPoint(), img.size()));
		}
		else
			painter->drawImage(r, img, QRect(QPoint(), img.size()));

		//painter->fillRect(QRect(0,0,200, 110), leftGradient);
	}

}

void DkFilePreview::drawFadeOut(QLinearGradient gradient, QRectF imgRect, QImage *img) {

	if (img && img->format() == QImage::Format_Indexed8)
		return;

	// compute current scaling
	QPointF scale(img->width()/imgRect.width(), img->height()/imgRect.height());
	QTransform wm;
	wm.scale(scale.x(), scale.y());
	wm.translate(-imgRect.left(), 0);

	QLinearGradient imgGradient = gradient;
	imgGradient.setStart(wm.map(gradient.start()).x(), 0);
	imgGradient.setFinalStop(wm.map(gradient.finalStop()).x(), 0);

	QImage mask = *img;
	QPainter painter(&mask);
	painter.fillRect(img->rect(), Qt::black);
	painter.fillRect(img->rect(), imgGradient);
	painter.end();

	img->setAlphaChannel(mask);
}

void DkFilePreview::createCurrentImgEffect(QImage img, QColor col) {
	
	QPixmap imgPx = QPixmap::fromImage(img);
	currentImgGlow = imgPx;
	currentImgGlow.fill(col);
	currentImgGlow.setAlphaChannel(imgPx.alphaChannel());
	
	//QPixmap glow = imgPx;
	//// Fills the whole pixmap with a certain color
	//// Change to whatever color you want the glow to be
	//// However, it is now just a red box
	//glow.fill(Qt::red);
	//
	//img = img.scaled(img.size()-QSize(20,20), Qt::IgnoreAspectRatio);
	//imgPx = QPixmap::fromImage(img);

	//// This masks out the transparent parts of the image
	//// (or at least that's my understanding of it - that's how
	//// it worked when I tried it, but I'm pretty new to this)
	//// This makes the glow the shape of the pixmap image
	//glow.setMask(imgPx.createHeuristicMask());

	//// To apply the effect, you need to make a QGraphicsItem
	//QGraphicsPixmapItem *glowItem = new QGraphicsPixmapItem(glow);

	//// Add the blur
	//QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
	//// You can fiddle with the blur to get different effects
	//blur->setBlurRadius(20);
	//glowItem->setGraphicsEffect(blur);
	//currentImgGlow = glowItem->pixmap();
	////glowItem->get
}

void DkFilePreview::createSelectedEffect(QImage img, QColor col) {

	QPixmap imgPx = QPixmap::fromImage(img);
	selectionGlow = imgPx;
	selectionGlow.fill(col);
	selectionGlow.setAlphaChannel(imgPx.alphaChannel());

	////what about an outer glow??
	//// To apply the effect, you need to make a QGraphicsItem
	//QGraphicsPixmapItem* glowItem = new QGraphicsPixmapItem(selectionGlow);

	//QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
	//// You can fiddle with the blur to get different effects
	//blur->setBlurRadius(50);
	//// Add the blur
	//glowItem->setGraphicsEffect(blur);
	//glowItem->setScale(1.5);
	//
	//selectionGlow = glowItem->pixmap();

	////selectionGlow = QPixmap(img.height()+30, img.width()+30);
	////QPainter painter(&selectionGlow);

	////glowItem->paint(painter, )
	////blur->draw(&painter);


}

void DkFilePreview::resizeEvent(QResizeEvent *event) {
	
	if (event->size() == event->oldSize() && this->width() == parent->width())
		return;

	minHeight = DkSettings::Display::thumbSize + yOffset;
	setMinimumHeight(1);
	setMaximumHeight(minHeight);

	resize(parent->width(), event->size().height());

	// now update...
	borderTrigger = (float)width()*winPercent;
	leftGradient.setFinalStop(QPoint(borderTrigger, 0));
	rightGradient.setStart(QPoint(width()-borderTrigger, 0));
	rightGradient.setFinalStop(QPoint(width(), 0));
	
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

	if (event->buttons() == Qt::MiddleButton) {

		float dx = std::fabs((float)(enterPos.x() - event->pos().x()))*0.015;
		dx = std::exp(dx);

		if (enterPos.x() - event->pos().x() < 0)
			dx = -dx;

		currentDx = dx;	// update dx
		return;
	}
 
	int mouseDir = event->pos().x() - lastMousePos.x();

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

	int ndx = width() - event->pos().x();
	int pdx = event->pos().x();

	bool left = pdx < ndx;
	float dx = (left) ? pdx : ndx;

	if (dx < borderTrigger && (mouseDir < 0 && left || mouseDir > 0 && !left)) {
		dx = std::exp((borderTrigger - dx)/borderTrigger*3);
		currentDx = (left) ? dx : -dx;
		
		scrollToCurrentImage = false;
		moveImageTimer->start(1);
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

				if ((size_t)selected <= thumbs.size() && selected >= 0) {
					DkThumbNail thumb = thumbs.at(selected);
					createSelectedEffect(thumb.getImage(), DkSettings::Display::highlightColor);
				
					// important: setText shows the label - if you then hide it here again you'll get a stack overflow
					if (fileLabel->height() < height())
						fileLabel->setText(thumbs.at(selected).getFile().fileName(), -1);
				}
				break;
			}
		}

		if (selected != -1 || selected != oldSelection) {
			
			update();
		}
		else if (selected == -1)
			fileLabel->hide();
	}
	else
		selected = -1;

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
		moveImageTimer->start(1);

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

			if (worldMatrix.mapRect(thumbRects.at(idx)).contains(event->pos())) {
				DkThumbNail thumb = thumbs.at(idx);
				emit loadFileSignal(thumb.getFile());
			}
		}
	}
	else
		unsetCursor();

}

void DkFilePreview::wheelEvent(QWheelEvent *event) {

	if (event->modifiers() == Qt::CTRL) {

		int newSize = DkSettings::Display::thumbSize;
		newSize += qRound(event->delta()*0.05f);

		// make sure it is even
		if (qRound(newSize*0.5f) != newSize*0.5f)
			newSize++;

		if (newSize < 8)
			newSize = 8;
		else if (newSize > 160)
			newSize = 160;

		if (newSize != DkSettings::Display::thumbSize) {
			DkSettings::Display::thumbSize = newSize;
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
	fileLabel->hide();
	update();
}

void DkFilePreview::moveImages() {

	if (scrollToCurrentImage) {
		
		float cDist = width()/2.0f - newFileRect.center().x();
		
		if (fabs(cDist) < width())
			currentDx = cDist/50.0f;
		else
			currentDx = cDist/4.0f;

		if (fabs(currentDx) < 1)
			currentDx = (currentDx < 0) ? -1.0f : 1.0f;

		// end position
		if (fabs(cDist) < 1) {
			currentDx = width()/2.0f-newFileRect.center().x();
			moveImageTimer->stop();
			scrollToCurrentImage = false;
		}
	}

	// do not scroll out of the thumbs
	if (worldMatrix.dx() >= width()*0.5 && currentDx > 0 || worldMatrix.dx() <= -(bufferDim.right()-width()*0.5+xOffset) && currentDx < 0)
		return;

	// set the last step to match the center of the screen...	(nicer if user scrolls very fast)
	if (worldMatrix.dx() < width()*0.5 && currentDx > 0 && worldMatrix.dx()+currentDx > width()*0.5 && currentDx > 0)
		currentDx = width()*0.5-worldMatrix.dx();
	else if (worldMatrix.dx() > -(bufferDim.right()-width()*0.5+xOffset) && worldMatrix.dx()+currentDx <= -(bufferDim.right()-width()*0.5+xOffset) && currentDx < 0)
		currentDx = -(bufferDim.right()-width()*0.5+xOffset+worldMatrix.dx());

	//qDebug() << "currentDx: " << currentDx;
	worldMatrix.translate(currentDx, 0);
	update();
}

void DkFilePreview::updateDir(QFileInfo file, int force) {

	currentFile = file;

	if (isVisible())
		indexDir(force);
}

void DkFilePreview::indexDir(int force) {
	
	QDir dir = currentFile.absoluteDir();
	dir.setNameFilters(DkImageLoader::fileFilters);
	dir.setSorting(QDir::LocaleAware);

	// new folder?
	if ((force == DkThumbsLoader::user_updated || force == DkThumbsLoader::dir_updated || 
		thumbsDir.absolutePath() != currentFile.absolutePath() || thumbs.empty()) &&
		!currentFile.absoluteFilePath().contains(":/nomacs/img/lena")) {	// do not load our resources as thumbs

		QStringList files;
		bool myChanges = false;
		if (force == DkThumbsLoader::dir_updated) {
			files = DkImageLoader::getFilteredFileList(dir);

			// this is nasty, but the exif writes to a back-up file so the index changes too if I save thumbnails
			myChanges = thumbsLoader && DkSettings::Display::saveThumb && files.size()-1 == thumbsLoader->getFiles().size() && thumbsLoader->isWorking();
		}

		

		// if a dir update was triggered, only update if the file index changed
		if (force != DkThumbsLoader::dir_updated || 
			(thumbsLoader && files != thumbsLoader->getFiles() && !myChanges)) {

			qDebug() << "force state: " << force;

			if (thumbsLoader && files != thumbsLoader->getFiles()) {
				qDebug() << "file index changed..........." << " my changes: " << myChanges;
				qDebug() << "new index: " << files.size() << " old index: " << thumbsLoader->getFiles().size();
			}

			if (thumbsLoader) {
				thumbsLoader->stop();
				thumbsLoader->wait();
				delete thumbsLoader;
				thumbsLoader = 0;
			}

			thumbs.clear();

			if (dir.exists()) {

				thumbsLoader = new DkThumbsLoader(&thumbs, dir, files);
				connect(thumbsLoader, SIGNAL(updateSignal()), this, SLOT(update()));

				thumbsLoader->start();
				thumbsDir = dir;
			}
		}
	}

	if (thumbsLoader) {
		oldFileIdx = currentFileIdx;
		currentFileIdx = thumbsLoader->getFileIdx(currentFile);
		
		if (currentFileIdx >= 0 && currentFileIdx < thumbsLoader->getFiles().size())
			scrollToCurrentImage = true;
		update();
	}

}

// DkFolderScrollBar --------------------------------------------------------------------
DkFolderScrollBar::DkFolderScrollBar(QWidget* parent) : QScrollBar(Qt::Horizontal, parent) {

	setStyle(new QPlastiqueStyle());

	// apply style
	QVector<QColor> dummy;
	QVector<int> dummyIdx;
	update(dummy, dummyIdx);
 
	dummyWidget = new DkWidget(this);
	
	colorLoader = 0;

	connect(this, SIGNAL(valueChanged(int)), this, SLOT(emitFileSignal(int)));

	qRegisterMetaType<QVector<QColor> >("QVector<QColor>");
	qRegisterMetaType<QVector<int> >("QVector<int>");

	handle = new QLabel(this);
	handle->setStyleSheet(QString("QLabel{border: 1px solid ")
		+ DkUtils::colorToString(DkSettings::Display::highlightColor) + 
		QString("; background-color: ") + DkUtils::colorToString(DkSettings::Display::bgColorWidget) + QString(";}"));

	init();
}

DkFolderScrollBar::~DkFolderScrollBar() {

	if (colorLoader) {
		colorLoader->stop();
		colorLoader->wait();
		delete colorLoader;
		colorLoader = 0;
	}

}

void DkFolderScrollBar::updateDir(QFileInfo file, int force) {

	currentFile = file;

	if (isVisible())
		indexDir(force);
}

void DkFolderScrollBar::indexDir(int force) {

	QDir dir = currentFile.absoluteDir();
	dir.setNameFilters(DkImageLoader::fileFilters);
	dir.setSorting(QDir::LocaleAware);

	// new folder?
	if ((force == DkThumbsLoader::user_updated || force == DkThumbsLoader::dir_updated || 
		currentDir.absolutePath() != currentFile.absolutePath() || files.empty()) &&
		!currentFile.absoluteFilePath().contains(":/nomacs/img/lena")) {	// do not load our resources as thumbs

			QStringList files = DkImageLoader::getFilteredFileList(dir);

			// if a dir update was triggered, only update if the file index changed
			if (force != DkThumbsLoader::dir_updated || files != this->files) {

					qDebug() << "force state: " << force;

					if (colorLoader) {
						colorLoader->stop();	// TODO: can't be stopped by now
						colorLoader->wait();
						delete colorLoader;
						colorLoader = 0;
					}

					if (dir.exists()) {

						colorLoader = new DkColorLoader(dir, files);
						connect(colorLoader, SIGNAL(updateSignal(const QVector<QColor>&, const QVector<int>&)), this, SLOT(update(const QVector<QColor>&, const QVector<int>&)));

						colorLoader->start();
						currentDir = dir;
					}
			}

			handle->setFixedWidth((qRound(1.0f/files.size()*this->width()) < 30) ? 30 : qRound(1.0f/files.size()*this->width()));

			this->files = files;
	}

	// TODO: update scrollbar
	setMaximum(files.size());

	blockSignals(true);
	setValue(files.indexOf(currentFile.fileName()));
	blockSignals(false);
}

void DkFolderScrollBar::update(const QVector<QColor>& colors, const QVector<int>& indexes) {

	QString gs = "qlineargradient(x1:0, y1:0, x2:1, y2:0 ";
	if (colors.empty()) gs += ", stop: 0 " + DkUtils::colorToString(DkSettings::Display::bgColorWidget);

	float maxFiles = (files.size() > 1920) ? 1920 : files.size();

	for (int idx = 0; idx < colors.size(); idx++) {

		QColor cCol = colors[idx];
		//cCol.setAlphaF(0.7);
		gs += ", stop: " + QString::number((float)indexes[idx]/files.size()) + " " + 
			DkUtils::colorToString(cCol); 
	}

	if (colors.size() == maxFiles) 
		gs += ");";
	else
		gs += ", stop: 1 " + DkUtils::colorToString(DkSettings::Display::bgColorWidget) + ");";

	setStyleSheet(QString("QScrollBar:horizontal { ") + 
		QString("border: none;") +
		QString("background: rgba(0,0,0,0);") +
		QString("margin: 0px 0px 0px 0px;") +
		QString("}") +
		// hide default handle
		QString("QScrollBar::handle:horizontal {") +
		QString("background-color: rgba(0,0,0,0); ") +
		QString("border: none;") + 
		QString("width: 0px; height: 0px;") +
		QString("}") +
		QString("QScrollBar::add-line:horizontal {") +
		QString("background: ") + gs +
		QString("border-bottom: 1px solid #000;") +
		QString("subcontrol-position: bottom;") +
		QString("subcontrol-origin: margin;") +
		QString("}") +
		// hide arrows
		QString("QScrollBar::sub-line:horizontal {") +
		QString("width: 0px;") +
		QString("height: 0px;") +
		QString("}"));

	qDebug() << "updating style...";
	
	if (!files.empty())
		handle->setFixedWidth((qRound(1.0f/files.size()*this->width()) < 30) ? 30 : qRound(1.0f/files.size()*this->width()));

}

void DkFolderScrollBar::setValue(int i) {

	if (!files.empty()) {
		QRect r((float)i/files.size()*this->width(), 0, handle->width(), height());
		handle->setGeometry(r);
	}

	QScrollBar::setValue(i);
}

void DkFolderScrollBar::emitFileSignal(int i) {

	qDebug() << "value: " << i;
	int skipIdx = i-files.indexOf(currentFile.fileName());
	emit changeFileSignal(skipIdx);
}

void DkFolderScrollBar::mouseMoveEvent(QMouseEvent *event) {

	if (sliding && event->buttons() == Qt::LeftButton)
		setValue((float)(event->pos().x()-handle->width()*0.5)/width()*maximum());

}

void DkFolderScrollBar::mousePressEvent(QMouseEvent *event) {

	sliding = handle->geometry().contains(event->pos());
}

void DkFolderScrollBar::mouseReleaseEvent(QMouseEvent *event) {

	//if (!sliding)
		setValue((float)(event->pos().x()-handle->width()*0.5)/width()*maximum());

	// do not propagate these events

}

void DkFolderScrollBar::resizeEvent(QResizeEvent *event) {

	if (handle && !files.empty())
		handle->setFixedWidth((qRound(1.0f/files.size()*event->size().width()) < 30) ? 30 : qRound(1.0f/files.size()*event->size().width()));

	QScrollBar::resizeEvent(event);
}

// scrollbar - DkWidget functions
void DkFolderScrollBar::init() {

	setMouseTracking(true);

	bgCol = (DkSettings::App::appMode == DkSettings::mode_frameless) ?
		DkSettings::Display::bgColorFrameless :
	DkSettings::Display::bgColorWidget;

	showing = false;
	hiding = false;
	blocked = false;
	displaySettingsBits = 0;
	opacityEffect = 0;

	// painter problems if the widget is a child of another that has the same graphicseffect
	// widget starts on hide
	opacityEffect = new QGraphicsOpacityEffect(this);
	opacityEffect->setOpacity(0);
	opacityEffect->setEnabled(false);
	setGraphicsEffect(opacityEffect);

	setVisible(false);
}

void DkFolderScrollBar::show() {

	// here is a strange problem if you add a DkWidget to another DkWidget -> painters crash
	if (!blocked && !showing) {
		hiding = false;
		showing = true;
		setVisible(true);
		animateOpacityUp();
	}
}

void DkFolderScrollBar::hide() {

	if (!hiding) {
		hiding = true;
		showing = false;
		animateOpacityDown();

		// set display bit here too -> since the final call to setVisible takes a few seconds
		if (displaySettingsBits && displaySettingsBits->size() > DkSettings::App::currentAppMode) {
			displaySettingsBits->setBit(DkSettings::App::currentAppMode, false);
		}
	}
}

void DkFolderScrollBar::setVisible(bool visible) {

	if (blocked) {
		QWidget::setVisible(false);
		return;
	}

	if (visible)
		indexDir(DkThumbsLoader::not_forced);	// false = do not force refreshing the folder

	if (visible && !isVisible() && !showing)
		opacityEffect->setOpacity(100);

	QWidget::setVisible(visible);
	emit visibleSignal(visible);	// if this gets slow -> put it into hide() or show()

	if (displaySettingsBits && displaySettingsBits->size() > DkSettings::App::currentAppMode) {
		displaySettingsBits->setBit(DkSettings::App::currentAppMode, visible);
	}
}

void DkFolderScrollBar::animateOpacityUp() {

	if (!showing)
		return;

	opacityEffect->setEnabled(true);
	if (opacityEffect->opacity() >= 1.0f || !showing) {
		opacityEffect->setOpacity(1.0f);
		showing = false;
		opacityEffect->setEnabled(false);
		return;
	}

	QTimer::singleShot(20, this, SLOT(animateOpacityUp()));
	opacityEffect->setOpacity(opacityEffect->opacity()+0.05);
}

void DkFolderScrollBar::animateOpacityDown() {

	if (!hiding)
		return;

	opacityEffect->setEnabled(true);
	if (opacityEffect->opacity() <= 0.0f) {
		opacityEffect->setOpacity(0.0f);
		hiding = false;
		setVisible(false);	// finally hide the widget
		opacityEffect->setEnabled(false);
		return;
	}

	QTimer::singleShot(20, this, SLOT(animateOpacityDown()));
	opacityEffect->setOpacity(opacityEffect->opacity()-0.05);
}


// DkThumbsSaver --------------------------------------------------------------------
void DkThumbsSaver::processDir(const QDir& dir, bool forceLoad) {
	
	if (thumbsLoader) {
		thumbsLoader->stop();
		thumbsLoader->wait();
		delete thumbsLoader;
	}

	thumbs.clear();

	if (dir.exists()) {

		thumbsLoader = new DkThumbsLoader(&thumbs, dir);
		thumbsLoader->setForceLoad(forceLoad);

		pd = new QProgressDialog(tr("\nCreating thumbnails...\n") + dir.absolutePath(), tr("Cancel"), 0, (int)thumbs.size(), DkNoMacs::getDialogParent());
		pd->setWindowTitle(tr("Thumbnails"));
		
		//pd->setWindowModality(Qt::WindowModal);

		connect(pd, SIGNAL(canceled()), thumbsLoader, SLOT(stop()));
		connect(thumbsLoader, SIGNAL(numFilesSignal(int)), pd, SLOT(setValue(int)));
		connect(thumbsLoader, SIGNAL(finished()), this, SLOT(stopProgress()));

		pd->show();

		thumbsLoader->start();

		thumbsLoader->loadAll();
	}
}

// DkOverview --------------------------------------------------------------------
DkOverview::DkOverview(QWidget* parent, Qt::WindowFlags flags) : DkWidget(parent, flags) {

	setObjectName("DkOverview");
	this->parent = parent;
	setMinimumSize(0, 0);
	setMaximumSize(200, 200);
}

void DkOverview::paintEvent(QPaintEvent *event) {

	if (img.isNull() || !imgMatrix || !worldMatrix)
		return;

	QPainter painter(this);

	int lm, tm, rm, bm;
	getContentsMargins(&lm, &tm, &rm, &bm);

	QSize viewSize = QSize(width()-lm-rm, height()-tm-bm);	// overview shall take 15% of the viewport....
	
	if (viewSize.width() > 2 && viewSize.height() > 2) {
	
		QRectF imgRect = QRectF(QPoint(), img.size());
		QRectF overviewRect = QRectF(QPoint(lm, tm), QSize(viewSize.width()-1, viewSize.height()-1));			// get the overview rect
		overviewRect = overviewRect.toRect();	// force round

		QRectF overviewImgRect(lm+1, tm+1, imgT.width(), imgT.height());
		overviewImgRect.moveCenter(overviewRect.center());

		QTransform overviewImgMatrix = getScaledImageMatrix();			// matrix that always resizes the image to the current viewport
		//QRectF overviewImgRect = overviewImgMatrix.mapRect(imgRect);
		//overviewImgRect.moveTop(overviewImgRect.top()+tm+1);
		//overviewImgRect.moveLeft(overviewImgRect.left()+lm+1);
		//overviewImgRect = overviewImgRect.toRect();	// force round
		//overviewImgRect.setWidth(overviewImgRect.width()-2);
		//overviewImgRect.setHeight(overviewImgRect.height()-2);

		//qDebug() << "overview image rect: " << overviewImgRect;
		//qDebug() << "overview img size: " << imgT.size();

		// now render the current view
		QRectF viewRect = viewPortRect;
		viewRect = worldMatrix->inverted().mapRect(viewRect);
		viewRect = imgMatrix->inverted().mapRect(viewRect);
		viewRect = overviewImgMatrix.mapRect(viewRect);
		viewRect.moveTopLeft(viewRect.topLeft()+QPointF(lm, tm));

		if(viewRect.topLeft().x() < overviewRect.topLeft().x()) viewRect.setTopLeft(QPointF(overviewRect.topLeft().x(), viewRect.topLeft().y()));
		if(viewRect.topLeft().y() < overviewRect.topLeft().y()) viewRect.setTopLeft(QPointF(viewRect.topLeft().x(), overviewRect.topLeft().y()));
		if(viewRect.bottomRight().x() > overviewRect.bottomRight().x()) viewRect.setBottomRight(QPointF(overviewRect.bottomRight().x(), viewRect.bottomRight().y()));
		if(viewRect.bottomRight().y() > overviewRect.bottomRight().y()) viewRect.setBottomRight(QPointF(viewRect.bottomRight().x(), overviewRect.bottomRight().y()));		

		//draw the image's location
		painter.setBrush(bgCol);
		painter.setPen(QColor(200, 200, 200));
		painter.drawRect(overviewRect);
		painter.setOpacity(0.8f);
		painter.drawImage(overviewImgRect, imgT, QRect(0, 0, imgT.width(), imgT.height()));

		QColor col = DkSettings::Display::highlightColor;
		col.setAlpha(255);
		painter.setPen(col);
		col.setAlpha(50);
		painter.setBrush(col);
		painter.drawRect(viewRect);

	}
	painter.end();

	DkWidget::paintEvent(event);
}

void DkOverview::mousePressEvent(QMouseEvent *event) {
	
	enterPos = event->pos();
	posGrab = event->pos();
}

void DkOverview::mouseReleaseEvent(QMouseEvent *event) {

	QPointF dxy = enterPos-QPointF(event->pos());

	if (dxy.manhattanLength() < 4) {
		
		int lm, tm, rm, bm;
		getContentsMargins(&lm, &tm, &rm, &bm);
		
		// move to the current position
		QRectF viewRect = viewPortRect;
		viewRect = worldMatrix->inverted().mapRect(viewRect);
		viewRect = imgMatrix->inverted().mapRect(viewRect);
		viewRect = getScaledImageMatrix().mapRect(viewRect);
		QPointF currentViewPoint = viewRect.center();

		float panningSpeed = -(worldMatrix->m11()/(getScaledImageMatrix().m11()/imgMatrix->m11()));

		QPointF cPos = event->pos()-QPointF(lm, tm);
		QPointF dxy = (cPos - currentViewPoint)/worldMatrix->m11()*panningSpeed;
		emit moveViewSignal(dxy);

		if (event->modifiers() == DkSettings::Global::altMod)
			emit sendTransformSignal();
	}

}

void DkOverview::mouseMoveEvent(QMouseEvent *event) {

	if (event->buttons() != Qt::LeftButton)
		return;

	float panningSpeed = -(worldMatrix->m11()/(getScaledImageMatrix().m11()/imgMatrix->m11()));

	QPointF cPos = event->pos();
	QPointF dxy = (cPos - posGrab)/worldMatrix->m11()*panningSpeed;
	posGrab = cPos;
	emit moveViewSignal(dxy);

	if (event->modifiers() == DkSettings::Global::altMod)
		emit sendTransformSignal();

}

void DkOverview::resizeEvent(QResizeEvent* event) {

	QSizeF newSize = event->size();
	newSize.setHeight(newSize.width() * viewPortRect.height()/viewPortRect.width());

	// in rare cases, the window won't be resized if width = maxWidth & height is < 1
	if (newSize.height() < 1)
		newSize.setWidth(0);
	
	resize(newSize.toSize());

	DkWidget::resizeEvent(event);
}

void DkOverview::resize(int w, int h) {

	resize(QSize(w, h));
}

void DkOverview::resize(const QSize& size) {

	DkWidget::resize(size);

	// update image
	resizeImg();
}

void DkOverview::resizeImg() {

	if (img.isNull())
		return;

	int lm, tm, rm, bm;
	getContentsMargins(&lm, &tm, &rm, &bm);

	QSize viewSize = QSize(width()-lm-rm, height()-tm-bm);	// overview shall take 15% of the viewport....
	QRectF overviewRect = QRectF(QPoint(lm, tm), QSize(viewSize.width()-2, viewSize.height()-2));			// get the overview rect
	overviewRect = overviewRect.toRect();	// force round

	QTransform overviewImgMatrix = getScaledImageMatrix();			// matrix that always resizes the image to the current viewport
	
	// is the overviewImgMatrix empty?
	if (overviewImgMatrix.isIdentity())
		return;
	
	if (overviewRect.width() <= 1|| overviewRect.height() <= 1)
		return;

	// fast downscaling
	imgT = img.scaled(overviewRect.size().width()*2, overviewRect.size().height()*2, Qt::KeepAspectRatio, Qt::FastTransformation);
	imgT = imgT.scaled(overviewRect.size().width(), overviewRect.size().height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QTransform DkOverview::getScaledImageMatrix() {

	if (img.isNull())
		return QTransform();

	int lm, tm, rm, bm;
	getContentsMargins(&lm, &tm, &rm, &bm);

	QSize iSize = QSize(width()-lm-rm, height()-tm-bm);	// inner size

	if (iSize.width() < 2 || iSize.height() < 2)
		return QTransform();

	// the image resizes as we zoom
	QRectF imgRect = QRectF(QPoint(lm, tm), img.size());
	float ratioImg = imgRect.width()/imgRect.height();
	float ratioWin = (float)(iSize.width())/(float)(iSize.height());

	QTransform imgMatrix;
	float s;
	if (imgRect.width() == 0 || imgRect.height() == 0)
		s = 1.0f;
	else
		s = (ratioImg > ratioWin) ? (float)iSize.width()/imgRect.width() : (float)iSize.height()/imgRect.height();

	imgMatrix.scale(s, s);

	QRectF imgViewRect = imgMatrix.mapRect(imgRect);
	imgMatrix.translate((iSize.width()-imgViewRect.width())*0.5f/s, (iSize.height()-imgViewRect.height())*0.5f/s);

	return imgMatrix;
}


// DkLabel --------------------------------------------------------------------
DkLabel::DkLabel(QWidget* parent, const QString& text) : QLabel(text, parent) {

	bgCol = (DkSettings::App::appMode == DkSettings::mode_frameless) ?
		DkSettings::Display::bgColorFrameless :
		DkSettings::Display::bgColorWidget;

	setMouseTracking(true);
	this->parent = parent;
	this->text = text;
	init();
	hide();
}

void DkLabel::init() {

	time = -1;
	fixedWidth = -1;
	fontSize = 17;
	textCol = QColor(255, 255, 255);
	blocked = false;
	
	timer = new QTimer();
	timer->setSingleShot(true);
	connect(timer, SIGNAL(timeout()), this, SLOT(hide()));

	// default look and feel
	QFont font;
	font.setPixelSize(fontSize);
	QLabel::setFont(font);
	QLabel::setTextInteractionFlags(Qt::TextSelectableByMouse);
	
	QLabel::setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	updateStyleSheet();
}

void DkLabel::hide() {
	time = 0;
	QLabel::hide();
}

void DkLabel::setText(const QString msg, int time) {

	this->text = msg;
	this->time = time;

	if (!time || msg.isEmpty()) {
		hide();
		return;
	}

	setTextToLabel();
	show();

	if (time != -1)
		timer->start(time);

}

void DkLabel::showTimed(int time) {

	this->time = time;

	if (!time) {
		hide();
		return;
	}

	show();

	if (time != -1)
		timer->start(time);

}


QString DkLabel::getText() {
	return this->text;
}

void DkLabel::setFontSize(int fontSize) {

	this->fontSize = fontSize;

	QFont font;
	font.setPixelSize(fontSize);
	QLabel::setFont(font);
	QLabel::adjustSize();
}

void DkLabel::stop() {
	timer->stop();
	hide();
}

void DkLabel::updateStyleSheet() {
	QLabel::setStyleSheet("QLabel{color: " + textCol.name() + "; margin: " + 
		QString::number(margin.y()) + "px " +
		QString::number(margin.x()) + "px " +
		QString::number(margin.y()) + "px " +
		QString::number(margin.x()) + "px;}");
}

void DkLabel::paintEvent(QPaintEvent *event) {

	if (blocked || !time)	// guarantee that the label is hidden after the time is up
		return;

	QPainter painter(this);
	draw(&painter);
	painter.end();

	QLabel::paintEvent(event);
}

void DkLabel::draw(QPainter* painter) {

	drawBackground(painter);
}

void DkLabel::setFixedWidth(int fixedWidth) {

	this->fixedWidth = fixedWidth;
	setTextToLabel();
}

void DkLabel::setTextToLabel() {

	if (fixedWidth == -1) {
		QLabel::setText(text);
		QLabel::adjustSize();
	}
	else {
		setToolTip(text);
		QLabel::setText(fontMetrics().elidedText(text, Qt::ElideRight, fixedWidth-2*margin.x()));
		QLabel::resize(fixedWidth, height());
	}

}

DkLabelBg::DkLabelBg(QWidget* parent, const QString& text) : DkLabel(parent, text) {

	bgCol = (DkSettings::App::appMode == DkSettings::mode_frameless) ?
		DkSettings::Display::bgColorFrameless :
		DkSettings::Display::bgColorWidget;

	setAttribute(Qt::WA_TransparentForMouseEvents);	// labels should forward mouse events
	
	setObjectName("DkLabelBg");
	updateStyleSheet();

	margin = QPoint(7,2);
	setMargin(margin);
}

void DkLabelBg::updateStyleSheet() {

	QLabel::setStyleSheet("QLabel#DkLabelBg{color: " + textCol.name() + "; padding: " + 
		QString::number(margin.y()) + "px " +
		QString::number(margin.x()) + "px " +
		QString::number(margin.y()) + "px " +
		QString::number(margin.x()) + "px; " +
		"background-color: " + DkUtils::colorToString(bgCol) + ";}");	// background
}

// DkGradientLabel --------------------------------------------------------------------
DkGradientLabel::DkGradientLabel(QWidget* parent, const QString& text) : DkLabel(parent, text) {

	init();
	hide();
}

void DkGradientLabel::init() {

	DkLabel::init();
	gradient = QImage(":/nomacs/img/label-gradient.png");
	end = QImage(":/nomacs/img/label-end.png");
	
	QLabel::setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	QLabel::setStyleSheet("QLabel{color: " + textCol.name() + "; margin: 5px " + QString::number(end.width()) + "px 5px 10px}");
	
}

void DkGradientLabel::updateStyleSheet() {
	
	QLabel::setStyleSheet("QLabel{color: " + textCol.name() + "; margin: " + 
		QString::number(margin.y()) + "px " +
		QString::number(end.width()) + "px " +		// the fade-out
		QString::number(margin.y()) + "px " +
		QString::number(margin.x()) + "px;}");
}

void DkGradientLabel::drawBackground(QPainter* painter) {

	QRect textRect = QRect(QPoint(), size());
	textRect.setWidth(textRect.width()-end.width()-1);
	QRectF endRect = QRect(textRect.right()+1, 0, end.width(), geometry().height());
	painter->drawImage(textRect, gradient);
	painter->drawImage(endRect, end);
}

// DkFadeLabel --------------------------------------------------------------------
DkFadeLabel::DkFadeLabel(QWidget* parent, const QString& text) : DkLabel(parent, text) {
	init();
}

void DkFadeLabel::init() {

	bgCol = (DkSettings::App::appMode == DkSettings::mode_frameless) ?
		DkSettings::Display::bgColorFrameless :
		DkSettings::Display::bgColorWidget;

	showing = false;
	hiding = false;
	blocked = false;
	displaySettingsBits = 0;

	// widget starts on hide
	opacityEffect = new QGraphicsOpacityEffect(this);
	opacityEffect->setOpacity(0);
	opacityEffect->setEnabled(false);	// default disabled -> otherwise we get problems with children having the same effect
	setGraphicsEffect(opacityEffect);
	
	setVisible(false);
}

void DkFadeLabel::show() {

	if (!blocked && !showing) {
		hiding = false;
		showing = true;
		setVisible(true);
		animateOpacityUp();
	}
}

void DkFadeLabel::hide() {

	if (!hiding) {
		hiding = true;
		showing = false;
		animateOpacityDown();
	}
}

void DkFadeLabel::setVisible(bool visible) {

	if (blocked) {
		DkLabel::setVisible(false);
		return;
	}

	if (visible && !isVisible() && !showing)
		opacityEffect->setOpacity(100);

	emit visibleSignal(visible);
	DkLabel::setVisible(visible);

	if (displaySettingsBits && displaySettingsBits->size() > DkSettings::App::currentAppMode) {
		qDebug() << "setting visible to: " << visible;
		displaySettingsBits->setBit(DkSettings::App::currentAppMode, visible);
	}

}

void DkFadeLabel::animateOpacityUp() {

	if (!showing)
		return;

	opacityEffect->setEnabled(true);
	if (opacityEffect->opacity() >= 1.0f || !showing) {
		opacityEffect->setOpacity(1.0f);
		opacityEffect->setEnabled(false);
		showing = false;
		return;
	}

	QTimer::singleShot(20, this, SLOT(animateOpacityUp()));
	opacityEffect->setOpacity(opacityEffect->opacity()+0.05);
}

void DkFadeLabel::animateOpacityDown() {

	if (!hiding)
		return;
	
	opacityEffect->setEnabled(true);
	if (opacityEffect->opacity() <= 0.0f) {
		opacityEffect->setOpacity(0.0f);
		hiding = false;
		opacityEffect->setEnabled(false);
		setVisible(false);	// finally hide the widget
		return;
	}

	QTimer::singleShot(20, this, SLOT(animateOpacityDown()));
	opacityEffect->setOpacity(opacityEffect->opacity()-0.05);
}

// DkButton --------------------------------------------------------------------
DkButton::DkButton(QWidget* parent) : QPushButton(parent) {

}

DkButton::DkButton(const QString& text, QWidget* parent) : QPushButton(text, parent) {

}

DkButton::DkButton(const QIcon& icon, const QString& text, QWidget* parent) : QPushButton(icon, text, parent) {

	checkedIcon = icon;
	setText(text);

	init();
}

DkButton::DkButton(const QIcon& checkedIcon, const QIcon& uncheckedIcon, const QString& text, QWidget* parent) : QPushButton(checkedIcon, text, parent) {

	this->checkedIcon = checkedIcon;
	this->uncheckedIcon = uncheckedIcon;
	this->setCheckable(true);
	setText(text);
	
	init();
}

void DkButton::init() {

	setIcon(checkedIcon);

	if (!checkedIcon.availableSizes().empty())
		this->setMaximumSize(checkedIcon.availableSizes()[0]);	// crashes if the image is empty!!
	
	mouseOver = false;
	keepAspectRatio = true;
}

void DkButton::setFixedSize(QSize size) {
	mySize = size;
	this->setMaximumSize(size);
}

void DkButton::paintEvent(QPaintEvent *event) {

 	QPainter painter(this);
	
	QPoint offset;
	QSize s;
	float opacity = 1.0f;

	if (!isEnabled())
		opacity = 0.5f;
	else if(!mouseOver)
		opacity = 0.7f;

	painter.setOpacity(opacity);
	painter.setRenderHint(QPainter::SmoothPixmapTransform);

	if (!mySize.isEmpty()) {
		
		offset = QPoint((float)(size().width()-mySize.width())*0.5f, (float)(size().height()-mySize.height())*0.5f);
		s = mySize;
	}
	else
		s = this->size();

	// scale to parent label
	QRect r = (keepAspectRatio) ? QRect(offset, checkedIcon.actualSize(s)) : QRect(offset, s);	// actual size preserves the aspect ratio
	QPixmap pm2draw;

	if (isChecked() || !isCheckable())
		pm2draw = checkedIcon.pixmap(s);
	else
		pm2draw = uncheckedIcon.pixmap(s);

	if (this->isDown()) {
		QPixmap effect = createSelectedEffect(&pm2draw);
		painter.drawPixmap(r, effect);
	}

	painter.drawPixmap(r, pm2draw);
	painter.end();
}

QPixmap DkButton::createSelectedEffect(QPixmap* pm) {
	
	QPixmap imgPx = pm->copy();
	QPixmap imgAlpha = imgPx;
	imgAlpha.fill(DkSettings::Display::highlightColor);
	imgAlpha.setAlphaChannel(imgPx.alphaChannel());

	return imgAlpha;
}

void DkButton::focusInEvent(QFocusEvent * event) {
	mouseOver = true;
}

void DkButton::focusOutEvent(QFocusEvent * event) {
	mouseOver = false;
}

void DkButton::enterEvent(QEvent *event) {
	mouseOver = true;
}

void DkButton::leaveEvent(QEvent *event) {
	mouseOver = false;
}

// star label --------------------------------------------------------------------
DkRatingLabel::DkRatingLabel(int rating, QWidget* parent, Qt::WindowFlags flags) : DkWidget(parent, flags) {

	setObjectName("DkRatingLabel");
	this->rating = rating;
	init();

	int iconSize = 16;
	int lastStarRight = 0;
	int timeToDisplay = 3000;

	layout = new QBoxLayout(QBoxLayout::LeftToRight);
	layout->setContentsMargins(0,0,0,0);
	layout->setSpacing(3);
	layout->addStretch();
	
	for (int idx = 0; idx < stars.size(); idx++) {
		stars[idx]->setFixedSize(QSize(iconSize, iconSize));
		layout->addWidget(stars[idx]);
	}
	
	this->setLayout(layout);
}

void DkRatingLabel::init() {

	QPixmap starDark = QPixmap(":/nomacs/img/star-dark.png");
	QPixmap starWhite = QPixmap(":/nomacs/img/star-white.png");

	stars.resize(5);
	
	stars[rating_1] = new DkButton(starWhite, starDark, tr("one star"), this);
	stars[rating_1]->setCheckable(true);
	connect(stars[rating_1], SIGNAL(released()), this, SLOT(rating1()));

	stars[rating_2] = new DkButton(starWhite, starDark, tr("two stars"), this);
	stars[rating_2]->setCheckable(true);
	connect(stars[rating_2], SIGNAL(released()), this, SLOT(rating2()));

	stars[rating_3] = new DkButton(starWhite, starDark, tr("three star"), this);
	stars[rating_3]->setCheckable(true);
	connect(stars[rating_3], SIGNAL(released()), this, SLOT(rating3()));

	stars[rating_4] = new DkButton(starWhite, starDark, tr("four star"), this);
	stars[rating_4]->setCheckable(true);
	connect(stars[rating_4], SIGNAL(released()), this, SLOT(rating4()));

	stars[rating_5] = new DkButton(starWhite, starDark, tr("five star"), this);
	stars[rating_5]->setCheckable(true);
	connect(stars[rating_5], SIGNAL(released()), this, SLOT(rating5()));

}

DkRatingLabelBg::DkRatingLabelBg(int rating, QWidget* parent, Qt::WindowFlags flags) : DkRatingLabel(rating, parent, flags) {

	timeToDisplay = 4000;
	hideTimer = new QTimer(this);
	hideTimer->setInterval(timeToDisplay);
	hideTimer->setSingleShot(true);

	// we want a margin
	layout->setContentsMargins(10,4,10,4);
	layout->setSpacing(4);

	actions.resize(6);

	actions[rating_0] = new QAction(tr("no rating"), this);
	actions[rating_0]->setShortcut(Qt::Key_0);
	connect(actions[rating_0], SIGNAL(triggered()), this, SLOT(rating0()));

	actions[rating_1] = new QAction(tr("one star"), this);
	actions[rating_1]->setShortcut(Qt::Key_1);
	connect(actions[rating_1], SIGNAL(triggered()), this, SLOT(rating1()));

	actions[rating_2] = new QAction(tr("two stars"), this);
	actions[rating_2]->setShortcut(Qt::Key_2);
	connect(actions[rating_2], SIGNAL(triggered()), this, SLOT(rating2()));

	actions[rating_3] = new QAction(tr("three stars"), this);
	actions[rating_3]->setShortcut(Qt::Key_3);
	connect(actions[rating_3], SIGNAL(triggered()), this, SLOT(rating3()));

	actions[rating_4] = new QAction(tr("four stars"), this);
	actions[rating_4]->setShortcut(Qt::Key_4);
	connect(actions[rating_4], SIGNAL(triggered()), this, SLOT(rating4()));

	actions[rating_5] = new QAction(tr("five stars"), this);
	actions[rating_5]->setShortcut(Qt::Key_5);
	connect(actions[rating_5], SIGNAL(triggered()), this, SLOT(rating5()));

	stars[rating_1]->addAction(actions[rating_1]);
	stars[rating_2]->addAction(actions[rating_2]);
	stars[rating_3]->addAction(actions[rating_3]);
	stars[rating_4]->addAction(actions[rating_4]);
	stars[rating_5]->addAction(actions[rating_5]);
	
	connect(hideTimer, SIGNAL(timeout()), this, SLOT(hide()));

}

void DkRatingLabelBg::paintEvent(QPaintEvent *event) {

	QPainter painter(this);
	painter.fillRect(QRect(QPoint(), this->size()), bgCol);
	painter.end();

	DkRatingLabel::paintEvent(event);
}

// title info --------------------------------------------------------------------
DkFileInfoLabel::DkFileInfoLabel(QWidget* parent) : DkFadeLabel(parent) {

	setObjectName("DkFileInfoLabel");
	setStyleSheet("QLabel#DkFileInfoLabel{background-color: " + DkUtils::colorToString(bgCol) + ";} QLabel{color: white;}");
	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

	minWidth = 110;
	this->parent = parent;
	title = new QLabel(this);
	title->setMouseTracking(true);
	title->setTextInteractionFlags(Qt::TextSelectableByMouse);
	date = new QLabel(this);
	date->setMouseTracking(true);
	date->setTextInteractionFlags(Qt::TextSelectableByMouse);
	rating = new DkRatingLabel(0, this);
	setMinimumWidth(minWidth);
	
	createLayout();
}

void DkFileInfoLabel::createLayout() {

	layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	layout->setSpacing(2);

	layout->addWidget(title);
	layout->addWidget(date);
	layout->addWidget(rating);
	//layout->addStretch();
}

void DkFileInfoLabel::setVisible(bool visible) {

	// nothing to display??
	if (!DkSettings::SlideShow::display.testBit(DkDisplaySettingsWidget::display_file_name) &&
		!DkSettings::SlideShow::display.testBit(DkDisplaySettingsWidget::display_creation_date) &&
		!DkSettings::SlideShow::display.testBit(DkDisplaySettingsWidget::display_file_rating) && visible) {
			
			QMessageBox infoDialog(parent);
			infoDialog.setWindowTitle(tr("Info Box"));
			infoDialog.setText(tr("All information fields are currently hidden.\nDo you want to show them again?"));
			infoDialog.setIcon(QMessageBox::Information);
			infoDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			infoDialog.setDefaultButton(QMessageBox::Yes);
			infoDialog.show();
			int choice = infoDialog.exec();

			if (choice == QMessageBox::No) {
				DkFadeLabel::setVisible(false);
				return;
			}
			else {
				DkSettings::SlideShow::display.setBit(DkDisplaySettingsWidget::display_file_name, true);
				DkSettings::SlideShow::display.setBit(DkDisplaySettingsWidget::display_creation_date, true);
				DkSettings::SlideShow::display.setBit(DkDisplaySettingsWidget::display_file_rating, true);
			}
	}

	DkFadeLabel::setVisible(visible);
	title->setVisible(DkSettings::SlideShow::display.testBit(DkDisplaySettingsWidget::display_file_name));
	date->setVisible(DkSettings::SlideShow::display.testBit(DkDisplaySettingsWidget::display_creation_date));
	rating->setVisible(DkSettings::SlideShow::display.testBit(DkDisplaySettingsWidget::display_file_rating));

	int height = 32;
	if (title->isVisible())
		height += title->sizeHint().height();
	if (date->isVisible())
		height += date->sizeHint().height();
	if (rating->isVisible())
		height += rating->sizeHint().height();

	qDebug() << "my minimum height: " << height;
	setMinimumHeight(height);
	updateWidth();
}

void DkFileInfoLabel::setEdited(bool edited) {

	if (!isVisible() || !edited)
		return;

	QString cFileName = title->text() + "*";
	this->title->setText(cFileName);

}

DkRatingLabel* DkFileInfoLabel::getRatingLabel() {
	return rating;
}

void DkFileInfoLabel::updateInfo(const QFileInfo& file, const QString& date, const int rating) {

	updateTitle(file);
	updateDate(date);
	updateRating(rating);

	updateWidth();
}

void DkFileInfoLabel::updateTitle(const QFileInfo& file) {
	
	this->file = file;
	updateDate();
	this->title->setText(file.fileName());
	this->title->setAlignment(Qt::AlignRight);

	updateWidth();
}

void DkFileInfoLabel::updateDate(const QString& date) {

	QString dateConverted = DkUtils::convertDate(date, file);

	this->date->setText(dateConverted);
	this->date->setAlignment(Qt::AlignRight);

	updateWidth();
}

void DkFileInfoLabel::updateRating(const int rating) {
	
	this->rating->setRating(rating);

}

void DkFileInfoLabel::updateWidth() {

	int width = 20;		// mar
	width += qMax(qMax(title->sizeHint().width(), date->sizeHint().width()), rating->sizeHint().width());
	
	if (width < minimumWidth())
		setMinimumWidth(width);
	
	setMaximumWidth(width);
}

// player --------------------------------------------------------------------
DkPlayer::DkPlayer(QWidget* parent) : DkWidget(parent) {

	init();
}

void DkPlayer::init() {
	
	setObjectName("DkPlayer");

	// slide show
	int timeToDisplayPlayer = 3000;
	timeToDisplay = DkSettings::SlideShow::time*1000;
	playing = false;
	displayTimer = new QTimer(this);
	displayTimer->setInterval(timeToDisplay);
	displayTimer->setSingleShot(true);
	connect(displayTimer, SIGNAL(timeout()), this, SLOT(autoNext()));

	hideTimer = new QTimer(this);
	hideTimer->setInterval(timeToDisplayPlayer);
	hideTimer->setSingleShot(true);
	connect(hideTimer, SIGNAL(timeout()), this, SLOT(hide()));

	actions.resize(1);
	actions[play_action] = new QAction(tr("play"), this);
	actions[play_action]->setShortcut(Qt::Key_Space);
	connect(actions[play_action], SIGNAL(triggered()), this, SLOT(togglePlay()));

	QPixmap icon = QPixmap(":/nomacs/img/player-back.png");
	previousButton = new DkButton(icon, tr("previous"), this);
	previousButton->keepAspectRatio = false;
	connect(previousButton, SIGNAL(pressed()), this, SLOT(previous()));

	icon = QPixmap(":/nomacs/img/print.png");
	playButton = new DkButton(icon, tr("play"), this);
	playButton->keepAspectRatio = false;
	playButton->setChecked(false);
	playButton->addAction(actions[play_action]);
	connect(playButton, SIGNAL(toggled(bool)), this, SLOT(play(bool)));

	icon = QPixmap(":/nomacs/img/player-next.png");
	nextButton = new DkButton(icon, tr("next"), this);
	nextButton->keepAspectRatio = false;
	connect(nextButton, SIGNAL(pressed()), this, SLOT(next()));

	// now add to layout
	container = new QWidget(this);
	QHBoxLayout *layout = new QHBoxLayout(container);
	layout->setContentsMargins(0,0,0,0);
	layout->setSpacing(0);
	layout->addWidget(previousButton);
	layout->addWidget(playButton);
	layout->addWidget(nextButton);


	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	setMinimumSize(15, 5);
	setMaximumSize(304, 118);
}

void DkPlayer::resizeEvent(QResizeEvent *event) {

	if (event->oldSize() == event->size())
		return;

	// always preserve the player's aspect ratio
	QSizeF s = event->size();
	QSizeF ms = maximumSize();
	float aRatio = s.width()/s.height();
	float amRatio = ms.width()/ms.height();
	
	if (aRatio != amRatio && s.width() / amRatio <= s.height()) {
		s.setHeight(s.width() / amRatio);

		QRect r = QRect(QPoint(), s.toSize());
		r.moveBottom(event->size().height()-1);
		r.moveCenter(QPoint(qRound((float)event->size().width()/2.0f), r.center().y()));
		container->setGeometry(r);
	}
	else {
		s.setWidth(s.height() * amRatio);

		QRect r = QRect(QPoint(), s.toSize());
		r.moveBottom(event->size().height()-1);
		r.moveCenter(QPoint(qRound((float)event->size().width()/2.0f), r.center().y()));
		container->setGeometry(r);
	}

	QWidget::resizeEvent(event);
}

void DkPlayer::setTimeToDisplay(int ms) {

	timeToDisplay = ms;
	displayTimer->setInterval(ms);
}

void DkPlayer::show(int ms) {		
	
	if (ms > 0 && !hideTimer->isActive()) {
		hideTimer->setInterval(ms);
		hideTimer->start();
	}

	bool showPlayer = getCurrentDisplaySetting();

	DkWidget::show();

	// automatic showing, don't store it in the display bits
	if (ms > 0 && displaySettingsBits && displaySettingsBits->size() > DkSettings::App::currentAppMode) {
		displaySettingsBits->setBit(DkSettings::App::currentAppMode, showPlayer);
	}
}
 
// DkMetaDataInfo ------------------------------------------------------------------

////enums for tags divided in exif and iptc
//static enum exifT {
//	exif_width,
//	exif_length,
//	exif_orientation,
//	exif_make,
//	exif_model,
//	exif_rating,
//	exif_aperture,
//	exif_shutterspeed,
//	exif_flash,
//	exif_focallength,
//	exif_exposuredmode,
//	exif_exposuretime,
//	exif_usercomment,
//	exif_datetime,
//	exif_datetimeoriginal,
//	exif_description,
//
//	exif_end
//};
//
//static enum iptcT {
//	iptc_creator = exif_end,
//	iptc_creatortitle,
//	iptc_city,
//	iptc_country,
//	iptc_headline,
//	iptc_caption,
//	iptc_copyright,
//	iptc_keywords,
//
//	iptc_end
//};

//QString DkMetaDataInfo::sExifTags = QString("ImageWidth ImageLength Orientation Make Model Rating ApertureValue ShutterSpeedValue Flash FocalLength ") %
//	QString("ExposureMode ExposureTime UserComment DateTime DateTimeOriginal ImageDescription");
//QString DkMetaDataInfo::sExifDesc = QString("Image Width;Image Length;Orientation;Make;Model;Rating;Aperture Value;Shutter Speed Value;Flash;FocalLength;") %
//	QString("Exposure Mode;Exposure Time;User Comment;Date Time;Date Time Original;Image Description");
//QString DkMetaDataInfo::sIptcTags = QString("Iptc.Application2.Byline Iptc.Application2.BylineTitle Iptc.Application2.City Iptc.Application2.Country ") %
//	QString("Iptc.Application2.Headline Iptc.Application2.Caption Iptc.Application2.Copyright Iptc.Application2.Keywords");
//QString DkMetaDataInfo::sIptcDesc = QString("Creator;Creator Title;City;Country;Headline;Caption;Copyright;Keywords");

QString DkMetaDataInfo::sCamDataTags = QString("ImageSize Orientation Make Model ApertureValue ISOSpeedRatings Flash FocalLength ") %
	QString("ExposureMode ExposureTime");

QString DkMetaDataInfo::sDescriptionTags = QString("Rating UserComment DateTime DateTimeOriginal ImageDescription Byline BylineTitle City Country ") %
	QString("Headline Caption CopyRight Keywords Path FileSize");




DkMetaDataInfo::DkMetaDataInfo(QWidget* parent) : DkWidget(parent) {
	
	setObjectName("DkMetaDataInfo");

	this->parent = parent;
	
	exifHeight = 120;
	minWidth = 900;
	fontSize = 12;
	textMargin = 10;
	numLines = 6;
	maxCols = 4;
	numLabels = 0;
	gradientWidth = 100;

	yMargin = 6;
	xMargin = 8;

	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
}

void DkMetaDataInfo::init() {

	mapIptcExif[DkMetaDataSettingsWidget::camData_size] = 0;
	mapIptcExif[DkMetaDataSettingsWidget::camData_orientation] = 0;
	mapIptcExif[DkMetaDataSettingsWidget::camData_make] = 0;
	mapIptcExif[DkMetaDataSettingsWidget::camData_model] = 0;
	mapIptcExif[DkMetaDataSettingsWidget::camData_aperture] = 0;
	//mapIptcExif[DkMetaDataSettingsWidget::camData_shutterspeed] = 0;
	mapIptcExif[DkMetaDataSettingsWidget::camData_flash] = 0;
	mapIptcExif[DkMetaDataSettingsWidget::camData_focallength] = 0;
	mapIptcExif[DkMetaDataSettingsWidget::camData_exposuremode] = 0;
	mapIptcExif[DkMetaDataSettingsWidget::camData_exposuretime] = 0;

	mapIptcExif[DkMetaDataSettingsWidget::desc_rating] = 0;
	mapIptcExif[DkMetaDataSettingsWidget::desc_usercomment] = 0;
	mapIptcExif[DkMetaDataSettingsWidget::desc_date] = 0;
	mapIptcExif[DkMetaDataSettingsWidget::desc_datetimeoriginal] = 0;
	mapIptcExif[DkMetaDataSettingsWidget::desc_imagedescription] = 0;
	mapIptcExif[DkMetaDataSettingsWidget::desc_creator] = 1;
	mapIptcExif[DkMetaDataSettingsWidget::desc_creatortitle] = 1;
	mapIptcExif[DkMetaDataSettingsWidget::desc_city] = 1;
	mapIptcExif[DkMetaDataSettingsWidget::desc_country] = 1;
	mapIptcExif[DkMetaDataSettingsWidget::desc_headline] = 1;
	mapIptcExif[DkMetaDataSettingsWidget::desc_caption] = 1;
	mapIptcExif[DkMetaDataSettingsWidget::desc_copyright] = 1;
	mapIptcExif[DkMetaDataSettingsWidget::desc_keywords] = 1;

	mapIptcExif[DkMetaDataSettingsWidget::desc_path] = 2;
	mapIptcExif[DkMetaDataSettingsWidget::desc_filesize] = 2;

	for (int i = 0; i  < DkMetaDataSettingsWidget::scamDataDesc.size(); i++) 
		camDTags << qApp->translate("nmc::DkMetaData", DkMetaDataSettingsWidget::scamDataDesc.at(i).toAscii());

	for (int i = 0; i  < DkMetaDataSettingsWidget::sdescriptionDesc.size(); i++)
		descTags << qApp->translate("nmc::DkMetaData", DkMetaDataSettingsWidget::sdescriptionDesc.at(i).toAscii());


	exposureModes.append(tr("not defined"));
	exposureModes.append(tr("manual"));
	exposureModes.append(tr("normal"));
	exposureModes.append(tr("aperture priority"));
	exposureModes.append(tr("shutter priority"));
	exposureModes.append(tr("program creative"));
	exposureModes.append(tr("high-speed program"));
	exposureModes.append(tr("portrait mode"));
	exposureModes.append(tr("landscape mode"));

	// flash mapping is taken from: http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/EXIF.html#Flash
	flashModes.insert(0x0, tr("No Flash"));
	flashModes.insert(0x1, tr("Fired"));
	flashModes.insert(0x5, tr("Fired, Return not detected"));
	flashModes.insert(0x7, tr("Fired, Return detected"));
	flashModes.insert(0x8, tr("On, Did not fire"));
	flashModes.insert(0x9, tr("On, Fired"));
	flashModes.insert(0xd, tr("On, Return not detected"));
	flashModes.insert(0xf, tr("On, Return detected"));
	flashModes.insert(0x10, tr("Off, Did not fire"));
	flashModes.insert(0x14, tr("Off, Did not fire, Return not detected"));
	flashModes.insert(0x18, tr("Auto, Did not fire"));
	flashModes.insert(0x19, tr("Auto, Fired"));
	flashModes.insert(0x1d, tr("Auto, Fired, Return not detected"));
	flashModes.insert(0x1f, tr("Auto, Fired, Return detected"));
	flashModes.insert(0x20, tr("No flash function"));
	flashModes.insert(0x30, tr("Off, No flash function"));
	flashModes.insert(0x41, tr("Fired, Red-eye reduction"));
	flashModes.insert(0x45, tr("Fired, Red-eye reduction, Return not detected"));
	flashModes.insert(0x47, tr("Fired, Red-eye reduction, Return detected"));
	flashModes.insert(0x49, tr("On, Red-eye reduction"));
	flashModes.insert(0x4d, tr("On, Red-eye reduction, Return not detected"));
	flashModes.insert(0x4f, tr("On, Red-eye reduction, Return detected"));
	flashModes.insert(0x50, tr("Off, Red-eye reduction"));
	flashModes.insert(0x58, tr("Auto, Did not fire, Red-eye reduction"));
	flashModes.insert(0x59, tr("Auto, Fired, Red-eye reduction"));
	flashModes.insert(0x5d, tr("Auto, Fired, Red-eye reduction, Return not detected"));
	flashModes.insert(0x5f, tr("Auto, Fired, Red-eye reduction, Return detected"));

	worldMatrix = QTransform();

	if (camDTags.size() != DkMetaDataSettingsWidget::camData_end)
		qDebug() << "wrong definition of Camera Data (Exif). Size of CamData tags is different from enum";
	if (descTags.size() != DkMetaDataSettingsWidget::desc_end - DkMetaDataSettingsWidget::camData_end)
		qDebug() << "wrong definition of Description Data (Exif). Size of Descriptions tags is different from enum";

	setMouseTracking(true);
	//readTags();

	QColor tmpCol = bgCol;
	tmpCol.setAlpha(0);

	leftGradientRect = QRect(QPoint(), QSize(gradientWidth, size().height()));
	leftGradient = QLinearGradient(leftGradientRect.topLeft(), leftGradientRect.topRight());
	leftGradient.setColorAt(0, tmpCol);
	leftGradient.setColorAt(1, bgCol);

	rightGradientRect = QRect(QPoint(size().width()-gradientWidth, 0), QSize(gradientWidth, size().height()));
	rightGradient = QLinearGradient(rightGradientRect.topLeft(), rightGradientRect.topRight());
	rightGradient.setColorAt(0, bgCol);
	rightGradient.setColorAt(1, tmpCol);

}

void DkMetaDataInfo::getResolution(float &xResolution, float &yResolution) {
	float xR, yR;
	QString xRes, yRes;
	xR = 72.0f;
	yR = 72.0f;

	try {

		if (!file.absoluteFilePath().isEmpty()) {
			//metaData = DkImageLoader::imgMetaData;
			xRes = QString(DkImageLoader::imgMetaData.getExifValue("XResolution").c_str());
			QStringList res;
			res = xRes.split("/");
			if (res.size() != 2) {
				throw DkException("no x resolution found\n");
			}
			xR = res.at(1).toFloat() != 0 ? res.at(0).toFloat()/res.at(1).toFloat() : 72;

			yRes = QString(DkImageLoader::imgMetaData.getExifValue("YResolution").c_str());
			res = yRes.split("/");

			qDebug() << "Resolution"  << xRes << " " << yRes;
			if (res.size() != 2)
				throw DkException("no y resolution found\n");
			yR = res.at(1).toFloat() != 0 ? res.at(0).toFloat()/res.at(1).toFloat() : 72;
		}
	} catch (...) {
		qDebug() << "could not load Exif resolution, set to 72dpi";
		xR = 72;
		yR = 72;
	}

	xResolution = xR;
	yResolution = yR;
	
}

QString DkMetaDataInfo::getGPSCoordinates() {
	
	QString Lat, LatRef, Lon, LonRef, gpsInfo;
	QStringList help;
	
	try {

		////gps test
		//Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open("H:\\img\\exif\\gps.jpg");
		////////assert (image.get() != 0);
		//image->readMetadata();
		//Exiv2::ExifData &exifData = image->exifData();
		//
		//if (exifData.empty()) {
		//	printf("empty exif data\n");
		//}
		//Exiv2::ExifData::const_iterator end = exifData.end();
		//for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i) {
		//	const char* tn = i->typeName();
		//	std::cout << std::setw(44) << std::setfill(' ') << std::left
		//		<< i->key() << " "
		//		<< "0x" << std::setw(4) << std::setfill('0') << std::right
		//		<< std::hex << i->tag() << " "
		//		<< std::setw(9) << std::setfill(' ') << std::left
		//		<< (tn ? tn : "Unknown") << " "
		//		<< std::dec << std::setw(3)
		//		<< std::setfill(' ') << std::right
		//		<< i->count() << "  "
		//		<< std::dec << i->value()
		//		<< "\n";
		//}
		////gps test ends...

		if (!file.absoluteFilePath().isEmpty()) {
			//metaData = DkImageLoader::imgMetaData;
			Lat = QString(DkImageLoader::imgMetaData.getNativeExifValue("Exif.GPSInfo.GPSLatitude").c_str());
			LatRef = QString(DkImageLoader::imgMetaData.getNativeExifValue("Exif.GPSInfo.GPSLatitudeRef").c_str());
			Lon = QString(DkImageLoader::imgMetaData.getNativeExifValue("Exif.GPSInfo.GPSLongitude").c_str());
			LonRef = QString(DkImageLoader::imgMetaData.getNativeExifValue("Exif.GPSInfo.GPSLongitudeRef").c_str());
			//example url
			//http://maps.google.at/maps?q=N+48°+8'+31.940001''+E16°+15'+35.009998''

			gpsInfo = "http://maps.google.at/maps?q=" + LatRef + "+";

			help = Lat.split(" ");
			for (int i=0; i<help.size(); ++i) {
				float val1, val2;
				QString valS;
				QStringList coordP;
				valS = help.at(i);
				coordP = valS.split("/");
				if (coordP.size() != 2)
					throw DkException(tr("could not parse GPS Data").toStdString());

				val1 = coordP.at(0).toFloat();
				val2 = coordP.at(1).toFloat();
				val1 = val2 != 0 ? val1/val2 : val1;
				
				if (i==0) {
					valS.setNum((int)val1);
					gpsInfo += valS + "°";
				}
				if (i==1) {
					if (val2 > 1)							
						valS.setNum(val1, 'f', 6);
					else
						valS.setNum((int)val1);
					gpsInfo += "+" + valS + "'";
				}
				if (i==2) {
					if (val1 != 0) {
						valS.setNum(val1, 'f', 6);
						gpsInfo += "+" + valS + "''";
					}
				}
			}

			gpsInfo += "+" + LonRef;
			help = Lon.split(" ");
			for (int i=0; i<help.size(); ++i) {
				float val1, val2;
				QString valS;
				QStringList coordP;
				valS = help.at(i);
				coordP = valS.split("/");
				if (coordP.size() != 2)
					throw DkException(tr("could not parse GPS Data").toStdString());

				val1 = coordP.at(0).toFloat();
				val2 = coordP.at(1).toFloat();
				val1 = val2 != 0 ? val1/val2 : val1;

				if (i==0) {
					valS.setNum((int)val1);
					gpsInfo += valS + "°";
					//gpsInfo += valS + QString::fromUtf16((ushort*)"0xb0");//QChar('°');
					//gpsInfo += valS + QString::setUnicode("0xb0");//QChar('°');
				}
				if (i==1) {
					if (val2 > 1)							
						valS.setNum(val1, 'f', 6);
					else
						valS.setNum((int)val1);
					gpsInfo += "+" + valS + "'";
				}
				if (i==2) {
					if (val1 != 0) {
						valS.setNum(val1, 'f', 6);
						gpsInfo += "+" + valS + "''";
					}
				}
			}

		}

	} catch (...) {
		gpsInfo = "";
		//qDebug() << "could not load Exif GPS information";
	}

	return gpsInfo;
}

void DkMetaDataInfo::readTags() {

	try {
		if (mapIptcExif.empty())
			init();

		camDValues.clear();
		descValues.clear();

		QStringList camSearchTags = sCamDataTags.split(" ");
		QStringList descSearchTags = sDescriptionTags.split(" ");

		//QString preExifI = "Exif.Image.";
		//QString preExifP = "Exif.Photo.";
		QString preIptc = "Iptc.Application2.";
		
		if (!file.absoluteFilePath().isEmpty()) {
			//metaData = DkImageLoader::imgMetaData;

			for (int i=0; i<camSearchTags.size(); i++) {
				QString tmp, Value;

				if (mapIptcExif[i] == 0) {
					
					//tmp = preExifI + camDTags.at(i);
					tmp = camSearchTags.at(i);
					
					//special treatments
					// aperture
					if (i == DkMetaDataSettingsWidget::camData_aperture) {
						
						QString aValue = QString::fromStdString(DkImageLoader::imgMetaData.getExifValue(tmp.toStdString()));

						qDebug() << aValue;
						if (aValue.isEmpty()) qDebug() << "trying the fNumber";

						if (aValue.isEmpty()) QString::fromStdString(DkImageLoader::imgMetaData.getExifValue("FNumber"));

						QStringList sList = aValue.split('/');

						if (sList.size() == 2) {
							double val = std::pow(1.4142, sList[0].toDouble()/sList[1].toDouble());	// see the exif documentation (e.g. http://www.media.mit.edu/pia/Research/deepview/exif.html)
							Value = QString::fromStdString(DkUtils::stringify(val,1));
						}
						else
							Value = aValue;

					}
					// focal length
					else if (i == DkMetaDataSettingsWidget::camData_focallength) {

						QString aValue = QString::fromStdString(DkImageLoader::imgMetaData.getExifValue(tmp.toStdString()));
						QStringList sList = aValue.split('/');

						if (sList.size() == 2) {
							double val = sList[0].toDouble()/sList[1].toDouble();
							Value = QString::fromStdString(DkUtils::stringify(val,1)) + " mm";
						}
						else
							Value = aValue;

					}
					// exposure time
					else if (i == DkMetaDataSettingsWidget::camData_exposuretime) {

						QString aValue = QString::fromStdString(DkImageLoader::imgMetaData.getExifValue(tmp.toStdString()));
						QStringList sList = aValue.split('/');

						if (sList.size() == 2) {
							int nom = sList[0].toInt();		// nominator
							int denom = sList[1].toInt();	// denominator

							// if exposure time is less than a second -> compute the gcd for nice values (1/500 instead of 2/1000)
							if (nom <= denom) {
								int gcd = DkMath::gcd(denom, nom);
								Value = QString::number(nom/gcd) % QString("/") % QString::number(denom/gcd);
							}
							else
								Value = QString::fromStdString(DkUtils::stringify((float)nom/(float)denom,1));

							Value += " sec";
						}
						else
							Value = aValue;

					}
					else if (i == DkMetaDataSettingsWidget::camData_size) {	
						Value = QString::number(imgSize.width()) + " x " + QString::number(imgSize.height());
					}
					else if (i == DkMetaDataSettingsWidget::camData_exposuremode) {
						//qDebug() << "exposure mode was found";
						Value = QString::fromStdString(DkImageLoader::imgMetaData.getExifValue(tmp.toStdString()));
						int mode = Value.toInt();

						if (mode >= 0 && mode < exposureModes.size())
							Value = exposureModes[mode];
						
					} 
					else if (i == DkMetaDataSettingsWidget::camData_flash) {

						Value = QString::fromStdString(DkImageLoader::imgMetaData.getExifValue(tmp.toStdString()));
						unsigned int mode = Value.toUInt();
						Value = flashModes[mode];
					}
					else {
						//qDebug() << "size" << imgSize.width() << imgSize.height();
						Value = QString(DkImageLoader::imgMetaData.getExifValue(tmp.toStdString()).c_str());
					}
				} else if (mapIptcExif[i] == 1) {
					tmp = preIptc + camSearchTags.at(i);
					Value = QString(DkImageLoader::imgMetaData.getIptcValue(tmp.toStdString()).c_str());
				}

				camDValues << Value;
			}
			//use getRating for Rating Value... otherwise the value is probably not correct: also Xmp.xmp.Rating, Xmp.MicrosoftPhoto.Rating is used
			QString rating;
			float tmp = DkImageLoader::imgMetaData.getRating();
			if (tmp < 0) tmp=0;
			rating.setNum(tmp);
			descValues << rating;

		
			for (int i=1; i<descSearchTags.size(); i++) {
				QString tmp, Value;

				if (mapIptcExif[DkMetaDataSettingsWidget::camData_end + i] == 0) {
					//tmp = preExifI + camDTags.at(i);
					tmp = descSearchTags.at(i);
					//qDebug() << tmp;
					Value = QString(DkImageLoader::imgMetaData.getExifValue(tmp.toStdString()).c_str());

					if (tmp.contains("Date")) {
						
						Value = DkUtils::convertDate(Value, file);
					}


				} else if (mapIptcExif[DkMetaDataSettingsWidget::camData_end + i] == 1) {
					tmp = preIptc + descSearchTags.at(i);
					Value = QString(DkImageLoader::imgMetaData.getIptcValue(tmp.toStdString()).c_str());
				} else if (mapIptcExif[DkMetaDataSettingsWidget::camData_end + i] == 2) {
					//all other defined tags not in metadata
					tmp = descSearchTags.at(i);
					if (!tmp.compare("Path"))
						Value = QString(file.absoluteFilePath());
					else if (!tmp.compare("FileSize")) {
						Value = DkUtils::readableByte(file.size());
					} else
						Value = QString();
					
					//qDebug() << Value << "should be filepath";
				}
				descValues << Value;
			}
		} else
			qDebug() << "Exif: file is not defined...";

	} catch (...) {
		qDebug() << "could not load Exif information";
	}
}

void DkMetaDataInfo::createLabels() {


	if (camDValues.empty() && descValues.empty()) {
		qDebug() << "no labels read (Exif)";
		return;
	}

	if (camDValues.size() != camDTags.size() || descValues.size() != descTags.size()) {
		qDebug() << "error reading metadata: tag number is not equal value number";
		return;
	}

	for (int idx = 0; idx < pLabels.size(); idx++) {
		delete pLabels.at(idx);
	}
	
	for (int idx = 0; idx < pValues.size(); idx++) {
		delete pValues.at(idx);
	}

	pLabels.clear();
	pValues.clear();

	//int commentWidth;

	//pLabels.resize(camDTags.size() + descTags.size());
	//6 Lines...
	maxLenLabel.resize(numLines);
	for (int i=0; i<numLines; i++)
		maxLenLabel[i] = 0;

	numLabels=0;

	for(int i=0; i<camDTags.size(); i++) {
		//if bit set, create Label
		if (DkSettings::MetaData::metaDataBits.testBit(i)) {
			DkLabel* pl = new DkLabel(this);//, camDTags.at(i));
			pl->setText(camDTags.at(i)+":",-1);
			pl->setFontSize(fontSize);
			pl->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
			pl->setMouseTracking(true);
			DkLabel* pv = new DkLabel(this);//, camDValues.at(i));
			pv->setText(camDValues.at(i),-1);
			pv->setFontSize(fontSize);
			pv->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
			pv->setMouseTracking(true);
			pLabels << pl;
			pValues << pv; 
			if (pl->geometry().width() > maxLenLabel[numLabels/numLines]) maxLenLabel[numLabels/numLines] = pl->geometry().width();
			numLabels++;
		}
	}

	for(int i=0; i<descTags.size(); i++) {
		//if bit set, create Label
		if (DkSettings::MetaData::metaDataBits.testBit(DkMetaDataSettingsWidget::camData_end + i)) {
			DkLabel* pl = new DkLabel(this);
			pl->setText(descTags.at(i)+":",-1);
			pl->setFontSize(fontSize);
			pl->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
			pl->setMouseTracking(true);
			DkLabel* pv = new DkLabel(this);
			pv->setText(descValues.at(i),-1);
			pv->setFontSize(fontSize);
			pv->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
			pv->setMouseTracking(true);
			pLabels << pl;
			pValues << pv;
			if (pl->geometry().width() > maxLenLabel[numLabels/numLines]) maxLenLabel[numLabels/numLines] = pl->geometry().width();
			numLabels++;
		}
	}


	//qDebug() << camDTags;
	//qDebug() << camDValues;

	layoutLabels();
}

void DkMetaDataInfo::layoutLabels() {

	if (pLabels.isEmpty())
		return;

	// #Labels / numLines = #Spalten
	int cols = (numLabels+numLines-1)/numLines > 0 ? (numLabels+numLines-1)/numLines : 1;

	//qDebug() << "numCols: " << cols;

	if (cols > maxCols)
		qDebug() << "Labels are skipped...";

	if (cols == 1) {
		exifHeight = (pLabels.at(0)->height() + yMargin)*numLabels + yMargin;
	} else exifHeight = 120;

	//widget size
	if (parent->width() < minWidth)
		QWidget::setCursor(Qt::OpenHandCursor);
	else
		QWidget::unsetCursor();

	int width;
	//if (widthParent)
	//	width = widthParent > minWidth ? widthParent : minWidth;
	//else
	width = parent->width() > minWidth ? parent->width() : minWidth;

	//qDebug() << "width" << parent->width();

	//set Geometry if exif height is changed
	setGeometry(0, parent->height()-exifHeight, parent->width(), exifHeight);

	//subtract label length
	for (int i=0; i<cols; i++) width -= (maxLenLabel[i] + xMargin);
	width-=xMargin;

	//rest length/#cols = column width for tags
	int widthValues = width/cols > 0 ? width/cols : 10;

	//subtract margin
	for (int i=0; i< cols; i++) widthValues -= xMargin;

	
	QPoint pos(xMargin + maxLenLabel[0], yMargin);
	int textHeight = fontSize;

	//set positions for labels
	for (int i=0; i<pLabels.size(); i++) {

		if (i%numLines == 0 && i>0) {
			pos = QPoint(pos.x() + maxLenLabel[i/numLines] + widthValues + xMargin*2, yMargin);

		}

		QPoint tmp = pos + QPoint(0, i*textHeight + i*yMargin);

		//pLabels.at(i)->move(pos + QPoint(0, (i%numLines)*textHeight + (i%numLines)*yMargin));
		QRect tmpRect = pLabels.at(i)->geometry();
		tmpRect.moveTopRight(QPoint(pos.x(), pos.y() + (i%numLines)*textHeight + (i%numLines)*yMargin));
		//qDebug() << "Pos X" << pos.x() << "PosY" << pos.y() + (i%numLines)*textHeight + (i%numLines)*yMargin;
		pLabels.at(i)->setGeometry(tmpRect);

		tmpRect = pValues.at(i)->geometry();
		tmpRect.moveTopLeft(pos + QPoint(xMargin, (i%numLines)*textHeight + (i%numLines)*yMargin));
		pValues.at(i)->setGeometry(tmpRect);
		pValues.at(i)->setFixedWidth(widthValues);

	}

}

void DkMetaDataInfo::updateLabels() {

	if (mapIptcExif.empty())
		setFileInfo(file, imgSize);

	createLabels();
}

void DkMetaDataInfo::setRating(int rating) {

	QString sRating;
	sRating.setNum(rating);

	for (int i=0; i<pLabels.size(); i++) {

		QString tmp = pLabels.at(i)->getText();
		if (!tmp.compare("Rating:")) {
			pValues.at(i)->setText(sRating, -1);
		}
	}
}

void DkMetaDataInfo::setResolution(int xRes, int yRes) {

	QString x,y;
	x.setNum(xRes);
	y.setNum(yRes);
	x=x+"/1";
	y=y+"/1";

	DkImageLoader::imgMetaData.setExifValue("Exif.Image.XResolution",x.toStdString());
	DkImageLoader::imgMetaData.setExifValue("Exif.Image.YResolution",y.toStdString());

}


void DkMetaDataInfo::paintEvent(QPaintEvent *event) {

	QPainter painter(this);
	
	//painter.setWorldTransform(worldMatrix);
	//painter.setWorldMatrixEnabled(true);

	draw(&painter);
	painter.end();

	DkWidget::paintEvent(event);
}

void DkMetaDataInfo::resizeEvent(QResizeEvent *resizeW) {
	//qDebug() << "resizeW:" << resizeW->size().width();
	//qDebug() << parent->width();

	setMinimumHeight(1);
	setMaximumHeight(exifHeight);

	resize(parent->width(), resizeW->size().height());
	//setGeometry(0, parent->height()-exifHeight, parent->width(), exifHeight);
	
	int gw = qMin(gradientWidth, qRound(0.2f*resizeW->size().width()));

	rightGradientRect.setTopLeft(QPoint(parent->width()-gw, 0));
	rightGradientRect.setSize(QSize(gw, exifHeight));
	leftGradientRect.setSize(QSize(gw, exifHeight));

	rightGradient.setStart(rightGradientRect.topLeft());
	rightGradient.setFinalStop(rightGradientRect.topRight());
	leftGradient.setStart(leftGradientRect.topLeft());
	leftGradient.setFinalStop(leftGradientRect.topRight());

	if (parent->width() > minWidth) {
		worldMatrix = QTransform();
		layoutLabels();
		//qDebug() << "parent->width() > minWidth  d.h. layoutlabels";
	}

	if (parent->width() < minWidth && worldMatrix.dx() == 0) {
		layoutLabels();
	}

	if ((parent->width() < minWidth) && (worldMatrix.dx()+minWidth < parent->width())) {
		//layoutLabels();
		QTransform tmpMatrix = QTransform();
		int dX = (parent->width()-minWidth) - worldMatrix.dx();
		
		tmpMatrix.translate(dX, 0);
		worldMatrix.translate(dX, 0);

		for (int i=0; i< pLabels.size(); i++) {
			pLabels.at(i)->setGeometry(tmpMatrix.mapRect(pLabels.at(i)->geometry()));
			pValues.at(i)->setGeometry(tmpMatrix.mapRect(pValues.at(i)->geometry()));
		}
		//layoutLabels();
	}

	DkWidget::resizeEvent(resizeW);
	update();
}

void DkMetaDataInfo::draw(QPainter* painter) {

	//QImage* img;

	if (!painter)
		return;

	//labels are left outside of the widget -> set gradient
	if (parent->width() < minWidth && worldMatrix.dx() < 0) {
		
		if (-worldMatrix.dx() < leftGradientRect.width())
			leftGradient.setFinalStop(-worldMatrix.dx(), 0);
		painter->fillRect(leftGradientRect, leftGradient);
	}
	else
		painter->fillRect(leftGradientRect, bgCol);

	//labels are right outside of the widget -> set gradient
	if (parent->width() < minWidth && worldMatrix.dx()+minWidth > parent->width()) {
		
		int rightOffset = worldMatrix.dx()+minWidth-parent->width();
		if (rightOffset < rightGradientRect.width())
			rightGradient.setStart(rightGradientRect.left()+(rightGradientRect.width() - rightOffset), 0);
		painter->fillRect(rightGradientRect, rightGradient);
	}
	else
		painter->fillRect(rightGradientRect, bgCol);
	
	painter->fillRect(QRect(QPoint(leftGradientRect.right()+1,0), QSize(size().width()-leftGradientRect.width()-rightGradientRect.width(), size().height())), bgCol);
}

void DkMetaDataInfo::mouseMoveEvent(QMouseEvent *event) {


	if (lastMousePos.isNull()) {
		lastMousePos = event->pos();
		QWidget::mouseMoveEvent(event);
		return;
	}

	if (event->buttons() == Qt::LeftButton && parent->width() < minWidth)
		QWidget::setCursor(Qt::ClosedHandCursor);
	if (event->buttons() != Qt::LeftButton && parent->width() > minWidth)
		QWidget::unsetCursor();
	if (event->buttons() != Qt::LeftButton && parent->width() < minWidth)
		QWidget::setCursor(Qt::OpenHandCursor);


	if (event->buttons() != Qt::LeftButton)
		lastMousePos = event->pos();

	int mouseDir = event->pos().x() - lastMousePos.x();


	if (parent->width() < minWidth && event->buttons() == Qt::LeftButton) {
		
		currentDx = mouseDir;

		lastMousePos = event->pos();

		QTransform tmpMatrix = QTransform();
		tmpMatrix.translate(currentDx, 0);

		if (((worldMatrix.dx()+currentDx)+minWidth >= parent->width() && currentDx < 0) ||
		    ((worldMatrix.dx()+currentDx) <= 0 && currentDx > 0)) {

			worldMatrix.translate(currentDx, 0);

			for (int i=0; i< pLabels.size(); i++) {
				pLabels.at(i)->setGeometry(tmpMatrix.mapRect(pLabels.at(i)->geometry()));
				pValues.at(i)->setGeometry(tmpMatrix.mapRect(pValues.at(i)->geometry()));
			}
		}

		update();

		return;
	}

	lastMousePos = event->pos();

	QWidget::mouseMoveEvent(event);
}


// DkTransformRectangle --------------------------------------------------------------------
DkTransformRect::DkTransformRect(int idx, DkRotatingRect* rect, QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f) {

	this->parentIdx = idx;
	this->size = QSize(12, 12);
	this->rect = rect;

	init();

	this->resize(size);
	setCursor(Qt::CrossCursor);
}

void DkTransformRect::init() {

}

void DkTransformRect::draw(QPainter *painter) {

	QPen penNoStroke;
	penNoStroke.setWidth(0);
	penNoStroke.setColor(QColor(0,0,0,0));
	
	QPen pen;
	//pen.setWidth(1);
	pen.setColor(QColor(255,255,0,100));

	QRectF visibleRect(QPointF(), QSizeF(5,5));
	QRectF whiteRect(QPointF(), QSize(7,7));
	visibleRect.moveCenter(geometry().center());
	whiteRect.moveCenter(geometry().center());

	// draw the control point
	painter->setWorldMatrixEnabled(false);
	painter->setPen(penNoStroke);
	painter->setBrush(QColor(0, 0, 0, 0));
	painter->drawRect(geometry());	// invisible rect for mouseevents...
	//painter->setPen(pen);
	painter->setBrush(QColor(255,255,255, 100));
	painter->drawRect(whiteRect);
	painter->setBrush(QColor(0,0,0));
	painter->drawRect(visibleRect);
	painter->setWorldMatrixEnabled(true);
}

void DkTransformRect::mousePressEvent(QMouseEvent *event) {
	
	if (event->buttons() == Qt::LeftButton) {
		posGrab = event->globalPos();
		initialPos = geometry().topLeft();

		emit updateDiagonal(parentIdx);
	}
	qDebug() << "mouse pressed control point";
}

void DkTransformRect::mouseMoveEvent(QMouseEvent *event) {

	if (event->buttons() == Qt::LeftButton) {
		
		QPointF pt = initialPos+event->globalPos()-posGrab;
		emit ctrlMovedSignal(parentIdx, pt, event->modifiers() == Qt::ShiftModifier);
	}
}

void DkTransformRect::mouseReleaseEvent(QMouseEvent *event) {

}

void DkTransformRect::enterEvent(QEvent *event) {

	if (rect)
		setCursor(rect->cpCursor(parentIdx));
}

// DkEditableRectangle --------------------------------------------------------------------
DkEditableRect::DkEditableRect(QRectF rect, QWidget* parent, Qt::WindowFlags f) : DkWidget(parent, f) {

	this->parent = parent;
	this->rect = rect;

	rotatingCursor = QCursor(QPixmap(":/nomacs/img/rotating-cursor.png"));
	
	setAttribute(Qt::WA_MouseTracking);

	pen = QPen(QColor(0, 0, 0, 255), 1);
	pen.setCosmetic(true);
	brush = (DkSettings::App::appMode == DkSettings::mode_frameless) ?
		DkSettings::Display::bgColorFrameless :
		DkSettings::Display::bgColorWidget;

	state = do_nothing;
	worldTform = 0;
	imgTform = 0;
	imgRect = 0;

	oldDiag = DkVector(-1.0f, -1.0f);
	
	for (int idx = 0; idx < 8; idx++) {
		ctrlPoints.push_back(new DkTransformRect(idx, &this->rect, this));
		ctrlPoints[idx]->hide();
		connect(ctrlPoints[idx], SIGNAL(ctrlMovedSignal(int, QPointF, bool)), this, SLOT(updateCorner(int, QPointF, bool)));
		connect(ctrlPoints[idx], SIGNAL(updateDiagonal(int)), this, SLOT(updateDiagonal(int)));
	}
		
}

void DkEditableRect::reset() {

	rect = QRectF();
	//for (int idx = 0; idx < ctrlPoints.size(); idx++)
	//	ctrlPoints[idx]->reset();

}

QPointF DkEditableRect::map(const QPointF &pos) {

	QPointF posM = pos;
	if (worldTform) posM = worldTform->inverted().map(posM);
	if (imgTform)	posM = imgTform->inverted().map(posM);
	
	return posM;
}

QPointF DkEditableRect::clipToImage(const QPointF &pos) {
	
	if (!imgRect)
		return QPointF(pos);

	QRectF imgViewRect(*imgRect);
	if (worldTform) imgViewRect = worldTform->mapRect(imgViewRect);

	float x = pos.x();
	float y = pos.y();

	if (x < imgViewRect.left())
		x = imgViewRect.left();
	if (x > imgViewRect.right())
		x = imgViewRect.right();

	if (y < imgViewRect.top())
		y = imgViewRect.top();
	if (y > imgViewRect.bottom())
		y = imgViewRect.bottom();

	return QPointF(x,y);		// round
}

void DkEditableRect::updateDiagonal(int idx) {

	// we need to store the old diagonal in order to enable "keep aspect ratio"
	if (rect.isEmpty())
		oldDiag = DkVector(-1.0f, -1.0f);
	else
		oldDiag = rect.getDiagonal(idx);
}

void DkEditableRect::updateCorner(int idx, QPointF point, bool isShiftDown) {

	DkVector diag = (isShiftDown) ? oldDiag : DkVector();

	rect.updateCorner(idx, map(point), diag);
	update();
}

void DkEditableRect::paintEvent(QPaintEvent *event) {

	// create path
	QPainterPath path;
	QRectF canvas = QRectF(geometry().x()-1, geometry().y()-1, geometry().width()+1, geometry().height()+1);
	path.addRect(canvas);
	
	QPolygonF p;
	if (!rect.isEmpty()) {
		// TODO: directly map the points (it's easier and not slower at all)
		p = rect.getClosedPoly();
		p = tTform.map(p);
		p = rTform.map(p); 
		p = tTform.inverted().map(p);
		if (imgTform) p = imgTform->map(p);
		if (worldTform) p = worldTform->map(p);
		path.addPolygon(p);
		qDebug() << "drawing rect";
	}

	// now draw
	QPainter painter(this);

	painter.setPen(pen);
	painter.setBrush(brush);
	painter.drawPath(path);

	//// debug
	//painter.drawPoint(rect.getCenter());

	// this changes the painter -> do it at the end
	if (!rect.isEmpty()) {
		
		for (int idx = 0; idx < ctrlPoints.size(); idx++) {
			
			QPointF cp;
			
			if (idx < 4) {
				QPointF c = p[idx];
				cp = c-ctrlPoints[idx]->getCenter();
			}
			// paint control points in the middle of the edge
			else if (idx >= 4) {
				QPointF s = ctrlPoints[idx]->getCenter();

				QPointF lp = p[idx % 4];
				QPointF rp = p[(idx+1) % 4];

				QVector2D lv = QVector2D(lp-s);
				QVector2D rv = QVector2D(rp-s);

				cp = (lv + 0.5*(rv - lv)).toPointF();
			}

			ctrlPoints[idx]->move(qRound(cp.x()+0.5f), qRound(cp.y()+0.5f));
			ctrlPoints[idx]->draw(&painter);
		}
	}
 
	painter.end();
}

// make events callable
void DkEditableRect::mousePressEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->buttons() == Qt::LeftButton && event->modifiers() == DkSettings::Global::altMod) {
		event->setModifiers(Qt::NoModifier);	// we want a 'normal' action in the viewport
		event->ignore();
		return;
	}

	posGrab = map(event->posF());
	clickPos = event->posF();

	if (rect.isEmpty()) {
		state = initializing;
	}
	else if (rect.getPoly().containsPoint(posGrab, Qt::OddEvenFill)) {
		state = moving;
	}
	else {
		state = rotating;
	}

	// we should not need to do this?!
	setFocus(Qt::ActiveWindowFocusReason);


	//QWidget::mousePressEvent(event);
}

void DkEditableRect::mouseMoveEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->modifiers() == DkSettings::Global::altMod) {
		
		if (event->buttons() != Qt::LeftButton)
			setCursor(Qt::OpenHandCursor);
		else
			setCursor(Qt::ClosedHandCursor);

		event->setModifiers(Qt::NoModifier);
		event->ignore();
		return;
	}

	// why do we need to do this?
	if (!hasFocus())
		setFocus(Qt::ActiveWindowFocusReason);

	QPointF posM = map(event->posF());
	
	if (event->buttons() != Qt::LeftButton && !rect.isEmpty()) {
		// show rotating - moving
		if (rect.getPoly().containsPoint(map(event->pos()), Qt::OddEvenFill))
			setCursor(Qt::SizeAllCursor);
		else
			setCursor(rotatingCursor);
	}
	else if (rect.isEmpty())
		setCursor(Qt::CrossCursor);
	
	if (state == initializing && event->buttons() == Qt::LeftButton) {

		// TODO: we need a snap function otherwise you'll never get the bottom left corner...
		
		qDebug() << "pg: " << posGrab;

		QPointF clipPos = clipToImage(event->posF());

		if (!imgRect || !rect.isEmpty() || clipPos == event->posF()) {
			
			if (rect.isEmpty()) {

				for (int idx = 0; idx < ctrlPoints.size(); idx++)
					ctrlPoints[idx]->show();

				QPointF p = map(clipToImage(clickPos));
				rect.setAllCorners(p);
			}
			
			// when initializing shift should make the rect a square
			DkVector diag = (event->modifiers() == Qt::ShiftModifier) ? DkVector(-1.0f, -1.0f) : DkVector();
			rect.updateCorner(2, map(clipPos), diag);
			update();
		}
 
	}
	else if (state == moving && event->buttons() == Qt::LeftButton) {
		
		QPointF dxy = posM-posGrab;
		rTform.translate(dxy.x(), dxy.y());
		posGrab = posM;
		update();
	}
	else if (state == rotating && event->buttons() == Qt::LeftButton) {

		DkVector c(rect.getCenter());
		DkVector xt(posGrab);
		DkVector xn(posM);

		// compute the direction vector;
		xt = c-xt;
		xn = c-xn;
		double angle = xn.angle() - xt.angle();


		// just rotate in CV_PI*0.25 steps if shift is pressed
		if (event->modifiers() == Qt::ShiftModifier) {
			double angleRound = DkMath::normAngleRad(angle+rect.getAngle(), -CV_PI*0.125, CV_PI*0.125);
			angle -= angleRound;
		}
			
		if (!tTform.isTranslating())
			tTform.translate(-c.x, -c.y);
		
		rTform.reset();
		rTform.rotateRadians(angle);

		update();
	}

	//QWidget::mouseMoveEvent(event);
	//qDebug() << "edit rect mouse move";
}

void DkEditableRect::mouseReleaseEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->buttons() == Qt::LeftButton && event->modifiers() == DkSettings::Global::altMod) {
		setCursor(Qt::OpenHandCursor);
		event->setModifiers(Qt::NoModifier);
		event->ignore();
		return;
	}

	state = do_nothing;

	// apply transform
	QPolygonF p = rect.getPoly();
	p = tTform.map(p);
	p = rTform.map(p); 
	p = tTform.inverted().map(p);

	// Cropping tool fix start

	// Check the order or vertexes
	float signedArea = (p[1].x() - p[0].x()) * (p[2].y() - p[0].y()) - (p[1].y()- p[0].y()) * (p[2].x() - p[0].x());
	// If it's wrong, just change it
	if (signedArea > 0)
	{
		QPointF tmp = p[1];
		p[1] = p[3];
		p[3] = tmp;
	}
	// Cropping tool fix end

	rect.setPoly(p);

	rTform.reset();	
	tTform.reset();
	update();
	//QWidget::mouseReleaseEvent(event);
}

void DkEditableRect::keyPressEvent(QKeyEvent *event) {

	if (event->key() == Qt::Key_Alt)
		setCursor(Qt::OpenHandCursor);

	QWidget::keyPressEvent(event);
}

void DkEditableRect::keyReleaseEvent(QKeyEvent *event) {

	if (event->key() == Qt::Key_Escape)
		hide();
	else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
		
		if (!rect.isEmpty())
			emit enterPressedSignal(rect);

		setVisible(false);
		setWindowOpacity(0);
	}

	qDebug() << "key pressed rect";

	QWidget::keyPressEvent(event);
}

void DkEditableRect::setVisible(bool visible) {

	if (!visible) {
		
		rect = DkRotatingRect();
		for (int idx = 0; idx < ctrlPoints.size(); idx++)
			ctrlPoints[idx]->hide();
	}
	else {
		setFocus(Qt::ActiveWindowFocusReason);
		setCursor(Qt::CrossCursor);
	}

	DkWidget::setVisible(visible);
}


DkAnimationLabel::DkAnimationLabel(QString animationPath, QWidget* parent) : DkLabel(parent) {

	init(animationPath, QSize());
}

DkAnimationLabel::DkAnimationLabel(QString animationPath, QSize size, QWidget* parent) : DkLabel(parent) {

	init(animationPath, size);
}

DkAnimationLabel::~DkAnimationLabel() {

	animation->deleteLater();
}

void DkAnimationLabel::init(const QString& animationPath, const QSize& size) {
	
	animation = new QMovie(animationPath);
	margin = QSize(14, 14);

	QSize s = size;
	if(s.isEmpty()) {
		animation->jumpToNextFrame();
		s = animation->currentPixmap().size();
		animation->jumpToFrame(0);

		// padding
		s += margin;
	}

	setFixedSize(s);
	setMovie(animation);
	hide();

	setStyleSheet("QLabel {background-color: " + DkUtils::colorToString(bgCol) + "; border-radius: 10px;}");
}

void DkAnimationLabel::halfSize() {

	// allows for anti-aliased edges of gif animations
	if (animation) {
		QSize s = (size()-margin)*0.5f;
		animation->setScaledSize(s);
		setFixedSize(s+margin);
	}
}

void DkAnimationLabel::showTimed(int time) {
	
	if(!this->animation.isNull() &&
		(this->animation->state() == QMovie::NotRunning ||
		 this->animation->state() == QMovie::Paused)) {
		
			this->animation->start();
	}
	DkLabel::showTimed(time);
}


void DkAnimationLabel::hide() {
	
	if(!this->animation.isNull()) {
		if(this->animation->state() == QMovie::Running) {
			
			this->animation->stop();
		}
	}

	DkLabel::hide();
}

DkColorChooser::DkColorChooser(QColor defaultColor, QString text, QWidget* parent, Qt::WindowFlags flags) : QWidget(parent, flags) {

	this->defaultColor = defaultColor;
	this->text = text;

	init();

}

void DkColorChooser::init() {

	accept = false;

	colorDialog = new QColorDialog(this);
	colorDialog->setObjectName("colorDialog");
	colorDialog->setOption(QColorDialog::ShowAlphaChannel, true);

	QVBoxLayout* vLayout = new QVBoxLayout(this);
	vLayout->setContentsMargins(11,0,11,0);
	
	QLabel* colorLabel = new QLabel(text);
	colorButton = new QPushButton("", this);
	colorButton->setFlat(true);
	colorButton->setObjectName("colorButton");
	
	QPushButton* resetButton = new QPushButton(tr("Reset"), this);
	resetButton->setObjectName("resetButton");
	//resetButton->setAutoDefault(true);

	QWidget* colWidget = new QWidget(this);
	QHBoxLayout* hLayout = new QHBoxLayout(colWidget);
	hLayout->setContentsMargins(11,0,11,0);

	hLayout->addWidget(colorButton);
	hLayout->addWidget(resetButton);
	hLayout->addStretch();

	vLayout->addWidget(colorLabel);
	vLayout->addWidget(colWidget);

	setColor(defaultColor);
	QMetaObject::connectSlotsByName(this);
}

void DkColorChooser::setColor(QColor color) {

	colorDialog->setCurrentColor(color);
	colorButton->setStyleSheet("QPushButton {background-color: " + DkUtils::colorToString(color) + "; border: 1px solid #888; min-height: 24px}");
}

QColor DkColorChooser::getColor() {
	return colorDialog->currentColor();
}

void DkColorChooser::on_resetButton_clicked() {
	setColor(defaultColor);
	emit resetClicked();
}

void DkColorChooser::on_colorButton_clicked() {
	colorDialog->show();
}

void DkColorChooser::on_colorDialog_accepted() {
	
	setColor(colorDialog->currentColor());
	accept = true;
	emit accepted();
}

// Image histogram  -------------------------------------------------------------------
DkHistogram::DkHistogram(QWidget *parent) : DkWidget(parent){
	
	setObjectName("DkHistogram");
	this->parent = parent;
	this->setMinimumWidth(260);
	this->setMinimumHeight(130);
	isPainted = false;
	maxValue = 20;
	scaleFactor = 1;
}

DkHistogram::~DkHistogram() {


}

/**
 * Paints the image histogram
 **/
void DkHistogram::paintEvent(QPaintEvent* event) {

	QPainter painter(this);
	painter.setPen(QColor(200, 200, 200));
	painter.fillRect(1, 1, width() - 3, height() - 2, bgCol);
	painter.drawRect(1, 1, width() - 3, height() - 2);

	if(isPainted && maxValue > 0){
		for(int i = 0; i < 256; i++){
			int rLineHeight = ((int) (hist[0][i] * (height() - 4) * scaleFactor / maxValue) < height() - 4) ? (int) (hist[0][i] * (height() - 4) * scaleFactor / maxValue) : height() - 4;
			int gLineHeight = ((int) (hist[1][i] * (height() - 4) * scaleFactor / maxValue) < height() - 4) ? (int) (hist[1][i] * (height() - 4) * scaleFactor / maxValue) : height() - 4;
			int bLineHeight = ((int) (hist[2][i] * (height() - 4) * scaleFactor / maxValue) < height() - 4) ? (int) (hist[2][i] * (height() - 4) * scaleFactor / maxValue) : height() - 4;
			int maxLineHeight = (rLineHeight > gLineHeight) ? ((rLineHeight > bLineHeight) ? rLineHeight : bLineHeight) :  ((gLineHeight > bLineHeight) ? gLineHeight : bLineHeight);

			int vCombined = qMin(qMin(rLineHeight, gLineHeight), bLineHeight);

			for(int j = 0; j <= maxLineHeight; j++) {

				if(j <= rLineHeight && j <= gLineHeight && j <= bLineHeight) {
				
					// make last pixel lighter -> enhances visual appearence
					int c = (j == vCombined && rLineHeight == gLineHeight && gLineHeight == bLineHeight) ? 200 : 100;
					painter.setPen(QColor(c,c,c));
				}
				else if(j <= rLineHeight && j <= gLineHeight) painter.setPen(Qt::yellow);
				else if(j <= rLineHeight && j <= bLineHeight) painter.setPen(Qt::magenta);
				else if(j <= gLineHeight && j <= bLineHeight) painter.setPen(Qt::cyan);
				else if(j <= rLineHeight) painter.setPen(Qt::red);
				else if(j <= gLineHeight) painter.setPen(Qt::green);
				else if(j <= bLineHeight) painter.setPen(Qt::blue);
				else 
					continue;

				painter.drawPoint(2 + i, height() - j - 2);
			}
		}
	}
}

/**
 * Goes through the image and counts pixels values. They are used to create the image histogram.
 * @param currently displayed image
 **/ 
void DkHistogram::drawHistogram(QImage imgQt) {

	if (!isVisible() || imgQt.isNull()) {
		setPainted(false);
		return;
	}

	DkTimer dt;

#ifdef WITH_OPENCV


	long histValues[3][256];

	for (int idx = 0; idx < 256; idx++) {
		histValues[0][idx] = 0;
		histValues[1][idx] = 0;
		histValues[2][idx] = 0;
	}
	

	// 8 bit images
	if (imgQt.depth() == 8) {

		qDebug() << "8 bit histogram -------------------";

		for (int rIdx = 0; rIdx < imgQt.height(); rIdx++) {

			const unsigned char* pixel = imgQt.constScanLine(rIdx);

			for (int cIdx = 0; cIdx < imgQt.width(); cIdx++, pixel++) {

				histValues[0][*pixel]++;
				histValues[1][*pixel]++;
				histValues[2][*pixel]++;
			}
		}
	}
	// 24 bit images
	else if (imgQt.depth() == 24) {
		
		qDebug() << "24 bit histogram -------------------";

		// TODO: not tested!!
		for (int rIdx = 0; rIdx < imgQt.height(); rIdx++) {

			const unsigned char* pixel = imgQt.constScanLine(rIdx);

			for (int cIdx = 0; cIdx < imgQt.width(); cIdx++) {

				// If I understood the api correctly, the first bits are 0 if we have 24bpp & < 8 bits per channel
				histValues[0][*pixel]++; pixel++;
				histValues[1][*pixel]++; pixel++;
				histValues[2][*pixel]++; pixel++;
			}
		}
	}
	// 32 bit images
	else if (imgQt.depth() == 32) {
		
		for (int rIdx = 0; rIdx < imgQt.height(); rIdx++) {
		
			const QRgb* pixel = (QRgb*)(imgQt.constScanLine(rIdx));
	
			for (int cIdx = 0; cIdx < imgQt.width(); cIdx++, pixel++) {

				histValues[0][qRed(*pixel)]++;
				histValues[1][qGreen(*pixel)]++;
				histValues[2][qBlue(*pixel)]++;
			}
		}
	}

	int maxHistValue = 0;

	for (int idx = 0; idx < 256; idx++) {
		
		if (histValues[0][idx] > maxHistValue)
			maxHistValue = histValues[0][idx];
		if (histValues[1][idx] > maxHistValue)
			maxHistValue = histValues[1][idx];
		if (histValues[2][idx] > maxHistValue)
			maxHistValue = histValues[2][idx];
	}

	//Mat imgMat = DkImage::qImage2Mat(imgQt);
	//
	//vector<Mat> imgChannels;
	//split(imgMat, imgChannels);

	//int noChannels = (imgChannels.size() < 3) ? 1 : 3;

	//// Set the number of bins
	//int histSize = 256;
	//// Set the ranges for B,G,R
	//float range[] = { 0, 256 } ;
	//const float* histRange = { range };

	//MatND hist;
	//// note: long == int if compiled with a 32bit compiler
	//long histValues[3][256];
	//long maxHistValue = 0;

	//for (int i = 0; i < noChannels; i++) {

	//	calcHist( &imgChannels[(noChannels - 1) - i], 1, 0, Mat(), hist, 1, &histSize, &histRange, true, false); // careful! channels are rotated: B,G,R
	//	
	//	for (int j = 0; j < 256; j++) histValues[i][j] = hist.at<float>(j);
	//	hist.setTo(0);
	//}

	//if (noChannels == 1) {

	//	for (int i = 0; i < 256; i++) {
	//		histValues[2][i] = histValues[1][i] = histValues[0][i];
	//		
	//		if(histValues[0][i] > maxHistValue) maxHistValue = histValues[0][i];
	//	}
	//}
	//else {

	//	for (int i = 0; i < 256; i++) {
	//		long maxRGB = (histValues[0][i] > histValues[1][i]) ? 
	//			((histValues[0][i] > histValues[2][i]) ? histValues[0][i] : histValues[2][i]) :  
	//			((histValues[1][i] > histValues[2][i]) ? histValues[1][i] : histValues[2][i]);

	//		if(maxRGB > maxHistValue) maxHistValue = maxRGB;
	//	}
	//}
	//qDebug() << "computing the histogram took me: " << QString::fromStdString(dt.getTotal());

	setMaxHistogramValue(maxHistValue);
	updateHistogramValues(histValues);
	setPainted(true);

#else

	setPainted(false);

#endif
	
	qDebug() << "drawing the histogram took me: " << QString::fromStdString(dt.getTotal());

	update();
}

/**
 * Clears the histogram panel
 **/ 
void DkHistogram::clearHistogram() {

	setPainted(false);
	update();
}

void DkHistogram::setPainted(bool isPainted) {

	this->isPainted = isPainted;
}

void DkHistogram::setMaxHistogramValue(long maxValue) {

	if (maxValue == 0)
		setPainted(false);

	this->maxValue = maxValue;
}

/**
 * Updates histogram values.
 * @param values to be copied
 **/ 
void DkHistogram::updateHistogramValues(long histValues[][256]) {

	for(int i = 0; i < 256; i++) {
		this->hist[0][i] = histValues[0][i];
		this->hist[1][i] = histValues[1][i];
		this->hist[2][i] = histValues[2][i];
	}
}

/**
 * Mouse events for scaling the histogram - enlarge the histogram between the bottom axis and the cursor position
 **/ 
void DkHistogram::mousePressEvent(QMouseEvent *event) {

	// always propagate mouse events
	if (event->buttons() != Qt::LeftButton)
		DkWidget::mousePressEvent(event);
}

void DkHistogram::mouseMoveEvent(QMouseEvent *event) {

	if (event->buttons() == Qt::LeftButton) {
		
		float cp = height() - event->pos().y();
		
		if (cp > 0) {
			scaleFactor = height() / cp;
			update();
		}
	}
	else
		DkWidget::mouseMoveEvent(event);

}

void DkHistogram::mouseReleaseEvent(QMouseEvent *event) {
	
	scaleFactor = 1;
	update();

	if (event->buttons() != Qt::LeftButton)
		DkWidget::mouseReleaseEvent(event);
}

// DkSlider --------------------------------------------------------------------
DkSlider::DkSlider(QString title, QWidget* parent) : QWidget(parent) {

	createLayout();
	
	// init
	titleLabel->setText(title);
	
	// defaults
	setMinimum(0);	
	setMaximum(100);
	setTickInterval(1);
	setValue(50);
}

void DkSlider::createLayout() {

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	layout->setContentsMargins(0,0,0,0);
	
	QWidget* dummy = new QWidget();
	QHBoxLayout* titleLayout = new QHBoxLayout(dummy);
	titleLayout->setContentsMargins(0,0,0,5);

	QWidget* dummyBounds = new QWidget();
	QHBoxLayout* boundsLayout = new QHBoxLayout(dummyBounds);
	boundsLayout->setContentsMargins(0,0,0,0);

	titleLabel = new QLabel();
	
	sliderBox = new QSpinBox();

	slider = new QSlider();
	slider->setOrientation(Qt::Horizontal);

	minValLabel = new QLabel();
	maxValLabel = new QLabel();
	
	titleLayout->addWidget(titleLabel);
	titleLayout->addStretch();
	titleLayout->addWidget(sliderBox);

	boundsLayout->addWidget(minValLabel);
	boundsLayout->addStretch();
	boundsLayout->addWidget(maxValLabel);

	layout->addWidget(dummy);
	layout->addWidget(slider);
	layout->addWidget(dummyBounds);

	// connect
	connect(slider, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
	connect(sliderBox, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
}

}


