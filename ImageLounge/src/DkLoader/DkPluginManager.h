/*******************************************************************************************************
 DkPluginManager.h
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

#pragma once

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QDialog>
#include <QMap>
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QTextEdit>
#include <QLabel>
#pragma warning(pop)		// no warnings from includes - end

#include "DkPluginInterface.h"

// Qt defines
class QNetworkReply;
class QNetworkAccessManager;
class QPluginLoader;
class QModelIndex;
class QTableView;
class QItemSelectionModel;
class QItemSelection;
class QProgressDialog;
class QSortFilterProxyModel;

namespace nmc {

// nomacs defines
class DkPluginTableWidget;
class DkInstalledPluginsModel;
class DkPluginDownloader;

enum pluginManagerTabs {
	tab_installed_plugins,
	tab_download_plugins,
};

enum installedPluginsColumns {
	ip_column_name,
	ip_column_version,
	ip_column_enabled,
	ip_column_uninstall,
	ip_column_size,
};

enum downloadPluginsColumns {
	dp_column_name,
	dp_column_version,
	dp_column_install,
	dp_column_size,
};

enum pluginRequestType {
	request_none,
	request_xml,
	request_xml_for_update,
	request_preview,
	request_plugin,
	request_plugin_files_list,
};

enum xmlUsage {
	xml_usage_update,
	xml_usage_download,
};

struct XmlPluginData {
	QString id;
	QString name;
	QString version;
	QString decription;
	QString previewImgUrl;
	bool isWin64;
	bool isWin86;
};

struct QPairFirstComparer {
	template<typename T1, typename T2>
	bool operator()(const QPair<T1,T2> & a, const QPair<T1,T2> & b) const {
		return a.first < b.first;
	}
};

struct QPairSecondComparer {
	template<typename T1, typename T2>
	bool operator()(const QPair<T1,T2> & a, const QPair<T1,T2> & b) const {
		return a.second < b.second;
	}
};

class DllExport DkPluginActionManager : public QObject {
	Q_OBJECT

public:
	DkPluginActionManager(QObject* parent = 0);

	void setMenu(QMenu* menu);
	QMenu* menu() const;

	QVector<QAction*> pluginDummyActions() const;
	QVector<QAction*> pluginActions() const;
	QVector<QMenu*> pluginSubMenus() const;

public slots:
	void runLoadedPlugin();
	void runPluginFromShortcut();
	void addPluginsToMenu();
	void updateMenu();

signals:
	void runPlugin(DkViewPortInterface* plugin, bool close) const;
	void runPlugin(DkPluginInterface* plugin, const QString& key) const;
	void applyPluginChanges(bool askForSaving) const;

protected:
	void assignCustomPluginShortcuts();

	void savePluginActions(QVector<QAction *> actions) const;

	QVector<QAction *> mPluginActions;
	QVector<QAction *> mPluginDummyActions;
	QMenu* mMenu = 0;
	QVector<QMenu*> mPluginSubMenus;
};

class DkPluginManager {

public:
	static DkPluginManager& instance();
	~DkPluginManager();

	void addPlugin(const QString& pluginId, const QString& filePath, DkPluginInterface* plugin);
	QMap<QString, DkPluginInterface *> getPlugins() const;
	DkPluginInterface* getPlugin(const QString& key) const;
	DkPluginInterface* getPluginByName(const QString& pluginName) const;
	QString actionNameToRunId(const QString& pluginId, const QString& actionName) const;
	
	QString getPluginFilePath(const QString& key) const;
	QMap<QString, QString> getPluginFilePaths() const;
	
	QMap<QString, QString> getRunId2PluginId() const;
	void setRunId2PluginId(QMap<QString, QString> newMap);
	
	QList<QString> getPluginIdList() const;
	void setPluginIdList(QList<QString> newPlugins);

	void removePlugin(const QString& id);
	void clear();

	void saveSettings() const;
	void loadPlugins();

	bool singlePluginLoad(const QString& filePath);

	QVector<DkPluginInterface*> getBasicPlugins() const;

	// functions for active plugin
	QString getRunningPluginKey() const {return mRunningPlugin; };
	void clearRunningPluginKey();
	DkPluginInterface* getRunningPlugin() const;
	DkPluginInterface* runPlugin(const QString& key);

private:
	DkPluginManager();

	QMap<QString, DkPluginInterface *> loadedPlugins;
	QMap<QString, QString> pluginFiles;
	QList<QString> pluginIdList;
	QMap<QString, QString> runId2PluginId;
	QMap<QString, QPluginLoader *> pluginLoaders;	// needed for unloading plug-ins when uninstalling them

	QString mRunningPlugin;
};

// Plug-in manager dialog for enabling/disabling plug-ins and downloading new ones
class DkPluginManagerDialog : public QDialog {
	Q_OBJECT

public:
	DkPluginManagerDialog(QWidget* parent = 0);
	~DkPluginManagerDialog();
		
	void loadPlugins();
	void deletePlugin(QString pluginID);
	void deleteInstance(QString id);
	QMap<QString, QString> getPreviouslyInstalledPlugins();

protected slots:
	void closePressed();
	void tabChanged(int tab);

protected:
	int dialogWidth;
	int dialogHeight;
	
	QTabWidget* tabs;
	DkPluginTableWidget* tableWidgetInstalled;
	DkPluginTableWidget* tableWidgetDownload;
	QMap<QString, QString> previouslyInstalledPlugins;

	void init();
	void createLayout();
	void showEvent(QShowEvent *event);
	void loadPreviouslyInstalledPluginsList();
};

// widget with all plug-in information
class DkPluginTableWidget: public QWidget {

Q_OBJECT

public:    
	DkPluginTableWidget(int tab, DkPluginManagerDialog* manager, QWidget* parent);
	~ DkPluginTableWidget();

	void clearTableFilters();
	void updateInstalledModel();
	void downloadPluginInformation(int usage);
	DkPluginManagerDialog* getPluginManager();
	int getOpenedTab();
	DkPluginDownloader* getDownloader();
	void getPluginUpdateData();
	
public slots:
	void uninstallPlugin(const QModelIndex &index);
	void installPlugin(const QModelIndex &index);
	void pluginInstalled(const QModelIndex &index);
	void pluginUpdateFinished(bool finishedSuccessfully);
	void reloadPlugins();

private:
	void createLayout();
	void fillDownloadTable();	
	void getListOfUpdates();

	int mOpenedTab = 0;
	DkPluginManagerDialog* mPluginManager = 0;
	QSortFilterProxyModel *mProxyModel = 0;
	QAbstractTableModel* mModel = 0;
	DkPluginDownloader* mPluginDownloader = 0;
	QTableView* mTableView = 0;
	QLineEdit* mFilterEdit = 0;
	QList<XmlPluginData> mPluginsToUpdate;
	QPushButton* mUpdateButton = 0;

protected slots:
	void showDownloaderMessage(QString msg, QString title);
	void manageParsedXmlData(int usage);
	void filterTextChanged();
	void updateSelectedPlugins();	
};

// model for the table in the installed plug-ins tab 
class DkInstalledPluginsModel : public QAbstractTableModel {

Q_OBJECT

public:
    DkInstalledPluginsModel(QObject *parent=0);
    DkInstalledPluginsModel(QList<QString> data, QObject *parent=0);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());
	
	QList<QString> getPluginData();
	void setDataToInsert(QString newData);
	void setEnabledData(QMap<QString, bool> enabledData);
	QMap<QString, bool> getEnabledData();
	void loadPluginsEnabledSettings();
	void savePluginsEnabledSettings();
	
private:
	QList<QString> mPluginData;
	QMap<QString, bool> mPluginsEnabled;
	QString mDataToInsert;
	DkPluginTableWidget* mParentTable = 0;

};

class DkDownloadPluginsModel : public QAbstractTableModel {

Q_OBJECT

public:
    DkDownloadPluginsModel(QObject *parent=0);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());

	void updateInstalledData(const QModelIndex &index, bool installed);	
	QList<XmlPluginData> getPluginData();
	void setDataToInsert(XmlPluginData newData);
	void setInstalledData(QMap<QString, bool> installedData);
	QMap<QString, bool> getInstalledData();
	
private:
	QList<XmlPluginData> mPluginData;
	QMap<QString, bool> mPluginsInstalled;
	XmlPluginData mDataToInsert;
	DkPluginTableWidget* mParentTable = 0;
};


// checkbox delegate based on code from http://stackoverflow.com/questions/3363190/qt-qtableview-how-to-have-a-checkbox-only-column
class DkPluginCheckBoxDelegate : public QStyledItemDelegate {
	Q_OBJECT

public:
	DkPluginCheckBoxDelegate(QObject *parent = 0);
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

private:
	QTableView* mParentTable = 0;
};

// pushbutton delegate : adds mButtons in the table column
class DkPushButtonDelegate : public QStyledItemDelegate {
	Q_OBJECT

public:
	DkPushButtonDelegate(QObject *parent = 0);
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

signals:
    void buttonClicked(const QModelIndex &index) const;

private:
	QTableView* mParentTable = 0;
	int mCRow = -1;
	QStyle::State mPushButonState = QStyle::State_Enabled;
};

// delegate for download table: icon if already downloaded or button if not
class DkDownloadDelegate : public QStyledItemDelegate {

Q_OBJECT

public:
	DkDownloadDelegate(QObject *parent = 0);
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

signals:
    void buttonClicked(const QModelIndex &index) const;

private:
	QTableView* mParentTable = 0;
	int mCRow = -1;
	QStyle::State mPushButtonState = QStyle::State_Enabled;
};

// text edit connected to tables selection
class DkDescriptionEdit : public QTextEdit {

Q_OBJECT

public:
	DkDescriptionEdit(QAbstractTableModel* data, QSortFilterProxyModel* proxy, QItemSelectionModel* selection, QWidget *parent = 0);

protected slots:
	void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
	void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
	QAbstractTableModel* mDataModel = 0;
	QSortFilterProxyModel* mProxyModel = 0;
	QItemSelectionModel* mSelectionModel = 0;
	DkPluginTableWidget* mParentTable = 0;
	QString mDefaultString;
	void updateText();
};

// label connected to table selections
class DkDescriptionImage : public QLabel {

Q_OBJECT

public:
	DkDescriptionImage(QAbstractTableModel* data, QSortFilterProxyModel* proxy, QItemSelectionModel* selection, QWidget *parent = 0);

protected slots:
	void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
	void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
	void updateImageFromReply(QImage img);

private:
	QAbstractTableModel* mDataModel = 0;
	QSortFilterProxyModel* mProxyModel = 0;
	QItemSelectionModel* mSelectionModel = 0;
	DkPluginTableWidget* mParentTable = 0;
	QImage mDefaultImage;
	void updateImage();
};

// plugin xml data parser and downloadr
class DkPluginDownloader : public QObject {
	Q_OBJECT;

public:	
	DkPluginDownloader(QWidget* parent);

	void downloadXml(int usage);
	void downloadPreviewImg(QString url);
	void downloadPlugin(const QModelIndex &index, QString url, QString pluginName);
	void updatePlugins(QList<QString> urls);
	QList<XmlPluginData> getXmlPluginData();

signals:
	void showDownloaderMessage(QString msg, QString title) const;
	void parsingFinished(int usage) const;
	void imageDownloaded(QImage img) const;
	void pluginDownloaded(const QModelIndex &index) const;
	void allPluginsUpdated(bool finishedSuccessfully) const;
	void pluginFilesDownloadingFinished() const;
	void reloadPlugins() const;

protected slots:
	void replyFinished(QNetworkReply*);
	void updateDownloadProgress(qint64 received, qint64 total);
	void cancelUpdate();

private:
	QNetworkAccessManager* mAccessManagerPlugin;
	QNetworkReply* mReply;
	QProgressDialog* mProgressDialog;
	bool mDownloadAborted = false;
	QList<XmlPluginData> mXmlPluginData;
	int mRequestType;
	QString mFileName;
	int mCurrUsage;
	QStringList mFilesToDownload;

	void parseXml(QNetworkReply* reply);
	void replyToImg(QNetworkReply* reply);
	void startPluginDownload(QNetworkReply* reply);
	void parseFileList(QNetworkReply* reply);
	void downloadSingleFile(QString url);
	void downloadPluginFileList(QString url);
	void createProgressDialog();
};


};


