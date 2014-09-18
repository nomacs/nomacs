/*******************************************************************************************************
 DkMetaDataWidgets.cpp
 Created on:	17.08.2014
 
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

#include "DkMetaDataWidgets.h"

#include <QVBoxLayout>

namespace nmc {

// DkMetaDataModel --------------------------------------------------------------------
DkMetaDataModel::DkMetaDataModel(QObject* parent /* = 0 */) : QAbstractTableModel(parent) {

	// create root
	QVector<QVariant> rootData;
	rootData << tr("Key") << tr("Value");

	rootItem = new TreeItem(rootData);
}

DkMetaDataModel::~DkMetaDataModel() {

	delete rootItem;
}

void DkMetaDataModel::clear() {

	beginResetModel();
	rootItem->clear();
	endResetModel();
}

void DkMetaDataModel::addMetaData(QSharedPointer<DkMetaDataT> metaData) {

	//metaData->printMetaData();

	DkTimer dt;
	QStringList fileKeys, fileValues;
	metaData->getFileMetaData(fileKeys, fileValues);

	for (int idx = 0; idx < fileKeys.size(); idx++) {

		QString lastKey = fileKeys.at(idx).split(".").last();
		createItem(fileKeys.at(idx), lastKey, fileValues.at(idx));
	}

	QStringList exifKeys = metaData->getExifKeys();

	for (int idx = 0; idx < exifKeys.size(); idx++) {
		
		QString lastKey = exifKeys.at(idx).split(".").last();
		QString translatedKey = DkMetaDataHelper::getInstance().translateKey(lastKey);
		QString exifValue = metaData->getNativeExifValue(exifKeys.at(idx));
		exifValue = DkMetaDataHelper::getInstance().resolveSpecialValue(metaData, lastKey, exifValue);

		createItem(exifKeys.at(idx), translatedKey, exifValue);
	}

	QStringList iptcKeys = metaData->getIptcKeys();

	for (int idx = 0; idx < iptcKeys.size(); idx++) {

		QString lastKey = iptcKeys.at(idx).split(".").last();
		QString translatedKey = DkMetaDataHelper::getInstance().translateKey(lastKey);
		QString exifValue = metaData->getIptcValue(exifKeys.at(idx));
		exifValue = DkMetaDataHelper::getInstance().resolveSpecialValue(metaData, lastKey, exifValue);

		createItem(iptcKeys.at(idx), translatedKey, exifValue);
	}

	QStringList xmpKeys = metaData->getXmpKeys();

	for (int idx = 0; idx < xmpKeys.size(); idx++) {

		QString lastKey = xmpKeys.at(idx).split(".").last();
		QString translatedKey = DkMetaDataHelper::getInstance().translateKey(lastKey);
		QString exifValue = metaData->getXmpValue(xmpKeys.at(idx));
		exifValue = DkMetaDataHelper::getInstance().resolveSpecialValue(metaData, lastKey, exifValue);

		createItem(xmpKeys.at(idx), translatedKey, exifValue);
	}

	qDebug() << "model refreshed in: " << dt.getTotal();
}

void DkMetaDataModel::createItem(const QString& key, const QString& keyName, const QString& value) {

	// Split key first
	QStringList keyHierarchy = key.split('.');

	if (keyHierarchy.empty()) {
		qDebug() << "no key hierarchy... skipping: " << key;
		return;
	}

	TreeItem* item = rootItem;

	for (int idx = 0; idx < keyHierarchy.size()-1; idx++) {

		QString cKey = keyHierarchy.at(idx);
		TreeItem* cHierarchyItem = item->find(cKey, 0);

		if (!cHierarchyItem) {
			QVector<QVariant> keyData;
			keyData << cKey;
			cHierarchyItem = new TreeItem(keyData, item);
			item->appendChild(cHierarchyItem);
		}

		item = cHierarchyItem;	// switch to next hierarchy level
	}

	QString cleanValue = DkUtils::cleanFraction(value);
	
	QVector<QVariant> metaDataEntry;
	metaDataEntry << keyName;

	QDateTime pd = DkUtils::getConvertableDate(cleanValue);

	if (!pd.isNull())
		metaDataEntry << pd;
	else
		metaDataEntry << cleanValue;

	TreeItem* dataItem = new TreeItem(metaDataEntry, item);
	item->appendChild(dataItem);

}

