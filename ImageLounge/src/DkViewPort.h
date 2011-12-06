/*******************************************************************************************************
 DkViewPort.h
 Created on:	05.05.2011
 
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

#pragma once

// Qt
#include <QtGui/QDesktopWidget>
#include <QtGui/QGraphicsView>
#include <QtGui/QPrintDialog>
#include <QtGui/QWidget>
#include <QtGui/QLabel>
#include <QtGui/QInputDialog>
#include <QtGui/QPainterPathStroker>
#include <QtGui/QBitmap>
#include <QtGui/QApplication>
#include <QUrl>
#include <QPrinter>

// OpenCV
#ifdef WITH_OPENCV
#include <opencv/cv.h>
#endif

// my stuff
#include "DkImage.h"
#include "DkWidgets.h"
#include "DkNetwork.h"
#include "DkSettings.h"

#ifdef DK_DLL
#define DllExport __declspec(dllexport)
#else
#define DllExport
#endif

class DkImageLoader;
class DkLoader;

class DllExport DkViewPort : public QGraphicsView {
	Q_OBJECT

public:
	DkViewPort(QWidget *parent = 0, Qt::WFlags flags = 0);
	virtual ~DkViewPort();

	void release();
	
	void setCenterInfo(QString msg, int time = 3000);
	void setBottomInfo(QString msg, int time = 3000);
	void zoom(float factor = 0.5, QPointF center = QPointF(-1,-1));

	void setFrameless(bool frameless = true);

	void setFullScreen(bool fullScreen);
	QTransform getWorldMatrix() { 
		return worldMatrix;
	};

#ifdef WITH_OPENCV
	void setImage(cv::Mat newImg);
#endif

	// getter
	DkImageLoader* getImageLoader();
	DkPlayer* getPlayer();
	DkOverview* getOverview();
	DkMetaDataInfo* getMetaDataWidget();
	DkFilePreview* getFilePreview();
	DkFileInfoLabel* getFileInfoWidget();
	QImage& getImage();
	bool isTestLoaded() { return testLoaded; };
	void setVisibleStatusbar(bool visibleStatusbar) {this->visibleStatusbar = visibleStatusbar;};

signals:
	void sendTransformSignal(QTransform transform, QTransform imgTransform, QPointF canvasSize);
	void windowTitleSignal(QFileInfo file, QSize s = QSize());
	void sendNewFileSignal(qint16 op, QString filename = "");
	void sendImageSignal(QImage img, QString title);
	void statusInfoSignal(QString msg);
	void enableNoImageSignal(bool enable);
	void newClientConnectedSignal();

#ifdef DK_DLL
	void keyReleaseSignal(QKeyEvent* event);
#endif

public slots:
	void shiftLeft();
	void shiftRight();
	void shiftUp();
	void shiftDown();
	void zoomIn();
	void zoomOut();
	void rotateCW();
	void rotateCCW();
	void rotate180();
	void resetView();
	QRect initialWindow();
	void fullView();
	void printImage();
	void resizeEvent(QResizeEvent* event);
	void paintEvent(QPaintEvent* event);
	void toggleResetMatrix();
	void toggleShowOverview();
	void toggleShowPlayer();
	void showPreview();
	void showExif();
	void showInfo();
	void moveView(QPointF);
	
	// tcp actions
	void tcpSetTransforms(QTransform worldMatrix, QTransform imgMatrix, QPointF canvasSize);
	void tcpSetWindowRect(QRect rect);
	void tcpSynchronize(QTransform relativeMatrix = QTransform());
	void tcpLoadFile(qint16 idx, QString filename);
	void tcpShowConnections(QList<DkPeer> peers);
	void tcpSendImage();
	
	void setTitleLabel(QFileInfo file, int time = -1);
	void setInfo(QString msg, int time = 3000, int location = DkInfoLabel::center_label);
	void updateRating(int rating);
	
	// file actions
	void loadFile(QFileInfo file, bool updateFolder = true, bool silent = false);
	void reloadFile();
	void loadNextFile(bool silent = false);
	void loadPrevFile(bool silent = false);
	void loadFirst();
	void loadLast();
	void loadSkipNext10();
	void loadSkipPrev10();
	void loadLena();
	void unloadImage();
	void fileNotLoaded(QFileInfo file);

	virtual void setImage(QImage newImg);

	void settingsChanged();

protected:
	
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void leaveEvent(QEvent *event);
	void focusOutEvent(QEvent *event);
	void contextMenuEvent(QContextMenuEvent *event);

	QPainter* painter;
	bool testLoaded;
	bool isFrameless;
	bool altKeyPressed;
	bool visibleStatusbar;

	QWidget *parent;
	QImage imgQt;
	QImage imgBg;
	QLabel* wheelButton;

	QTransform imgMatrix;
	QTransform worldMatrix;
	QRectF imgViewRect;
	QRectF viewportRect;
	QRectF imgRect;

	DkImageLoader* loader;
	DkInfoLabel* centerLabel;
	DkInfoLabel* bottomLabel;
	DkInfoLabel* bottomRightLabel;
	DkInfoLabel* topLeftLabel;
	DkFileInfoLabel* fileInfoLabel;

	DkMetaDataInfo* metaDataInfo;
	DkPlayer* player;
	DkFilePreview* filePreview;
	DkRatingLabelBg* ratingLabel;
	DkOverview* overviewWindow;

	//DkBox imgViewRect;
	//DkVector ratio;
	//DkVector mouseCoord;
	QPointF posGrab;
	QPointF enterPos;
	QPoint bottomOffset;
	QPoint topOffset;

	// overview rendering
	QRectF cornerPos;
	float overviewSize;
	float overviewMargin;

	int rating;

	// functions
	virtual void draw(QPainter *painter);
	void drawPolygon(QPainter *painter, QPolygon *polygon);
	void drawFrame(QPainter* painter);
	bool imageInside();
	void updateImageMatrix();
	QTransform getScaledImageMatrix();
	void controlImagePosition(float lb = -1, float ub = -1);
	void centerImage();
	void showZoom();
	virtual void changeCursor();
	virtual void saveXML() {};	// dummy function
	QPoint newCenter(QSize s);	// for frameless
	void setFramelessGeometry(QRect r);
	void toggleLena();
	void getPixelInfo(const QPoint& pos);

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
//	DkLoadImageEvent(QImage& img, QString title = QString("DkNoMacs"), QString attr = QString()) : QEvent(DkLoadImageEvent::type()) {
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

