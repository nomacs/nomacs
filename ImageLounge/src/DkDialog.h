/*******************************************************************************************************
 DkImage.h
 Created on:	21.04.2011
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2012 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2012 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2012 Florian Kleber <florian@nomacs.org>

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

#include <QPrintPreviewWidget>
#include <QPageSetupDialog>
#include <QPrintDialog>
#include <QToolBar>
#include <QFormLayout>

#include "DkWidgets.h"


namespace nmc {

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

class DkTifDialog : public QDialog {
	Q_OBJECT

public:
	DkTifDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	int getCompression() {

		return (noCompressionButton->isChecked()) ? 0 : 1;
	};

	bool wasOkPressed() {
		return isOk;
	};

	// TODO: make it a bit more stylish

	public slots:
		void okPressed();
		void cancelPressed();


protected:
	void init();
	QRadioButton* noCompressionButton;
	QRadioButton* compressionButton;
	bool isOk;

};

class DkJpgDialog : public QDialog {
	Q_OBJECT

public:
	DkJpgDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	bool wasOkPressed() {
		return isOk;
	};
	void imageHasAlpha(bool hasAlpha) {
		this->hasAlpha = hasAlpha;
		colButton->setEnabled(hasAlpha);
		colLabel->setEnabled(hasAlpha);
	};

	QColor getBackgroundColor() {
		return bgCol;
	};

	int getCompression() {
		return slider->value();
	};

	void setImage(QImage* img) {
		this->img = img;
		updateSnippets();
		drawPreview();
	};

	protected slots:
		void okPressed();
		void cancelPressed();
		void openColorDialog() {
			colorDialog->show();
		};
		void newBgCol() {
			bgCol = colorDialog->currentColor();
			colButton->setStyleSheet("QPushButton {background-color: "+ bgCol.name()+";border:1px; min-height:24px;}");
			drawPreview();
		};

		void updateSliderLabel(int val) {
			sliderValueLabel->setValue(val);
			drawPreview();
		};

		void updateSliderValue(int val) {
			slider->setValue(val);
			drawPreview();
		};

protected:
	bool isOk;
	bool hasAlpha;
	QColor bgCol;
	int leftSpacing;
	int margin;
	QLabel* colLabel;
	QSpinBox* sliderValueLabel;
	QSlider* slider;
	QColorDialog* colorDialog;
	QPushButton* colButton;
	QImage* img;
	QImage origImg;
	QImage newImg;
	QWidget* centralWidget;
	QLabel* previewLabel;


	void init();
	void createLayout();
	void showEvent(QShowEvent *event);
	void drawPreview();
	void updateSnippets();
};

class DkOpenWithDialog : public QDialog {
	Q_OBJECT


		enum {
			app_photoshop,
			app_irfan_view,
			app_picasa,
			app_end,
	};

public:
	DkOpenWithDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	bool wasOkClicked() {
		return userClickedOk;
	};


protected slots:
	void softwareSelectionChanged();
	void okClicked();
	void cancelClicked();
	void browseAppFile();
	void softwareCleanClicked();

protected:

	// input
	QStringList organizations;
	QStringList applications;
	QStringList pathKeys;
	QStringList exeNames;
	QStringList screenNames;

	QList<QPixmap> appIcons;
	QList<QRadioButton*> userRadios;
	QList<QPushButton*> userCleanButtons;
	QList<QLabel*> userCleanSpace;
	QButtonGroup* userRadiosGroup;
	QStringList userAppPaths;
	QStringList appPaths;

	QBoxLayout* layout;
	QCheckBox* neverAgainBox;

	// output
	int numDefaultApps;
	int defaultApp;
	bool userClickedOk;

	// functions
	void init();
	void createLayout();
	QString searchForSoftware(int softwareIdx);
	QPixmap getIcon(QFileInfo path);

	QString getPath() {

		qDebug() << "app idx: " << defaultApp;

		if (defaultApp < numDefaultApps) {
			qDebug() << "default path...";
			return appPaths[defaultApp];
		}
		else {
			qDebug() << "user app path";
			return userAppPaths[defaultApp-numDefaultApps];
		}
	};

};

class DkSearchDialog : public QDialog {
	Q_OBJECT

public:

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

	QPushButton* findButton;
	QPushButton* filterButton;
	QPushButton* cancelButton;

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

	enum{ipl_nearest, ipl_area, ipl_linear, ipl_cubic, ipl_lanczos, ipl_end};
	enum{size_pixel, size_percent, size_end};
	enum{unit_cm, unit_mm, unit_inch, unit_end};
	enum{res_ppi, res_ppc, res_end};  

	bool wasOkPressed() {
		return isOk;
	};

	void setImage(QImage img) {
		this->img = img;
		initBoxes();
		updateSnippets();
		drawPreview();
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
		void okPressed();
		void cancelPressed();

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

protected:
	bool isOk;
	int leftSpacing;
	int margin;
	QImage img;
	QImage origImg;
	QImage newImg;
	QWidget* centralWidget;
	QLabel* previewLabel;

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
	void showEvent(QShowEvent *event);
	void drawPreview();
	void updateSnippets();
	void updateHeight();
	void updateWidth();
	void updatePixelWidth();
	void updatePixelHeight();
	void updateResolution();
	QImage resizeImg(QImage img, bool silent = true);
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

	public slots:
		virtual void paintEvent(QPaintEvent* event);


};

class DkZoomValidator : public QDoubleValidator {
	public:
		DkZoomValidator(qreal bottom, qreal top, int decimals, QObject* parent) : QDoubleValidator(bottom, top, decimals, parent) {};
		State validate(QString &input, int &pos) const {
			bool replacePercent = false;
			if (input.endsWith(QLatin1Char('%'))) {
				input = input.left(input.length() - 1);
				replacePercent = true;
			}
			State state = QDoubleValidator::validate(input, pos);
			if (replacePercent)
				input += QLatin1Char('%');
			const int num_size = 4;
			if (state == Intermediate) {
				int i = input.indexOf(QLocale::system().decimalPoint());
				if ((i == -1 && input.size() > num_size)
					|| (i != -1 && i > num_size))
					return Invalid;
			}
			return state;			
		}
};



class DkPrintPreviewDialog : public QMainWindow {
	Q_OBJECT

	public:
		DkPrintPreviewDialog(QImage img, float dpi, QPrinter* printer = 0, QWidget* parent = 0, Qt::WindowFlags flags = 0);

		void init();

	protected:
		void setup_Actions();
		void createLayout();
		void setIcon(QAction* action, const QLatin1String &name);

	private slots:
		void paintRequested(QPrinter* printer);
		void fitImage(QAction* action);
		void zoomIn();
		void zoomOut();
		void zoomFactorChanged();
		void updateZoomFactor();
		void pageSetup();
		void print();

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

		QComboBox *zoomFactor;
		QComboBox *dpiFactor;


		DkPrintPreviewWidget* preview;
		QPrinter* printer;
		QPrintDialog* printDialog;

		float dpi;
};

}
