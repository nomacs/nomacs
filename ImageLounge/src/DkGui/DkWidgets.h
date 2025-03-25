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

#pragma warning(push, 0) // no warnings from includes - begin
#include <QDockWidget>
#include <QFileIconProvider>
#include <QFileSystemModel>
#include <QFutureWatcher>
#include <QLineEdit>
#include <QListWidget>
#include <QPen>
#include <QPointer>
#include <QProgressBar>
#include <QPushButton>
#include <QSlider>
#include <QSortFilterProxyModel>
#pragma warning(pop) // no warnings from includes - end

#pragma warning(disable : 4251) // TODO: remove

#include "DkBaseWidgets.h"
#include "DkImageContainer.h"
#include "DkMath.h"

// Qt defines
class QColorDialog;
class QSpinBox;
class QDoubleSpinBox;
class QToolBar;
class QBoxLayout;
class QProgressDialog;
class QMovie;
class QTreeView;
class QSlider;
class QGridLayout;
class QVBoxLayout;
class QSvgRenderer;
class QSettings;

namespace nmc
{
// nomacs defines
class DkCropToolBar;

class DkButton : public QPushButton
{
    Q_OBJECT

public:
    DkButton(QWidget *parent = 0);
    DkButton(const QString &text, QWidget *parent = 0);
    DkButton(const QIcon &icon, const QString &text, QWidget *parent = 0);
    DkButton(const QIcon &checkedIcon, const QIcon &uncheckedIcon, const QString &text, QWidget *parent = 0);
    ~DkButton(){};

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

