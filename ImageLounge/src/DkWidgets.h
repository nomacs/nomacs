/*******************************************************************************************************
 DkWidgets.h
 Created on:	17.05.2011
 
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

#pragma once

#include <QApplication>
#include <QtGui/QRadioButton>
#include <QtGui/QAction>
#include <QtGui/QBoxLayout>
#include <QtGui/QDialog>
#include <QtGui/QGraphicsBlurEffect>
#include <QtGui/QGraphicsPixmapItem>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QPushButton>
#include <QtGui/QMouseEvent>
#include <QtGui/QShortcut>
#include <QtGui/QToolButton>
#include <QtGui/QComboBox>
#include <QtGui/QMessageBox>
#include <QStringBuilder>
#include <QPointer>
#include <QTimer>
#include <QMap>
#include <QDesktopServices>
#include <QVector2D>

// gif animation label -----
#include <QVBoxLayout>
#include <QLabel>
#include <QMovie>
// gif animation label -----

#include "DkImage.h"
#include "DkNetwork.h"
#include "DkSettings.h"
#include "DkMath.h"

#ifdef Q_WS_WIN
#include <ShObjIdl.h>
#include <ShlObj.h>
#endif

namespace nmc {
class DkThumbNail;

class DkWidget : public QWidget {
	Q_OBJECT

public:
	DkWidget(QWidget* parent = 0, Qt::WFlags flags = 0);

	void registerAction(QAction* action) {
		connect(this, SIGNAL(visibleSignal(bool)), action, SLOT(setChecked(bool)));
	};

	void block(bool blocked) {
		this->blocked = blocked;
		setVisible(false);
	};

	void setDisplaySettings(QBitArray* displayBits) {
		displaySettingsBits = displayBits;
	};

	bool getCurrentDisplaySetting() {
		
		if (!displaySettingsBits)
			return false;

		if (DkSettings::App::currentAppMode < 0 || DkSettings::App::currentAppMode >= displaySettingsBits->size()) {
			qDebug() << "[WARNING] illegal app mode: " << DkSettings::App::currentAppMode;
			return false;
		}

		qDebug() << objectName();
		qDebug() << "default: " << displaySettingsBits->testBit(0);
		qDebug() << "frameless: " << displaySettingsBits->testBit(1);
		qDebug() << "contrast: " << displaySettingsBits->testBit(2);
		qDebug() << "default F: " << displaySettingsBits->testBit(3);
		qDebug() << "fameless F: " << displaySettingsBits->testBit(4);
		qDebug() << "contrast F: " << displaySettingsBits->testBit(5);

		return displaySettingsBits->testBit(DkSettings::App::currentAppMode);

	};


signals:
	void visibleSignal(bool visible);

public slots:
	void show();
	void hide();
	void setVisible(bool visible);

	void animateOpacityUp();
	void animateOpacityDown();

protected:

	QColor bgCol;
	bool blocked;
	bool hiding;
	bool showing;

	QGraphicsOpacityEffect* opacityEffect;
	QBitArray* displaySettingsBits;

	// functions
	void init();
};

class DkLabel : public QLabel {
	Q_OBJECT

public:
	DkLabel(QWidget* parent = 0, const QString& text = QString());
	virtual ~DkLabel() {
		if (timer) delete timer;
		timer = 0;
	};

	virtual void showTimed(int time = 3000);
	virtual void setText(const QString msg, int time = 3000);
	QString getText();
	void setFontSize(int fontSize);
	void stop();
	
	void block(bool blocked) {
		this->blocked = blocked;
	};
	
	void setFixedWidth(int fixedWidth);
	
	void setMargin(const QPoint& margin) {
		this->margin = margin;
		updateStyleSheet();
	};

	void registerAction(QAction* action) {
		connect(this, SIGNAL(visibleSignal(bool)), action, SLOT(setChecked(bool)));
	};

	void setDisplaySettings(QBitArray* displayBits) {
		displaySettingsBits = displayBits;
	};

	bool getCurrentDisplaySetting() {

		if (!displaySettingsBits)
			return false;

		if (DkSettings::App::currentAppMode < 0 || DkSettings::App::currentAppMode >= displaySettingsBits->size()) {
			qDebug() << "[WARNING] illegal app mode: " << DkSettings::App::currentAppMode;
			return false;
		}

		return displaySettingsBits->testBit(DkSettings::App::currentAppMode);
	};


public slots:
	virtual void hide();
	virtual void setVisible(bool visible) {
	
		QLabel::setVisible(visible);
		emit visibleSignal(visible);

		if (displaySettingsBits && displaySettingsBits->size() > DkSettings::App::currentAppMode) {
			displaySettingsBits->setBit(DkSettings::App::currentAppMode, visible);
		}
	};

signals:
	bool visibleSignal(bool visible);

protected:
	QWidget* parent;
	QTimer* timer;
	QString text;
	QColor textCol;
	int defaultTime;
	int fontSize;
	int time;
	int fixedWidth;
	QPoint margin;
	bool blocked;
	QColor bgCol;
	QBitArray* displaySettingsBits;

	virtual void init();
	virtual void paintEvent(QPaintEvent *event);
	virtual void draw(QPainter* painter);

	// for my children...
	virtual void drawBackground(QPainter* painter) {};
	virtual void setTextToLabel();
	virtual void updateStyleSheet();
};

class DkLabelBg : public DkLabel {
	Q_OBJECT

public:
	DkLabelBg(QWidget* parent = 0, const QString& text = QString());
	virtual ~DkLabelBg() {};

protected:
	virtual void updateStyleSheet();
};

class DkGradientLabel : public DkLabel {
	Q_OBJECT

public:
	DkGradientLabel(QWidget* parent = 0, const QString& text = QString());
	virtual ~DkGradientLabel() {};

protected:
	void init();
	void drawBackground(QPainter* painter);
	void updateStyleSheet();

	QImage gradient;
	QImage end;

};

/**
 * This label fakes the DkWidget behavior.
 * (allows for registering actions + fades in and out)
 * we need this class too, since we cannot derive from DkLabel & DkWidget
 * at the same time -> both have QObject as common base class.
 **/
