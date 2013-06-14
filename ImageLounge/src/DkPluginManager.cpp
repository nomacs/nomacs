/*******************************************************************************************************
 DkPluginManager.cpp
 Created on:	20.05.2013

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

#include "DkPluginManager.h"


namespace nmc {

/**********************************************************************************
* Plugin manager dialog
**********************************************************************************/
DkPluginManager::DkPluginManager(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

	init();
}

DkPluginManager::~DkPluginManager() {

}

/**
* initialize plugin manager dialog - set sizes
 **/
void DkPluginManager::init() {
	
	loadedPlugins = QMap<QString, DkPluginInterface *>();
	pluginLoaders = QMap<QString, QPluginLoader *>();
	pluginFiles = QMap<QString, QString>();
	loadPlugins();

	dialogWidth = 700;
	dialogHeight = 500;
	/*
	tabsMargin = 20;
	tabsWidth = dialogWidth - 2 * tabsMargin;
	tabsHeight = dialogHeight - tabsMargin - 30;
	*/
	setWindowTitle(tr("Plug-in manager"));
	setFixedSize(dialogWidth, dialogHeight);
	createLayout();
}

/**
* create plugin manager dialog layout
 **/
void DkPluginManager::createLayout() {
	/*
	// bottom widget - buttons	
	QWidget* bottomWidget = new QWidget(this);
	QHBoxLayout* bottomWidgetHBoxLayout = new QHBoxLayout(bottomWidget);

	QPushButton* buttonClose = new QPushButton(tr("&Close"));
	connect(buttonClose, SIGNAL(clicked()), this, SLOT(closePressed()));

	QSpacerItem* spacer = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Expanding);
	bottomWidgetHBoxLayout->addItem(spacer);
	bottomWidgetHBoxLayout->addWidget(buttonClose);
	
	// central widget - tabs
	QWidget* centralWidget = new QWidget(this);

	QTabWidget* tabs = new QTabWidget(centralWidget);
	tabs->setGeometry(QRect(QPoint(tabsMargin, tabsMargin), QSize(tabsWidth, tabsHeight)));

	tabs->addTab(new QWidget(), tr("Manage installed plug-ins"));  
	tabs->addTab(new QWidget(), tr("Download new plug-ins"));

	BorderLayout* borderLayout = new BorderLayout;
	borderLayout->addWidget(bottomWidget, BorderLayout::South);
	borderLayout->addWidget(centralWidget, BorderLayout::Center);
	this->setSizeGripEnabled(false);

	this->setLayout(borderLayout);

	*/

	QVBoxLayout* verticalLayout = new QVBoxLayout(this);
	tabs = new QTabWidget(this);

	tableWidgetInstalled = new DkPluginTableWidget(tab_installed_plugins, this, tabs->currentWidget());
	tabs->addTab(tableWidgetInstalled, tr("Manage installed plug-ins"));
	tableWidgetDownload = new DkPluginTableWidget(tab_download_plugins, this, tabs->currentWidget());
	tabs->addTab(tableWidgetDownload, tr("Download new plug-ins"));
	connect(tabs, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    verticalLayout->addWidget(tabs);
	
    QHBoxLayout* horizontalLayout = new QHBoxLayout();
    QSpacerItem* horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout->addItem(horizontalSpacer);

	QPushButton* buttonClose = new QPushButton(tr("&Close"));
	connect(buttonClose, SIGNAL(clicked()), this, SLOT(closePressed()));
	buttonClose->setDefault(true);

    horizontalLayout->addWidget(buttonClose);

    verticalLayout->addLayout(horizontalLayout);

	this->setSizeGripEnabled(false);
}


void DkPluginManager::closePressed() {

	this->close();
}

void DkPluginManager::showEvent(QShowEvent *event) {

	loadPlugins();
	tabs->setCurrentIndex(tab_installed_plugins);
	tableWidgetInstalled->clearTableFilters();
	tableWidgetInstalled->updateModels();
	tableWidgetDownload->clearTableFilters();
}

/**
* Loads/reloads installed plug-ins
 **/
void DkPluginManager::loadPlugins() {

	// if reloading first delete all instances
	if (!loadedPlugins.isEmpty()) {
		pluginFiles.clear();
		pluginIdList.clear();
		loadedPlugins.clear();
		for (auto it = pluginLoaders.begin(); it != pluginLoaders.end();) {

			QPluginLoader *loaderToDelete = it.value();

			if(loaderToDelete->unload()) delete loaderToDelete;
			else qDebug() << "Could not unload plug-in loader!";

			pluginLoaders.erase(it++);
		}		
	}

	QDir pluginsDir = QDir(qApp->applicationDirPath());
    pluginsDir.cd("plugins");

	foreach(QString fileName, pluginsDir.entryList(QDir::Files)) {

		QPluginLoader *loader = new QPluginLoader(pluginsDir.absoluteFilePath(fileName));
		QObject *plugin = loader->instance();
		if(plugin) {

			DkPluginInterface *initializedPlugin = qobject_cast<DkPluginInterface*>(plugin);
			if(initializedPlugin) {
				QString pluginID = initializedPlugin->pluginID();
				pluginIdList.append(pluginID);
				loadedPlugins.insert(pluginID, initializedPlugin);
				pluginLoaders.insert(pluginID, loader);
				pluginFiles.insert(pluginID, pluginsDir.absoluteFilePath(fileName));
			}
		}
		else delete loader;
	}
}

/**
* returns map with id and interface
**/
QMap<QString, DkPluginInterface *> DkPluginManager::getPlugins() {

	return loadedPlugins;
}

QList<QString> DkPluginManager::getPluginIdList() {

	return pluginIdList;
}

QMap<QString, QString> DkPluginManager::getPluginFileNames() {

	return pluginFiles;
}

void DkPluginManager::setPluginIdList(QList<QString> newPlugins) {

	pluginIdList = newPlugins;
}

void DkPluginManager::setRunId2PluginId(QMap<QString, QString> newMap) {

	runId2PluginId = newMap;
}

QMap<QString, QString> DkPluginManager::getRunId2PluginId(){

	return runId2PluginId;
}

void DkPluginManager::tabChanged(int tab){

	if(tab == tab_installed_plugins) tableWidgetInstalled->updateModels();
	else if(tab == tab_download_plugins) tableWidgetDownload->updateModels();
}

void DkPluginManager::deletePlugin(QString pluginID) {

	QPluginLoader *loaderToDelete = pluginLoaders.take(pluginID);
		
	if(loaderToDelete->unload()) delete loaderToDelete;
	else qDebug() << "Could not unload plug-in loader!";

	QFile file(pluginFiles.take(pluginID));
	if(!file.remove()) {
		qDebug() << "Failed to delete plug-in file!";
		QMessageBox::critical(this, tr("Plug-in manager"), tr("The dll could not be deleted!\nPlease restart nomacs and try again."));
	}

	loadedPlugins.remove(pluginID);
}


/**********************************************************************************
*DkPluginTableWidget : Widget with table views containing plugin data
**********************************************************************************/

DkPluginTableWidget::DkPluginTableWidget(int tab, DkPluginManager* manager, QWidget* parent) : QWidget(parent) {

	//init();
	openedTab = tab;
	pluginManager = manager;
	createLayout();
}

DkPluginTableWidget::~DkPluginTableWidget() {

}

/**
* create the main layout of the plugin manager
**/
void DkPluginTableWidget::createLayout() {

	QVBoxLayout* verticalLayout = new QVBoxLayout(this);

	// search line edit and update button
	QHBoxLayout* searchHorLayout = new QHBoxLayout();
	QLabel* searchLabel = new QLabel(tr("&Search plug-ins: "), this);
	searchHorLayout->addWidget(searchLabel);
	filterEdit = new QLineEdit(this);
	filterEdit->setFixedSize(160,20);
	connect(filterEdit, SIGNAL(textChanged(QString)), this, SLOT(filterTextChanged()));
	searchLabel->setBuddy(filterEdit);
	searchHorLayout->addWidget(filterEdit);
	QSpacerItem* horizontalSpacer;
	if(openedTab == tab_installed_plugins) horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	else horizontalSpacer = new QSpacerItem(40, 23, QSizePolicy::Expanding, QSizePolicy::Minimum);
	searchHorLayout->addItem(horizontalSpacer);
	if(openedTab == tab_installed_plugins) {
		QPushButton *updateButton = new QPushButton("&Update plug-ins", this);
		connect(updateButton, SIGNAL(clicked()),this, SLOT(updatePlugins()));
		updateButton->setFixedWidth(120);
		searchHorLayout->addWidget(updateButton);
	}
	verticalLayout->addLayout(searchHorLayout);

	// main table
    tableView = new QTableView(this);
	proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setDynamicSortFilter(true);
	//tableView->setMaximumHeight(100);
	if(openedTab == tab_installed_plugins) model = new DkInstalledPluginsModel(pluginManager->getPluginIdList(), this);
	else if (openedTab == tab_download_plugins) model = new DkDownloadPluginsModel(pluginManager->getPluginIdList(), this);
	proxyModel->setSourceModel(model);
	tableView->setModel(proxyModel);
	tableView->resizeColumnsToContents();
	if(openedTab == tab_installed_plugins) {
		tableView->setColumnWidth(ip_column_name, qMax(tableView->columnWidth(ip_column_name), 300));
		tableView->setColumnWidth(ip_column_version, qMax(tableView->columnWidth(ip_column_version), 80));
		tableView->setColumnWidth(ip_column_enabled, qMax(tableView->columnWidth(ip_column_enabled), 130));
	} else if (openedTab == tab_download_plugins) {
		tableView->setColumnWidth(dp_column_name, qMax(tableView->columnWidth(dp_column_name), 360));
		tableView->setColumnWidth(dp_column_version, qMax(tableView->columnWidth(dp_column_version), 80));
		//tableView->setColumnWidth(ip_column_enabled, qMax(tableView->columnWidth(ip_column_enabled), 130));
	}

	tableView->resizeRowsToContents();	
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setSortingEnabled(true);
	tableView->sortByColumn(ip_column_name, Qt::AscendingOrder);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->verticalHeader()->hide();
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
	tableView->setAlternatingRowColors(true);
	//tableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	if(openedTab == tab_installed_plugins) {
		tableView->setItemDelegateForColumn(ip_column_enabled, new DkCheckBoxDelegate(tableView));
		DkPushButtonDelegate* buttonDelegate = new DkPushButtonDelegate(tableView);
		tableView->setItemDelegateForColumn(ip_column_uninstall, buttonDelegate);
		connect(buttonDelegate, SIGNAL(buttonClicked(QModelIndex)), this, SLOT(uninstallPlugin(QModelIndex)));
	} else if (openedTab == tab_download_plugins) {
		DkDownloadDelegate* buttonDelegate = new DkDownloadDelegate(tableView);
		tableView->setItemDelegateForColumn(dp_column_install, buttonDelegate);
		connect(buttonDelegate, SIGNAL(buttonClicked(QModelIndex)), this, SLOT(uninstallPlugin(QModelIndex)));
	}
    verticalLayout->addWidget(tableView);

	QSpacerItem* verticalSpacer = new QSpacerItem(40, 15, QSizePolicy::Expanding, QSizePolicy::Minimum);
	verticalLayout->addItem(verticalSpacer);

	QVBoxLayout* bottomVertLayout = new QVBoxLayout();

	// additional information
	QHBoxLayout* topHorLayout = new QHBoxLayout();
	QLabel* descLabel = new QLabel(tr("Plug-in description:"));
	topHorLayout->addWidget(descLabel);
	QLabel* previewLabel = new QLabel(tr("Plug-in preview:"));
	topHorLayout->addWidget(previewLabel);
	bottomVertLayout->addLayout(topHorLayout);

	QHBoxLayout* bottHorLayout = new QHBoxLayout();
	int layoutHWidth = this->geometry().width();
	int layoutHWidth2 = this->contentsRect().width();

	//QTextEdit* decriptionEdit = new QTextEdit();
	//decriptionEdit->setReadOnly(true);
	//bottHorLayout->addWidget(decriptionEdit);
	DkDescriptionEdit* decriptionEdit = new DkDescriptionEdit(model, proxyModel, tableView->selectionModel(), this);
	connect(tableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), decriptionEdit, SLOT(selectionChanged(const QItemSelection &, const QItemSelection &)));
	connect(proxyModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), decriptionEdit, SLOT(dataChanged(const QModelIndex &, const QModelIndex &)));
	bottHorLayout->addWidget(decriptionEdit);

	//QLabel* imgLabel = new QLabel();
	//imgLabel->setMinimumWidth(layoutHWidth/2 + 4);
	//imgLabel->setPixmap(QPixmap::fromImage(QImage(":/nomacs/img/imgDescriptionMissing.png")));	
	DkDescriptionImage* decriptionImg = new DkDescriptionImage(model, proxyModel, tableView->selectionModel(), this);
	//decriptionImg->setMinimumWidth(324);//layoutHWidth/2 + 4);
	//decriptionImg->setMaximumWidth(324);//layoutHWidth/2 + 4);
	decriptionImg->setMaximumSize(324,168);
	decriptionImg->setMinimumSize(324,168);
	connect(tableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), decriptionImg, SLOT(selectionChanged(const QItemSelection &, const QItemSelection &)));
	connect(proxyModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), decriptionImg, SLOT(dataChanged(const QModelIndex &, const QModelIndex &)));
	bottHorLayout->addWidget(decriptionImg);
	bottomVertLayout->addLayout(bottHorLayout);

	verticalLayout->addLayout(bottomVertLayout);
}

