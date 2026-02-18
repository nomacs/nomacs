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

class QVBoxLayout;
class QMimeData;
class QTimer;

namespace nmc
{
class DkImageLoader;
class DkControlWidget;
class DkPluginContainer;
class DkBaseManipulator;
class DkResizeDialog;
class DkThumbLoader;
class DkFadeButton;

class DllCoreExport DkViewPort : public DkBaseViewPort
{
    Q_OBJECT

public:
    explicit DkViewPort(DkThumbLoader *thumbLoader, QWidget *parent = nullptr);
    ~DkViewPort() override;

    void zoom(double factor = 0.5, const QPointF &center = QPointF(-1, -1), bool force = false) override;

    void setFullScreen(bool fullScreen);

    QTransform *getWorldMatrixPtr()
    {
        return &mWorldMatrix;
    };

    QTransform *getImageMatrixPtr()
    {
        return &mImgMatrix;
    };

    void setPaintWidget(QWidget *widget, bool removeWidget);

    // getter
    QSharedPointer<DkImageContainerT> imageContainer() const;
    void setImageLoader(QSharedPointer<DkImageLoader> newLoader);
    DkControlWidget *getController();

    QString getCurrentPixelHexValue();

    // map window location (cursor position) to image pixel location, return {-1,-1} if out of bounds
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
    // void loadFile(int skipIdx);
    void loadFirst();
    void loadLast();
    void loadSkipNext10();
    void loadSkipPrev10();
    bool unloadImage();
    void deactivate();
    void cropImage(const DkRotatingRect &rect, const QColor &bgCol, bool cropToMetaData);
    void repeatZoom();

    void applyPlugin(DkPluginContainer *plugin, const QString &key);

    // image saving
    bool isEdited() const;
    QImage getImage() const override;
    void saveFile();
    void saveFileAs(bool silent = false);
    void saveFileWeb();
    void setAsWallpaper();

    // copy & paste
    void copyPixelColorValue();
    void copyImageBuffer();
    void copyImagePath();
    QMimeData *createMimeForDrag() const;

    // image manipulators
    virtual void applyManipulator();
    void manipulatorApplied();

    void updateLoadedImage();
    void onImageLoaded(QSharedPointer<DkImageContainerT> image, bool loaded = true);
    virtual void setImageUpdated();
    virtual void loadImage(const QImage &newImg);
    virtual void setEditedImage(QSharedPointer<DkImageContainerT> img);
    void setImage(QImage newImg) override;

    void settingsChanged();
    void pauseMovie(bool paused);
    void stopMovie();
    virtual void loadMovie();
    virtual void loadSvg();
    void nextMovieFrame();
    void previousMovieFrame();
    void animateFade();
    void togglePattern(bool show) override;

protected:
    // events
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void leaveEvent(QEvent *event) override;

    bool mTestLoaded = false;
    bool mGestureStarted = false;
    bool mDisabledBackground = false; // disables drawBackground() (frameless dialog)

    QRectF mOldImgRect;

    QTimer *mRepeatZoomTimer;

    // fading stuff
    QTimer *mAnimationTimer;
    DkTimer mAnimationTime;
    QImage mAnimationBuffer;
    RenderParams mAnimationParams;
    bool mAnimationBufferHasAlpha = false;
    double mAnimationValue;
    QString mPrevFilePath;
    bool mNextSwipe = true;

    QImage mImgBg;

    QVBoxLayout *mPaintLayout = nullptr;
    DkControlWidget *mController = nullptr;
    QSharedPointer<DkImageLoader> mLoader = QSharedPointer<DkImageLoader>();
    DkResizeDialog *mResizeDialog = nullptr;

    QPoint mCurrentPixelPos;

    DkRotatingRect mCropRect;

    DkFadeButton *mNextButton = nullptr;
    DkFadeButton *mPrevButton = nullptr;

    // image manipulators
    QFutureWatcher<QImage> mManipulatorWatcher;
    QSharedPointer<DkBaseManipulator> mActiveManipulator;

    // functions
    virtual int swipeRecognition(QPoint start, QPoint end);
    virtual void swipeAction(int swipeGesture);

    void createShortcuts();
    void drawPolygon(QPainter &painter, const QPolygon &polygon);
    void eraseBackground(QPainter &painter) const override;
    void updateImageMatrix() override;
    void showZoom();
    void getPixelInfo(const QPoint &pos);

private:
    [[nodiscard]] ZoomPos calcZoomCenter(const QPointF &center, double factor) const override;
};

class DllCoreExport DkViewPortFrameless : public DkViewPort
{
    Q_OBJECT

public:
    explicit DkViewPortFrameless(DkThumbLoader *thumbLoader, QWidget *parent = nullptr);
    ~DkViewPortFrameless() override;

public slots:
    void moveView(const QPointF &delta) override;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    // functions
    void draw(QPainter &painter, double opacity, int flags) override;
    void drawFrame(QPainter &painter);
    void eraseBackground(QPainter &painter) const override;
    void controlImagePosition(float lb = -1, float ub = -1) override;
    void centerImage() override;

    // variables
    QVector<QAction *> mStartActions;
    QVector<QIcon> mStartIcons;
    QVector<QRectF> mStartActionsRects;
    QVector<QPixmap> mStartActionsIcons;
    QRectF mStartBgRect;

private:
    [[nodiscard]] qreal imageMatrixPaddingRatio() const override
    {
        return 0.1;
    }
    [[nodiscard]] ZoomPos calcZoomCenter(const QPointF &center, double factor) const override;
};

class DllCoreExport DkViewPortContrast : public DkViewPort
{
    Q_OBJECT

public:
    explicit DkViewPortContrast(DkThumbLoader *thumbLoader, QWidget *parent = nullptr);
    ~DkViewPortContrast() override;

signals:
    void tFSliderAdded(qreal pos) const;
    void imageModeSet(int mode) const;
    void cancelPickColor() const;

public slots:
    void changeChannel(int channel);
    void changeColorTable(QGradientStops stops);
    void pickColor(bool enable);
    void updateImage(bool enable);

protected:
    QImage getImage() const override;
    void setImage(QImage newImg) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    bool mDrawFalseColorImg = false;
    bool mIsColorPickerActive = false;
    int mActiveChannel = 0;

    QVector<QImage> mImgs;
    QVector<QRgb> mColorTable;
};

}
