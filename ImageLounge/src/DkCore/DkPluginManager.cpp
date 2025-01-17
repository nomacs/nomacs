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
#include "DkActionManager.h"
#include "DkDependencyResolver.h"
#include "DkSettings.h"
#include "DkTimer.h"
#include "DkUpdater.h"
#include "DkUtils.h"

#pragma warning(push, 0) // no warnings from includes - begin
#include <QAction>
#include <QDebug>
#include <QDir>
#include <QHeaderView>
#include <QJsonValue>
#include <QLibraryInfo>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QNetworkAccessManager>
#include <QNetworkProxyFactory>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPluginLoader>
#include <QProgressDialog>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSettings>
#include <QSlider>
#include <QSortFilterProxyModel>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableView>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <QXmlStreamReader>
#include <QtGlobal>
#pragma warning(pop) // no warnings from includes - end

#ifdef QT_NO_DEBUG_OUTPUT
#pragma warning(disable : 4127) // no 'conditional expression is constant' if qDebug() messages are removed
#endif

namespace nmc
{

// DkLibrary --------------------------------------------------------------------
DkLibrary::DkLibrary(const QString &name)
{
    mName = name;
}

QString DkLibrary::fullPath() const
{
    return mFullPath;
}

QString DkLibrary::name() const
{
    return mName;
}

bool DkLibrary::isLoaded() const
{
    if (mLib)
        return mLib->isLoaded();

    return false;
}

bool DkLibrary::load()
{
    QString suffix;
    QString prefix;

#if defined(Q_OS_LINUX) // TODO: add your operating system if libs are prefixed
    prefix = "lib";
#endif

    mLib = QSharedPointer<QLibrary>(new QLibrary());

    for (const QString &libPath : QCoreApplication::libraryPaths()) {
        QString fullPath = libPath + QDir::separator() + prefix + mName + suffix;
        mLib->setFileName(fullPath);
        mLib->load();

        if (mLib->isLoaded()) {
            mFullPath = fullPath;
            break;
        }

        // just needed for windows dlls
        if (QFileInfo(fullPath).exists())
            mFullPath = fullPath;
    }

    // if we could find the library but not load it, there is probably a dependency missing...
    if (!mLib->isLoaded() && !mFullPath.isEmpty()) {
        mDependencies = loadDependencies();

        mLib->setFileName(mFullPath);
        return mLib->load();
    } else if (!mLib->isLoaded())
        return false;

    return true;
}

bool DkLibrary::uninstall()
{
    if (mLib)
        mLib->unload();

    return QFile::remove(fullPath());
}

QVector<DkLibrary> DkLibrary::loadDependencies() const
{
    QVector<DkLibrary> dependencies;
    DkDllDependency d(mFullPath);

    if (!d.findDependencies()) {
        qDebug() << "sorry, but I could not find dependencies of" << mName;
        return dependencies;
    }

    // debug:
    // qInfo() << name() << "has these dependencies: " << d.dependencies();

    QStringList fd = d.filteredDependencies();

    for (const QString &n : fd) {
        DkLibrary lib(n);
        if (lib.load()) {
            dependencies << lib;
            // qInfo() << lib.name() << "loaded for" << name();
        } else
            qWarning() << "could not load" << lib.name() << "which is needed for" << name();
    }

    return dependencies;
}

// DkPluginContainer --------------------------------------------------------------------
DkPluginContainer::DkPluginContainer(const QString &pluginPath)
{
    mPluginPath = pluginPath;
    mLoader = QSharedPointer<QPluginLoader>(new QPluginLoader(mPluginPath));
    loadJson();
}

DkPluginContainer::~DkPluginContainer()
{
}

bool operator<(const QSharedPointer<DkPluginContainer> &l, const QSharedPointer<DkPluginContainer> &r)
{
    if (!l || !r)
        return false;

    return l->pluginName() < r->pluginName();
}

void DkPluginContainer::setActive(bool active)
{
    mActive = active;

    DkPluginInterface *p = plugin();
    if (p && p->interfaceType() == DkPluginInterface::interface_viewport) {
        DkViewPortInterface *vPlugin = pluginViewPort();

        if (!vPlugin)
            return;

        vPlugin->setVisible(false);
    }
}

bool DkPluginContainer::isActive() const
{
    return mActive;
}

bool DkPluginContainer::isLoaded() const
{
    return mLoader->isLoaded();
}

bool DkPluginContainer::load()
{
    DkTimer dt;

    if (!isValid()) {
        // inform that we have found a dll that does not fit what we expect
        if (!mPluginPath.contains("opencv") && !mPluginPath.contains("Read"))
#ifdef Q_OS_WIN
            if (mPluginPath.contains("dll"))
#endif
                qInfo() << "Invalid: " << mPluginPath;
        return false;
    } else {
        QString fn = QFileInfo(mLoader->fileName()).fileName();

#ifdef Q_OS_WIN
        // ok - load it's dependencies first
        DkLibrary l(fn);
        l.load();
#endif

        if (!mLoader->load()) {
            qWarning() << "Could not load:" << fn;
            qInfo() << "name: " << mPluginName;
            qInfo() << "modified: " << mDateModified.toString("dd-MM-yyyy");
            qInfo() << "error: " << mLoader->errorString();
            return false;
        }
    }

    if (pluginViewPort())
        mType = type_viewport;
    else if (batchPlugin()) {
        // load the settings
        mType = type_batch;
        batchPlugin()->loadSettings();
    } else if (plugin())
        mType = type_simple;
    else {
        qWarning() << "could not initialize: " << mPluginPath << "unknown interface";
        return false;
    }

    if (mType != type_unknown) {
        // init actions
        plugin()->createActions(DkUtils::getMainWindow());
        createMenu();
    }

    qInfo() << mPluginPath << "loaded in" << dt;
    return true;
}

bool DkPluginContainer::uninstall()
{
    mLoader->unload();
    // NOTE: dependencies are not removed yet -> they might be used by other plugins

    return QFile::remove(mPluginPath);
}

void DkPluginContainer::createMenu()
{
    DkPluginInterface *p = plugin();

    // empty menu if we do not have any actions
    if (!p || p->pluginActions().empty())
        return;

    mPluginMenu = new QMenu(pluginName(), DkUtils::getMainWindow());

    for (auto action : p->pluginActions()) {
        mPluginMenu->addAction(action);
        connect(action, &QAction::triggered, this, &DkPluginContainer::run, Qt::UniqueConnection);
    }
}

void DkPluginContainer::loadJson()
{
    QJsonObject metaData = mLoader->metaData();
    QStringList keys = metaData.keys();

    for (const QString &key : keys) {
        if (key == "MetaData")
            loadMetaData(metaData.value(key));
        else if (key == "IID" && metaData.value(key).toString().contains("com.nomacs.ImageLounge"))
            mIsValid = true;
#ifndef _DEBUG // warn if we have a debug & are not in debug ourselves
        else if (key == "debug") {
            bool isDebug = metaData.value(key).toBool();
            if (isDebug)
                qWarning() << "I cannot load a debug dll since I am compiled in release!";
        }
#endif

        // qDebug() << key << "|" << metaData.value(key);
    }
}

void DkPluginContainer::loadMetaData(const QJsonValue &val)
{
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

    for (const QString &key : keys) {
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
            mTagline = metaData.value(key).toString();
        else if (key == "Version")
            mVersion = metaData.value(key).toString();
        else if (key == "PluginId") {
            mId = metaData.value(key).toString();
        } else
            qWarning() << "unknown key" << key << "|" << metaData.value(key);

        // qDebug() << "parsing:" << key << "|" << metaData.value(key);
    }

    if (!isValid() && !keys.empty()) {
        qWarning() << "invalid plugin - missing the PluginName in the json metadata...";
    }
}

void DkPluginContainer::run()
{
    DkPluginInterface *p = plugin();

    if (p && p->interfaceType() == DkPluginInterface::interface_viewport) {
        // init viewport (if no image is there already)
        emit DkActionManager::instance().pluginActionManager()->showViewPort();

        DkViewPortInterface *vPlugin = pluginViewPort();
        mActive = true;

        if (!vPlugin)
            return;

        if (!vPlugin->getViewPort())
            vPlugin->createViewPort(vPlugin->getMainWindow());

        if (!vPlugin->getViewPort()) {
            qWarning() << "NULL viewport detected in" << mPluginName;
            return;
        }

        vPlugin->setVisible(true);
        emit runPlugin(vPlugin, false);
    } else if (p && (p->interfaceType() == DkPluginInterface::interface_basic || p->interfaceType() == DkPluginInterface::interface_batch)) {
        QAction *a = qobject_cast<QAction *>(QObject::sender());

        if (a)
            emit runPlugin(this, a->data().toString());
    } else
        qWarning() << "plugin with illegal interface detected in DkPluginContainer::run()";
}

bool DkPluginContainer::isValid() const
{
    return mIsValid;
}

QString DkPluginContainer::pluginPath() const
{
    return mPluginPath;
}

QString DkPluginContainer::pluginName() const
{
    return mPluginName;
}

QString DkPluginContainer::authorName() const
{
    return mAuthorName;
}

QString DkPluginContainer::company() const
{
    return mCompany;
}

QString DkPluginContainer::version() const
{
    return mVersion;
}

QString DkPluginContainer::description() const
{
    return mDescription;
}

QString DkPluginContainer::fullDescription() const
{
    QString trAuthor = tr("Author:");
    QString trCompany = tr("Company:");
    QString trCreated = tr("Created:");
    QString trModified = tr("Last Modified:");

    QString fs;

    fs += "<h3>" + pluginName() + "</h3>";

    if (!tagline().isEmpty())
        fs += "<i>" + tagline() + "</i>";
    fs += "<p>" + mDescription + "</p>";
    fs += "<b>" + trAuthor + "</b> " + mAuthorName + "<br>";

    if (!company().isEmpty())
        fs += "<b>" + trCompany + "</b> " + company() + "<br>";

    fs += "<b>" + trCreated + "</b> " + mDateCreated.toString() + "<br>";
    fs += "<b>" + trModified + "</b> " + mDateModified.toString() + "<br>";

    return fs;
}

QString DkPluginContainer::tagline() const
{
    return mTagline;
}

QString DkPluginContainer::id() const
{
    return mId;
}

QDate DkPluginContainer::dateCreated() const
{
    return mDateCreated;
}

QDate DkPluginContainer::dateModified() const
{
    return mDateModified;
}

QMenu *DkPluginContainer::pluginMenu() const
{
    return mPluginMenu;
}

QSharedPointer<QPluginLoader> DkPluginContainer::loader() const
{
    return mLoader;
}

DkPluginInterface *DkPluginContainer::plugin() const
{
    // is everything fine here??
    if (!mLoader)
        return 0;

    DkPluginInterface *pi = qobject_cast<DkPluginInterface *>(mLoader->instance());

    if (!pi && pluginViewPort())
        return pluginViewPort();
    else if (!pi && batchPlugin())
        return batchPlugin();

    return pi;
}

DkBatchPluginInterface *DkPluginContainer::batchPlugin() const
{
    // is everything fine here??
    if (!mLoader)
        return 0;

    return qobject_cast<DkBatchPluginInterface *>(mLoader->instance());
}

DkViewPortInterface *DkPluginContainer::pluginViewPort() const
{
    // is everything fine here??
    if (!mLoader)
        return 0;

    return qobject_cast<DkViewPortInterface *>(mLoader->instance());
}

QString DkPluginContainer::actionNameToRunId(const QString &actionName) const
{
    if (!plugin())
        return QString();

    QList<QAction *> actions = plugin()->pluginActions();
    for (const QAction *a : actions) {
        if (a->text() == actionName)
            return a->data().toString();
    }

    return QString();
}

/**********************************************************************************
 * Plugin manager dialog
 **********************************************************************************/
DkPluginManagerDialog::DkPluginManagerDialog(QWidget *parent)
    : QDialog(parent)
{
    init();
}

DkPluginManagerDialog::~DkPluginManagerDialog()
{
}

/**
 * initialize plugin manager dialog - set sizes
 **/
void DkPluginManagerDialog::init()
{
    dialogWidth = 700;
    dialogHeight = 500;

    setWindowTitle(tr("Plugin Manager"));
    setMinimumSize(dialogWidth, dialogHeight);
    createLayout();
}

/*
 * create plugin manager dialog layout
 **/
void DkPluginManagerDialog::createLayout()
{
    tableWidgetInstalled = new DkPluginTableWidget(this);

    QPushButton *buttonClose = new QPushButton(tr("&Close"));
    connect(buttonClose, &QPushButton::clicked, this, &DkPluginManagerDialog::closePressed);
    buttonClose->setDefault(true);

    QWidget *dummy = new QWidget(this);
    QHBoxLayout *hLayout = new QHBoxLayout(dummy);
    hLayout->setAlignment(Qt::AlignRight);
    hLayout->addWidget(buttonClose);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tableWidgetInstalled);
    layout->addWidget(dummy);
}

