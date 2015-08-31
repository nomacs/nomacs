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

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QItemDelegate>
#include <QPrintPreviewWidget>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QMainWindow>
#include <QDialog>
#include <QDir>
#include <QFutureWatcher>
#pragma warning(pop)		// no warnings from includes - end

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

namespace nmc {

// nomacs defines
class DkBaseViewPort;
class TreeItem;
class DkSlider;
class DkButton;
class DkThumbNail;

// needed because of http://stackoverflow.com/questions/1891744/pyqt4-qspinbox-selectall-not-working-as-expected 
// and http://qt-project.org/forums/viewthread/8590
class DkSelectAllLineEdit : public QLineEdit {

public:
	DkSelectAllLineEdit(QWidget* parent = 0) : QLineEdit(parent) {selectOnMousePressEvent = false;};

protected:
	void focusInEvent(QFocusEvent *event) {
		QLineEdit::focusInEvent(event);
		selectAll();
		selectOnMousePressEvent = true;
	}

	void mousePressEvent(QMouseEvent *event) {
		QLineEdit::mousePressEvent(event);
		if (selectOnMousePressEvent) {
			selectAll();
			selectOnMousePressEvent = false;
		}
	}
private:
	bool selectOnMousePressEvent; 
};

class DkSelectAllDoubleSpinBox : public QDoubleSpinBox {
	public:
		DkSelectAllDoubleSpinBox(QWidget* parent = 0) : QDoubleSpinBox(parent) {
			DkSelectAllLineEdit* le = new DkSelectAllLineEdit(this); 
			setLineEdit(le);
		};
};

class DkSplashScreen : public QDialog {
	Q_OBJECT

public:
	DkSplashScreen(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~DkSplashScreen() {};

protected:
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void showClose();

private:
	QPoint mouseGrab;
	QString text;
	QLabel* textLabel;
	QLabel* imgLabel;
	QTimer* showTimer;
	QPushButton* exitButton;
};

class DkFileValidator : public QValidator {
	Q_OBJECT

public:
	DkFileValidator(const QString& lastFile = "", QObject * parent = 0);

	void setLastFile(QString lastFile) {
		mLastFile = lastFile;
	};
	virtual void fixup(QString& input) const;
	virtual State validate(QString& input, int& pos) const;

protected:
	QString mLastFile;
};

class DkTrainDialog : public QDialog {
	Q_OBJECT
	
public:
	DkTrainDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	QString getAcceptedFile() {
		return mAcceptedFile;
	};

	void setCurrentFile(const QString& file) {
		mFile = file;
	};

public slots:
	void textChanged(const QString& text);
	void loadFile(const QString& filePath = "");
	void openFile();
	void accept();

protected:
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);

	void createLayout();
	void userFeedback(const QString& msg, bool error = false);

	DkFileValidator mFileValidator;
	QDialogButtonBox* mButtons;
	QLineEdit* mPathEdit;
	QLabel* mFeedbackLabel;
	DkBaseViewPort* mViewport;
	
	QString mAcceptedFile;
	QString mFile;
};

class DkAppManager : public QObject{
	Q_OBJECT

public:
	DkAppManager(QWidget* parent = 0);
	~DkAppManager();

	void setActions(QVector<QAction* > actions);
	QVector<QAction* > getActions() const;
	QAction* createAction(QString filePath);
	QAction* findAction(QString appPath) const;

	enum defaultAppIdx {

		app_photohsop,
		app_picasa,
		app_picasa_viewer,
		app_irfan_view,
		app_explorer,

		app_idx_end
	};

public slots:
	void openTriggered() const;

signals:
	void openFileSignal(QAction* action) const;

protected:
	void saveSettings() const;
	void loadSettings();
	void assignIcon(QAction* app) const;
	bool containsApp(QVector<QAction* > apps, const QString& appName) const;

	QString searchForSoftware(const QString& organization, const QString& application, const QString& pathKey = "", const QString& exeName = "") const;
	void findDefaultSoftware();

	QVector<QString> mDefaultNames;
	QVector<QAction* > mApps;
	bool mFirstTime = true;
};

class DkAppManagerDialog : public QDialog {
	Q_OBJECT

public:
	DkAppManagerDialog(DkAppManager* manager = 0, QWidget* parent = 0, Qt::WindowFlags flags = 0);

public slots:
	void on_addButton_clicked();
	void on_deleteButton_clicked();
	void on_runButton_clicked();
	virtual void accept();

signals:
	void openWithSignal(QAction* act);

protected:
	DkAppManager* manager;
	QStandardItemModel* model;

