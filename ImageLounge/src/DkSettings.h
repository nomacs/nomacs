/*******************************************************************************************************
 DkSettings.h
 Created on:	07.07.2011
 
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
#include <QTranslator>
#include <QComboBox>


#include "BorderLayout.h"

#ifdef DK_DLL
#define DllExport Q_DECL_EXPORT
#else
#define DllExport Q_DECL_IMPORT
#endif

namespace nmc {

class DkSettingsWidget;
class DkGlobalSettingsWidget;
class DkDisplaySettingsWidget;
class DkFileWidget;
class DkSynchronizeSettingsWidget;
class DkMetaDataSettingsWidget;
class DkResourceSettingsWidgets;
class DkSettingsListView;
class DkSpinBoxWidget;
class DkDoubleSpinBoxWidget;

class DllExport DkSettings {

public:

	enum modes {
		mode_default = 0,
		mode_frameless,
		mode_contrast,
		mode_default_fullscreen,
		mode_frameless_fullscreen,
		mode_contrast_fullscreen,
		mode_end,
	};

	enum sortMode {
		sort_filename,
		sort_date_created,
		sort_date_modified,

		sort_end,
	};

	enum sortDir {
		sort_ascending,
		sort_descending,

		sort_dir_end,
	};

	struct App {
		bool showToolBar;
		bool showMenuBar;
		bool showStatusBar;
		QBitArray showFilePreview;
		QBitArray showFileInfoLabel;
		QBitArray showPlayer;
		QBitArray showMetaData;
		QBitArray showHistogram;
		QBitArray showOverview;
		QBitArray showScroller;
		int appMode;
		int currentAppMode;
		bool advancedSettings;
		bool closeOnEsc;
	};

	struct Display {
		bool keepZoom;
		bool invertZoom;
		bool tpPattern;
		QColor highlightColor;
		QColor bgColorWidget;
		QColor bgColor;
		QColor bgColorFrameless;
		QColor iconColor;
		bool useDefaultColor;
		bool defaultIconColor;
		int thumbSize;
		int thumbPreviewSize;
		bool saveThumb;
		int interpolateZoomLevel;
		bool antiAliasing;
		bool smallIcons;
		bool toolbarGradient;
		bool showBorder;
	};

	struct Global {
		int skipImgs;
		int numFiles;
		bool loop;
		bool scanSubFolders;

		QString lastDir;
		QString lastSaveDir;
		QStringList recentFiles;
		QStringList recentFolders;
		bool useTmpPath;
		QString tmpPath;
		QString language;
		QStringList searchHistory;
		
		Qt::KeyboardModifier altMod;
		Qt::KeyboardModifier ctrlMod;
		bool zoomOnWheel;

		QString setupPath;
		QString setupVersion;

		// open with
		QString defaultAppPath;
		int defaultAppIdx;
		bool showDefaultAppDialog;
		int numUserChoices;
		QStringList userAppPaths;
		int sortMode;
		int sortDir;
	};

	struct SlideShow {
		int filter;
		float time;
		bool silentFullscreen;
		QBitArray display;
		QColor backgroundColor;
	};
	struct Sync {
		bool enableNetworkSync;
		bool allowTransformation;
		bool allowPosition;
		bool allowFile;
		bool allowImage;
		bool updateDialogShown;
		QDate lastUpdateCheck;
		bool syncAbsoluteTransform;
		bool switchModifier;
	};
	struct MetaData {
		QBitArray metaDataBits;
		bool ignoreExifOrientation;
		bool saveExifOrientation;
	};
		
	struct Resources {
		float cacheMemory;
		bool fastThumbnailPreview;
		bool filterRawImages;
		bool filterDuplicats;
		QString preferredExtension;
	};

	static App& getAppSettings();
	static Display& getDisplaySettings();
	static Global& getGlobalSettings();
	static SlideShow& getSlideShowSettings();
	static Sync& getSyncSettings();
	static MetaData& getMetaDataSettings();
	static Resources& getResourceSettings();

	static void load(bool force = false);
	static void save(bool force = false);
	static void setToDefaultSettings();

	static App& app;
	static Global& global;
	static Display& display;
	static SlideShow& slideShow;
	static Sync& sync;
	static MetaData& metaData;
	static Resources& resources;

protected:
	static App app_p;
	static Global global_p;
	static Display display_p;
	static SlideShow slideShow_p;
	static Sync sync_p;
	static MetaData meta_p;
	static Resources resources_p;

	static App app_d;
	static Global global_d;
	static Display display_d;
	static SlideShow slideShow_d;
	static Sync sync_d;
	static MetaData meta_d;
	static Resources resources_d;
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
		void languageChanged();
		void settingsChanged();
		void setToDefaultSignal();

	private:
		void init();
		void createLayout();
		void createSettingsWidgets();

	private slots:
		void listViewSelected(const QModelIndex & qmodel);
		void saveSettings();
		void cancelPressed() { close(); };
		void initWidgets();
		void setToDefault() {
			
			DkSettings::setToDefaultSettings();
			initWidgets();

			// for main window
			emit setToDefaultSignal();
			emit settingsChanged();
		};
		void advancedSettingsChanged(int state);

	protected:
		BorderLayout* borderLayout;
		DkSettingsListView* listView;
		QWidget* rightWidget;
		QLabel* leftLabel;
		QPushButton* buttonOk;
		QPushButton* buttonCancel;
		QCheckBox* cbAdvancedSettings;

		QList<DkSettingsWidget*> widgetList;
		QWidget* centralWidget;
		QHBoxLayout* centralLayout;
		DkGlobalSettingsWidget* globalSettingsWidget;
		DkDisplaySettingsWidget* displaySettingsWidget;
		DkFileWidget* slideshowSettingsWidget;
		DkSynchronizeSettingsWidget* synchronizeSettingsWidget;
		DkMetaDataSettingsWidget* exifSettingsWidget;
		DkResourceSettingsWidgets* resourceSettingsWidget;
};

class DkSettingsWidget : public QWidget {
Q_OBJECT	

public:
		DkSettingsWidget(QWidget* parent) : QWidget(parent) { showOnlyInAdvancedMode = false;};
		virtual void writeSettings() = 0;
		virtual void init() = 0;
		virtual void toggleAdvancedOptions(bool showAdvancedOptions) {};

		bool showOnlyInAdvancedMode;
};

class DkColorChooser;

class DkGlobalSettingsWidget : public DkSettingsWidget {
	Q_OBJECT;

	public:
		DkGlobalSettingsWidget(QWidget* parent);
	
		void writeSettings();

	signals:
		void applyDefault();

	private slots:
		void setToDefaultPressed() {
			qDebug() << "apply default pressed...";
			emit applyDefault();
		};
		void bgColorReset() {
			DkSettings::display.useDefaultColor = true;
		};
		void iconColorReset() {
			DkSettings::display.defaultIconColor = true;
		};



	private:
		void init();
		void createLayout();

		
		DkDoubleSpinBoxWidget* displayTimeSpin;
		QCheckBox* cbShowMenu;
		QCheckBox* cbShowToolbar;
		QCheckBox* cbShowStatusbar;
		QCheckBox* cbSmallIcons;
		QCheckBox* cbToolbarGradient;
		QCheckBox* cbCloseOnEsc;
		QCheckBox* cbZoomOnWheel;

		DkColorChooser* highlightColorChooser;
		DkColorChooser* bgColorWidgetChooser;
		DkColorChooser* bgColorChooser;
		DkColorChooser* iconColorChooser;
		DkColorChooser* fullscreenColChooser;

		QComboBox* langCombo;
		
		QPushButton* buttonDefaultSettings;

		QString curLanguage;
		QStringList languages;

		bool loop;
		bool scanSubFolders;

};


class DkDisplaySettingsWidget : public DkSettingsWidget {
	Q_OBJECT	

	public:
		enum DisplayItems{
			display_file_name,
			display_creation_date,
			display_file_rating,

			display_end
		};

		DkDisplaySettingsWidget(QWidget* parent);

		void writeSettings();

	private slots:
		void showFileName(bool checked);
		void showCreationDate(bool checked);
		void showRating(bool checked);

	private:
		void init();
		void createLayout();

		QGroupBox* gbThumb;

		QCheckBox* cbKeepZoom;
		QCheckBox* cbInvertZoom;

		DkSpinBoxWidget* interpolateWidget;
		QCheckBox* cbCreationDate;
		QCheckBox* cbName;
		QCheckBox* cbRating;
		QCheckBox* cbSilentFullscreen;



		DkSpinBoxWidget* maximalThumbSizeWidget; 
		QCheckBox* cbSaveThumb;
		QCheckBox* cbShowBorder;

		bool keepZoom;
		bool invertZoom;

};


class DkFileWidget : public DkSettingsWidget {
Q_OBJECT	

	public:
		DkFileWidget(QWidget* parent);

		void writeSettings();
	
	private slots:
		void tmpPathButtonPressed();
		void useTmpPathChanged(int state);
		void lineEditChanged(QString path);
		void openWithDialog();

	private:
		void init();
		void createLayout();
		bool existsDirectory(QString path);


		QVBoxLayout* vBoxLayout;
		QLineEdit* leTmpPath;
		QPushButton* pbTmpPath;
		QCheckBox* cbUseTmpPath;
		DkSpinBoxWidget* skipImgWidget;
		DkSpinBoxWidget* numberFiles;
		QCheckBox* cbWrapImages;
		

		QGroupBox* 	gbDragDrop;

		int filter;
		QString tmpPath;
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
		QCheckBox* cbAllowFile;
		QCheckBox* cbSwitchModifier;
		QGroupBox* gbNetworkSettings;
};

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
			camData_iso,
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

		static QStringList scamDataDesc;
		static QStringList sdescriptionDesc;

		DkMetaDataSettingsWidget(QWidget* parent);

		void writeSettings();

	private:
		void init();
		void createLayout();

		QCheckBox* cbIgnoreOrientation;
		QCheckBox* cbSaveOrientation;

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

class DkResourceSettingsWidgets: public DkSettingsWidget {
	Q_OBJECT

public:
	DkResourceSettingsWidgets(QWidget* parent);

	void writeSettings();

	private slots:
		void memorySliderChanged(int newValue);

	private:
		void init();
		void createLayout();

		QCheckBox* cbFastThumbnailPreview;
		QCheckBox* cbFilterRawImages;
		QCheckBox* cbRemoveDuplicates;
		QComboBox* cmExtensions;
		QSlider* sliderMemory;
		QLabel* labelMemory;
	
		double stepSize;
		double totalMemory;
};



class DkSpinBoxWidget : public QWidget {
	Q_OBJECT;
public:	
	DkSpinBoxWidget(QWidget* parent = 0);
	DkSpinBoxWidget(QString upperString, QString lowerString, int spinBoxMin, int spinBoxMax, QWidget* parent=0, int step = 1);
	QSpinBox* getSpinBox() { return spinBox;};
	void setSpinBoxValue(int value) {spinBox->setValue(value);};
	int getSpinBoxValue() {return spinBox->value();};

	//virtual QSize sizeHint() const {

	//	return optimalSize;
	//}

private:
	QSpinBox* spinBox;
	QLabel* upperLabel;
	QLabel* lowerLabel;
	QWidget* lowerWidget;
	QVBoxLayout* vboxLayout;
	QHBoxLayout* hboxLowerLayout;
	QSize optimalSize;
};

class DkDoubleSpinBoxWidget : public QWidget {
	Q_OBJECT;
public:	
	DkDoubleSpinBoxWidget(QWidget* parent = 0);
	DkDoubleSpinBoxWidget(QString upperString, QString lowerString, float spinBoxMin, float spinBoxMax, QWidget* parent=0, int step = 1, int decimals = 2);
	QDoubleSpinBox* getSpinBox() { return spinBox;};
	void setSpinBoxValue(float value) {spinBox->setValue(value);};
	float getSpinBoxValue() {return spinBox->value();};

	//virtual QSize sizeHint() const {

	//	return optimalSize;
	//}

private:
	QDoubleSpinBox* spinBox;
	QLabel* upperLabel;
	QLabel* lowerLabel;
	QWidget* lowerWidget;
	QVBoxLayout* vboxLayout;
	QHBoxLayout* hboxLowerLayout;
	QSize optimalSize;
};

};
