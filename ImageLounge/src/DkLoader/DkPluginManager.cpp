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
#include "DkSettings.h"
#include "DkActionManager.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QWidget>
#include <QSlider>
#include <QSpinBox>
#include <QTextEdit>
#include <QProgressDialog>
#include <QTabWidget>
#include <QPluginLoader>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QScrollBar>
#include <QTableView>
#include <QXmlStreamReader>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QMessageBox>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QMouseEvent>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkProxyFactory>
#include <QAction>
#include <QMenu>
#pragma warning(pop)		// no warnings from includes - end

#ifdef QT_NO_DEBUG_OUTPUT
#pragma warning(disable: 4127)		// no 'conditional expression is constant' if qDebug() messages are removed
#endif

namespace nmc {

/**********************************************************************************
* Plugin manager dialog
**********************************************************************************/
DkPluginManagerDialog::DkPluginManagerDialog(QWidget* parent) : QDialog(parent) {
	init();
};

DkPluginManagerDialog::~DkPluginManagerDialog() {}

/**
* initialize plugin manager dialog - set sizes
 **/
void DkPluginManagerDialog::init() {
	
	dialogWidth = 700;
	dialogHeight = 500;

	setWindowTitle(tr("Plugin manager"));
	setMinimumSize(dialogWidth, dialogHeight);
	createLayout();
}

/*
* create plugin manager dialog layout
 **/
void DkPluginManagerDialog::createLayout() {
	
	QVBoxLayout* verticalLayout = new QVBoxLayout(this);
	tabs = new QTabWidget(this);

	tableWidgetInstalled = new DkPluginTableWidget(tab_installed_plugins, this, tabs->currentWidget());
	tabs->addTab(tableWidgetInstalled, tr("Manage installed plugins"));
	tableWidgetDownload = new DkPluginTableWidget(tab_download_plugins, this, tabs->currentWidget());
	tabs->addTab(tableWidgetDownload, tr("Download new plugins"));
	connect(tabs, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    verticalLayout->addWidget(tabs);
	
    QHBoxLayout* horizontalLayout = new QHBoxLayout(this);
    QSpacerItem* horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout->addItem(horizontalSpacer);

	QPushButton* buttonClose = new QPushButton(tr("&Close"));
	connect(buttonClose, SIGNAL(clicked()), this, SLOT(closePressed()));
	buttonClose->setDefault(true);

    horizontalLayout->addWidget(buttonClose);

    verticalLayout->addLayout(horizontalLayout);

	this->setSizeGripEnabled(false);
}


void DkPluginManagerDialog::closePressed() {

	this->close();
}

void DkPluginManagerDialog::showEvent(QShowEvent* ev) {

	qDebug() << "show event called...";
	loadPreviouslyInstalledPluginsList();
	loadPlugins();
	tableWidgetInstalled->getPluginUpdateData();

	tabs->setCurrentIndex(tab_installed_plugins);
	tableWidgetInstalled->clearTableFilters();
	tableWidgetInstalled->updateInstalledModel();
	tableWidgetDownload->clearTableFilters();

	QDialog::showEvent(ev);
}

void DkPluginManagerDialog::deleteInstance(QString id) {

	DkPluginManager::instance().removePlugin(id);
}

/**
* Loads/reloads installed plugins
 **/
void DkPluginManagerDialog::loadPlugins() {

	QStringList pluginIdList = DkPluginManager::instance().getPluginIdList();

	for (const QString& pluginId : pluginIdList) {
		DkPluginManager::instance().removePlugin(pluginId);
	}

	QStringList loadedPluginFileNames = QStringList();
	QStringList libPaths = QCoreApplication::libraryPaths();

	for (int idx = 0; idx < libPaths.size(); idx++) {
		
		QDir pluginsDir(libPaths.at(idx));
		qDebug() << "trying to load from: " << pluginsDir.absolutePath();

		for(const QString& fileName : pluginsDir.entryList(QDir::Files)) {

			QString shortFileName = fileName.split("/").last();
			if (!loadedPluginFileNames.contains(shortFileName)) { // prevent double loading of the same plugin

				//qDebug() << "close to loading:" << pluginsDir.absoluteFilePath(fileName);
				if (DkPluginManager::instance().singlePluginLoad(pluginsDir.absoluteFilePath(fileName)))
					loadedPluginFileNames.append(shortFileName);
			}
		}
	}
	
	DkPluginManager::instance().saveSettings();

}

void DkPluginManagerDialog::loadPreviouslyInstalledPluginsList() {

	previouslyInstalledPlugins = QMap<QString, QString>();

	QSettings& settings = Settings::instance().getSettings();
	int size = settings.beginReadArray("PluginSettings/filePaths");
	for (int i = 0; i < size; i++) {
		settings.setArrayIndex(i);
		previouslyInstalledPlugins.insert(settings.value("pluginId").toString(), settings.value("version").toString());
	}
	settings.endArray();
}

QMap<QString, QString> DkPluginManagerDialog::getPreviouslyInstalledPlugins() {

	return previouslyInstalledPlugins;
}

void DkPluginManagerDialog::tabChanged(int tab){
	
	if(tab == tab_installed_plugins) tableWidgetInstalled->updateInstalledModel();
	else if(tab == tab_download_plugins) tableWidgetDownload->downloadPluginInformation(xml_usage_download);
}

void DkPluginManagerDialog::deletePlugin(QString pluginID) {

	QFile file(DkPluginManager::instance().getPluginFilePath(pluginID));
	DkPluginManager::instance().removePlugin(pluginID);

	if (!file.remove()) {
		qDebug() << "Failed to delete plugin file!";
		QMessageBox::critical(this, tr("Plugin manager"), tr("The dll could not be deleted!\nPlease restart nomacs and try again."));
	}
	else
		DkPluginManager::instance().removePlugin(pluginID);
}

/**********************************************************************************
 * DkPluginTableWidget : Widget with table views containing plugin data
 **********************************************************************************/

DkPluginTableWidget::DkPluginTableWidget(int tab, DkPluginManagerDialog* manager, QWidget* parent) : QWidget(parent) {

	mPluginDownloader = new DkPluginDownloader(this);
	mOpenedTab = tab;
	mPluginManager = manager;
	createLayout();
}

DkPluginTableWidget::~DkPluginTableWidget() {

}


// create the main layout of the plugin manager
void DkPluginTableWidget::createLayout() {

	QVBoxLayout* verticalLayout = new QVBoxLayout(this);

	// search line edit and update button
	QHBoxLayout* searchHorLayout = new QHBoxLayout();
	QLabel* searchLabel = new QLabel(tr("&Search plugins: "), this);
	searchHorLayout->addWidget(searchLabel);
	mFilterEdit = new QLineEdit(this);
	mFilterEdit->setFixedSize(160,20);
	connect(mFilterEdit, SIGNAL(textChanged(QString)), this, SLOT(filterTextChanged()));
	searchLabel->setBuddy(mFilterEdit);
	searchHorLayout->addWidget(mFilterEdit);
	QSpacerItem* horizontalSpacer;
	if(mOpenedTab == tab_installed_plugins) 
		horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	else 
		horizontalSpacer = new QSpacerItem(40, 23, QSizePolicy::Expanding, QSizePolicy::Minimum);
	searchHorLayout->addItem(horizontalSpacer);
	
	if(mOpenedTab == tab_installed_plugins) {
		mUpdateButton = new QPushButton("", this);
		connect(mUpdateButton, SIGNAL(clicked()),this, SLOT(updateSelectedPlugins()));
		mUpdateButton->setFixedWidth(140);
		searchHorLayout->addWidget(mUpdateButton);
	}
	verticalLayout->addLayout(searchHorLayout);

	// main table
    mTableView = new QTableView(this);
	mProxyModel = new QSortFilterProxyModel(this);
    mProxyModel->setDynamicSortFilter(true);
	//tableView->setMaximumHeight(100);
	if(mOpenedTab == tab_installed_plugins) 
		mModel = new DkInstalledPluginsModel(DkPluginManager::instance().getPluginIdList(), this);
	else if (mOpenedTab == tab_download_plugins) 
		mModel = new DkDownloadPluginsModel(this);
	mProxyModel->setSourceModel(mModel);
	mTableView->setModel(mProxyModel);
	mTableView->resizeColumnsToContents();
	if(mOpenedTab == tab_installed_plugins) {
		mTableView->setColumnWidth(ip_column_name, qMax(mTableView->columnWidth(ip_column_name), 300));
		mTableView->setColumnWidth(ip_column_version, qMax(mTableView->columnWidth(ip_column_version), 80));
		mTableView->setColumnWidth(ip_column_enabled, qMax(mTableView->columnWidth(ip_column_enabled), 130));
	} else if (mOpenedTab == tab_download_plugins) {
		mTableView->setColumnWidth(dp_column_name, qMax(mTableView->columnWidth(dp_column_name), 360));
		mTableView->setColumnWidth(dp_column_version, qMax(mTableView->columnWidth(dp_column_version), 80));
		//tableView->setColumnWidth(ip_column_enabled, qMax(tableView->columnWidth(ip_column_enabled), 130));
	}

	mTableView->resizeRowsToContents();	
    mTableView->horizontalHeader()->setStretchLastSection(true);
    mTableView->setSortingEnabled(true);
	mTableView->sortByColumn(ip_column_name, Qt::AscendingOrder);
    mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    mTableView->verticalHeader()->hide();
    mTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mTableView->setSelectionMode(QAbstractItemView::SingleSelection);
	mTableView->setAlternatingRowColors(true);
	//tableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	if(mOpenedTab == tab_installed_plugins) {
		mTableView->setItemDelegateForColumn(ip_column_enabled, new DkPluginCheckBoxDelegate(mTableView));
		DkPushButtonDelegate* buttonDelegate = new DkPushButtonDelegate(mTableView);
		mTableView->setItemDelegateForColumn(ip_column_uninstall, buttonDelegate);
		connect(buttonDelegate, SIGNAL(buttonClicked(QModelIndex)), this, SLOT(uninstallPlugin(QModelIndex)));
		connect(mPluginDownloader, SIGNAL(allPluginsUpdated(bool)), this, SLOT(pluginUpdateFinished(bool)));
		connect(mPluginDownloader, SIGNAL(reloadPlugins()), this, SLOT(reloadPlugins()));
	} else if (mOpenedTab == tab_download_plugins) {
		DkDownloadDelegate* buttonDelegate = new DkDownloadDelegate(mTableView);
		mTableView->setItemDelegateForColumn(dp_column_install, buttonDelegate);
		connect(buttonDelegate, SIGNAL(buttonClicked(QModelIndex)), this, SLOT(installPlugin(QModelIndex)));
		connect(mPluginDownloader, SIGNAL(pluginDownloaded(const QModelIndex &)), this, SLOT(pluginInstalled(const QModelIndex &)));
	}
    verticalLayout->addWidget(mTableView);

	QSpacerItem* verticalSpacer = new QSpacerItem(40, 15, QSizePolicy::Expanding, QSizePolicy::Minimum);
	verticalLayout->addItem(verticalSpacer);

	QVBoxLayout* bottomVertLayout = new QVBoxLayout();

	// additional information
	QHBoxLayout* topHorLayout = new QHBoxLayout();
	QLabel* descLabel = new QLabel(tr("Plugin description:"));
	topHorLayout->addWidget(descLabel);
	QSpacerItem* horizontalSpacer1 = new QSpacerItem(300, 1, QSizePolicy::Expanding, QSizePolicy::Minimum);
	topHorLayout->addItem(horizontalSpacer1);
	QLabel* previewLabel = new QLabel(tr("Plugin preview:"));
	topHorLayout->addWidget(previewLabel);
	QSpacerItem* horizontalSpacer2 = new QSpacerItem(250, 1, QSizePolicy::Minimum, QSizePolicy::Minimum);
	topHorLayout->addItem(horizontalSpacer2);
	bottomVertLayout->addLayout(topHorLayout);

	QHBoxLayout* bottHorLayout = new QHBoxLayout();

	//QTextEdit* decriptionEdit = new QTextEdit();
	//decriptionEdit->setReadOnly(true);
	//bottHorLayout->addWidget(decriptionEdit);
	DkDescriptionEdit* decriptionEdit = new DkDescriptionEdit(mModel, mProxyModel, mTableView->selectionModel(), this);
	connect(mTableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), decriptionEdit, SLOT(selectionChanged(const QItemSelection &, const QItemSelection &)));
	connect(mProxyModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), decriptionEdit, SLOT(dataChanged(const QModelIndex &, const QModelIndex &)));
	bottHorLayout->addWidget(decriptionEdit);

	//QLabel* imgLabel = new QLabel();
	//imgLabel->setMinimumWidth(layoutHWidth/2 + 4);
	//imgLabel->setPixmap(QPixmap::fromImage(QImage(":/nomacs/img/imgDescriptionMissing.png")));	
	DkDescriptionImage* decriptionImg = new DkDescriptionImage(mModel, mProxyModel, mTableView->selectionModel(), this);
	//decriptionImg->setMinimumWidth(324);//layoutHWidth/2 + 4);
	//decriptionImg->setMaximumWidth(324);//layoutHWidth/2 + 4);
	decriptionImg->setMaximumSize(324,168);
	decriptionImg->setMinimumSize(324,168);
	connect(mTableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), decriptionImg, SLOT(selectionChanged(const QItemSelection &, const QItemSelection &)));
	connect(mProxyModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), decriptionImg, SLOT(dataChanged(const QModelIndex &, const QModelIndex &)));
	connect(mPluginDownloader, SIGNAL(imageDownloaded(QImage)), decriptionImg, SLOT(updateImageFromReply(QImage)));
	bottHorLayout->addWidget(decriptionImg);
	bottomVertLayout->addLayout(bottHorLayout);

	verticalLayout->addLayout(bottomVertLayout);
}