    void paintEvent(QPaintEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void enterEvent(DkEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    QPixmap createSelectedEffect(QPixmap *pm);
};

class DkRatingLabel : public DkFadeWidget
{
    Q_OBJECT

public:
    enum {
        rating_1,
        rating_2,
        rating_3,
        rating_4,
        rating_5,
        rating_0, // no image for that one
    };

    DkRatingLabel(int rating = 0, QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
    ~DkRatingLabel(){};

    void setRating(int rating)
    {
        mRating = rating;
        updateRating();
    };

    virtual void changeRating(int newRating)
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
    void newRatingSignal(int rating = 0);

public slots:
    void rating0()
    {
        changeRating(0);
    };

    void rating1()
    {
        changeRating(1);
    };

    void rating2()
    {
        changeRating(2);
    };

    void rating3()
    {
        changeRating(3);
    };

    void rating4()
    {
        changeRating(4);
    };

    void rating5()
    {
        changeRating(5);
    };

protected:
    QVector<DkButton *> mStars;
    QBoxLayout *mLayout = 0;
    int mRating = 0;

    void updateRating()
    {
        for (int idx = 0; idx < mStars.size(); idx++) {
            mStars[idx]->setChecked(idx < mRating);
        }
    };

    virtual void init();
};

class DkRatingLabelBg : public DkRatingLabel
{
    Q_OBJECT

public:
    DkRatingLabelBg(int rating = 0, QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
    ~DkRatingLabelBg();

    void changeRating(int newRating) override;

protected:
    QTimer *mHideTimer;
    int mTimeToDisplay = 4000;

    virtual void paintEvent(QPaintEvent *event) override;
};

class DkFileInfoLabel : public DkFadeLabel
{
    Q_OBJECT

public:
    DkFileInfoLabel(QWidget *parent = 0);
    ~DkFileInfoLabel(){};

    void createLayout();
    void updateInfo(const QString &filePath, const QString &attr, const QString &date, const int rating);
    void updateTitle(const QString &filePath, const QString &attr);
    void updateDate(const QString &date = QString());
    void updateRating(const int rating);
    void setEdited(bool edited);
    DkRatingLabel *getRatingLabel();

public slots:
    virtual void setVisible(bool visible, bool saveSettings = true) override;

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

    DkPlayer(QWidget *parent = 0);
    ~DkPlayer(){};

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
    DkHudNavigation(QWidget *parent = 0);
    ~DkHudNavigation(){};

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
    DkFolderScrollBar(QWidget *parent = 0);
    ~DkFolderScrollBar();

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

// this class is one of the first batch processing classes -> move them to a new file in the (near) future
class DkThumbsSaver : public DkWidget
{
    Q_OBJECT

public:
    DkThumbsSaver(QWidget *parent = 0);

    void processDir(QVector<QSharedPointer<DkImageContainerT>> images, bool forceSave);

signals:
    void numFilesSignal(int currentFileIdx);

public slots:
    void stopProgress();
    void thumbLoaded(bool loaded);
    void loadNext();

protected:
    QFileInfo mCurrentDir;
    QProgressDialog *mPd = 0;
    bool mStop = false;
    bool mForceSave = false;
    int mNumSaved = false;
    QVector<QSharedPointer<DkImageContainerT>> mImages;
};

class DkFileSystemModel : public QFileSystemModel
{
    Q_OBJECT

public:
    DkFileSystemModel(QObject *parent = 0);

protected:
    QFileIconProvider *mIconProvider = nullptr;
};

class DkSortFileProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    DkSortFileProxyModel(QObject *parent = 0);

protected:
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

class DllCoreExport DkExplorer : public DkDockWidget
{
    Q_OBJECT

public:
    DkExplorer(const QString &title, QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
    virtual ~DkExplorer();

    DkFileSystemModel *getModel()
    {
        return mFileModel;
    };

public slots:
    void setCurrentImage(QSharedPointer<DkImageContainerT> img);
    void setCurrentPath(const QString &filePath);
    void fileClicked(const QModelIndex &index) const;
    void showColumn(bool show);
    void setEditable(bool editable);
    void adjustColumnWidth();
    void loadSelectedToggled(bool checked);
    void openSelected();

signals:
    void openFile(const QString &filePath) const;
    void openDir(const QString &dir) const;

protected:
    void closeEvent(QCloseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

    virtual void createLayout();
    virtual void writeSettings();
    virtual void readSettings();

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
    DkBrowseExplorer(const QString &title, QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
    ~DkBrowseExplorer();

public slots:
    void browseClicked();
    void scrollToCurrentClicked();
    void setRootPath(const QString &root);

protected:
    void createLayout() override;
    void readSettings() override;
    void writeSettings() override;

    QString mRootPath;
    DkElidedLabel *mRootPathLabel;
};

class DkOverview : public QLabel
{
    Q_OBJECT

public:
    DkOverview(QWidget *parent = 0);
    ~DkOverview(){};

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
    QTransform getScaledImageMatrix();
};

class DkZoomWidget : public DkFadeLabel
{
    Q_OBJECT

public:
    DkZoomWidget(QWidget *parent = 0);

    DkOverview *getOverview() const;

signals:
    void zoomSignal(double zoomLevel);

public slots:
    void updateZoom(double zoomLevel);
    void onSbZoomValueChanged(double zoomLevel);
    void onSlZoomValueChanged(int zoomLevel);

protected:
    void createLayout();

    DkOverview *mOverview = 0;
    QSlider *mSlZoom = 0;
    QDoubleSpinBox *mSbZoom = 0;
};

class DkTransformRect : public DkWidget
{
    Q_OBJECT

public:
    DkTransformRect(int idx = -1, DkRotatingRect *rect = 0, QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~DkTransformRect(){};

    void draw(QPainter *painter);

    QPointF getCenter()
    {
        return QPointF(size.width() * 0.5f, size.height() * 0.5f);
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

    DkRotatingRect *rect;
    QPointF initialPos;
    QPointF posGrab;
    int parentIdx;
    QSize size;
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

    enum { do_nothing, initializing, rotating, moving, scaling };

    DkEditableRect(const QRectF &rect = QRect(), QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~DkEditableRect(){};

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

    virtual void setVisible(bool visible) override;

signals:
    void cropImageSignal(const DkRotatingRect &cropArea, const QColor &bgCol = QColor(0, 0, 0, 0), bool cropToMetaData = false) const;
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
    QTransform *mImgTform = 0;
    QTransform *mWorldTform = 0;
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
    QRectF *mImgRect = 0;
    bool mPanning = false;
    int mPaintMode = rule_of_thirds;
    bool mShowInfo = false;
};

class DkCropWidget : public DkEditableRect
{
    Q_OBJECT

public:
    DkCropWidget(QRectF rect = QRect(), QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

    DkCropToolBar *getToolbar() const;

public slots:
    void crop(bool cropToMetadata = false);
    virtual void setVisible(bool visible) override;

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

    DkHistogram(QWidget *parent);
    ~DkHistogram();

    void drawHistogram(QImage img);
    void clearHistogram();
    void setMaxHistogramValue(int maxValue);
    void updateHistogramValues(int histValues[][256]);
    void setPainted(bool isPainted);

public slots:
    void onToggleStatsTriggered(bool show);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void contextMenuEvent(QContextMenuEvent *event) override;

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

    QMenu *mContextMenu = 0;
};

class DkFileInfo
{
public:
    DkFileInfo();
    DkFileInfo(const QFileInfo &fileInfo);

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
    DkFolderLabel(const DkFileInfo &fileInfo, QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

signals:
    void loadFileSignal(const QString &) const;

protected:
    void mousePressEvent(QMouseEvent *ev) override;

    DkFileInfo fileInfo;
};

class DkDirectoryEdit : public QLineEdit
{
    Q_OBJECT

public:
    DkDirectoryEdit(QWidget *parent = 0);
    DkDirectoryEdit(const QString &content, QWidget *parent = 0);

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
    DkDirectoryChooser(const QString &dirPath = "", QWidget *parent = 0);

public slots:
    void onDirButtonClicked();

signals:
    void directoryChanged(const QString &dirPath) const;

protected:
    void createLayout(const QString &dirPath);

    DkDirectoryEdit *mDirEdit = 0;
};

class DkDelayedInfo : public QObject
{
    Q_OBJECT

public:
    DkDelayedInfo(int time = 0, QObject *parent = 0)
        : QObject(parent)
    {
        timer = new QTimer();
        timer->setSingleShot(true);

        if (time)
            timer->start(time);

        connect(timer, &QTimer::timeout, this, &DkDelayedInfo::sendInfo);
    }

    virtual ~DkDelayedInfo()
    {
        if (timer && timer->isActive())
            timer->stop();

        if (timer)
            delete timer;

        timer = 0;
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
    DkDelayedMessage(const QString &msg = QString(), int time = 0, QObject *parent = 0)
        : DkDelayedInfo(time, parent)
    {
        mMsg = msg;
    }
    DkDelayedMessage(QObject *parent = 0)
        : DkDelayedInfo(0, parent)
    {
    }

    ~DkDelayedMessage()
    {
    }

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
    DkListWidget(QWidget *parent);

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
    DkProgressBar(QWidget *parent = 0);

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

class DllCoreExport DkGenericProfileWidget : public DkNamedWidget
{
    Q_OBJECT

public:
    DkGenericProfileWidget(const QString &name, QWidget *parent, const QString &settingsPath = "");
    virtual ~DkGenericProfileWidget();

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

class DllCoreExport DkTabEntryWidget : public QPushButton
{
    Q_OBJECT

public:
    DkTabEntryWidget(const QIcon &icon, const QString &text, QWidget *parent);

protected:
    void paintEvent(QPaintEvent *event) override;
};

class DllCoreExport DkDisplayWidget : public DkFadeWidget
{
    Q_OBJECT

public:
    DkDisplayWidget(QWidget *parent);

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
