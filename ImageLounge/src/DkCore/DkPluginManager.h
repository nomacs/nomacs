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
 along with this program. If not, see <http://www.gnu.org/licenses/>.

 *******************************************************************************************************/

#pragma once

#pragma warning(push, 0) // no warnings from includes - begin
#include <QAbstractTableModel>
#include <QDate>
#include <QDialog>
#include <QLabel>
#include <QLibrary>
#include <QMap>
#include <QStyledItemDelegate>
#include <QTextEdit>
#pragma warning(pop) // no warnings from includes - end

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
class QJsonValue;

namespace nmc
{

// nomacs defines
class DkPluginTableWidget;
class DkInstalledPluginsModel;
class DkPluginDownloader;

enum installedPluginsColumns {
    ip_column_name,
    ip_column_version,
    ip_column_uninstall,
    ip_column_size,
};

class DkLibrary
{
public:
    DkLibrary(const QString &name = QString());

    QString fullPath() const;
    QString name() const;

    bool isLoaded() const;

    bool load();
    bool uninstall();

protected:
    QString mFullPath;
    QString mName;

    QSharedPointer<QLibrary> mLib;
    QVector<DkLibrary> mDependencies;

    QVector<DkLibrary> loadDependencies() const;
};

class DllCoreExport DkPluginContainer : public QObject
{
    Q_OBJECT

public:
    DkPluginContainer(const QString &pluginPath);
    ~DkPluginContainer();

    enum PluginType {
        type_unknown = 0,
        type_simple,
        type_batch,
        type_viewport,

        type_end
    };

    friend bool operator<(const QSharedPointer<DkPluginContainer> &l, const QSharedPointer<DkPluginContainer> &r);

    void setActive(bool active = true);
    bool isActive() const;

    bool isValid() const;
    bool isLoaded() const;
    bool load();
    bool uninstall();

    // attributes
    QString pluginPath() const;
    QString pluginName() const;
    QString authorName() const;
    QString company() const;
    QString version() const;
    QString description() const;
    QString fullDescription() const;
    QString tagline() const;
    QString id() const;

    QDate dateCreated() const;
    QDate dateModified() const;

    QMenu *pluginMenu() const;

    QSharedPointer<QPluginLoader> loader() const;
    DkPluginInterface *plugin() const;
    DkBatchPluginInterface *batchPlugin() const;
    DkViewPortInterface *pluginViewPort() const;
    QString actionNameToRunId(const QString &actionName) const;

signals:
    void runPlugin(DkViewPortInterface *viewport, bool close) const;
    void runPlugin(DkPluginContainer *plugin, const QString &key) const;

public slots:
    void run();

protected:
    QString mPluginPath;
    QString mPluginName;
    QString mAuthorName;
    QString mCompany;
    QString mDescription;
    QString mVersion;
    QString mTagline;
    QString mId;

    QDate mDateCreated;
    QDate mDateModified;

    bool mActive = false;
    bool mIsValid = false;

    PluginType mType = type_unknown;

    QMenu *mPluginMenu = 0;

    QSharedPointer<QPluginLoader> mLoader = QSharedPointer<QPluginLoader>();

    void createMenu();
    void loadJson();
    void loadMetaData(const QJsonValue &val);
};

class DllCoreExport DkPluginActionManager : public QObject
{
    Q_OBJECT

public:
    DkPluginActionManager(QObject *parent = 0);

    void setMenu(QMenu *menu);
    QMenu *menu() const;

    QVector<QAction *> pluginDummyActions() const;
    QVector<QAction *> pluginActions() const;
    QVector<QMenu *> pluginSubMenus() const;

    static void createPluginsPath();

public slots:
    // void runLoadedPlugin();
    void runPluginFromShortcut();
    void addPluginsToMenu();
    void updateMenu();

signals:
    void runPlugin(DkViewPortInterface *plugin, bool close) const;
    void runPlugin(DkPluginContainer *plugin, const QString &key) const;
    void applyPluginChanges(bool askForSaving) const;
    void showViewPort() const;

protected:
    void assignCustomPluginShortcuts();
    void savePluginActions(QVector<QAction *> actions) const;

    QVector<QAction *> mPluginActions;
    QVector<QAction *> mPluginDummyActions;
    QMenu *mMenu = 0;
    QVector<QMenu *> mPluginSubMenus;
};

class DllCoreExport DkPluginManager
{
public:
    static DkPluginManager &instance();
    ~DkPluginManager();

    // singleton
    DkPluginManager(DkPluginManager const &) = delete;
    void operator=(DkPluginManager const &) = delete;