void DkPluginTableWidget::updatePlugins() {
					
	QMessageBox msgBox;
	msgBox.setText("Updating plug-ins");
	msgBox.exec();
}

void DkPluginTableWidget::filterTextChanged() {

	Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive; //filterCaseSensitivityCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;

	QRegExp regExp(filterEdit->text(), caseSensitivity, QRegExp::FixedString);
	proxyModel->setFilterRegExp(regExp);
	tableView->resizeRowsToContents();
}

void DkPluginTableWidget::uninstallPlugin(const QModelIndex &index) {

	DkInstalledPluginsModel* installedPluginsModel = static_cast<DkInstalledPluginsModel*>(model);
	int selectedRow = proxyModel->mapToSource(index).row();
	QString pluginID = installedPluginsModel->getPluginData().at(selectedRow);

	QMessageBox msgBox;
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::No);
	msgBox.setEscapeButton(QMessageBox::No);
	msgBox.setIcon(QMessageBox::Question);
	msgBox.setWindowTitle(tr("Uninstall plug-ins"));

	msgBox.setText(tr("Do you really want to uninstall the plug-in <i>%1</i>?").arg(pluginManager->getPlugins().value(pluginID)->pluginName()));

	if(msgBox.exec() == QMessageBox::Yes) {

		QMap<QString, bool> enabledSettings = installedPluginsModel->getEnabledData();

		QList<QString> pluginIdList = pluginManager->getPluginIdList();	
		pluginIdList.removeAt(selectedRow);			
		enabledSettings.remove(pluginID);

		pluginManager->setPluginIdList(pluginIdList);
		installedPluginsModel->setEnabledData(enabledSettings);
		installedPluginsModel->savePluginsEnabledSettings();
		updateModels();	// !!! update model before deleting the interface

		pluginManager->deletePlugin(pluginID);
	}
}

