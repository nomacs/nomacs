/*******************************************************************************************************
DkNoMacs.h
Created on:	26.10.2014

nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

Copyright (C) 2011-2014 Markus Diem <markus@nomacs.org>
Copyright (C) 2011-2014 Stefan Fiel <stefan@nomacs.org>
Copyright (C) 2011-2014 Florian Kleber <florian@nomacs.org>

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
#include <QItemSelection>
#include <QPushButton>
#include <QTextEdit>
#include <QUrl>
#include <QWidget>
#pragma warning(pop) // no warnings from includes - end

#include "DkBaseWidgets.h"
#include "DkBatchInfo.h"
#include "DkImageContainer.h"
#include "DkImageLoader.h"
#include "DkManipulators.h"
#include "DkThumbs.h"

// Qt defines
class QListView;
class QVBoxLayout;
class QLabel;
class QFileInfo;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QGridLayout;
class QCheckBox;
class QButtonGroup;
class QRadioButton;
class QDialogButtonBox;
class QProgressBar;
class QTabWidget;
class QListWidget;
class QStackedLayout;
class QStandardItem;
class QStandardItemModel;

namespace nmc
{

// TODO: these enums are global - they should be put into the respective classes
enum fileNameTypes {
    fileNameTypes_fileName,
    fileNameTypes_Number,
    fileNameTypes_Text,

    fileNameTypes_end
};

enum fileNameWidget {
    fileNameWidget_type,
    fileNameWidget_input1,
    fileNameWidget_input2,
    fileNameWidget_plus,
    fileNameWidget_minus,

    fileNameWidget_end
};

// nomacs defines
class DkPluginBatch;
class DkManipulatorBatch;
class DkBatchProcessing;
class DkBatchTransform;
class DkBatchContent;
class DkButton;
class DkThumbScrollWidget;
class DkImageLoader;
class DkExplorer;
class DkDirectoryEdit;
class DkListWidget;
class DkBatchConfig;
class DkProgressBar;
class DkSettingsWidget;
class DkBatchPluginInterface;
class DkRectWidget;

class DkBatchContent : public DkWidget
{
    Q_OBJECT

public:
    DkBatchContent(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
    virtual bool hasUserInput() const = 0;
    virtual bool requiresUserInput() const = 0;
    virtual void applyDefault() = 0;

signals:
    void newHeaderText(const QString &txt) const;
};

class DkBatchTabButton : public QPushButton
{
    Q_OBJECT

public:
    DkBatchTabButton(const QString &title, const QString &info = QString(), QWidget *parent = 0);

    QString info() const;

public slots:
    void setInfo(const QString &info);

signals:
    void infoChanged(const QString &msg) const;

protected:
    void paintEvent(QPaintEvent *event) override;

    QString mInfo;
};

class DkBatchContainer : public QObject
{
    Q_OBJECT

public:
    DkBatchContainer(const QString &titleString, const QString &headerString, QWidget *parent = 0);

    void setContentWidget(QWidget *batchContent);
    QWidget *contentWidget() const;
    DkBatchContent *batchContent() const;
    DkBatchTabButton *headerWidget() const;

public slots:
    void showContent(bool show) const;

signals:
    void showSignal() const;

protected:
    virtual void createLayout();

    DkBatchTabButton *mHeaderButton = 0;
    DkBatchContent *mBatchContent = 0;
};

class DkInputTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    DkInputTextEdit(QWidget *parent = 0);

    QStringList getFileList() const;
    void appendDir(const QString &newDir, bool recursive = false);
    void insertFromMimeData(const QMimeData *src) override;
    void clear();
    QString firstDirPath() const;

signals:
    void fileListChangedSignal() const;

public slots:
    void appendFiles(const QStringList &fileList);

protected:
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void appendFromMime(const QMimeData *mimeData, bool recursive = false);

    QList<int> mResultList;
};

class DkBatchInput : public DkBatchContent
{
    Q_OBJECT

public:
    enum {
        tab_thumbs = 0,
        tab_text_input,
        tab_results,

        tab_end
    };

    DkBatchInput(DkThumbLoader *thumbLoader, QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

    QString getDir() const;
    QStringList getSelectedFiles() const;
    QStringList getSelectedFilesBatch();
    DkInputTextEdit *getInputEdit() const;

    virtual bool hasUserInput() const override
    {
        return mHUserInput;
    };
    virtual bool requiresUserInput() const override
    {
        return mRUserInput;
    };
    virtual void applyDefault() override;

    void changeTab(int tabIdx) const;
    void startProcessing();
    void stopProcessing();
    void setResults(const QStringList &results);

public slots:
    void setDir(const QString &dirPath);
    void browse();
    void updateDir(QVector<QSharedPointer<DkImageContainerT>>);
    void parameterChanged();
    void selectionChanged();
    void setFileInfo(QFileInfo file);

signals:
    void updateDirSignal(QVector<QSharedPointer<DkImageContainerT>>) const;
    void updateInputDir(const QString &) const;
    void changed() const;

protected:
    virtual void createLayout(DkThumbLoader *);

    QString mCDirPath;
    QListView *mFileWidget = 0;
    DkThumbScrollWidget *mThumbScrollWidget = 0;
    DkInputTextEdit *mInputTextEdit = 0;
    QTextEdit *mResultTextEdit = 0;
    DkExplorer *mExplorer = 0;
    DkDirectoryEdit *mDirectoryEdit = 0;
    QTabWidget *mInputTabs = 0;
    QSharedPointer<DkImageLoader> mLoader = QSharedPointer<DkImageLoader>(new DkImageLoader());

private:
    bool mHUserInput = false;
    bool mRUserInput = false;
};

class DkFilenameWidget : public DkWidget
{
    Q_OBJECT

public:
    DkFilenameWidget(QWidget *parent = 0);
    void enableMinusButton(bool enable);
    void enablePlusButton(bool enable);
    bool hasUserInput() const
    {
        return hasChanged;
    };
    QString getTag() const;
    bool setTag(const QString &tag);

signals:
    void plusPressed(DkFilenameWidget *) const;
    void minusPressed(DkFilenameWidget *) const;
    void changed() const;

private slots:
    void typeCBChanged(int index);
    void pbPlusPressed();
    void pbMinusPressed();
    void checkForUserInput();
    void digitCBChanged(int index);

private:
    void createLayout();
    void showOnlyText();
    void showOnlyNumber();
    void showOnlyFilename();

    QComboBox *mCbType = 0;

    QLineEdit *mLeText = 0;
    QComboBox *mCbCase = 0;

    QSpinBox *mSbNumber = 0;
    QComboBox *mCbDigits = 0;

    QPushButton *mPbPlus = 0;
    QPushButton *mPbMinus = 0;

    QGridLayout *mLayout = 0;

    bool hasChanged = false;
};

class DkBatchOutput : public DkBatchContent
{
    Q_OBJECT

public:
    DkBatchOutput(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

    virtual bool hasUserInput() const override;
    virtual bool requiresUserInput() const override
    {
        return mRUserInput;
    };
    void applyDefault() override;
    void loadProperties(const DkBatchConfig &config);

    DkSaveInfo::OverwriteMode overwriteMode() const;
    int getCompression() const;
    bool useInputDir() const;
    bool deleteOriginal() const;
    QString getOutputDirectory();
    QString getFilePattern();
    void loadFilePattern(const QString &pattern);
    void setExampleFilename(const QString &exampleName);

signals:
    void changed() const;

public slots:
    void setInputDir(const QString &dirPath);

protected slots:
    void browse();
    void plusPressed(DkFilenameWidget *widget, const QString &tag = QString());
    void minusPressed(DkFilenameWidget *widget);
    void extensionCBChanged(int index);
    void parameterChanged();
    void updateFileLabelPreview();
    void useInputFolderChanged(bool checked);
    void setDir(const QString &dirPath, bool updateLineEdit = true);

protected:
    virtual void createLayout();
    virtual void updateCBCompression();
    DkFilenameWidget *createFilenameWidget(const QString &tag = QString());
    void addFilenameWidget(const QString &tag = QString());

private:
    bool mHUserInput = false;
    bool mRUserInput = false;
    QString mOutputDirectory;
    QString mInputDirectory;
    QVector<DkFilenameWidget *> mFilenameWidgets;
    DkDirectoryEdit *mOutputlineEdit = 0;
    QVBoxLayout *mFilenameVBLayout = 0;
    QCheckBox *mCbOverwriteExisting = 0;
    QCheckBox *mCbDoNotSave = 0;
    QCheckBox *mCbUseInput = 0;
    QCheckBox *mCbDeleteOriginal = 0;
    QPushButton *mOutputBrowseButton = 0;

    QComboBox *mCbExtension = 0;
    QComboBox *mCbNewExtension = 0;
    QComboBox *mCbCompression = 0;
    QLabel *mOldFileNameLabel = 0;
    QLabel *mNewFileNameLabel = 0;
    QString mExampleName = 0;
};

class DkProfileSummaryWidget : public DkFadeWidget
{
    Q_OBJECT

public:
    DkProfileSummaryWidget(QWidget *parent = 0);

    void setProfile(const QString &name, const DkBatchConfig &config);

public slots:
    void onDeleteButtonClicked();
    void onUpdateButtonClicked();
    void onExportButtonClicked();

signals:
    void deleteCurrentProfile() const;
    void updateCurrentProfile() const;
    void exportCurrentProfile() const;

protected:
    void createLayout();

    QLabel *mTitle = 0;
    QLabel *mNumFiles = 0;
    QLabel *mOutputDir = 0;
    QLabel *mFunctions = 0;
};

class DkProfileWidget : public DkBatchContent
{
    Q_OBJECT

public:
    DkProfileWidget(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

    // void transferProperties(QSharedPointer<DkResizeBatch> batchResize) const;
    bool hasUserInput() const override;
    bool requiresUserInput() const override;
    void applyDefault() override;
    void profileSaved(const QString &profileName);

public slots:
    void onProfileListItemSelectionChanged();
    void onSaveButtonClicked();
    void onResetButtonClicked();

    void updateCurrentProfile();
    void deleteCurrentProfile();
    void exportCurrentProfile();

signals:
    void loadProfileSignal(const QString &profilePath) const;
    void saveProfileSignal(const QString &profilePath) const;
    void applyDefaultSignal() const;

protected:
    void createLayout();
    void saveProfile();
    void updateProfileList();
    void changeProfile(const QString &profileName);
    QString currentProfile() const;
    void loadDefaultProfile();

    QListWidget *mProfileList = 0;
    DkProfileSummaryWidget *mSummary = 0;
};

#ifdef WITH_PLUGINS
class DkBatchPluginWidget : public DkBatchContent
{
    Q_OBJECT

public:
    DkBatchPluginWidget(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

    void transferProperties(QSharedPointer<DkPluginBatch> batchPlugin) const;
    bool loadProperties(QSharedPointer<DkPluginBatch> batchPlugin);
    bool hasUserInput() const override;
    bool requiresUserInput() const override;
    void applyDefault() override;
    void setSettingsPath(const QString &settingsPath);

public slots:
    void itemChanged(QStandardItem *item);
    void selectionChanged(const QItemSelection &selected);

    void changeSetting(const QString &key, const QVariant &value, const QStringList &parentList) const;
    void removeSetting(const QString &key, const QStringList &parentList) const;

public slots:
    void updateHeader() const;

protected:
    void createLayout();
    void addPlugins(QStandardItemModel *model) const;
    void selectPlugin(const QString &pluginName);
    QStringList selectedPlugins(bool selected = true) const;
    QSharedPointer<QSettings> settings() const;

    QStandardItemModel *mModel = 0;
    DkSettingsWidget *mSettingsEditor = 0;
    QLabel *mSettingsTitle = 0;
    QSharedPointer<QSettings> mSettings;
    DkBatchPluginInterface *mCurrentPlugin = 0;
};
#endif

class DkBatchManipulatorWidget : public DkBatchContent
{
    Q_OBJECT

public:
    DkBatchManipulatorWidget(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

    void transferProperties(QSharedPointer<DkManipulatorBatch> batchPlugin) const;
    bool loadProperties(QSharedPointer<DkManipulatorBatch> batchPlugin);
    bool hasUserInput() const override;
    bool requiresUserInput() const override;
    void applyDefault() override;
    void setExampleFile(const QString &filePath);

public slots:
    void itemChanged(QStandardItem *item);
    void selectionChanged(const QItemSelection &selected);
    void selectManipulator(QSharedPointer<DkBaseManipulator> mpl);
    void selectManipulator();

public slots:
    void updateHeader() const;

protected:
    void createLayout();
    void addSettingsWidgets(DkManipulatorManager &manager);
    void setManager(const DkManipulatorManager &manager);

    QStandardItemModel *mModel = 0;
    DkManipulatorManager mManager;
    QVector<QWidget *> mMplWidgets;
    QVBoxLayout *mSettingsLayout = 0;
    QLabel *mSettingsTitle = 0;
    QLabel *mPreviewLabel = 0;
    QString mPreviewPath;
    QImage mPreview;
    int mMaxPreview = 300;
};

class DkBatchTransformWidget : public DkBatchContent
{
    Q_OBJECT

public:
    DkBatchTransformWidget(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

    void transferProperties(QSharedPointer<DkBatchTransform> batchTransform) const;
    bool loadProperties(QSharedPointer<DkBatchTransform> batchTransform);
    bool hasUserInput() const override;
    bool requiresUserInput() const override;
    void applyDefault() override;

public slots:
    void updateHeader() const;
    void modeChanged();

protected:
    void createLayout();
    int getAngle() const;
    QRect cropRect() const;

    QButtonGroup *mRotateGroup = 0;
    QRadioButton *mRbRotate0 = 0;
    QRadioButton *mRbRotateLeft = 0;
    QRadioButton *mRbRotateRight = 0;
    QRadioButton *mRbRotate180 = 0;

    QCheckBox *mCbCropMetadata = 0;

    QCheckBox *mCbCropRectangle = 0;
    DkRectWidget *mCropRectWidget = 0;
    QCheckBox *mCbCropRectCenter = 0;

    QComboBox *mResizeComboMode;
    QComboBox *mResizeComboProperties;
    QSpinBox *mResizeSbPx;
    QLabel *mResizeSbZoomLabel;
    QSpinBox *mResizeSbZoomHeightPx;
    QDoubleSpinBox *mResizeSbPercent;
};

class DkBatchButtonsWidget : public DkWidget
{
    Q_OBJECT

public:
    DkBatchButtonsWidget(QWidget *parent = 0);

    void setPaused(bool paused = true);
    QPushButton *logButton();
    QPushButton *playButton();

signals:
    void playSignal(bool play = true) const;
    void showLogSignal() const;

protected:
    void createLayout();

    QPushButton *mPlayButton = 0;
    QPushButton *mLogButton = 0;
};

class DkBatchInfoWidget : public DkFadeWidget
{
    Q_OBJECT

public:
    DkBatchInfoWidget(QWidget *parent);

    enum InfoMode {
        info_message,
        info_warning,
        info_critical,

        info_end
    };

public slots:
    void setInfo(const QString &message, const DkBatchInfoWidget::InfoMode &mode = DkBatchInfoWidget::InfoMode::info_message);

protected:
    void createLayout();

    QLabel *mInfo = 0;
    QLabel *mIcon = 0;
};

class DkBatchWidget : public DkWidget
{
    Q_OBJECT

public:
    DkBatchWidget(DkThumbLoader *thumbLoader, const QString &currentDirectory = QString(), QWidget *parent = 0);
    ~DkBatchWidget();

    enum batchWidgets {
        batch_input,
        batch_manipulator,
        batch_transform,
        batch_plugin,
        batch_output,
        batch_profile,

        batchWidgets_end
    };

    bool cancel();

public slots:
    void toggleBatch(bool start);
    void widgetChanged();
    void showLog();
    void processingFinished();
    void updateProgress(int progress);
    void updateLog();
    void setSelectedFiles(const QStringList &selFiles);
    void changeWidget(DkBatchContainer *widget = 0);
    void nextTab();
    void previousTab();
    void saveProfile(const QString &profilePath) const;
    void loadProfile(const QString &profilePath);
    void applyDefault();

signals:
    void infoSignal(const QString &message, const DkBatchInfoWidget::InfoMode &mode = DkBatchInfoWidget::InfoMode::info_message) const;

protected:
    void createLayout(DkThumbLoader *);
    DkBatchConfig createBatchConfig(bool strict = true) const;
    void startProcessing();
    void stopProcessing();
    void startBatch();
    DkBatchInput *inputWidget() const;
    DkBatchOutput *outputWidget() const;
    DkBatchManipulatorWidget *manipulatorWidget() const;
    DkProfileWidget *profileWidget() const;
    DkBatchTransformWidget *transformWidget() const;

#ifdef WITH_PLUGINS
    DkBatchPluginWidget *pluginWidget() const;
#endif

    QVector<DkBatchContainer *> mWidgets;

    QStackedLayout *mCentralLayout = 0;
    int mCurrentIndex = 0;

    QString mCurrentDirectory;

    DkBatchProcessing *mBatchProcessing = 0;

    // info/controls
    DkBatchInfoWidget *mInfoWidget = 0;
    DkProgressBar *mProgressBar = 0;
    DkBatchButtonsWidget *mButtonWidget = 0;

    QTimer mLogUpdateTimer;
    bool mLogNeedsUpdate = false;

    // title
    QLabel *mContentTitle = 0;
    QLabel *mContentInfo = 0;
};

}