	void createLayout();
	QList<QStandardItem* > getItems(QAction* action);
	QTableView* appTableView;
};

class DkSearchDialog : public QDialog {
	Q_OBJECT

public:

	enum Buttons {
		cancel_button = 0,
		find_button,
		filter_button,

		button_end,
	};

	DkSearchDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	void setFiles(const QStringList& fileList);
	void setPath(const QString& dirPath);
	bool filterPressed() const;
	void setDefaultButton(int defaultButton = find_button);

public slots:
	void on_searchBar_textChanged(const QString& text);
	void on_okButton_pressed();
	void on_filterButton_pressed();
	void on_resultListView_doubleClicked(const QModelIndex& modelIndex);
	void on_resultListView_clicked(const QModelIndex& modelIndex);
	virtual void accept();

signals:
	void loadFileSignal(const QString& filePath) const;
	void filterSignal(const QStringList&) const;

protected:

	void updateHistory();
	void init();
	QStringList makeViewable(const QStringList& resultList, bool forceAll = false);

	QStringListModel* mStringModel = 0;
	QListView* mResultListView = 0;
	QLineEdit* mSearchBar = 0;
	QDialogButtonBox* mButtons = 0;

	QPushButton* mFilterButton = 0;

	QString mCurrentSearch;

	QString mPath;
	QStringList mFileList;
	QStringList mResultList;

	QString mEndMessage;

	bool mAllDisplayed = true;
	bool mIsFilterPressed = false;
};

class DkResizeDialog : public QDialog {
	Q_OBJECT

public:
	DkResizeDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~DkResizeDialog() {};

	enum{ipl_nearest, ipl_area, ipl_linear, ipl_cubic, ipl_lanczos, ipl_end};
	enum{size_pixel, size_percent, size_end};
	enum{unit_cm, unit_mm, unit_inch, unit_end};
	enum{res_ppi, res_ppc, res_end};  

	void setImage(QImage img);
	QImage getResizedImage();
	void setExifDpi(float exifDpi);
	float getExifDpi();
	bool resample();

protected slots:
	void on_lockButtonDim_clicked();
	void on_lockButton_clicked();

	void on_wPixelEdit_valueChanged(QString text);
	void on_hPixelEdit_valueChanged(QString text);

	void on_widthEdit_valueChanged(QString text);
	void on_heightEdit_valueChanged(QString text);
	void on_resolutionEdit_valueChanged(QString text);

	void on_sizeBox_currentIndexChanged(int idx);
	void on_unitBox_currentIndexChanged(int idx);
	void on_resUnitBox_currentIndexChanged(int idx);
	void on_resampleBox_currentIndexChanged(int idx);

	void on_resampleCheck_clicked();
	void on_gammaCorrection_clicked();

	void drawPreview();

	void setVisible(bool visible) {
		updateSnippets();
		drawPreview();

		QDialog::setVisible(visible);
	}

public slots:
	virtual void accept();

protected:
	int leftSpacing;
	int margin;
	QImage img;
	QImage origImg;
	QImage newImg;
	QWidget* centralWidget;
	QLabel* previewLabel;
	
	DkBaseViewPort* origView;

	// resize gui:
	QDoubleSpinBox* wPixelEdit;
	QDoubleSpinBox* hPixelEdit;
	DkButton* lockButton;

	QDoubleSpinBox* widthEdit;
	QDoubleSpinBox* heightEdit;
	QComboBox* unitBox;
	QComboBox* sizeBox;
	DkButton* lockButtonDim;

	QDoubleSpinBox* resolutionEdit;
	QComboBox* resUnitBox;
	QCheckBox* resampleCheck;
	QCheckBox* gammaCorrection;
	QComboBox* resampleBox;

	float exifDpi;
	QVector<float> unitFactor;
	QVector<float> resFactor;

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
};

class DkShortcutDelegate : public QItemDelegate {
	Q_OBJECT

public:
	DkShortcutDelegate(QObject* parent = 0);

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;

signals:
	void checkDuplicateSignal(QString text, void* item);
	void checkDuplicateSignal(const QKeySequence& keySequence, void* item);
	void clearDuplicateSignal() const;

protected slots:
	void textChanged(QString text = QString());
	void keySequenceChanged(const QKeySequence& keySequence);

protected:
	bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index);
	void setEditorData(QWidget* editor, const QModelIndex& index) const;
	void* item;
	//virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;

};

