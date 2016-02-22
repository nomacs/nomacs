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
#include <QJsonValue>
#pragma warning(pop)		// no warnings from includes - end

#ifdef QT_NO_DEBUG_OUTPUT
#pragma warning(disable: 4127)		// no 'conditional expression is constant' if qDebug() messages are removed
#endif

namespace nmc {

// DkPluginContainer --------------------------------------------------------------------
DkPluginContainer::DkPluginContainer(const QString& pluginPath) {
	
	mPluginPath = pluginPath;
	mLoader = QSharedPointer<QPluginLoader>(new QPluginLoader(mPluginPath));
	loadJson();
}

DkPluginContainer::~DkPluginContainer() {

	//if (mLoader) {
	//	qDebug() << "unloaded: " << mLoader->unload();
	//}
}

bool operator<(const QSharedPointer<DkPluginContainer>& l, const QSharedPointer<DkPluginContainer>& r) {

	if (!l || !r)
		return false;

	return l->pluginName() < r->pluginName();
}

void DkPluginContainer::setActive(bool active) {
	mActive = active;
}

bool DkPluginContainer::isActive() const {
	return mActive;
}

bool DkPluginContainer::isLoaded() const {
	return mLoader->isLoaded();
}

bool DkPluginContainer::load() {

	if (!isValid()) {
		qDebug() << "Invalid: " << mPluginPath;
		return false;
	}
	else if (!mLoader->load()) {
		qDebug() << "Could not load: " << mPluginPath;
		return false;
	}

	if (pluginViewPort()) {
		mType = type_viewport;
	}
	else if (plugin())
		mType = type_simple;
	else {
		qDebug() << "could not initialize: " << mPluginPath;
		return false;
	}

	if (mType != type_unknown) {
		// init actions
		plugin()->createActions(QApplication::activeWindow());
		createMenu();
	}
	
	qDebug() << mPluginPath << " loaded...";
	return true;

}

bool DkPluginContainer::uninstall() {

	mLoader->unload();
	return QFile::remove(mPluginPath);
}

void DkPluginContainer::createMenu() {

	DkPluginInterface* p = plugin();

	// empty menu if we do not have any actions
	if (!p || p->pluginActions().empty())
		return;

	qDebug() << "creating plugin menu for " << pluginName();
	mPluginMenu = new QMenu(pluginName(), QApplication::activeWindow());

	for (auto action : p->pluginActions()) {
		mPluginMenu->addAction(action);
		connect(action, SIGNAL(triggered()), this, SLOT(run()), Qt::UniqueConnection);
	}

}

void DkPluginContainer::loadJson() {

	QJsonObject metaData = mLoader->metaData();
	QStringList keys = metaData.keys();

	for (const QString& key : keys) {
	
		if (key == "MetaData")
			loadMetaData(metaData.value(key));
		else if (key == "IID" && metaData.value(key).toString().contains("com.nomacs.ImageLounge"))
			mIsValid = true;
#ifndef _DEBUG	// warn if we have a debug & are not in debug ourselves
		else if (key == "debug") {
			bool isDebug = metaData.value(key).toBool();
			if (isDebug)
				qWarning() << "I cannot load a debug dll since I am compiled in release!";
		}
#endif

		//qDebug() << key << "|" << metaData.value(key);
	}
}

void DkPluginContainer::loadMetaData(const QJsonValue& val) {

	// we expect something like this...
	//{
	//		"PluginName" 	: [ "BinarizationPlugin" ],
	//		"AuthorName" 	: [ "Markus Diem" ],
	//		"Company"		: [ "Computer Vision Lab" ],
	//		"DateCreated" 	: [ "04.02.2016" ],
	//		"DateModified" 	: [ "04.02.2016" ],
	//		"Description"	: [ "Document Binarization Plugin" ],
	//		"StatusTip" 	: [ "Applies e.g. the Su et al. binarzation to an image." ]
	//}
	
	QJsonObject metaData = val.toObject();
	QStringList keys = metaData.keys();

	for (const QString& key : keys) {

		if (key == "PluginName")
			mPluginName = metaData.value(key).toString();
		else if (key == "AuthorName")
			mAuthorName = metaData.value(key).toString();
		else if (key == "Company")
			mCompany = metaData.value(key).toString();
		else if (key == "DateCreated")
			mDateCreated = QDate::fromString(metaData.value(key).toString(), "yyyy-MM-dd");
		else if (key == "DateModified")
			mDateModified = QDate::fromString(metaData.value(key).toString(), "yyyy-MM-dd");
		else if (key == "Description")
			mDescription = metaData.value(key).toString();
		else if (key == "Tagline")
			mStatusTip = metaData.value(key).toString();
		else if (key == "Version") {
			// currently nothing to do here...
		}
		else if (key == "PluginId") {
			// currently nothing to do here...
		}
		else 
			qDebug() << "unknown key" << key << "|" << metaData.value(key);

		//qDebug() << "parsing:" << key << "|" << metaData.value(key);
	}

	if (!isValid() && !keys.empty()) {
		qWarning() << "invalid plugin - missing the PluginName in the jason metadata...";
	}

}

void DkPluginContainer::run() {

	DkPluginInterface* p = plugin();

	if (p && p->interfaceType() == DkPluginInterface::interface_viewport) {

		DkViewPortInterface* vPlugin = pluginViewPort();
		mActive = true;

		if(!vPlugin || !vPlugin->getViewPort()) 
			return;

		connect(vPlugin->getViewPort(), SIGNAL(showToolbar(QToolBar*, bool)), vPlugin->getMainWindow(), SLOT(showToolbar(QToolBar*, bool)));
		emit runPlugin(vPlugin, false);
	}
	else if (p && p->interfaceType() == DkPluginInterface::interface_basic) {

		QAction* a = qobject_cast<QAction*>(QObject::sender());

		if (a)
			emit runPlugin(this, a->data().toString());
	}
	else
		qWarning() << "plugin with illegal interface detected in DkPluginContainer::run()";

}

bool DkPluginContainer::isValid() const {
	return mIsValid;
}

QString DkPluginContainer::pluginPath() const{
	return mPluginPath;
}

QString DkPluginContainer::pluginName() const {
	return mPluginName;
}

QString DkPluginContainer::authorName() const {
	return mAuthorName;
}

QString DkPluginContainer::company() const {
	return mCompany;
}

QString DkPluginContainer::version() const {
	
	if (plugin())
		return plugin()->version();
	else
		return "";
}

QString DkPluginContainer::description() const {
	return mDescription;
}

QString DkPluginContainer::fullDescription() const {
	
	QString fs;
	fs += "<b>Author:</b> " + mAuthorName;
	fs += "<br><b>Last Modified:</b> " + mDateModified.toString(Qt::LocalDate);
	fs += "<br>" + mDescription;
	
	return fs;
}

QString DkPluginContainer::statusTip() const {
	return mStatusTip;
}

QDate DkPluginContainer::dateCreated() const {
	return mDateCreated;
}

QDate DkPluginContainer::dateModified() const {
	return mDateModified;
}

QMenu * DkPluginContainer::pluginMenu() const {
	return mPluginMenu;
}

QSharedPointer<QPluginLoader> DkPluginContainer::loader() const {
	return mLoader;
}

DkPluginInterface* DkPluginContainer::plugin() const {
	
	// is everything fine here??
	if (!mLoader)
		return 0;

	DkPluginInterface* pi = qobject_cast<DkPluginInterface*>(mLoader->instance());

	if (!pi)
		return pluginViewPort();

	return pi;
}

DkViewPortInterface* DkPluginContainer::pluginViewPort() const {

	// is everything fine here??
	if (!mLoader)
		return 0;

	return qobject_cast<DkViewPortInterface*>(mLoader->instance());
}

QString DkPluginContainer::actionNameToRunId(const QString & actionName) const {
	
	if (!plugin())
		return QString();

	QList<QAction*> actions = plugin()->pluginActions();
	for (const QAction* a : actions) {
		if (a->text() == actionName)
			return a->data().toString();
	}

	return QString();
}

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