void DkPluginTableWidget::getPluginUpdateData() {

	downloadPluginInformation(xml_usage_update);
}

void DkPluginTableWidget::reloadPlugins() {

	mPluginManager->loadPlugins();
	updateInstalledModel();
}

void DkPluginTableWidget::manageParsedXmlData(int usage) {

	if (usage == xml_usage_download) fillDownloadTable();
	else if (usage == xml_usage_update) 
		getListOfUpdates();
	else 
		showDownloaderMessage(tr("Sorry, too many connections at once."), tr("Plugin manager"));
}

void DkPluginTableWidget::getListOfUpdates() {

	DkInstalledPluginsModel* installedPluginsModel = static_cast<DkInstalledPluginsModel*>(mModel);
	QList<QString> installedIdList = installedPluginsModel->getPluginData();
	QList<XmlPluginData> updateList = mPluginDownloader->getXmlPluginData();
	QMap<QString, QString> previouslyInstalledPlugins = mPluginManager->getPreviouslyInstalledPlugins();
	QMap<QString, QString> installedPluginsList = previouslyInstalledPlugins;
	mPluginsToUpdate = QList<XmlPluginData>();

	for (int i = 0; i < installedIdList.size(); i++)
		if (!installedPluginsList.contains(installedIdList.at(i)))
			installedPluginsList.insert(installedIdList.at(i), DkPluginManager::instance().getPlugins().value(installedIdList.at(i))->pluginVersion());
	
	mUpdateButton->setEnabled(false);
	mUpdateButton->setText(tr("Plugins up to date"));
	mUpdateButton->setToolTip(tr("No available updates."));

	for (int i = 0; i < updateList.size(); i++) {
		QMapIterator<QString, QString> iter(installedPluginsList);
		while (iter.hasNext()) {
			iter.next();
			if(updateList.at(i).id == iter.key()) {
				QStringList cVersion = iter.value().split('.');
				QStringList nVersion = updateList.at(i).version.split('.');

				if (cVersion.size() < 3 || nVersion.size() < 3) {
					qDebug() << "sorry, I could not parse the plugin version number...";
					break;
				}

				if (nVersion[0].toInt() > cVersion[0].toInt()  ||	// major release
					(nVersion[0].toInt() == cVersion[0].toInt() &&	// major release
					nVersion[1].toInt() > cVersion[1].toInt())  ||	// minor release
					(nVersion[0].toInt() == cVersion[0].toInt() &&	// major release
					nVersion[1].toInt() == cVersion[1].toInt() &&	// minor release
					nVersion[2].toInt() >  cVersion[2].toInt())) {	// minor-minor release
						mPluginsToUpdate.append(updateList.at(i));
				}
				break;
			}
		}
	}

	if (mPluginsToUpdate.size() > 0) {
		QStringList pluginsNames = QStringList();
		mUpdateButton->setText(tr("Available updates: %1").arg(mPluginsToUpdate.size()));
		mUpdateButton->setEnabled(true);
		for (int i = 0; i < mPluginsToUpdate.size(); i++) pluginsNames.append(mPluginsToUpdate.at(i).name + "    v:" + mPluginsToUpdate.at(i).version);
		mUpdateButton->setToolTip(tr("Updates:<br><i>%1</i>").arg(pluginsNames.join("<br>")));
	}
}