    QVector<QSharedPointer<DkPluginContainer>> getPlugins() const;
    QSharedPointer<DkPluginContainer> getPluginByName(const QString &pluginName) const;
    QSharedPointer<DkPluginContainer> getPluginByPath(const QString &path) const;

    void reload();
    void removePlugin(QSharedPointer<DkPluginContainer> plugin);
    bool deletePlugin(QSharedPointer<DkPluginContainer> plugin);
    void clear();

    void loadPlugins();

    bool singlePluginLoad(const QString &filePath);

    QVector<QSharedPointer<DkPluginContainer>> getBasicPlugins() const;
    QVector<QSharedPointer<DkPluginContainer>> getBatchPlugins() const;

    // functions for active plugin
    void clearRunningPlugin();
    QSharedPointer<DkPluginContainer> getRunningPlugin() const;
    void runPlugin(QSharedPointer<DkPluginContainer> plugin);

    bool isBlackListed(const QString &pluginPath) const;
    static QStringList blackList();
    static void createPluginsPath();

private:
    DkPluginManager();

    QVector<QSharedPointer<DkPluginContainer>> mPlugins;
};

// Plug-in manager dialog for enabling/disabling plug-ins and downloading new ones
class DllCoreExport DkPluginManagerDialog : public QDialog
{
    Q_OBJECT

public:
    DkPluginManagerDialog(QWidget *parent = 0);
    ~DkPluginManagerDialog();

    void deleteInstance(QSharedPointer<DkPluginContainer> plugin);
    QMap<QString, QString> getPreviouslyInstalledPlugins();

protected slots:
    void closePressed();

protected:
    int dialogWidth;
    int dialogHeight;

    DkPluginTableWidget *tableWidgetInstalled;
    QMap<QString, QString> previouslyInstalledPlugins;

    void init();
    void createLayout();
    void showEvent(QShowEvent *event) override;
};

// widget with all plug-in information
class DkPluginTableWidget : public DkWidget
{
    Q_OBJECT

public:
    DkPluginTableWidget(QWidget *parent);
    ~DkPluginTableWidget();

    void clearTableFilters();
    void updateInstalledModel();

public slots:
    void uninstallPlugin(const QModelIndex &index);
    void reloadPlugins();

private:
    void createLayout();

    DkPluginManagerDialog *mPluginManager = 0;
    QSortFilterProxyModel *mProxyModel = 0;
    QAbstractTableModel *mModel = 0;
    QTableView *mTableView = 0;
    QLineEdit *mFilterEdit = 0;

protected slots:
    void filterTextChanged();
    // void on_updateButton_clicked();
};

// model for the table in the installed plug-ins tab
class DkInstalledPluginsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    DkInstalledPluginsModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;

    void setDataToInsert(QSharedPointer<DkPluginContainer> newData);

private:
    QSharedPointer<DkPluginContainer> mPluginToInsert;
    DkPluginTableWidget *mParentTable = 0;
};

// checkbox delegate based on code from http://stackoverflow.com/questions/3363190/qt-qtableview-how-to-have-a-checkbox-only-column
class DkPluginCheckBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    DkPluginCheckBoxDelegate(QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

private:
    QTableView *mParentTable = 0;
};

// pushbutton delegate : adds mButtons in the table column
class DkPushButtonDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    DkPushButtonDelegate(QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

signals:
    void buttonClicked(const QModelIndex &index) const;

private:
    QTableView *mParentTable = 0;
    int mCRow = -1;
    QStyle::State mPushButonState = QStyle::State_Enabled;
};

// text edit connected to tables selection
class DkDescriptionEdit : public QTextEdit
{
    Q_OBJECT

public:
    DkDescriptionEdit(QAbstractTableModel *data, QSortFilterProxyModel *proxy, QItemSelectionModel *selection, QWidget *parent = 0);

protected slots:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    QAbstractTableModel *mDataModel = 0;
    QSortFilterProxyModel *mProxyModel = 0;
    QItemSelectionModel *mSelectionModel = 0;
    DkPluginTableWidget *mParentTable = 0;
    void updateText();
};

// label connected to table selections
class DkDescriptionImage : public QLabel
{
    Q_OBJECT

public:
    DkDescriptionImage(QAbstractTableModel *data, QSortFilterProxyModel *proxy, QItemSelectionModel *selection, QWidget *parent = 0);

protected slots:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    QAbstractTableModel *mDataModel = 0;
    QSortFilterProxyModel *mProxyModel = 0;
    QItemSelectionModel *mSelectionModel = 0;
    DkPluginTableWidget *mParentTable = 0;
    QPixmap mDefaultImage;
    void updateImage();
};

}