void DkPluginManagerDialog::closePressed()
{
    close();
}

void DkPluginManagerDialog::showEvent(QShowEvent *ev)
{
    qDebug() << "show event called...";
    DkPluginManager::instance().loadPlugins();

    tableWidgetInstalled->clearTableFilters();
    tableWidgetInstalled->updateInstalledModel();

    QDialog::showEvent(ev);
}

void DkPluginManagerDialog::deleteInstance(QSharedPointer<DkPluginContainer> plugin)
{
    DkPluginManager::instance().removePlugin(plugin);
}

QMap<QString, QString> DkPluginManagerDialog::getPreviouslyInstalledPlugins()
{
    return previouslyInstalledPlugins;
}

/**********************************************************************************
 * DkPluginTableWidget : Widget with table views containing plugin data
 **********************************************************************************/
DkPluginTableWidget::DkPluginTableWidget(QWidget *parent)
    : DkWidget(parent)
{
    createLayout();
}

DkPluginTableWidget::~DkPluginTableWidget()
{
}

// create the main layout of the plugin manager
void DkPluginTableWidget::createLayout()
{
    // search line edit and update button
    mFilterEdit = new QLineEdit(this);
    mFilterEdit->setPlaceholderText(tr("Search plugins"));
    connect(mFilterEdit, &QLineEdit::textChanged, this, &DkPluginTableWidget::filterTextChanged);

    // QPushButton* updateButton = new QPushButton(tr("Add or Remove Plugins"), this);
    // updateButton->setObjectName("updateButton");
    // if (DkSettingsManager::param().isPortable())
    //	updateButton->hide();

    QWidget *searchWidget = new QWidget(this);
    QHBoxLayout *sLayout = new QHBoxLayout(searchWidget);
    sLayout->setContentsMargins(0, 0, 0, 0);
    sLayout->addWidget(mFilterEdit);
    // sLayout->addStretch();
    // sLayout->addWidget(updateButton);

    // main table
    mTableView = new QTableView(this);
    mProxyModel = new QSortFilterProxyModel(this);
    mProxyModel->setDynamicSortFilter(true);

    mModel = new DkInstalledPluginsModel(this);
    mProxyModel->setSourceModel(mModel);
    mTableView->setModel(mProxyModel);
    mTableView->resizeColumnsToContents();
    mTableView->setColumnWidth(ip_column_name, qMax(mTableView->columnWidth(ip_column_name), 300));
    mTableView->setColumnWidth(ip_column_version, qMax(mTableView->columnWidth(ip_column_version), 80));

    mTableView->resizeRowsToContents();
    mTableView->horizontalHeader()->setStretchLastSection(true);
    mTableView->setSortingEnabled(true);
    mTableView->sortByColumn(ip_column_name, Qt::AscendingOrder);
    mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    mTableView->verticalHeader()->hide();
    mTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    mTableView->setAlternatingRowColors(true);

    if (DkSettingsManager::instance().param().isPortable()) {
        DkPushButtonDelegate *buttonDelegate = new DkPushButtonDelegate(mTableView);
        mTableView->setItemDelegateForColumn(ip_column_uninstall, buttonDelegate);
        connect(buttonDelegate, &DkPushButtonDelegate::buttonClicked, this, &DkPluginTableWidget::uninstallPlugin);
    }

    DkDescriptionEdit *descriptionEdit = new DkDescriptionEdit(mModel, mProxyModel, mTableView->selectionModel(), this);
    connect(mTableView->selectionModel(), &QItemSelectionModel::selectionChanged, descriptionEdit, &DkDescriptionEdit::selectionChanged);
    connect(mProxyModel, &QSortFilterProxyModel::dataChanged, descriptionEdit, &DkDescriptionEdit::dataChanged);

    DkDescriptionImage *descriptionImg = new DkDescriptionImage(mModel, mProxyModel, mTableView->selectionModel(), this);
    connect(mTableView->selectionModel(), &QItemSelectionModel::selectionChanged, descriptionImg, &DkDescriptionImage::selectionChanged);
    connect(mProxyModel, &QSortFilterProxyModel::dataChanged, descriptionImg, &DkDescriptionImage::dataChanged);

    QWidget *descWidget = new QWidget(this);
    QHBoxLayout *dLayout = new QHBoxLayout(descWidget);
    dLayout->setContentsMargins(0, 0, 0, 0);
    dLayout->addWidget(descriptionEdit);
    dLayout->addWidget(descriptionImg);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(searchWidget);
    layout->addWidget(mTableView);
    layout->addWidget(descWidget);
}

