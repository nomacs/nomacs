/*******************************************************************************************************
 DkWidgets.cpp
 Created on:	17.05.2011
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011 Florian Kleber <florian@nomacs.org>

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

namespace nmc {

DkWidget::DkWidget(QWidget* parent, Qt::WFlags flags) : QWidget(parent, flags) {
	init();
}

void DkWidget::init() {

	setMouseTracking(true);

	bgCol = QColor(0, 0, 0, 100);
	showing = false;
	hiding = false;
	blocked = false;

	// widget starts on hide
	opacityEffect = new QGraphicsOpacityEffect(this);
	opacityEffect->setOpacity(0);
	setGraphicsEffect(opacityEffect);
	QWidget::hide();
}

void DkWidget::show() {
	
	if (!blocked && !showing) {
		hiding = false;
		showing = true;
		QWidget::show();
		animateOpacityUp();
	}
}

void DkWidget::hide() {
	
	if (!hiding) {
		hiding = true;
		showing = false;
		animateOpacityDown();
	}
}

void DkWidget::setVisible(bool visible) {
	
	if (blocked) {
		QWidget::setVisible(false);
		return;
	}

	if (visible && !isVisible() && !showing)
		opacityEffect->setOpacity(100);

	emit visibleSignal(visible);	// if this gets slow -> put it into hide() or show()
	QWidget::setVisible(visible);
}

void DkWidget::animateOpacityUp() {

	if (!showing)
		return;

	if (opacityEffect->opacity() >= 1.0f || !showing) {
		opacityEffect->setOpacity(1.0f);
		showing = false;
		return;
	}

	QTimer::singleShot(20, this, SLOT(animateOpacityUp()));
	opacityEffect->setOpacity(opacityEffect->opacity()+0.05);
}

void DkWidget::animateOpacityDown() {

	if (!hiding)
		return;

	if (opacityEffect->opacity() <= 0.0f) {
		opacityEffect->setOpacity(0.0f);
		hiding = false;
		QWidget::hide();	// finally hide the widget
		return;
	}

	QTimer::singleShot(20, this, SLOT(animateOpacityDown()));
	opacityEffect->setOpacity(opacityEffect->opacity()-0.05);
}

// DkSplashScreen --------------------------------------------------------------------
DkSplashScreen::DkSplashScreen(QWidget* parent, Qt::WFlags flags) : QDialog(0, flags) {

	QPixmap img(":/nomacs/img/splash-screen.png");
	setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
	setAttribute(Qt::WA_TranslucentBackground);
	
	imgLabel = new QLabel(this, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
	imgLabel->setPixmap(img);
	imgLabel->show();

	// create exit shortcuts
	QShortcut* escExit = new QShortcut(Qt::Key_Escape, this);
	QObject::connect(escExit, SIGNAL( activated ()), this, SLOT( close() ));

	// set the text
	text = 
		QString("Flo was here und wünscht<br>" 
		"Stefan fiel Spaß während<br>" 
		"Markus rockt... <br><br>" 
		
		"<a href=\"http://www.nomacs.org\">www.nomacs.org</a><br>"
		"<a href=\"mailto:developers@nomacs.org\">developers@nomacs.org</a><br><br>" 

		"This program is licensed under GNU Public License v3<br>"
		"&#169; Markus Diem, Stefan Fiel and Florian Kleber, 2011<br><br>"

		"Press [ESC] to exit");

	textLabel = new QLabel(this, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
	textLabel->setScaledContents(true);
	textLabel->setTextFormat(Qt::RichText);
	textLabel->setText(text);
	textLabel->move(131, 280);
	textLabel->setOpenExternalLinks(true);

	QLabel* versionLabel = new QLabel(this, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
	versionLabel->setTextFormat(Qt::RichText);
	versionLabel->setText("Version: " % QApplication::applicationVersion() % " &beta;<br>"
#ifdef WITH_OPENCV
				"RAW support: Yes"
#else
				"RAW support: No"
#endif  
			     );

	versionLabel->move(390, 280);

	qDebug() << "splash screen created...";
}

//void DkSplashScreen::mousePressEvent(QMouseEvent* event) {
//
//	close();
//}


// DkFilePreview --------------------------------------------------------------------
DkFilePreview::DkFilePreview(QWidget* parent, Qt::WFlags flags) : DkWidget(parent, flags) {

	this->parent = parent;
	init();
}

void DkFilePreview::init() {

	if (parent)
		resize(parent->width(), 220);

	setMouseTracking (true);//receive mouse event everytime
	thumbsLoader = 0;

	xOffset = 20;
	yOffset = 20;

	currentDx = 0;
	currentFileIdx = 0;
	oldFileIdx = 0;
	mouseTrace = 0;

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

	minHeight = DkSettings::DisplaySettings::thumbSize + yOffset;
	resize(parent->width(), minHeight);
	selected = -1;

	// load a default image
	QImageReader imageReader(":/nomacs/img/dummy-img.png");
	float fw = (float)DkSettings::DisplaySettings::thumbSize/(float)imageReader.size().width();
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
	//else

	minHeight = DkSettings::DisplaySettings::thumbSize + yOffset;
	resize(parent->width(), minHeight);

	QPainter painter(this);
	painter.setBackground(bgCol);
	
	painter.setPen(Qt::NoPen);
	painter.setBrush(bgCol);
	QRect r = this->geometry();
	r.setHeight(minHeight);
	painter.drawRect(r);

	painter.setWorldTransform(worldMatrix);
	painter.setWorldMatrixEnabled(true);

	// TODO: paint dummies
	if (thumbs.empty())
		return;

	drawThumbs(&painter);

	if (currentFileIdx != oldFileIdx) {
		oldFileIdx = currentFileIdx;
		moveImages();
	}	

}

void DkFilePreview::drawThumbs(QPainter* painter) {
	
	bufferDim = QRectF(QPointF(0, yOffset/2), QSize(xOffset, 0));
	thumbRects.clear();

	// update stub??
	if (stubImg.width() != DkSettings::DisplaySettings::thumbSize) {
		// load a default image
		QImageReader imageReader(":/nomacs/img/dummy-img.png");
		float fw = (float)DkSettings::DisplaySettings::thumbSize/(float)imageReader.size().width();
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

		// center vertically
		if (img.height() < img.width() || img.height() < DkSettings::DisplaySettings::thumbSize)
			r.moveCenter(QPoint(r.center().x(), height()/2));

		// update the buffer dim
		bufferDim.setRight(bufferDim.right() + img.width() + xOffset/2);
		thumbRects.push_back(r);

		QRectF imgWorldRect = worldMatrix.mapRect(r);
		
		// if the image was just loaded -> go to the current thumbnail
		if (currentFileIdx != oldFileIdx && currentFileIdx == idx)
			currentDx = -(imgWorldRect.center().x()-width()/2.0f);
			// TODO: there is still a bug -> not centered if the thumbnails before me are not loaded yet

		// is the current image within the canvas?
		if (imgWorldRect.right() < 0)
			continue;
		if (imgWorldRect.left() > width()) 
			continue;

		// load the thumb!
		if (thumb.hasImage() == DkThumbNail::not_loaded && currentFileIdx == oldFileIdx)
			thumbsLoader->setLoadLimits(idx-10, idx+10);

		// show that there are more images...
		if (worldMatrix.dx() < 0 && imgWorldRect.left() < leftGradient.finalStop().x())
			drawFadeOut(leftGradient, imgWorldRect, &img);
		if (/*worldMatrix.dx() >= -(bufferDim.right()-width()+xOffset) && */imgWorldRect.right() > rightGradient.start().x())
			drawFadeOut(rightGradient, imgWorldRect, &img);

		if (idx == selected && !selectionGlow.isNull()) {
			painter->drawPixmap(r, selectionGlow, QRect(QPoint(), img.size()));
			painter->setOpacity(0.8);
			painter->drawImage(r, img, QRect(QPoint(), img.size()));
			painter->setOpacity(1.0f);
		}
		else if (idx == currentFileIdx) {

			if (currentImgGlow.isNull() || currentFileIdx != oldFileIdx || currentImgGlow.size() != img.size())
				createCurrentImgEffect(img.copy(), QColor(100, 214, 44));

			painter->drawPixmap(r, currentImgGlow, QRect(QPoint(), img.size()));
			painter->setOpacity(0.8);
			painter->drawImage(r, img, QRect(QPoint(), img.size()));
			painter->setOpacity(1.0f);
		}
		else
			painter->drawImage(r, img, QRect(QPoint(), img.size()));


		//painter->fillRect(QRect(0,0,200, 110), leftGradient);
		//if (x > width())
		//	break;
	}

}