void DkPluginTableWidget::clearTableFilters(){

	filterEdit->clear();
	filterEdit->setFocus();
}

/*
* update models if new plug-ins are installed or copied into the folder
*/
void DkPluginTableWidget::updateModels() {

	DkInstalledPluginsModel* installedPluginsModel = static_cast<DkInstalledPluginsModel*>(model);
	installedPluginsModel->loadPluginsEnabledSettings();

	QList<QString> newPluginList = pluginManager->getPluginIdList();
	QList<QString> tableList = installedPluginsModel->getPluginData();

	for (int i = tableList.size() - 1; i >= 0; i--) {
		if (!newPluginList.contains(tableList.at(i))) installedPluginsModel->removeRows(i, 1);
	}

	for (int i = newPluginList.size() - 1; i >= 0; i--) {
		if (!tableList.contains(newPluginList.at(i))) {
			installedPluginsModel->setDataToInsert(newPluginList.at(i));
			installedPluginsModel->insertRows(installedPluginsModel->getPluginData().size(), 1);
		}
	}

	tableView->resizeRowsToContents();
}

DkPluginManager* DkPluginTableWidget::getPluginManager() {

	return pluginManager;
}

int DkPluginTableWidget::getOpenedTab() {

	return openedTab;
}


/**********************************************************************************
*DkInstalledPluginsModel : Model managing installed plug-ins data in the table
**********************************************************************************/

