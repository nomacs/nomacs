/*******************************************************************************************************
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2016 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2016 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2016 Florian Kleber <florian@nomacs.org>

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

 related links:
 [1] https://nomacs.org/
 [2] https://github.com/nomacs/
 [3] http://download.nomacs.org
 *******************************************************************************************************/

#include "DkSettingsWidget.h"

#include "DkActionManager.h"

#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QTreeView>
#include <QVBoxLayout>

namespace nmc
{

// DkSettingsWidget --------------------------------------------------------------------
DkSettingsWidget::DkSettingsWidget(QWidget *parent)
    : DkWidget(parent)
{
    createLayout();
}

void DkSettingsWidget::setSettingsPath(const QString &settingsPath, const QString &parentName)
{
    QSettings settings(settingsPath, QSettings::IniFormat);

    DkSettingsGroup sg = DkSettingsGroup::fromSettings(settings, parentName);
    addSettingsGroup(sg);
}

void DkSettingsWidget::addSettingsGroup(const DkSettingsGroup &group)
{
    if (group.name().isEmpty()) {
        for (auto g : group.children())
            mSettingsModel->addSettingsGroup(g);
    } else
        mSettingsModel->addSettingsGroup(group);

    // udpate proxy
    mProxyModel->setSourceModel(mSettingsModel);
}

void DkSettingsWidget::clear()
{
    mProxyModel->invalidate();
    mSettingsModel->clear();
}

void DkSettingsWidget::changeSetting(QSettings &settings,
                                     const QString &key,
                                     const QVariant &value,
                                     const QStringList &groups)
{
    QStringList groupsClean = groups;
    groupsClean.pop_front(); // remove default group: settings

    for (const QString &gName : groupsClean) {
        settings.beginGroup(gName);
    }

    settings.setValue(key, value);
    qDebug() << key << ":" << value << "written...";

    for (int idx = 0; idx < groupsClean.size(); idx++)
        settings.endGroup();
}

void DkSettingsWidget::removeSetting(QSettings &settings, const QString &key, const QStringList &groups)
{
    QStringList groupsClean = groups;
    groupsClean.pop_front();

    for (const QString &gName : groupsClean) {
        settings.beginGroup(gName);
    }

    settings.remove(key);
    qDebug() << key << "removed...";

    for (int idx = 0; idx < groupsClean.size(); idx++)
        settings.endGroup();
}

void DkSettingsWidget::onSettingsModelSettingChanged(const QString &key,
                                                     const QVariant &value,
                                                     const QStringList &groups)
{
    emit changeSettingSignal(key, value, groups);
}

void DkSettingsWidget::onSettingsModelSettingRemoved(const QString &key, const QStringList &groups)
{
    emit removeSettingSignal(key, groups);
}

void DkSettingsWidget::onRemoveRowsTriggered()
{
    QModelIndexList selList = mTreeView->selectionModel()->selectedRows();
    for (const QModelIndex index : selList) {
        const QModelIndex cIndex = mProxyModel->mapToSource(index.parent());
        mSettingsModel->removeRows(index.row(), 1, cIndex);
    }
}

void DkSettingsWidget::createLayout()
{
    mSettingsFilter = new QLineEdit(this);
    mSettingsFilter->setPlaceholderText(tr("Filter Settings"));
    connect(mSettingsFilter, &QLineEdit::textChanged, this, &DkSettingsWidget::onFilterTextChanged);

    // create our beautiful shortcut view
    mSettingsModel = new DkSettingsModel(this);
    connect(mSettingsModel, &DkSettingsModel::settingChanged, this, &DkSettingsWidget::onSettingsModelSettingChanged);
    connect(mSettingsModel, &DkSettingsModel::settingRemoved, this, &DkSettingsWidget::onSettingsModelSettingRemoved);

    mProxyModel = new DkSettingsProxyModel(this);
    mProxyModel->setSourceModel(mSettingsModel);
    // mProxyModel->setDynamicSortFilter(true);

    mTreeView = new QTreeView(this);
    mTreeView->setModel(mProxyModel);
    mTreeView->setAlternatingRowColors(true);
    // mTreeView->setIndentation(8);
    mTreeView->header()->resizeSection(0, 200);
    // mTreeView->setSortingEnabled(true);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mSettingsFilter);
    layout->addWidget(mTreeView);

    // contextMenu
    auto *contextMenu = new QMenu(mTreeView);
    mTreeView->setContextMenuPolicy(Qt::ActionsContextMenu);

    auto *removeAction = new QAction(tr("Delete"), contextMenu);
    removeAction->setShortcut(QKeySequence::Delete);
    mTreeView->addAction(removeAction);
    connect(removeAction, &QAction::triggered, this, &DkSettingsWidget::onRemoveRowsTriggered);
}

void DkSettingsWidget::filter(const QString &filterText)
{
    if (!filterText.isEmpty())
        mTreeView->expandAll();

    mProxyModel->setFilterRegularExpression(
        QRegularExpression(QRegularExpression::escape(filterText), QRegularExpression::CaseInsensitiveOption));
    qDebug() << "filtering: " << filterText;
}

void DkSettingsWidget::expandAll()
{
    mTreeView->expandAll();
}

void DkSettingsWidget::onFilterTextChanged(const QString &filterText)
{
    filter(filterText);
}

// DkSettingsEntry --------------------------------------------------------------------
DkSettingsEntry::DkSettingsEntry(const QString &key, const QVariant &value)
{
    mKey = key;
    mValue = value;
}

QString DkSettingsEntry::key() const
{
    return mKey;
}

void DkSettingsEntry::setValue(const QVariant &value)
{
    mValue = value;
}

QVariant DkSettingsEntry::value() const
{
    return mValue;
}

DkSettingsEntry DkSettingsEntry::fromSettings(const QString &key, const QSettings &settings)
{
    DkSettingsEntry se(key);

    // int settings
    bool ok = false;

    // double settings?
    double dVal = settings.value(key, -1.0).toDouble(&ok);

    // we first cast to double & check if the number is rational
    // if it is not, we pass it to the int cast
    if (ok && (double)qRound(dVal) != dVal) {
        se.setValue(dVal);
        return se;
    }

    int iVal = settings.value(key, -1).toString().toInt(&ok); // double is ok e.g. 1.3 -> iVal=1

    if (ok) {
        se.setValue(iVal);
        return se;
    }

    se.setValue(settings.value(key));
    return se;
}

// DkSettingsGroup --------------------------------------------------------------------
DkSettingsGroup::DkSettingsGroup(const QString &name)
{
    mGroupName = name;
}

bool DkSettingsGroup::isEmpty() const
{
    return mEntries.empty() && mChildren.empty();
}

DkSettingsGroup DkSettingsGroup::fromSettings(QSettings &settings, const QString &groupName)
{
    DkSettingsGroup sg(groupName);
    settings.beginGroup(groupName);

    for (const QString &key : settings.allKeys()) {
        if (!key.contains("/")) // skip entries from different hierarchies
            sg.mEntries << DkSettingsEntry::fromSettings(key, settings);
    }

    for (const QString &gn : settings.childGroups()) {
        sg.addChild(DkSettingsGroup::fromSettings(settings, gn));
    }

    settings.endGroup();

    return sg;
}

QString DkSettingsGroup::name() const
{
    return mGroupName;
}

int DkSettingsGroup::size() const
{
    return mEntries.size();
}

QVector<DkSettingsEntry> DkSettingsGroup::entries() const
{
    return mEntries;
}

QVector<DkSettingsGroup> DkSettingsGroup::children() const
{
    return mChildren;
}

void DkSettingsGroup::addChild(const DkSettingsGroup &group)
{
    mChildren << group;
}

// DkSettingsProxyModel --------------------------------------------------------------------
DkSettingsProxyModel::DkSettingsProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool DkSettingsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    const auto *t = static_cast<TreeItem *>(index.internalPointer());
    if (t) {
        return t->contains(filterRegularExpression(), filterKeyColumn());
    }