class DkFadeLabel : public DkLabel {
	Q_OBJECT

public:
	DkFadeLabel(QWidget* parent = 0, const QString& text = QString());
	virtual ~DkFadeLabel() {};

	void block(bool blocked) {
		this->blocked = blocked;
		setVisible(false);
	};

signals:
	void visibleSignal(bool visible);

public slots:
	void show();
	void hide();
	void setVisible(bool visible);

	void animateOpacityUp();
	void animateOpacityDown();

protected:

	QColor bgCol;
	bool blocked;
	bool hiding;
	bool showing;

	QGraphicsOpacityEffect *opacityEffect;

	// functions
	void init();

};

class DkButton : public QPushButton {
	Q_OBJECT

public:
	DkButton ( QWidget * parent = 0 );
	DkButton ( const QString & text, QWidget * parent = 0 );
	DkButton ( const QIcon & icon, const QString & text, QWidget * parent = 0 );
	DkButton ( const QIcon & checkedIcon, const QIcon & uncheckedIcon, const QString & text, QWidget * parent = 0 );
	~DkButton() {};

	void adjustSize();
	void setFixedSize(QSize size);

	bool keepAspectRatio;

protected:
	QIcon checkedIcon;
	QIcon uncheckedIcon;
	bool mouseOver;
	QSize mySize;
	
	// functions
	void init();

	void paintEvent(QPaintEvent * event);
	void focusInEvent(QFocusEvent * event);
	void focusOutEvent(QFocusEvent * event);
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
	QPixmap createSelectedEffect(QPixmap* pm);

};

class DkRatingLabel : public DkWidget {
	Q_OBJECT

public:
	enum {
		rating_1,
		rating_2,
		rating_3,
		rating_4,
		rating_5,
		rating_0,	// no image for that one
	};