DkInstalledPluginsModel::DkInstalledPluginsModel(QObject *parent) : QAbstractTableModel(parent) {

}

DkInstalledPluginsModel::DkInstalledPluginsModel(QList<QString> data, QObject *parent) : QAbstractTableModel(parent) {

	parentTable = static_cast<DkPluginTableWidget*>(parent);

	pluginData = data;
	pluginsEnabled = QMap<QString, bool>();

	loadPluginsEnabledSettings();
}

int DkInstalledPluginsModel::rowCount(const QModelIndex &parent) const {

	return pluginData.size();
}

int DkInstalledPluginsModel::columnCount(const QModelIndex &parent) const {

	return ip_column_size;
}

QVariant DkInstalledPluginsModel::data(const QModelIndex &index, int role) const {

    if (!index.isValid()) return QVariant();

    if (index.row() >= pluginData.size() || index.row() < 0) return QVariant();

    if (role == Qt::DisplayRole) {
		
		QString pluginID = pluginData.at(index.row());

        if (index.column() == ip_column_name) {
			return parentTable->getPluginManager()->getPlugins().value(pluginID)->pluginName();
		}
        else if (index.column() == ip_column_version) {
			return parentTable->getPluginManager()->getPlugins().value(pluginID)->pluginVersion();
		}
        else if (index.column() == ip_column_enabled)
			return pluginsEnabled.value(pluginID, true);
		else if (index.column() == ip_column_uninstall)
			return QString(tr("Uninstall"));
    }
	
    return QVariant();
}

