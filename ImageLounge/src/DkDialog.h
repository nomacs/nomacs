/*******************************************************************************************************
 DkImage.h
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

#include <QDialog>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>
#include <QSlider>
#include <QColorDialog>
#include <QPushButton>
#include <QBoxLayout>
#include <QCheckBox>
#include <QFileInfo>
#include <QTableView>
#include <QCompleter>
#include <QMainWindow>
#include <QDialogButtonBox>
#include <QStandardItemModel>
#include <QItemDelegate>
#include <QItemEditorFactory>
#include <QHeaderView>
#include <QTreeView>

#include <QPrintPreviewWidget>
#include <QPageSetupDialog>
#include <QPrintDialog>
#include <QToolBar>
#include <QFormLayout>
#include <QProgressBar>
#include <QFuture>
#include <QtConcurrentRun>
#include <QFutureWatcher>

#include "DkWidgets.h"
#include "DkViewPort.h"
#include "DkThumbs.h"

namespace nmc {

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

class DkMessageBox : public QDialog {
	Q_OBJECT

public:
	DkMessageBox(QMessageBox::Icon icon, 
		const QString& title, 
		const QString& text, 
		QMessageBox::StandardButtons buttons = QMessageBox::NoButton,
		QWidget* parent = 0, 
		Qt::WindowFlags f = Qt::Dialog);
	DkMessageBox(QWidget* parent = 0);

	~DkMessageBox();

	virtual void setVisible(bool visible);

public slots:
	void buttonClicked(QAbstractButton* button);
	int exec();

protected:
	
	QLabel* iconLabel;
	QLabel* textLabel;
	QMessageBox::Icon icon;
	QDialogButtonBox* buttonBox;
	QCheckBox* showAgain;

	void createLayout(const QMessageBox::Icon& userIcon, const QString& userText, QMessageBox::StandardButtons buttons);
	void updateSize();
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
	DkSplashScreen(QWidget* parent = 0, Qt::WFlags flags = 0);
	~DkSplashScreen() {};

	//protected:
	//	void mousePressEvent(QMouseEvent *event);

private:
	QString text;
	QLabel* textLabel;
	QLabel* imgLabel;

};

class DkFileValidator : public QValidator {
	Q_OBJECT

public:
	DkFileValidator(QString lastFile = "", QObject * parent = 0);

	void setLastFile(QString lastFile) {
		this->lastFile = lastFile;
	};
	virtual void fixup(QString& input) const;
	virtual State validate(QString& input, int& pos) const;

protected:
	QString lastFile;
};

class DkTrainDialog : public QDialog {
	Q_OBJECT
	
public:
	DkTrainDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	QFileInfo getAcceptedFile() {
		return acceptedFile;
	};

	void setCurrentFile(const QFileInfo& file) {
		cFile = file;
	};

public slots:
	void textChanged(QString text);
	void loadFile(QString filePath = "");
	void openFile();
	void accept();

protected:
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);

	void createLayout();
	void userFeedback(const QString& msg, bool error = false);

	DkFileValidator fileValidator;
	QDialogButtonBox* buttons;
	QLineEdit* pathEdit;
	QLabel* feedbackLabel;
	DkBaseViewPort* viewport;
	
	QFileInfo acceptedFile;
	QFileInfo cFile;
};

class DkAppManager : public QObject{
	Q_OBJECT

public:
	DkAppManager(QWidget* parent = 0);
	~DkAppManager();

	void setActions(QVector<QAction* > actions);
	QVector<QAction* >& getActions();
	QAction* createAction(QString filePath);
	QAction* findAction(QString appPath);

	enum defaultAppIdx {

		app_photohsop,
		app_picasa,
		app_picasa_viewer,
		app_irfan_view,
		app_explorer,

		app_idx_end
	};

public slots:
	void openTriggered();

signals:
	void openFileSignal(QAction* action);

protected:
	void saveSettings();
	void loadSettings();
	void assignIcon(QAction* app);
	bool containsApp(QVector<QAction* > apps, QString appName);

	QString searchForSoftware(QString organization, QString application, QString pathKey = "", QString exeName = "");
	void findDefaultSoftware();

	QVector<QString> defaultNames;
	QVector<QAction* > apps;
	QWidget* parent;
};

class DkAppManagerDialog : public QDialog {
	Q_OBJECT

public:
	DkAppManagerDialog(DkAppManager* manager = 0, QWidget* parent = 0, Qt::WindowFlags flags = 0);

public slots:
	void on_addButton_clicked();
	void on_deleteButton_clicked();
	virtual void accept();

protected:
	DkAppManager* manager;
	QStandardItemModel* model;

	void createLayout();
	QList<QStandardItem* > getItems(QAction* action);
	QTableView* appTableView;
};

class DkChangeTranslationDialog : public QDialog {
	Q_OBJECT

public:
	DkChangeTranslationDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);

public slots:
	virtual void accept();

protected:
	QStandardItemModel* model;
	QTableView* translationTableView;

	QStringList origStrings;
	QStringList activeStrings;

	void createLayout();
	QList<QStandardItem* > getItems(int idx);
	
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

	void setFiles(QStringList fileList) {
		this->fileList = fileList;
		this->resultList = fileList;
		stringModel->setStringList(makeViewable(fileList));
	};

	void setPath(QDir path) {
		this->path = path;
	};

	bool filterPressed() {
		return isFilterPressed;
	};

	void setDefaultButton(int defaultButton = find_button);

public slots:
	void on_searchBar_textChanged(const QString& text);
	void on_okButton_pressed();
	void on_filterButton_pressed();
	void on_cancelButton_pressed();
	void on_resultListView_doubleClicked(const QModelIndex& modelIndex);
	void on_resultListView_clicked(const QModelIndex& modelIndex);

signals:
	void loadFileSignal(QFileInfo file);
	void filterSignal(QStringList);

protected:

	void updateHistory();
	void init();
	QStringList makeViewable(const QStringList& resultList, bool forceAll = false);

	QStringListModel* stringModel;
	QListView* resultListView;
	QLineEdit* searchBar;

	QVector<QPushButton*> buttons;

	QString currentSearch;

	QDir path;
	QStringList fileList;
	QStringList resultList;

	QString defaultStyleSheet;
	QString endMessage;

	bool allDisplayed;
	bool isFilterPressed;
};

class DkResizeDialog : public QDialog {
	Q_OBJECT

public:
	DkResizeDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~DkResizeDialog();

	enum{ipl_nearest, ipl_area, ipl_linear, ipl_cubic, ipl_lanczos, ipl_end};
	enum{size_pixel, size_percent, size_end};
	enum{unit_cm, unit_mm, unit_inch, unit_end};
	enum{res_ppi, res_ppc, res_end};  

	void setImage(QImage img) {
		this->img = img;
		initBoxes();
		updateSnippets();
		drawPreview();
		wPixelEdit->selectAll();
	};

	QImage getResizedImage() {

		return resizeImg(img, false);
	};

	void setExifDpi(float exifDpi) {

		//if (exifDpi < 1)
		//	return;

		this->exifDpi = exifDpi;
		resolutionEdit->setValue(exifDpi);
	};

	float getExifDpi() {
		return exifDpi;
	};

	bool resample() {
		return resampleCheck->isChecked();
	};

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

	void drawPreview();

	void setVisible(bool visible) {
		updateSnippets();
		drawPreview();

		QDialog::setVisible(visible);
	}

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
	QComboBox* resampleBox;

	float exifDpi;
	QVector<float> unitFactor;
	QVector<float> resFactor;

	void init();
	void initBoxes();
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

protected slots:
	void textChanged(QString text = QString());

protected:
	bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index);
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

// from: http://qt-project.org/doc/qt-4.8/itemviews-simpletreemodel.html
class TreeItem {

public:
	TreeItem(const QVector<QVariant> &data, TreeItem *parent = 0);
	~TreeItem();

	void appendChild(TreeItem *child);

	TreeItem *child(int row);
	int childCount() const;
	int columnCount() const;
	QVariant data(int column) const;
	void setData(const QVariant& value, int column);
	int row() const;
	TreeItem* parent() const;
	TreeItem* find(const QVariant& value, int column);
	void setParent(TreeItem* parent);


private:
	QVector<TreeItem*> childItems;
	QVector<QVariant> itemData;
	TreeItem *parentItem;
};



class DkShortcutsModel : public QAbstractTableModel {
	Q_OBJECT

public:
	DkShortcutsModel(QObject* parent = 0);
	~DkShortcutsModel();
	//DkShortcutsModel(QVector<QPair<QString, QKeySequence> > actions, QObject *parent = 0);

	QModelIndex index(int row, int column, const QModelIndex &parent) const;
	QModelIndex parent(const QModelIndex &index) const;

	// return item of the model
	//virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
	//virtual QModelIndex parent(const QModelIndex& index) const;
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	virtual Qt::ItemFlags flags(const QModelIndex& index) const;
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

	//QVector<QVector<QPair<QString, QKeySequence> > >* getActions() {
	//	return &actions;
	//}

	void addDataActions(QVector<QAction*> actions, QString name);

	void saveActions();

public slots:
	void checkDuplicate(QString text, void* item);

signals:
	void duplicateSignal(QString info);

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

protected:
	void createLayout();
	
	QVector<QAction*> actions;
	QTreeView* treeView;
	QTableView* actionTable;
	DkShortcutsModel* model;
	QLabel* notificationLabel;


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
	bool isFitting() {
		return (fitGroup->isExclusive() && (fitWidthAction->isChecked() || fitPageAction->isChecked()));
	};
		

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

	int value() {
		return slider->value();
	};

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
	void setFile(const QFileInfo& file);
	void accept();
	void reject();
	int exportImages(QFileInfo file, QFileInfo saveFile, int from, int to, bool overwrite);
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

	QFileInfo cFile;
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
	void setFile(const QFileInfo& file);
	void compute();
	void reject();
	int computeMosaic(QFileInfo file, QString filter, QString suffix, int from, int to);
	void mosaicFinished();
	void postProcessFinished();
	void buttonClicked(QAbstractButton* button);
	void updatePatchRes();

signals:
	void updateImage(QImage img);
	void updateProgress(int);
	void infoMessage(QString msg);

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

	QFileInfo cFile;
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

	bool forceSave() {

		return cbForceSave->isChecked();
	}

	void setDir(const QDir& fileInfo);

protected:
	void createLayout();

	QLabel* infoLabel;
	QCheckBox* cbForceSave;
};


}
