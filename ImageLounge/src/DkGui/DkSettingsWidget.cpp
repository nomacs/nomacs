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
 [1] http://www.nomacs.org/
 [2] https://github.com/nomacs/
 [3] http://download.nomacs.org
 *******************************************************************************************************/

#include "DkSettingsWidget.h"

#pragma warning(push, 0)	// no warnings from includes
#include <QTreeView>
#include <QVBoxLayout>
#pragma warning(pop)

namespace nmc {


// DkSettingsWidget --------------------------------------------------------------------
DkSettingsWidget::DkSettingsWidget(QWidget* parent) : QWidget(parent) {

	createLayout();
}

void DkSettingsWidget::setSettings(QSettings & settings, const QString& parentName) {

	for (const QString& gName : settings.childGroups()) {
		
		DkSettingsGroup sg = DkSettingsGroup::fromSettings(gName, settings);
		mModel->addSettingsGroup(sg, parentName);

		qDebug() << parentName << ">" << gName;

		settings.beginGroup(gName);
		setSettings(settings, gName);
		settings.endGroup();
	}
}

void DkSettingsWidget::createLayout() {


//	// register our special shortcut editor
//	QItemEditorFactory *factory = new QItemEditorFactory;
//
//#if QT_VERSION < 0x050000
//	QItemEditorCreatorBase *shortcutListCreator =
//		new QStandardItemEditorCreator<DkShortcutEditor>();
//#else
//	QItemEditorCreatorBase *shortcutListCreator =
//		new QStandardItemEditorCreator<QKeySequenceEdit>();
//#endif
//
//	factory->registerEditor(QVariant::KeySequence, shortcutListCreator);
//
//	QItemEditorFactory::setDefaultFactory(factory);

	// create our beautiful shortcut view
	mModel = new DkSettingsModel(this);

	//DkShortcutDelegate* scDelegate = new DkShortcutDelegate(this);

	QTreeView* treeView = new QTreeView(this);
	treeView->setModel(mModel);
	//treeView->setItemDelegate(scDelegate);
	treeView->setAlternatingRowColors(true);
	treeView->setIndentation(8);
	//treeView->header()->resizeSection(0, 200);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(treeView);

}

// DkSettingsEntry --------------------------------------------------------------------
DkSettingsEntry::DkSettingsEntry(const QString & key, const QString & value) {
	mKey = key;
	mValue = value;
}

QString DkSettingsEntry::key() const {
	return mKey;
}

void DkSettingsEntry::setValue(const QString & value) {
	mValue = value;
}

QString DkSettingsEntry::value() const {
	return mValue;
}

// DkSettingsGroup --------------------------------------------------------------------
DkSettingsGroup::DkSettingsGroup(const QString & name) {
	mGroupName = name;
}

bool DkSettingsGroup::isEmpty() const {
	return mEntries.empty();
}

DkSettingsGroup DkSettingsGroup::fromSettings(const QString & groupName, QSettings & settings) {

	DkSettingsGroup sg(groupName);
	settings.beginGroup(groupName);

	for (const QString& key : settings.allKeys()) {
		sg.mEntries << DkSettingsEntry(key, settings.value(key).toString());
	}

	settings.endGroup();

	return sg;
}

QString DkSettingsGroup::name() const {
	return mGroupName;
}

int DkSettingsGroup::size() const {
	return mEntries.size();
}

QVector<DkSettingsEntry> DkSettingsGroup::entries() const {
	return mEntries;
}

// DkSettingsModel --------------------------------------------------------------------
DkSettingsModel::DkSettingsModel(QObject* parent) : QAbstractItemModel(parent) {

	// create root
	QVector<QVariant> rootData;
	rootData << tr("Key") << tr("Value");

	mRootItem = new TreeItem(rootData);

}

DkSettingsModel::~DkSettingsModel() {
	delete mRootItem;
}

QModelIndex DkSettingsModel::index(int row, int column, const QModelIndex &parent) const {

	if (!hasIndex(row, column, parent))
		return QModelIndex();

	TreeItem *parentItem;

	if (!parent.isValid())
		parentItem = mRootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());

	TreeItem *childItem = parentItem->child(row);

	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex DkSettingsModel::parent(const QModelIndex &index) const {

	if (!index.isValid())
		return QModelIndex();

	TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
	TreeItem *parentItem = childItem->parent();

	if (parentItem == mRootItem)
		return QModelIndex();

	//qDebug() << "creating index for: " << childItem->data(0);

	return createIndex(parentItem->row(), 0, parentItem);
}

int DkSettingsModel::rowCount(const QModelIndex& parent) const {

	TreeItem *parentItem;
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		parentItem = mRootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());

	return parentItem->childCount();
}

int DkSettingsModel::columnCount(const QModelIndex& parent) const {

	if (parent.isValid())
		return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
	else
		return mRootItem->columnCount();
	//return 2;
}

QVariant DkSettingsModel::data(const QModelIndex& index, int role) const {

	if (!index.isValid()) {
		qDebug() << "invalid row: " << index.row();
		return QVariant();
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole) {

		TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
		//qDebug() << "returning: " << item->data(0) << "row: " << index.row();

		return item->data(index.column());
	}

	return QVariant();
}


QVariant DkSettingsModel::headerData(int section, Qt::Orientation orientation, int role) const {

	if (orientation != Qt::Horizontal || role != Qt::DisplayRole) 
		return QVariant();

	return mRootItem->data(section);
} 

bool DkSettingsModel::setData(const QModelIndex& index, const QVariant& value, int role) {

	if (!index.isValid() || role != Qt::EditRole)
		return false;

	if (index.column() == 1) {

		QString val = value.value<QString>();
		// TODO: check value & write it directely?

		TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
		item->setData(val, index.column());
	}
	else {
		TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
		item->setData(value, index.column());
	}

	//emit duplicateSignal("");		// TODO: we also have to clear if the user hits ESC
	emit dataChanged(index, index);
	return true;
}

Qt::ItemFlags DkSettingsModel::flags(const QModelIndex& index) const {

	if (!index.isValid())
		return Qt::ItemIsEditable;

	//// no editing on root items
	//if (item->parent() == rootItem)
	//	return QAbstractTableModel::flags(index);

	Qt::ItemFlags flags;

	if (index.column() == 0)
		flags = QAbstractItemModel::flags(index);
	if (index.column() == 1)
		flags = QAbstractItemModel::flags(index) | Qt::ItemIsEditable;

	return flags;
}

void DkSettingsModel::addSettingsGroup(const DkSettingsGroup& group, const QString& parentName) {

	// create root
	QVector<QVariant> data;
	data << group.name();

	TreeItem* parentItem = mRootItem->find(parentName, 0);
	if (!parentItem)
		parentItem = mRootItem;

	TreeItem* settingsItem = new TreeItem(data, parentItem);

	for (const DkSettingsEntry& entry : group.entries()) {

		QVector<QVariant> settingsData;
		settingsData << entry.key() << entry.value();

		TreeItem* dataItem = new TreeItem(settingsData, settingsItem);
		settingsItem->appendChild(dataItem);
	}

	parentItem->appendChild(settingsItem);
	//qDebug() << "menu item has: " << menuItem->childCount();

}

}