QVariant DkInstalledPluginsModel::headerData(int section, Qt::Orientation orientation, int role) const {

    if (role != Qt::DisplayRole) return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
            case ip_column_name:
                return tr("Name");
            case ip_column_version:
                return tr("Version");
            case ip_column_enabled:
                return tr("Enabled/Disabled");
			case ip_column_uninstall:
				return tr("Uninstall plug-in");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

Qt::ItemFlags DkInstalledPluginsModel::flags(const QModelIndex &index) const {

    if (!index.isValid())
        return Qt::ItemIsEnabled;

	return QAbstractTableModel::flags(index);
}

bool DkInstalledPluginsModel::setData(const QModelIndex &index, const QVariant &value, int role) {

	if (index.isValid() && role == Qt::EditRole) {
        
		if (index.column() == ip_column_enabled) {
			pluginsEnabled.insert(pluginData.at(index.row()), value.toBool());	// TODO modify Qsettings
			savePluginsEnabledSettings();

			emit(dataChanged(index, index));

			return true;
		}
    }

    return false;
}

bool DkInstalledPluginsModel::insertRows(int position, int rows, const QModelIndex &index) {

    beginInsertRows(QModelIndex(), position, position+rows-1);

    for (int row=0; row < rows; row++) {
		pluginData.insert(position, dataToInsert);
    }

    endInsertRows();
    return true;
}

bool DkInstalledPluginsModel::removeRows(int position, int rows, const QModelIndex &index) {

    beginRemoveRows(QModelIndex(), position, position+rows-1);

    for (int row=0; row < rows; row++) {
        pluginData.removeAt(position);
    }

    endRemoveRows();
    return true;
}

QList<QString> DkInstalledPluginsModel::getPluginData() {
	
	return pluginData;
}

void DkInstalledPluginsModel::setDataToInsert(QString newData) {

	dataToInsert = newData;
}

void DkInstalledPluginsModel::setEnabledData(QMap<QString, bool> enabledData) {

	pluginsEnabled = enabledData;
}

QMap<QString, bool> DkInstalledPluginsModel::getEnabledData() {

	return pluginsEnabled;
}

void DkInstalledPluginsModel::loadPluginsEnabledSettings() {

	pluginsEnabled.clear();

	QSettings settings;
	int size = settings.beginReadArray("PluginSettings/disabledPlugins");
	for (int i = 0; i < size; i++) {

		settings.setArrayIndex(i);
		pluginsEnabled.insert(settings.value("pluginId").toString(), false);
	}
	settings.endArray();
}

void DkInstalledPluginsModel::savePluginsEnabledSettings() {
	
	QSettings settings;
	settings.remove("PluginSettings/disabledPlugins");
	
	if (pluginsEnabled.size() > 0) {

		int i = 0;
		QMapIterator<QString, bool> iter(pluginsEnabled);	

		settings.beginWriteArray("PluginSettings/disabledPlugins");
		while(iter.hasNext()) {
			iter.next();
			if (!iter.value()) {
				settings.setArrayIndex(i++);
				settings.setValue("pluginId", iter.key());
				settings.setValue("pluginFileName", parentTable->getPluginManager()->getPluginFileNames().value(iter.key()));
			}
		}

		settings.endArray();
	}
}


/**********************************************************************************
*DkDownloadPluginsModel : Model managing the download plug.in data
**********************************************************************************/

DkDownloadPluginsModel::DkDownloadPluginsModel(QObject *parent) : QAbstractTableModel(parent) {

}

DkDownloadPluginsModel::DkDownloadPluginsModel(QList<QString> data, QObject *parent) : QAbstractTableModel(parent) {

	parentTable = static_cast<DkPluginTableWidget*>(parent);

	pluginData = data;
	pluginsEnabled = QMap<QString, bool>();

	loadPluginsEnabledSettings();
}

int DkDownloadPluginsModel::rowCount(const QModelIndex &parent) const {

	return pluginData.size();
}

int DkDownloadPluginsModel::columnCount(const QModelIndex &parent) const {

	return dp_column_size;
}

QVariant DkDownloadPluginsModel::data(const QModelIndex &index, int role) const {

    if (!index.isValid()) return QVariant();

    if (index.row() >= pluginData.size() || index.row() < 0) return QVariant();

    if (role == Qt::DisplayRole) {
		
		QString pluginID = pluginData.at(index.row());

        if (index.column() == dp_column_name) {
			return parentTable->getPluginManager()->getPlugins().value(pluginID)->pluginName();
		}
        else if (index.column() == dp_column_version) {
			return parentTable->getPluginManager()->getPlugins().value(pluginID)->pluginVersion();
		}
//       else if (index.column() == ip_column_enabled)
//			return pluginsEnabled.value(pluginID, true);
		else if (index.column() == dp_column_install)
			return QString(tr("Download and Install"));
    }

	if (role == Qt::UserRole) {
		if (index.row() == 7 || index.row() == 3 || index.row() == 1) return false;
		else return true;
	}
	
    return QVariant();
}

QVariant DkDownloadPluginsModel::headerData(int section, Qt::Orientation orientation, int role) const {

    if (role != Qt::DisplayRole) return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
            case dp_column_name:
                return tr("Name");
            case dp_column_version:
                return tr("Version");
			case dp_column_install:
				return tr("Download and install plug-in");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

Qt::ItemFlags DkDownloadPluginsModel::flags(const QModelIndex &index) const {

    if (!index.isValid())
        return Qt::ItemIsEnabled;

	return QAbstractTableModel::flags(index);
}

bool DkDownloadPluginsModel::setData(const QModelIndex &index, const QVariant &value, int role) {

	if (index.isValid() && role == Qt::EditRole) {
        
		if (index.column() == ip_column_enabled) {
			pluginsEnabled.insert(pluginData.at(index.row()), value.toBool());	// TODO modify Qsettings
			savePluginsEnabledSettings();

			emit(dataChanged(index, index));

			return true;
		}
    }

    return false;
}

bool DkDownloadPluginsModel::insertRows(int position, int rows, const QModelIndex &index) {

    beginInsertRows(QModelIndex(), position, position+rows-1);

    for (int row=0; row < rows; row++) {
		pluginData.insert(position, dataToInsert);
    }

    endInsertRows();
    return true;
}

bool DkDownloadPluginsModel::removeRows(int position, int rows, const QModelIndex &index) {

    beginRemoveRows(QModelIndex(), position, position+rows-1);

    for (int row=0; row < rows; row++) {
        pluginData.removeAt(position);
    }

    endRemoveRows();
    return true;
}

QList<QString> DkDownloadPluginsModel::getPluginData() {
	
	return pluginData;
}

void DkDownloadPluginsModel::setDataToInsert(QString newData) {

	dataToInsert = newData;
}

void DkDownloadPluginsModel::setEnabledData(QMap<QString, bool> enabledData) {

	pluginsEnabled = enabledData;
}

QMap<QString, bool> DkDownloadPluginsModel::getEnabledData() {

	return pluginsEnabled;
}

void DkDownloadPluginsModel::loadPluginsEnabledSettings() {

	pluginsEnabled.clear();

	QSettings settings;
	int size = settings.beginReadArray("PluginSettings/disabledPlugins");
	for (int i = 0; i < size; i++) {

		settings.setArrayIndex(i);
		pluginsEnabled.insert(settings.value("pluginId").toString(), false);
	}
	settings.endArray();
}

void DkDownloadPluginsModel::savePluginsEnabledSettings() {
	
	QSettings settings;
	settings.remove("PluginSettings/disabledPlugins");
	
	if (pluginsEnabled.size() > 0) {

		int i = 0;
		QMapIterator<QString, bool> iter(pluginsEnabled);	

		settings.beginWriteArray("PluginSettings/disabledPlugins");
		while(iter.hasNext()) {
			iter.next();
			if (!iter.value()) {
				settings.setArrayIndex(i++);
				settings.setValue("pluginId", iter.key());
			}
		}

		settings.endArray();
	}
}

/**********************************************************************************
*DkCheckBoxDelegate : delagete for checkbox only column in the model
**********************************************************************************/

static QRect CheckBoxRect(const QStyleOptionViewItem &viewItemStyleOptions) {

	QStyleOptionButton checkBoxStyleOption;
	QRect checkBoxRect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkBoxStyleOption);
	QPoint checkBoxPoint(viewItemStyleOptions.rect.x() + viewItemStyleOptions.rect.width() / 2 - checkBoxRect.width() / 2,
                         viewItemStyleOptions.rect.y() + viewItemStyleOptions.rect.height() / 2 - checkBoxRect.height() / 2);
	return QRect(checkBoxPoint, checkBoxRect.size());
}

DkCheckBoxDelegate::DkCheckBoxDelegate(QObject *parent) : QStyledItemDelegate(parent) {
	
	parentTable = static_cast<QTableView*>(parent);
}

void DkCheckBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

	if (option.state & QStyle::State_Selected) {
		if (parentTable->hasFocus()) painter->fillRect(option.rect, option.palette.highlight());
		else  painter->fillRect(option.rect, option.palette.background());
	}
	//else if (index.row() % 2 == 1) painter->fillRect(option.rect, option.palette.alternateBase());	// already done automatically

	bool checked = index.model()->data(index, Qt::DisplayRole).toBool();

	QStyleOptionButton checkBoxStyleOption;
	checkBoxStyleOption.state |= QStyle::State_Enabled;
	if (checked) {
		checkBoxStyleOption.state |= QStyle::State_On;
	} else {
		checkBoxStyleOption.state |= QStyle::State_Off;
	}
	checkBoxStyleOption.rect = CheckBoxRect(option);

	QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxStyleOption, painter);
}

