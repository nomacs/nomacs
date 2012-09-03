/*******************************************************************************************************
 DkViewPort.h
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

#pragma once

// Qt
#include <QtGui/QDesktopWidget>
#include <QtGui/QGraphicsView>
#include <QtGui/QPrintDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QWidget>
#include <QtGui/QLabel>
#include <QtGui/QInputDialog>
#include <QtGui/QPainterPathStroker>
#include <QtGui/QBitmap>
#include <QtGui/QApplication>
#include <QUrl>
#include <QPrinter>
#include <QtGui/QGradientStops>

// OpenCV
#ifdef WITH_OPENCV

#ifdef Q_WS_WIN
#pragma warning(disable: 4996)
#endif

#endif

// my stuff
#include "DkImage.h"
#include "DkWidgets.h"
#include "DkNetwork.h"
#include "DkSettings.h"

#include "DkMath.h"

#include "DkTransferToolBar.h"

#ifdef DK_DLL
#define DllExport __declspec(dllexport)
#else
#define DllExport
#endif

namespace nmc {

class DkDelayedInfo : public QObject {
	Q_OBJECT

public:
	DkDelayedInfo(int time = 0) {
		timer = new QTimer();
		timer->setSingleShot(true);
		
		if (time)
			timer->start(time);

		connect(timer, SIGNAL(timeout()), this, SLOT(sendInfo()));
	}

	virtual ~DkDelayedInfo() {

		if (timer && timer->isActive())
			timer->stop();

		if (timer)
			delete timer;

		timer = 0;
	}

	void stop() {

		if (timer && timer->isActive())
			timer->stop();
		else
			emit infoSignal(1);
	}

	void setInfo(int time = 1000) {

		if (!timer)
			return;

		timer->start(time);
	}

signals:
	void infoSignal(int time);

protected slots:
	virtual void sendInfo() {
		emit infoSignal(-1);
	}

protected:
	QTimer* timer;

};


class DkDelayedMessage : public DkDelayedInfo {
	Q_OBJECT

public:
	DkDelayedMessage(QString msg  = QString(), int time = 0) : DkDelayedInfo(time) {
		this->msg = msg;
	}

	~DkDelayedMessage() {}

	void stop() {
		
		if (timer && timer->isActive())
			timer->stop();
		else
			emit infoSignal(msg, 1);
	}

	void setInfo(QString& msg, int time = 1000) {

		DkDelayedInfo::setInfo(time);
		this->msg = msg;
	}

signals:
	void infoSignal(QString msg, int time);

protected slots:
	void sendInfo() {
		
		emit infoSignal(msg, -1);
	}

protected:
	QString msg;

};

class DkViewPort;

class DllExport DkControlWidget : public QWidget {
	Q_OBJECT

public:
	
	enum VerPos {top=0, top_info, ver_center, bottom_info, bottom, ver_pos_end};
	enum HorPos {left= 0, hor_center, right, hor_pos_end};

	enum InfoPos {
		center_label,
		bottom_left_label,
		bottom_right_label,
		top_left_label
	};


	DkControlWidget(DkViewPort *parent = 0, Qt::WFlags flags = 0);
	virtual ~DkControlWidget() {};

	void setFullScreen(bool fullscreen);

	DkFilePreview* getFilePreview() {
		return filePreview;
	}

	DkMetaDataInfo* getMetaDataWidget() {
		return metaDataInfo;
	}

	DkOverview* getOverview() {
		return overviewWindow;
	}

	DkPlayer* getPlayer() {
		return player;
	}

	DkFileInfoLabel* getFileInfoLabel() {
		return fileInfoLabel;
	}

	DkHistogram* getHistogram() {
		return histogram;
	}

	int getRating() {
		return rating;
	}

	DkEditableRect* getEditRect() {
		return editRect;
	}

	void stopLabels();
	void showWidgetsSettings();

	void settingsChanged();

public slots:
	void showPreview(bool visible);
	void showMetaData(bool visible);
	void showFileInfo(bool visible);
	void showPlayer(bool visible);
	void showCrop(bool visible);
	void showOverview(bool visible);
	void showHistogram(bool visible);

	void setFileInfo(QFileInfo fileInfo, QSize size = QSize(), bool edited = false);
	void setInfo(QString msg, int time = 3000, int location = center_label);
	virtual void setInfoDelayed(QString msg, bool start = false, int delayTime = 1000);
	virtual void setSpinner(int time = 3000);
	virtual void setSpinnerDelayed(bool start = false, int time = 3000);
	void updateRating(int rating);

	void imageLoaded(bool loaded);

	void update();

protected:

	// events
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);

	//void resizeEvent(QResizeEvent *event);

	// functions
	void init();
	void connectWidgets();

	QWidget* editWidget;
	QWidget* hudWidget;

	DkViewPort* viewport;
	DkEditableRect* editRect;

	DkFilePreview* filePreview;
	DkMetaDataInfo* metaDataInfo;
	DkOverview* overviewWindow;
	DkPlayer* player;
	DkHistogram* histogram;

	DkFileInfoLabel* fileInfoLabel;
	DkRatingLabelBg* ratingLabel;

	DkDelayedMessage* delayedInfo;
	DkDelayedInfo* delayedSpinner;

	DkAnimationLabel* spinnerLabel;
	DkLabelBg* centerLabel;
	DkLabelBg* bottomLabel;
	DkLabelBg* bottomLeftLabel;

	QLabel* wheelButton;

	QPointF enterPos;
	int rating;

};




class DllExport DkBaseViewPort : public QGraphicsView {
	Q_OBJECT

public:
	DkBaseViewPort(QWidget *parent = 0, Qt::WFlags flags = 0);
	virtual ~DkBaseViewPort();

	virtual void release();
	virtual void zoom(float factor = 0.5, QPointF center = QPointF(-1,-1));
	virtual QTransform getWorldMatrix() { 
		return worldMatrix;
	};
	virtual QRect getMainGeometry() {
		return geometry();
	};

	virtual QImage getScaledImage(float factor);

#ifdef WITH_OPENCV
	virtual void setImage(cv::Mat newImg);
#endif

	virtual QImage getImage();

	virtual QRectF getImageViewRect();

signals:
	void enableNoImageSignal(bool enable);
	void showStatusBar(bool show, bool permanent);
//#ifdef DK_DLL
	void keyReleaseSignal(QKeyEvent* event);	// make key presses available
//#endif

public slots:
	virtual void shiftLeft();
	virtual void shiftRight();
	virtual void shiftUp();
	virtual void shiftDown();
	virtual void moveView(QPointF);
	virtual void zoomIn();
	virtual void zoomOut();
	virtual void resetView();
	virtual void fullView();
	virtual void resizeEvent(QResizeEvent* event);
	virtual void paintEvent(QPaintEvent* event);
	virtual void stopBlockZooming();

	virtual void unloadImage();

	virtual void setImage(QImage newImg);

protected:

	virtual void keyPressEvent(QKeyEvent *event);
	virtual void keyReleaseEvent(QKeyEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void wheelEvent(QWheelEvent *event);
	virtual void mouseDoubleClickEvent(QMouseEvent *event);
	virtual void contextMenuEvent(QContextMenuEvent *event);

	QPainter* painter;
	Qt::KeyboardModifier altMod;		// it makes sense to switch these modifiers on linux (alt + mouse moves windows there)
	Qt::KeyboardModifier ctrlMod;

	QWidget *parent;
	QImage imgQt;
	QMap<int, QImage> imgPyramid;

	QTransform imgMatrix;
	QTransform worldMatrix;
	QRectF imgViewRect;
	QRectF viewportRect;
	QRectF imgRect;

	QPointF posGrab;

	bool blockZooming;
	QTimer* zoomTimer;

	// functions
	virtual void draw(QPainter *painter);
	virtual bool imageInside();	// always return false?!
	virtual void updateImageMatrix();
	virtual QTransform getScaledImageMatrix();
	virtual void controlImagePosition(float lb = -1, float ub = -1);
	virtual void centerImage();
	virtual void changeCursor();
};

class DkImageLoader;
class DkLoader;

class DllExport DkViewPort : public DkBaseViewPort {
	Q_OBJECT

public:
	DkViewPort(QWidget *parent = 0, Qt::WFlags flags = 0);
	virtual ~DkViewPort();

	virtual void release();
	
	void zoom(float factor = 0.5, QPointF center = QPointF(-1,-1));

	void setFullScreen(bool fullScreen);
	QTransform getWorldMatrix() { 
		return worldMatrix;
	};

#ifdef WITH_OPENCV
	void setImage(cv::Mat newImg);
#endif

	// getter
	DkImageLoader* getImageLoader();
	DkControlWidget* getController();
	bool isTestLoaded() { return testLoaded; };
	void setVisibleStatusbar(bool visibleStatusbar) {
		this->visibleStatusbar = visibleStatusbar;
	};

signals:
	void sendTransformSignal(QTransform transform, QTransform imgTransform, QPointF canvasSize);
	void windowTitleSignal(QFileInfo file, QSize s = QSize(), bool edited = false);
	void sendNewFileSignal(qint16 op, QString filename = "");
	void sendImageSignal(QImage img, QString title);
	void statusInfoSignal(QString msg);
	void newClientConnectedSignal();

public slots:
	void rotateCW();
	void rotateCCW();
	void rotate180();
	void resetView();
	void fullView();
	void printImage();
	void resizeEvent(QResizeEvent* event);
	virtual void paintEvent(QPaintEvent* event);
	void toggleResetMatrix();
	
	// tcp actions
	void tcpSetTransforms(QTransform worldMatrix, QTransform imgMatrix, QPointF canvasSize);
	void tcpSetWindowRect(QRect rect);
	void tcpSynchronize(QTransform relativeMatrix = QTransform());
	void tcpLoadFile(qint16 idx, QString filename);
	void tcpShowConnections(QList<DkPeer> peers);
	void tcpSendImage();
	
	// file actions
	void loadFile(QFileInfo file, bool silent = false);
	void reloadFile();
	void loadFullFile(bool silent = false);
	void loadNextFile(bool silent = false);
	void loadPrevFile(bool silent = false);
	void loadNextFileFast(bool silent = false);
	void loadPrevFileFast(bool silent = false);
	void loadFileFast(int skipIdx, bool silent = false);
	void loadFile(int skipIdx, bool silent = false);
	void loadFirst();
	void loadLast();
	void loadSkipNext10();
	void loadSkipPrev10();
	void loadLena();
	void unloadImage();
	void fileNotLoaded(QFileInfo file);
	void cropImage(DkRotatingRect rect);

	virtual void updateImage();
	virtual void setEditedImage(QImage newImg);
	virtual void setImage(QImage newImg);
	virtual void setThumbImage(QImage newImg);

	void settingsChanged();

protected:
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void wheelEvent(QWheelEvent *event);
	virtual bool event(QEvent *event);

	QFileInfo thumbFile;
	bool thumbLoaded;
	bool testLoaded;
	bool visibleStatusbar;

	QRectF oldImgRect;
	QRectF oldImgViewRect;
	QTransform oldWorldMatrix;
	QTransform oldImgMatrix;

	QImage imgBg;

	DkControlWidget* controller;
	DkImageLoader* loader;

	// functions
	void drawPolygon(QPainter *painter, QPolygon *polygon);
	virtual void drawBackground(QPainter *painter);
	virtual void controlImagePosition(float lb = -1, float ub = -1);
	virtual void updateImageMatrix();
	void showZoom();
	QPoint newCenter(QSize s);	// for frameless
	void toggleLena();
	void getPixelInfo(const QPoint& pos);

};

class DllExport DkViewPortFrameless : public DkViewPort {
	Q_OBJECT

public:
	DkViewPortFrameless(QWidget *parent = 0, Qt::WFlags flags = 0);
	virtual ~DkViewPortFrameless();

	void release();
	void addStartActions(QAction *startAction, QIcon *startIcon = 0);
	virtual void zoom(float factor = 0.5, QPointF center = QPointF(-1,-1));
	virtual void setMainGeometry(const QRect &geometry) {
		mainScreen = geometry;
	};

	virtual QRect getMainGeometry() {
		return mainScreen;
	};

public slots:
	virtual void setImage(QImage newImg);
	virtual void resetView();
	virtual void paintEvent(QPaintEvent* event);
	virtual void moveView(QPointF);

protected:
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void resizeEvent(QResizeEvent* event);

	// variables
	QVector<QAction*> startActions;
	QVector<QIcon*> startIcons;
	QVector<QRectF> startActionsRects;
	QVector<QPixmap> startActionsIcons;
	QRect mainScreen;

	// functions
	QTransform getScaledImageMatrix();
	virtual void updateImageMatrix();
	virtual void draw(QPainter *painter);
	void drawFrame(QPainter* painter);
	virtual void drawBackground(QPainter *painter);
	void controlImagePosition(float lb = -1, float ub = -1);
	virtual void centerImage();
};

class DllExport DkViewPortContrast : public DkViewPort {
	Q_OBJECT

public:
	DkViewPortContrast(QWidget *parent = 0, Qt::WFlags flags = 0);
	virtual ~DkViewPortContrast();

	void release();

signals:
	void tFSliderAdded(qreal pos);
	void imageModeSet(int mode);


public slots:
	//TODO: remove the functions, which are not used anymore:
	void changeChannel(int channel);
	void changeColorTable(QGradientStops stops);
	void pickColor();
	void enableTF(bool enable);
	QImage getImage();

	virtual void setImage(QImage newImg);

protected:
	virtual void draw(QPainter *painter);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);
private:
	QImage falseColorImg;
	bool drawFalseColorImg;
	bool isColorPickerActive;
	int activeChannel;
	//Mat origImg, cmImg, imgUC3;
		
	QVector<QImage> imgs;
	QVector<QRgb> colorTable;
	void drawImageHistogram();

};


//// custom events --------------------------------------------------------------------
//class DkInfoEvent : public QEvent {
//
//public:
//
//	DkInfoEvent() : QEvent(DkInfoEvent::type()) {};
//
//	DkInfoEvent(QString& msg, int time = 3000, int infoType = DkInfoLabel::center_label) : QEvent(DkInfoEvent::type()) {
//		this->msg = msg;
//		this->time = time;
//		this->infoType = infoType;
//	}
//
//	virtual ~DkInfoEvent() {}
//
//	QString getMessage() {
//		
//		return msg;
//	}
//
//	int getTime() {
//
//		return time;
//	}
//
//	int getInfoType() {
//
//		return infoType;
//	}
//
//	static QEvent::Type type() {
//		return infoEventType;
//	}
//
//private:
//
//	static QEvent::Type infoEventType;
//	QString msg;
//	int time;
//	int infoType;
//
//};
//
//class DkLoadImageEvent : public QEvent {
//
//public:
//
//	DkLoadImageEvent() : QEvent(DkLoadImageEvent::type()) {
//
//		this->title = "DkNoMacs";
//	}
//
//	DkLoadImageEvent(QImage img, QString title = QString("DkNoMacs"), QString attr = QString()) : QEvent(DkLoadImageEvent::type()) {
//		this->img = img;
//		this->title = title;
//		this->attr = attr;
//	};
//
//	virtual ~DkLoadImageEvent() {};
//
//	QImage getImage() {
//		return img;	
//	};
//
//	QString getTitle() {
//		return title;
//	};
//
//	QString getAttr() {
//		return attr;
//	};
//
//	static QEvent::Type type() {
//		return eventType;
//	};
//
//private:
//
//	static QEvent::Type eventType;
//	QImage img;
//	QString title;
//	QString attr;
//};

};