    return true;
}

// DkSettingsModel --------------------------------------------------------------------
DkSettingsModel::DkSettingsModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    // create root
    QVector<QVariant> rootData;
    rootData << tr("Settings") << tr("Value");

    mRootItem = new TreeItem(rootData);
}

DkSettingsModel::~DkSettingsModel()
{
    delete mRootItem; // crash detected
}

QModelIndex DkSettingsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = mRootItem;
    else
        parentItem = static_cast<TreeItem *>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);

    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex DkSettingsModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    const auto *childItem = static_cast<TreeItem *>(index.internalPointer());
    const TreeItem *parentItem = childItem->parent();

    if (parentItem == mRootItem)
        return QModelIndex();

    // qDebug() << "creating index for: " << childItem->data(0);

    return createIndex(parentItem->row(), 0, parentItem);
}

int DkSettingsModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = mRootItem;
    else
        parentItem = static_cast<TreeItem *>(parent.internalPointer());

    return parentItem->childCount();
}

int DkSettingsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<TreeItem *>(parent.internalPointer())->columnCount();
    else
        return mRootItem->columnCount();
}

QVariant DkSettingsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        qDebug() << "invalid row: " << index.row();
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        auto *item = static_cast<TreeItem *>(index.internalPointer());
        return item->data(index.column());
    }

    return QVariant();
}

