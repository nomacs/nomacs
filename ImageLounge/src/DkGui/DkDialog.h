/*******************************************************************************************************
 DkDialog.h
 Created on:	21.04.2011

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

#pragma warning(push, 0) // no warnings from includes - begin
#include <QDialog>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFutureWatcher>
#include <QItemDelegate>
#include <QLineEdit>
#include <QMainWindow>
#include <QPrintPreviewWidget>
#pragma warning(pop) // no warnings from includes - end

#include "DkBasicLoader.h"

// Qt defines
class QStandardItemModel;
class QStandardItem;
class QTableView;
class QStringListModel;
class QListView;
class QTextEdit;
class QListWidget;
class QDialogButtonBox;
class QTreeView;
class QActionGroup;
class QPrintDialog;
class QAbstractButton;
class QLabel;
class QComboBox;
class QCheckBox;
class QProgressBar;
class QSettings;

namespace nmc
{

// nomacs defines
class DkBaseViewPort;
class TreeItem;
class DkSlider;
class DkButton;
class DkAppManager;
class DkDisplayWidget;
class DkCentralWidget;

namespace DkDialog
{
QFileDialog::Options fileDialogOptions();
};

// needed because of http://stackoverflow.com/questions/1891744/pyqt4-qspinbox-selectall-not-working-as-expected
// and http://qt-project.org/forums/viewthread/8590
class DkSelectAllLineEdit : public QLineEdit
{
public:
    DkSelectAllLineEdit(QWidget *parent = 0)
        : QLineEdit(parent)
    {
        selectOnMousePressEvent = false;
    };

protected:
    void focusInEvent(QFocusEvent *event)
    {
        QLineEdit::focusInEvent(event);
        selectAll();
        selectOnMousePressEvent = true;
    }

    void mousePressEvent(QMouseEvent *event)
    {
        QLineEdit::mousePressEvent(event);
        if (selectOnMousePressEvent) {
            selectAll();
            selectOnMousePressEvent = false;
        }
    }

private:
    bool selectOnMousePressEvent;
};

class DkSelectAllDoubleSpinBox : public QDoubleSpinBox
{
public:
    DkSelectAllDoubleSpinBox(QWidget *parent = 0)
        : QDoubleSpinBox(parent)
    {
        DkSelectAllLineEdit *le = new DkSelectAllLineEdit(this);
        setLineEdit(le);
    };
};

class DkSplashScreen : public QDialog
{
    Q_OBJECT

public:
    DkSplashScreen(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~DkSplashScreen();

private:
    QPoint mDragStart;
    QTimer *mTimer;
    QPushButton *mCloseButton;
    QPushButton *mCopyButton;

    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void showButtons();
};

class DkFileValidator : public QValidator
{
    Q_OBJECT

public:
    DkFileValidator(const QString &lastFile = QString(), QObject *parent = 0);

    void setLastFile(const QString &lastFile)
    {
        mLastFile = lastFile;
    };
    virtual void fixup(QString &input) const;
    virtual State validate(QString &input, int &pos) const;

protected:
    QString mLastFile;
};

class DkTrainDialog : public QDialog
{
    Q_OBJECT

public:
    DkTrainDialog(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());

    QString getAcceptedFile()
    {
        return mAcceptedFile;
    };

    void setCurrentFile(const QString &file)
    {
        mFile = file;
    };

public slots:
    void textChanged(const QString &text);
    void loadFile(const QString &filePath = "");
    void openFile();
    void accept() override;

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void createLayout();
    void userFeedback(const QString &msg, bool error = false);

    DkFileValidator mFileValidator;
    QDialogButtonBox *mButtons;
    QLineEdit *mPathEdit;
    QLabel *mFeedbackLabel;
    DkBaseViewPort *mViewport;

    QString mAcceptedFile;
    QString mFile;
};

class DkAppManagerDialog : public QDialog
{
    Q_OBJECT

public:
    DkAppManagerDialog(DkAppManager *manager = 0, QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());

public slots:
    void onAddButtonClicked();
    void onDeleteButtonClicked();
    void onRunButtonClicked();
    virtual void accept() override;

signals:
    void openWithSignal(QAction *act);

protected:
    DkAppManager *manager;
    QStandardItemModel *model;

    void createLayout();
    QList<QStandardItem *> getItems(QAction *action);
    QTableView *appTableView;
};

class DkSearchDialog : public QDialog
{
    Q_OBJECT

public:
    enum Buttons {
        cancel_button = 0,
        find_button,
        filter_button,

        button_end,
    };

    DkSearchDialog(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());

    void setFiles(const QStringList &fileList);
    void setPath(const QString &dirPath);
    bool filterPressed() const;
    void setDefaultButton(int defaultButton = find_button);

public slots:
    void onSearchBarTextChanged(const QString &text);
    void onFilterButtonPressed();
    void onResultListViewDoubleClicked(const QModelIndex &modelIndex);
    void onResultListViewClicked(const QModelIndex &modelIndex);
    virtual void accept() override;

signals:
    void loadFileSignal(const QString &filePath) const;
    void filterSignal(const QString &) const;

protected:
    void updateHistory();
    void init();
    QStringList makeViewable(const QStringList &resultList, bool forceAll = false);

    QStringListModel *mStringModel = 0;
    QListView *mResultListView = 0;
    QLineEdit *mSearchBar = 0;
    QDialogButtonBox *mButtons = 0;

    QPushButton *mFilterButton = 0;

    QString mCurrentSearch;

    QString mPath;
    QStringList mFileList;
    QStringList mResultList;

    QString mEndMessage;

    bool mAllDisplayed = true;
    bool mIsFilterPressed = false;
};

class DkResizeDialog : public QDialog
{
    Q_OBJECT

public:
    DkResizeDialog(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
    ~DkResizeDialog(){};

    enum {
        ipl_nearest,
        ipl_area,
        ipl_linear,
        ipl_cubic,
        ipl_lanczos,
        ipl_end
    };
    enum {
        size_pixel,
        size_percent,
        size_end
    };
    enum {
        unit_cm,
        unit_mm,
        unit_inch,
        unit_end
    };
    enum {
        res_ppi,
        res_ppc,
        res_end
    };

    void setImage(const QImage &img);
    QImage getResizedImage();
    void setExifDpi(float exifDpi);
    float getExifDpi();
    bool resample();

protected slots:
    void onLockButtonDimClicked();
    void onLockButtonClicked();

    void onWPixelSpinValueChanged(double val);
    void onHPixelSpinValueChanged(double val);

    void onWidthSpinValueChanged(double val);
    void onHeightSpinValueChanged(double val);
    void onResolutionSpinValueChanged(double val);

    void onSizeBoxCurrentIndexChanged(int idx);
    void onUnitBoxCurrentIndexChanged(int idx);
    void onResUnitBoxCurrentIndexChanged(int idx);
    void onResampleBoxCurrentIndexChanged(int idx);

    void onResampleCheckClicked();
    void onGammaCorrectionClicked();

    void drawPreview();

    void setVisible(bool visible) override;

public slots:
    virtual void accept() override;

protected:
    QImage mImg;
    QLabel *mPreviewLabel = 0;

    DkBaseViewPort *mOrigView = 0;

    // resize gui:
    QDoubleSpinBox *mWPixelSpin = 0;
    QDoubleSpinBox *mHPixelSpin = 0;
    DkButton *mLockButton = 0;

    QDoubleSpinBox *mWidthSpin = 0;
    QDoubleSpinBox *mHeightSpin = 0;
    QComboBox *mUnitBox = 0;
    QComboBox *mSizeBox = 0;
    DkButton *mLockButtonDim = 0;

    QDoubleSpinBox *mResolutionSpin = 0;
    QComboBox *mResUnitBox = 0;
    QCheckBox *mResampleCheck = 0;
    QCheckBox *mGammaCorrection = 0;
    QComboBox *mResampleBox = 0;

    float mExifDpi = 72;
    QVector<float> mUnitFactor;
    QVector<float> mResFactor;

    void init();
    void initBoxes(bool updateSettings = false);
    void createLayout();
    void updateSnippets();
    void updateHeight();
    void updateWidth();
    void updatePixelWidth();
    void updatePixelHeight();
    void updateResolution();
    void loadSettings();
    void saveSettings();
    QImage resizeImg(QImage img, bool silent = true);
    void resizeEvent(QResizeEvent *re) override;
};

class DkShortcutDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    DkShortcutDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

signals:
    void checkDuplicateSignal(const QString &text, void *item);
    void checkDuplicateSignal(const QKeySequence &keySequence, void *item);
    void clearDuplicateSignal() const;

protected slots:
    void textChanged(const QString &text = QString());
    void keySequenceChanged(const QKeySequence &keySequence);

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void *mItem;

    QPixmap mClearPm;
};

class DkShortcutsModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    DkShortcutsModel(QObject *parent = 0);
    ~DkShortcutsModel();

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &index) const;

    // return item of the model
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    void addDataActions(QVector<QAction *> actions, const QString &name);

    void resetActions();
    void saveActions() const;

public slots:
    void checkDuplicate(const QString &text, void *item); // deprecated (Qt4)
    void checkDuplicate(const QKeySequence &ks, void *item);
    void clearDuplicateInfo() const;

signals:
    void duplicateSignal(const QString &info) const;

protected:
    TreeItem *mRootItem;
    QVector<QVector<QAction *>> mActions;
};

class DkShortcutsDialog : public QDialog
{
    Q_OBJECT

public:
    DkShortcutsDialog(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());

    void addActions(const QVector<QAction *> &actions, const QString &name);

public slots:
    void accept() override;

protected slots:
    void contextMenu(const QPoint &cur);
    void defaultButtonClicked();

protected:
    void createLayout();

    DkShortcutsModel *mModel;
    QLabel *mNotificationLabel;
    QPushButton *mDefaultButton;
};

class DkTextDialog : public QDialog
{
    Q_OBJECT

public:
    DkTextDialog(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());

    void setText(const QStringList &text);
    QTextEdit *getTextEdit()
    {
        return textEdit;
    };

public slots:
    virtual void save();

protected:
    void createLayout();

    QTextEdit *textEdit;
};

class DkUpdateDialog : public QDialog
{
    Q_OBJECT

public:
    DkUpdateDialog(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
    QLabel *upperLabel;

signals:
    void startUpdate();

protected slots:
    void okButtonClicked();

protected:
    void init();
    void createLayout();

    QPushButton *okButton;
    QPushButton *cancelButton;
};

class DkPrintImage
{
public:
    DkPrintImage(const QImage &img = QImage(), QPrinter *printer = 0);

    QImage image() const;
    void draw(QPainter &p, bool highQuality = false);

    void fit();
    void center();
    void scale(double sf);

    double dpi();

private:
    void center(QTransform &t) const;

    QImage mImg;
    QPrinter *mPrinter;

    QTransform mTransform;
};

class DkPrintPreviewWidget : public QPrintPreviewWidget
{
    Q_OBJECT

public:
    DkPrintPreviewWidget(QPrinter *printer, QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());

    void setImage(const QImage &img);
    void addImage(const QImage &img);
    void fitImages();

public slots:
    void paintForPrinting();
    void paintPreview(QPrinter *printer);
    void changeDpi(int value);
    void centerImage();

    void setLandscapeOrientation();
    void setPortraitOrientation();

signals:
    void zoomChanged() const;
    void dpiChanged(int dpi);

protected:
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;

private:
    QPrinter *mPrinter;
    QVector<QSharedPointer<DkPrintImage>> mPrintImages;
};

class DkPrintPreviewDialog : public QDialog
{
    Q_OBJECT

public:
    enum icons {
        print_fit_width,
        print_fit_page,
        print_zoom_out,
        print_zoom_in,
        print_reset_dpi,
        print_portrait,
        print_landscape,
        print_setup,
        print_printer,
        print_end,
    };

    DkPrintPreviewDialog(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());

    void setImage(const QImage &img);
    void addImage(const QImage &img);

    void init();

protected:
    void createLayout();
    void createIcons();

private slots:
    void zoomIn();
    void zoomOut();
    void updateDpiFactor(qreal dpi);
    void pageSetup();
    void print();

    void zoom(int scale);
    void previewFitWidth();
    void previewFitPage();

private:
    QSpinBox *mDpiBox = 0;

    DkPrintPreviewWidget *mPreview = 0;
    QPrinter *mPrinter = 0;

    QVector<QIcon> mIcons;
};

class DkOpacityDialog : public QDialog
{
    Q_OBJECT

public:
    DkOpacityDialog(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
    int value() const;

protected:
    void createLayout();

    DkSlider *slider;
};

class DkExportTiffDialog : public QDialog
{
    Q_OBJECT

public:
    DkExportTiffDialog(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

public slots:
    void onOpenButtonPressed();
    void onSaveButtonPressed();
    void onFileEditTextChanged(const QString &filename);
    void setFile(const QString &filePath);
    void accept() override;
    void reject() override;
    int exportImages(const QString &saveFilePath, int from, int to, bool overwrite);
    void processingFinished();

signals:
    void updateImage(const QImage &img) const;
    void updateProgress(int) const;
    void infoMessage(const QString &msg) const;

protected:
    void createLayout();
    void enableTIFFSave(bool enable);
    void enableAll(bool enable);

    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;

    DkBaseViewPort *mViewport;
    QLabel *mTiffLabel;
    QLabel *mFolderLabel;
    QLineEdit *mFileEdit;
    QComboBox *mSuffixBox;
    QSpinBox *mFromPage;
    QSpinBox *mToPage;
    QDialogButtonBox *mButtons;
    QProgressBar *mProgress;
    QLabel *mMsgLabel;
    QWidget *mControlWidget;
    QCheckBox *mOverwrite;

    QString mFilePath;
    QString mSaveDirPath;
    DkBasicLoader mLoader;
    QFutureWatcher<int> mWatcher;

    bool mProcessing = false;

    enum {
        finished,
        question_save,
        error,
    };
};

#ifdef WITH_OPENCV

class DkMosaicDialog : public QDialog
{
    Q_OBJECT

public:
    DkMosaicDialog(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
    QImage getImage();

public slots:
    void onOpenButtonPressed();
    void onDbButtonPressed();
    void onFileEditTextChanged(const QString &filename);
    void onNewWidthBoxValueChanged(int i);
    void onNewHeightBoxValueChanged(int i);
    void onNumPatchesVValueChanged(int i);
    void onNumPatchesHValueChanged(int i);
    void onDarkenSliderValueChanged(int i);
    void onLightenSliderValueChanged(int i);
    void onSaturationSliderValueChanged(int i);

    void setFile(const QString &file);
    void compute();
    void reject() override;
    int computeMosaic(const QString &filter, const QString &suffix, int from, int to); // TODO: make const!
    void mosaicFinished();
    void postProcessFinished();
    void buttonClicked(QAbstractButton *button);
    void updatePatchRes();

signals:
    void updateImage(const QImage &img) const;
    void updateProgress(int) const;
    void infoMessage(const QString &msg) const;

protected:
    void updatePostProcess();
    bool postProcessMosaic(float multiply = 0.3f, float screen = 0.5f, float saturation = 0.5f, bool computePreview = true);
    void createLayout();
    void enableMosaicSave(bool enable);
    void enableAll(bool enable);
    QString getRandomImagePath(const QString &cPath, const QString &ignore, const QString &suffix);
    void matchPatch(const cv::Mat &img, const cv::Mat &thumb, int patchRes, cv::Mat &cc);
    cv::Mat createPatch(const QImage &thumb, const QString &filePath, int patchRes);

    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;

    DkBaseViewPort *mViewport = 0;
    DkBaseViewPort *mPreview = 0;
    QLabel *mFileLabel = 0;
    QLabel *mFolderLabel = 0;
    QLineEdit *mFilterEdit = 0;
    QComboBox *mSuffixBox = 0;
    QSpinBox *mNewWidthBox = 0;
    QSpinBox *mNewHeightBox = 0;
    QSpinBox *mNumPatchesV = 0;
    QSpinBox *mNumPatchesH = 0;
    QDialogButtonBox *mButtons = 0;
    QProgressBar *mProgress = 0;
    QLabel *mMsgLabel = 0;
    QWidget *mControlWidget = 0;
    QCheckBox *mOverwrite = 0;
    QLabel *mRealResLabel = 0;
    QLabel *mPatchResLabel = 0;

    QWidget *mSliderWidget = 0;
    QSlider *mDarkenSlider = 0;
    QSlider *mLightenSlider = 0;
    QSlider *mSaturationSlider = 0;

    QString mFilePath;
    QString mSavePath;
    DkBasicLoader mLoader;
    QFutureWatcher<int> mMosaicWatcher;
    QFutureWatcher<bool> mPostProcessWatcher;

    bool mUpdatePostProcessing = false;
    bool mPostProcessing = false;
    bool mProcessing = false;
    cv::Mat mOrigImg;
    cv::Mat mMosaicMat;
    cv::Mat mMosaicMatSmall;
    QImage mMosaic;
    QVector<QFileInfo> mFilesUsed;

    enum {
        finished,
        question_save,
        error,
    };
};
#endif

class DkForceThumbDialog : public QDialog
{
    Q_OBJECT

public:
    DkForceThumbDialog(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

    bool forceSave() const;
    void setDir(const QDir &fileInfo);

protected:
    void createLayout();

    QLabel *infoLabel;
    QCheckBox *cbForceSave;
};

class DkWelcomeDialog : public QDialog
{
    Q_OBJECT

public:
    DkWelcomeDialog(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

    bool isLanguageChanged();

public slots:
    virtual void accept() override;

protected:
    void createLayout();

    QComboBox *mLanguageCombo = 0;
    QCheckBox *mRegisterFilesCheckBox = 0;
    QCheckBox *mSetAsDefaultCheckBox = 0;
    QStringList mLanguages;
    bool mLanguageChanged = false;
};

class DkSvgSizeDialog : public QDialog
{
    Q_OBJECT

public:
    DkSvgSizeDialog(const QSize &size, QWidget *parent);

    QSize size() const;

public slots:
    void onWidthValueChanged(int val);
    void onHeightValueChanged(int val);

private:
    void createLayout();

    enum {
        b_width,
        b_height,

        b_end
    };

    double mARatio = 0;
    QSize mSize;
    QVector<QSpinBox *> mSizeBox;
};

class DkChooseMonitorDialog : public QDialog
{
    Q_OBJECT

public:
    DkChooseMonitorDialog(QWidget *parent);

    QRect screenRect() const;
    bool showDialog() const;

public slots:
    int exec() override;

private:
    void createLayout();
    QList<QScreen *> screens() const;
    void loadSettings();
    void saveSettings() const;

    QList<QScreen *> mScreens;
    DkDisplayWidget *mDisplayWidget;
    QCheckBox *mCbRemember;
};

#ifdef WITH_QUAZIP
class DkArchiveExtractionDialog : public QDialog
{
    Q_OBJECT

public:
    DkArchiveExtractionDialog(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());

    void setCurrentFile(const QString &filePath, bool isZip);

public slots:
    void textChanged(const QString &text);
    void checkbocChecked(int state);
    void dirTextChanged(const QString &text);
    void loadArchive(const QString &filePath = "");
    void openArchive();
    void openDir();
    void accept() override;

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void createLayout();
    void userFeedback(const QString &msg, bool error = false);
    QStringList extractFilesWithProgress(const QString &fileCompressed, const QStringList &files, const QString &dir, bool removeSubfolders);

    DkFileValidator mFileValidator;
    QDialogButtonBox *mButtons = 0;
    QLineEdit *mArchivePathEdit = 0;
    QLineEdit *mDirPathEdit = 0;
    QListWidget *mFileListDisplay = 0;
    QLabel *mFeedbackLabel = 0;
    QCheckBox *mRemoveSubfolders = 0;

    QStringList mFileList;
    QString mFilePath;
};
#endif

class DkDialogManager : public QObject
{
    Q_OBJECT

public:
    DkDialogManager(QObject *parent = 0);

    void setCentralWidget(DkCentralWidget *cw);

public slots:
    void openShortcutsDialog() const;
    void openAppManager() const;
    void openMosaicDialog() const;
    void openPrintDialog() const;

private:
    DkCentralWidget *mCentralWidget = 0;
};

}