QModelIndex DkMetaDataModel::index(int row, int column, const QModelIndex &parent) const {

	if (!hasIndex(row, column, parent))
		return QModelIndex();

	TreeItem *parentItem;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());

	TreeItem *childItem = parentItem->child(row);

	//qDebug() << " creating index for: " << childItem->data(0) << " row: " << row;
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex DkMetaDataModel::parent(const QModelIndex &index) const {

	if (!index.isValid())
		return QModelIndex();

	TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
	TreeItem *parentItem = childItem->parent();

	if (!parentItem || parentItem == rootItem)
		return QModelIndex();

	//qDebug() << "parent is: " << childItem->data(0);

	return createIndex(parentItem->row(), 0, parentItem);
}

int DkMetaDataModel::rowCount(const QModelIndex& parent) const {

	TreeItem *parentItem;
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());

	return parentItem->childCount();
}

int DkMetaDataModel::columnCount(const QModelIndex& parent) const {

	if (parent.isValid())
		return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
	else
		return rootItem->columnCount();
	//return 2;
}

QVariant DkMetaDataModel::data(const QModelIndex& index, int role) const {

	if (!index.isValid()) {
		qDebug() << "invalid row: " << index.row();
		return QVariant();
	}

	//if (index.row() > rowCount())
	//	return QVariant();

	//if (index.column() > columnCount())
	//	return QVariant();

	if (role == Qt::DisplayRole || role == Qt::EditRole) {

		TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
		//qDebug() << "returning: " << item->data(0) << "row: " << index.row();

		return item->data(index.column());
	}

	return QVariant();
}


QVariant DkMetaDataModel::headerData(int section, Qt::Orientation orientation, int role) const {

	if (orientation != Qt::Horizontal || role != Qt::DisplayRole) 
		return QVariant();

	return rootItem->data(section);
} 

//bool DkMetaDataModel::setData(const QModelIndex& index, const QVariant& value, int role) {
//
//	if (!index.isValid() || role != Qt::EditRole)
//		return false;
//
//	//if (index.column() == 1) {
//
//	//	TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
//	//	item->setData(ks, index.column());
//
//	//}
//	//else {
//		TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
//		item->setData(value, index.column());
//	//}
//
//	emit dataChanged(index, index);
//	return true;
//}
//
//Qt::ItemFlags DkMetaDataModel::flags(const QModelIndex& index) const {
//
//	if (!index.isValid())
//		return Qt::ItemIsEditable;
//
//	TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
//
//	Qt::ItemFlags flags;
//
//	if (index.column() == 0)
//		flags = QAbstractTableModel::flags(index);
//	if (index.column() == 1)	// TODO: check if the value is editable
//		flags = QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
//
//	return flags;
//}


// DkMetaDataDock --------------------------------------------------------------------
DkMetaDataDock::DkMetaDataDock(const QString& title, QWidget* parent /* = 0 */, Qt::WindowFlags flags /* = 0 */ ) : 
	QDockWidget(title, parent, flags) {

		setObjectName("DkMetaDataDock");

		createLayout();
		readSettings();
}

DkMetaDataDock::~DkMetaDataDock() {
	writeSettings();
}

void DkMetaDataDock::writeSettings() {

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup(objectName());
	
	for (int idx = 0; idx < model->columnCount(QModelIndex()); idx++) {

		QString headerVal = model->headerData(idx, Qt::Horizontal).toString();
		settings.setValue(headerVal + "Size", treeView->columnWidth(idx));
	}

	settings.setValue("expandedNames", expandedNames);
	qDebug() << "settings write expanded names: " << expandedNames;

	settings.endGroup();
}

void DkMetaDataDock::readSettings() {
	
	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup(objectName());

	for (int idx = 0; idx < model->columnCount(QModelIndex()); idx++) {

		QString headerVal = model->headerData(idx, Qt::Horizontal).toString();

		int colWidth = settings.value(headerVal + "Size", -1).toInt();
		if (colWidth != -1) 
			treeView->setColumnWidth(idx, colWidth);
	}
	expandedNames = settings.value("expandedNames", QStringList()).toStringList();
	qDebug() << "settings expanded names: " << expandedNames;

	settings.endGroup();
}