// void DkPluginTableWidget::on_updateButton_clicked() {
//
//	// TODO!
//	//DkInstallUpdater::updateNomacs("--manage-packages");
// }

void DkPluginTableWidget::reloadPlugins()
{
    DkPluginManager::instance().reload();
    updateInstalledModel();
}

void DkPluginTableWidget::filterTextChanged()
{
    QRegularExpression regExp(mFilterEdit->text(), QRegularExpression::CaseInsensitiveOption);
    mProxyModel->setFilterRegularExpression(regExp);
    mTableView->resizeRowsToContents();
}

void DkPluginTableWidget::uninstallPlugin(const QModelIndex &index)
{
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

    bool removed = DkPluginManager::instance().deletePlugin(plugin);

    if (!removed)
        return;

    // updateInstalledModel();	// !!! update model before deleting the interface

    mTableView->model()->removeRow(index.row());
    // emit dataChanged(index, index);

    mTableView->resizeRowsToContents();
}

void DkPluginTableWidget::clearTableFilters()
{
    mFilterEdit->clear();
    mFilterEdit->setFocus();
}

// update models if new plugins are installed or copied into the folder
void DkPluginTableWidget::updateInstalledModel()
{
    clearTableFilters();
    // mTableView->relo;
}

//**********************************************************************************
// DkInstalledPluginsModel : Model managing installed plugins data in the table
//**********************************************************************************
DkInstalledPluginsModel::DkInstalledPluginsModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    mParentTable = static_cast<DkPluginTableWidget *>(parent);
}