	DkRatingLabel(int rating = 0, QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~DkRatingLabel() {};

	void setRating(int rating) {
		this->rating = rating;
		updateRating();
	};

	virtual void changeRating(int newRating) {
		rating = newRating;
		updateRating();
		emit newRatingSignal(rating);
	};

	int getRating() {
		return rating;
	};

signals:
	void newRatingSignal(int rating = 0);

public slots:
	void rating0() {
		changeRating(0);
	};

	void rating1() {
		changeRating(1);
	};

	void rating2() {
		changeRating(2);
	};

	void rating3() {
		changeRating(3);
	};

	void rating4() {
		changeRating(4);
	};

	void rating5() {
		changeRating(5);
	};

protected:
	QVector<DkButton*> stars;
	QBoxLayout* layout;
	int rating;
	QPoint margin;
	int spacing;

	void updateRating() {
		
		for (int idx = 0; idx < stars.size(); idx++) {
			stars[idx]->setChecked(idx < rating);
		}
	};

	virtual void init();
};

class DkRatingLabelBg : public DkRatingLabel {
	Q_OBJECT

public:
	DkRatingLabelBg(int rating = 0, QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~DkRatingLabelBg() {
		if (hideTimer) delete hideTimer;
		hideTimer = 0;
	};

	void changeRating(int newRating) {
		DkRatingLabel::changeRating(newRating);
		show();
		hideTimer->start();
	}

	QVector<QAction*> getActions() {
		return actions;
	};


protected:
	QVector<QAction*> actions;
	QTimer* hideTimer;
	int timeToDisplay;
	
	virtual void paintEvent(QPaintEvent *event);
};

// TODO: check why it's not working with DkFadeLabel
class DkFileInfoLabel : public DkLabel {
	Q_OBJECT

public:
	DkFileInfoLabel(QWidget* parent = 0);
	~DkFileInfoLabel() {};

	void createLayout();
	void updateInfo(const QFileInfo& file, const QString& date, const int rating);
	void updateTitle(const QFileInfo& file);
	void updateDate(const QString& date = QString());
	void updateRating(const int rating);
	void setVisible(bool visible);
	DkRatingLabel* getRatingLabel();

protected:
	QPoint offset;
	QFileInfo file;

	QBoxLayout* layout;
	QWidget* parent;
	QLabel* title;
	QLabel* date;
	DkRatingLabel* rating;
	int minWidth;

	void updateWidth();
};

class DkPlayer : public DkWidget {
	Q_OBJECT

public:
	enum {
		play_action,	// if more actions are to be assigned
	};

	DkPlayer(QWidget* parent = 0);
	~DkPlayer() {};

	void draw(QPainter* painter);
	void setTimeToDisplay(int ms = 1000);
	void show(int ms = 0);

	QVector<QAction*> getActions() {
		return actions;
	};

signals:
	void nextSignal(bool silent = true);
	void previousSignal(bool silent = true);

public slots:
	void play(bool play) {
		
		if (play != playing)	// emulate a click
			playButton->setChecked(play);

		playing = play;

		if (play) {
			displayTimer->start();
			hideTimer->start();
		}
		else
			displayTimer->stop();
	};

	void togglePlay() {
		
		show();
		playing = !playing;
		playButton->click();
	};

	void startTimer() {
		if (playing) {
			displayTimer->setInterval(DkSettings::SlideShow::time*1000);	// if it was updated...
			displayTimer->start();
		}
	};

	void next() {
		hideTimer->stop();
		emit nextSignal();
	};
	void previous() {
		hideTimer->stop();
		emit previousSignal();
	};


protected:
	void resizeEvent(QResizeEvent *event);
	void init();
	bool playing;

	int timeToDisplay;
	QTimer* displayTimer;
	QTimer* hideTimer;

	DkButton* previousButton;
	DkButton* nextButton;
	DkButton* playButton;
	QWidget* container;

	QVector<QAction*> actions;

};

class DkFilePreview : public DkWidget {
	Q_OBJECT

public:
	DkFilePreview(QWidget* parent = 0, Qt::WFlags flags = 0);
	
	~DkFilePreview() {
		
		if (thumbsLoader) {
			thumbsLoader->stop();
			thumbsLoader->wait();
			delete thumbsLoader;
		}
	};