void DkFilePreview::drawFadeOut(QLinearGradient gradient, QRectF imgRect, QImage *img) {

	if (img && img->format() == QImage::Format_Indexed8)
		return;

	QTransform wm;
	wm.translate(imgRect.left(), 0);

	QLinearGradient imgGradient = gradient;
	imgGradient.setStart(imgGradient.start().x() - imgRect.left(), 0);
	imgGradient.setFinalStop(imgGradient.finalStop().x() - imgRect.left(), 0);

	//qDebug() << "gradient: " << gradient.start() << gradient.finalStop();
	//qDebug() << "image gradient: " << imgGradient.start() << imgGradient.finalStop();
	QImage mask = *img;
	QPainter painter(&mask);
	//painter.setWorldTransform(wm);
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

	resize(parent->width(), event->size().height());

	// now update...
	borderTrigger = (float)width()*winPercent;
	leftGradient.setFinalStop(QPoint(borderTrigger, 0));
	rightGradient.setStart(QPoint(width()-borderTrigger, 0));
	rightGradient.setFinalStop(QPoint(width(), 0));
	
	update();
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

		qDebug() << "enterPos: " << enterPos.x();

		float dx = std::fabs((float)(enterPos.x() - event->pos().x()))*0.015;
		qDebug() << "dx: " << dx;
		
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
		moveImages();
		return;
	}
	else
		unsetCursor();

	int ndx = width() - event->pos().x();
	int pdx = event->pos().x();

	bool left = pdx < ndx;
	float dx = (left) ? pdx : ndx;

	if (dx < borderTrigger && (mouseDir < 0 && left || mouseDir > 0 && !left)) {
		dx = std::exp((borderTrigger - dx)/borderTrigger*3);
		currentDx = (left) ? dx : -dx;
		
		moveImageTimer->start(1);
	}
	else if (dx > borderTrigger)
		moveImageTimer->stop();

	// select the current thumbnail
	if (dx > borderTrigger*0.5) {

		int oldSelection = selected;
		selected = -1;

		// find out where is the mouse
		for (int idx = 0; idx < thumbRects.size(); idx++) {

			if (worldMatrix.mapRect(thumbRects.at(idx)).contains(event->pos())) {
				selected = idx;
				DkThumbNail thumb = thumbs.at(selected);
				createSelectedEffect(thumb.getImage(), DkSettings::DisplaySettings::highlightColor);
				fileLabel->setText(thumbs.at(selected).getFile().fileName(), -1);
				qDebug() << "selected...";
				break;
			}
		}

		if (selected != -1 || selected != oldSelection) {
			//QPoint pos = event->pos() + QPoint(16, 16);
			//fileLabel->setPosition(pos);
			update();
		}
		else if (selected == -1)
			fileLabel->hide();
	}
	else
		selected = -1;

	lastMousePos = event->pos();

	QWidget::mouseMoveEvent(event);
}