int DkInstalledPluginsModel::rowCount(const QModelIndex &) const
{
    return DkPluginManager::instance().getPlugins().size();
}

int DkInstalledPluginsModel::columnCount(const QModelIndex &) const
{
    return DkSettingsManager::param().isPortable() ? ip_column_size : ip_column_size - 1;
}

QVariant DkInstalledPluginsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const QVector<QSharedPointer<DkPluginContainer>> &plugins = DkPluginManager::instance().getPlugins();

    if (index.row() >= plugins.size() || index.row() < 0) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        QSharedPointer<DkPluginContainer> plugin = plugins.at(index.row());
        if (index.column() == ip_column_name) {
            return plugin->pluginName();
        } else if (index.column() == ip_column_version) {
            return plugin->version();
        } else if (index.column() == ip_column_uninstall)
            return QString(tr("Uninstall"));
    }

    return QVariant();
}

QVariant DkInstalledPluginsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

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

Qt::ItemFlags DkInstalledPluginsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractTableModel::flags(index);
}

bool DkInstalledPluginsModel::removeRows(int position, int rows, const QModelIndex &index)
{
    beginRemoveRows(QModelIndex(), position, position + rows - 1);
    endRemoveRows();

    emit dataChanged(index, index);

    return true;
}

void DkInstalledPluginsModel::setDataToInsert(QSharedPointer<DkPluginContainer> newData)
{
    mPluginToInsert = newData;
}

//*********************************************************************************
// DkCheckBoxDelegate : delagete for checkbox only column in the model
//*********************************************************************************