void DkPluginTableWidget::updateSelectedPlugins() {

	mUpdateButton->setText(tr("Plugins up to date"));
	mUpdateButton->setToolTip(tr("No available updates."));
	mUpdateButton->setEnabled(false);

	if (mPluginsToUpdate.size() > 0) {
		QList<QString> updatedList = DkPluginManager::instance().getPluginIdList();
		for (int i = 0; i < mPluginsToUpdate.size(); i++) {
			for (int j = updatedList.size() - 1; j >= 0; j--) 
				if (updatedList.at(j) == mPluginsToUpdate.at(i).id) {
					updatedList.removeAt(j);
					break;
			}
		}
		DkPluginManager::instance().setPluginIdList(updatedList);
		updateInstalledModel(); // before deleting instance remove entries from table

		for (int i = 0; i < mPluginsToUpdate.size(); i++) 
			mPluginManager->deleteInstance(mPluginsToUpdate.at(i).id);

		// after deleting instances the file are not in use anymore -> update
		QList<QString> urls = QList<QString>();
		while (mPluginsToUpdate.size() > 0) {
			XmlPluginData pluginData = mPluginsToUpdate.takeLast();
			QString downloadFileListUrl = "http://www.nomacs.org/plugins-download/" + pluginData.id + "/" + pluginData.version + "/d.txt";
			urls.append(downloadFileListUrl);
		}

		mPluginDownloader->updatePlugins(urls);

		mPluginManager->loadPlugins();
	}
}

void DkPluginTableWidget::pluginUpdateFinished(bool finishedSuccessfully) {

	mPluginManager->loadPlugins();
	updateInstalledModel();
	if (finishedSuccessfully) showDownloaderMessage(tr("The plugins have been updated."), tr("Plugin manager"));
}

void DkPluginTableWidget::filterTextChanged() {

	QRegExp regExp(mFilterEdit->text(), Qt::CaseInsensitive, QRegExp::FixedString);
	mProxyModel->setFilterRegExp(regExp);
	mTableView->resizeRowsToContents();
}

void DkPluginTableWidget::uninstallPlugin(const QModelIndex &index) {

	DkInstalledPluginsModel* installedPluginsModel = static_cast<DkInstalledPluginsModel*>(mModel);
	int selectedRow = mProxyModel->mapToSource(index).row();
	QString pluginID = installedPluginsModel->getPluginData().at(selectedRow);

	QMessageBox msgBox;
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::No);
	msgBox.setEscapeButton(QMessageBox::No);
	msgBox.setIcon(QMessageBox::Question);
	msgBox.setWindowTitle(tr("Uninstall plugins"));

	msgBox.setText(tr("Do you really want to uninstall the plugin <i>%1</i>?").arg(DkPluginManager::instance().getPlugins().value(pluginID)->pluginName()));

	if(msgBox.exec() == QMessageBox::Yes) {

		QMap<QString, bool> enabledSettings = installedPluginsModel->getEnabledData();
		QList<QString> pluginIdList = installedPluginsModel->getPluginData();	
		pluginIdList.removeAt(selectedRow);			
		enabledSettings.remove(pluginID);

		DkPluginManager::instance().setPluginIdList(pluginIdList);
		installedPluginsModel->setEnabledData(enabledSettings);
		installedPluginsModel->savePluginsEnabledSettings();
		updateInstalledModel();	// !!! update model before deleting the interface

		mPluginManager->deletePlugin(pluginID);
	}
	msgBox.deleteLater();
}

void DkPluginTableWidget::installPlugin(const QModelIndex &index) {

	DkDownloadPluginsModel* downloadPluginsModel = static_cast<DkDownloadPluginsModel*>(mModel);
	QModelIndex sourceIndex = mProxyModel->mapToSource(index);
	int selectedRow = sourceIndex.row();
	
	QString downloadFileListUrl = "http://www.nomacs.org/plugins-download/" + downloadPluginsModel->getPluginData().at(selectedRow).id 
		+ "/" + downloadPluginsModel->getPluginData().at(selectedRow).version + "/d.txt";

	//QDir pluginsDir = QDir(qApp->applicationDirPath());
    //pluginsDir.mkdir("plugins");
	QDir pluginsDir = Settings::param().global().pluginsDir;
	
	if (!pluginsDir.exists())
		pluginsDir.mkpath(pluginsDir.absolutePath());

	qDebug() << "install path: " << Settings::param().global().pluginsDir;

	mPluginDownloader->downloadPlugin(sourceIndex, downloadFileListUrl, downloadPluginsModel->getPluginData().at(selectedRow).name);	
}

void DkPluginTableWidget::pluginInstalled(const QModelIndex &index) {

	DkDownloadPluginsModel* downloadPluginsModel = static_cast<DkDownloadPluginsModel*>(mModel);
	downloadPluginsModel->updateInstalledData(index, true);
	mPluginManager->loadPlugins();

	qDebug() << "plugin saved to: " << Settings::param().global().pluginsDir;
}

void DkPluginTableWidget::clearTableFilters(){

	mFilterEdit->clear();
	mFilterEdit->setFocus();
}


//update models if new plugins are installed or copied into the folder
void DkPluginTableWidget::updateInstalledModel() {

	DkInstalledPluginsModel* installedPluginsModel = static_cast<DkInstalledPluginsModel*>(mModel);
	installedPluginsModel->loadPluginsEnabledSettings();

	QList<QString> newPluginList = DkPluginManager::instance().getPluginIdList();
	QList<QString> tableList = installedPluginsModel->getPluginData();

	for (int i = tableList.size() - 1; i >= 0; i--) {
		if (!newPluginList.contains(tableList.at(i))) installedPluginsModel->removeRows(i, 1);
	}

	tableList = installedPluginsModel->getPluginData();

	for (int i = newPluginList.size() - 1; i >= 0; i--) {
		if (!tableList.contains(newPluginList.at(i))) {
			installedPluginsModel->setDataToInsert(newPluginList.at(i));
			installedPluginsModel->insertRows(installedPluginsModel->getPluginData().size(), 1);
		}
	}

	mTableView->resizeRowsToContents();
}

void DkPluginTableWidget::downloadPluginInformation(int usage) {

	mPluginDownloader->downloadXml(usage);
}

void DkPluginTableWidget::fillDownloadTable() {

	QList<XmlPluginData> xmlPluginData = mPluginDownloader->getXmlPluginData();
	DkDownloadPluginsModel* downloadPluginsModel = static_cast<DkDownloadPluginsModel*>(mModel);
	QList<XmlPluginData> modelData = downloadPluginsModel->getPluginData();
	QMap<QString, bool> installedPlugins = downloadPluginsModel->getInstalledData();

	// delete rows if needed
	if (modelData.size() > 0) {
		for (int i = modelData.size() - 1; i >= 0; i--) {
			int j;
			for (j = 0; j < xmlPluginData.size(); j++) if (xmlPluginData.at(j).id == modelData.at(i).id && xmlPluginData.at(j).decription == modelData.at(i).decription
				&& xmlPluginData.at(j).previewImgUrl == modelData.at(i).previewImgUrl && xmlPluginData.at(j).name == modelData.at(i).name &&
				xmlPluginData.at(j).version == modelData.at(i).version) break;
			if (j >= xmlPluginData.size()) downloadPluginsModel->removeRows(i, 1);
		}
	}

	modelData = downloadPluginsModel->getPluginData(); //refresh model data after removing rows

	// insert new rows if needed
	if (modelData.size() > 0) {
		for (int i = 0; i < xmlPluginData.size(); i++) {
			int j;
			for (j = 0; j < modelData.size(); j++) if (xmlPluginData.at(i).id == modelData.at(j).id) break;
			if (j >= modelData.size()) {
				downloadPluginsModel->setDataToInsert(xmlPluginData.at(i));
				downloadPluginsModel->insertRows(downloadPluginsModel->getPluginData().size(), 1);
			}
		}
	}
	else {
		for (int i = 0; i < xmlPluginData.size(); i++) {
			downloadPluginsModel->setDataToInsert(xmlPluginData.at(i));
			downloadPluginsModel->insertRows(downloadPluginsModel->getPluginData().size(), 1);
		}
	}

	// check if installed
	modelData.clear();
	modelData = downloadPluginsModel->getPluginData();	
	QList<QString> pluginIdList = DkPluginManager::instance().getPluginIdList();

	for (int i = 0; i < modelData.size(); i++) {
		if (pluginIdList.contains(modelData.at(i).id)) downloadPluginsModel->updateInstalledData(downloadPluginsModel->index(i, dp_column_install), true);
		else downloadPluginsModel->updateInstalledData(downloadPluginsModel->index(i, dp_column_install), false);
	}

	mTableView->resizeRowsToContents();
}

void DkPluginTableWidget::showDownloaderMessage(QString msg, QString title) {
	QMessageBox infoDialog(this);
	infoDialog.setWindowTitle(title);
	infoDialog.setIcon(QMessageBox::Information);
	infoDialog.setText(msg);
	infoDialog.show();

	infoDialog.exec();
}

DkPluginManagerDialog* DkPluginTableWidget::getPluginManager() {

	return mPluginManager;
}

int DkPluginTableWidget::getOpenedTab() {

	return mOpenedTab;
}

DkPluginDownloader* DkPluginTableWidget::getDownloader() {

	return mPluginDownloader;
}

//**********************************************************************************
//DkInstalledPluginsModel : Model managing installed plugins data in the table
//**********************************************************************************

