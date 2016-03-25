/*******************************************************************************************************
 DkViewPort.h
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

#pragma once

#include "DkBaseViewPort.h"
#include "DkImageContainer.h"
#include "DkTimer.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QTimer>	// needed to construct mTimers
#pragma warning(pop)		// no warnings from includes - end

#ifndef DllGuiExport
#ifdef DK_GUI_DLL_EXPORT
#define DllGuiExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllGuiExport Q_DECL_IMPORT
#else
#define DllGuiExport Q_DECL_IMPORT
#endif
#endif

#pragma warning(disable: 4275)	// no dll interface of base class

// OpenCV
#ifdef WITH_OPENCV
#ifdef Q_OS_WIN
#pragma warning(disable: 4996)
#endif
#endif

class QVBoxLayout;

namespace nmc {

// some dummies
class DkImageLoader;
class DkLoader;
class DkControlWidget;
class DkPeer;
class DkRotatingRect;
class DkPluginInterface;
class DkPluginContainer;

class DllGuiExport DkViewPort : public DkBaseViewPort {
	Q_OBJECT

public:
	DkViewPort(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	virtual ~DkViewPort();

	virtual void release();
	
	void zoom(float factor = 0.5, QPointF center = QPointF(-1,-1));

	void setFullScreen(bool fullScreen);
		
	QTransform getWorldMatrix() { 
		return mWorldMatrix;
	};

	QTransform* getWorldMatrixPtr() {
		return &mWorldMatrix;
	};

	QTransform* getImageMatrixPtr() {
		return &mImgMatrix;
	};

	void setPaintWidget(QWidget* widget, bool removeWidget);

#ifdef WITH_OPENCV
	void setImage(cv::Mat newImg);
#endif

	// getter
	QSharedPointer<DkImageContainerT> imageContainer() const;
	void setImageLoader(QSharedPointer<DkImageLoader> newLoader);
	DkControlWidget* getController();
	bool isTestLoaded() { return mTestLoaded; };
	
	QString getCurrentPixelHexValue();
	QPoint mapToImage(const QPoint& windowPos) const;
	
	void connectLoader(QSharedPointer<DkImageLoader> loader, bool connectSignals = true);

	// fun
	void toggleDissolve();

signals:
	void sendTransformSignal(QTransform transform, QTransform imgTransform, QPointF canvasSize) const;
	void sendNewFileSignal(qint16 op, QString filename = "") const;
	void sendImageSignal(QImage img, QString title) const;
	void newClientConnectedSignal(bool connect, bool local) const;
	void movieLoadedSignal(bool isMovie) const;
	void infoSignal(const QString& msg) const;	// needed to forward signals
	void addTabSignal(const QString& filePath) const;
	void zoomSignal(float zoomLevel) const;
	void mouseClickSignal(QMouseEvent* event, QPoint imgPos) const;

public slots:
	void rotateCW();
	void rotateCCW();
	void rotate180();
	void resetView();
	void zoomToFit();
	void fullView();
	void resizeEvent(QResizeEvent* event);
	void toggleResetMatrix();
	void zoomTo(float zoomLevel, const QPoint& pos = QPoint(-1, -1));
	
	// tcp actions
	void tcpSetTransforms(QTransform worldMatrix, QTransform imgMatrix, QPointF canvasSize);
	void tcpSetWindowRect(QRect rect);
	void tcpSynchronize(QTransform relativeMatrix = QTransform());
	void tcpForceSynchronize();
	void tcpLoadFile(qint16 idx, QString filename);
	void tcpShowConnections(QList<DkPeer*> peers);
	void tcpSendImage(bool silent = false);
	
	// file actions
	void loadFile(const QString& filePath);
	void reloadFile();
	void loadNextFileFast();
	void loadPrevFileFast();
	void loadFileFast(int skipIdx);
	void loadFile(int skipIdx);
	void loadFirst();
	void loadLast();
	void loadSkipNext10();
	void loadSkipPrev10();
	void loadLena();
	bool unloadImage(bool fileChange = true);
	void deactivate();
	//void fileNotLoaded(QFileInfo file);
	void cropImage(const DkRotatingRect& rect, const QColor& bgCol);
	void repeatZoom();

	void applyPlugin(DkPluginContainer* plugin, const QString& key);

	// copy & paste
	void copyPixelColorValue();
	void copyImageBuffer();
	void copyImage();

	virtual void updateImage(QSharedPointer<DkImageContainerT> image, bool loaded = true);
	virtual void loadImage(const QImage& newImg);
	virtual void loadImage(QSharedPointer<DkImageContainerT> img);
	virtual void setEditedImage(const QImage& newImg, const QString& editName);
	virtual void setEditedImage(QSharedPointer<DkImageContainerT> img);
	virtual void setImage(QImage newImg);
	virtual void setThumbImage(QImage newImg);

	void settingsChanged();
	void pauseMovie(bool paused);
	void stopMovie();
	virtual void loadMovie();
	virtual void loadSvg();
	void nextMovieFrame();
	void previousMovieFrame();
	void animateFade();
	virtual void togglePattern(bool show);

protected:
	
	// events
	virtual void dragLeaveEvent(QDragLeaveEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void wheelEvent(QWheelEvent *event);
	virtual bool event(QEvent *event);
	virtual void paintEvent(QPaintEvent* event);
	//QTransform getSwipeTransform() const;

	bool mTestLoaded = false;
	bool mGestureStarted = false;

	QRectF mOldImgRect;

	QTimer* mRepeatZoomTimer;// = new QTimer(this);
	
	// fading stuff
	QTimer* mAnimationTimer;
	DkTimer mAnimationTime;
	QImage mAnimationBuffer;
	double mAnimationValue;
	QRectF mFadeImgViewRect;
	QRectF mFadeImgRect;
	bool mNextSwipe = true;

	// fun
	bool mDissolveImage = false;
	
	QImage mImgBg;

	QVBoxLayout* mPaintLayout;
	DkControlWidget* mController;
	QSharedPointer<DkImageLoader> mLoader;

	QPoint mCurrentPixelPos;
	
	// functions
#if QT_VERSION < 0x050000
#ifndef QT_NO_GESTURES
	virtual int swipeRecognition(QNativeGestureEvent* event);
#endif
#endif
	virtual int swipeRecognition(QPoint start, QPoint end);
	virtual void swipeAction(int swipeGesture);
	virtual void createShortcuts();

	void drawPolygon(QPainter *painter, QPolygon *polygon);
	virtual void drawBackground(QPainter *painter);
	virtual void updateImageMatrix();
	void showZoom();
	void toggleLena(bool fullscreen);
	void getPixelInfo(const QPoint& pos);

};

class DllGuiExport DkViewPortFrameless : public DkViewPort {
	Q_OBJECT

public:
	DkViewPortFrameless(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	virtual ~DkViewPortFrameless();

	void release();
	virtual void zoom(float factor = 0.5, QPointF center = QPointF(-1,-1));
	virtual void setMainGeometry(const QRect &geometry) {
		mMainScreen = geometry;
	};

	virtual QRect getMainGeometry() {
		return mMainScreen;
	};

public slots:
	virtual void setImage(QImage newImg);
	virtual void resetView();
	virtual void moveView(QPointF);

protected:
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void resizeEvent(QResizeEvent* event);
	virtual void paintEvent(QPaintEvent* event);

	// functions
	//QTransform getScaledImageMatrix();
	virtual void updateImageMatrix();
	virtual void draw(QPainter *painter, double opacity = 1.0);
	void drawFrame(QPainter* painter);
	virtual void drawBackground(QPainter *painter);
	void controlImagePosition(float lb = -1, float ub = -1);
	virtual void centerImage();

	// variables
	QVector<QAction*> mStartActions;
	QVector<QIcon> mStartIcons;
	QVector<QRectF> mStartActionsRects;
	QVector<QPixmap> mStartActionsIcons;
	QRect mMainScreen;	// TODO: let user choose which one to take
};

class DllGuiExport DkViewPortContrast : public DkViewPort {
	Q_OBJECT

public:
	DkViewPortContrast(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	virtual ~DkViewPortContrast();

	void release();

signals:
	void tFSliderAdded(qreal pos) const;
	void imageModeSet(int mode) const;

public slots:
	void changeChannel(int channel);
	void changeColorTable(QGradientStops stops);
	void pickColor(bool enable);
	void enableTF(bool enable);
	QImage getImage() const override;

	virtual void setImage(QImage newImg);

protected:
	virtual void draw(QPainter *painter, double opacity = 1.0);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);

private:
	QImage mFalseColorImg;
	bool mDrawFalseColorImg = false;
	bool mIsColorPickerActive = false;
	int mActiveChannel = 0;
		
	QVector<QImage> mImgs;
	QVector<QRgb> mColorTable;

	// functions
	void drawImageHistogram();
};

};