class DkShortcutEditor : public QLineEdit {
	Q_OBJECT
	Q_PROPERTY(QKeySequence shortcut READ shortcut WRITE setShortcut)

public:
	DkShortcutEditor(QWidget *widget = 0);
	QKeySequence shortcut() const;
	void setShortcut(const QKeySequence shortcut);

protected:
	//void keyPressEvent(QKeyEvent *event);
	//void keyReleaseEvent(QKeyEvent* event);
	bool eventFilter(QObject *obj, QEvent *event);

	QKeySequence ks;

};

class DkShortcutsModel : public QAbstractItemModel {
	Q_OBJECT

public:
	DkShortcutsModel(QObject* parent = 0);
	~DkShortcutsModel();
	//DkShortcutsModel(QVector<QPair<QString, QKeySequence> > actions, QObject *parent = 0);

	QModelIndex index(int row, int column, const QModelIndex &parent) const;
	QModelIndex parent(const QModelIndex &index) const;

	// return item of the model
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	virtual Qt::ItemFlags flags(const QModelIndex& index) const;
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

	void addDataActions(QVector<QAction*> actions, QString name);

	void resetActions();
	void saveActions();

public slots:
	void checkDuplicate(QString text, void* item);		// deprecated (Qt4)
	void checkDuplicate(const QKeySequence& ks, void* item);
	void clearDuplicateInfo() const;

signals:
	void duplicateSignal(QString info) const;

protected:

	void setupModelData(const QVector<QAction*> actions, TreeItem* parent = 0);

	TreeItem* rootItem;
	QVector<QVector<QAction*> > actions;

};

class DkShortcutsDialog : public QDialog {
	Q_OBJECT

public:
	DkShortcutsDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	void addActions(const QVector<QAction*>& actions, const QString& name);

public slots:
	void accept();

protected slots:
	void contextMenu(const QPoint& cur);
	void defaultButtonClicked();

protected:
	void createLayout();
	
	QVector<QAction*> actions;
	QTreeView* treeView;
	QTableView* actionTable;
	DkShortcutsModel* model;
	QLabel* notificationLabel;
	QPushButton* defaultButton;

};

class DkTextDialog : public QDialog {
	Q_OBJECT

public:
	DkTextDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	void setText(const QStringList& text);
	QTextEdit* getTextEdit() { return textEdit; };

public slots:
	virtual void save();

protected:
	void createLayout();

	QTextEdit* textEdit;
};

class DkUpdateDialog : public QDialog {
	Q_OBJECT

public:
	DkUpdateDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	QLabel* upperLabel;

signals:
	void startUpdate();

protected slots:
	void okButtonClicked();

protected:
	void init();
	void createLayout();

		
	QPushButton* okButton;
	QPushButton* cancelButton;
};


class DkPrintPreviewWidget : public QPrintPreviewWidget {
	Q_OBJECT
public:
	DkPrintPreviewWidget(QPrinter* printer, QWidget* parent = 0, Qt::WindowFlags flags = 0);

signals:
	void zoomChanged();

//public slots:
	//virtual void paintEvent(QPaintEvent* event);

protected:
	virtual void wheelEvent(QWheelEvent *event);		


};

class DkPrintPreviewValidator : public QDoubleValidator {
	
public:
	DkPrintPreviewValidator(QString suffix, qreal bottom, qreal top, int decimals, QObject* parent) : QDoubleValidator(bottom, top, decimals, parent) { this->suffix = suffix;};
	State validate(QString &input, int &pos) const {
		bool replaceSuffix = false;
		if (input.endsWith(suffix)) {
			input = input.left(input.length() - suffix.length());
			replaceSuffix = true;
		}
		State state = QDoubleValidator::validate(input, pos);
		if (replaceSuffix)
			input += suffix;
		const int num_size = 4+suffix.length();
		if (state == Intermediate) {
			int i = input.indexOf(QLocale::system().decimalPoint());
			if ((i == -1 && input.size() > num_size)
				|| (i != -1 && i > num_size))
				return Invalid;
		}
			
		return state;			
	}
private:
	QString suffix;
};



class DkPrintPreviewDialog : public QMainWindow {
	Q_OBJECT

public:
	enum icons{
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

	DkPrintPreviewDialog(QImage img, float dpi, QPrinter* printer = 0, QWidget* parent = 0, Qt::WindowFlags flags = 0);
	void setImage(QImage img, float dpi);
	void init();

public slots:
	void updateZoomFactor();

protected:
	void setup_Actions();
	void createLayout();
	void setIcon(QAction* action, const QLatin1String &name);
	void createIcons();

private slots:
	void paintRequested(QPrinter* printer);
	void fitImage(QAction* action);
	void zoomIn();
	void zoomOut();
	void zoomFactorChanged();
	void dpiFactorChanged();
	void updateDpiFactor(qreal dpi);
	void resetDpi();
	void pageSetup();
	void print();
	void centerImage();

private:
	void setFitting(bool on);
	void scaleImage();
	bool isFitting();
		