	setWindowTitle(tr("Plugin Manager"));
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

	setSizeGripEnabled(false);
}


void DkPluginManagerDialog::closePressed() {

	close();
}

void DkPluginManagerDialog::showEvent(QShowEvent* ev) {

	qDebug() << "show event called...";
	DkPluginManager::instance().loadPlugins();
	tableWidgetInstalled->getPluginUpdateData();

	tabs->setCurrentIndex(tab_installed_plugins);
	tableWidgetInstalled->clearTableFilters();
	tableWidgetInstalled->updateInstalledModel();
	tableWidgetDownload->clearTableFilters();

	QDialog::showEvent(ev);
}

void DkPluginManagerDialog::deleteInstance(QSharedPointer<DkPluginContainer> plugin) {

	DkPluginManager::instance().removePlugin(plugin);
}

QMap<QString, QString> DkPluginManagerDialog::getPreviouslyInstalledPlugins() {

	return previouslyInstalledPlugins;
}

void DkPluginManagerDialog::tabChanged(int tab){
	
	if(tab == tab_installed_plugins) 
		tableWidgetInstalled->updateInstalledModel();
	else if(tab == tab_download_plugins) 
		tableWidgetDownload->downloadPluginInformation(xml_usage_download);
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
		mModel = new DkInstalledPluginsModel(this);
	else if (mOpenedTab == tab_download_plugins) 
		mModel = new DkDownloadPluginsModel(this);
	mProxyModel->setSourceModel(mModel);
	mTableView->setModel(mProxyModel);
	mTableView->resizeColumnsToContents();
	if(mOpenedTab == tab_installed_plugins) {
		mTableView->setColumnWidth(ip_column_name, qMax(mTableView->columnWidth(ip_column_name), 300));
		mTableView->setColumnWidth(ip_column_version, qMax(mTableView->columnWidth(ip_column_version), 80));
	} else if (mOpenedTab == tab_download_plugins) {
		mTableView->setColumnWidth(dp_column_name, qMax(mTableView->columnWidth(dp_column_name), 360));
		mTableView->setColumnWidth(dp_column_version, qMax(mTableView->columnWidth(dp_column_version), 80));
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

	DkDescriptionEdit* decriptionEdit = new DkDescriptionEdit(mModel, mProxyModel, mTableView->selectionModel(), this);
	connect(mTableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), decriptionEdit, SLOT(selectionChanged(const QItemSelection &, const QItemSelection &)));
	connect(mProxyModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), decriptionEdit, SLOT(dataChanged(const QModelIndex &, const QModelIndex &)));
	bottHorLayout->addWidget(decriptionEdit);

	DkDescriptionImage* decriptionImg = new DkDescriptionImage(mModel, mProxyModel, mTableView->selectionModel(), this);
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

	DkPluginManager::instance().reload();
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

	QVector<QSharedPointer<DkPluginContainer> > plugins = DkPluginManager::instance().getPlugins();
	QList<XmlPluginData> updateList = mPluginDownloader->getXmlPluginData();
	mPluginsToUpdate = QList<XmlPluginData>();

	mUpdateButton->setEnabled(false);
	mUpdateButton->setText(tr("Plugins up to date"));
	mUpdateButton->setToolTip(tr("No available updates."));

	for (int i = 0; i < updateList.size(); i++) {
		
		
		for (auto plugin : plugins) {
			
			if(updateList.at(i).id == plugin->plugin()->id()) {
				QStringList cVersion = plugin->version().split('.');
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
	
	mUpdateButton->setText(tr("Plugins up-to-date"));
	mUpdateButton->setToolTip(tr("No updates available."));
	mUpdateButton->setEnabled(false);

	if (mPluginsToUpdate.size() > 0) {
		
		//DkPluginManager::instance().clear();

		// after deleting instances the file are not in use anymore -> update
		QList<QString> urls = QList<QString>();
		while (mPluginsToUpdate.size() > 0) {
			XmlPluginData pluginData = mPluginsToUpdate.takeLast();
			QString downloadFileListUrl = "http://www.nomacs.org/plugins-download/" + pluginData.id + "/" + pluginData.version + "/d.txt";
			urls.append(downloadFileListUrl);
		}

		mPluginDownloader->updatePlugins(urls);
		DkPluginManager::instance().reload();
	}
}

void DkPluginTableWidget::pluginUpdateFinished(bool finishedSuccessfully) {

	DkPluginManager::instance().reload();
	updateInstalledModel();
	if (finishedSuccessfully) 
		showDownloaderMessage(tr("The plugins have been updated."), tr("Plugin manager"));
}

void DkPluginTableWidget::filterTextChanged() {

	QRegExp regExp(mFilterEdit->text(), Qt::CaseInsensitive, QRegExp::FixedString);
	mProxyModel->setFilterRegExp(regExp);
	mTableView->resizeRowsToContents();
}

void DkPluginTableWidget::uninstallPlugin(const QModelIndex &index) {

	int selectedRow = mProxyModel->mapToSource(index).row();

	if (selectedRow < 0 || selectedRow > DkPluginManager::instance().getPlugins().size()) {
		qWarning() << "illegal row in uninstall plugin: " << selectedRow;
		return;
	}

	QSharedPointer<DkPluginContainer> plugin = DkPluginManager::instance().getPlugins().at(selectedRow);

	if (!plugin) {
		qWarning() << "plugin is NULL... aborting uninstall";
		return;
	}

	DkPluginManager::instance().deletePlugin(plugin);
	//updateInstalledModel();	// !!! update model before deleting the interface
	mTableView->model()->removeRow(index.row());
	//emit dataChanged(index, index);

	mTableView->resizeRowsToContents();
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
	DkPluginManager::instance().loadPlugins();

	qDebug() << "plugin saved to: " << Settings::param().global().pluginsDir;
}

void DkPluginTableWidget::clearTableFilters(){

	mFilterEdit->clear();
	mFilterEdit->setFocus();
}


//update models if new plugins are installed or copied into the folder
void DkPluginTableWidget::updateInstalledModel() {

	clearTableFilters();
	//mTableView->relo;
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

	for (int i = 0; i < modelData.size(); i++) {

		bool installed = DkPluginManager::instance().getPlugin(modelData.at(i).id) != 0;
		downloadPluginsModel->updateInstalledData(downloadPluginsModel->index(i, dp_column_install), installed);
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

	mParentTable = static_cast<DkPluginTableWidget*>(parent);
}

int DkInstalledPluginsModel::rowCount(const QModelIndex&) const {

	return DkPluginManager::instance().getPlugins().size();
}

int DkInstalledPluginsModel::columnCount(const QModelIndex&) const {

	return ip_column_size;
}

QVariant DkInstalledPluginsModel::data(const QModelIndex &index, int role) const {

	if (!index.isValid()) {
		return QVariant();
	}

	const QVector<QSharedPointer<DkPluginContainer> >& plugins = DkPluginManager::instance().getPlugins();

	if (index.row() >= plugins.size() || index.row() < 0) {
		return QVariant();
	}

    if (role == Qt::DisplayRole) {
		
		QSharedPointer<DkPluginContainer> plugin = plugins.at(index.row());
        if (index.column() == ip_column_name) {
			return plugin->pluginName();
		}
        else if (index.column() == ip_column_version) {
			return plugin->version();
		}
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

bool DkInstalledPluginsModel::removeRows(int position, int rows, const QModelIndex & index) {
	
	beginRemoveRows(QModelIndex(), position, position+rows-1);
	endRemoveRows();

	emit dataChanged(index, index);

	return true;
}


void DkInstalledPluginsModel::setDataToInsert(QSharedPointer<DkPluginContainer> newData) {

	mPluginToInsert = newData;
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

	if(installed) 
		mPluginsInstalled.insert(mPluginData.at(index.row()).id, true);
	else 
		mPluginsInstalled.remove(mPluginData.at(index.row()).id);
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
			
			if (mParentTable->getOpenedTab()==tab_installed_plugins) {
				const QVector<QSharedPointer<DkPluginContainer> >& plugins = DkPluginManager::instance().getPlugins();
				QSharedPointer<DkPluginContainer> plugin = plugins.at(sourceIndex.row());
				if (plugin) 
					text = plugin->fullDescription();
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
			QImage img;
			if (mParentTable->getOpenedTab()==tab_installed_plugins) {

				const QVector<QSharedPointer<DkPluginContainer> >& plugins = DkPluginManager::instance().getPlugins();
				QSharedPointer<DkPluginContainer> plugin = plugins.at(sourceIndex.row());
				
				if (plugin && plugin->plugin())
					img = plugin->plugin()->image();
				if (!img.isNull()) 
					setPixmap(QPixmap::fromImage(img));
				else 
					setPixmap(QPixmap::fromImage(mDefaultImage));
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
			if (mRequestType == request_xml) 
				emit showDownloaderMessage(tr("Sorry, I could not download plugin information."), tr("Plugin manager"));
			else if (mRequestType == request_preview) 
				emit showDownloaderMessage(tr("Sorry, I could not download plugin preview."), tr("Plugin manager"));
			else if (mRequestType == request_plugin_files_list || mRequestType == request_plugin) 
				emit showDownloaderMessage(tr("Sorry, I could not download plugin."), tr("Plugin manager"));
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

	QFileInfo pluginPath(Settings::param().global().pluginsDir, mFileName);

	// remove old plugin if available
	QSharedPointer<DkPluginContainer> pc = DkPluginManager::instance().getPluginByPath(pluginPath.absoluteFilePath());
	if (pc && !pc->uninstall()) 
		emit showDownloaderMessage(tr("Sorry, %1 could not be removed...").arg(mFileName), tr("Plugin manager"));
	

	QFile file(pluginPath.absoluteFilePath());

	//if (file.exists() && !file.remove()) {
	//	qDebug() << "Failed to delete plugin file!";
	//}

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
	
	//loadPlugins();
}

DkPluginManager::~DkPluginManager() {
}

//returns map with id and interface
QVector<QSharedPointer<DkPluginContainer> >  DkPluginManager::getPlugins() const {

	return mPlugins;
}

QSharedPointer<DkPluginContainer> DkPluginManager::getPlugin(const QString& id) const {

	for (auto cPlugin : mPlugins) {

		if (cPlugin->plugin() && cPlugin->plugin()->id() == id) {
			return cPlugin;
		}

	}
	
	qWarning() << "could not find plugin for" << id;
	return QSharedPointer<DkPluginContainer>();
}

QString DkPluginManager::getPluginFilePath(const QString& id) const {
	
	QSharedPointer<DkPluginContainer> plugin = getPlugin(id);

	if (plugin)
		return plugin->pluginPath();
	else
		return "";
}

//QMap<QString, QString> DkPluginManager::getPluginFilePaths() const {
//
//	return pluginFiles;
//}

void DkPluginManager::reload() {
	clear();
	loadPlugins();
}

void DkPluginManager::removePlugin(QSharedPointer<DkPluginContainer> plugin) {

	if (plugin) 
		mPlugins.remove(mPlugins.indexOf(plugin));
	else
		qWarning() << "Could not delete plugin - it is NULL";

}

void DkPluginManager::deletePlugin(QSharedPointer<DkPluginContainer> plugin) {
	
	if (plugin) {
		
		mPlugins.remove(mPlugins.indexOf(plugin));
		
		if (!plugin->uninstall()) {
			qDebug() << "Failed to delete plugin file!";
			QMessageBox::critical(QApplication::activeWindow(), QObject::tr("Plugin manager"), QObject::tr("The dll could not be deleted!\nPlease restart nomacs and try again."));
		}
	}
}

void DkPluginManager::clear() {
	mPlugins.clear();
}

//Loads enabled plugins when the menu is first hit
void DkPluginManager::loadPlugins() {

	// do not load twice
	if (!mPlugins.empty())
		return;

	QStringList loadedPluginFileNames = QStringList();
	QStringList libPaths = QCoreApplication::libraryPaths();
	libPaths.append(QCoreApplication::applicationDirPath() + "/plugins");

	qDebug() << "lib paths" << libPaths;

	for (const QString& cPath : libPaths) {

		// skip the nomacs dir
		if (cPath == QApplication::applicationDirPath())
			continue;

		QDir pluginsDir(cPath);

		for (const QString& fileName : pluginsDir.entryList(QDir::Files)) {

			QString shortFileName = fileName.split("/").last();
			if (!loadedPluginFileNames.contains(shortFileName)) { // prevent double loading of the same plugin
				
				if (singlePluginLoad(pluginsDir.absoluteFilePath(fileName)))
					loadedPluginFileNames.append(shortFileName);
			}
			else
				qDebug() << "rejected since it is twice: " << shortFileName;
		}
	}

	qSort(mPlugins.begin(), mPlugins.end());// , &DkPluginContainer::operator<);
}

/**
* Loads one plugin from file fileName
* @param fileName
**/
bool DkPluginManager::singlePluginLoad(const QString& filePath) {

	QSharedPointer<DkPluginContainer> plugin = QSharedPointer<DkPluginContainer>(new DkPluginContainer(filePath));
	if (plugin->load())
		mPlugins.append(plugin);

	return plugin->isLoaded();
}

QSharedPointer<DkPluginContainer> DkPluginManager::getPluginByName(const QString & pluginName) const {

	for (auto p : mPlugins) {

		if (p && pluginName == p->pluginName())
			return p;
	}

	return QSharedPointer<DkPluginContainer>();
}

QSharedPointer<DkPluginContainer> DkPluginManager::getPluginByPath(const QString & path) const {

	for (auto p : mPlugins) {

		if (p && path == p->pluginPath())
			return p;
	}

	return QSharedPointer<DkPluginContainer>();
}

QVector<QSharedPointer<DkPluginContainer> > DkPluginManager::getBasicPlugins() const {
	
	QVector<QSharedPointer<DkPluginContainer> > plugins;

	for (auto plugin : mPlugins) {
		
		DkPluginInterface* p = plugin->plugin();

		if (p && p->interfaceType() == DkPluginInterface::interface_basic) {
			plugins.append(plugin);
		}
	}

	return plugins;
}

QSharedPointer<DkPluginContainer> DkPluginManager::getRunningPlugin() const {

	for (auto plugin : mPlugins) {
		if (plugin->isActive())
			return plugin;
	}

	return QSharedPointer<DkPluginContainer>();
}

void DkPluginManager::clearRunningPlugin() {
	
	for (auto plugin : mPlugins)
		plugin->setActive(false);
}

void DkPluginManager::runPlugin(QSharedPointer<DkPluginContainer> plugin) {

	if (getRunningPlugin()) {

		// the plugin is not closed in time
		QMessageBox infoDialog(QApplication::activeWindow());
		infoDialog.setWindowTitle(QObject::tr("Close plugin"));
		infoDialog.setIcon(QMessageBox::Information);
		infoDialog.setText(QObject::tr("Please close the currently opened plugin."));
		infoDialog.show();

		infoDialog.exec();

		//TODO: dialog with yes/no - unload plugin if user wants to
	}

	plugin->setActive();
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

	DkPluginManager::instance().loadPlugins();
	QVector<QSharedPointer<DkPluginContainer> > plugins = DkPluginManager::instance().getPlugins();

	if (plugins.empty()) {
		//mPluginActions.resize(DkActionManager::menu_plugins_end);
		mPluginActions = DkActionManager::instance().pluginActions();
	}
	mMenu->clear();

	for (auto p : plugins) {
		connect(p.data(), SIGNAL(runPlugin(DkViewPortInterface*, bool)), this, SIGNAL(runPlugin(DkViewPortInterface*, bool)), Qt::UniqueConnection);
		connect(p.data(), SIGNAL(runPlugin(DkPluginContainer*, const QString&)), this, SIGNAL(runPlugin(DkPluginContainer*, const QString&)), Qt::UniqueConnection);
	}

	if (plugins.isEmpty()) { // no  plugins
		mMenu->addAction(mPluginActions[DkActionManager::menu_plugin_manager]);
		mPluginActions.resize(DkActionManager::menu_plugin_manager);		// reduce the size again
	}
	else {
		// delete old plugin actions	
		for (int idx = mPluginActions.size(); idx > DkActionManager::menu_plugins_end; idx--) {
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

 	QVector<QSharedPointer<DkPluginContainer> > loadedPlugins = DkPluginManager::instance().getPlugins();
	qSort(loadedPlugins);

	mPluginSubMenus.clear();

	QStringList pluginMenu = QStringList();

	for (auto plugin : loadedPlugins) {

		DkPluginInterface* pi = plugin->plugin();

		if (pi && plugin->pluginMenu()) {
			QList<QAction*> actions = pi->createActions(QApplication::activeWindow());
			mPluginSubMenus.append(plugin->pluginMenu());
			mMenu->addMenu(plugin->pluginMenu());
		}
		else if (pi) {
			QAction* a = new QAction(plugin->pluginName(), this);
			a->setData(pi->id());
			mPluginActions.append(a);
			mMenu->addAction(a);
			connect(a, SIGNAL(triggered()), plugin.data(), SLOT(run()));
		}
	}

	mMenu->addSeparator();
	mMenu->addAction(DkActionManager::instance().action(DkActionManager::menu_plugin_manager));

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

//void DkPluginActionManager::runLoadedPlugin() {
//
//	QAction* action = qobject_cast<QAction*>(sender());
//
//	if (!action)
//		return;
//
//	emit applyPluginChanges(true);
//	QApplication::sendPostedEvents();
//
//	QString id = action->data().toString();
//
//	DkPluginInterface* cPlugin = DkPluginManager::instance().runPlugin(id);
//
//	if (cPlugin && cPlugin->interfaceType() == DkPluginInterface::interface_viewport) {
//
//		DkViewPortInterface* vPlugin = dynamic_cast<DkViewPortInterface*>(cPlugin);
//
//		if(!vPlugin || !vPlugin->getViewPort()) 
//			return;
//
//		connect(vPlugin->getViewPort(), SIGNAL(showToolbar(QToolBar*, bool)), vPlugin->getMainWindow(), SLOT(showToolbar(QToolBar*, bool)));
//
//		emit runPlugin(vPlugin, false);
//	}
//	else if (cPlugin && cPlugin->interfaceType() == DkPluginInterface::interface_basic) {
//		emit runPlugin(cPlugin, id);
//	}
//}

void DkPluginActionManager::savePluginActions(QVector<QAction *> actions) const {

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup("CustomPluginShortcuts");
	settings.remove("");
	for (int i = 0; i < actions.size(); i++)
		settings.setValue(actions.at(i)->text(), actions.at(i)->text());
	settings.endGroup();
}


};

