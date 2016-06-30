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

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QWidget>
#include <QUrl>
#include <QTextEdit>
#include <QPushButton>
#pragma warning(pop)		// no warnings from includes - end

#include "DkImageContainer.h"
#include "DkImageLoader.h"

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

namespace nmc {

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
class DkResizeBatch;
class DkPluginBatch;
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

class DkBatchContent {

public:
	virtual bool hasUserInput() const = 0;
	virtual bool requiresUserInput() const = 0;
	virtual void applyDefault() = 0;
};

class DkBatchTabButton : public QPushButton {
	Q_OBJECT

public:
	DkBatchTabButton(const QString& title, const QString& info = QString(), QWidget* parent = 0);

	QString info() const;

public slots:
	void setInfo(const QString& info);

signals:
	void infoChanged(const QString& msg) const;

protected:
	void paintEvent(QPaintEvent* event) override;

	QString mInfo;
};

class DkBatchContainer : public QObject {
	Q_OBJECT

public:
	DkBatchContainer(const QString& titleString, const QString& headerString, QWidget* parent = 0);
	
	void setContentWidget(QWidget* batchContent);
	QWidget* contentWidget() const;
	DkBatchContent* batchContent() const;
	DkBatchTabButton* headerWidget() const;

public slots:
	void showContent(bool show) const;

signals:
	void showSignal() const;

protected:
	virtual void createLayout();

	DkBatchTabButton* mHeaderButton = 0;
	DkBatchContent* mBatchContent = 0;
};

class DkInputTextEdit : public QTextEdit {
	Q_OBJECT

public:
	DkInputTextEdit(QWidget* parent = 0);

	QStringList getFileList() const;
	void appendDir(const QString& newDir, bool recursive = false);
	void insertFromMimeData(const QMimeData *src);
	void clear();

signals:
	void fileListChangedSignal() const;

public slots:
	void appendFiles(const QStringList& fileList);

protected:
	void dropEvent(QDropEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void appendFromMime(const QMimeData* mimeData, bool recursive = false);

	QList<int> mResultList;
};

class DkFileSelection : public QWidget, public DkBatchContent  {
	Q_OBJECT

public:

	enum {
		tab_thumbs = 0,
		tab_text_input,
		tab_results,

		tab_end
	};

	DkFileSelection(QWidget* parent = 0, Qt::WindowFlags f = 0);

	QString getDir() const;
	QStringList getSelectedFiles() const;
	QStringList getSelectedFilesBatch();
	DkInputTextEdit* getInputEdit() const;

	virtual bool hasUserInput() const {return mHUserInput;};
	virtual bool requiresUserInput() const {return mRUserInput;};
	virtual void applyDefault();

	void changeTab(int tabIdx) const;
	void startProcessing();
	void stopProcessing();
	void setResults(const QStringList& results);

public slots:
	void setDir(const QString& dirPath);
	void browse();
	void updateDir(QVector<QSharedPointer<DkImageContainerT> >);
	void setVisible(bool visible);
	void emitChangedSignal();
	void selectionChanged();
	void setFileInfo(QFileInfo file);

signals:
	void updateDirSignal(QVector<QSharedPointer<DkImageContainerT> >) const;
	void newHeaderText(const QString&) const;
	void updateInputDir(const QString&) const;
	void changed() const;

protected:
	virtual void createLayout();

	QString mCDirPath;
	QListView* mFileWidget = 0;
	DkThumbScrollWidget* mThumbScrollWidget = 0;
	DkInputTextEdit* mInputTextEdit = 0;
	QTextEdit* mResultTextEdit = 0;
	DkExplorer* mExplorer = 0;
	DkDirectoryEdit* mDirectoryEdit = 0;
	QTabWidget* mInputTabs = 0;
	QSharedPointer<DkImageLoader> mLoader = QSharedPointer<DkImageLoader>(new DkImageLoader());

private:
	bool mHUserInput = false;
	bool mRUserInput = false;

};

class DkFilenameWidget : public QWidget {
	Q_OBJECT

public:	
	DkFilenameWidget(QWidget* parent = 0);
	void enableMinusButton(bool enable);
	void enablePlusButton(bool enable);
	bool hasUserInput() const {return hasChanged;};
	QString getTag() const;

signals:
	void plusPressed(DkFilenameWidget*) const;
	void minusPressed(DkFilenameWidget*) const;
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