DkInstalledPluginsModel::DkInstalledPluginsModel(QObject *parent) : QAbstractTableModel(parent) {

}

DkInstalledPluginsModel::DkInstalledPluginsModel(QList<QString> data, QObject *parent) : QAbstractTableModel(parent) {

	mParentTable = static_cast<DkPluginTableWidget*>(parent);

	mPluginData = data;
	mPluginsEnabled = QMap<QString, bool>();

	loadPluginsEnabledSettings();
}

int DkInstalledPluginsModel::rowCount(const QModelIndex&) const {

	return mPluginData.size();
}

int DkInstalledPluginsModel::columnCount(const QModelIndex&) const {

	return ip_column_size;
}

QVariant DkInstalledPluginsModel::data(const QModelIndex &index, int role) const {

    if (!index.isValid()) return QVariant();

    if (index.row() >= mPluginData.size() || index.row() < 0) return QVariant();

    if (role == Qt::DisplayRole) {
		
		QString pluginID = mPluginData.at(index.row());
		QList<QString> fsd = DkPluginManager::instance().getPluginIdList();
        if (index.column() == ip_column_name) {
			return DkPluginManager::instance().getPlugins().value(pluginID)->pluginName();
		}
        else if (index.column() == ip_column_version) {
			return DkPluginManager::instance().getPlugins().value(pluginID)->pluginVersion();
		}
        else if (index.column() == ip_column_enabled)
			return mPluginsEnabled.value(pluginID, true);
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
				return tr("Uninstall plugin");
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
			mPluginsEnabled.insert(mPluginData.at(index.row()), value.toBool());
			savePluginsEnabledSettings();

			emit(dataChanged(index, index));

			return true;
		}
    }

    return false;
}

bool DkInstalledPluginsModel::insertRows(int position, int rows, const QModelIndex&) {

    beginInsertRows(QModelIndex(), position, position+rows-1);

    for (int row=0; row < rows; row++) {
		mPluginData.insert(position, mDataToInsert);
    }

    endInsertRows();
    return true;
}

bool DkInstalledPluginsModel::removeRows(int position, int rows, const QModelIndex&) {

    beginRemoveRows(QModelIndex(), position, position+rows-1);

	for (int row=0; row < rows; row++) {
        mPluginData.removeAt(position+row);
    }

    endRemoveRows();
    return true;
}

QList<QString> DkInstalledPluginsModel::getPluginData() {
	
	return mPluginData;
}

void DkInstalledPluginsModel::setDataToInsert(QString newData) {

	mDataToInsert = newData;
}

void DkInstalledPluginsModel::setEnabledData(QMap<QString, bool> enabledData) {

	mPluginsEnabled = enabledData;
}

QMap<QString, bool> DkInstalledPluginsModel::getEnabledData() {

	return mPluginsEnabled;
}

void DkInstalledPluginsModel::loadPluginsEnabledSettings() {

	mPluginsEnabled.clear();

	QSettings& settings = Settings::instance().getSettings();
	int size = settings.beginReadArray("PluginSettings/disabledPlugins");
	for (int i = 0; i < size; i++) {

		settings.setArrayIndex(i);
		mPluginsEnabled.insert(settings.value("pluginId").toString(), false);
	}
	settings.endArray();
}