static QRect CheckBoxRect(const QStyleOptionViewItem &viewItemStyleOptions)
{
    QStyleOptionButton checkBoxStyleOption;
    QRect checkBoxRect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkBoxStyleOption);
    QPoint checkBoxPoint(viewItemStyleOptions.rect.x() + viewItemStyleOptions.rect.width() / 2 - checkBoxRect.width() / 2,
                         viewItemStyleOptions.rect.y() + viewItemStyleOptions.rect.height() / 2 - checkBoxRect.height() / 2);
    return QRect(checkBoxPoint, checkBoxRect.size());
}

DkPluginCheckBoxDelegate::DkPluginCheckBoxDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    mParentTable = static_cast<QTableView *>(parent);
}

void DkPluginCheckBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected) {
        if (mParentTable->hasFocus())
            painter->fillRect(option.rect, option.palette.highlight());
        else
            painter->fillRect(option.rect, option.palette.window());
    }
    // else if (index.row() % 2 == 1) painter->fillRect(option.rect, option.palette.alternateBase());	// already done automatically

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

bool DkPluginCheckBoxDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if ((event->type() == QEvent::MouseButtonRelease) || (event->type() == QEvent::MouseButtonDblClick)) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() != Qt::LeftButton || !CheckBoxRect(option).contains(mouseEvent->pos())) {
            return false;
        }
        if (event->type() == QEvent::MouseButtonDblClick) {
            return true;
        }
    } else if (event->type() == QEvent::KeyPress) {
        if (static_cast<QKeyEvent *>(event)->key() != Qt::Key_Space && static_cast<QKeyEvent *>(event)->key() != Qt::Key_Select) {
            return false;
        }
    } else
        return false;

    bool checked = index.model()->data(index, Qt::DisplayRole).toBool();
    return model->setData(index, !checked, Qt::EditRole);
}

//*********************************************************************************
// DkPushButtonDelegate : delagete for uninstall column in the model
//*********************************************************************************

static QRect PushButtonRect(const QStyleOptionViewItem &viewItemStyleOptions)
{
    QRect pushButtonRect = viewItemStyleOptions.rect;
    // pushButtonRect.setHeight(pushButtonRect.height() - 2);
    // pushButtonRect.setWidth(pushButtonRect.width() - 2);
    QPoint pushButtonPoint(viewItemStyleOptions.rect.x() + viewItemStyleOptions.rect.width() / 2 - pushButtonRect.width() / 2,
                           viewItemStyleOptions.rect.y() + viewItemStyleOptions.rect.height() / 2 - pushButtonRect.height() / 2);
    return QRect(pushButtonPoint, pushButtonRect.size());
}

DkPushButtonDelegate::DkPushButtonDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    mParentTable = static_cast<QTableView *>(parent);
}

void DkPushButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected) {
        if (mParentTable->hasFocus())
            painter->fillRect(option.rect, option.palette.highlight());
        else
            painter->fillRect(option.rect, option.palette.window());
    }

    QStyleOptionButton pushButtonStyleOption;
    pushButtonStyleOption.text = index.model()->data(index, Qt::DisplayRole).toString();
    if (mCRow == index.row())
        pushButtonStyleOption.state = mPushButonState | QStyle::State_Enabled;
    else
        pushButtonStyleOption.state = QStyle::State_Enabled;
    pushButtonStyleOption.rect = PushButtonRect(option);

    QApplication::style()->drawControl(QStyle::CE_PushButton, &pushButtonStyleOption, painter);
}

bool DkPushButtonDelegate::editorEvent(QEvent *event, QAbstractItemModel *, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if ((event->type() == QEvent::MouseButtonRelease) || (event->type() == QEvent::MouseButtonPress)) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() != Qt::LeftButton || !PushButtonRect(option).contains(mouseEvent->pos())) {
            mPushButonState = QStyle::State_Raised;
            return false;
        }
    } else if (event->type() == QEvent::KeyPress) {
        if (static_cast<QKeyEvent *>(event)->key() != Qt::Key_Space && static_cast<QKeyEvent *>(event)->key() != Qt::Key_Select) {
            mPushButonState = QStyle::State_Raised;
            return false;
        }
    } else {
        mPushButonState = QStyle::State_Raised;
        return false;
    }

    if (event->type() == QEvent::MouseButtonPress) {
        mPushButonState = QStyle::State_Sunken;
        mCRow = index.row();
    } else if (event->type() == QEvent::MouseButtonRelease) {
        mPushButonState = QStyle::State_Raised;
        emit buttonClicked(index);
    }
    return true;
}

//**********************************************************************************
// DkDescriptionEdit : text edit connected to tableView selection and models
//**********************************************************************************
DkDescriptionEdit::DkDescriptionEdit(QAbstractTableModel *data, QSortFilterProxyModel *proxy, QItemSelectionModel *selection, QWidget *parent)
    : QTextEdit(parent)
{
    mParentTable = static_cast<DkPluginTableWidget *>(parent);
    mDataModel = data;
    mProxyModel = proxy;
    mSelectionModel = selection;
    setReadOnly(true);
}

void DkDescriptionEdit::updateText()
{
    if (mSelectionModel->selection().indexes().empty()) {
        setText("");
        return;
    }

    QString text;
    QModelIndex sourceIndex = mProxyModel->mapToSource(mSelectionModel->selection().indexes().first());

    const QVector<QSharedPointer<DkPluginContainer>> &plugins = DkPluginManager::instance().getPlugins();
    QSharedPointer<DkPluginContainer> plugin = plugins.at(sourceIndex.row());
    if (plugin)
        text = plugin->fullDescription();

    if (text.isNull())
        text = tr("No metadata available!");
    setText(text);
}