	void setCurrentDx(float dx) {
		this->currentDx = dx;
	};

	QTimer* getMoveImageTimer() {
		return moveImageTimer;
	};

	void setVisible(bool visible) {

		if (visible)
			indexDir();

		DkWidget::setVisible(visible);
	}

public slots:
	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void leaveEvent(QEvent *event);
	void moveImages();
	void updateDir(QFileInfo file, bool force = false);

signals:
	void loadFileSignal(QFileInfo file);
	void loadThumbsSignal(int start, int end);


private:
	std::vector<DkThumbNail> thumbs;
	QWidget* parent;
	QTransform worldMatrix;
	QPoint lastMousePos;
	QPoint enterPos;
	QTimer* moveImageTimer;
	QRectF bufferDim;
	QVector<QRectF> thumbRects;
	QLinearGradient leftGradient;
	QLinearGradient rightGradient;
	QPixmap selectionGlow;
	QPixmap currentImgGlow;
	QImage stubImg;
	int mouseTrace;
	DkThumbsLoader* thumbsLoader;
	QFileInfo currentFile;
	int currentFileIdx;
	int oldFileIdx;
	float currentDx;
	QLabel* wheelButton;
	QDir thumbsDir;

	DkGradientLabel* fileLabel;

	int selected;
	float winPercent;
	float borderTrigger;

	int xOffset;
	int yOffset;
	int minHeight;
	
	void init();
	void indexDir(bool force = true);
	void drawThumbs(QPainter* painter);
	void drawFadeOut(QLinearGradient gradient, QRectF imgRect, QImage *img);
	void createSelectedEffect(QImage img, QColor col);
	void createCurrentImgEffect(QImage img, QColor col);
};


class DkOverview : public DkWidget {
	Q_OBJECT

public:
	DkOverview(QWidget * parent = 0, Qt::WindowFlags f = 0);
	~DkOverview() {};

	void resize(int w, int h);
	void resize(const QSize& size);

	void setImage(QImage img) {
		this->img = img;
		resizeImg();
	};

	void setTransforms(QTransform* worldMatrix, QTransform* imgMatrix){
		this->worldMatrix = worldMatrix;
		this->imgMatrix = imgMatrix;
	};

	void setViewPortRect(QRectF viewPortRect) {
		this->viewPortRect = viewPortRect;	
	};

signals:
	void moveViewSignal(QPointF dxy);
	void sendTransformSignal();

protected:
	QImage img;
	QImage imgT;
	QTransform* scaledImgMatrix;
	QTransform* worldMatrix;
	QTransform* imgMatrix;
	QRectF viewPortRect;
	QPointF posGrab;
	QPointF enterPos;
	QWidget* parent;

	void resizeImg();
	void paintEvent(QPaintEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void resizeEvent(QResizeEvent* event);
	QTransform getScaledImageMatrix();
};

class DkMetaDataInfo : public DkWidget {
	Q_OBJECT

public:

	static QString sCamDataTags;
	static QString sDescriptionTags;
		
	DkMetaDataInfo(QWidget* parent = 0);
	~DkMetaDataInfo() {};

	void draw(QPainter* painter);
	void createLabels();
	void resizeEvent(QResizeEvent *resizeW);
	void getResolution(float &xResolution, float &yResolution);
	QString getGPSCoordinates();
	
	//DkMetaData* getMetaData() {
	//	return &metaData;
	//};
	
signals:
	void enableGpsSignal(bool);
	
public slots:
	void setFileInfo(QFileInfo file, QSize s) {
		
		this->file = file;
		imgSize = s;
		worldMatrix = QTransform();

		//DkTimer dt;
		readTags();
			
		emit enableGpsSignal(!getGPSCoordinates().isEmpty());

		if (isVisible())
			createLabels();
		//qDebug() << "reading tags & creating labels: " << QString::fromStdString(dt.getTotal());
	}