void DkInstalledPluginsModel::savePluginsEnabledSettings() {
	
	QSettings& settings = Settings::instance().getSettings();
	settings.remove("PluginSettings/disabledPlugins");
	
	if (mPluginsEnabled.size() > 0) {

		int i = 0;
		QMapIterator<QString, bool> iter(mPluginsEnabled);	

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


//**********************************************************************************
//DkDownloadPluginsModel : Model managing the download plug.in data
//**********************************************************************************

DkDownloadPluginsModel::DkDownloadPluginsModel(QObject *parent) : QAbstractTableModel(parent) {

	mParentTable = static_cast<DkPluginTableWidget*>(parent);

	mPluginData = QList<XmlPluginData>();
	mPluginsInstalled = QMap<QString, bool>();
}

int DkDownloadPluginsModel::rowCount(const QModelIndex&) const {

	return mPluginData.size();
}

int DkDownloadPluginsModel::columnCount(const QModelIndex&) const {

	return dp_column_size;
}

QVariant DkDownloadPluginsModel::data(const QModelIndex &index, int role) const {

    if (!index.isValid()) return QVariant();

    if (index.row() >= mPluginData.size() || index.row() < 0) return QVariant();

    if (role == Qt::DisplayRole) {
 
		if (index.column() == dp_column_name) {
			return mPluginData.at(index.row()).name;
		}
        else if (index.column() == dp_column_version) {
			return mPluginData.at(index.row()).version;
		}
		else if (index.column() == dp_column_install)
			return QString(tr("Download and Install"));
    }

	if (role == Qt::UserRole) {
		return mPluginsInstalled.value(mPluginData.at(index.row()).id, false);
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
				return tr("Download and install plugin");
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

bool DkDownloadPluginsModel::setData(const QModelIndex&, const QVariant&, int) {

    return false;
}

bool DkDownloadPluginsModel::insertRows(int position, int rows, const QModelIndex&) {

    beginInsertRows(QModelIndex(), position, position+rows-1);

    for (int row=0; row < rows; row++) {
		mPluginData.insert(position, mDataToInsert);
    }

    endInsertRows();
    return true;
}

bool DkDownloadPluginsModel::removeRows(int position, int rows, const QModelIndex&) {

    beginRemoveRows(QModelIndex(), position, position+rows-1);

    for (int row=0; row < rows; row++) {
        mPluginData.removeAt(position+row);
    }

    endRemoveRows();
    return true;
}

void DkDownloadPluginsModel::updateInstalledData(const QModelIndex &index, bool installed) {

	if(installed) mPluginsInstalled.insert(mPluginData.at(index.row()).id, true);
	else mPluginsInstalled.remove(mPluginData.at(index.row()).id);
	emit dataChanged(index, index);
}

QList<XmlPluginData> DkDownloadPluginsModel::getPluginData() {
	
	return mPluginData;
}

void DkDownloadPluginsModel::setDataToInsert(XmlPluginData newData) {

	mDataToInsert = newData;
}

void DkDownloadPluginsModel::setInstalledData(QMap<QString, bool> installedData) {

	mPluginsInstalled = installedData;
}

QMap<QString, bool> DkDownloadPluginsModel::getInstalledData() {

	return mPluginsInstalled;
}

//*********************************************************************************
//DkCheckBoxDelegate : delagete for checkbox only column in the model
//*********************************************************************************

static QRect CheckBoxRect(const QStyleOptionViewItem &viewItemStyleOptions) {

	QStyleOptionButton checkBoxStyleOption;
	QRect checkBoxRect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkBoxStyleOption);
	QPoint checkBoxPoint(viewItemStyleOptions.rect.x() + viewItemStyleOptions.rect.width() / 2 - checkBoxRect.width() / 2,
                         viewItemStyleOptions.rect.y() + viewItemStyleOptions.rect.height() / 2 - checkBoxRect.height() / 2);
	return QRect(checkBoxPoint, checkBoxRect.size());
}

DkPluginCheckBoxDelegate::DkPluginCheckBoxDelegate(QObject *parent) : QStyledItemDelegate(parent) {
	
	mParentTable = static_cast<QTableView*>(parent);
}

void DkPluginCheckBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

	if (option.state & QStyle::State_Selected) {
		if (mParentTable->hasFocus()) painter->fillRect(option.rect, option.palette.highlight());
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

bool DkPluginCheckBoxDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
	
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



//*********************************************************************************
//DkPushButtonDelegate : delagete for uninstall column in the model
//*********************************************************************************

static QRect PushButtonRect(const QStyleOptionViewItem &viewItemStyleOptions) {

	
	QRect pushButtonRect = viewItemStyleOptions.rect;
	//pushButtonRect.setHeight(pushButtonRect.height() - 2);
	//pushButtonRect.setWidth(pushButtonRect.width() - 2);
	QPoint pushButtonPoint(viewItemStyleOptions.rect.x() + viewItemStyleOptions.rect.width() / 2 - pushButtonRect.width() / 2,
                         viewItemStyleOptions.rect.y() + viewItemStyleOptions.rect.height() / 2 - pushButtonRect.height() / 2);
	return QRect(pushButtonPoint, pushButtonRect.size());
}

DkPushButtonDelegate::DkPushButtonDelegate(QObject *parent) : QStyledItemDelegate(parent) {
	
	mParentTable = static_cast<QTableView*>(parent);
}

void DkPushButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

	if (option.state & QStyle::State_Selected) {
		if (mParentTable->hasFocus()) painter->fillRect(option.rect, option.palette.highlight());
		else  painter->fillRect(option.rect, option.palette.background());
	}

	QStyleOptionButton pushButtonStyleOption;
	pushButtonStyleOption.text = index.model()->data(index, Qt::DisplayRole).toString();
	if (mCRow == index.row()) pushButtonStyleOption.state = mPushButonState | QStyle::State_Enabled;
	else pushButtonStyleOption.state = QStyle::State_Enabled;
	pushButtonStyleOption.rect = PushButtonRect(option);

	QApplication::style()->drawControl(QStyle::CE_PushButton, &pushButtonStyleOption, painter);
}

bool DkPushButtonDelegate::editorEvent(QEvent *event, QAbstractItemModel*, const QStyleOptionViewItem &option, const QModelIndex &index) {
	
	if ((event->type() == QEvent::MouseButtonRelease) || (event->type() == QEvent::MouseButtonPress)) {

		QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
		if (mouseEvent->button() != Qt::LeftButton || !PushButtonRect(option).contains(mouseEvent->pos())) {
			mPushButonState = QStyle::State_Raised;
			return false;
		}
	}
	else if (event->type() == QEvent::KeyPress) {
		if (static_cast<QKeyEvent*>(event)->key() != Qt::Key_Space && static_cast<QKeyEvent*>(event)->key() != Qt::Key_Select) {
			mPushButonState = QStyle::State_Raised;
			return false;
		}
	} 
	else {
		mPushButonState = QStyle::State_Raised;
		return false;
	}

    if(event->type() == QEvent::MouseButtonPress) {
		mPushButonState = QStyle::State_Sunken;
		mCRow = index.row();
	}
	else if( event->type() == QEvent::MouseButtonRelease) {
		mPushButonState = QStyle::State_Raised;
        emit buttonClicked(index);
    }    
    return true;

}


//**********************************************************************************
//DkDownloadDelegate : icon if already downloaded or button if not
//**********************************************************************************

DkDownloadDelegate::DkDownloadDelegate(QObject *parent) : QStyledItemDelegate(parent) {
	
	mParentTable = static_cast<QTableView*>(parent);
}

void DkDownloadDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

	if (option.state & QStyle::State_Selected) {
		if (mParentTable->hasFocus()) painter->fillRect(option.rect, option.palette.highlight());
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
		if (mCRow == index.row()) pushButtonStyleOption.state = mPushButtonState | QStyle::State_Enabled;
		else pushButtonStyleOption.state = QStyle::State_Enabled;
		pushButtonStyleOption.rect = PushButtonRect(option);

		QApplication::style()->drawControl(QStyle::CE_PushButton, &pushButtonStyleOption, painter);
	}
}

bool DkDownloadDelegate::editorEvent(QEvent *event, QAbstractItemModel*, const QStyleOptionViewItem &option, const QModelIndex &index) {
	
	if (index.model()->data(index, Qt::UserRole).toBool()) return false;
	else {
		if ((event->type() == QEvent::MouseButtonRelease) || (event->type() == QEvent::MouseButtonPress)) {

			QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
			if (mouseEvent->button() != Qt::LeftButton || !PushButtonRect(option).contains(mouseEvent->pos())) {
				mPushButtonState = QStyle::State_Raised;
				return false;
			}
		}
		else if (event->type() == QEvent::KeyPress) {
			if (static_cast<QKeyEvent*>(event)->key() != Qt::Key_Space && static_cast<QKeyEvent*>(event)->key() != Qt::Key_Select) {
				mPushButtonState = QStyle::State_Raised;
				return false;
			}
		} 
		else {
			mPushButtonState = QStyle::State_Raised;
			return false;
		}

		if(event->type() == QEvent::MouseButtonPress) {
			mPushButtonState = QStyle::State_Sunken;
			mCRow = index.row();
		}
		else if( event->type() == QEvent::MouseButtonRelease) {
			mPushButtonState = QStyle::State_Raised;
			emit buttonClicked(index);
		}    
	}
    return true;

}



//**********************************************************************************
//DkDescriptionEdit : text edit connected to tableView selection and models
//**********************************************************************************

DkDescriptionEdit::DkDescriptionEdit(QAbstractTableModel* data, QSortFilterProxyModel* proxy, QItemSelectionModel* selection, QWidget *parent) : QTextEdit(parent) {
	
	mParentTable = static_cast<DkPluginTableWidget*>(parent);
	mDataModel = data;
	mProxyModel = proxy;
	mSelectionModel = selection;
	mDefaultString = QString(tr("<i>Select a table row to show the plugin description.</i>"));
	this->setText(mDefaultString);
	this->setReadOnly(true);
}

void DkDescriptionEdit::updateText() {

	switch(mSelectionModel->selection().indexes().count())
	{
		case 0:
			this->setText(mDefaultString);
			break;
		default:
			QString text = QString();
			QModelIndex sourceIndex = mProxyModel->mapToSource(mSelectionModel->selection().indexes().first());
			QString pluginID = QString();
			if (mParentTable->getOpenedTab()==tab_installed_plugins) {
				DkInstalledPluginsModel* installedPluginsModel = static_cast<DkInstalledPluginsModel*>(mDataModel);
				pluginID = installedPluginsModel->getPluginData().at(sourceIndex.row());
				if (!pluginID.isNull()) 
					text = DkPluginManager::instance().getPlugins().value(pluginID)->pluginDescription();
			}
			else if (mParentTable->getOpenedTab()==tab_download_plugins) {
				DkDownloadPluginsModel* downloadPluginsModel = static_cast<DkDownloadPluginsModel*>(mDataModel);
				text = downloadPluginsModel->getPluginData().at(sourceIndex.row()).decription;
			}		
			
			if (text.isNull()) text = tr("Wrong plugin GUID!");
			this->setText(text);
			break;
	}
}

void DkDescriptionEdit::dataChanged(const QModelIndex&, const QModelIndex&) {

	updateText();
}

void DkDescriptionEdit::selectionChanged(const QItemSelection&, const QItemSelection&) {

	updateText();
}


//**********************************************************************************
//DkDescriptionImage : image label connected to tableView selection and models
//**********************************************************************************

DkDescriptionImage::DkDescriptionImage(QAbstractTableModel* data, QSortFilterProxyModel* proxy, QItemSelectionModel* selection, QWidget *parent) : QLabel(parent) {
	
	mParentTable = static_cast<DkPluginTableWidget*>(parent);
	mDataModel = data;
	mProxyModel = proxy;
	mSelectionModel = selection;
	mDefaultImage = QImage(":/nomacs/img/plugin-banner.svg");
	this->setPixmap(QPixmap::fromImage(mDefaultImage));	
}

void DkDescriptionImage::updateImage() {

	switch(mSelectionModel->selection().indexes().count())
	{
		case 0:
			this->setPixmap(QPixmap::fromImage(mDefaultImage));	
			break;
		default:
			QModelIndex sourceIndex = mProxyModel->mapToSource(mSelectionModel->selection().indexes().first());
			QString pluginID;
			QImage img;
			if (mParentTable->getOpenedTab()==tab_installed_plugins) {
				DkInstalledPluginsModel* installedPluginsModel = static_cast<DkInstalledPluginsModel*>(mDataModel);
				pluginID = installedPluginsModel->getPluginData().at(sourceIndex.row());
				img = DkPluginManager::instance().getPlugins().value(pluginID)->pluginDescriptionImage();
				if (!img.isNull()) this->setPixmap(QPixmap::fromImage(img));
				else this->setPixmap(QPixmap::fromImage(mDefaultImage));
			}
			else if (mParentTable->getOpenedTab()==tab_download_plugins) {
				DkDownloadPluginsModel* downloadPluginsModel = static_cast<DkDownloadPluginsModel*>(mDataModel);
				mParentTable->getDownloader()->downloadPreviewImg(downloadPluginsModel->getPluginData().at(sourceIndex.row()).previewImgUrl);
			}
			break;
	}
}

void DkDescriptionImage::updateImageFromReply(QImage img) {
	
	if (!img.isNull()) this->setPixmap(QPixmap::fromImage(img));
	else this->setPixmap(QPixmap::fromImage(mDefaultImage));
}

void DkDescriptionImage::dataChanged(const QModelIndex&, const QModelIndex&) {

	updateImage();
}

void DkDescriptionImage::selectionChanged(const QItemSelection&, const QItemSelection&) {

	updateImage();
}


//**********************************************************************************
//DkPluginDownloader : load download plugin data and download new plugins
//**********************************************************************************

DkPluginDownloader::DkPluginDownloader(QWidget* parent) {

	mReply = 0;
	mProgressDialog = 0;

	mRequestType = request_none;
	mDownloadAborted = false;
	QList<XmlPluginData> xmlPluginData = QList<XmlPluginData>();
	mAccessManagerPlugin = new QNetworkAccessManager(this);

	QNetworkProxyQuery npq(QUrl("http://www.nomacs.org"));
	QList<QNetworkProxy> listOfProxies = QNetworkProxyFactory::systemProxyForQuery(npq);
	if (!listOfProxies.empty() && listOfProxies[0].hostName() != "") {
		mAccessManagerPlugin->setProxy(listOfProxies[0]);
	}

	connect(mAccessManagerPlugin, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)) );
	connect(this, SIGNAL(showDownloaderMessage(QString, QString)), parent, SLOT(showDownloaderMessage(QString, QString)));
	connect(this, SIGNAL(parsingFinished(int)), parent, SLOT(manageParsedXmlData(int)));

}