void DkDescriptionEdit::dataChanged(const QModelIndex &, const QModelIndex &)
{
    updateText();
}

void DkDescriptionEdit::selectionChanged(const QItemSelection &, const QItemSelection &)
{
    updateText();
}

//**********************************************************************************
// DkDescriptionImage : image label connected to tableView selection and models
//**********************************************************************************

DkDescriptionImage::DkDescriptionImage(QAbstractTableModel *data, QSortFilterProxyModel *proxy, QItemSelectionModel *selection, QWidget *parent)
    : QLabel(parent)
{
    mParentTable = static_cast<DkPluginTableWidget *>(parent);
    mDataModel = data;
    mProxyModel = proxy;
    mSelectionModel = selection;
    mDefaultImage = QPixmap(":/nomacs/img/plugin-banner.svg");
    setPixmap(mDefaultImage);
}

void DkDescriptionImage::updateImage()
{
    switch (mSelectionModel->selection().indexes().count()) {
    case 0:
        setPixmap(mDefaultImage);
        break;
    default:
        QModelIndex sourceIndex = mProxyModel->mapToSource(mSelectionModel->selection().indexes().first());
        QImage img;
        const QVector<QSharedPointer<DkPluginContainer>> &plugins = DkPluginManager::instance().getPlugins();
        QSharedPointer<DkPluginContainer> plugin = plugins.at(sourceIndex.row());

        if (plugin && plugin->plugin())
            img = plugin->plugin()->image();
        if (!img.isNull())
            setPixmap(QPixmap::fromImage(img));
        else
            setPixmap(mDefaultImage);
        break;
    }
}

void DkDescriptionImage::dataChanged(const QModelIndex &, const QModelIndex &)
{
    updateImage();
}

void DkDescriptionImage::selectionChanged(const QItemSelection &, const QItemSelection &)
{
    updateImage();
}

// DkPluginManager --------------------------------------------------------------------
DkPluginManager &DkPluginManager::instance()
{
    static DkPluginManager inst;
    return inst;
}

DkPluginManager::DkPluginManager()
{
    // loadPlugins();
}

DkPluginManager::~DkPluginManager()
{
}

// returns map with id and interface
QVector<QSharedPointer<DkPluginContainer>> DkPluginManager::getPlugins() const
{
    return mPlugins;
}

void DkPluginManager::reload()
{
    clear();
    loadPlugins();
}

void DkPluginManager::removePlugin(QSharedPointer<DkPluginContainer> plugin)
{
    if (plugin)
        mPlugins.remove(mPlugins.indexOf(plugin));
    else
        qWarning() << "Could not delete plugin - it is NULL";
}

bool DkPluginManager::deletePlugin(QSharedPointer<DkPluginContainer> plugin)
{
    if (plugin) {
        mPlugins.remove(mPlugins.indexOf(plugin));

        if (!plugin->uninstall()) {
            qDebug() << "Failed to delete plugin file!";
            QMessageBox::critical(DkUtils::getMainWindow(),
                                  QObject::tr("Plugin Manager"),
                                  QObject::tr("The dll could not be deleted!\nPlease restart nomacs and try again."));
            return false;
        } else
            return true;
    }

    return false;
}

void DkPluginManager::clear()
{
    mPlugins.clear();
}

// Loads enabled plugins (i.e. when the menu is first hit)
void DkPluginManager::loadPlugins()
{
    //// DEBUG --------------------------------------------------------------------
    // nmc::DkDependencyWalker dw("C:/VSProjects/READ/nomacs/build2015-x64/Debug/plugins/writerIdentificationPlugin.dll");
    // if (!dw.findDependencies())
    //	qWarning() << "could not find dependencies for" << dw.filePath();

    // qDebug() << "all dependencies:" << dw.dependencies();
    // qDebug() << "filtered dependencies:" << dw.filteredDependencies();

    // return 0;
    //// DEBUG --------------------------------------------------------------------

    // do not load twice
    if (!mPlugins.empty())
        return;

    DkTimer dt;

    QStringList loadedPluginFileNames = QStringList();
    QStringList libPaths = QCoreApplication::libraryPaths();
    libPaths.append(QCoreApplication::applicationDirPath() + "/plugins");

    for (const QString &cPath : libPaths) {
        // skip the nomacs dir
        if (cPath == QApplication::applicationDirPath())
            continue;

        // skip image plugins
        if (cPath.contains("imageformats"))
            continue;

        QDir pluginsDir(cPath);

        for (const QString &fileName : pluginsDir.entryList(QDir::Files)) {
            DkTimer dtt;
#ifdef Q_OS_LINUX
            // needed because of symbolic links of sonames
            QFileInfo file(pluginsDir.absoluteFilePath(fileName));
            if (file.isSymLink())
                continue;
#elif defined(Q_OS_WIN)
            if (!fileName.contains(".dll"))
                continue;

#endif
            QString shortFileName = fileName.split("/").last();
            if (!loadedPluginFileNames.contains(shortFileName)) { // prevent double loading of the same plugin
                if (singlePluginLoad(pluginsDir.absoluteFilePath(fileName)))
                    loadedPluginFileNames.append(shortFileName);
            }
            // else
            //	qDebug() << "rejected since it is twice: " << shortFileName;
        }
    }

    std::sort(mPlugins.begin(), mPlugins.end()); // , &DkPluginContainer::operator<);
    qInfo() << mPlugins.size() << "plugins loaded in" << dt;

    if (mPlugins.empty())
        qInfo() << "I was searching these paths" << libPaths;
}