	QImage img;

	QActionGroup* fitGroup;
	QAction *fitWidthAction;
	QAction *fitPageAction;

	QActionGroup* zoomGroup;
	QAction *zoomInAction;
	QAction *zoomOutAction;

	QActionGroup* orientationGroup;
	QAction *portraitAction;
	QAction *landscapeAction;

	QActionGroup *printerGroup;
	QAction *printAction;
	QAction *pageSetupAction;

	QActionGroup *dpiGroup;
	QAction *resetDpiAction;

	QComboBox *zoomFactor;
	QComboBox *dpiFactor;
	QString dpiEditorSuffix;


	DkPrintPreviewWidget* preview;
	QPrinter* printer;
	QPrintDialog* printDialog;

	QVector<QIcon> icons;

	QTransform imgTransform;

	float dpi, origdpi;
	bool initialPaint;
};

class DkOpacityDialog : public QDialog {
	Q_OBJECT

public:
	DkOpacityDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
	int value() const;

protected:
	void createLayout();

	DkSlider* slider;
};

class DkExportTiffDialog : public QDialog {
	Q_OBJECT

public:
	DkExportTiffDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);

public slots:
	void on_openButton_pressed();
	void on_saveButton_pressed();
	void on_fileEdit_textChanged(const QString& filename);
	void setFile(const QString& filePath);
	void accept();
	void reject();
	int exportImages(const QString& filePath, const QString& saveFilePath, int from, int to, bool overwrite);
	void processingFinished();

signals:
	void updateImage(QImage img);
	void updateProgress(int);
	void infoMessage(QString msg);

protected:
	void createLayout();
	void enableTIFFSave(bool enable);
	void enableAll(bool enable);
	void dropEvent(QDropEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);

	DkBaseViewPort* viewport;
	QLabel* tiffLabel;
	QLabel* folderLabel;
	QLineEdit* fileEdit;
	QComboBox* suffixBox;
	QSpinBox* fromPage;
	QSpinBox* toPage;
	QDialogButtonBox* buttons;
	QProgressBar* progress;
	QLabel* msgLabel;
	QWidget* controlWidget;
	QCheckBox* overwrite;

	QString mFilePath;
	QDir saveDir;
	DkBasicLoader loader;
	QFutureWatcher<int> watcher;

	bool processing;

	enum {
		finished,
		question_save,
		error,

	};
};
#ifdef WITH_OPENCV
class DkUnsharpDialog : public QDialog {
	Q_OBJECT

public:
	DkUnsharpDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
	QImage getImage();

public slots:
	void on_sigmaSlider_valueChanged(int i);
	void on_amountSlider_valueChanged(int i);
	void setFile(const QString& filePath);
	void setImage(const QImage& img);
	void computePreview();
	void reject();
	QImage computeUnsharp(const QImage img, int sigma, int amount);
	void unsharpFinished();

signals:
	void updateImage(QImage img);

protected:
	void createLayout();
	void dropEvent(QDropEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);

	DkBaseViewPort* viewport;
	QLabel* preview;
	QDialogButtonBox* buttons;
	QFutureWatcher<QImage> unsharpWatcher;

	DkSlider* sigmaSlider;
	DkSlider* amountSlider;

	bool processing;
	QImage img;
};

class DkTinyPlanetDialog : public QDialog {
	Q_OBJECT

public:
	DkTinyPlanetDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
	QImage getImage();

public slots:
	void on_scaleSlider_valueChanged(int i);
	void on_scaleLogSlider_valueChanged(int i);
	void on_angleSlider_valueChanged(int i);
	void on_invertBox_toggled(bool t);
	void setFile(const QString& filePath);
	void setImage(const QImage& img);
	void computePreview();
	void updateImageSlot(QImage);
	void reject();
	QImage computeTinyPlanet(const QImage img, float scaleLog, float scale, double angle, QSize s);
	void tinyPlanetFinished();

signals:
	void updateImage(QImage img);

protected:
	void createLayout();
	void dropEvent(QDropEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);
	void resizeEvent(QResizeEvent *event);

	QLabel* imgPreview;
	QLabel* preview;
	QDialogButtonBox* buttons;
	QFutureWatcher<QImage> unsharpWatcher;

	DkSlider* scaleSlider;
	DkSlider* scaleLogSlider;
	DkSlider* angleSlider;
	QCheckBox* invertBox;

	bool processing;
	QImage img;
};

