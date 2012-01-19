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
#include <QtGui/QMessageBox>
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

#ifdef WITH_OPENCV
	virtual void setImage(cv::Mat newImg);
#endif

	virtual QImage& getImage();

signals:
	void enableNoImageSignal(bool enable);

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
	virtual void leaveEvent(QEvent *event);
	virtual void focusOutEvent(QEvent *event);
	virtual void contextMenuEvent(QContextMenuEvent *event);

	QPainter* painter;
	bool altKeyPressed;

	QWidget *parent;
	QImage imgQt;

	QTransform imgMatrix;
	QTransform worldMatrix;
	QRectF imgViewRect;
	QRectF viewportRect;
	QRectF imgRect;

	QPointF posGrab;
	QPointF enterPos;

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
	
	void setCenterInfo(QString msg, int time = 3000);
	void setBottomInfo(QString msg, int time = 3000);
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
	DkPlayer* getPlayer();
	DkOverview* getOverview();
	DkMetaDataInfo* getMetaDataWidget();
	DkFilePreview* getFilePreview();
	DkFileInfoLabel* getFileInfoWidget();
	bool isTestLoaded() { return testLoaded; };
	void setVisibleStatusbar(bool visibleStatusbar) {
		this->visibleStatusbar = visibleStatusbar;
	};

signals:
	void sendTransformSignal(QTransform transform, QTransform imgTransform, QPointF canvasSize);
	void windowTitleSignal(QFileInfo file, QSize s = QSize());
	void sendNewFileSignal(qint16 op, QString filename = "");
	void sendImageSignal(QImage img, QString title);
	void statusInfoSignal(QString msg);
	void newClientConnectedSignal();

#ifdef DK_DLL
	void keyReleaseSignal(QKeyEvent* event);
#endif

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
	void toggleShowOverview();
	void toggleShowPlayer();
	void showPreview();
	void showExif();
	void showInfo();
	
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
	void toggleCropImageWidget(bool croping);
	void cropImage(DkRotatingRect rect);

	virtual void setImage(QImage newImg);

	void settingsChanged();

protected:
	
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);

	bool testLoaded;
	bool visibleStatusbar;
	bool isCropActive;

	QImage imgBg;
	QLabel* wheelButton;

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

	DkEditableRect* editRect;

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
	void addStartActions(QAction *startAction);
	virtual void zoom(float factor = 0.5, QPointF center = QPointF(-1,-1));

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
	QVector<QRectF> startActionsRects;
	QVector<QPixmap> startActionsIcons;

	// functions
	QTransform getScaledImageMatrix();
	virtual void updateImageMatrix();
	virtual void draw(QPainter *painter);
	void drawFrame(QPainter* painter);
	virtual void drawBackground(QPainter *painter);
	void controlImagePosition(float lb = -1, float ub = -1);

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