void DkFilePreview::mousePressEvent(QMouseEvent *event) {

	if (event->buttons() == Qt::LeftButton) {
		mouseTrace = 0;
	}
	else if (event->buttons() == Qt::MiddleButton) {
		
		enterPos = event->pos();
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

void DkFilePreview::leaveEvent(QEvent *event) {

	selected = -1;
	moveImageTimer->stop();
	fileLabel->hide();
	update();
}

void DkFilePreview::moveImages() {

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

void DkFilePreview::updateDir(QFileInfo file, bool force) {

	QFileInfo oldFile = currentFile;
	currentFile = file;
	QDir dir = file.absoluteDir();
	dir.setNameFilters(DkImageLoader::fileFilters);
	dir.setSorting(QDir::LocaleAware);
	
	if (thumbsLoader) {
		oldFileIdx = currentFileIdx;
		currentFileIdx = thumbsLoader->getFileIdx(file);
		update();
	}

	if (!force && oldFile.absoluteDir() == file.absoluteDir() && !thumbs.empty() || 
		file.absoluteFilePath().contains(":/nomacs/img/lena"))	// do not load our resources as thumbs
		return;

	if (thumbsLoader) {
		thumbsLoader->stop();
		thumbsLoader->wait();
		delete thumbsLoader;
	}

	thumbs.clear();

	if (dir.exists()) {

		thumbsLoader = new DkThumbsLoader(&thumbs, dir);
		connect(thumbsLoader, SIGNAL(updateSignal()), this, SLOT(update()));

		thumbsLoader->start();
	}
}

// DkOverview --------------------------------------------------------------------
DkOverview::DkOverview(QWidget* parent, Qt::WindowFlags flags) : DkWidget(parent, flags) {

	this->parent = parent;
}

void DkOverview::paintEvent(QPaintEvent *event) {

	if (img.isNull() || !imgMatrix || !worldMatrix)
		return;

	QPainter painter(this);

	QSize viewSize = size();	// overview shall take 15% of the viewport....
	QRectF imgRect = QRectF(QPoint(), img.size());
	QRectF overviewRect = QRectF(QPoint(), QSize(geometry().width()-1, geometry().height()-1));			// get the overview rect

	QTransform overviewImgMatrix = getScaledImageMatrix();			// matrix that always resizes the image to the current viewport
	QRectF overviewImgRect = overviewImgMatrix.mapRect(imgRect);
	overviewImgRect.setTop(overviewImgRect.top()+1);
	overviewImgRect.setLeft(overviewImgRect.left()+1);
	overviewImgRect.setWidth(overviewImgRect.width()-1);			// we have a border... correct that...
	overviewImgRect.setHeight(overviewImgRect.height()-1);

	// now render the current view
	QRectF viewRect = viewPortRect;
	viewRect = worldMatrix->inverted().mapRect(viewRect);
	viewRect = imgMatrix->inverted().mapRect(viewRect);
	viewRect = overviewImgMatrix.mapRect(viewRect);

	//draw the image's location
	painter.setBrush(bgCol);
	painter.setPen(QColor(200, 200, 200));
	painter.drawRect(overviewRect);
	painter.setOpacity(0.8f);
	painter.drawImage(overviewImgRect, imgT, QRect(0, 0, imgT.width(), imgT.height()));

	painter.setPen(QColor(100, 0, 0));
	painter.setBrush(QColor(100, 0, 0, 50));
	painter.drawRect(viewRect);
	painter.end();

	DkWidget::paintEvent(event);
}

void DkOverview::mousePressEvent(QMouseEvent *event) {
	
	enterPos = event->pos();
	posGrab = event->pos();
	// TODO: if it is just clicked -> move the view to that position
}

void DkOverview::mouseReleaseEvent(QMouseEvent *event) {

	QPointF dxy = enterPos-QPointF(event->pos());

	if (dxy.manhattanLength() < 4) {
		// move to the current position
		// now render the current view
		QRectF viewRect = viewPortRect;
		viewRect = worldMatrix->inverted().mapRect(viewRect);
		viewRect = imgMatrix->inverted().mapRect(viewRect);
		viewRect = getScaledImageMatrix().mapRect(viewRect);
		QPointF currentViewPoint = viewRect.center();

		float panningSpeed = -(worldMatrix->m11()/(getScaledImageMatrix().m11()/imgMatrix->m11()));

		QPointF cPos = event->pos();
		QPointF dxy = (cPos - currentViewPoint)/worldMatrix->m11()*panningSpeed;
		emit moveViewSignal(dxy);

		if (event->modifiers() == DkSettings::GlobalSettings::altMod)
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

	if (event->modifiers() == DkSettings::GlobalSettings::altMod)
		emit sendTransformSignal();

}

void DkOverview::resizeEvent(QResizeEvent* event) {

	if (event->size() == size())
		return;

	resizeImg();

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

	QRectF imgRect = QRectF(QPoint(), img.size());

	QTransform overviewImgMatrix = getScaledImageMatrix();			// matrix that always resizes the image to the current viewport
	QRectF overviewImgRect = overviewImgMatrix.mapRect(imgRect);
	overviewImgRect.setTop(overviewImgRect.top()+1);
	overviewImgRect.setLeft(overviewImgRect.left()+1);
	overviewImgRect.setWidth(overviewImgRect.width()-1);			// we have a border... correct that...
	overviewImgRect.setHeight(overviewImgRect.height()-1);

	// fast downscaling
	imgT = img.scaled(overviewImgRect.size().width()*2, overviewImgRect.size().height()*2, Qt::KeepAspectRatio, Qt::FastTransformation);
	imgT = imgT.scaled(overviewImgRect.size().width(), overviewImgRect.size().height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QTransform DkOverview::getScaledImageMatrix() {

	if (img.isNull())
		return QTransform();

	// the image resizes as we zoom
	QRectF imgRect = QRectF(QPoint(), img.size());
	float ratioImg = imgRect.width()/imgRect.height();
	float ratioWin = (float)width()/(float)height();

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


// DkLabel --------------------------------------------------------------------
DkLabel::DkLabel(QWidget* parent, const QString& text) : QLabel(text, parent) {

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
	
	init();
}

void DkLabelBg::init() {

	margin = QPoint(7, 2);
	updateStyleSheet();
	setAttribute(Qt::WA_TransparentForMouseEvents);	// labels should forward mouse events
}

void DkLabelBg::drawBackground(QPainter* painter) {

	painter->fillRect(QRect(QPoint(), size()), QColor(0, 0, 0, 100));
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

	QColor color = QColor(0, 0, 0, 200);	//TODO: make setter

	QRect textRect = QRect(QPoint(), size());
	textRect.setWidth(textRect.width()-end.width()-1);
	QRectF endRect = QRect(textRect.right()+1, 0, end.width(), geometry().height());
	painter->drawImage(textRect, gradient);
	painter->drawImage(endRect, end);
}

// DkInfoLabel --------------------------------------------------------------------
DkInfoLabel::DkInfoLabel(QWidget* parent, const QString& text, int position) : DkLabelBg(parent, text) {

	this->parent = parent;
	this->position = position;
	init();
	hide();
	
}

void DkInfoLabel::init() {

	DkLabelBg::init();
	marginParent = QPoint(10, 10);
}

void DkInfoLabel::setText(const QString msg, int time) {

	DkLabel::setText(msg, time);
	updatePos(offset);
}

void DkInfoLabel::updatePos(const QPoint& offset) {

	if(!parent)
		return;

	QPoint pos;

	if (position == center_label)
		pos = QPoint(parent->width()/2-width()/2, parent->height()/2-height()/2);
	else if (position == bottom_left_label)
		pos = QPoint(0, parent->height()-height()-50);
	else if (position == bottom_right_label)
		pos = QPoint(parent->width() - width() - marginParent.x(), parent->height()-height()-marginParent.y());
	else if (position == top_left_label)
		pos = marginParent;
	//qDebug() << "offset: " << offset;

	this->offset = offset;

	move(pos+offset);
	controlPosition();
}

void DkInfoLabel::controlPosition() {

	if (!parent)
		return;

	// reset label
	setFixedWidth(-1);
	QLabel::adjustSize();

	QTransform labelTransform;
	
	// check if the label fits to the parent's canvas
	setFixedWidth( (width() > parent->width()) ? parent->width() : -1);
	
	QRectF sRect = labelTransform.mapRect(geometry());

	// check if the label's position fits to the canvas
	QRectF canvas = parent->geometry();
	if (sRect.left() < canvas.left())
		labelTransform.translate((canvas.left()-geometry().left()), 0);
	if (sRect.right() > canvas.right())
		labelTransform.translate((canvas.right()-geometry().right()), 0);

	// control vertical position
	if (sRect.top() < canvas.top())
		labelTransform.translate(0, canvas.top()-geometry().top());
	if (sRect.bottom() > canvas.bottom())
		labelTransform.translate(0, canvas.bottom()-geometry().bottom());

	setGeometry(labelTransform.mapRect(geometry()));
}

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
		opacity = 0.7f;
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
	imgAlpha.fill(DkSettings::DisplaySettings::highlightColor);
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

	this->rating = rating;
	init();

	int iconSize = 16;
	QPoint margin = QPoint(10, 4);
	int lastStarRight = 0;
	int timeToDisplay = 3000;

	layout = new QBoxLayout(QBoxLayout::LeftToRight);
	layout->setContentsMargins(margin.x(), margin.y(), margin.x(), margin.y());
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
DkFileInfoLabel::DkFileInfoLabel(QWidget* parent) : DkWidget(parent) {

	marginParent = QPoint(10, 10);

	this->parent = parent;
	title = new DkLabel(this);
	date = new DkLabel(this);
	rating = new DkRatingLabel(0, this);

	createLayout();
}

void DkFileInfoLabel::setVisible(bool visible) {

	// TODO: unbalanced warning (in DkRatingLabel) makes me feel uncomfortable

	// nothing to display??
	if (!DkSettings::SlideShowSettings::display.testBit(DkSlideshowSettingsWidget::display_file_name) &&
		!DkSettings::SlideShowSettings::display.testBit(DkSlideshowSettingsWidget::display_creation_date) &&
		!DkSettings::SlideShowSettings::display.testBit(DkSlideshowSettingsWidget::display_file_rating)) {
			DkWidget::setVisible(false);
			return;
	}

	DkWidget::setVisible(visible);
	title->setVisible(DkSettings::SlideShowSettings::display.testBit(DkSlideshowSettingsWidget::display_file_name));
	date->setVisible(DkSettings::SlideShowSettings::display.testBit(DkSlideshowSettingsWidget::display_creation_date));
	rating->setVisible(DkSettings::SlideShowSettings::display.testBit(DkSlideshowSettingsWidget::display_file_rating));
		
	adjustSize();
}

DkRatingLabel* DkFileInfoLabel::getRatingLabel() {
	return rating;
}

void DkFileInfoLabel::createLayout() {
	
	layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	layout->setSpacing(0);
	
	layout->addWidget(title);
	layout->addWidget(date);
	layout->addWidget(rating);
	layout->addStretch();

	adjustSize();
}

void DkFileInfoLabel::updatePos(const QPoint& pos) {

	QPoint m = QPoint(parent->width() - DkWidget::geometry().width() - marginParent.x(), parent->height() - height() - marginParent.y());

	if (pos.x() != -1 && pos.y() != -1)
		this->offset = pos;
	move(m+offset);
}

void DkFileInfoLabel::updateInfo(const QFileInfo& file, const QString& date, const int rating) {

	updateTitle(file);
	updateDate(date);
	updateRating(rating);
	
	adjustSize();
	updatePos();
}

void DkFileInfoLabel::updateTitle(const QFileInfo& file) {
	
	this->file = file;
	updateDate();
	this->title->setText(file.fileName(), -1);
	this->title->setAlignment(Qt::AlignRight);
	adjustSize();
}

void DkFileInfoLabel::updateDate(const QString& date) {

	//// convert date
	//QString dateConverted;
	//QStringList dateSplit = date.split(QRegExp("[/: \t]"));

	//if (dateSplit.size() >= 3) {

	//	QDate dateV = QDate(dateSplit[0].toInt(), dateSplit[1].toInt(), dateSplit[2].toInt());
	//	dateConverted = dateV.toString(Qt::SystemLocaleShortDate);

	//	if (dateSplit.size() >= 6) {
	//		QTime time = QTime(dateSplit[3].toInt(), dateSplit[4].toInt(), dateSplit[5].toInt());
	//		dateConverted += " " + time.toString(Qt::SystemLocaleShortDate);
	//	}
	//}
	//else if (file.exists()) {
	//	QDateTime dateCreated = file.created();
	//	dateConverted += dateCreated.toString(Qt::SystemLocaleShortDate);
	//}
	//else
	//	dateConverted = "unknown date";

	QString dateConverted = DkUtils::convertDate(date, file);

	this->date->setText(dateConverted, -1);
	this->date->setAlignment(Qt::AlignRight);
	adjustSize();
}

void DkFileInfoLabel::updateRating(const int rating) {
	
	this->rating->setRating(rating);
	adjustSize();
}

void DkFileInfoLabel::adjustSize() {

	DkWidget::adjustSize();
	updatePos();
}

void DkFileInfoLabel::paintEvent(QPaintEvent *event) {

	// simply take a DkWidget??
	QPainter painter(this);
	painter.fillRect(QRect(QPoint(), size()), bgCol);
	painter.end();

	//DkWidget::paintEvent(event);
}

// player --------------------------------------------------------------------
DkPlayer::DkPlayer(QWidget* parent) : DkWidget(parent) {

	init();
}

void DkPlayer::init() {
	
	// slide show
	
	int timeToDisplayPlayer = 3000;
	timeToDisplay = DkSettings::SlideShowSettings::time*1000;	// TODO: settings
	playing = false;
	displayTimer = new QTimer(this);
	displayTimer->setInterval(timeToDisplay);
	displayTimer->setSingleShot(true);
	connect(displayTimer, SIGNAL(timeout()), this, SLOT(next()));

	hideTimer = new QTimer(this);
	hideTimer->setInterval(timeToDisplayPlayer);
	hideTimer->setSingleShot(true);
	connect(hideTimer, SIGNAL(timeout()), this, SLOT(hide()));

	// gui
	int prevNextTopMargin = 10;
	int spacing = 0;

	actions.resize(1);
	actions[play_action] = new QAction(tr("play"), this);
	actions[play_action]->setShortcut(Qt::Key_Space);
	connect(actions[play_action], SIGNAL(triggered()), this, SLOT(togglePlay()));

	QPixmap icon = QPixmap(":/nomacs/img/player-back.png");
	previousButton = new DkButton(icon, tr("previous"), this);
	previousButton->keepAspectRatio = false;
	connect(previousButton, SIGNAL(pressed()), this, SLOT(previous()));

	icon = QPixmap(":/nomacs/img/player-pause.png");
	QPixmap icon2 = QPixmap(":/nomacs/img/player-play.png");
	playButton = new DkButton(icon, icon2, tr("play"), this);
	previousButton->keepAspectRatio = false;
	playButton->setChecked(false);	// TODO: settings
	playButton->addAction(actions[play_action]);
	connect(playButton, SIGNAL(toggled(bool)), this, SLOT(play(bool)));

	icon = QPixmap(":/nomacs/img/player-next.png");
	nextButton = new DkButton(icon, tr("next"), this);
	previousButton->keepAspectRatio = false;
	connect(nextButton, SIGNAL(pressed()), this, SLOT(next()));

	// now add to layout
	QHBoxLayout *layout = new QHBoxLayout;
	//layout->setSpacing(-100);	// why is there no negative spacing??

	layout->addWidget(previousButton);
	layout->addWidget(playButton);
	layout->addWidget(nextButton);

	setLayout(layout);

	//maxSize = size();
	maxSize = QSize(315, 113);	// sorry, but we need to know this...
	minSize = maxSize * 0.5f;
}

void DkPlayer::setGeometry(int x, int y, int width, int height) {

	setGeometry(QRect(x, y, width, height));
}

void DkPlayer::setGeometry(const QRect& rect) {

	if (rect.topLeft() == geometry().topLeft() && rect.width() == geometry().width())
		return;

	QRect r = rect;

	if (rect.width() > maxSize.width())
		r.setSize(maxSize);
	if (rect.height() == 0)
		r.setHeight(r.width()*maxSize.height()/maxSize.width());

	if (r.width() < minSize.width() || r.height() < minSize.height())
		r.setSize(minSize);

	QWidget::setGeometry(r);

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

	DkWidget::show();
}

// tiff dialog --------------------------------------------------------------------
DkTifDialog::DkTifDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {
	init();
}

void DkTifDialog::init() {

	isOk = false;
	setWindowTitle("TIF compression");
	setFixedSize(270, 146);
	setLayout(new QVBoxLayout(this));
	
	//QWidget* buttonWidget = new QWidget(this);
	QGroupBox* buttonWidget = new QGroupBox(tr("TIF compression"), this);
	QVBoxLayout* vBox = new QVBoxLayout(buttonWidget);
	QButtonGroup* bGroup = new QButtonGroup(buttonWidget);
	noCompressionButton = new QRadioButton( tr("&no compression"), this);
	compressionButton = new QRadioButton(tr("&LZW compression (lossless)"), this);
	compressionButton->setChecked(true);
	bGroup->addButton(noCompressionButton);
	bGroup->addButton(compressionButton);

	vBox->addWidget(noCompressionButton);
	vBox->addWidget(compressionButton);

	QPushButton* buttonOk = new QPushButton(tr("&Ok"), this);
	connect(buttonOk, SIGNAL(clicked()), this, SLOT(okPressed()));
	QPushButton* buttonCancel = new QPushButton(tr("&Cancel"), this);
	connect(buttonCancel, SIGNAL(clicked()), this, SLOT(cancelPressed()));


	QWidget* okCancelWidget = new QWidget(this);
	QHBoxLayout* hBox = new QHBoxLayout(okCancelWidget);
	hBox->setDirection(QBoxLayout::RightToLeft);
	hBox->addWidget(buttonCancel);
	hBox->addWidget(buttonOk);

	hBox->addStretch();

	layout()->addWidget(buttonWidget);
	layout()->addWidget(okCancelWidget);
}

void DkTifDialog::okPressed() {

	isOk = true;
	this->close();
}

void DkTifDialog::cancelPressed() {

	this->close();
}


//
DkJpgDialog::DkJpgDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

	init();
}

void DkJpgDialog::init() {

	isOk = false;
	hasAlpha = false;

	img = 0;
	leftSpacing = 40;
	margin = 10;

	
	colorDialog = new QColorDialog(this);
	colorDialog->setCurrentColor(Qt::white);
	connect(colorDialog, SIGNAL(accepted()), this, SLOT(newBgCol()));

	setWindowTitle("JPG Settings");
	setFixedSize(600, 500);
	createLayout();

}

void DkJpgDialog::createLayout() {

	// bottom widget
	QWidget* bottomWidget = new QWidget(this);
	QHBoxLayout* bottomWidgetHBoxLayout = new QHBoxLayout(bottomWidget);

	QPushButton* buttonOk = new QPushButton(tr("&Ok"));
	connect(buttonOk, SIGNAL(clicked()), this, SLOT(okPressed()));
	QPushButton* buttonCancel = new QPushButton(tr("&Cancel"));
	connect(buttonCancel, SIGNAL(clicked()), this, SLOT(cancelPressed()));

	QSpacerItem* spacer = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Expanding);
	bottomWidgetHBoxLayout->addItem(spacer);
	bottomWidgetHBoxLayout->addWidget(buttonOk);
	bottomWidgetHBoxLayout->addWidget(buttonCancel);

	// central widget
	centralWidget = new QWidget(this);

	// slider
	QLabel* compLabel = new QLabel(tr("Compression"), centralWidget);
	compLabel->move(leftSpacing, margin*3.5+10);
	
	// preview
	QSize s = QSize(width()-2*leftSpacing-10, width()-2*leftSpacing-10);
	s *= 0.5;

	slider = new QSlider(centralWidget);
	slider->setMinimum(0);
	slider->setMaximum(100);
	slider->setValue(80);
	slider->setTickInterval(10);
	slider->setOrientation(Qt::Horizontal);
	slider->setGeometry(QRect(leftSpacing, compLabel->geometry().bottom()-margin, s.width(), 20));

	sliderValueLabel = new QSpinBox(centralWidget);
	sliderValueLabel->setGeometry(slider->geometry().right()-45, margin*3.5, 45, 20);
	sliderValueLabel->setMinimum(0);
	sliderValueLabel->setMaximum(100);

	updateSliderLabel(slider->value());
	//connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(updateSliderLabel(int)));
	connect(slider, SIGNAL(valueChanged(int)), this, SLOT(updateSliderLabel(int)));
	connect(sliderValueLabel, SIGNAL(valueChanged(int)), this, SLOT(updateSliderValue(int)));

	QLabel* minLabel = new QLabel("0", centralWidget);
	minLabel->move(leftSpacing, slider->geometry().bottom());

	QLabel* maxLabel = new QLabel("100", centralWidget);
	maxLabel->move(slider->geometry().right()-20, minLabel->geometry().top());

	QLabel* origLabel = new QLabel(tr("Original"), centralWidget);
	origLabel->move(QPoint(slider->geometry().left(), slider->geometry().bottom() + margin*2.5));
	QLabel* newLabel = new QLabel(tr("New"), centralWidget);
	newLabel->move(QPoint(s.width()+10+leftSpacing, origLabel->geometry().top()));
	previewLabel = new QLabel(centralWidget);
	previewLabel->setGeometry(QRect(QPoint(slider->geometry().left(), slider->geometry().bottom() + margin*4), s));

	// color button
	colButton = new QPushButton(centralWidget);
	colButton->setFlat(true);
	colButton->setText("");
	colButton->move(leftSpacing, previewLabel->geometry().bottom()+margin*2);
	newBgCol();
	colButton->setEnabled(hasAlpha);
	connect(colButton, SIGNAL(clicked()), this, SLOT(openColorDialog()));

	colLabel = new QLabel(tr("Background Color"), centralWidget);
	colLabel->move(leftSpacing+32, colButton->geometry().top()+7);	// dirty hack
	colLabel->setEnabled(hasAlpha);
	
	BorderLayout* borderLayout = new BorderLayout;
	borderLayout->addWidget(bottomWidget, BorderLayout::South);
	borderLayout->addWidget(centralWidget, BorderLayout::Center);
	this->setSizeGripEnabled(false);

	this->setLayout(borderLayout);


}

void DkJpgDialog::updateSnippets() {

	if (!img)
		return;

	QSize s = QSize(width()-2*leftSpacing-10, width()-2*leftSpacing-10);
	s *= 0.5;
	origImg = QImage(s, QImage::Format_ARGB32);
	origImg.fill(Qt::transparent);
	QRect imgRect = QRect(QPoint(img->width()*0.5-origImg.width()*0.5, img->height()*0.5-origImg.height()*0.5), origImg.size());

	QPainter painter(&origImg);
	painter.setBackgroundMode(Qt::TransparentMode);
	painter.drawImage(QRect(QPoint(), origImg.size()), *img, imgRect);

}

void DkJpgDialog::drawPreview() {

	if (!img)
		return;

	newImg = QImage(origImg.size(), QImage::Format_ARGB32);
	newImg.fill(bgCol.rgb());

	QPainter bgPainter(&newImg);
	bgPainter.drawImage(QRect(QPoint(), origImg.size()), origImg, QRect(QPoint(), origImg.size()));
	bgPainter.end();

	// pre-compute the jpg compression
	QByteArray ba;
	QBuffer buffer(&ba);
	buffer.open(QIODevice::ReadWrite);
	newImg.save(&buffer, "JPG", slider->value());
	newImg.loadFromData(ba, "JPG");

	QImage preview = QImage(origImg.width()*2 + 10, origImg.height(), QImage::Format_ARGB32);
	preview.fill(Qt::transparent);
	QRect pos = QRect(QPoint(), origImg.size());
	QRect posM = pos;
	posM.setSize(QSize(pos.size().width()-1, pos.size().height()-1));

	QPainter painter(&preview);
	painter.setBackgroundMode(Qt::TransparentMode);
	painter.drawImage(pos, origImg, QRect(QPoint(), origImg.size()));
	painter.setPen(QColor(0,0,0,30));
	painter.drawRect(posM);
	pos.moveTopLeft(QPoint(origImg.width()+10, 0));
	posM.moveTopLeft(QPoint(origImg.width()+10, 0));
	painter.drawImage(pos, newImg, QRect(QPoint(), newImg.size()));
	painter.drawRect(posM);

	previewLabel->setGeometry(QRect(QPoint(slider->geometry().left(), slider->geometry().bottom() + margin*4), preview.size()));
	previewLabel->setPixmap(QPixmap::fromImage(preview));
}

void DkJpgDialog::okPressed() {
	
	isOk = true;
	this->close();
}

void DkJpgDialog::cancelPressed() {

	this->close();
}

void DkJpgDialog::showEvent(QShowEvent *event) {
	isOk = false;
}

// DkResizeDialog --------------------------------------------------------------------
//
DkResizeDialog::DkResizeDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

	init();
}

void DkResizeDialog::init() {

	isOk = false;

	img = 0;
	leftSpacing = 40;
	margin = 10;
	exifDpi = 72;

	unitFactor.resize(unit_end);
	unitFactor.insert(unit_cm, 1.0f);
	unitFactor.insert(unit_mm, 10.0f);
	unitFactor.insert(unit_inch, 1.0f/2.54f);

	resFactor.resize(res_end);
	resFactor.insert(res_ppi, 2.54f);
	resFactor.insert(res_ppc, 1.0f);

	setWindowTitle("Resize Image");
	setFixedSize(600, 512);
	createLayout();
	initBoxes();

	QMetaObject::connectSlotsByName(this);

}

void DkResizeDialog::createLayout() {

	// bottom widget
	QWidget* bottomWidget = new QWidget(this);
	QHBoxLayout* bottomWidgetHBoxLayout = new QHBoxLayout(bottomWidget);

	QPushButton* buttonOk = new QPushButton(tr("&Ok"));
	connect(buttonOk, SIGNAL(clicked()), this, SLOT(okPressed()));
	QPushButton* buttonCancel = new QPushButton(tr("&Cancel"));
	connect(buttonCancel, SIGNAL(clicked()), this, SLOT(cancelPressed()));

	QSpacerItem* spacer = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Expanding);
	bottomWidgetHBoxLayout->addItem(spacer);
	bottomWidgetHBoxLayout->addWidget(buttonOk);
	bottomWidgetHBoxLayout->addWidget(buttonCancel);

	// central widget
	centralWidget = new QWidget(this);

	// preview
	QSize s = QSize(width()-2*leftSpacing-10, width()-2*leftSpacing-10);
	s *= 0.5;
	int minPx = 1;
	int maxPx = 100000;
	int minWidth = 0.1;
	int maxWidth = 500000;
	int decimals = 2;
	
	QLabel* origLabel = new QLabel(tr("Original"), centralWidget);
	origLabel->move(QPoint(leftSpacing, 20));
	QLabel* newLabel = new QLabel(tr("New"), centralWidget);
	newLabel->move(QPoint(s.width()+10+leftSpacing, origLabel->geometry().top()));
	previewLabel = new QLabel(centralWidget);
	previewLabel->setGeometry(QRect(QPoint(newLabel->geometry().left(), newLabel->geometry().bottom()), s));

	// all text dialogs... // TODO: go on here...
	QIntValidator* intValidator = new QIntValidator(1, 100000, 0);
	QDoubleValidator* doubleValidator = new QDoubleValidator(1, 1000000, 2, 0);
	doubleValidator->setRange(0, 100, 2);
	
	QWidget* resizeBoxes = new QWidget(centralWidget);
	resizeBoxes->setGeometry(QRect(QPoint(leftSpacing, 300), QSize(400, 170)));

	QGridLayout* gridLayout = new QGridLayout(resizeBoxes);
	
	QLabel* wPixelLabel = new QLabel(tr("Width: "));
	wPixelLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	wPixelEdit = new QSpinBox();
	wPixelEdit->setObjectName("wPixelEdit");
	wPixelEdit->setRange(minPx, maxPx);
	
	lockButton = new DkButton(QIcon(":/nomacs/img/lock.png"), QIcon(":/nomacs/img/lock-unlocked.png"), "lock");
	lockButton->setFixedSize(QSize(16,16));
	lockButton->setObjectName("lockButton");
	lockButton->setCheckable(true);
	lockButton->setChecked(true);

	QLabel* hPixelLabel = new QLabel(tr("Height: "));
	hPixelLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	hPixelEdit = new QSpinBox();
	hPixelEdit->setObjectName("hPixelEdit");
	hPixelEdit->setRange(minPx, maxPx);

	QLabel* unitPixelLabel = new QLabel(tr("pixel"));

	// first row
	int rIdx = 1;
	gridLayout->addWidget(wPixelLabel, 1, rIdx);
	gridLayout->addWidget(wPixelEdit, 1, ++rIdx);
	gridLayout->addWidget(lockButton, 1, ++rIdx);
	gridLayout->addWidget(hPixelLabel, 1, ++rIdx);
	gridLayout->addWidget(hPixelEdit, 1, ++rIdx);
	gridLayout->addWidget(unitPixelLabel, 1, ++rIdx);

	// Document dimensions
	QLabel* widthLabel = new QLabel(tr("Width: "));
	widthLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	widthEdit = new QDoubleSpinBox();
	widthEdit->setObjectName("widthEdit");
	widthEdit->setRange(minWidth, maxWidth);
	widthEdit->setDecimals(decimals);

	lockButtonDim = new DkButton(QIcon(":/nomacs/img/lock.png"), QIcon(":/nomacs/img/lock-unlocked.png"), "lock");
	//lockButtonDim->setIcon(QIcon(":/nomacs/img/lock.png"));
	lockButtonDim->setFixedSize(QSize(16,16));
	lockButtonDim->setObjectName("lockButtonDim");
	lockButtonDim->setCheckable(true);
	lockButtonDim->setChecked(true);
	
	QLabel* heightLabel = new QLabel(tr("Height: "));
	heightLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	heightEdit = new QDoubleSpinBox();
	heightEdit->setObjectName("heightEdit");
	heightEdit->setRange(minWidth, maxWidth);
	heightEdit->setDecimals(decimals);

	unitBox = new QComboBox();
	QStringList unitList;
	unitList.insert(unit_cm, "cm");
	unitList.insert(unit_mm, "mm");
	unitList.insert(unit_inch, "inch");
	unitBox->addItems(unitList);
	unitBox->setObjectName("unitBox");

	// second row
	rIdx = 1;
	gridLayout->addWidget(widthLabel, 2, rIdx);
	gridLayout->addWidget(widthEdit, 2, ++rIdx);
	gridLayout->addWidget(lockButtonDim, 2, ++rIdx);
	gridLayout->addWidget(heightLabel, 2, ++rIdx);
	gridLayout->addWidget(heightEdit, 2, ++rIdx);
	gridLayout->addWidget(unitBox, 2, ++rIdx);
	
	// resolution
	QLabel* resolutionLabel = new QLabel(tr("Resolution: "));
	resolutionLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	resolutionEdit = new QDoubleSpinBox();
	resolutionEdit->setObjectName("resolutionEdit");
	resolutionEdit->setRange(minWidth, maxWidth);
	resolutionEdit->setDecimals(decimals);

	resUnitBox = new QComboBox();
	QStringList resUnitList;
	resUnitList.insert(res_ppi, "pixel/inch");
	resUnitList.insert(res_ppc, "pixel/cm");
	resUnitBox->addItems(resUnitList);
	resUnitBox->setObjectName("resUnitBox");

	// third row
	rIdx = 1;
	gridLayout->addWidget(resolutionLabel, 3, rIdx);
	gridLayout->addWidget(resolutionEdit, 3, ++rIdx);
	gridLayout->addWidget(resUnitBox, 3, ++rIdx, 1, 2);

	// resample
	resampleCheck = new QCheckBox(tr("Resample Image:"));
	resampleCheck->setChecked(true);
	resampleCheck->setObjectName("resampleCheck");

	// TODO: disable items if no opencv is available
	resampleBox = new QComboBox();
	QStringList resampleList;
	resampleList.insert(ipl_nearest, tr("Nearest Neighbor"));
	resampleList.insert(ipl_area, tr("Area (best for downscaling)"));
	resampleList.insert(ipl_linear, tr("Linear"));
	resampleList.insert(ipl_cubic, tr("Bicubic (4x4 pixel interpolation)"));
	resampleList.insert(ipl_lanczos, tr("Lanczos (8x8 pixel interpolation)"));
	resampleBox->addItems(resampleList);
	resampleBox->setObjectName("resampleBox");
	resampleBox->setCurrentIndex(ipl_cubic);

	// last two rows
	gridLayout->addWidget(resampleCheck, 4, 2, 1, 3);
	gridLayout->addWidget(resampleBox, 5, 2, 1, 3);

	BorderLayout* borderLayout = new BorderLayout;
	borderLayout->addWidget(bottomWidget, BorderLayout::South);
	borderLayout->addWidget(centralWidget, BorderLayout::Center);
	this->setSizeGripEnabled(false);

	this->setLayout(borderLayout);
	this->show();
}

void DkResizeDialog::initBoxes() {

	if (!img || img->isNull())
		return;

	wPixelEdit->setValue(img->width());
	hPixelEdit->setValue(img->height());

	float units = resFactor.at(resUnitBox->currentIndex()) * unitFactor.at(unitBox->currentIndex());
	float width = (float)img->width()/exifDpi * units;
	widthEdit->setValue(width);

	float height = (float)img->height()/exifDpi * units;
	heightEdit->setValue(height);
}

void DkResizeDialog::updateWidth() {

	float pWidth = wPixelEdit->text().toDouble();
	float units = resFactor.at(resUnitBox->currentIndex()) * unitFactor.at(unitBox->currentIndex());
	float width = pWidth/exifDpi * units;
	widthEdit->setValue(width);
}

void DkResizeDialog::updateHeight() {

	float pHeight = hPixelEdit->text().toDouble();
	float units = resFactor.at(resUnitBox->currentIndex()) * unitFactor.at(unitBox->currentIndex());
	float height = pHeight/exifDpi * units;
	heightEdit->setValue(height);
}

void DkResizeDialog::updateResolution() {

	qDebug() << "updating resolution...";
	float wPixel = wPixelEdit->text().toDouble();
	float width = widthEdit->text().toDouble();

	float units = resFactor.at(resUnitBox->currentIndex()) * unitFactor.at(unitBox->currentIndex());
	float resolution = wPixel/width * units;
	resolutionEdit->setValue(resolution);

}

void DkResizeDialog::updatePixelHeight() {

	float height = heightEdit->text().toDouble();
	float units = resFactor.at(resUnitBox->currentIndex()) * unitFactor.at(unitBox->currentIndex());
	int pixelHeight = cvRound(height*exifDpi / units);
	hPixelEdit->setValue(pixelHeight);
}

void DkResizeDialog::updatePixelWidth() {

	float width = widthEdit->text().toDouble();
	float units = resFactor.at(resUnitBox->currentIndex()) * unitFactor.at(unitBox->currentIndex());
	int pixelWidth = cvRound(width*exifDpi / units);
	wPixelEdit->setValue(pixelWidth);
}

void DkResizeDialog::on_lockButtonDim_clicked() {

	lockButton->setChecked(lockButtonDim->isChecked());
	if (!lockButtonDim->isChecked())
		return;

	initBoxes();
	drawPreview();
}

void DkResizeDialog::on_lockButton_clicked() {

	lockButtonDim->setChecked(lockButton->isChecked());
	
	if (!lockButton->isChecked())
		return;

	initBoxes();
	drawPreview();
}

void DkResizeDialog::on_wPixelEdit_valueChanged(QString text) {

	if (!wPixelEdit->hasFocus())
		return;

	updateWidth();
	
	if (!lockButton->isChecked()) {
		drawPreview();
		return;
	}

	int newHeight = cvRound((float)text.toInt()/(float)img->width() * img->height());
	hPixelEdit->setValue(newHeight);
	updateHeight();
	drawPreview();

}

void DkResizeDialog::on_hPixelEdit_valueChanged(QString text) {

	if(!hPixelEdit->hasFocus())
		return;

	updateWidth();

	if (!lockButton->isChecked()) {
		drawPreview();
		return;
	}

	int newWidth = cvRound((float)text.toInt()/(float)img->height() * (float)img->width());
	wPixelEdit->setValue(newWidth);
	updateHeight();
	drawPreview();
}

void DkResizeDialog::on_widthEdit_valueChanged(QString text) {

	if (!widthEdit->hasFocus())
		return;

	if (resampleCheck->isChecked()) 
		updatePixelWidth();

	if (!lockButton->isChecked()) {
		drawPreview();
		return;
	}

	float newHeight = text.toFloat()/(float)img->width() * (float)img->height();
	heightEdit->setValue(newHeight);

	if (resampleCheck->isChecked()) 
		updatePixelHeight();

	if (!resampleCheck->isChecked())
		updateResolution();
	
	drawPreview();
}

void DkResizeDialog::on_heightEdit_valueChanged(QString text) {

	if (!heightEdit->hasFocus())
		return;

	if (resampleCheck->isChecked()) 
		updatePixelHeight();

	if (!lockButton->isChecked()) {
		drawPreview();
		return;
	}
	
	int newWidth = text.toFloat()/(float)img->height() * (float)img->width();
	widthEdit->setValue(newWidth);

	if (resampleCheck->isChecked()) 
		updatePixelWidth();

	if (!resampleCheck->isChecked())
		updateResolution();
	drawPreview();
}

void DkResizeDialog::on_resolutionEdit_valueChanged(QString text) {

	exifDpi = text.toDouble();

	if (!resolutionEdit->hasFocus())
		return;

	updatePixelWidth();
	updatePixelHeight();

	if (resampleCheck->isChecked()) {
		drawPreview();
		return;
	}

	initBoxes();
}


void DkResizeDialog::on_unitBox_currentIndexChanged(int idx) {
	
	updateHeight();
	updateWidth();
	//initBoxes();
}

void DkResizeDialog::on_resUnitBox_currentIndexChanged(int idx) {
	
	updateResolution();
	//initBoxes();
}

void DkResizeDialog::on_resampleCheck_clicked() {

	resampleBox->setEnabled(resampleCheck->isChecked());
	wPixelEdit->setEnabled(resampleCheck->isChecked());
	hPixelEdit->setEnabled(resampleCheck->isChecked());

	if (!resampleCheck->isChecked()) {
		lockButton->setChecked(true);
		lockButtonDim->setChecked(true);
		initBoxes();
	}
	else
		drawPreview();
}

void DkResizeDialog::on_resampleBox_currentIndexChanged(int idx) {
	drawPreview();
}

void DkResizeDialog::updateSnippets() {

	if (!img)
		return;

	QSize s = QSize(width()-2*leftSpacing-10, width()-2*leftSpacing-10);
	s *= 0.5;
	origImg = QImage(s, QImage::Format_ARGB32);
	origImg.fill(Qt::transparent);
	QRect imgRect = QRect(QPoint(img->width()*0.5-origImg.width()*0.5, img->height()*0.5-origImg.height()*0.5), origImg.size());

	QPainter painter(&origImg);
	painter.setBackgroundMode(Qt::TransparentMode);
	painter.drawImage(QRect(QPoint(), origImg.size()), *img, imgRect);

}

void DkResizeDialog::drawPreview() {

	if (!img)
		return;

	newImg = resizeImg(origImg);
	
	QImage preview = QImage(origImg.width()*2 + 10, origImg.height(), QImage::Format_ARGB32);
	preview.fill(Qt::transparent);
	QRect pos = QRect(QPoint(), origImg.size());
	QRect posM = pos;
	posM.setSize(QSize(pos.size().width()-1, pos.size().height()-1));

	QPainter painter(&preview);
	painter.setBackgroundMode(Qt::TransparentMode);
	painter.drawImage(pos, origImg, QRect(QPoint(), origImg.size()));
	painter.setPen(QColor(0,0,0,30));
	painter.drawRect(posM);
	pos.moveTopLeft(QPoint(origImg.width()+10, 0));
	posM.moveTopLeft(QPoint(origImg.width()+10, 0));
	painter.drawImage(pos, newImg, QRect(QPoint(), newImg.size()));
	painter.drawRect(posM);

	//previewLabel->setGeometry(QRect(QPoint(slider->geometry().left(), slider->geometry().bottom() + margin*4), preview.size()));
	previewLabel->setGeometry(QRect(QPoint(40, margin*4), preview.size()));
	previewLabel->setPixmap(QPixmap::fromImage(preview));

	updateSnippets();
}

QImage DkResizeDialog::resizeImg(QImage img, bool silent) {

	if (!this->img || img.isNull())
		return img;

	QSize newSize = QSize(wPixelEdit->text().toInt(), hPixelEdit->text().toInt());
	QSize imgSize = this->img->size();

	qDebug() << "new size: " << newSize;

	// nothing to do
	if (this->img->size() == newSize)
		return img;

	if (this->img->size() != img.size()) {
		// compute relative size
		float relWidth = (float)newSize.width()/(float)imgSize.width();
		float relHeight = (float)newSize.height()/(float)imgSize.height();
		
		qDebug() << "relative size: " << newSize;

		newSize = QSize(img.width()*relWidth, img.height()*relHeight);
	}

	if (newSize.width() < wPixelEdit->minimum() || newSize.width() > wPixelEdit->maximum() || 
		newSize.height() < hPixelEdit->minimum() || newSize.height() > hPixelEdit->maximum()) {
		
		// TODO: if real transform is applied: error dialog
		return QImage();
	}

	Qt::TransformationMode iplQt;
		switch(resampleBox->currentIndex()) {
		case ipl_nearest:	
		case ipl_area:		iplQt = Qt::FastTransformation; break;
		case ipl_linear:	
		case ipl_cubic:		
		case ipl_lanczos:	iplQt = Qt::SmoothTransformation; break;
	}
#ifdef WITH_OPENCV

	int ipl = CV_INTER_CUBIC;
	switch(resampleBox->currentIndex()) {
		case ipl_nearest:	ipl = CV_INTER_NN; break;
		case ipl_area:		ipl = CV_INTER_AREA; break;
		case ipl_linear:	ipl = CV_INTER_LINEAR; break;
		case ipl_cubic:		ipl = CV_INTER_CUBIC; break;
#ifdef DISABLE_LANCZOS
		case ipl_lanczos:	ipl = CV_INTER_CUBIC; break;
#else
		case ipl_lanczos:	ipl = CV_INTER_LANCZOS4; break;
#endif
	}

	Mat resizeImage = DkImage::qImage2Mat(img);

	// is the image convertable?
	if (resizeImage.empty() || newSize.width() < 1 || newSize.height() < 1) {
		return img.scaled(newSize, Qt::IgnoreAspectRatio, iplQt);
	}
	else {

		Mat tmp;
		cv::resize(resizeImage, tmp, cv::Size(newSize.width(), newSize.height()), 0, 0, ipl);
		resizeImage = tmp;
		return DkImage::mat2QImage(resizeImage);
	}
#else

	return img.scaled(newSize, Qt::IgnoreAspectRatio, iplQt);

#endif

}

void DkResizeDialog::okPressed() {

	isOk = true;
	this->close();
}

void DkResizeDialog::cancelPressed() {

	this->close();
}

void DkResizeDialog::showEvent(QShowEvent *event) {
	isOk = false;
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

QString DkMetaDataInfo::sCamDataTags = QString("ImageSize Orientation Make Model ApertureValue Flash FocalLength ") %
	QString("ExposureMode ExposureTime");

QString DkMetaDataInfo::sDescriptionTags = QString("Rating UserComment DateTime DateTimeOriginal ImageDescription Byline BylineTitle City Country ") %
	QString("Headline Caption CopyRight Keywords Path FileSize");




DkMetaDataInfo::DkMetaDataInfo(QWidget* parent) : DkWidget(parent) {
	
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
		qDebug() << "could not load Exif GPS information, set to 72dpi";
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
		qDebug() << "could not load Exif GPS information";
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
						//Value = QString(file.absoluteFilePath());
						//qDebug() << "should be size" << file.size();
						if ((float)file.size()/1024.0f > 1024.0f)
							Value = QString::number((double)file.size()/(1024.0f*1024.0f), 'f', 2) + " MB";
						else
							Value = QString::number(file.size()/(1024)) + " KB";
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
		if (DkSettings::MetaDataSettings::metaDataBits.testBit(i)) {
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
		if (DkSettings::MetaDataSettings::metaDataBits.testBit(DkMetaDataSettingsWidget::camData_end + i)) {
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

	setGeometry(0, parent->height()-exifHeight, parent->width(), exifHeight);
	
	rightGradientRect.setTopLeft(QPoint(parent->width()-gradientWidth, 0));
	rightGradientRect.setSize(QSize(gradientWidth, exifHeight));
	leftGradientRect.setSize(QSize(gradientWidth, exifHeight));

	rightGradient.setStart(rightGradientRect.topLeft());
	rightGradient.setFinalStop(rightGradientRect.topRight());

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


// OpenWithDialog --------------------------------------------------------------------
DkOpenWithDialog::DkOpenWithDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

	init();
}

void DkOpenWithDialog::init() {

	defaultApp = (DkSettings::GlobalSettings::defaultAppIdx < 0) ? 0 : DkSettings::GlobalSettings::defaultAppIdx;
	numDefaultApps = 0;
	userClickedOk = false;

	// TODO: qt obviously saves the settings if the keys are not found...
	// TODO: add GIMP & other software

	// the order must be correct!
	organizations = (QStringList()	<< "Adobe"				<< "Google"			<< "Google"						<< "IrfanView");
	applications =	(QStringList()	<< "Photoshop"			<< "Picasa"			<< "Picasa"						<< "shell");
	pathKeys =		(QStringList()	<< "ApplicationPath"	<< "Directory"		<< "Directory"					<< "");
	exeNames =		(QStringList()	<< ""					<< ""				<< "PicasaPhotoViewer.exe"		<< "");
	screenNames =	(QStringList()	<< tr("&Photoshop")		<< tr("Pi&casa")	<< tr("Picasa Ph&oto Viewer")	<< tr("&IrfanView"));
	
	// find paths to pre-defined software
	for (int idx = 0; idx < organizations.size(); idx++) {
		appPaths.append(searchForSoftware(idx));
		appIcons.append(getIcon(appPaths[idx]));
	}

	createLayout();
	setWindowTitle(tr("Open With..."));

}

void DkOpenWithDialog::createLayout() {
	userRadiosGroup = new QButtonGroup;
	userRadiosGroup->setExclusive(true);

	layout = new QBoxLayout(QBoxLayout::TopToBottom);
	
	QGroupBox* groupBox = new QGroupBox(tr("3rd Party Software"));
	groupBox->setObjectName("softwareGroupBox");
	QGridLayout* bl = new QGridLayout();

	// add default applications
	bool first = true;

	for (int idx = 0; idx < appPaths.size(); idx++) {
						
		if (!appPaths[idx].isEmpty()) {
				
			// create
			QRadioButton* radio = new QRadioButton(screenNames[idx]);
			radio->setObjectName(screenNames[idx]);
			radio->setIcon(appIcons[idx]);
			userRadiosGroup->addButton(radio);

			qDebug() << "appPath: " << appPaths[idx];

			connect(radio, SIGNAL(clicked()), this, SLOT(softwareSelectionChanged()));			
				
			// always check first one
			if (DkSettings::GlobalSettings::defaultAppIdx == -1 && first ||
				DkSettings::GlobalSettings::defaultAppIdx == idx ) {
		
				radio->setChecked(true);
				first = false;
				defaultApp = idx;	// set to default app
			}

			bl->addWidget(radio, numDefaultApps, 0);
			numDefaultApps++;
		}
	}


	QStringList tmpUserPaths = DkSettings::GlobalSettings::userAppPaths; // shortcut

	for (int idx = 0; idx < DkSettings::GlobalSettings::numUserChoices; idx++) {

		// default initialization
		userRadios.append(new QRadioButton(tr("Choose Application")));
		connect(userRadios[idx], SIGNAL(clicked()), this, SLOT(softwareSelectionChanged()));
		userRadios[idx]->setDisabled(true);
		userRadiosGroup->addButton(userRadios[idx]);

		QIcon iconX = QIcon(":/nomacs/img/close.png"); 
		userCleanButtons.append(new QPushButton(this));
		userCleanButtons[idx]->setFlat(true);
		//userCleanButtons[idx]->setStyleSheet("QPushButton:pressed {border:0px; margin:0px;};"); // stay flat when pressed
		userCleanButtons[idx]->setIcon(iconX);
		userCleanButtons[idx]->setVisible(false);
		userCleanButtons[idx]->setFixedWidth(16);
		connect(userCleanButtons[idx], SIGNAL(clicked()), this, SLOT(softwareCleanClicked()));
		
		userCleanSpace.append(new QLabel(this));
		userCleanSpace[idx]->setFixedWidth(16);
		userCleanSpace[idx]->setVisible(true);


		QPushButton* userBrowse = new QPushButton(tr("Browse..."));
		userBrowse->setObjectName("browse-" % QString::number(idx));
		connect(userBrowse, SIGNAL(clicked()), this, SLOT(browseAppFile()));

		screenNames.append("");
		userAppPaths.append("");

		int userIdx = idx + numDefaultApps;

		if (DkSettings::GlobalSettings::defaultAppIdx == userIdx)
			userRadios[idx]->setChecked(true);
		
		// is an application set & is it still installed?
		if (idx < tmpUserPaths.size() &&
			QFileInfo(tmpUserPaths[idx]).exists()) {

				// remove file extension for GUI
				QFileInfo filePathInfo = QFileInfo(tmpUserPaths[idx]);
				screenNames[userIdx] = filePathInfo.fileName();
				screenNames[userIdx].replace("." + filePathInfo.suffix(), "");	

				userAppPaths[idx] = tmpUserPaths[idx];
				userRadios[idx]->setObjectName(screenNames[userIdx]);
				userRadios[idx]->setText(screenNames[userIdx]);
				userRadios[idx]->setIcon(getIcon(tmpUserPaths[idx]));
				userRadios[idx]->setEnabled(true);

				userCleanButtons[idx]->setVisible(true);
				userCleanSpace[idx]->setVisible(false);
		}

		bl->addWidget(userRadios[idx], numDefaultApps+idx, 0);
		bl->addWidget(userCleanButtons[idx], numDefaultApps+idx,1);
		bl->addWidget(userCleanSpace[idx], numDefaultApps+idx,1);
		bl->addWidget(userBrowse, numDefaultApps+idx, 2);
	}

	// never again checkbox
	neverAgainBox = new QCheckBox(tr("Never show this dialog again"));
	neverAgainBox->setObjectName("neverAgainBox");
	neverAgainBox->setChecked(!DkSettings::GlobalSettings::showDefaultAppDialog);
	neverAgainBox->setToolTip(tr("Do not be scared, you can always open this window in Preferences -> Global Settings"));
	
	// ok, cancel button
	QWidget* bottomWidget = new QWidget(this);
	QHBoxLayout* bottomWidgetHBoxLayout = new QHBoxLayout(bottomWidget);

	QPushButton* buttonOk = new QPushButton(tr("&Ok"));
	connect(buttonOk, SIGNAL(clicked()), this, SLOT(okClicked()));
	QPushButton* buttonCancel = new QPushButton(tr("&Cancel"));
	connect(buttonCancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));

	QSpacerItem* spacer = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Expanding);
	bottomWidgetHBoxLayout->addStretch();
	bottomWidgetHBoxLayout->addWidget(buttonOk);
	bottomWidgetHBoxLayout->addWidget(buttonCancel);
	
	groupBox->setLayout(bl);
	layout->addWidget(groupBox);
	layout->addWidget(neverAgainBox);
	layout->addStretch();
	layout->addWidget(bottomWidget);

	setLayout(layout);
}

void DkOpenWithDialog::browseAppFile() {

	QString sender = QObject::sender()->objectName();

	if (!sender.contains("browse"))
		return;

	// identify the browse button
	QStringList splitSender = sender.split("-");

	// was it really a browse button?
	if (splitSender.size() != 2)
		return;

	int senderIdx = splitSender[1].toInt();

	// is the sender valid?
	if (senderIdx < 0 || senderIdx >= userRadios.size())
		return;

	// load system default open dialog
	QString appFilter;
	QString defaultPath = userAppPaths[senderIdx];
#ifdef WIN32
	appFilter += tr("Executable Files (*.exe);;");
	if (!QFileInfo(defaultPath).exists())
		defaultPath = getenv("PROGRAMFILES");
#else
	defaultPath = QDesktopServices::storageLocation(QDesktopServices::ApplicationsLocation); // retrieves startmenu on windows?!
#endif
	
	QString filePath = QFileDialog::getOpenFileName(this, tr("Open Application"),
		defaultPath, 
		appFilter);

	if (filePath.isEmpty())
		return;

	int userIdx = senderIdx+numDefaultApps;

	// remove file extension for GUI
	QFileInfo filePathInfo = QFileInfo(filePath);
	screenNames[userIdx] = filePathInfo.fileName();
	screenNames[userIdx].replace("." % filePathInfo.suffix(), "");	

	userAppPaths[senderIdx] = filePath;
	userRadios[senderIdx]->setObjectName(screenNames[userIdx]);	// needed for slot	
	userRadios[senderIdx]->setText(screenNames[userIdx]);
	userRadios[senderIdx]->setIcon(getIcon(filePath));
	userRadios[senderIdx]->setEnabled(true);
	userRadios[senderIdx]->setChecked(true);
	defaultApp = userIdx;

	userCleanButtons[senderIdx]->setVisible(true);
	userCleanSpace[senderIdx]->setVisible(false);
	qDebug() << "default app idx: " << defaultApp;
}

void DkOpenWithDialog::okClicked() {

	userClickedOk = true;

	// store everything
	DkSettings::GlobalSettings::showDefaultAppDialog = !neverAgainBox->isChecked();
	DkSettings::GlobalSettings::defaultAppIdx = defaultApp;
	DkSettings::GlobalSettings::defaultAppPath = getPath();
	DkSettings::GlobalSettings::userAppPaths = userAppPaths;

	close();
}

void DkOpenWithDialog::cancelClicked() {

	// do not store anything
	close();
}

void DkOpenWithDialog::softwareSelectionChanged() {
	
	QString sender = QObject::sender()->objectName();

	for (int idx = 0; idx < screenNames.size(); idx++) {

		qDebug() << screenNames[idx] << " - " << sender;

		if (screenNames[idx] == sender)
			defaultApp = idx;
	}

	qDebug() << "default app idx..." << defaultApp;
}

void DkOpenWithDialog::softwareCleanClicked() {
	QPushButton* button = (QPushButton*)QObject::sender();
	int idx = userCleanButtons.indexOf(button);
	if (idx == -1)
		return;

	userAppPaths.replace(idx, QString());

	if(userRadios[idx]->isChecked()) {
		userRadiosGroup->setExclusive(false);
		userRadiosGroup->checkedButton()->setChecked(false);
		userRadiosGroup->setExclusive(true);

		QList<QAbstractButton*> buttons = userRadiosGroup->buttons();
		if (buttons.size() > 0)
			buttons[0]->setChecked(true);
		defaultApp = 0;
	}

	userRadios[idx]->setText(tr("Choose Application"));
	userRadios[idx]->setDisabled(true);
	userRadios[idx]->setIcon(QIcon());



	userCleanButtons[idx]->setVisible(false);
	userCleanSpace[idx]->setVisible(true);
}

QString DkOpenWithDialog::searchForSoftware(int softwareIdx) {

	if (softwareIdx < 0 || softwareIdx >= organizations.size())
		return "";

	qDebug() << "\n\nsearching for: " << organizations[softwareIdx] << " " << applications[softwareIdx];

	// locate the settings entry
	QSettings* softwareSettings = new QSettings(QSettings::UserScope, organizations[softwareIdx], applications[softwareIdx]);
	QStringList keys = softwareSettings->allKeys();

	//// debug
	//for (int idx = 0; idx < keys.size(); idx++) {
	//	qDebug() << keys[idx] << " - " << softwareSettings->value(keys[idx]).toString();
	//}
	//// debug

	QString appPath;

	for (int idx = 0; idx < keys.length(); idx++) {

		// find the path
		if (keys[idx].contains(pathKeys[softwareIdx])) {
			appPath = softwareSettings->value(keys[idx]).toString();
			break;
		}
	}

	// if we did not find it -> return
	if (appPath.isEmpty()) {
		// clean up
		delete softwareSettings;
		return appPath;
	}

	if (exeNames[softwareIdx].isEmpty()) {

		// locate the exe
		QDir appFile = appPath.replace("\"", "");	// the string must not have extra quotes
		QFileInfoList apps = appFile.entryInfoList(QStringList() << "*.exe");

		for (int idx = 0; idx < apps.size(); idx++) {
			
			if (apps[idx].fileName().contains(applications[softwareIdx])) {
				appPath = apps[idx].absoluteFilePath();
				break;
			}
		}

		qDebug() << appPath;
	}
	else
		appPath = QFileInfo(appPath, exeNames[softwareIdx]).absoluteFilePath();	// for correct separators

	// clean up
	delete softwareSettings;

	return appPath;
}

QPixmap DkOpenWithDialog::getIcon(QFileInfo file) {


#ifdef WIN32
#include <windows.h>

	// icon extraction should take between 2ms and 13ms
	QPixmap appIcon;
	QString winPath = QDir::toNativeSeparators(file.absoluteFilePath());

	WCHAR* wDirName = new WCHAR[winPath.length()];
	
	// CMakeLists.txt:
	// if compile error that toWCharArray is not recognized:
	// in msvc: Project Properties -> C/C++ -> Language -> Treat WChar_t as built-in type: set to No (/Zc:wchar_t-)
	int dirLength = winPath.toWCharArray(wDirName);
	wDirName[dirLength] = L'\0';	// append null character

	int nIcons = ExtractIconExW(wDirName, 0, NULL, NULL, 0);

	if (!nIcons)
		return appIcon;

	HICON largeIcon;
	HICON smallIcon;
	int err = ExtractIconExW(wDirName, 0, &largeIcon, &smallIcon, 1);

	if (nIcons != 0 && largeIcon != NULL)
		appIcon = QPixmap::fromWinHICON(largeIcon);

	DestroyIcon(largeIcon);
	DestroyIcon(smallIcon);

	return appIcon;

#endif
	
	return QPixmap();
}

// DkTransformRectangle --------------------------------------------------------------------
DkTransformRect::DkTransformRect(int idx, DkRotatingRect* rect, QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f) {

	this->parentIdx = idx;
	this->size = QSize(11, 11);
	this->rect = rect;

	init();

	this->resize(size);
	setCursor(Qt::CrossCursor);
}

void DkTransformRect::init() {

}

void DkTransformRect::draw(QPainter *painter) {

	QPen pen;
	pen.setWidth(0);
	pen.setColor(QColor(0,0,0,0));

	float ro = 2.5f;	// rect offset
	QRectF visibleRect(geometry().topLeft()+QPointF(ro,ro), geometry().size()-QSizeF(ro*2,ro*2));

	// draw the control point
	painter->setWorldMatrixEnabled(false);
	painter->setPen(pen);
	painter->setBrush(QColor(0, 0, 0, 0));
	painter->drawRect(geometry());	// invisible rect for mouseevents...
	painter->setBrush(QColor(0, 0, 0));
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

	qDebug() << "mouse pressed control point";
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
	
	//setAttribute(Qt::WA_MouseTracking);

	pen = QPen(QColor(0, 0, 0, 255), 1);
	pen.setCosmetic(true);
	brush = QColor(0, 0, 0, 90);

	state = do_nothing;
	worldTform = 0;
	imgTform = 0;
	imgRect = 0;

	oldDiag = DkVector(-1.0f, -1.0f);
	qDebug() << "my size: " << geometry();
	
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
	
	// TODO: directly map the points (it's easier and not slower at all)
	QPolygonF p = rect.getClosedPoly();
	p = tTform.map(p);
	p = rTform.map(p); 
	p = tTform.inverted().map(p);
	if (imgTform) p = imgTform->map(p);
	if (worldTform) p = worldTform->map(p);
	path.addPolygon(p);

	// now draw
	QPainter painter(this);

	painter.setPen(pen);
	painter.setBrush(brush);
	painter.drawPath(path);

	//if (imgRect) {
	//	QRectF imgViewRect = QRectF(*imgRect);
	//	if (worldTform) imgViewRect = worldTform->mapRect(imgViewRect);
	//	painter.drawRect(imgViewRect);
	//}
	//if (worldTform)
	//	painter.setWorldTransform(*worldTform);

	//// debug
	//painter.drawPoint(rect.getCenter());

	// this changes the painter -> do it at the end
	if (!rect.isEmpty()) {
		
		// TODO: offset bug when zooming
		for (int idx = 0; idx < ctrlPoints.size(); idx++) {
			
			QPointF cp;
			
			if (idx < 4) {
				QPointF c = p[idx];
				cp = c-ctrlPoints[idx]->getCenter();
			}
			// paint control points in the middle of the edge
			else if (idx >= 4) {
				QPointF s = ctrlPoints[idx]->getCenter();//QPointF(ctrlPoints[idx % 4]->geometry().size().width(), ctrlPoints[idx % 4]->geometry().size().height());

				QPointF lp = p[idx % 4];
				QPointF rp = p[(idx+1) % 4];

				QVector2D lv = QVector2D(lp-s);
				QVector2D rv = QVector2D(rp-s);

				cp = (lv + 0.5*(rv - lv)).toPointF();
			}


			ctrlPoints[idx]->move(cp.x(), cp.y());
			ctrlPoints[idx]->draw(&painter);
		}
	}
 
	painter.end();
}

// make events callable
void DkEditableRect::mousePressEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->buttons() == Qt::LeftButton && event->modifiers() == DkSettings::GlobalSettings::altMod) {
		event->setModifiers(Qt::NoModifier);	// we want a 'normal' action in the viewport
		event->ignore();
		return;
	}

	posGrab = map(event->posF());

	if (rect.isEmpty()) {
		state = initializing;

		rect.setAllCorners(posGrab);
	}
	else if (rect.getPoly().containsPoint(posGrab, Qt::OddEvenFill)) {
		state = moving;
	}
	else {
		state = rotating;
	}

	qDebug() << "mousepress edit rect";
	
	// we should not need to do this?!
	setFocus(Qt::ActiveWindowFocusReason);


	//QWidget::mousePressEvent(event);
}

void DkEditableRect::mouseMoveEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->modifiers() == DkSettings::GlobalSettings::altMod) {
		
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
		//QRectF imgViewRect;
		//
		//if (imgRect) {
		//	imgViewRect = QRectF(*imgRect);
		//	if (worldTform) imgViewRect = worldTform->mapRect(imgViewRect);
		//}
		//
		//if (!imgRect || imgViewRect.contains(event->posF())) {
			
			qDebug() << "contains point...";
			
			// when initializing shift should make the rect a square
			DkVector diag = (event->modifiers() == Qt::ShiftModifier) ? DkVector(-1.0f, -1.0f) : DkVector();
			rect.updateCorner(2, posM, diag);
			update();
		//}
 
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
	if (event->buttons() == Qt::LeftButton && event->modifiers() == DkSettings::GlobalSettings::altMod) {
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

		qDebug() << " enter pressed";
	}

	qDebug() << "key pressed rect";

	QWidget::keyPressEvent(event);
}

void DkEditableRect::setVisible(bool visible) {

	if (!visible) {
		for (int idx = 0; idx < ctrlPoints.size(); idx++)
			ctrlPoints[idx]->hide();
	}
	else {
		for (int idx = 0; idx < ctrlPoints.size(); idx++)
			ctrlPoints[idx]->show();

		setFocus(Qt::ActiveWindowFocusReason);
		setCursor(Qt::CrossCursor);
	}

	DkWidget::setVisible(visible);
}
}