void DkPluginDownloader::downloadXml(int usage) {

	mCurrUsage = usage;
	mXmlPluginData.clear();
	mRequestType = request_xml;
	mDownloadAborted = false;
	mReply = mAccessManagerPlugin->get(QNetworkRequest(QUrl("http://www.nomacs.org/plugins/list.php")));
	QEventLoop loop;
    connect(mReply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
}

void DkPluginDownloader::downloadPreviewImg(QString url) {

	mRequestType = request_preview;
	mDownloadAborted = false;
	mReply = mAccessManagerPlugin->get(QNetworkRequest(QUrl(url)));
	QEventLoop loop;
    connect(mReply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
}

void DkPluginDownloader::downloadPluginFileList(QString url) {

	if (!mProgressDialog)
		createProgressDialog();

	mFilesToDownload = QStringList();
	mRequestType = request_plugin_files_list;
	mDownloadAborted = false;
	mReply = mAccessManagerPlugin->get(QNetworkRequest(QUrl(url)));
	mProgressDialog->setLabelText(tr("Downloading file information..."));
	mProgressDialog->show();
	connect(mReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(updateDownloadProgress(qint64, qint64)));
	QEventLoop loop;
    connect(this, SIGNAL(pluginFilesDownloadingFinished()), &loop, SLOT(quit()));
    loop.exec();
	
	if (mDownloadAborted) {
		mProgressDialog->hide();
		qDebug() << "plugin download aborted.";
		return;
	}
}

void DkPluginDownloader::downloadPlugin(const QModelIndex &index, QString url, QString pluginName) {

	if (!mProgressDialog)
		createProgressDialog();

	downloadPluginFileList(url);
	if (mDownloadAborted) return;
	
	//file list parsed, start file download
	for (int i = 0; i < mFilesToDownload.size(); i++) {
		mProgressDialog->setLabelText(tr("Downloading plugin files: %1 of %2").arg(QString::number(i+1), QString::number(mFilesToDownload.size())));
		mProgressDialog->setValue(0);
		mProgressDialog->show();
		downloadSingleFile(mFilesToDownload.at(i));

		if (mDownloadAborted) {
			mProgressDialog->hide();
			qDebug() << "plugin download aborted";
			return;
		}
	}

	// // the check mark suggests this anyway
	//emit showDownloaderMessage(tr("Plugin %1 was successfully installed.").arg(pluginName), tr("Plugin manager"));
	//// TODO: check if the file exists before giving a success notification
	emit pluginDownloaded(index);
}

void DkPluginDownloader::downloadSingleFile(QString url) {

	mRequestType = request_plugin;	
	mFileName = url.split("/").last();
	mDownloadAborted = false;
	mReply = mAccessManagerPlugin->get(QNetworkRequest(QUrl(url)));
	connect(mReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(updateDownloadProgress(qint64, qint64)));
	QEventLoop loop;
    connect(this, SIGNAL(pluginFilesDownloadingFinished()), &loop, SLOT(quit()));
    loop.exec();
}

void DkPluginDownloader::updatePlugins(QList<QString> urls) {

	if (!mProgressDialog)
		createProgressDialog();

	for (int i = 0; i < urls.size(); i++) {

		downloadPluginFileList(urls.at(i));
		if (mDownloadAborted) {
			emit reloadPlugins();
			return;
		}

		for (int j = 0; j < mFilesToDownload.size(); j++) {
			mProgressDialog->setLabelText(tr("Updating plugin %1 of %2 (file: %3 of %4)").arg(QString::number(i+1), QString::number(urls.size()), QString::number(j+1), QString::number(mFilesToDownload.size())));
			mProgressDialog->setValue(0);
			mProgressDialog->show();
			downloadSingleFile(mFilesToDownload.at(j));

			if (mDownloadAborted) {
				mProgressDialog->hide();
				qDebug() << "plugin update aborted";
				emit reloadPlugins();
				return;
			}
		}
	}
	emit allPluginsUpdated(true);
}

void DkPluginDownloader::replyFinished(QNetworkReply* reply) {

	if (!reply)
		return;

	if(!mDownloadAborted) {
		if (reply->error() != QNetworkReply::NoError) {
			qDebug() << reply->error();
			if (mRequestType == request_xml) emit showDownloaderMessage(tr("Sorry, I could not download plugin information."), tr("Plugin manager"));
			else if (mRequestType == request_preview) emit showDownloaderMessage(tr("Sorry, I could not download plugin preview."), tr("Plugin manager"));
			else if (mRequestType == request_plugin_files_list || mRequestType == request_plugin) emit showDownloaderMessage(tr("Sorry, I could not download plugin."), tr("Plugin manager"));
			cancelUpdate();
			return;
		}

		switch(mRequestType) {
			case request_xml:
				parseXml(reply);
				break;
			case request_preview:
				replyToImg(reply);
				break;
			case request_plugin:
				startPluginDownload(reply);
				break;
			case request_plugin_files_list:
				parseFileList(reply);
				break;
		}
	}
}

void DkPluginDownloader::parseXml(QNetworkReply* reply) {
	
	if (!reply)
		return;

	qDebug() << "xml with plugin data downloaded, starting parsing";

	QXmlStreamReader xml(reply->readAll());

	while(!xml.atEnd() && !xml.hasError()) {

		QXmlStreamReader::TokenType token = xml.readNext();
		if(token == QXmlStreamReader::StartElement) {

			if(xml.name() == "Plugin") {
				QXmlStreamAttributes xmlAttributes = xml.attributes();
				XmlPluginData data;
				data.id = xmlAttributes.value("id").toString();
				data.name = xmlAttributes.value("name").toString();
				data.version = xmlAttributes.value("version").toString();
				data.decription = xmlAttributes.value("description").toString();
				data.previewImgUrl = xmlAttributes.value("preview_url").toString();
				data.isWin64 = (xmlAttributes.value("win_x64").toString().compare("true", Qt::CaseInsensitive) == 0);
				data.isWin86 = (xmlAttributes.value("win_x86").toString().compare("true", Qt::CaseInsensitive) == 0);
				#if defined _WIN64
					if (data.isWin64) 
						mXmlPluginData.append(data);
				#elif _WIN32
					if (data.isWin86) 
						mXmlPluginData.append(data);
				#endif
			}
		}
	}

	if(xml.hasError())  {
		emit showDownloaderMessage(tr("Sorry, I could not parse the downloaded plugin data xml"), tr("Plugin manager"));
		xml.clear();
		return;
	}
	xml.clear();

	emit parsingFinished(mCurrUsage);
}

void DkPluginDownloader::replyToImg(QNetworkReply* reply) {

	if (!reply)
		return;

    QByteArray imgData = reply->readAll();
	QImage downloadedImg;
	downloadedImg.loadFromData(imgData);
	emit imageDownloaded(downloadedImg);
}

void DkPluginDownloader::startPluginDownload(QNetworkReply* reply) {

	if (!reply)
		return;

	QDir pluginsDir = Settings::param().global().pluginsDir;

	QFile file(pluginsDir.absolutePath().append("/").append(mFileName));

	if (file.exists()) {
		if(!file.remove())	qDebug() << "Failed to delete plugin file!";
	}

	if (!file.open(QIODevice::WriteOnly)) {
		emit showDownloaderMessage(tr("Sorry, the plugin could not be saved."), tr("Plugin manager"));
		cancelUpdate();
        return;
	}
    bool writeSuccess = (file.write(reply->readAll()) > 0);
	file.close();

	if (!writeSuccess) {
		emit showDownloaderMessage(tr("Sorry, the plugin could not be saved."), tr("Plugin manager"));
		cancelUpdate();
		return;
	}

	emit pluginFilesDownloadingFinished();
}

void DkPluginDownloader::parseFileList(QNetworkReply* reply) {

	QString urlFileName = reply->url().toString().split("/").last();
	QString url = reply->url().toString().remove(urlFileName);

	mFilesToDownload = QStringList();
	// parse each line
	QByteArray line;
	do {
		line = reply->readLine();
		QString str(line);
		QStringList list = str.split(" ");
		QString downloadUrl = QString();
	#if defined _WIN64
		if(list.at(0).compare("x64") == 0) downloadUrl = list.at(1);
	#elif _WIN32
		if(list.at(0).compare("x86") == 0) downloadUrl = list.at(1);
	#endif
		downloadUrl.remove(QRegExp("[\\n\\t\\r]"));

		if(!downloadUrl.isEmpty()) {
			downloadUrl.prepend(url);
			mFilesToDownload.append(downloadUrl);
		}
	} while (!line.isNull());
	
	if (mFilesToDownload.size() == 0) {
		emit showDownloaderMessage(tr("Could not find plugins to download."), tr("Plugin manager"));
		mDownloadAborted = true;
	}

	emit pluginFilesDownloadingFinished();
}

QList<XmlPluginData> DkPluginDownloader::getXmlPluginData() {

	return mXmlPluginData;
}

void DkPluginDownloader::cancelUpdate()  {

	if (!mReply)
		return;

	mReply->abort();
	mDownloadAborted = true;
	emit pluginFilesDownloadingFinished();
}

void DkPluginDownloader::updateDownloadProgress(qint64 received, qint64 total) { 

	if (!mProgressDialog)
		createProgressDialog();

	mProgressDialog->setMaximum((int)total);
	mProgressDialog->setValue((int)received);
}

void DkPluginDownloader::createProgressDialog() {
	
	mProgressDialog = new QProgressDialog("", tr("Cancel Update"), 0, 100, QApplication::activeWindow());
	connect(mProgressDialog, SIGNAL(canceled()), this, SLOT(cancelUpdate()));
	connect(this, SIGNAL(pluginDownloaded(const QModelIndex &)), mProgressDialog, SLOT(hide()));
	connect(this, SIGNAL(allPluginsUpdated(bool)), mProgressDialog, SLOT(hide()));
}

// DkPluginManager --------------------------------------------------------------------
DkPluginManager & DkPluginManager::instance() {
	
	static QSharedPointer<DkPluginManager> inst;

	if (!inst)
		inst = QSharedPointer<DkPluginManager>(new DkPluginManager());
	
	return *inst;
}

DkPluginManager::DkPluginManager() {
	
	loadedPlugins = QMap<QString, DkPluginInterface *>();
	pluginFiles = QMap<QString, QString>();
	pluginIdList = QList<QString>();
	runId2PluginId = QMap<QString, QString>();
	pluginLoaders = QMap<QString, QPluginLoader *>();

	loadPlugins();	// NOTE: the new json files would allow us not to load all plugins here - think about it : )
}

DkPluginManager::~DkPluginManager() {
}

void DkPluginManager::addPlugin(const QString& pluginId, const QString& filePath, DkPluginInterface* plugin) {

	pluginIdList.append(pluginId);
	loadedPlugins.insert(pluginId, plugin);
	pluginFiles.insert(pluginId, filePath);
}

//returns map with id and interface
QMap<QString, DkPluginInterface*> DkPluginManager::getPlugins() const {

	return loadedPlugins;
}

DkPluginInterface* DkPluginManager::getPlugin(const QString& key) const {

	DkPluginInterface* cPlugin = loadedPlugins.value(getRunId2PluginId().value(key));

	// if we could not find the runID, try to see if it is a pluginID
	if (!cPlugin)
		cPlugin = loadedPlugins.value(key);

	return cPlugin;
}

QList<QString> DkPluginManager::getPluginIdList() const {

	return pluginIdList;
}

QString DkPluginManager::getPluginFilePath(const QString& key) const {
	return pluginFiles.value(key);
}

QMap<QString, QString> DkPluginManager::getPluginFilePaths() const {

	return pluginFiles;
}

void DkPluginManager::setPluginIdList(QList<QString> newPlugins) {

	pluginIdList = newPlugins;
}

void DkPluginManager::setRunId2PluginId(QMap<QString, QString> newMap) {

	runId2PluginId = newMap;
}

QMap<QString, QString> DkPluginManager::getRunId2PluginId() const {

	return runId2PluginId;
}

void DkPluginManager::removePlugin(const QString& id) {

	pluginFiles.remove(id);
	pluginIdList.removeAll(id);
	loadedPlugins.remove(id);

	QPluginLoader* loaderToDelete = pluginLoaders.take(id);

	if (!loaderToDelete) {
		qDebug() << "cannot remove plugin - empty plugin loader...";
		return;
	}

	if(!loaderToDelete->unload()) 
		qDebug() << "Could not unload plugin loader!";
	delete loaderToDelete;
	loaderToDelete = 0;
}

void DkPluginManager::clear() {
	pluginFiles.clear();
	runId2PluginId.clear();
	loadedPlugins.clear();
	pluginIdList.clear();
}

void DkPluginManager::saveSettings() const {

	QSettings& settings = Settings::instance().getSettings();

	settings.remove("PluginSettings/filePaths");
	settings.beginWriteArray("PluginSettings/filePaths");

	for (int idx = 0; idx < pluginIdList.size(); idx++) {
		settings.setArrayIndex(idx);
		settings.setValue("pluginId", pluginIdList.at(idx));
		settings.setValue("pluginFilePath", pluginFiles.value(pluginIdList.at(idx)));
		settings.setValue("version", loadedPlugins.value(pluginIdList.at(idx))->pluginVersion());
	}
	settings.endArray();
}

//Loads enabled plugins when the menu is first hit
void DkPluginManager::loadPlugins() {

	if (!loadedPlugins.isEmpty()) 
		qDebug() << "Plugin list is not empty where it should be!";

	QMap<QString, QString> pluginsPaths = QMap<QString, QString>();
	QList<QString> disabledPlugins = QList<QString>();
	QSettings& settings = Settings::instance().getSettings();

	int size = settings.beginReadArray("PluginSettings/filePaths");
	for (int i = 0; i < size; i++) {
		settings.setArrayIndex(i);
		pluginsPaths.insert(settings.value("pluginId").toString(), settings.value("pluginFilePath").toString());
	}
	settings.endArray();

	size = settings.beginReadArray("PluginSettings/disabledPlugins");
	for (int i = 0; i < size; i++) {
		settings.setArrayIndex(i);
		disabledPlugins.append(settings.value("pluginId").toString());
	}
	settings.endArray();

	QMapIterator<QString, QString> iter(pluginsPaths);	

	while(iter.hasNext()) {
		iter.next();
		singlePluginLoad(iter.value());
	}
}

/**
* Loads one plugin from file fileName
* @param fileName
**/
bool DkPluginManager::singlePluginLoad(const QString& filePath) {

	QPluginLoader* loader = new QPluginLoader(filePath);

	if (!loader->load()) {
		qDebug() << "Could not load: " << filePath;
		return false;
	}

	QObject* pluginObject = loader->instance();

	if(pluginObject) {

		DkPluginInterface* initializedPlugin = qobject_cast<DkPluginInterface*>(pluginObject);

		if (!initializedPlugin)
			initializedPlugin = qobject_cast<DkViewPortInterface*>(pluginObject);

		if(initializedPlugin) {
			QString pluginID = initializedPlugin->pluginID();
			pluginLoaders.insert(pluginID, loader);

			addPlugin(pluginID, filePath, initializedPlugin);

			// init actions
			initializedPlugin->createActions(QApplication::activeWindow());
		}
		else {
			delete loader;
			qDebug() << "could not initialize: " << filePath;
			return false;
		}
	}
	else {
		delete loader;
		qDebug() << "could not load: " << filePath << "NULL Object";
		return false;
	}

	qDebug() << filePath << " loaded...";

	return true;
}

DkPluginInterface * DkPluginManager::getPluginByName(const QString & pluginName) const {

	for (const QString& pluginId : pluginIdList) {

		DkPluginInterface* p = getPlugin(pluginId);

		if (p && pluginName == p->pluginName())
			return p;
	}

	return nullptr;
}

QString DkPluginManager::actionNameToRunId(const QString & pluginId, const QString & actionName) const {

	DkPluginInterface* p = getPlugin(pluginId);

	if (p) {
		QList<QAction*> actions = p->pluginActions();
		for (const QAction* a : actions) {
			if (a->text() == actionName)
				return a->data().toString();
		}
	}

	return QString();
}

QVector<DkPluginInterface*> DkPluginManager::getBasicPlugins() const {
	
	QVector<DkPluginInterface*> plugins;

	for (const QString& pluginId : pluginIdList) {
		
		DkPluginInterface* p = getPlugin(pluginId);

		if (p && p->interfaceType() == DkPluginInterface::interface_basic) {
			plugins.append(p);
		}
	}

	return plugins;
}

DkPluginInterface* DkPluginManager::getRunningPlugin() const {

	if (!mRunningPlugin.isEmpty())
		return getPlugin(mRunningPlugin);

	return 0;
}

void DkPluginManager::clearRunningPluginKey() {
	mRunningPlugin = "";
}

DkPluginInterface* DkPluginManager::runPlugin(const QString& key) {

	if (!mRunningPlugin.isEmpty()) {

		// the plugin is not closed in time
		QMessageBox infoDialog(QApplication::activeWindow());
		infoDialog.setWindowTitle("Close plugin");
		infoDialog.setIcon(QMessageBox::Information);
		infoDialog.setText("Please first close the currently opened plugin.");
		infoDialog.show();

		infoDialog.exec();

		//TODO: dialog with yes/no - unload plugin if user wants to

		return 0;
	}

	DkPluginInterface* cPlugin = DkPluginManager::instance().getPlugin(key);
	mRunningPlugin = key;

	return cPlugin;
}

// DkPluginActionManager --------------------------------------------------------------------
DkPluginActionManager::DkPluginActionManager(QObject* parent) : QObject(parent) {
	
	assignCustomPluginShortcuts();
}

void DkPluginActionManager::assignCustomPluginShortcuts() {

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup("CustomPluginShortcuts");
	QStringList psKeys = settings.allKeys();
	settings.endGroup();

	if (psKeys.size() > 0) {

		settings.beginGroup("CustomShortcuts");

		mPluginDummyActions = QVector<QAction *>();

		for (int i = 0; i< psKeys.size(); i++) {

			QAction* action = new QAction(psKeys.at(i), this);
			QString val = settings.value(psKeys.at(i), "no-shortcut").toString();
			if (val != "no-shortcut")
				action->setShortcut(val);
			connect(action, SIGNAL(triggered()), this, SLOT(runPluginFromShortcut()));
			// assign widget shortcuts to all of them
			action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
			mPluginDummyActions.append(action);
			qDebug() << "new plugin action: " << psKeys.at(i);
		}

		settings.endGroup();
	}
}

void DkPluginActionManager::setMenu(QMenu* menu) {
	mMenu = menu;
	connect(mMenu, SIGNAL(aboutToShow()), this, SLOT(updateMenu()));
}

QMenu* DkPluginActionManager::menu() const {
	return mMenu;
}

QVector<QAction*> DkPluginActionManager::pluginDummyActions() const {
	return mPluginDummyActions;
}

QVector<QAction*> DkPluginActionManager::pluginActions() const {
	return mPluginActions;
}

QVector<QMenu*> DkPluginActionManager::pluginSubMenus() const {
	return mPluginSubMenus;
}

void DkPluginActionManager::updateMenu() {
	qDebug() << "CREATING plugin menu";

	if (!mMenu) {
		qWarning() << "plugin menu is NULL where it should not be!";
	}

	if (mPluginActions.empty()) {
		//mPluginActions.resize(DkActionManager::menu_plugins_end);
		mPluginActions = DkActionManager::instance().pluginActions();
	}

	mMenu->clear();

	QList<QString> pluginIdList = DkPluginManager::instance().getPluginIdList();

	qDebug() << "id list: " << pluginIdList;

	if (pluginIdList.isEmpty()) { // no  plugins
		mMenu->addAction(mPluginActions[DkActionManager::menu_plugin_manager]);
		mPluginActions.resize(DkActionManager::menu_plugin_manager);	// reduce the size again
	}
	else {
		// delete old plugin actions	
		for (int idx = mPluginActions.size(); idx > DkActionManager::menu_plugins_end; idx--) {
			mPluginActions.last()->deleteLater();
			mPluginActions.last() = 0;
			mPluginActions.pop_back();
		}
		addPluginsToMenu();
	}

}

/**
* Creates the plugin menu when it is not empty
* called in DkNoMacs::createPluginsMenu()
**/
void DkPluginActionManager::addPluginsToMenu() {

	QMap<QString, DkPluginInterface *> loadedPlugins = DkPluginManager::instance().getPlugins();
	QList<QString> pluginIdList = DkPluginManager::instance().getPluginIdList();

	QMap<QString, QString> runId2PluginId = QMap<QString, QString>();
	QList<QPair<QString, QString> > sortedNames = QList<QPair<QString, QString> >();
	mPluginSubMenus.clear();

	QStringList pluginMenu = QStringList();

	for (int i = 0; i < pluginIdList.size(); i++) {

		DkPluginInterface* cPlugin = loadedPlugins.value(pluginIdList.at(i));

		if (cPlugin) {

			QStringList runID = cPlugin->runID();
			QList<QAction*> actions = cPlugin->createActions(QApplication::activeWindow());

			if (!actions.empty()) {

				for (int iAction = 0; iAction < actions.size(); iAction++) {
					connect(actions.at(iAction), SIGNAL(triggered()), this, SLOT(runLoadedPlugin()), Qt::UniqueConnection);
					runId2PluginId.insert(actions.at(iAction)->data().toString(), pluginIdList.at(i));
				}

				QMenu* sm = new QMenu(cPlugin->pluginMenuName(), mMenu);
				sm->setStatusTip(cPlugin->pluginStatusTip());
				sm->addActions(actions);
				runId2PluginId.insert(cPlugin->pluginMenuName(), pluginIdList.at(i));

				mPluginSubMenus.append(sm);


			}
			else {

				// deprecated!
				for (int j = 0; j < runID.size(); j++) {

					runId2PluginId.insert(runID.at(j), pluginIdList.at(i));
					sortedNames.append(qMakePair(runID.at(j), cPlugin->pluginMenuName(runID.at(j))));
				}
			}
		}
	}

	mMenu->addAction(DkActionManager::instance().action(DkActionManager::menu_plugin_manager));
	mMenu->addSeparator();

	QMap<QString, bool> pluginsEnabled = QMap<QString, bool>();

	QSettings& settings = Settings::instance().getSettings();
	int size = settings.beginReadArray("PluginSettings/disabledPlugins");
	for (int i = 0; i < size; ++i) {
		settings.setArrayIndex(i);
		if (pluginIdList.contains(settings.value("pluginId").toString())) pluginsEnabled.insert(settings.value("pluginId").toString(), false);
	}
	settings.endArray();

	for(int i = 0; i < sortedNames.size(); i++) {


		if (pluginsEnabled.value(runId2PluginId.value(sortedNames.at(i).first), true)) {

			QAction* pluginAction = new QAction(sortedNames.at(i).second, this);
			pluginAction->setStatusTip(loadedPlugins.value(runId2PluginId.value(sortedNames.at(i).first))->pluginStatusTip(sortedNames.at(i).first));
			pluginAction->setData(sortedNames.at(i).first);
			connect(pluginAction, SIGNAL(triggered()), this, SLOT(runLoadedPlugin()), Qt::UniqueConnection);

			mMenu->addAction(pluginAction);
			pluginAction->setToolTip(pluginAction->statusTip());

			mPluginActions.append(pluginAction);
		}		
	}

	for (int idx = 0; idx < mPluginSubMenus.size(); idx++) {

		if (pluginsEnabled.value(runId2PluginId.value(mPluginSubMenus.at(idx)->title()), true))
			mMenu->addMenu(mPluginSubMenus.at(idx));

	}

	DkPluginManager::instance().setRunId2PluginId(runId2PluginId);

	QVector<QAction*> allPluginActions = mPluginActions;

	for (const QMenu* m : mPluginSubMenus) {
		allPluginActions << m->actions().toVector();
	}

	DkActionManager::instance().assignCustomShortcuts(allPluginActions);
	savePluginActions(allPluginActions);
}

void DkPluginActionManager::runPluginFromShortcut() {

	qDebug() << "running plugin shortcut...";

	QAction* action = qobject_cast<QAction*>(sender());
	QString actionName = action->text();

	updateMenu();

	QVector<QAction*> allPluginActions = mPluginActions;

	for (const QMenu* m : mPluginSubMenus) {
		allPluginActions << m->actions().toVector();
	}

	// this method fails if two plugins have the same action name!!
	for (int i = 0; i < allPluginActions.size(); i++)
		if (allPluginActions.at(i)->text().compare(actionName) == 0) {
			allPluginActions.at(i)->trigger();
			break;
		}
}

void DkPluginActionManager::runLoadedPlugin() {

	QAction* action = qobject_cast<QAction*>(sender());

	if (!action)
		return;

	emit applyPluginChanges(true);
	QApplication::sendPostedEvents();

	QString key = action->data().toString();

	DkPluginInterface* cPlugin = DkPluginManager::instance().runPlugin(key);

	if (cPlugin && cPlugin->interfaceType() == DkPluginInterface::interface_viewport) {

		DkViewPortInterface* vPlugin = dynamic_cast<DkViewPortInterface*>(cPlugin);

		if(!vPlugin || !vPlugin->getViewPort()) 
			return;

		connect(vPlugin->getViewPort(), SIGNAL(showToolbar(QToolBar*, bool)), vPlugin->getMainWindow(), SLOT(showToolbar(QToolBar*, bool)));

		emit runPlugin(vPlugin, false);
	}
	else if (cPlugin && cPlugin->interfaceType() == DkPluginInterface::interface_basic) {
		emit runPlugin(cPlugin, key);
	}
}

void DkPluginActionManager::savePluginActions(QVector<QAction *> actions) const {

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup("CustomPluginShortcuts");
	settings.remove("");
	for (int i = 0; i < actions.size(); i++)
		settings.setValue(actions.at(i)->text(), actions.at(i)->text());
	settings.endGroup();
}


};

