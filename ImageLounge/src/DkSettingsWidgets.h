/*******************************************************************************************************
 DkSettingsWidgets.h
 Created on:	17.03.2014
 
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

#include <QObject>
#include <QDialog>
#include <QModelIndex>
#include <QGroupBox>
#include <QSpinBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QListView>
#include <QKeyEvent>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QDebug>
#include <QButtonGroup>
#include <QFileDialog>
#include <QStandardItem>
#include <QTableView>
#include <QStyledItemDelegate>

#include "DkSettings.h"
#include "BorderLayout.h"

namespace nmc {

class DkColorChooser;

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

class DkCheckBoxDelegate : public QStyledItemDelegate {
	Q_OBJECT
public:
	DkCheckBoxDelegate(QObject* parent = 0) : QStyledItemDelegate(parent) {};


	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

	void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

	private slots:
		void cbChanged(int);

};

class DkWhiteListViewModel : public QAbstractTableModel {
	Q_OBJECT
public:
	DkWhiteListViewModel(QObject* parent=0);

	virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const { return 3;};
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role);

	void addWhiteListEntry(bool checked, QString name, QDateTime lastSeen);
	Qt::ItemFlags flags(const QModelIndex& index) const;

	QVector<bool> getCheckedVector() {return checked;};
	QVector<QString> getNamesVector() {return names;};

private:
	QVector<bool> checked;
	QVector<QString> names;
	QVector<QDateTime> lastSeen;
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

class DkSettingsWidget : public QWidget {
	Q_OBJECT	

public:
	DkSettingsWidget(QWidget* parent) : QWidget(parent) { showOnlyInAdvancedMode = false;};
	virtual void writeSettings() = 0;
	virtual void init() = 0;
	virtual void toggleAdvancedOptions(bool showAdvancedOptions) {};

	bool showOnlyInAdvancedMode;
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

private:
	void init();
	void createLayout();
	bool existsDirectory(QString path);


	QVBoxLayout* vBoxLayout;
	QLineEdit* leTmpPath;
	QPushButton* pbTmpPath;
	QCheckBox* cbUseTmpPath;
	DkSpinBoxWidget* skipImgWidget;
	//DkSpinBoxWidget* numberFiles;
	QCheckBox* cbWrapImages;
	
	QGroupBox* 	gbDragDrop;

	QRadioButton* rbWaitForImage;
	QRadioButton* rbSkipImage;

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

class DkFileFilterSettingWidget: public DkSettingsWidget {
	Q_OBJECT

public:
	DkFileFilterSettingWidget(QWidget* parent);
	virtual void writeSettings();
	virtual void init();

protected:
	void createLayout();
	QList<QStandardItem*> getItems(const QString& filter, bool browse, bool reg);
	bool checkFilter(const QString& filter, const QStringList& filters) const;

	QTableView* filterTableView;
	QStandardItemModel* model;
	QCheckBox* browseAll, registerAll;

};

class DkDisplaySettingsWidget : public DkSettingsWidget {
	Q_OBJECT	

public:

	DkDisplaySettingsWidget(QWidget* parent);

	void writeSettings();

	private slots:
		void showFileName(bool checked);
		void showCreationDate(bool checked);
		void showRating(bool checked);

private:
	void init();
	void createLayout();

	//QGroupBox* gbThumb;

	QCheckBox* cbInvertZoom;

	DkSpinBoxWidget* interpolateWidget;
	QCheckBox* cbCreationDate;
	QCheckBox* cbName;
	QCheckBox* cbRating;
	QCheckBox* cbSilentFullscreen;
	DkDoubleSpinBoxWidget* fadeSlideShow;

	QVector<QRadioButton* > keepZoomButtons;
	QButtonGroup* keepZoomButtonGroup;

	DkSpinBoxWidget* maximalThumbSizeWidget; 
	//QCheckBox* cbSaveThumb;
	QCheckBox* cbShowBorder;

	bool keepZoom;
	bool invertZoom;

};


class DkMetaDataSettingsWidget : public DkSettingsWidget {
	Q_OBJECT

public:

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

	QCheckBox* cbFilterRawImages;
	QCheckBox* cbRemoveDuplicates;
	QComboBox* cmExtensions;
	QSlider* sliderMemory;
	QLabel* labelMemory;

	double stepSize;
	double totalMemory;

	QVector<QRadioButton* > rawThumbButtons;
	QButtonGroup* rawThumbButtonGroup;
};

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
	QCheckBox* cbShowRecentFiles;
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


class DkRemoteControlWidget: public DkSettingsWidget {
	Q_OBJECT

public:
	DkRemoteControlWidget(QWidget* parent);

	void writeSettings();

private:
	void init();
	void createLayout();

	QGridLayout* whiteListGrid;

	QTableView* table;
	DkWhiteListViewModel* whiteListModel;

};

class DkSettingsDialog : public QDialog {
	Q_OBJECT;

public:
	DkSettingsDialog(QWidget* parent);
	//DkSettingsDialog(const DkSettingsDialog& dialog) {
	//	this->borderLayout = dialog.borderLayout;
	//	this->listView = dialog.listView;
	//	this->rightWidget = dialog.rightWidget;
	//	this->leftLabel = dialog.leftLabel;
	//	this->buttonOk = dialog.buttonOk;
	//	this->buttonCancel = dialog.buttonCancel;
	//	this->widgetList = dialog.widgetList;
	//	this->centralWidget = dialog.centralWidget;
	//	this->centralLayout = dialog.centralLayout;
	//	this->globalSettingsWidget = dialog.globalSettingsWidget;
	//	this->slideshowSettingsWidget = dialog.slideshowSettingsWidget;
	//	this->synchronizeSettingsWidget = dialog.synchronizeSettingsWidget;
	//}
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
	DkFileFilterSettingWidget* fileFilterSettingsWidget;
	DkRemoteControlWidget* remoteControlWidget;
};

};