bool DkCheckBoxDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
	
	if ((event->type() == QEvent::MouseButtonRelease) || (event->type() == QEvent::MouseButtonDblClick)) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
		if (mouseEvent->button() != Qt::LeftButton || !CheckBoxRect(option).contains(mouseEvent->pos())) {
			return false;
		}
		if (event->type() == QEvent::MouseButtonDblClick) {
			return true;
		}
	} 
	else if (event->type() == QEvent::KeyPress) {
		if (static_cast<QKeyEvent*>(event)->key() != Qt::Key_Space && static_cast<QKeyEvent*>(event)->key() != Qt::Key_Select) {
			return false;
		}
	} 
	else return false;

	bool checked = index.model()->data(index, Qt::DisplayRole).toBool();
	return model->setData(index, !checked, Qt::EditRole);
}



/**********************************************************************************
*DkPushButtonDelegate : delagete for uninstall column in the model
**********************************************************************************/

static QRect PushButtonRect(const QStyleOptionViewItem &viewItemStyleOptions) {

	
	QRect pushButtonRect = viewItemStyleOptions.rect;
	//pushButtonRect.setHeight(pushButtonRect.height() - 2);
	//pushButtonRect.setWidth(pushButtonRect.width() - 2);
	QPoint pushButtonPoint(viewItemStyleOptions.rect.x() + viewItemStyleOptions.rect.width() / 2 - pushButtonRect.width() / 2,
                         viewItemStyleOptions.rect.y() + viewItemStyleOptions.rect.height() / 2 - pushButtonRect.height() / 2);
	return QRect(pushButtonPoint, pushButtonRect.size());
}