class DkMosaicDialog : public QDialog {
	Q_OBJECT

public:
	DkMosaicDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
	QImage getImage();

public slots:
	void on_openButton_pressed();
	void on_dbButton_pressed();
	void on_fileEdit_textChanged(const QString& filename);
	void on_newWidthBox_valueChanged(int i);
	void on_newHeightBox_valueChanged(int i);
	void on_numPatchesV_valueChanged(int i);
	void on_numPatchesH_valueChanged(int i);
	void on_darkenSlider_valueChanged(int i);
	void on_lightenSlider_valueChanged(int i);
	void on_saturationSlider_valueChanged(int i);
	
	void setFile(const QString& file);
	void compute();
	void reject();
	int computeMosaic(const QString& filePath, const QString& filter, const QString& suffix, int from, int to);		// TODO: make const!
	void mosaicFinished();
	void postProcessFinished();
	void buttonClicked(QAbstractButton* button);
	void updatePatchRes();

signals:
	void updateImage(QImage img) const;
	void updateProgress(int) const;
	void infoMessage(QString msg) const;

protected:
	void updatePostProcess();
	bool postProcessMosaic(float multiply = 0.3f, float screen = 0.5f, float saturation = 0.5f, bool computePreview = true);
	void createLayout();
	void enableMosaicSave(bool enable);
	void enableAll(bool enable);
	void dropEvent(QDropEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);
	QString getRandomImagePath(const QString& cPath, const QString& ignore, const QString& suffix);
	void matchPatch(const cv::Mat& img, const cv::Mat& thumb, int patchRes, cv::Mat& cc);
	cv::Mat createPatch(const DkThumbNail& thumb, int patchRes);
	
	DkBaseViewPort* viewport;
	DkBaseViewPort* preview;
	QLabel* fileLabel;
	QLabel* folderLabel;
	QLineEdit* filterEdit;
	QComboBox* suffixBox;
	QSpinBox* newWidthBox;
	QSpinBox* newHeightBox;
	QSpinBox* numPatchesV;
	QSpinBox* numPatchesH;
	QDialogButtonBox* buttons;
	QProgressBar* progress;
	QLabel* msgLabel;
	QWidget* controlWidget;
	QCheckBox* overwrite;
	QLabel* realResLabel;
	QLabel* patchResLabel;
	
	QWidget* sliderWidget;
	QSlider* darkenSlider;
	QSlider* lightenSlider;
	QSlider* saturationSlider;

	QString mFilePath;
	QDir saveDir;
	DkBasicLoader loader;
	QFutureWatcher<int> mosaicWatcher;
	QFutureWatcher<bool> postProcessWatcher;

	bool updatePostProcessing;
	bool postProcessing;
	bool processing;
	cv::Mat origImg;
	cv::Mat mosaicMat;
	cv::Mat mosaicMatSmall;
	QImage mosaic;
	QVector<QFileInfo> filesUsed;


	enum {
		finished,
		question_save,
		error,

	};
};
#endif

class DkForceThumbDialog : public QDialog {
	Q_OBJECT

public:
	DkForceThumbDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);

	bool forceSave() const;
	void setDir(const QDir& fileInfo);

protected:
	void createLayout();

	QLabel* infoLabel;
	QCheckBox* cbForceSave;
};

class DkWelcomeDialog : public QDialog {
	Q_OBJECT

public:
	DkWelcomeDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);

	bool isLanguageChanged();

public slots:
	virtual void accept();

protected:
	void createLayout();
	
	QComboBox* languageCombo;
	QCheckBox* registerFilesCheckBox;
	QStringList languages;
	bool languageChanged;
};

#ifdef WITH_QUAZIP
class DkArchiveExtractionDialog : public QDialog {
	Q_OBJECT
	
public:
	DkArchiveExtractionDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	void setCurrentFile(const QFileInfo& file, bool isZip);

public slots:
	void textChanged(QString text);
	void checkbocChecked(int state);
	void dirTextChanged(QString text);
	void loadArchive(QString filePath = "");
	void openArchive();
	void openDir();
	void accept();

protected:
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);

	void createLayout();
	void userFeedback(const QString& msg, bool error = false);
	QStringList extractFilesWithProgress(QString fileCompressed, QStringList files, QString dir, bool removeSubfolders);

	DkFileValidator fileValidator;
	QDialogButtonBox* buttons;
	QLineEdit* archivePathEdit;
	QLineEdit* dirPathEdit;
	QListWidget* fileListDisplay;
	QLabel* feedbackLabel;
	QCheckBox* removeSubfolders;

	QStringList fileList;
	QFileInfo cFile;
};
#endif

}