	QComboBox* mCbType = 0;
		
	QLineEdit* mLeText = 0;
	QComboBox* mCbCase = 0;

	QSpinBox* mSbNumber = 0;
	QComboBox* mCbDigits = 0;
		
	QPushButton* mPbPlus = 0;
	QPushButton* mPbMinus = 0;

	QGridLayout* mLayout = 0;

	bool hasChanged = false;
};

class DkBatchOutput : public QWidget, public DkBatchContent {
Q_OBJECT

public:
	DkBatchOutput(QWidget* parent = 0, Qt::WindowFlags f = 0);

	virtual bool hasUserInput() const;
	virtual bool requiresUserInput() const {return mRUserInput;};
	void applyDefault();
	void loadProperties(const DkBatchConfig& config);

	int overwriteMode() const;
	int getCompression() const;
	bool useInputDir() const;
	bool deleteOriginal() const;
	QString getOutputDirectory();
	QString getFilePattern();
	void setExampleFilename(const QString& exampleName);

signals:
	void newHeaderText(const QString&) const;
	void changed() const;

public slots:
	void setInputDir(const QString& dirPath);

protected slots:
	void browse();
	void plusPressed(DkFilenameWidget* widget);
	void minusPressed(DkFilenameWidget* widget);
	void extensionCBChanged(int index);
	void emitChangedSignal();
	void updateFileLabelPreview();
	void useInputFolderChanged(bool checked);
	void setDir(const QString& dirPath, bool updateLineEdit = true);

protected:
	virtual void createLayout();

private:

	bool mHUserInput = false;
	bool mRUserInput = false;
	QString mOutputDirectory;
	QString mInputDirectory;
	QVector<DkFilenameWidget*> mFilenameWidgets;
	DkDirectoryEdit* mOutputlineEdit = 0;
	QVBoxLayout* mFilenameVBLayout = 0;
	QCheckBox* mCbOverwriteExisting = 0;
	QCheckBox* mCbUseInput = 0;
	QCheckBox* mCbDeleteOriginal = 0;
	QPushButton* mOutputBrowseButton = 0;

	QComboBox* mCbExtension = 0;
	QComboBox* mCbNewExtension = 0;
	QSpinBox* mSbCompression = 0;
	QLabel* mOldFileNameLabel = 0;
	QLabel* mNewFileNameLabel = 0;
	QString mExampleName = 0;

};

class DkBatchResizeWidget : public QWidget, public DkBatchContent {
	Q_OBJECT

public:
	DkBatchResizeWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);

	void transferProperties(QSharedPointer<DkResizeBatch> batchResize) const;
	bool loadProperties(QSharedPointer<DkResizeBatch> batchResize) const;
	bool hasUserInput() const;
	bool requiresUserInput() const;
	virtual void applyDefault();

public slots:
	void modeChanged(int idx);
	void percentChanged(double val);
	void pxChanged(int val);

signals:
	void newHeaderText(const QString& txt) const;

protected:
	void createLayout();

	QComboBox* mComboMode;
	QComboBox* mComboProperties;
	QSpinBox* mSbPx;
	QDoubleSpinBox* mSbPercent;
};

class DkProfileWidget : public QWidget, public DkBatchContent {
	Q_OBJECT

public:
	DkProfileWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);

	//void transferProperties(QSharedPointer<DkResizeBatch> batchResize) const;
	bool hasUserInput() const;
	bool requiresUserInput() const;
	void applyDefault();
	void profileSaved(const QString& profileName);

public slots:
	void on_profileCombo_currentIndexChanged(const QString& text);
	void on_saveButton_clicked();

signals:
	void newHeaderText(const QString& txt) const;
	void loadProfileSignal(const QString& profilePath) const;
	void saveProfileSignal(const QString& profilePath) const;
	void applyDefaultSignal() const;

protected:
	void createLayout();
	void saveProfile();
	void updateProfileCombo();