DkPushButtonDelegate::DkPushButtonDelegate(QObject *parent) : QStyledItemDelegate(parent) {
	
	parentTable = static_cast<QTableView*>(parent);
	pushButonState = QStyle::State_Enabled;
	currRow = -1;
}

void DkPushButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

	if (option.state & QStyle::State_Selected) {
		if (parentTable->hasFocus()) painter->fillRect(option.rect, option.palette.highlight());
		else  painter->fillRect(option.rect, option.palette.background());
	}

	QStyleOptionButton pushButtonStyleOption;
	pushButtonStyleOption.text = index.model()->data(index, Qt::DisplayRole).toString();
	if (currRow == index.row()) pushButtonStyleOption.state = pushButonState | QStyle::State_Enabled;
	else pushButtonStyleOption.state = QStyle::State_Enabled;
	pushButtonStyleOption.rect = PushButtonRect(option);

	QApplication::style()->drawControl(QStyle::CE_PushButton, &pushButtonStyleOption, painter);
}

bool DkPushButtonDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
	
	if ((event->type() == QEvent::MouseButtonRelease) || (event->type() == QEvent::MouseButtonPress)) {

		QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
		if (mouseEvent->button() != Qt::LeftButton || !PushButtonRect(option).contains(mouseEvent->pos())) {
			pushButonState = QStyle::State_Raised;
			return false;
		}
	}
	else if (event->type() == QEvent::KeyPress) {
		if (static_cast<QKeyEvent*>(event)->key() != Qt::Key_Space && static_cast<QKeyEvent*>(event)->key() != Qt::Key_Select) {
			pushButonState = QStyle::State_Raised;
			return false;
		}
	} 
	else {
		pushButonState = QStyle::State_Raised;
		return false;
	}

    if(event->type() == QEvent::MouseButtonPress) {
		pushButonState = QStyle::State_Sunken;
		currRow = index.row();
	}
	else if( event->type() == QEvent::MouseButtonRelease) {
		pushButonState = QStyle::State_Raised;
        emit buttonClicked(index);
    }    
    return true;

}


/**********************************************************************************
*DkDownloadDelegate : icon if already downloaded or button if not
**********************************************************************************/

DkDownloadDelegate::DkDownloadDelegate(QObject *parent) : QStyledItemDelegate(parent) {
	
	parentTable = static_cast<QTableView*>(parent);
	pushButonState = QStyle::State_Enabled;
	currRow = -1;
}

void DkDownloadDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

	if (option.state & QStyle::State_Selected) {
		if (parentTable->hasFocus()) painter->fillRect(option.rect, option.palette.highlight());
		else  painter->fillRect(option.rect, option.palette.background());
	}

	if (index.model()->data(index, Qt::UserRole).toBool()) {

		QPixmap installedIcon(16, 16);
		installedIcon.load(":/nomacs/img/downloaded.png", "PNG");

		QStyleOptionViewItem iconOption = option;
		iconOption.displayAlignment = Qt::AlignCenter;
		QPoint installedIconPoint(option.rect.x() + option.rect.width() / 2 - installedIcon.width() / 2,
                         option.rect.y() + option.rect.height() / 2 - installedIcon.height() / 2);
		iconOption.rect = QRect(installedIconPoint, installedIcon.size());
		painter->drawPixmap(iconOption.rect, installedIcon);
	}
	else {

		QStyleOptionButton pushButtonStyleOption;
		pushButtonStyleOption.text = index.model()->data(index, Qt::DisplayRole).toString();
		if (currRow == index.row()) pushButtonStyleOption.state = pushButonState | QStyle::State_Enabled;
		else pushButtonStyleOption.state = QStyle::State_Enabled;
		pushButtonStyleOption.rect = PushButtonRect(option);

		QApplication::style()->drawControl(QStyle::CE_PushButton, &pushButtonStyleOption, painter);
	}
}

