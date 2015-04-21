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

	if (!metaData)
		return;

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
		QString exifValue = metaData->getIptcValue(iptcKeys.at(idx));
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
Qt::ItemFlags DkMetaDataModel::flags(const QModelIndex& index) const {

	if (!index.isValid())
		return Qt::ItemIsEditable;

	//TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

	Qt::ItemFlags flags;

	if (index.column() == 0)
		flags = QAbstractItemModel::flags(index);
	if (index.column() == 1)
		flags = QAbstractItemModel::flags(index) | Qt::ItemIsEditable;

	return flags;
}


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


	QLabel* titleLabel = new QLabel(tr("Image Information"), this);
	titleLabel->setObjectName("DkMetaDataHUDTitle");

	QLabel* titleSeparator = new QLabel("", this);
	titleSeparator->setObjectName("DkSeparator");

	titleWidget = new QWidget(this);
	QVBoxLayout* titleLayout = new QVBoxLayout(titleWidget);
	titleLayout->addWidget(titleLabel);
	titleLayout->addWidget(titleSeparator);

	QString scrollbarStyle = 
		QString("QScrollBar:vertical {border: 1px solid #FFF; background: rgba(0,0,0,0); width: 7px; margin: 0 0 0 0;}")
		+ QString("QScrollBar::handle:vertical {background: #FFF; min-height: 0px;}")
		+ QString("QScrollBar::add-line:vertical {height: 0px;}")
		+ QString("QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {background: rgba(0,0,0,0); width: 1px;}")
		+ QString("QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {height: 0;}")
	+ QString("QScrollBar:horizontal {border: 1px solid #FFF; background: rgba(0,0,0,0); height: 7px; margin: 0 0 0 0;}")	// horizontal
		+ QString("QScrollBar::handle:horizontal {background: #FFF; min-width: 0px;}")
		+ QString("QScrollBar::add-line:horizontal {width: 0px;}")
		+ QString("QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {background: rgba(0,0,0,0); height: 1px;}")
		+ QString("QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {width: 0;}");

	scrollArea = new DkResizableScrollArea(this);
	scrollArea->setObjectName("DkScrollAreaMetaData");
	scrollArea->setWidgetResizable(true);
	scrollArea->setStyleSheet(scrollbarStyle + scrollArea->styleSheet());
	scrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	contentWidget = new QWidget(this);
	contentWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	contentLayout = new QGridLayout(contentWidget);
	updateLabels();

	scrollArea->setWidget(contentWidget);

	QVBoxLayout* l = new QVBoxLayout(this);
	l->setSpacing(0);
	l->setContentsMargins(3,3,3,3);
	l->addWidget(scrollArea);
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

	keyValues.append("File." + QObject::tr("Filename"));
	keyValues.append("File." + QObject::tr("Path"));
	keyValues.append("File." + QObject::tr("Size"));
	keyValues.append("Exif.Image.Make");
	keyValues.append("Exif.Image.Model");
	keyValues.append("Exif.Image.DateTime");
	keyValues.append("Exif.Image.ImageDescription");

	keyValues.append("Exif.Photo.ISO");
	keyValues.append("Exif.Photo.FocalLength");
	keyValues.append("Exif.Photo.ExposureTime");
	keyValues.append("Exif.Photo.Flash");
	keyValues.append("Exif.Photo.FNumber");

	return keyValues;
}

void DkMetaDataHUD::updateMetaData(const QSharedPointer<DkImageContainerT> cImg) {

	if (cImg) {
		metaData = cImg->getMetaData();
		
		// only update if I am visible
		if (isVisible())
			updateMetaData(metaData);
	}
	else
		metaData = QSharedPointer<DkMetaDataT>();
}

void DkMetaDataHUD::updateMetaData(const QSharedPointer<DkMetaDataT> metaData) {

	// clean up
	for (QLabel* cLabel : entryKeyLabels)
		delete cLabel;
	for (QLabel* cLabel : entryValueLabels)
		delete cLabel;

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

	// stretch between labels
	// we need this for correct context menu handling
	int cS = (orientation == Qt::Horizontal) ? 10 : 0;
	contentLayout->setColumnStretch(cIdx, cS); cIdx++;
	contentLayout->setRowStretch(rIdx, cS);

	titleWidget->setVisible(orientation == Qt::Vertical);
	if (orientation == Qt::Vertical)
		contentLayout->addWidget(titleWidget, 0, 0, 1, 4);

	for (int idx = 0; idx < entryKeyLabels.size(); idx++) {

		if (idx && idx % nRows == 0) {
			rIdx = 0;
			cIdx += 3;
			contentLayout->setColumnStretch(cIdx-1, cS);
		}
		 
		contentLayout->addWidget(entryKeyLabels.at(idx), rIdx+1, cIdx, 1, 1, Qt::AlignTop);
		contentLayout->addWidget(entryValueLabels.at(idx), rIdx+1, cIdx+1, 1, 1, Qt::AlignTop);
		rIdx++;
	}
	
	contentLayout->setColumnStretch(cIdx+1, cS);
	contentLayout->setRowStretch(1000, 10);	// stretch a reasonably high row (we assume to have less than 1000 entries)

	// remove old columnStretches
	for (int idx = cIdx+2; idx < 40; idx++)
		contentLayout->setColumnStretch(idx, 0);

	if (orientation == Qt::Vertical) {
		// some scroll area settings need to be adopted to the orientation
		scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	}
	else {
		// some scroll area settings need to be adopted to the orientation
		scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	}

	// TODO: I give now up on this:
	// I do not understand why Qt does not simply resize according to the
	// child widget's constraints if Qt::ScrollBarAlwaysOff is set
	// to me, this would be intended behavior
	// resizing itself is fixed, however, on layout changes it won't
	// decrease it's size
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
void DkMetaDataHUD::setVisible(bool visible, bool saveSetting /* = true */) {

	DkWidget::setVisible(visible, saveSetting);

	updateMetaData(metaData);
}

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