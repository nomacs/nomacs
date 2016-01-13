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

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QDialog>
#include <QStyledItemDelegate>
#include <QListView>
#include <QDateTime>
#pragma warning(pop)		// no warnings from includes - end

// Qt defines
class QGroupBox;
class QSpinBox;
class QDoubleSpinBox;
class QLabel;
class QVBoxLayout;
class QHBoxLayout;
class QKeyEvent;
class QCheckBox;
class QRadioButton;
class QButtonGroup;
class QStandardItem;
class QTableView;
class QStandardItemModel;
class QComboBox;
class QGridLayout;

namespace nmc {

// nomacs defines
class DkColorChooser;
class DkDirectoryEdit;

class DkSpinBoxWidget : public QWidget {
	Q_OBJECT;
public:	
	DkSpinBoxWidget(QWidget* parent = 0);
	DkSpinBoxWidget(QString upperString, QString lowerString, int spinBoxMin, int spinBoxMax, QWidget* parent=0, int step = 1);
	QSpinBox* getSpinBox() const;
	void setSpinBoxValue(int value);
	int getSpinBoxValue() const;

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
	QDoubleSpinBox* getSpinBox() const;
	void setSpinBoxValue(float value);
	float getSpinBoxValue() const;

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
	virtual int columnCount(const QModelIndex& = QModelIndex()) const { return 3;};
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role);

	void addWhiteListEntry(bool checked, QString name, QDateTime lastSeen);
	Qt::ItemFlags flags(const QModelIndex& index) const;

	QVector<bool> getCheckedVector() {return mChecked;};
	QVector<QString> getNamesVector() {return mNames;};

private:
	QVector<bool> mChecked;
	QVector<QString> mNames;
	QVector<QDateTime> mLastSeen;
};



class DkSettingsListView : public QListView {
	Q_OBJECT;

public:
	DkSettingsListView(QWidget* parent) : QListView(parent) {};
	~DkSettingsListView() {QItemSelectionModel* sm = this->selectionModel(); delete sm;};	// wtf?

public slots:
	void keyPressEvent(QKeyEvent *event);

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
	virtual void toggleAdvancedOptions(bool) {};

	bool showOnlyInAdvancedMode;
};

class DkFileWidget : public DkSettingsWidget {
	Q_OBJECT	

public:
	DkFileWidget(QWidget* parent);

	void writeSettings();

private slots:
	void tmpPathButtonPressed();
	void useTmpPathChanged(int idx);

private:
	void init();
	void createLayout();
	bool existsDirectory(QString path);

	QVBoxLayout* vBoxLayout;
	DkDirectoryEdit* leTmpPath;
	QPushButton* pbTmpPath;
	QCheckBox* cbUseTmpPath;
	DkSpinBoxWidget* skipImgWidget;
	QCheckBox* cbWrapImages;
	QCheckBox* cbAskToSaveDeletedFiles;
	QCheckBox* cbLogRecentFiles;

	QCheckBox* cbIgnoreOrientation;
	QCheckBox* cbSaveOrientation;

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

public slots:
	void itemChanged(QStandardItem* item);
	void openDefault() const;

protected:
	void createLayout();
	QList<QStandardItem*> getItems(const QString& filter, bool browse, bool reg);
	bool checkFilter(const QString& filter, const QStringList& filters) const;

	QTableView* filterTableView;
	QStandardItemModel* model;
	QCheckBox* browseAll;
	QCheckBox* registerAll;
	bool saveSettings;
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
	void setToDefaultPressed();
	void bgColorReset();
	void iconColorReset();

private:
	void init();
	void createLayout();

	DkDoubleSpinBoxWidget* displayTimeSpin;
	QCheckBox* cbSmallIcons;
	QCheckBox* cbToolbarGradient;
	QCheckBox* cbCloseOnEsc;
	QCheckBox* cbShowRecentFiles;
	QCheckBox* cbZoomOnWheel;
	QCheckBox* cbCheckForUpdates;

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
	~DkSettingsDialog();

signals:
	void languageChanged();
	void settingsChangedRestart();
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
	void setToDefault();
	void advancedSettingsChanged(int idx);

protected:
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
	DkResourceSettingsWidgets* resourceSettingsWidget;
	DkFileFilterSettingWidget* fileFilterSettingsWidget;
	DkRemoteControlWidget* remoteControlWidget;
};

};