void DkMetaDataDock::createLayout() {

	QWidget* widget = new QWidget(this);
	QVBoxLayout* layout = new QVBoxLayout(widget);

	//// register our special shortcut editor
	//QItemEditorFactory *factory = new QItemEditorFactory;

	//QItemEditorCreatorBase *shortcutListCreator =
	//	new QStandardItemEditorCreator<DkShortcutEditor>();

	//factory->registerEditor(QVariant::KeySequence, shortcutListCreator);

	//QItemEditorFactory::setDefaultFactory(factory);

	// create our beautiful shortcut view
	model = new DkMetaDataModel(this);
	treeView = new QTreeView(this);
	treeView->setModel(model);
	treeView->setAlternatingRowColors(true);
	treeView->setIndentation(8);
	//treeView->setStyleSheet("QTreeView{border-color: #C3C3C4; alternate-background-color: blue; background: #AAAAAA;}");

	thumbNailLabel = new QLabel(tr("Thumbnail"), this);
	thumbNailLabel->hide();

	// thumb layout
	QWidget* thumbWidget = new QWidget(this);
	QHBoxLayout* thumbLayout = new QHBoxLayout(thumbWidget);
	thumbLayout->addStretch();
	thumbLayout->addWidget(thumbNailLabel);
	thumbLayout->addStretch();

	layout->addWidget(treeView);
	layout->addWidget(thumbWidget);
	setWidget(widget);
}

void DkMetaDataDock::updateEntries() {

	int numRows = model->rowCount(QModelIndex());

	for (int idx = 0; idx < numRows; idx++)
		getExpandedItemNames(model->index(idx,0,QModelIndex()), expandedNames);

	model->clear();

	if (!imgC)
		return;

	model->addMetaData(imgC->getMetaData());
	
	treeView->setUpdatesEnabled(false);
	numRows = model->rowCount(QModelIndex());
	for (int idx = 0; idx < numRows; idx++)
		expandRows(model->index(idx, 0, QModelIndex()), expandedNames);
	treeView->setUpdatesEnabled(true);

	// for values we should adjust the size at least to the currently visible rows...
	treeView->resizeColumnToContents(1);
	//if (treeView->columnWidth(1) > 1000)
	//	treeView->setColumnWidth(1, 1000);

}

void DkMetaDataDock::setImage(QSharedPointer<DkImageContainerT> imgC) {

	this->imgC = imgC;

	if (isVisible())
		updateEntries();

	if (imgC) {

		// we need to load the thumbnail fresh to guarantee, that we just consider the exif thumb
		// the imgC thumbnail might be created from the image
		thumb = QSharedPointer<DkThumbNailT>(new DkThumbNailT(imgC->file()));
		connect(thumb.data(), SIGNAL(thumbLoadedSignal(bool)), this, SLOT(thumbLoaded(bool)));
		thumb->fetchThumb(DkThumbNailT::force_exif_thumb);
	}
}

void DkMetaDataDock::thumbLoaded(bool loaded) {

	if (loaded) {
		QImage thumbImg = thumb->getImage();
		
		if (thumbImg.width() > width()) {
			thumbNailLabel->setFixedWidth(width()-20);
			thumbImg = thumbImg.scaled(QSize(width(), thumbImg.height()), Qt::KeepAspectRatio);
		}
		else
			thumbNailLabel->setFixedHeight(thumbImg.height());

		thumbNailLabel->setPixmap(QPixmap::fromImage(thumbImg));
		thumbNailLabel->show();
	}
	else
		thumbNailLabel->hide();

	qDebug() << "has thumbnail: " << loaded;

}

void DkMetaDataDock::getExpandedItemNames(const QModelIndex& index, QStringList& expandedNames) {

	if (!treeView || !index.isValid())
		return;

	QString entryName = model->data(index,Qt::DisplayRole).toString();

	if (treeView->isExpanded(index) && !expandedNames.contains(entryName))
		expandedNames.append(entryName);
	else if (!treeView->isExpanded(index))
		expandedNames.removeAll(model->data(index,Qt::DisplayRole).toString());

	int rows = model->rowCount(index);

	qDebug() << "entry: " << entryName;

	for (int idx = 0; idx < rows; idx++)
		getExpandedItemNames(model->index(idx, 0, index), expandedNames);

}

void DkMetaDataDock::expandRows(const QModelIndex& index, const QStringList& expandedNames) {

	if (!index.isValid())
		return;

	if (expandedNames.contains(model->data(index).toString())) {
		qDebug() << "expanding: " << model->data(index).toString();
		treeView->setExpanded(index, true);
	}

	for (int idx = 0; idx < model->rowCount(index); idx++) {

		QModelIndex cIndex = index.child(idx, 0);

		if (expandedNames.contains(model->data(cIndex).toString())) {
			treeView->setExpanded(cIndex, true);
			expandRows(cIndex, expandedNames);
		}
	}
}

//void DkMetaDataDock::setVisible(bool visible) {
//
//	if (visible)
//		updateEntries();
//
//	QDockWidget::setVisible(visible);
//}



}