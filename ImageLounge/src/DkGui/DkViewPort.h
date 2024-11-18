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
#include "DkMath.h"
#include "DkTimer.h"

#pragma warning(push, 0) // no warnings from includes - begin
#include <QTimer> // needed to construct mTimers
#pragma warning(pop) // no warnings from includes - end

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

#pragma warning(disable : 4275) // no dll interface of base class

// OpenCV
#ifdef WITH_OPENCV
#ifdef Q_OS_WIN
#pragma warning(disable : 4996)
#endif
#endif

class QVBoxLayout;
class QMimeData;
class QPushButton;

namespace nmc
{
// some dummies
class DkImageLoader;
class DkLoader;
class DkControlWidget;
class DkPeer;
class DkRotatingRect;
class DkPluginInterface;
class DkPluginContainer;
class DkBaseManipulator;
class DkResizeDialog;
class DkHudNavigation;

class DllCoreExport DkViewPort : public DkBaseViewPort
{
    Q_OBJECT

public:
    DkViewPort(QWidget *parent = 0);
    virtual ~DkViewPort();

    void zoom(double factor = 0.5, const QPointF &center = QPointF(-1, -1), bool force = false) override;

    void setFullScreen(bool fullScreen);

    QTransform getWorldMatrix() override
    {
        return mWorldMatrix;
    };

    QTransform *getWorldMatrixPtr()
    {
        return &mWorldMatrix;
    };

    QTransform *getImageMatrixPtr()
    {
        return &mImgMatrix;
    };

    void setPaintWidget(QWidget *widget, bool removeWidget);

#ifdef WITH_OPENCV
    void setImage(cv::Mat newImg) override;
#endif

    // getter
    QSharedPointer<DkImageContainerT> imageContainer() const;
    void setImageLoader(QSharedPointer<DkImageLoader> newLoader);
    DkControlWidget *getController();

    QString getCurrentPixelHexValue();
    QPoint mapToImage(const QPoint &windowPos) const;

    void connectLoader(QSharedPointer<DkImageLoader> loader, bool connectSignals = true);

signals:
    void sendTransformSignal(QTransform transform, QTransform imgTransform, QPointF canvasSize) const;
    void sendNewFileSignal(qint16 op, QString filename = "") const;
    void movieLoadedSignal(bool isMovie) const;
    void infoSignal(const QString &msg) const; // needed to forward signals
    void addTabSignal(const QString &filePath) const;
    void zoomSignal(double zoomLevel) const;
    void mouseClickSignal(QMouseEvent *event, QPoint imgPos) const;
    void showProgress(bool show, int time = -1) const;
    void imageUpdatedSignal() const;

public slots:
    void fullView() override;
    void resetView() override;

    void rotateCW();
    void rotateCCW();
    void rotate180();
    void resizeImage();
    void deleteImage();
    void zoomToFit();
    void resizeEvent(QResizeEvent *event) override;
    void zoomTo(double zoomLevel);

    // tcp actions
    void tcpSetTransforms(QTransform worldMatrix, QTransform imgMatrix, QPointF canvasSize);
    void tcpSetWindowRect(QRect rect);
    void tcpForceSynchronize();
    void tcpSynchronize(QTransform relativeMatrix = QTransform(), bool force = false);
    void tcpLoadFile(qint16 idx, QString filename);

    // file actions
    void loadFile(const QString &filePath);
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
    bool unloadImage(bool fileChange = true) override;
    void deactivate();
    void cropImage(const DkRotatingRect &rect, const QColor &bgCol, bool cropToMetaData);
    void repeatZoom();

    void applyPlugin(DkPluginContainer *plugin, const QString &key);

    // image saving
    QImage getImage() const override;
    void saveFile();
    void saveFileAs(bool silent = false);
    void saveFileWeb();
    void setAsWallpaper();

    // copy & paste
    void copyPixelColorValue();
    void copyImageBuffer();
    void copyImage();
    QMimeData *createMime() const;

    // image manipulators
    virtual void applyManipulator();
    void manipulatorApplied();