	void setRating(int rating);
	void setResolution(int xRes, int yRes);
	void updateLabels();
	void mouseMoveEvent(QMouseEvent *event);
	void setVisible(bool visible) {

		if (visible)
			createLabels();

		qDebug() << "[DkMetaData] setVisible: " << visible;

		DkWidget::setVisible(visible);
	};

protected:
	void init();
	void readTags();
	void layoutLabels();
	void paintEvent(QPaintEvent *event);
	

	QWidget* parent;
	QPoint lastMousePos;
	QTransform worldMatrix;
	int exifHeight;
	int fontSize;
	int textMargin;
	int numLines;
	int maxCols;
	int numLabels;
	int minWidth;
	int gradientWidth;

	int yMargin;
	int xMargin;

	float currentDx;

	QVector<int> maxLenLabel;

	QFileInfo file;
	//DkMetaData metaData;
	QVector<DkLabel *> pLabels;
	QVector<DkLabel *> pValues;
	QSize imgSize;

	QStringList camDTags;
	QStringList camDValues;

	QStringList descTags;
	QStringList descValues;

	QRect leftGradientRect;
	QLinearGradient leftGradient;
	QRect rightGradientRect;
	QLinearGradient rightGradient;

	QMap<int, int> mapIptcExif;
	QStringList exposureModes;
	QMap<int, QString> flashModes;

};

// TODO: move to DkMath
class DkRotatingRect {

public:
	DkRotatingRect(QRectF rect = QRect()) {

		if (rect.isEmpty()) {

			for (int idx = 0; idx < 4; idx++)
				this->rect.push_back(QPointF());
		}
		else
			this->rect = rect;

	};
	
	virtual ~DkRotatingRect() {};

		friend std::ostream& operator<<(std::ostream& s, DkRotatingRect& r){

		return r.put(s);
	};

	bool isEmpty() {

		if (rect.size() < 4)
			return true;

		QPointF lp = rect[0]; 
		for (int idx = 1; idx < rect.size(); idx++) {
			
			if (lp != rect[idx]) {
				return false;
			}
			lp = rect[idx];
		}
		return true;
	};

	void setAllCorners(QPointF &p) {
		
		for (int idx = 0; idx < rect.size(); idx++)
			rect[idx] = p;

	};

	DkVector getDiagonal(int cIdx) {

		DkVector c0 = rect[cIdx % 4];
		DkVector c2 = rect[(cIdx+2) % 4];

		return c2 - c0;
	};

	QCursor cpCursor(int idx) {

		double angle = 0;

		if (idx >= 0 && idx < 4) {

			// this seems a bit complicated...
			// however the points are not necessarily stored in clockwise order...
			DkVector e1 = rect[(idx+1) % 4] - rect[idx];
			DkVector e2 = rect[(idx+3) % rect.size()] - rect[idx];
			e1.normalize();
			e2.normalize();
			DkVector rv = e1-e2;
			rv = rv.normalVec();
			angle = rv.angle();
		}
		else {
			DkVector edge = rect[(idx+1) % 4] - rect[idx % 4];
			angle = edge.normalVec().angle();	// the angle of the normal vector
		}

		angle = DkMath::normAngleRad(angle, -CV_PI/8.0, 7.0*CV_PI/8.0);

		if (angle > 5.0*CV_PI/8.0)
			return QCursor(Qt::SizeBDiagCursor);
		else if (angle > 3.0*CV_PI/8.0)
			return QCursor(Qt::SizeVerCursor);
		else if (angle > CV_PI/8.0)
			return QCursor(Qt::SizeFDiagCursor);
		else
			return QCursor(Qt::SizeHorCursor);

	};