QVariant DkSettingsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    return mRootItem->data(section);
}

bool DkSettingsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (index.column() == 1) {
        auto val = value.value<QString>();
        // TODO: check value & write it directly?

        auto *item = static_cast<TreeItem *>(index.internalPointer());
        item->setData(val, index.column());
    } else {
        auto *item = static_cast<TreeItem *>(index.internalPointer());
        item->setData(value, index.column());
    }

    auto *item = static_cast<TreeItem *>(index.internalPointer());

    if (item) {
        item->setData(value, index.column());

        if (index.column() == 1) {
            emit settingChanged(item->data(0).toString(), item->data(1), item->parentList());
        }
    }

    emit dataChanged(index, index);
    return true;
}

Qt::ItemFlags DkSettingsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEditable;

    //// no editing on root items
    // if (item->parent() == rootItem)
    //	return QAbstractTableModel::flags(index);

    Qt::ItemFlags flags;

    if (index.column() == 0)
        flags = QAbstractItemModel::flags(index);
    if (index.column() == 1)
        flags = QAbstractItemModel::flags(index) | Qt::ItemIsEditable;

    return flags;
}

void DkSettingsModel::addSettingsGroup(const DkSettingsGroup &group, const QString &parentName)
{
    beginResetModel();
    // create root
    QVector<QVariant> data;
    data << group.name();

    TreeItem *parentItem = mRootItem->find(parentName, 0);
    if (!parentItem)
        parentItem = mRootItem;

    auto *settingsItem = new TreeItem(data, parentItem);

    for (const DkSettingsEntry &entry : group.entries()) {
        QVector<QVariant> settingsData;
        settingsData << entry.key() << entry.value();

        auto *dataItem = new TreeItem(settingsData, settingsItem);
        settingsItem->appendChild(dataItem);
    }

    parentItem->appendChild(settingsItem);

    for (const DkSettingsGroup &g : group.children()) {
        addSettingsGroup(g, group.name());
    }
    endResetModel();

    // qDebug() << "item - child count:" << settingsItem->childCount();
}

void DkSettingsModel::clear()
{
    beginResetModel();
    mRootItem->clear();
    endResetModel();
}

bool DkSettingsModel::removeRows(int row, int count, const QModelIndex &parent)
{
    bool success = false;

    auto *item = static_cast<TreeItem *>(parent.internalPointer());
    if (!item)
        item = mRootItem;

    beginRemoveRows(parent, row, row);
    for (int rIdx = row; rIdx < row + count; rIdx++) {
        TreeItem *deleteRow = item->child(rIdx);

        if (deleteRow) {
            emit settingRemoved(deleteRow->data(0).toString(), deleteRow->parentList());
            item->remove(rIdx);
            success = true;
        } else
            qWarning() << "I cannot delete a non-existing row:" << row;
    }
    endRemoveRows();

    return success;
}

}