/**
 * Loads one plugin from file fileName
 * @param fileName
 **/
bool DkPluginManager::singlePluginLoad(const QString &filePath)
{
    if (isBlackListed(filePath))
        return false;

    DkTimer dt;
    QSharedPointer<DkPluginContainer> plugin = QSharedPointer<DkPluginContainer>(new DkPluginContainer(filePath));
    if (plugin->load())
        mPlugins.append(plugin);

    return plugin->isLoaded();
}

QSharedPointer<DkPluginContainer> DkPluginManager::getPluginByName(const QString &pluginName) const
{
    for (auto p : mPlugins) {
        if (p && pluginName == p->pluginName())
            return p;
    }

    return QSharedPointer<DkPluginContainer>();
}

QSharedPointer<DkPluginContainer> DkPluginManager::getPluginByPath(const QString &path) const
{
    for (auto p : mPlugins) {
        if (p && path == p->pluginPath())
            return p;
    }

    return QSharedPointer<DkPluginContainer>();
}

QVector<QSharedPointer<DkPluginContainer>> DkPluginManager::getBasicPlugins() const
{
    QVector<QSharedPointer<DkPluginContainer>> plugins;

    for (auto plugin : mPlugins) {
        DkPluginInterface *p = plugin->plugin();

        if (p && p->interfaceType() == DkPluginInterface::interface_basic) {
            plugins.append(plugin);
        }
    }

    return plugins;
}

QVector<QSharedPointer<DkPluginContainer>> DkPluginManager::getBatchPlugins() const
{
    QVector<QSharedPointer<DkPluginContainer>> plugins;

    for (auto plugin : mPlugins) {
        DkPluginInterface *p = plugin->plugin();

        if (p && (p->interfaceType() == DkPluginInterface::interface_basic || p->interfaceType() == DkPluginInterface::interface_batch)) {
            plugins.append(plugin);
        }
    }

    return plugins;
}

QSharedPointer<DkPluginContainer> DkPluginManager::getRunningPlugin() const
{
    for (auto plugin : mPlugins) {
        if (plugin->isActive())
            return plugin;
    }

    return QSharedPointer<DkPluginContainer>();
}

void DkPluginManager::clearRunningPlugin()
{
    for (auto plugin : mPlugins)
        plugin->setActive(false);
}

void DkPluginManager::runPlugin(QSharedPointer<DkPluginContainer> plugin)
{
    if (getRunningPlugin()) {
        // the plugin is not closed in time
        QMessageBox infoDialog(DkUtils::getMainWindow());
        infoDialog.setWindowTitle(QObject::tr("Close plugin"));
        infoDialog.setIcon(QMessageBox::Information);
        infoDialog.setText(QObject::tr("Please close the currently opened plugin."));
        infoDialog.show();

        infoDialog.exec();

        // TODO: dialog with yes/no - unload plugin if user wants to
    }

    plugin->setActive();
}

bool DkPluginManager::isBlackListed(const QString &pluginPath) const
{
    QString fileName = QFileInfo(pluginPath).fileName();

    for (const QString &filter : blackList())
        if (pluginPath.contains(filter))
            return true;

    return false;
}

QStringList DkPluginManager::blackList()
{
    return QStringList() << "opencv";
}

void DkPluginManager::createPluginsPath()
{
#ifdef WITH_PLUGINS
    // initialize plugin paths -----------------------------------------
#if defined(Q_OS_WIN)
    QDir pluginsDir = QCoreApplication::applicationDirPath() + "/plugins";
#elif defined(Q_OS_MAC)
    // .app/Contents/PlugIns contains Qt plugins so make our own subdirectory here.
    // This dir is treated differently by macdeployqt, dylibs here are not
    // not copied into .app/Contents/Frameworks as is the usual case
    QDir pluginsDir = QCoreApplication::applicationDirPath() + "/../PlugIns/nomacs";
#else
    QDir pluginsDir = QLibraryInfo::location(QLibraryInfo::LibrariesPath) + "/nomacs-plugins/";
#endif // Q_OS_WIN

    if (!pluginsDir.exists())
        pluginsDir.mkpath(pluginsDir.absolutePath());

    nmc::DkSettingsManager::param().global().pluginsDir = pluginsDir.absolutePath();
    qInfo() << "plugins dir set to: " << nmc::DkSettingsManager::param().global().pluginsDir;

    QCoreApplication::addLibraryPath(nmc::DkSettingsManager::param().global().pluginsDir);
    QCoreApplication::addLibraryPath("./imageformats");

#endif // WITH_PLUGINS
}

// DkPluginActionManager --------------------------------------------------------------------
DkPluginActionManager::DkPluginActionManager(QObject *parent)
    : QObject(parent)
{
    assignCustomPluginShortcuts();
}

