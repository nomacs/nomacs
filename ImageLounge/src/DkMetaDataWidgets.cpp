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
#include "DkBasicLoader.h"
#include "DkImageContainer.h"
#include "DkMetaData.h"
#include "DkUtils.h"
#include "DkTimer.h"
#include "DkImageStorage.h"
#include "DkSettings.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QDockWidget>
#include <QTreeView>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QPushButton>
#include <QPainter>
#include <QSettings>
#include <QScrollArea>
#include <QAction>
#include <QMenu>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QInputDialog>
#pragma warning(pop)		// no warnings from includes - end

namespace nmc {

// DkMetaDataModel --------------------------------------------------------------------
DkMetaDataModel::DkMetaDataModel(QObject* parent /* = 0 */) : QAbstractItemModel(parent) {

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
	DkDockWidget(title, parent, flags) {

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

// DkMetaDataSelection --------------------------------------------------------------------
DkMetaDataSelection::DkMetaDataSelection(const QSharedPointer<DkMetaDataT> metaData, QWidget* parent) : QWidget(parent) {

	setObjectName("DkMetaDataSelection");
	this->metaData = metaData;
	createLayout();
	selectionChanged();
}

void DkMetaDataSelection::createLayout() {

	createEntries(metaData, keys, values);

	QWidget* lWidget = new QWidget(this);
	layout = new QGridLayout(lWidget);

	for (int idx = 0; idx < keys.size(); idx++) {
		appendGUIEntry(keys.at(idx), values.at(idx), idx);
	}

	layout->setColumnStretch(2, 10);

	QScrollArea* scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setMinimumSize(QSize(200, 200));
	scrollArea->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setBackgroundRole(QPalette::Light);
	scrollArea->setWidget(lWidget);

	cbCheckAll = new QCheckBox(tr("Check All"), this);
	cbCheckAll->setTristate(true);
	connect(cbCheckAll, SIGNAL(clicked(bool)), this, SLOT(checkAll(bool)));

	QVBoxLayout* l = new QVBoxLayout(this);
	l->addWidget(scrollArea);
	l->addWidget(cbCheckAll);
}

void DkMetaDataSelection::appendGUIEntry(const QString& key, const QString& value, int idx) {

	QString cleanKey = key;
	cleanKey = cleanKey.replace(".", " > ");

	QCheckBox* cb = new QCheckBox(cleanKey, this);
	connect(cb, SIGNAL(clicked()), this, SLOT(selectionChanged()));
	selection.append(cb);

	QString cleanValue = DkUtils::cleanFraction(value);
	QDateTime pd = DkUtils::getConvertableDate(cleanValue);

	if (!pd.isNull())
		cleanValue = pd.toString(Qt::SystemLocaleShortDate);

	QLabel* label = new QLabel(cleanValue, this);
	label->setObjectName("DkMetadataValueLabel");

	if (idx == -1)
		idx = keys.size();

	layout->addWidget(cb, idx, 1);
	layout->addWidget(label, idx, 2);
}

void DkMetaDataSelection::checkAll(bool checked) {

	for (QCheckBox* cb : selection)
		cb->setChecked(checked);

}

void DkMetaDataSelection::selectionChanged() {

	bool sel = false;
	bool partial = false;
	cbCheckAll->setTristate(false);

	for (int idx = 0; idx < selection.size(); idx++) {

		if (idx > 0 && sel != selection.at(idx)->isChecked()) {
			cbCheckAll->setCheckState(Qt::PartiallyChecked);
			partial = true;
			break;
		}

		sel = selection.at(idx)->isChecked();
	}

	if (!partial)
		cbCheckAll->setChecked(sel);

	qDebug() << "selection changed...";
}

void DkMetaDataSelection::setSelectedKeys(const QStringList& selKeys) {

	for (QString key : selKeys) {

		int idx = keys.indexOf(key);

		if (idx != -1) {
			selection.at(idx)->setChecked(true);
		}
		else {
			
			// append entries that are not available in the current image
			keys.append(key);
			appendGUIEntry(key, "");
			selection.last()->setChecked(true);
		}
	}

	selectionChanged();
}

QStringList DkMetaDataSelection::getSelectedKeys() const {

	QStringList selKeys;

	for (int idx = 0; idx < selection.size(); idx++) {
		
		if (selection.at(idx)->isChecked())
			selKeys.append(keys.at(idx));
	}

	return selKeys;
}

void DkMetaDataSelection::createEntries(QSharedPointer<DkMetaDataT> metaData, QStringList& outKeys, QStringList& outValues) const {

	if (!metaData)
		return;

	metaData->getFileMetaData(outKeys, outValues);
	metaData->getAllMetaData(outKeys, outValues);
}

// DkMetaDataHUD --------------------------------------------------------------------
DkMetaDataHUD::DkMetaDataHUD(QWidget* parent) : DkWidget(parent) {

	setObjectName("DkMetaDataHUD");

	// some inits
	numColumns = -1;
	windowPosition = pos_south;
	orientation = Qt::Horizontal;
	keyValues = getDefaultKeys();
	contextMenu = 0;

	loadSettings();

	if (windowPosition == pos_west || windowPosition == pos_east)
		orientation = Qt::Vertical;

	createLayout();
	createActions();
}

DkMetaDataHUD::~DkMetaDataHUD() {

	saveSettings();
}

void DkMetaDataHUD::createLayout() {

	contentLayout = new QGridLayout(this);
	updateLabels();
}

void DkMetaDataHUD::createActions() {

	actions.resize(action_end);

	actions[action_change_keys] = new QAction(tr("Change Entries"), this);
	actions[action_change_keys]->setStatusTip(tr("You can customize the entries displayed here."));
	connect(actions[action_change_keys], SIGNAL(triggered()), this, SLOT(changeKeys()));

	actions[action_num_columns] = new QAction(tr("Number of Columns"), this);
	actions[action_num_columns]->setStatusTip(tr("Select the desired number of columns."));
	connect(actions[action_num_columns], SIGNAL(triggered()), this, SLOT(changeNumColumns()));

	actions[action_set_to_default] = new QAction(tr("Set to Default"), this);
	actions[action_set_to_default]->setStatusTip(tr("Reset the metadata panel."));
	connect(actions[action_set_to_default], SIGNAL(triggered()), this, SLOT(setToDefault()));

	// orientations
	actions[action_pos_west] = new QAction(tr("Show Left"), this);
	actions[action_pos_west]->setStatusTip(tr("Shows the Metadata on the Left"));
	connect(actions[action_pos_west], SIGNAL(triggered()), this, SLOT(newPosition()));

	actions[action_pos_north] = new QAction(tr("Show Top"), this);
	actions[action_pos_north]->setStatusTip(tr("Shows the Metadata at the Top"));
	connect(actions[action_pos_north], SIGNAL(triggered()), this, SLOT(newPosition()));

	actions[action_pos_east] = new QAction(tr("Show Right"), this);
	actions[action_pos_east]->setStatusTip(tr("Shows the Metadata on the Right"));
	connect(actions[action_pos_east], SIGNAL(triggered()), this, SLOT(newPosition()));

	actions[action_pos_south] = new QAction(tr("Show Bottom"), this);
	actions[action_pos_south]->setStatusTip(tr("Shows the Metadata at the Bottom"));
	connect(actions[action_pos_south], SIGNAL(triggered()), this, SLOT(newPosition()));

}

void DkMetaDataHUD::loadSettings() {

	QSettings& settings = Settings::instance().getSettings();

	settings.beginGroup(objectName());
	QStringList keyVals = settings.value("keyValues", QStringList()).toStringList();
	numColumns = settings.value("numColumns", numColumns).toInt();
	windowPosition = settings.value("windowPosition", windowPosition).toInt();
	settings.endGroup();

	if (!keyVals.isEmpty())
		keyValues = keyVals;
}

void DkMetaDataHUD::saveSettings() const {

	if (keyValues.isEmpty())
		return;

	QSettings& settings = Settings::instance().getSettings();

	settings.beginGroup(objectName());
	settings.setValue("keyValues", keyValues);
	settings.setValue("numColumns", numColumns);
	settings.setValue("windowPosition", windowPosition);
	settings.endGroup();
}

int DkMetaDataHUD::getWindowPosition() const {
	
	return windowPosition;
}

QStringList DkMetaDataHUD::getDefaultKeys() const {

	QStringList keyValues;

	// TODO: use useful ones
	keyValues.append("File." + QObject::tr("Filename"));
	keyValues.append("File." + QObject::tr("Path"));
	keyValues.append("Exif.Image.Make");
	keyValues.append("Xmp.xmp.Rating");
	keyValues.append("Exif.Image.ImageWidth");
	keyValues.append("Exif.Image.Model");
	keyValues.append("Exif.Image.Orientation");
	keyValues.append("Exif.Image.DateTime");
	keyValues.append("Exif.Photo.FNumber");
	keyValues.append("Exif.Photo.MaxApertureValue");
	keyValues.append("Exif.Image.Software");
	keyValues.append("Exif.Image.DateTime");
	keyValues.append("Exif.Photo.ImageLength");

	return keyValues;
}

void DkMetaDataHUD::updateMetaData(const QSharedPointer<DkImageContainerT> cImg) {

	if (cImg) {
		metaData = cImg->getMetaData();
		updateMetaData(metaData);
	}
	else
		metaData = QSharedPointer<DkMetaDataT>();
}

void DkMetaDataHUD::updateMetaData(const QSharedPointer<DkMetaDataT> metaData) {

	// clean up
	for (QLabel* cLabel : entryKeyLabels)
		cLabel->deleteLater();
	for (QLabel* cLabel : entryValueLabels)
		cLabel->deleteLater();

	entryKeyLabels.clear();
	entryValueLabels.clear();

	if (!metaData) {

		// create dummy entries
		for (QString cKey : keyValues) {
			entryKeyLabels.append(createKeyLabel(cKey));
		}
		return;
	}

	DkTimer dt;

	QStringList fileKeys, fileValues;
	metaData->getFileMetaData(fileKeys, fileValues);

	for (int idx = 0; idx < fileKeys.size(); idx++) {

		QString cKey = fileKeys.at(idx);
		qDebug() << "ckey: " << cKey;
		if (keyValues.contains(cKey)) {
			entryKeyLabels.append(createKeyLabel(cKey));
			entryValueLabels.append(createValueLabel(fileValues.at(idx)));
		}
	}

	QStringList exifKeys = metaData->getExifKeys();

	for (int idx = 0; idx < exifKeys.size(); idx++) {

		QString cKey = exifKeys.at(idx);

		if (keyValues.contains(cKey)) {
			QString lastKey = cKey.split(".").last();
			QString exifValue = metaData->getNativeExifValue(exifKeys.at(idx));
			exifValue = DkMetaDataHelper::getInstance().resolveSpecialValue(metaData, lastKey, exifValue);

			entryKeyLabels.append(createKeyLabel(cKey));
			entryValueLabels.append(createValueLabel(exifValue));
		}
	}

	QStringList iptcKeys = metaData->getIptcKeys();

	for (int idx = 0; idx < iptcKeys.size(); idx++) {

		QString cKey = iptcKeys.at(idx);

		if (keyValues.contains(cKey)) {

			QString lastKey = iptcKeys.at(idx).split(".").last();
			QString exifValue = metaData->getIptcValue(iptcKeys.at(idx));
			exifValue = DkMetaDataHelper::getInstance().resolveSpecialValue(metaData, lastKey, exifValue);

			entryKeyLabels.append(createKeyLabel(cKey));
			entryValueLabels.append(createValueLabel(exifValue));
		}
	}

	QStringList xmpKeys = metaData->getXmpKeys();

	for (int idx = 0; idx < xmpKeys.size(); idx++) {

		QString cKey = xmpKeys.at(idx);

		if (keyValues.contains(cKey)) {

			QString lastKey = xmpKeys.at(idx).split(".").last();
			QString exifValue = metaData->getXmpValue(xmpKeys.at(idx));
			exifValue = DkMetaDataHelper::getInstance().resolveSpecialValue(metaData, lastKey, exifValue);

			entryKeyLabels.append(createKeyLabel(cKey));
			entryValueLabels.append(createValueLabel(exifValue));
		}
	}

	updateLabels();
}

void DkMetaDataHUD::updateLabels(int numColumns /* = -1 */) {

	if (numColumns == -1 && this->numColumns == -1) {
		int numLines = 6;
		numColumns = ((float)entryKeyLabels.size()+numLines-1)/numLines > 2 ? qRound(((float)entryKeyLabels.size()+numLines-1)/numLines) : 2;
	}
	else if (numColumns == -1) {
		numColumns = this->numColumns;
	}

	if (orientation == Qt::Vertical)
		numColumns = 1;

	int cIdx = 0;
	int rIdx = 0;
	int nRows = cvCeil((float)(entryKeyLabels.size())/numColumns);

	for (int idx = 0; idx < entryKeyLabels.size(); idx++) {

		if (idx && idx % nRows == 0) {
			rIdx = 0;
			cIdx += 2;
		}
		 
		contentLayout->addWidget(entryKeyLabels.at(idx), rIdx, cIdx, 1, 1, Qt::AlignTop);
		contentLayout->addWidget(entryValueLabels.at(idx), rIdx, cIdx+1, 1, 1, Qt::AlignTop);
		rIdx++;
	}


	if (orientation == Qt::Vertical)
		contentLayout->setRowStretch(1000, 10);
	else
		contentLayout->setRowStretch(1000, 0);

}

QLabel* DkMetaDataHUD::createKeyLabel(const QString& key) {

	QString labelString = key.split(".").last();
	labelString = DkMetaDataHelper::getInstance().translateKey(labelString);
	QLabel* keyLabel = new QLabel(labelString, this);
	keyLabel->setObjectName("DkMetaDataKeyLabel");
	keyLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	keyLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

	return keyLabel;
}

QLabel* DkMetaDataHUD::createValueLabel(const QString& val) {

	QString cleanValue = DkUtils::cleanFraction(val);
	QDateTime pd = DkUtils::getConvertableDate(cleanValue);

	if (!pd.isNull())
		cleanValue = pd.toString(Qt::SystemLocaleShortDate);

	QLabel* valLabel = new QLabel(cleanValue.trimmed(), this);
	valLabel->setObjectName("DkMetaDataLabel");
	valLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	valLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

	return valLabel;
}

// events
void DkMetaDataHUD::contextMenuEvent(QContextMenuEvent *event) {

	if (!contextMenu) {
		contextMenu = new QMenu(tr("Metadata Menu"), this);
		contextMenu->addActions(actions.toList());
	}

	contextMenu->exec(event->globalPos());
	event->accept();

	//DkWidget::contextMenuEvent(event);
}

// public slots...
void DkMetaDataHUD::newPosition() {

	QAction* sender = static_cast<QAction*>(QObject::sender());

	if (!sender)
		return;

	int pos = 0;
	Qt::Orientation orient = Qt::Horizontal;

	if (sender == actions[action_pos_west]) {
		pos = pos_west;
		orient = Qt::Vertical;
	}
	else if (sender == actions[action_pos_east]) {
		pos = pos_east;
		orient = Qt::Vertical;
	}
	else if (sender == actions[action_pos_north]) {
		pos = pos_north;
		orient = Qt::Horizontal;
	}
	else {
		pos = pos_south;
		orient = Qt::Horizontal;
	}

	windowPosition = pos;
	orientation = orient;
	emit positionChangeSignal(windowPosition);

	updateLabels();
}

void DkMetaDataHUD::changeKeys() {

	QDialog* dialog = new QDialog(this);
	QVBoxLayout* layout = new QVBoxLayout(dialog);
		
	DkMetaDataSelection* selWidget = new DkMetaDataSelection(metaData, this);
	selWidget->setSelectedKeys(keyValues);

	// buttons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), dialog, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), dialog, SLOT(reject()));

	layout->addWidget(selWidget);
	layout->addWidget(buttons);

	int res = dialog->exec();

	if (res == QDialog::Accepted) {
		keyValues = selWidget->getSelectedKeys();
		updateMetaData(metaData);
	}

	dialog->deleteLater();
}

void DkMetaDataHUD::changeNumColumns() {

	bool ok;
	int val = QInputDialog::getInt(this, tr("Number of Columns"), tr("Number of columns (-1 is default)"), numColumns, -1, 20, 1, &ok);

	if (ok) {
		numColumns = val;
		updateLabels(numColumns);
	}
}

void DkMetaDataHUD::setToDefault() {

	numColumns = -1;
	keyValues = getDefaultKeys();
	updateMetaData(metaData);
}

//QString DkMetaDataInfo::sExifTags = QString("ImageWidth ImageLength Orientation Make Model Rating ApertureValue ShutterSpeedValue Flash FocalLength ") %
//	QString("ExposureMode ExposureTime UserComment DateTime DateTimeOriginal ImageDescription");
//QString DkMetaDataInfo::sExifDesc = QString("Image Width;Image Length;Orientation;Make;Model;Rating;Aperture Value;Shutter Speed Value;Flash;FocalLength;") %
//	QString("Exposure Mode;Exposure Time;User Comment;Date Time;Date Time Original;Image Description");
//QString DkMetaDataInfo::sIptcTags = QString("Iptc.Application2.Byline Iptc.Application2.BylineTitle Iptc.Application2.City Iptc.Application2.Country ") %
//	QString("Iptc.Application2.Headline Iptc.Application2.Caption Iptc.Application2.Copyright Iptc.Application2.Keywords");
//QString DkMetaDataInfo::sIptcDesc = QString("Creator;Creator Title;City;Country;Headline;Caption;Copyright;Keywords");

DkMetaDataInfo::DkMetaDataInfo(QWidget* parent) : DkWidget(parent) {

	setObjectName("DkMetaDataInfo");

	this->parent = parent;

	exifHeight = 120;
	minWidth = 900;
	fontSize = 12;
	textMargin = 10;
	numLines = 6;
	maxCols = 4;
	numLabels = 0;
	gradientWidth = 100;

	yMargin = 6;
	xMargin = 8;

	setMaximumSize(QWIDGETSIZE_MAX, exifHeight);
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
}

void DkMetaDataInfo::init() {

	mapIptcExif[DkSettings::camData_size] = 0;
	mapIptcExif[DkSettings::camData_orientation] = 0;
	mapIptcExif[DkSettings::camData_make] = 0;
	mapIptcExif[DkSettings::camData_model] = 0;
	mapIptcExif[DkSettings::camData_aperture] = 0;
	//mapIptcExif[DkSettings::camData_shutterspeed] = 0;
	mapIptcExif[DkSettings::camData_flash] = 0;
	mapIptcExif[DkSettings::camData_focallength] = 0;
	mapIptcExif[DkSettings::camData_exposuremode] = 0;
	mapIptcExif[DkSettings::camData_exposuretime] = 0;

	mapIptcExif[DkSettings::desc_rating] = 0;
	mapIptcExif[DkSettings::desc_usercomment] = 0;
	mapIptcExif[DkSettings::desc_date] = 0;
	mapIptcExif[DkSettings::desc_datetimeoriginal] = 0;
	mapIptcExif[DkSettings::desc_imagedescription] = 0;
	mapIptcExif[DkSettings::desc_creator] = 1;
	mapIptcExif[DkSettings::desc_creatortitle] = 1;
	mapIptcExif[DkSettings::desc_city] = 1;
	mapIptcExif[DkSettings::desc_country] = 1;
	mapIptcExif[DkSettings::desc_headline] = 1;
	mapIptcExif[DkSettings::desc_caption] = 1;
	mapIptcExif[DkSettings::desc_copyright] = 1;
	mapIptcExif[DkSettings::desc_keywords] = 1;

	mapIptcExif[DkSettings::desc_path] = 2;
	mapIptcExif[DkSettings::desc_filesize] = 2;

	worldMatrix = QTransform();

	setMouseTracking(true);
	//readTags();

	QColor tmpCol = bgCol;
	tmpCol.setAlpha(0);

	leftGradientRect = QRect(QPoint(), QSize(gradientWidth, exifHeight));
	leftGradient = QLinearGradient(leftGradientRect.topLeft(), leftGradientRect.topRight());
	leftGradient.setColorAt(0, tmpCol);
	leftGradient.setColorAt(1, bgCol);

	rightGradientRect = QRect(QPoint(size().width()-gradientWidth, 0), QSize(gradientWidth, exifHeight));
	rightGradient = QLinearGradient(rightGradientRect.topLeft(), rightGradientRect.topRight());
	rightGradient.setColorAt(0, bgCol);
	rightGradient.setColorAt(1, tmpCol);

}
void DkMetaDataInfo::setImageInfo(QSharedPointer<DkImageContainerT> imgC) {

	this->imgC = imgC;
	worldMatrix = QTransform();

	//DkTimer dt;
	if (isVisible()) {
		readTags();
		createLabels();
	}
}

void DkMetaDataInfo::readTags() {

	if (!imgC)
		return;

	try {
		if (mapIptcExif.empty())
			init();

		camDValues.clear();
		descValues.clear();


		//QString preExifI = "Exif.Image.";
		//QString preExifP = "Exif.Photo.";
		QString preIptc = "Iptc.Application2.";

		QFileInfo file = imgC->file();
#ifdef WITH_QUAZIP		
		if(imgC->isFromZip()) file = imgC->getZipData()->getZipFileInfo();
#endif
		QSharedPointer<DkMetaDataT> metaData = imgC->getMetaData();
		QStringList camSearchTags = DkMetaDataHelper::getInstance().getCamSearchTags();
		QStringList descSearchTags = DkMetaDataHelper::getInstance().getDescSearchTags();

		//if (metaData->isLoaded()) {

		for (int i=0; i<camSearchTags.size(); i++) {
			QString tmp, Value;

			if (mapIptcExif[i] == 0) {

				//tmp = preExifI + camDTags.at(i);
				tmp = camSearchTags.at(i);

				//special treatments
				// aperture
				if (i == DkSettings::camData_aperture) {
					Value = DkMetaDataHelper::getInstance().getApertureValue(metaData);
				}
				// focal length
				else if (i == DkSettings::camData_focallength) {
					Value = DkMetaDataHelper::getInstance().getFocalLength(metaData);
				}
				// exposure time
				else if (i == DkSettings::camData_exposuretime) {
					Value = DkMetaDataHelper::getInstance().getExposureTime(metaData);
				}
				else if (i == DkSettings::camData_size) {	
					Value = QString::number(imgC->image().width()) + " x " + QString::number(imgC->image().height());
				}
				else if (i == DkSettings::camData_exposuremode) {
					Value = DkMetaDataHelper::getInstance().getExposureMode(metaData);						
				} 
				else if (i == DkSettings::camData_flash) {
					Value = DkMetaDataHelper::getInstance().getFlashMode(metaData);
				}
				else {
					//qDebug() << "size" << imgSize.width() << imgSize.height();
					Value = metaData->getExifValue(tmp);
				}
			} else if (mapIptcExif[i] == 1) {
				tmp = preIptc + camSearchTags.at(i);
				Value = metaData->getIptcValue(tmp);
			}

			camDValues << Value;
		}
		//use getRating for Rating Value... otherwise the value is probably not correct: also Xmp.xmp.Rating, Xmp.MicrosoftPhoto.Rating is used
		QString rating;
		float tmp = (float)metaData->getRating();
		if (tmp < 0) tmp=0;
		rating.setNum(tmp);
		descValues << rating;

		for (int i=1; i<descSearchTags.size(); i++) {
			QString tmp, Value;

			if (mapIptcExif[DkSettings::camData_end + i] == 0) {
				//tmp = preExifI + camDTags.at(i);
				tmp = descSearchTags.at(i);
				//qDebug() << tmp;
				Value = metaData->getExifValue(tmp);

				if (tmp.contains("Date"))
					Value = DkUtils::convertDateString(Value, file);

			} else if (mapIptcExif[DkSettings::camData_end + i] == 1) {
				tmp = preIptc + descSearchTags.at(i);
				Value = metaData->getIptcValue(tmp);
			} else if (mapIptcExif[DkSettings::camData_end + i] == 2) {
				//all other defined tags not in metadata
				tmp = descSearchTags.at(i);
				if (!tmp.compare("Path")) {
					Value = QString(file.absoluteFilePath());
				}
				else if (!tmp.compare("FileSize")) {
					Value = DkUtils::readableByte((float)file.size());
				} else
					Value = QString();

				//qDebug() << Value << "should be filepath";
			}
			descValues << Value;
		}
		//} else
		//	qDebug() << "Exif: file is not defined...";

	} catch (...) {
		qDebug() << "could not load Exif information";
	}
}

void DkMetaDataInfo::createLabels() {


	if (camDValues.empty() && descValues.empty()) {
		qDebug() << "no labels read (Exif)";
		return;
	}

	QStringList camDTags = DkMetaDataHelper::getInstance().getTranslatedCamTags();
	QStringList descTags = DkMetaDataHelper::getInstance().getTranslatedDescTags();

	if (camDValues.size() != camDTags.size() || descValues.size() != descTags.size()) {
		qDebug() << "error reading metadata: tag number is not equal value number";
		qDebug() << "cam value size: " << camDValues.size() << " cam tag size: " << camDTags.size();
		qDebug() << "desc value size: " << descValues.size() << " desc tag size: " << descTags.size();
		return;
	}

	for (int idx = 0; idx < pLabels.size(); idx++) {
		delete pLabels.at(idx);
	}

	for (int idx = 0; idx < pValues.size(); idx++) {
		delete pValues.at(idx);
	}

	pLabels.clear();
	pValues.clear();

	numLabels = 0;
	numLines = 6;
	for (int idx = 0; idx < DkSettings::metaData.metaDataBits.size(); idx++) {
		if (DkSettings::metaData.metaDataBits.testBit(idx))
			numLabels++;
	}

	// well that's a bit of a hack
	int cols = ((float)numLabels+numLines-1)/numLines > 2 ? qRound(((float)numLabels+numLines-1)/numLines) : 2;
	numLines = cvCeil((float)numLabels/cols);


	//pLabels.resize(camDTags.size() + descTags.size());
	//6 Lines...
	maxLenLabel.resize(cols);
	for (int i=0; i<cols; i++)
		maxLenLabel[i] = 0;

	numLabels=0;

	for(int i=0; i<camDTags.size(); i++) {
		//if bit set, create Label
		if (DkSettings::metaData.metaDataBits.testBit(i)) {
			DkLabel* pl = new DkLabel(this);//, camDTags.at(i));
			pl->setText(camDTags.at(i)+":",-1);
			pl->setFontSize(fontSize);
			pl->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
			pl->setMouseTracking(true);
			DkLabel* pv = new DkLabel(this);//, camDValues.at(i));
			pv->setText(camDValues.at(i),-1);
			pv->setFontSize(fontSize);
			pv->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
			pv->setMouseTracking(true);
			pLabels << pl;
			pValues << pv; 
			if (pl->geometry().width() > maxLenLabel[numLabels/numLines]) maxLenLabel[numLabels/numLines] = pl->geometry().width();
			numLabels++;
		}
	}

	for(int i=0; i<descTags.size(); i++) {
		//if bit set, create Label
		if (DkSettings::metaData.metaDataBits.testBit(DkSettings::camData_end + i)) {
			DkLabel* pl = new DkLabel(this);
			pl->setText(descTags.at(i)+":",-1);
			pl->setFontSize(fontSize);
			pl->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
			pl->setMouseTracking(true);
			DkLabel* pv = new DkLabel(this);
			pv->setText(descValues.at(i),-1);
			pv->setFontSize(fontSize);
			pv->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
			pv->setMouseTracking(true);
			pLabels << pl;
			pValues << pv;
			if (pl->geometry().width() > maxLenLabel[numLabels/numLines]) maxLenLabel[numLabels/numLines] = pl->geometry().width();
			numLabels++;
		}
	}


	//qDebug() << camDTags;
	//qDebug() << camDValues;

	layoutLabels();
}

void DkMetaDataInfo::layoutLabels() {

	if (pLabels.isEmpty())
		return;

	// #Labels / numLines = #Spalten
	numLines = 6;
	int cols = ((float)numLabels+numLines-1)/numLines > 2 ? qRound(((float)numLabels+numLines-1)/numLines) : 2;
	numLines = cvCeil((float)numLabels/cols);

	//qDebug() << "numCols: " << cols;

	if (cols > maxCols)
		qDebug() << "Labels are skipped...";

	//if (cols == 1) {
	exifHeight = (pLabels.at(0)->height() + yMargin)*numLines + yMargin;
	//} else exifHeight = 120;

	//widget size
	if (width() < minWidth)
		QWidget::setCursor(Qt::OpenHandCursor);
	else
		QWidget::unsetCursor();

	int width;
	//if (widthParent)
	//	width = widthParent > minWidth ? widthParent : minWidth;
	//else
	width = this->width() > minWidth ? this->width() : minWidth;

	//qDebug() << "width" << parent->width();

	//set Geometry if exif height is changed
	//setGeometry(0, parent->height()-exifHeight, parent->width(), exifHeight);

	//subtract label length
	for (int i=0; i<maxLenLabel.size(); i++) width -= (maxLenLabel[i] + xMargin);
	width-=xMargin;

	//rest length/#cols = column width for tags
	int widthValues = width/cols > 0 ? width/cols : 10;

	//subtract margin
	for (int i=0; i< cols; i++) widthValues -= xMargin;


	QPoint pos(xMargin + maxLenLabel[0], yMargin);
	int textHeight = fontSize;

	//set positions for labels
	for (int i=0; i<pLabels.size(); i++) {

		if (i%numLines == 0 && i>0) {
			pos = QPoint(pos.x() + maxLenLabel[i/numLines] + widthValues + xMargin*2, yMargin);

		}

		QPoint tmp = pos + QPoint(0, i*textHeight + i*yMargin);

		QRect tmpRect = pLabels.at(i)->geometry();
		tmpRect.moveTopRight(QPoint(pos.x(), pos.y() + (i%numLines)*textHeight + (i%numLines)*yMargin));
		pLabels.at(i)->setGeometry(tmpRect);

		tmpRect = pValues.at(i)->geometry();
		tmpRect.moveTopLeft(pos + QPoint(xMargin, (i%numLines)*textHeight + (i%numLines)*yMargin));
		pValues.at(i)->setGeometry(tmpRect);
		pValues.at(i)->setFixedWidth(widthValues);

	}
}

void DkMetaDataInfo::updateLabels() {

	if (mapIptcExif.empty())
		setImageInfo(imgC);

	createLabels();
}

void DkMetaDataInfo::setVisible(bool visible, bool showSettings) {

	if (visible) {
		readTags();
		createLabels();
	}

	qDebug() << "[DkMetaData] setVisible: " << visible;

	DkWidget::setVisible(visible, showSettings);
}

void DkMetaDataInfo::setRating(int rating) {

	QString sRating;
	sRating.setNum(rating);

	for (int i=0; i<pLabels.size(); i++) {

		QString tmp = pLabels.at(i)->getText();
		if (!tmp.compare("Rating:")) {
			pValues.at(i)->setText(sRating, -1);
		}
	}
}

void DkMetaDataInfo::paintEvent(QPaintEvent *event) {

	QPainter painter(this);

	draw(&painter);
	painter.end();

	DkWidget::paintEvent(event);
}

void DkMetaDataInfo::resizeEvent(QResizeEvent *resizeW) {

	//resize(parent->width(), resizeW->size().height());

	int gw = qMin(gradientWidth, qRound(0.2f*resizeW->size().width()));

	rightGradientRect.setTopLeft(QPoint(resizeW->size().width()-gw, 0));
	rightGradientRect.setSize(QSize(gw, resizeW->size().height()));
	leftGradientRect.setSize(QSize(gw, resizeW->size().height()));

	rightGradient.setStart(rightGradientRect.topLeft());
	rightGradient.setFinalStop(rightGradientRect.topRight());
	leftGradient.setStart(leftGradientRect.topLeft());
	leftGradient.setFinalStop(leftGradientRect.topRight());

	if (resizeW->size().width() > minWidth) {
		worldMatrix = QTransform();
		layoutLabels();
		//qDebug() << "parent->width() > minWidth  d.h. layoutlabels";
	}

	if (resizeW->size().width() < minWidth && worldMatrix.dx() == 0) {
		layoutLabels();
	}

	if ((resizeW->size().width() < minWidth) && (worldMatrix.dx()+minWidth < resizeW->size().width())) {
		//layoutLabels();
		QTransform tmpMatrix = QTransform();
		int dX = (resizeW->size().width()-minWidth) - qRound(worldMatrix.dx());

		tmpMatrix.translate(dX, 0);
		worldMatrix.translate(dX, 0);

		for (int i=0; i< pLabels.size(); i++) {
			pLabels.at(i)->setGeometry(tmpMatrix.mapRect(pLabels.at(i)->geometry()));
			pValues.at(i)->setGeometry(tmpMatrix.mapRect(pValues.at(i)->geometry()));
		}
		//layoutLabels();
	}

	DkWidget::resizeEvent(resizeW);
}

void DkMetaDataInfo::draw(QPainter* painter) {

	//QImage* img;

	if (!painter)
		return;

	//labels are left outside of the widget -> set gradient
	if (width() < minWidth && worldMatrix.dx() < 0) {

		if (-worldMatrix.dx() < leftGradientRect.width())
			leftGradient.setFinalStop(-worldMatrix.dx(), 0);
		painter->fillRect(leftGradientRect, leftGradient);
	}
	else
		painter->fillRect(leftGradientRect, bgCol);

	//labels are right outside of the widget -> set gradient
	if (width() < minWidth && worldMatrix.dx()+minWidth > width()) {

		int rightOffset = qRound(worldMatrix.dx())+minWidth-width();
		if (rightOffset < rightGradientRect.width())
			rightGradient.setStart(rightGradientRect.left()+(rightGradientRect.width() - rightOffset), 0);
		painter->fillRect(rightGradientRect, rightGradient);
	}
	else
		painter->fillRect(rightGradientRect, bgCol);

	painter->fillRect(QRect(QPoint(leftGradientRect.right()+1,0), QSize(size().width()-leftGradientRect.width()-rightGradientRect.width(), size().height())), bgCol);
}

void DkMetaDataInfo::mouseMoveEvent(QMouseEvent *event) {


	if (lastMousePos.isNull()) {
		lastMousePos = event->pos();
		QWidget::mouseMoveEvent(event);
		return;
	}

	if (event->buttons() == Qt::LeftButton && width() < minWidth)
		QWidget::setCursor(Qt::ClosedHandCursor);
	if (event->buttons() != Qt::LeftButton && width() > minWidth)
		QWidget::unsetCursor();
	if (event->buttons() != Qt::LeftButton && width() < minWidth)
		QWidget::setCursor(Qt::OpenHandCursor);


	if (event->buttons() != Qt::LeftButton)
		lastMousePos = event->pos();

	int mouseDir = event->pos().x() - lastMousePos.x();


	if (width() < minWidth && event->buttons() == Qt::LeftButton) {

		currentDx = (float)mouseDir;

		lastMousePos = event->pos();

		QTransform tmpMatrix = QTransform();
		tmpMatrix.translate(currentDx, 0);

		if (((worldMatrix.dx()+currentDx)+minWidth >= width() && currentDx < 0) ||
			((worldMatrix.dx()+currentDx) <= 0 && currentDx > 0)) {

				worldMatrix.translate(currentDx, 0);

				for (int i=0; i< pLabels.size(); i++) {
					pLabels.at(i)->setGeometry(tmpMatrix.mapRect(pLabels.at(i)->geometry()));
					pValues.at(i)->setGeometry(tmpMatrix.mapRect(pValues.at(i)->geometry()));
				}
		}

		update();

		return;
	}

	lastMousePos = event->pos();

	QWidget::mouseMoveEvent(event);
}

// DkCommentTextEdit --------------------------------------------------------------------
DkCommentTextEdit::DkCommentTextEdit(QWidget* parent /* = 0 */) : QTextEdit(parent) {

}

void DkCommentTextEdit::focusOutEvent(QFocusEvent *focusEvent) {
	emit focusLost();
	QTextEdit::focusOutEvent(focusEvent);
}

void DkCommentTextEdit::paintEvent(QPaintEvent* e) {

	if (toPlainText().isEmpty() && !viewport()->hasFocus()) {
		QPainter p(viewport());
		p.setOpacity(0.5);
		p.drawText(QRect(QPoint(), viewport()->size()), Qt::AlignHCenter | Qt::AlignVCenter, tr("Click here to add notes"));
		qDebug() << "painting placeholder...";
	}
	
	QTextEdit::paintEvent(e);
}

// DkCommentWidget --------------------------------------------------------------------
DkCommentWidget::DkCommentWidget(QWidget* parent /* = 0 */, Qt::WindowFlags /* = 0 */) : DkFadeLabel(parent) {

	textChanged = false;
	setMaximumSize(220, 150);
	createLayout();
	QMetaObject::connectSlotsByName(this);
}

void DkCommentWidget::createLayout() {

	setObjectName("DkCommentWidget");

	titleLabel = new QLabel(tr("NOTES"), this);
	titleLabel->setObjectName("commentTitleLabel");

	QString scrollbarStyle = 
		QString("QScrollBar:vertical {border: 1px solid #FFF; background: rgba(0,0,0,0); width: 7px; margin: 0 0 0 0;}")
		+ QString("QScrollBar::handle:vertical {background: #FFF; min-height: 0px;}")
		+ QString("QScrollBar::add-line:vertical {height: 0px;}")
		+ QString("QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {background: rgba(0,0,0,0); width: 1px;}")
		+ QString("QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {height: 0;}");

	commentLabel = new DkCommentTextEdit(this);
	commentLabel->setObjectName("CommentLabel");
	commentLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	commentLabel->setStyleSheet(scrollbarStyle + commentLabel->styleSheet());
	commentLabel->setToolTip(tr("Enter your notes here. They will be saved to the image metadata."));

	QPushButton* saveButton = new QPushButton(this);
	saveButton->setObjectName("saveButton");
	saveButton->setFlat(true);
	saveButton->setIcon(QIcon(DkImage::colorizePixmap(QPixmap(":/nomacs/img/save.png"), QColor(255,255,255,255), 1.0f)));
	saveButton->setToolTip(tr("Save Note (CTRL + ENTER)"));
	saveButton->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return));

	QPushButton* cancelButton = new QPushButton(this);
	cancelButton->setObjectName("cancelButton");
	cancelButton->setFlat(true);
	cancelButton->setIcon(QIcon(DkImage::colorizePixmap(QPixmap(":/nomacs/img/trash.png"), QColor(255,255,255,255), 1.0f)));
	cancelButton->setToolTip(tr("Discard Changes (ESC)"));
	cancelButton->setShortcut(QKeySequence(Qt::Key_Escape));

	QWidget* titleWidget = new QWidget(this);
	QHBoxLayout* titleLayout = new QHBoxLayout(titleWidget);
	titleLayout->setAlignment(Qt::AlignLeft);
	titleLayout->setContentsMargins(0, 0, 0, 0);
	titleLayout->setSpacing(0);
	titleLayout->addWidget(titleLabel);
	titleLayout->addStretch();
	titleLayout->addWidget(cancelButton, 0, Qt::AlignVCenter);
	titleLayout->addWidget(saveButton, 0, Qt::AlignVCenter);

	QVBoxLayout* layout = new QVBoxLayout(this);
	//layout->setContentsMargins(0,0,0,0);
	layout->addWidget(titleWidget);
	layout->addWidget(commentLabel);

	setLayout(layout);
	setCursor(Qt::ArrowCursor);
}

void DkCommentWidget::setMetaData(QSharedPointer<DkMetaDataT> metaData) {

	this->metaData = metaData;
	setComment(metaData->getDescription());
}

void DkCommentWidget::setComment(const QString& description) {
	
	commentLabel->setText(description);

	oldText = description;
	dirty = false;
}

void DkCommentWidget::saveComment() {

	if (textChanged && commentLabel->toPlainText() != metaData->getDescription() && metaData) {
		
		if (!metaData->setDescription(commentLabel->toPlainText()) && !commentLabel->toPlainText().isEmpty()) {
			emit showInfoSignal(tr("Sorry, I cannot save comments for this image format."));
		}
		else
			dirty = true;
	}
}

void DkCommentWidget::on_CommentLabel_textChanged() {

	textChanged = true;
}

void DkCommentWidget::on_CommentLabel_focusLost() {

	saveComment();
}

void DkCommentWidget::on_saveButton_clicked() {

	commentLabel->clearFocus();
	//if (parent)
	//	parent->setFocus(Qt::MouseFocusReason);
}

void DkCommentWidget::on_cancelButton_clicked() {

	textChanged = false;
	commentLabel->clearFocus();
	commentLabel->setText("");

	saveComment();
	
	//if (parent)
	//	parent->setFocus(Qt::MouseFocusReason);
}

}