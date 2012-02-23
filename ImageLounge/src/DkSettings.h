/*******************************************************************************************************
 DkSettings.h
 Created on:	07.07.2011
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011 Florian Kleber <florian@nomacs.org>

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
#include <QWidget>
#include <QListView>
#include <QLabel>
#include <QStringList>
#include <QStringListModel>
#include <QSpacerItem>
#include <QPushButton>
#include <QCheckBox>
#include <QDebug>
#include <QSpinBox>
#include <QGroupBox>
#include <QSettings>
#include <QStyle>
#include <QBitArray>
#include <QColorDialog>
#include <QKeyEvent>
#include <QModelIndex>
#include <QStringBuilder>
#include <QDate>
#include <QButtonGroup>
#include <QRadioButton>
#include <QFileDialog>
#include <QLineEdit>

#include "BorderLayout.h"

namespace nmc {

class DkSettingsWidget;
class DkGlobalSettingsWidget;
class DkDisplaySettingsWidget;
class DkSlideshowSettingsWidget;
class DkSynchronizeSettingsWidget;
class DkMetaDataSettingsWidget;
class DkSettingsListView;
class DkSpinBoxWidget;

class DkSettings : public QObject {
	Q_OBJECT

	public:

		enum modes {
			mode_default = 0,
			mode_frameless,
			mode_contrast,
			mode_end,
		};

		DkSettings() {};
		DkSettings(const DkSettings& settings) {}; 

		struct AppSettings {
			static bool showToolBar;
			static bool showMenuBar;
			static bool showStatusBar;
			static int appMode;
		};

		struct DisplaySettings {
			static bool keepZoom;
			static bool invertZoom;
			static QColor highlightColor;
			static int thumbSize;
			static bool saveThumb;
			static int interpolateZoomLevel;
		};

		struct GlobalSettings {
			static int skipImgs;
			static bool showOverview;
			static bool showInfo;
			static bool loop;

			static QString lastDir;
			static QString lastSaveDir;
			static QStringList recentFiles;
			static QStringList recentFolders;
			static bool useTmpPath;
			static QString tmpPath;

			// open with
			static QString defaultAppPath;
			static int defaultAppIdx;
			static bool showDefaultAppDialog;
			static int numUserChoices;
			static QStringList userAppPaths;
		};
		struct SlideShowSettings {
			static int filter;
			static int time;
			static bool silentFullscreen;
			static QBitArray display;
			static QColor backgroundColor;
		};
		struct SynchronizeSettings {
			static bool enableNetworkSync;
			static bool allowTransformation;
			static bool allowPosition;
			static bool allowFile;
			static bool allowImage;
			static bool updateDialogShown;
			static QDate lastUpdateCheck;
			static bool syncAbsoluteTransform;
		};
		struct MetaDataSettings {
			static QBitArray metaDataBits;

			//static bool exifSize;
			//static bool exifOrientation;
			//static bool exifMake;
			//static bool exifModel;
			//static bool exifRate;
			//static bool exifUserComment;
			//static bool exifDate;
			//static bool exifAperture;
			//static bool exifShutterSpeed;
			//static bool exifFlash;
			//static bool exifFocalLength;
			//static bool exifExposureMode;

			//static bool exifExposureTime;
			//static bool exifDateTimeOriginal;
			//static bool exifImageDescription;

			//static bool iptcCreator;
			//static bool iptcCreatorTitle;
			//static bool iptcCity;
			//static bool iptcCountry;
			//static bool iptcHeadline;
			//static bool iptcCaption;
			//static bool iptcCopyRight;
			//static bool iptcKeywords;
		};

		void load();
		void save();
		void setToDefaultSettings();

signals:
		void setToDefaultSettingsSignal();

};

class DkSettingsDialog : public QDialog {
	Q_OBJECT;

	public:
		DkSettingsDialog(QWidget* parent);
		DkSettingsDialog(const DkSettingsDialog& dialog) {
			this->borderLayout = dialog.borderLayout;
			this->listView = dialog.listView;
			this->rightWidget = dialog.rightWidget;
			this->leftLabel = dialog.leftLabel;
			this->buttonOk = dialog.buttonOk;
			this->buttonCancel = dialog.buttonCancel;
			this->widgetList = dialog.widgetList;
			this->centralWidget = dialog.centralWidget;
			this->centralLayout = dialog.centralLayout;
			this->globalSettingsWidget = dialog.globalSettingsWidget;
			this->slideshowSettingsWidget = dialog.slideshowSettingsWidget;
			this->synchronizeSettingsWidget = dialog.synchronizeSettingsWidget;
		}
		~DkSettingsDialog();

	signals:
		void settingsChanged();
		void setToDefaultSignal();

	private:
		void init();
		void createLayout();

	private slots:
		void listViewSelected(const QModelIndex & qmodel);
		void saveSettings();
		void cancelPressed() { close(); };
		void initWidgets();
		void setToDefault() {
			
			if (s)
				s->setToDefaultSettings();

			// for main window
			emit setToDefaultSignal();
			emit settingsChanged();
		};

	protected:
		BorderLayout* borderLayout;
		DkSettingsListView* listView;
		QWidget* rightWidget;
		QLabel* leftLabel;
		QPushButton* buttonOk;
		QPushButton* buttonCancel;
		DkSettings* s;

		QList<DkSettingsWidget*> widgetList;
		QWidget* centralWidget;
		QHBoxLayout* centralLayout;
		DkGlobalSettingsWidget* globalSettingsWidget;
		DkDisplaySettingsWidget* displaySettingsWidget;
		DkSlideshowSettingsWidget* slideshowSettingsWidget;
		DkSynchronizeSettingsWidget* synchronizeSettingsWidget;
		DkMetaDataSettingsWidget* exifSettingsWidget;
};

class DkSettingsWidget : public QWidget {
Q_OBJECT	

public:
		DkSettingsWidget(QWidget* parent) : QWidget(parent) {};
		virtual void writeSettings() = 0;
		virtual void init() = 0;
};

class DkGlobalSettingsWidget : public DkSettingsWidget {
Q_OBJECT

	public:
		DkGlobalSettingsWidget(QWidget* parent);
	
		void writeSettings();

	signals:
		void applyDefault();
	private slots:
		void tmpPathButtonPressed();
		void useTmpPathChanged(int state);
		void lineEditChanged(QString path);
		void setToDefaultPressed() {

			qDebug() << "apply default pressed...";
			emit applyDefault();
		};

		void openWithDialog();

	private:
		void init();
		void createLayout();

		bool existsDirectory(QString path);

		
		DkSpinBoxWidget* skipImgWidget;
		QCheckBox* cbWrapImages;

		QString tmpPath;
		QLineEdit* leTmpPath;
		QCheckBox* cbUseTmpPath;
		QPushButton* pbTmpPath;
		
	

		QPushButton* buttonDefaultSettings;

		bool loop;

};


class DkDisplaySettingsWidget : public DkSettingsWidget {
	Q_OBJECT	

	public:
		DkDisplaySettingsWidget(QWidget* parent);

		void writeSettings();

	private slots:
		void highlightButtonClicked() {colorDialog->show();};
		void highlightDialogClosed() {setHighlightColor(colorDialog->currentColor());};
		void resetHighlightColor() {setHighlightColor(QColor(0, 204, 255));};


	private:
		void init();
		void createLayout();
		void setHighlightColor(QColor newColor);

		QCheckBox* cbKeepZoom;
		QCheckBox* cbInvertZoom;

		DkSpinBoxWidget* interpolateWidget;

		QCheckBox* cbShowMenu;
		QCheckBox* cbShowToolbar;
		QCheckBox* cbShowStatusbar;

		QColorDialog* colorDialog;
		QLabel* highlightColorLabel;
		QPushButton* highlightColorButton;
		QPushButton* highlightColorResetButton;

		DkSpinBoxWidget* maximalThumbSizeWidget; 
		QCheckBox* cbSaveThumb;

		bool keepZoom;
		bool invertZoom;

};


class DkSlideshowSettingsWidget : public DkSettingsWidget {
Q_OBJECT	

	public:
		enum DisplayItems{
			display_file_name,
			display_creation_date,
			display_file_rating,

			display_end
		};
		DkSlideshowSettingsWidget(QWidget* parent);

		void writeSettings();


	private slots:
		void backgroundButtonClicked();
		void backgroundDialogClosed();
		void resetBackground();
		void showFileName(bool checked);
		void showCreationDate(bool checked);
		void showRating(bool checked);

	private:
		void init();
		void createLayout();
		void setBackgroundColor(QColor newColor);

		QVBoxLayout* vBoxLayout;

		DkSpinBoxWidget* timeWidget;

		QLabel* labelBackgroundText;
		QPushButton* buttonBackgroundColor;
		QPushButton* buttonResetBackground;
		QColorDialog* colorDialog;

		QGroupBox* gbInfo;
		QCheckBox* cbCreationDate;
		QCheckBox* cbName;
		QCheckBox* cbRating;
		QCheckBox* cbSilentFullscreen;

		int filter;
		int time;

};

class DkSynchronizeSettingsWidget : public DkSettingsWidget {
	Q_OBJECT

	public:
		DkSynchronizeSettingsWidget(QWidget* parent);

		void writeSettings();

	private slots:
		void enableNetworkCheckBoxChanged(int state);

	private:
		void init();
		void createLayout();

		QVBoxLayout* vboxLayout;
		QRadioButton* rbSyncAbsoluteTransform;
		QRadioButton* rbSyncRelativeTransform;
		QCheckBox* cbEnableNetwork;
		QButtonGroup* buttonGroup;
		QCheckBox* cbAllowTransformation;
		QCheckBox* cbAllowPosition;
		QCheckBox* cbAllowImage;
		QCheckBox* cbAllowFile;};


class DkSettingsListView : public QListView {
Q_OBJECT;

	public:
		DkSettingsListView(QWidget* parent) : QListView(parent) {};
		~DkSettingsListView() {QItemSelectionModel* sm = this->selectionModel(); delete sm;};

	public slots:
		void keyPressEvent(QKeyEvent *event) {
			if (event->key() == Qt::Key_Up) {
				previousIndex(); 
				return;
			}
			else if (event->key() == Qt::Key_Down) {
				nextIndex(); 
				return;
			}
			QListView::keyPressEvent(event);
		};
	private:
		void previousIndex();
		void nextIndex();
};	


class DkMetaDataSettingsWidget : public DkSettingsWidget {
	Q_OBJECT

public:

	//enums for checkboxes - divide in camera data and description
	enum cameraData {
		camData_size,
		camData_orientation,
		camData_make,
		camData_model,
		camData_aperture,
		//camData_shutterspeed,
		camData_flash,
		camData_focallength,
		camData_exposuremode,
		camData_exposuretime,

		camData_end
	};

	enum descriptionT {
		desc_rating = camData_end,
		desc_usercomment,
		desc_date,
		desc_datetimeoriginal,
		desc_imagedescription,
		desc_creator,
		desc_creatortitle,
		desc_city,
		desc_country,
		desc_headline,
		desc_caption,
		desc_copyright,
		desc_keywords,
		desc_path,
		desc_filesize,
		
		desc_end
	};

	static QString scamDataDesc;
	static QString sdescriptionDesc;

	DkMetaDataSettingsWidget(QWidget* parent);

	void writeSettings();

private:
	void init();
	void createLayout();


	//Checkboxes
	QVector<QCheckBox *> pCbMetaData;

	//Tags not used, but maybe later...
	//Exif.Image.BitsPerSample
	//Exif.Image.ImageDescription
	//Exif.Image.XResolution, Exif.Image.YResolution
	//Exif.Image.ISOSpeedRatings
	//Exif.Image.BrightnessValue
	//Exif.Image.ExposureBiasValue
	//Exif.Image.MaxApertureValue
	//Exif.Image.LightSource
	//Exif.Image.Noise

	//IPTC
	//Iptc.Application2.DateCreated

	////XMP
	//CreateDate
	//CreatorTool
	//Identifier
	//Label
	//MetaDataDate
	//ModifyDate
	//Nickname
	//Rating
};


class DkSpinBoxWidget : public QWidget {
	Q_OBJECT;
	public:	
		DkSpinBoxWidget(QWidget* parent = 0);
		DkSpinBoxWidget(QString upperString, QString lowerString, int spinBoxMin, int spinBoxMax, QWidget* parent=0, int step = 1);
		QSpinBox* getSpinBox() { return spinBox;};
		void setSpinBoxValue(int value) {spinBox->setValue(value);};
		int getSpinBoxValue() {return spinBox->value();};

	private:
		QSpinBox* spinBox;
		QLabel* upperLabel;
		QLabel* lowerLabel;
		QWidget* lowerWidget;
		QVBoxLayout* vboxLayout;
		QHBoxLayout* hboxLowerLayout;
};

};