/*******************************************************************************************************
 DkWidgets.h
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

#pragma once

#include "DkQt5Compat.h"

#include <QFileSystemModel>
#include <QFutureWatcher>
#include <QLineEdit>
#include <QListWidget>
#include <QPen>
#include <QProgressBar>
#include <QPushButton>
#include <QSlider>
#include <QSortFilterProxyModel>

#include "DkBaseWidgets.h"
#include "DkImageContainer.h"
#include "DkMath.h"

class QDoubleSpinBox;
class QFileIconProvider;
class QBoxLayout;
class QProgressDialog;
class QTreeView;
class QVBoxLayout;

namespace nmc
{
class DkCropToolBar;

class DkButton : public QPushButton
{
    Q_OBJECT

public:
    explicit DkButton(QWidget *parent = nullptr);
    explicit DkButton(const QString &text, QWidget *parent = nullptr);
    DkButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);
    DkButton(const QIcon &checkedIcon, const QIcon &uncheckedIcon, const QString &text, QWidget *parent = nullptr);
    ~DkButton() override = default;

    void adjustSize();
    void setFixedSize(QSize size);

    bool keepAspectRatio;

protected:
    QIcon mCheckedIcon;
    QIcon mUncheckedIcon;
    bool mMouseOver;
    QSize mMySize;

    // functions
    void init();

    void paintEvent(QPaintEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void enterEvent(DkEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    QPixmap createSelectedEffect(QPixmap *pm);
};

class DkRatingLabel : public DkWidget
{
    Q_OBJECT

public:
    explicit DkRatingLabel(int rating = 0, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

    void setRating(int rating)
    {
        mRating = rating;
        updateRating();
    };

    void changeRating(int newRating)
    {
        mRating = newRating;
        updateRating();
        emit newRatingSignal(mRating);
    };

    int getRating()
    {
        return mRating;
    };

signals:
    void newRatingSignal(int rating);

protected:
    QVector<DkButton *> mStars;
    int mRating = 0;

    void updateRating();
    void init();
};

class DkFileInfoLabel : public DkFadeLabel
{
    Q_OBJECT

public:
    explicit DkFileInfoLabel(QWidget *parent = nullptr);
    ~DkFileInfoLabel() override = default;

    void createLayout();
    void updateInfo(const QString &filePath, const QString &attr, const QString &date, const int rating);
    void updateTitle(const QString &filePath, const QString &attr);
    void updateDate(const QString &date = QString());
    void updateRating(const int rating);
    void setEdited(bool edited);
    DkRatingLabel *getRatingLabel();

public slots:
    void setVisible(bool visible, bool saveSettings = true) override;

protected:
    QString mFilePath;

    QBoxLayout *mLayout;
    QLabel *mTitleLabel;
    QLabel *mDateLabel;
    DkRatingLabel *mRatingLabel;

    void updateWidth();
};

class DkPlayer : public DkFadeWidget
{
    Q_OBJECT

public:
    enum {
        play_action = 0, // if more actions are to be assigned
    };

    explicit DkPlayer(QWidget *parent = nullptr);
    ~DkPlayer() override = default;

    void setTimeToDisplay(int ms = 1000);

signals:
    void nextSignal();
    void previousSignal();

public slots:
    void play(bool play);
    void togglePlay();
    void startTimer();
    void autoNext();
    void next();
    void previous();
    void showTemporarily(bool autoHide = true);
    bool isPlaying() const;

protected:
    void init();
    void createLayout();

    bool playing = false;

    int timeToDisplay;
    QTimer *displayTimer;
    QTimer *hideTimer;

    QPushButton *previousButton;
    QPushButton *nextButton;
    QPushButton *playButton;
    QWidget *container;
};

class DkHudNavigation : public DkFadeWidget
{
    Q_OBJECT

public:
    explicit DkHudNavigation(QWidget *parent = nullptr);
    ~DkHudNavigation() override = default;

signals:
    void nextSignal();
    void previousSignal();

public slots:
    void showNext();
    void showPrevious();

protected:
    void createLayout();

    QPushButton *mPreviousButton;
    QPushButton *mNextButton;
};

class DkFolderScrollBar : public DkFadeMixin<QSlider>
{
    Q_OBJECT

public:
    explicit DkFolderScrollBar(QWidget *parent = nullptr);
    ~DkFolderScrollBar() override;

    virtual void setValue(int i);

public slots:
    void updateDir(QVector<QSharedPointer<DkImageContainerT>> images);

    void updateFile(int idx);

signals:
    void loadFileSignal(int idx) const;
    void visibleSignal(bool visible) const;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    QColor mBgCol;
    bool mMouseDown = false;

    void init();
};

extern template class DkFadeMixin<QSlider>;

// this class is one of the first batch processing classes -> move them to a new file in the (near) future
class DkThumbsSaver : public DkWidget
{
    Q_OBJECT

public:
    explicit DkThumbsSaver(QWidget *parent = nullptr);

    void processDir(QVector<QSharedPointer<DkImageContainerT>> images, bool forceSave);

signals:
    void numFilesSignal(int currentFileIdx);

public slots:
    void stopProgress();
    void thumbLoaded();

protected:
    QFileInfo mCurrentDir;
    QProgressDialog *mPd = nullptr;
    bool mStop = false;
    int mNumSaved = 0;
    std::vector<std::unique_ptr<QFutureWatcher<void>>> mWatchers{};
};

class DkFileSystemModel : public QFileSystemModel
{
    Q_OBJECT

public:
    explicit DkFileSystemModel(QObject *parent = nullptr);

protected:
    QFileIconProvider *mIconProvider = nullptr;
};

class DkSortFileProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit DkSortFileProxyModel(QObject *parent = nullptr);

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};

class DllCoreExport DkExplorer : public DkDockWidget
{
    Q_OBJECT

public:
    explicit DkExplorer(const QString &title, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~DkExplorer() override;

    DkFileSystemModel *getModel()
    {
        return mFileModel;
    };

public slots:
    void setCurrentImage(QSharedPointer<DkImageContainerT> img);
    void setCurrentPath(const QString &path);
    void fileClicked(const QModelIndex &index) const;
    void showColumn(bool show);
    void setEditable(bool editable);
    void adjustColumnWidth();
    void enableLoadSelected(bool enable);
    void openSelected();

signals:
    void openFile(const QString &filePath) const;
    void openDir(const QString &dir) const;

protected:
    void closeEvent(QCloseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

    void createLayout();
    void writeSettings();
    void readSettings();

    QVBoxLayout *mLayout = nullptr;
    QTreeView *mFileTree = nullptr;
    DkFileSystemModel *mFileModel = nullptr;
    DkSortFileProxyModel *mSortModel = nullptr;

private:
    QVector<QAction *> mColumnActions;
    bool mLoadSelected = false;
};

class DllCoreExport DkBrowseExplorer : public DkExplorer
{
    Q_OBJECT

public:
    explicit DkBrowseExplorer(const QString &title,
                              QWidget *parent = nullptr,
                              Qt::WindowFlags flags = Qt::WindowFlags());
    ~DkBrowseExplorer() override;

public slots:
    void browseClicked();
    void scrollToCurrentClicked();
    void setRootPath(const QString &root);

protected:
    void createLayout();
    void readSettings();
    void writeSettings();

    QString mRootPath;
    DkElidedLabel *mRootPathLabel;
};

class DkOverview : public QLabel
{
    Q_OBJECT

public:
    explicit DkOverview(QWidget *parent = nullptr);
    ~DkOverview() override = default;

    void setImage(const QImage &img)
    {
        mImg = img;
        mImgSize = img.size();
        mImgT = QImage();
    };

    void setTransforms(QTransform *worldMatrix, QTransform *imgMatrix)
    {
        mWorldMatrix = worldMatrix;
        mImgMatrix = imgMatrix;
    };

    void setViewPortRect(const QRectF &viewPortRect)
    {
        mViewPortRect = viewPortRect;
    };

signals:
    void moveViewSignal(const QPointF &dxy) const;
    void sendTransformSignal() const;

protected:
    QImage mImg;
    QImage mImgT;
    QSize mImgSize;
    QTransform *mScaledImgMatrix;
    QTransform *mWorldMatrix;
    QTransform *mImgMatrix;
    QRectF mViewPortRect;
    QPointF mPosGrab;
    QPointF mEnterPos;

    QImage resizedImg(const QImage &src);
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    QRectF getImageRect() const;
    // get matrix that resizes the image to the current mViewport
    QTransform getScaledImageMatrix();
};

class DkZoomWidget : public DkFadeLabel
{
    Q_OBJECT

public:
    explicit DkZoomWidget(QWidget *parent = nullptr);

    DkOverview *getOverview() const;

signals:
    void zoomSignal(double zoomLevel);

public slots:
    void updateZoom(double zoomLevel);
    void onSbZoomValueChanged(double zoomLevel);
    void onSlZoomValueChanged(int zoomLevel);

protected:
    void createLayout();

    DkOverview *mOverview = nullptr;
    QSlider *mSlZoom = nullptr;
    QDoubleSpinBox *mSbZoom = nullptr;
};

class DkTransformRect : public DkWidget
{
    Q_OBJECT

public:
    explicit DkTransformRect(int idx = -1,
                             DkRotatingRect *rect = nullptr,
                             QWidget *parent = nullptr,
                             Qt::WindowFlags f = Qt::WindowFlags());
    ~DkTransformRect() override = default;

    void draw(QPainter *painter);

    QPointF getCenter()
    {
        return QPointF(mSize.width() * 0.5f, mSize.height() * 0.5f);
    };

signals:
    void ctrlMovedSignal(int, const QPointF &, Qt::KeyboardModifiers, bool);
    void updateDiagonal(int);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(DkEnterEvent *event) override;
    void init();

    DkRotatingRect *mRect;
    QPointF mInitialPos;
    QPointF mPosGrab;
    int mParentIdx;
    QSize mSize;
};

class DkEditableRect : public DkFadeWidget
{
    Q_OBJECT

public:
    enum {
        no_guide = 0,
        rule_of_thirds,
        grid,

        mode_end,
    };

    enum {
        do_nothing,
        initializing,
        rotating,
        moving,
        scaling
    };

    explicit DkEditableRect(const QRectF &rect = QRect(),
                            QWidget *parent = nullptr,
                            Qt::WindowFlags f = Qt::WindowFlags());
    ~DkEditableRect() override = default;

    void reset();

    void setWorldTransform(QTransform *worldTform)
    {
        mWorldTform = worldTform;
    };

    void setImageTransform(QTransform *imgTform)
    {
        mImgTform = imgTform;
    };

    void setImageRect(QRectF *imgRect)
    {
        mImgRect = imgRect;
    };

    void setVisible(bool visible) override;

signals:
    void cropImageSignal(const DkRotatingRect &cropArea,
                         const QColor &bgCol = QColor(0, 0, 0, 0),
                         bool cropToMetaData = false) const;
    void angleSignal(double angle) const;
    void aRatioSignal(const QPointF &aRatio) const;
    void updateRectSignal(const QRect &r) const;

public slots:
    void updateCorner(int idx, const QPointF &point, Qt::KeyboardModifiers modifier, bool changeState = false);
    void updateDiagonal(int idx);
    void setFixedDiagonal(const DkVector &diag);
    void setAngle(double angle, bool apply = true);
    void setPanning(bool panning);
    void setPaintHint(int paintMode = rule_of_thirds);
    void setShadingHint(bool invert);
    void setShowInfo(bool showInfo);
    void setRect(const QRect &rect);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    QRect rect() const;

    QPointF clipToImage(const QPointF &pos);
    QPointF clipToImageForce(const QPointF &pos);
    void applyTransform();
    void drawGuide(QPainter *painter, const QPolygonF &p, int paintMode);
    QPointF map(const QPointF &pos);

    int mState = do_nothing;
    QTransform *mImgTform = nullptr;
    QTransform *mWorldTform = nullptr;
    QTransform mTtform;
    QTransform mRtform;
    QPointF mPosGrab;
    QPointF mClickPos;
    DkVector mOldDiag = DkVector(-1.0f, -1.0f);
    DkVector mFixedDiag;

    DkRotatingRect mRect;
    QPen mPen;
    QBrush mBrush;
    QVector<DkTransformRect *> mCtrlPoints;
    QCursor mRotatingCursor;
    QRectF *mImgRect = nullptr;
    bool mPanning = false;
    int mPaintMode = rule_of_thirds;
    bool mShowInfo = false;
};

class DkCropWidget : public DkEditableRect
{
    Q_OBJECT

public:
    explicit DkCropWidget(QRectF rect = QRect(), QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    DkCropToolBar *getToolbar() const;

public slots:
    void crop(bool cropToMetadata = false);
    void setVisible(bool visible) override;

signals:
    void hideSignal();

protected:
    void createToolbar();
    void mouseDoubleClickEvent(QMouseEvent *ev) override;

    DkCropToolBar *cropToolbar;
};

// Image histogram display
class DkHistogram : public DkFadeWidget
{
    Q_OBJECT

public:
    enum class DisplayMode {
        histogram_mode_simple = 0, /// shows just the histogram
        histogram_mode_extended = 1, /// shows histogram and data
        histogram_mode_end = 2,
    };

    explicit DkHistogram(QWidget *parent);
    ~DkHistogram() override;

    void drawHistogram(QImage img);
    void clearHistogram();
    void setMaxHistogramValue(int maxValue);
    void updateHistogramValues(int histValues[][256]);
    void setPainted(bool isPainted);

public slots:
    void onToggleStatsTriggered(bool show);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

    void loadSettings();

private:
    int mHist[3][256]; /// 3 channels 256 bin. channels duplicated when gray
    int mNumPixels = 0; /// image pixel count
    int mNumDistinctValues = 0; /// number of distinct values
    int mNumZeroPixels = 0; /// pixels with zero value
    int mNumSaturatedPixels = 0; /// pixels saturating RGB 8bit
    int mNumValues = 0; /// number of distinct histogram values
    int mMinBinValue = 256; /// (gray-only) minimum intensity value
    int mMaxBinValue = -1; /// (gray-only) maximum intensity value
    int mMaxValue = 20; /// maximum count over all bins
    bool mIsPainted = false;
    float mScaleFactor = 1;
    DisplayMode mDisplayMode = DisplayMode::histogram_mode_simple; /// determins shown histogram type

    QMenu *mContextMenu = nullptr;
};

class DkFileInfoWrapper
{
public:
    DkFileInfoWrapper();
    explicit DkFileInfoWrapper(const QFileInfo &fileInfo);

    QString getFilePath() const;
    bool exists() const;
    void setExists(bool fileExists);

    bool inUse() const;
    void setInUse(bool inUse);

protected:
    QFileInfo mFileInfo;
    bool mFileExists = false;
    bool mUsed = false;
};

class DkFolderLabel : public QLabel
{
    Q_OBJECT

public:
    explicit DkFolderLabel(const DkFileInfoWrapper &fileInfo,
                           QWidget *parent = nullptr,
                           Qt::WindowFlags f = Qt::WindowFlags());

signals:
    void loadFileSignal(const QString &) const;

protected:
    void mousePressEvent(QMouseEvent *ev) override;

    DkFileInfoWrapper mFileInfo;
};

class DkDirectoryEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit DkDirectoryEdit(QWidget *parent = nullptr);
    explicit DkDirectoryEdit(const QString &content, QWidget *parent = nullptr);

    bool existsDirectory()
    {
        return existsDirectory(text());
    };

signals:
    bool directoryChanged(const QString &path);

public slots:
    void lineEditChanged(const QString &path);

private:
    bool existsDirectory(const QString &path);

    QString mOldPath;
    bool showFolderButton = false;
};

class DkDirectoryChooser : public DkWidget
{
    Q_OBJECT
public:
    explicit DkDirectoryChooser(const QString &dirPath = "", QWidget *parent = nullptr);

public slots:
    void onDirButtonClicked();

signals:
    void directoryChanged(const QString &dirPath) const;

protected:
    void createLayout(const QString &dirPath);

    DkDirectoryEdit *mDirEdit = nullptr;
};

class DkDelayedInfo : public QObject
{
    Q_OBJECT

public:
    explicit DkDelayedInfo(int time = 0, QObject *parent = nullptr)
        : QObject(parent)
    {
        timer = new QTimer();
        timer->setSingleShot(true);

        if (time)
            timer->start(time);

        connect(timer, &QTimer::timeout, this, &DkDelayedInfo::sendInfo);
    }

    ~DkDelayedInfo() override
    {
        if (timer && timer->isActive())
            timer->stop();

        if (timer)
            delete timer;

        timer = nullptr;
    }

    void stop()
    {
        if (timer && timer->isActive())
            timer->stop();
        else
            emit infoSignal(1);
    }

    void setInfo(int time = 1000)
    {
        if (!timer)
            return;

        timer->start(time);
    }

signals:
    void infoSignal(int time);

protected slots:
    virtual void sendInfo()
    {
        emit infoSignal(-1);
    }

protected:
    QTimer *timer;
};

class DkDelayedMessage : public DkDelayedInfo
{
    Q_OBJECT

public:
    explicit DkDelayedMessage(const QString &msg = QString(), int time = 0, QObject *parent = nullptr)
        : DkDelayedInfo(time, parent)
    {
        mMsg = msg;
    }
    explicit DkDelayedMessage(QObject *parent = nullptr)
        : DkDelayedInfo(0, parent)
    {
    }

    ~DkDelayedMessage() override = default;

    void stop()
    {
        if (timer && timer->isActive())
            timer->stop();
        else
            emit infoSignal(mMsg, 1);
    }

    void setInfo(const QString &msg, int time = 1000)
    {
        DkDelayedInfo::setInfo(time);
        mMsg = msg;
    }

signals:
    void infoSignal(const QString &msg, int time = -1) const;

protected:
    QString mMsg;
};

class DkListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit DkListWidget(QWidget *parent);

    void startDrag(Qt::DropActions supportedActions) override;
    bool isEmpty() const;

    void setEmptyText(const QString &text);

signals:
    void dataDroppedSignal() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    QString mEmptyText = tr("Drag Items Here");
};

class DkProgressBar : public QProgressBar
{
    Q_OBJECT

public:
    explicit DkProgressBar(QWidget *parent = nullptr);

public slots:
    void setVisible(bool visible) override;
    void setVisibleTimed(bool visible, int time = -1);

protected:
    void paintEvent(QPaintEvent *ev) override;
    void initPoints();
    void animatePoint(double &xVal);

    QTimer mTimer;
    QTimer mShowTimer;
    QVector<double> mPoints;
};

#ifdef ENABLE_DEAD_CODE
class DllCoreExport DkGenericProfileWidget : public DkNamedWidget
{
    Q_OBJECT

public:
    DkGenericProfileWidget(const QString &name, QWidget *parent, const QString &settingsPath = "");
    ~DkGenericProfileWidget() override;

public slots:
    void saveSettings() const;
    virtual void saveSettings(const QString &name) const;
    virtual void loadSettings(const QString &name) = 0;
    void deleteCurrentSetting();
    void activate(bool active = true);
    void setDefaultModel() const;

protected:
    void paintEvent(QPaintEvent *ev) override;

    virtual void init();
    void createLayout();
    QStringList loadProfileStrings() const;
    QString loadDefaultProfileString() const;

    QPushButton *mSaveButton;
    QPushButton *mDeleteButton;
    QComboBox *mProfileList;
    bool mEmpty = true;

    QString mSettingsGroup = "ILLEGAL_GENERIC_GROUP";
    QString mSettingsPath;
};
#endif // ENABLE_DEAD_CODE

class DllCoreExport DkTabEntryWidget : public QPushButton
{
    Q_OBJECT

public:
    DkTabEntryWidget(const QIcon &icon, const QString &text, QWidget *parent);

protected:
    void paintEvent(QPaintEvent *event) override;
};

class DllCoreExport DkDisplayWidget : public DkWidget
{
    Q_OBJECT

public:
    explicit DkDisplayWidget(QWidget *parent);

    QRect screenRect() const;

    int count() const;

    int currentIndex() const;
    void setCurrentIndex(int index);

    // public slots:
    //	void changeDisplay();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void createLayout();
    void updateLayout();

    QList<QScreen *> mScreens;
    QList<QPushButton *> mScreenButtons;
};

}