    void updateLoadedImage(QSharedPointer<DkImageContainerT> image);
    virtual void updateImage(QSharedPointer<DkImageContainerT> image, bool loaded = true);
    virtual void setImageUpdated();
    virtual void loadImage(const QImage &newImg);
    virtual void loadImage(QSharedPointer<DkImageContainerT> img);
    virtual void setEditedImage(const QImage &newImg, const QString &editName);
    virtual void setEditedImage(QSharedPointer<DkImageContainerT> img);
    virtual void setImage(QImage newImg) override;

    void settingsChanged();
    void pauseMovie(bool paused);
    void stopMovie();
    virtual void loadMovie();
    virtual void loadSvg();
    void nextMovieFrame();
    void previousMovieFrame();
    void animateFade();
    virtual void togglePattern(bool show) override;

protected:
    // events
    virtual void dragLeaveEvent(QDragLeaveEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual bool event(QEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;

    bool mTestLoaded = false;
    bool mGestureStarted = false;

    QRectF mOldImgRect;

    QTimer *mRepeatZoomTimer;

    // fading stuff
    QTimer *mAnimationTimer;
    DkTimer mAnimationTime;
    QImage mAnimationBuffer;
    double mAnimationValue;
    QRectF mFadeImgViewRect;
    QRectF mFadeImgRect;
    bool mNextSwipe = true;

    QImage mImgBg;

    QVBoxLayout *mPaintLayout = 0;
    DkControlWidget *mController = 0;
    QSharedPointer<DkImageLoader> mLoader = QSharedPointer<DkImageLoader>();
    DkResizeDialog *mResizeDialog = 0;

    QPoint mCurrentPixelPos;

    DkRotatingRect mCropRect;

    DkHudNavigation *mNavigationWidget = 0;

    // image manipulators
    QFutureWatcher<QImage> mManipulatorWatcher;
    QSharedPointer<DkBaseManipulator> mActiveManipulator;

    // functions
    virtual int swipeRecognition(QPoint start, QPoint end);
    virtual void swipeAction(int swipeGesture);
    virtual void createShortcuts();

    void drawPolygon(QPainter &painter, const QPolygon &polygon);
    virtual void drawBackground(QPainter &painter);
    virtual void updateImageMatrix() override;
    void showZoom();
    void toggleLena(bool fullscreen);
    void getPixelInfo(const QPoint &pos);
};

class DllCoreExport DkViewPortFrameless : public DkViewPort
{
    Q_OBJECT

public:
    DkViewPortFrameless(QWidget *parent = 0);
    virtual ~DkViewPortFrameless();

    virtual void zoom(double factor = 0.5, const QPointF &center = QPointF(-1, -1), bool force = false) override;

public slots:
    virtual void resetView() override;
    virtual void moveView(QPointF);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;

    // functions
    virtual void updateImageMatrix() override;
    virtual void draw(QPainter &painter, double opacity = 1.0) override;
    void drawFrame(QPainter &painter);
    virtual void drawBackground(QPainter &painter) override;
    void controlImagePosition(float lb = -1, float ub = -1) override;
    virtual void centerImage() override;

    // variables
    QVector<QAction *> mStartActions;
    QVector<QIcon> mStartIcons;
    QVector<QRectF> mStartActionsRects;
    QVector<QPixmap> mStartActionsIcons;
};

class DllCoreExport DkViewPortContrast : public DkViewPort
{
    Q_OBJECT

public:
    DkViewPortContrast(QWidget *parent = 0);
    virtual ~DkViewPortContrast();

signals:
    void tFSliderAdded(qreal pos) const;
    void imageModeSet(int mode) const;

public slots:
    void changeChannel(int channel);
    void changeColorTable(QGradientStops stops);
    void pickColor(bool enable);
    void enableTF(bool enable);
    QImage getImage() const override;

    virtual void setImage(QImage newImg) override;

protected:
    virtual void draw(QPainter &painter, double opacity = 1.0) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;

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

}