	QComboBox* mProfileCombo;
};


#ifdef WITH_PLUGINS
class DkBatchPluginWidget : public QWidget, public DkBatchContent {
	Q_OBJECT

public:
	DkBatchPluginWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);

	void transferProperties(QSharedPointer<DkPluginBatch> batchPlugin) const;
	bool loadProperties(QSharedPointer<DkPluginBatch> batchPlugin);
	bool hasUserInput() const;
	bool requiresUserInput() const;
	void applyDefault();

public slots:
	void selectPlugin(const QString& actionName, bool select = true);

signals:
	void newHeaderText(const QString& txt) const;

public slots:
	void updateHeader() const;

protected:
	void createLayout();
	QStringList getPluginActionNames() const;

	DkListWidget* mLoadedPluginList;
	DkListWidget* mSelectedPluginList;

};
#endif

class DkBatchTransformWidget : public QWidget, public DkBatchContent {
	Q_OBJECT

public:
	DkBatchTransformWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);

	void transferProperties(QSharedPointer<DkBatchTransform> batchTransform) const;
	bool loadProperties(QSharedPointer<DkBatchTransform> batchTransform);
	bool hasUserInput() const;
	bool requiresUserInput() const;
	void applyDefault();

public slots:
	void updateHeader() const;

signals:
	void newHeaderText(const QString& txt) const;

protected:
	void createLayout();
	int getAngle() const;

	QButtonGroup* mRotateGroup = 0;
	QRadioButton* mRbRotate0 = 0;
	QRadioButton* mRbRotateLeft = 0;
	QRadioButton* mRbRotateRight = 0;
	QRadioButton* mRbRotate180 = 0;

	QCheckBox* mCbFlipH = 0;
	QCheckBox* mCbFlipV = 0;
	QCheckBox* mCbCropMetadata = 0;
};

class DkBatchWidget : public QWidget {
	Q_OBJECT

public:
	DkBatchWidget(const QString& currentDirectory = QString(), QWidget* parent = 0);
	~DkBatchWidget();

	enum batchWidgets {
		batch_input,
		batch_resize,
		batch_transform,
		batch_plugin,
		batch_output,
		batch_profile,

		batchWidgets_end
	};

public slots:
	void accept();
	bool close();
	void widgetChanged();
	void logButtonClicked();
	void processingFinished();
	void updateProgress(int progress);
	void updateLog();
	void setSelectedFiles(const QStringList& selFiles);
	void changeWidget(DkBatchContainer* widget = 0);
	void nextTab();
	void previousTab();
	void saveProfile(const QString& profilePath) const;
	void loadProfile(const QString& profilePath);
	void applyDefault();

protected:
	void createLayout();
	DkBatchConfig createBatchConfig(bool strict = true) const;
		
	QVector<DkBatchContainer*> mWidgets;
		
	QStackedLayout* mCentralLayout = 0;
	int mCurrentIndex = 0;

	QString mCurrentDirectory;
	QDialogButtonBox* mButtons = 0;
	DkFileSelection* mFileSelection = 0;
	DkBatchOutput* mOutputSelection = 0;
	DkBatchResizeWidget* mResizeWidget = 0;
	DkProfileWidget* mProfileWidget = 0;

#ifdef WITH_PLUGINS
	DkBatchPluginWidget* mPluginWidget = 0;
#endif

	DkBatchTransformWidget* mTransformWidget = 0;
	DkBatchProcessing* mBatchProcessing = 0;
	QPushButton* mLogButton = 0;
	QProgressBar* mProgressBar= 0;
	QLabel* mSummaryLabel = 0;
	QTimer mLogUpdateTimer;
	bool mLogNeedsUpdate = false;

	// title
	QLabel* mContentTitle = 0;
	QLabel* mContentInfo = 0;

	void startProcessing();
	void stopProcessing();
};

}