void DkPluginActionManager::assignCustomPluginShortcuts()
{
    DefaultSettings settings;
    settings.beginGroup("CustomPluginShortcuts");
    QStringList psKeys = settings.allKeys();
    settings.endGroup();

    if (psKeys.size() > 0) {
        settings.beginGroup("CustomShortcuts");

        mPluginDummyActions = QVector<QAction *>();

        for (int i = 0; i < psKeys.size(); i++) {
            QAction *action = new QAction(psKeys.at(i), this);
            QString val = settings.value(psKeys.at(i), "no-shortcut").toString();
            if (val != "no-shortcut")
                action->setShortcut(val);
            connect(action, &QAction::triggered, this, &DkPluginActionManager::runPluginFromShortcut);
            mPluginDummyActions.append(action);
        }

        settings.endGroup();
    }
}

void DkPluginActionManager::setMenu(QMenu *menu)
{
    mMenu = menu;
    connect(mMenu, &QMenu::aboutToShow, this, &DkPluginActionManager::updateMenu);

    // allow shortcuts to invoke plugins that haven't been loaded yet
    menu->addActions(pluginDummyActions().toList());
}

QMenu *DkPluginActionManager::menu() const
{
    return mMenu;
}

QVector<QAction *> DkPluginActionManager::pluginDummyActions() const
{
    return mPluginDummyActions;
}

QVector<QAction *> DkPluginActionManager::pluginActions() const
{
    return mPluginActions;
}

QVector<QMenu *> DkPluginActionManager::pluginSubMenus() const
{
    return mPluginSubMenus;
}

void DkPluginActionManager::updateMenu()
{
    qDebug() << "CREATING plugin menu";

    if (!mMenu) {
        qWarning() << "plugin menu is NULL where it should not be!";
    }

    DkPluginManager::instance().loadPlugins();
    QVector<QSharedPointer<DkPluginContainer>> plugins = DkPluginManager::instance().getPlugins();

    if (plugins.empty()) {
        // mPluginActions.resize(DkActionManager::menu_plugins_end);
        mPluginActions = DkActionManager::instance().pluginActions();
    }
    mMenu->clear();

    for (auto p : plugins) {
        connect(p.data(),
                QOverload<DkViewPortInterface *, bool>::of(&DkPluginContainer::runPlugin),
                this,
                QOverload<DkViewPortInterface *, bool>::of(&DkPluginActionManager::runPlugin),
                Qt::UniqueConnection);
        connect(p.data(),
                QOverload<DkPluginContainer *, const QString &>::of(&DkPluginContainer::runPlugin),
                this,
                QOverload<DkPluginContainer *, const QString &>::of(&DkPluginActionManager::runPlugin),
                Qt::UniqueConnection);
    }

    if (plugins.isEmpty()) { // no  plugins
        mMenu->addAction(mPluginActions[DkActionManager::menu_plugin_manager]);
        mPluginActions.resize(DkActionManager::menu_plugin_manager); // reduce the size again
    } else {
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
void DkPluginActionManager::addPluginsToMenu()
{
    QVector<QSharedPointer<DkPluginContainer>> loadedPlugins = DkPluginManager::instance().getPlugins();
    std::sort(loadedPlugins.begin(), loadedPlugins.end());

    mPluginSubMenus.clear();

    QStringList pluginMenu = QStringList();

    for (auto plugin : loadedPlugins) {
        DkPluginInterface *pi = plugin->plugin();

        if (pi && plugin->pluginMenu()) {
            QList<QAction *> actions = pi->createActions(DkUtils::getMainWindow());
            mPluginSubMenus.append(plugin->pluginMenu());
            mMenu->addMenu(plugin->pluginMenu());
        } else if (pi) {
            QAction *a = new QAction(plugin->pluginName(), this);
            a->setData(plugin->id());
            mPluginActions.append(a);
            mMenu->addAction(a);
            connect(a, &QAction::triggered, plugin.data(), &DkPluginContainer::run);
        }
    }

    mMenu->addSeparator();
    mMenu->addAction(DkActionManager::instance().action(DkActionManager::menu_plugin_manager));

    QVector<QAction *> allPluginActions = mPluginActions;

    for (const QMenu *m : mPluginSubMenus) {
        allPluginActions << m->actions().toVector();
    }

    DkActionManager::instance().assignCustomShortcuts(allPluginActions);
    savePluginActions(allPluginActions);
}

void DkPluginActionManager::runPluginFromShortcut()
{
    qDebug() << "running plugin shortcut...";

    QAction *action = qobject_cast<QAction *>(sender());
    QString actionName = action->text();

    updateMenu();

    QVector<QAction *> allPluginActions = mPluginActions;

    for (const QMenu *m : mPluginSubMenus) {
        allPluginActions << m->actions().toVector();
    }

    // this method fails if two plugins have the same action name!!
    for (int i = 0; i < allPluginActions.size(); i++)
        if (allPluginActions.at(i)->text().compare(actionName) == 0) {
            allPluginActions.at(i)->trigger();
            break;
        }
}

void DkPluginActionManager::savePluginActions(QVector<QAction *> actions) const
{
    DefaultSettings settings;
    settings.beginGroup("CustomPluginShortcuts");
    settings.remove("");
    for (int i = 0; i < actions.size(); i++)
        settings.setValue(actions.at(i)->text(), actions.at(i)->text());
    settings.endGroup();
}

}