	void updateCorner(int cIdx, QPointF nC, DkVector oldDiag = DkVector()) {

		// index does not exist
		if (cIdx < 0 || cIdx >= rect.size()*2)
			return;

		if (rect[(cIdx+1) % 4] == rect[(cIdx+3) % 4]) {
			QPointF oC = rect[(cIdx+2) % 4];	// opposite corner
			rect[cIdx] = nC;
			rect[(cIdx+1) % 4] = QPointF(nC.x(), oC.y());
			rect[(cIdx+3) % 4] = QPointF(oC.x(), nC.y());
		}
		// these indices indicate the control points on edges
		else if (cIdx >= 4 && cIdx < 8) {

			DkVector c0 = rect[cIdx % 4];
			DkVector n = (rect[(cIdx+1) % 4] - c0).normalVec();
			n.normalize();

			// compute the offset vector
			DkVector oV = n * n.scalarProduct(nC-c0);

			rect[cIdx % 4] = (rect[cIdx % 4] + oV).getQPointF();
			rect[(cIdx+1) % 4] = (rect[(cIdx+1) % 4] + oV).getQPointF();
		}
		else {

			// we have to update the n-1 and n+1 corner
			DkVector cN = nC;
			DkVector c0 = rect[cIdx];
			DkVector c1 = rect[(cIdx+1) % 4];
			DkVector c2 = rect[(cIdx+2) % 4];
			DkVector c3 = rect[(cIdx+3) % 4];
			
			if (!oldDiag.isEmpty()) {
				DkVector dN = oldDiag.normalVec();
				dN.normalize();

				float d = dN*(cN-c2);
				cN += (dN*-d);
			}

			// new diagonal
			float diagLength = (c2-cN).norm();
			float diagAngle = (c2-cN).angle();

			// compute the idx-1 corner
			float c1Angle = (c1-c0).angle();
			float newLength = cos(c1Angle - diagAngle)*diagLength;
			DkVector nc1 = DkVector((newLength), 0);
			nc1.rotate(-c1Angle);

			// compute the idx-3 corner
			float c3Angle = (c3-c0).angle();
			newLength = cos(c3Angle - diagAngle)*diagLength;
			DkVector nc3 = DkVector((newLength), 0);
			nc3.rotate(-c3Angle);
			
			rect[(cIdx+1) % 4] = (nc1+cN).getQPointF();			
			rect[(cIdx+3) % 4] = (nc3+cN).getQPointF();
			rect[cIdx] = cN.getQPointF();
		}
	};

	QPolygonF& getPoly() {

		return rect;
	};

	void setPoly(QPolygonF &poly) {
		
		rect = poly;
	};

	QPolygonF getClosedPoly() {

		if (rect.isEmpty())
			return QPolygonF();

		QPolygonF closedPoly = rect;
		closedPoly.push_back(closedPoly[0]);
		
		return closedPoly;
	};

	QPointF getCenter() {

		if (rect.empty())
			return QPointF();

		DkVector c1 = rect[0];
		DkVector c2 = rect[2];
		
		return ((c2-c1)*0.5f + c1).getQPointF();
	};

	double getAngle() {
		
		// default upper left corner is 0
		DkVector xV = rect[3] - rect[0];
		return xV.angle();
	};

	void getTransform(QTransform& tForm, QPointF& size) {

		if (rect.size() < 4)
			return;

		// default upper left corner is 0
		DkVector xV = DkVector(rect[3] - rect[0]).round();
		DkVector yV = DkVector(rect[1] - rect[0]).round();

		QPointF ul = QPointF(qRound(rect[0].x()), qRound(rect[0].y()));
		size = QPointF(xV.norm(), yV.norm());

		qDebug() << xV.getQPointF();
		qDebug() << "size: " << size;


		double angle = xV.angle();
		angle = DkMath::normAngleRad(angle, -CV_PI, CV_PI);

		if (abs(angle) > DBL_EPSILON)
			qDebug() << "angle is > eps...";

		// switch width/height for /\ and \/ quadrants
		if (abs(angle) > CV_PI*0.25 && abs(angle) < CV_PI*0.75) {
			float x = size.x();
			size.setX(size.y());
			size.setY(x);
		}

		//TODO: fix bug #132

		// invariance -> user does not want to make a difference between an upside down rect
		if (angle > CV_PI*0.25 && angle < CV_PI*0.75) {
			angle -= CV_PI*0.5;
			ul = rect[1];
		}
		else if (angle > -CV_PI*0.75 && angle < -CV_PI*0.25) {
			angle += CV_PI*0.5;
			ul = rect[3];
		}
		else if (angle >= CV_PI*0.75 || angle <= -CV_PI*0.75) {
			angle += CV_PI;
			ul = rect[2];
		}

		tForm.rotateRadians(-angle);
		tForm.translate(qRound(-ul.x()), qRound(-ul.y()));	// round guarantees that pixels are not interpolated

	};

protected:

	virtual std::ostream& put(std::ostream& s) {

		s << "DkRotatingRect: ";
		for (int idx = 0; idx < rect.size(); idx++) {
			DkVector vec = DkVector(rect[idx]);
			s << vec << ", ";
		}

		return s;
	};

	QPolygonF rect;
};

class DkTransformRect : public QWidget {
	Q_OBJECT

public:
	
	DkTransformRect(int idx = -1, DkRotatingRect* rect = 0, QWidget* parent = 0, Qt::WindowFlags f = 0);
	virtual ~DkTransformRect() {};

	void draw(QPainter *painter);
	
	QPointF getCenter() {
		return QPointF(size.width()*0.5f, size.height()*0.5f);
	};

signals:
	void ctrlMovedSignal(int, QPointF, bool);
	void updateDiagonal(int);

protected:

	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void enterEvent(QEvent *event);
	void init();
	
	DkRotatingRect* rect;
	QPointF initialPos;
	QPointF posGrab;
	int parentIdx;
	QSize size;
};


class DkEditableRect : public DkWidget {
	Q_OBJECT

public:

	enum {
		do_nothing,
		initializing,
		rotating,
		moving,
		scaling
	};

	DkEditableRect(QRectF rect = QRect(), QWidget* parent = 0, Qt::WindowFlags f = 0);
	virtual ~DkEditableRect() {};

	void reset();

	void setWorldTransform(QTransform *worldTform) {
		this->worldTform = worldTform;
	};	

	void setImageTransform(QTransform *imgTform) {

		this->imgTform = imgTform;
	};

	void setImageRect(QRectF* imgRect) {
	
		this->imgRect = imgRect;
	};

	void setVisible(bool visible);

signals:
	void enterPressedSignal(DkRotatingRect cropArea);

public slots:
	void updateCorner(int idx, QPointF point, bool isShiftDown);
	void updateDiagonal(int idx);

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
	
	void paintEvent(QPaintEvent *event);

	QPointF map(const QPointF &posM);

	int state;

	QTransform *imgTform;
	QTransform *worldTform;
	QTransform tTform;
	QTransform rTform;
	QPointF posGrab;
	DkVector oldDiag;

	QWidget* parent;
	DkRotatingRect rect;
	QPen pen;
	QBrush brush;
	QVector<DkTransformRect*> ctrlPoints;
	QCursor rotatingCursor;
	QRectF* imgRect;

};

/**
 * DkAnimationLabel
 * This code is based on: http://www.developer.nokia.com/Community/Wiki/CS001434_-_Creating_a_loading_animation_with_GIF,_QMovie,_and_QLabel
 *
 * Uses animation from the path
 * to display it in a DkLabel.
 */
class DkAnimationLabel : public DkLabel {

public:
	DkAnimationLabel(QString animationPath = QString(), QWidget* parent = 0);
	DkAnimationLabel(QString animationPath, QSize size, QWidget* parent);
	virtual ~DkAnimationLabel();

	virtual void showTimed(int time = 3000);
	virtual void hide();
	void halfSize();

private:
	
	QSize margin;
	QPointer<QMovie> animation;

	void init(const QString& animationPath, const QSize& size);

};

class DkColorChooser : public QWidget {
	Q_OBJECT

public:
	DkColorChooser(QColor defaultColor = QColor(), QString text = "Color", QWidget* parent = 0, Qt::WindowFlags flags = 0);
	virtual ~DkColorChooser() {};

	void setColor(QColor& color);
	QColor getColor();

public slots:
	void on_resetButton_clicked();
	void on_colorButton_clicked();
	void on_colorDialog_accepted();

protected:
	QColorDialog* colorDialog;
	QPushButton* colorButton;

	QColor defaultColor;
	QString text;

	void init();
};

};