bool DkDownloadDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
	
	if (index.model()->data(index, Qt::UserRole).toBool()) return false;
	else {
		if ((event->type() == QEvent::MouseButtonRelease) || (event->type() == QEvent::MouseButtonPress)) {

			QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
			if (mouseEvent->button() != Qt::LeftButton || !PushButtonRect(option).contains(mouseEvent->pos())) {
				pushButonState = QStyle::State_Raised;
				return false;
			}
		}
		else if (event->type() == QEvent::KeyPress) {
			if (static_cast<QKeyEvent*>(event)->key() != Qt::Key_Space && static_cast<QKeyEvent*>(event)->key() != Qt::Key_Select) {
				pushButonState = QStyle::State_Raised;
				return false;
			}
		} 
		else {
			pushButonState = QStyle::State_Raised;
			return false;
		}

		if(event->type() == QEvent::MouseButtonPress) {
			pushButonState = QStyle::State_Sunken;
			currRow = index.row();
		}
		else if( event->type() == QEvent::MouseButtonRelease) {
			pushButonState = QStyle::State_Raised;
			emit buttonClicked(index);
		}    
	}
    return true;

}



/**********************************************************************************
* DkDescriptionEdit : text edit connected to tableView selection and models
**********************************************************************************/

DkDescriptionEdit::DkDescriptionEdit(QAbstractTableModel* data, QSortFilterProxyModel* proxy, QItemSelectionModel* selection, QWidget *parent) : QTextEdit(parent) {
	
	parentTable = static_cast<DkPluginTableWidget*>(parent);
	dataModel = data;
	proxyModel = proxy;
	selectionModel = selection;
	defaultString = QString(tr("<i>Select a table row to show the plugin description.</i>"));
	this->setText(defaultString);
	this->setReadOnly(true);
}

void DkDescriptionEdit::updateText() {

	switch(selectionModel->selection().indexes().count())
	{
		case 0:
			this->setText(defaultString);
			break;
		default:
			QString text = QString();
			int row = selectionModel->selection().indexes().first().row();
			QModelIndex sourceIndex = proxyModel->mapToSource(selectionModel->selection().indexes().first());
			QString pluginID = QString();
			if (parentTable->getOpenedTab()==tab_installed_plugins) {
				DkInstalledPluginsModel* installedPluginsModel = static_cast<DkInstalledPluginsModel*>(dataModel);
				pluginID = installedPluginsModel->getPluginData().at(sourceIndex.row());
			}
			else if (parentTable->getOpenedTab()==tab_download_plugins) {
				DkDownloadPluginsModel* downloadPluginsModel = static_cast<DkDownloadPluginsModel*>(dataModel);
				pluginID = downloadPluginsModel->getPluginData().at(sourceIndex.row());
			}
			
			if (!pluginID.isNull()) text = parentTable->getPluginManager()->getPlugins().value(pluginID)->pluginDescription();
			else text = tr("Wrong plug-in GUID!");
			this->setText(text);
			break;
	}
}

void DkDescriptionEdit::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight) {

	updateText();
}

void DkDescriptionEdit::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {

	updateText();
}


/**********************************************************************************
* DkDescriptionImage : image label connected to tableView selection and models
**********************************************************************************/

DkDescriptionImage::DkDescriptionImage(QAbstractTableModel* data, QSortFilterProxyModel* proxy, QItemSelectionModel* selection, QWidget *parent) : QLabel(parent) {
	
	parentTable = static_cast<DkPluginTableWidget*>(parent);
	dataModel = data;
	proxyModel = proxy;
	selectionModel = selection;
	defaultImage = QImage(":/nomacs/img/imgDescriptionMissing.png");
	this->setPixmap(QPixmap::fromImage(defaultImage));	
}

void DkDescriptionImage::updateImage() {

	switch(selectionModel->selection().indexes().count())
	{
		case 0:
			this->setPixmap(QPixmap::fromImage(defaultImage));	
			break;
		default:
			int row = selectionModel->selection().indexes().first().row();
			QModelIndex sourceIndex = proxyModel->mapToSource(selectionModel->selection().indexes().first());
			QString pluginID;
			if (parentTable->getOpenedTab()==tab_installed_plugins) {
				DkInstalledPluginsModel* installedPluginsModel = static_cast<DkInstalledPluginsModel*>(dataModel);
				pluginID = installedPluginsModel->getPluginData().at(sourceIndex.row());
			}
			else if (parentTable->getOpenedTab()==tab_download_plugins) {
				DkDownloadPluginsModel* downloadPluginsModel = static_cast<DkDownloadPluginsModel*>(dataModel);
				pluginID = downloadPluginsModel->getPluginData().at(sourceIndex.row());
			}
			QImage img = parentTable->getPluginManager()->getPlugins().value(pluginID)->pluginDescriptionImage();
			if (!img.isNull()) this->setPixmap(QPixmap::fromImage(img));
			else this->setPixmap(QPixmap::fromImage(defaultImage));
			break;
	}
}

void DkDescriptionImage::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight) {

	updateImage();
}

void DkDescriptionImage::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {

	updateImage();
}



};