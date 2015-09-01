/*******************************************************************************************************
 DkQuickAccess.cpp
 Created on:	16.04.2015
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2015 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2015 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2015 Florian Kleber <florian@nomacs.org>

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

#include "DkQuickAccess.h"
#include "DkUtils.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#include <QStandardItemModel>
#include <QDebug>
#pragma warning(pop)		// no warnings from includes - end


namespace nmc {

// DkQuickFilterCompleter --------------------------------------------------------------------
DkQuickAccess::DkQuickAccess(QObject* parent /* = 0 */) : QObject(parent) {

	mModel = new QStandardItemModel(this);
	mModel->setColumnCount(1);

}

void DkQuickAccess::addActions(const QVector<QAction*>& actions) {

	// initialize the model
	int nRows = mModel->rowCount();
	mModel->setRowCount(nRows + actions.size());

	for (int rIdx = 0; rIdx < actions.size(); rIdx++) {
		
		if (!actions[rIdx]) {
			qDebug() << "WARNING: empty action detected...";
			continue;
		}

		QAction* a = actions[rIdx];
		QIcon icon = a->icon().isNull() ? QIcon(":/nomacs/img/movie-next.png") : a->icon();

		QString text = a->text().replace("&", "");
		QStandardItem* item = new QStandardItem(text);
		item->setIcon(icon);
		item->setToolTip(a->toolTip());
		mModel->setItem(nRows+rIdx, 0, item);
	}

	// we assume they are unique
	this->mActions << actions;
}

void DkQuickAccess::addFiles(const QStringList& filePaths) {

	addItems(filePaths, QIcon(":/nomacs/img/nomacs32.png"));
}

void DkQuickAccess::addDirs(const QStringList& dirPaths) {
	
	addItems(dirPaths, QIcon(":/nomacs/img/dir.png"));
}

void DkQuickAccess::addItems(const QStringList& itemTexts, const QIcon& icon) {

	int nRows = mModel->rowCount();
	mModel->setRowCount(nRows + itemTexts.size());

	for (int rIdx = 0; rIdx < itemTexts.size(); rIdx++) {

		QString text = itemTexts.at(rIdx);

		if (mFilePaths.contains(text))
			continue;

		QStandardItem* item = new QStandardItem(text);
		item->setIcon(icon);
		//item->setToolTip(a->toolTip());
		mModel->setItem(nRows + rIdx, 0, item);
		mFilePaths.append(text);
	}
}

void DkQuickAccess::fireAction(const QModelIndex& index) const {

	QString key = index.data().toString();

	if (mFilePaths.contains(key)) {
		emit loadFileSignal(key);
		return;
	}

	for (QAction* a : mActions) {

		QString aKey = a->text().replace("&", "");

		if (aKey == key) {
			
			if (a->isEnabled())
				a->trigger();
			// TODO: else feedback?
			return;
		}
	}
}

//void DkQuickFilterCompleter::setCompletionPrefix(const QString &prefix) {
//
//	//QStringListModel* sm = static_cast<QStringListModel*>(model());
//
//	//if (!sm)
//	//	QCompleter::setCompletionPrefix(prefix);
//
//
//	//QStringList strings = sm->stringList();
//	//strings = DkUtils::filterStringList(prefix, strings);
//	//sm->setStringList(strings);
//
//}


//// DkMetaDataModel --------------------------------------------------------------------
//DkQuickAcessModel::DkQuickAcessModel(QObject* parent /* = 0 */) : QAbstractItemModel(parent) {
//
//	// create root
//	QVector<QVariant> rootData;
//	rootData << tr("Key") << tr("Value");
//
//	rootItem = new TreeItem(rootData);
//}
//
//DkQuickAcessModel::~DkQuickAcessModel() {
//
//	delete rootItem;
//}
//
//void DkQuickAcessModel::clear() {
//
//	beginResetModel();
//	rootItem->clear();
//	endResetModel();
//}
//
//void DkQuickAcessModel::addActions(const QVector<QAction*>& actions) {
//
//	for (QAction* a : actions)
//		createItem(tr("Action"), a->text());
//
//}
//
//void DkQuickAcessModel::addPaths(const QStringList& filePaths) {
//
//	for (QString s : filePaths)
//		createItem(tr("File Path"), s);
//}
//
//void DkQuickAcessModel::createItem(const QString& key, const QVariant& value) {
//
//	TreeItem* item = rootItem;
//
//	QVector<QVariant> entry;
//	entry << value;
//	entry << key;
//
//	TreeItem* dataItem = new TreeItem(entry, item);
//	item->appendChild(dataItem);
//}
//
//QModelIndex DkQuickAcessModel::index(int row, int column, const QModelIndex &parent) const {
//
//	if (!hasIndex(row, column, parent))
//		return QModelIndex();
//
//	TreeItem *parentItem;
//
//	if (!parent.isValid())
//		parentItem = rootItem;
//	else
//		parentItem = static_cast<TreeItem*>(parent.internalPointer());
//
//	TreeItem *childItem = parentItem->child(row);
//
//	if (childItem)
//		return createIndex(row, column, childItem);
//	else
//		return QModelIndex();
//}
//
//QModelIndex DkQuickAcessModel::parent(const QModelIndex &index) const {
//
//	if (!index.isValid())
//		return QModelIndex();
//
//	TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
//	TreeItem *parentItem = childItem->parent();
//
//	if (!parentItem || parentItem == rootItem)
//		return QModelIndex();
//
//	//qDebug() << "parent is: " << childItem->data(0);
//
//	return createIndex(parentItem->row(), 0, parentItem);
//}
//
//int DkQuickAcessModel::rowCount(const QModelIndex& parent) const {
//
//	TreeItem *parentItem;
//	if (parent.column() > 0)
//		return 0;
//
//	if (!parent.isValid())
//		parentItem = rootItem;
//	else
//		parentItem = static_cast<TreeItem*>(parent.internalPointer());
//
//	return parentItem->childCount();
//}
//
//int DkQuickAcessModel::columnCount(const QModelIndex& parent) const {
//
//	if (parent.isValid())
//		return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
//	else
//		return rootItem->columnCount();
//}
//
//QVariant DkQuickAcessModel::data(const QModelIndex& index, int role) const {
//
//	if (!index.isValid()) {
//		return QVariant();
//	}
//
//	if (role == Qt::DisplayRole || role == Qt::EditRole) {
//
//		TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
//
//		return item->data(index.column());
//	}
//
//	return QVariant();
//}
//
//QVariant DkQuickAcessModel::headerData(int section, Qt::Orientation orientation, int role) const {
//
//	if (orientation != Qt::Horizontal || role != Qt::DisplayRole) 
//		return QVariant();
//
//	return rootItem->data(section);
//} 
//
//Qt::ItemFlags DkQuickAcessModel::flags(const QModelIndex& index) const {
//
//	if (!index.isValid())
//		return Qt::ItemIsEditable;
//
//	Qt::ItemFlags flags;
//
//	if (index.column() == 0)
//		flags = QAbstractItemModel::flags(index);
//	if (index.column() == 1)
//		flags = QAbstractItemModel::flags(index);
//
//	return flags;
//}
//
//
//
//
}