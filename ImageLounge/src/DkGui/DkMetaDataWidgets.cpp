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

#include "DkImageStorage.h"
#include "DkMetaData.h"
#include "DkSettings.h"
#include "DkShortcuts.h"
#include "DkThumbs.h"
#include "DkTimer.h"
#include "DkUtils.h"

#include <QAction>
#include <QCheckBox>
#include <QContextMenuEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QTextEdit>
#include <QTreeView>
#include <QVBoxLayout>
#include <qmath.h>

namespace nmc
{

// DkMetaDataModel --------------------------------------------------------------------
DkMetaDataModel::DkMetaDataModel(QObject *parent /* = 0 */)
    : QAbstractItemModel(parent)
{
    // create root
    QVector<QVariant> rootData;
    rootData << tr("Key") << tr("Value");

    rootItem = new TreeItem(rootData);
}

DkMetaDataModel::~DkMetaDataModel()
{
    delete rootItem;
}

void DkMetaDataModel::clear()
{
    beginResetModel();
    rootItem->clear();
    endResetModel();
}

/// <summary>
/// Adds the meta data.
/// </summary>
/// <param name="metaData">The meta data.</param>
void DkMetaDataModel::addMetaData(QSharedPointer<DkMetaDataT> metaData)
{
    // metaData->printMetaData();

    if (!metaData)
        return;

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
        QString exifValue = metaData->getNativeExifValue(exifKeys.at(idx), true);
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

    QStringList qtKeys = metaData->getQtKeys();

    for (QString cKey : qtKeys) {
        QString lastKey = cKey.split(".").last();
        QString translatedKey = DkMetaDataHelper::getInstance().translateKey(lastKey);
        QString exifValue = metaData->getQtValue(cKey);
        exifValue = DkMetaDataHelper::getInstance().resolveSpecialValue(metaData, lastKey, exifValue);

        createItem(tr("Data.") + cKey, translatedKey, exifValue);
    }
}

void DkMetaDataModel::createItem(const QString &key, const QString &keyName, const QString &value)
{
    // Split key first
    QStringList keyHierarchy = key.split('.');

    if (keyHierarchy.empty()) {
        qDebug() << "no key hierarchy... skipping: " << key;
        return;
    }

    TreeItem *item = rootItem;

    for (int idx = 0; idx < keyHierarchy.size() - 1; idx++) {
        QString cKey = keyHierarchy.at(idx);
        TreeItem *cHierarchyItem = item->find(cKey, 0);

        if (!cHierarchyItem) {
            QVector<QVariant> keyData;
            keyData << cKey;
            cHierarchyItem = new TreeItem(keyData, item);
            item->appendChild(cHierarchyItem);
        }

        item = cHierarchyItem; // switch to next hierarchy level
    }

    QString cleanValue = DkUtils::cleanFraction(value);

    QVector<QVariant> metaDataEntry;
    metaDataEntry << keyName;

    QDateTime pd = DkUtils::getConvertableDate(cleanValue);

    if (!pd.isNull())
        metaDataEntry << pd;
    else
        metaDataEntry << cleanValue;

    auto *dataItem = new TreeItem(metaDataEntry, item);
    item->appendChild(dataItem);
}

QModelIndex DkMetaDataModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    const TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem *>(parent.internalPointer());

    const TreeItem *childItem = parentItem->child(row);

    // qDebug() << " creating index for: " << childItem->data(0) << " row: " << row;
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex DkMetaDataModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    const auto *childItem = static_cast<TreeItem *>(index.internalPointer());
    const TreeItem *parentItem = childItem->parent();

    if (!parentItem || parentItem == rootItem)
        return QModelIndex();

    // qDebug() << "parent is: " << childItem->data(0);

    return createIndex(parentItem->row(), 0, parentItem);
}

int DkMetaDataModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem *>(parent.internalPointer());

    return parentItem->childCount();
}

int DkMetaDataModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<TreeItem *>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
    // return 2;
}

QVariant DkMetaDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        qDebug() << "invalid row: " << index.row();
        return QVariant();
    }

    // if (index.row() > rowCount())
    //	return QVariant();

    // if (index.column() > columnCount())
    //	return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        auto *item = static_cast<TreeItem *>(index.internalPointer());
        // qDebug() << "returning: " << item->data(0) << "row: " << index.row();

        return item->data(index.column());
    }

    return QVariant();
}

QVariant DkMetaDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    return rootItem->data(section);
}

// bool DkMetaDataModel::setData(const QModelIndex& index, const QVariant& value, int role) {
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
// }
//
Qt::ItemFlags DkMetaDataModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEditable;

    // TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    Qt::ItemFlags flags;

    if (index.column() == 0)
        flags = QAbstractItemModel::flags(index);
    if (index.column() == 1)
        flags = QAbstractItemModel::flags(index) | Qt::ItemIsEditable;

    return flags;
}

// DkMetaDataProxyModel --------------------------------------------------------------------
DkMetaDataProxyModel::DkMetaDataProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool DkMetaDataProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    auto *t = static_cast<TreeItem *>(index.internalPointer());
    if (t) {
        return t->contains(filterRegularExpression(), -1) /* | t->contains(filterRegExp(), 1)*/;
    }

    qWarning() << "[DkMetaDataProxyModel] Ich höre gerade, es ist ein bisschen was durcheinander gekommen";
    return true;
}

// DkMetaDataDock --------------------------------------------------------------------
DkMetaDataDock::DkMetaDataDock(const QString &title, QWidget *parent /* = 0 */, Qt::WindowFlags flags /* = 0 */)
    : DkDockWidget(title, parent, flags)
{
    setObjectName("DkMetaDataDock");

    createLayout();
    readSettings();
}

DkMetaDataDock::~DkMetaDataDock()
{
    // save settings
    writeSettings();
}

void DkMetaDataDock::writeSettings()
{
    DefaultSettings settings;
    settings.beginGroup(objectName());

    for (int idx = 0; idx < mModel->columnCount(QModelIndex()); idx++) {
        QString headerVal = mModel->headerData(idx, Qt::Horizontal).toString();
        settings.setValue(headerVal + "Size", mTreeView->columnWidth(idx));
    }

    settings.setValue("expandedNames", mExpandedNames);
    qDebug() << "settings write expanded names: " << mExpandedNames;

    settings.endGroup();
}

void DkMetaDataDock::readSettings()
{
    DefaultSettings settings;
    settings.beginGroup(objectName());

    for (int idx = 0; idx < mModel->columnCount(QModelIndex()); idx++) {
        QString headerVal = mModel->headerData(idx, Qt::Horizontal).toString();

        int colWidth = settings.value(headerVal + "Size", -1).toInt();
        if (colWidth != -1)
            mTreeView->setColumnWidth(idx, colWidth);
    }
    mExpandedNames = settings.value("expandedNames", QStringList()).toStringList();
    // qDebug() << "settings expanded names: " << mExpandedNames;

    settings.endGroup();
}

void DkMetaDataDock::createLayout()
{
    mFilterEdit = new QLineEdit(this);
    mFilterEdit->setPlaceholderText(tr("Filter"));
    mFilterEdit->setFocusPolicy(Qt::ClickFocus);
    connect(mFilterEdit, &QLineEdit::textChanged, this, &DkMetaDataDock::onFilterTextChanged);

    // create our beautiful shortcut view
    mModel = new DkMetaDataModel(this);

    mProxyModel = new DkMetaDataProxyModel(this);
    mProxyModel->setSourceModel(mModel);

    mTreeView = new QTreeView(this);
    mTreeView->setModel(mProxyModel);
    mTreeView->setAlternatingRowColors(true);
    mTreeView->setFocusPolicy(Qt::ClickFocus);
    // mTreeView->setIndentation(8);
    // mTreeView->setStyleSheet("QTreeView{border: none;}");

    mThumbNailLabel = new QLabel(tr("Thumbnail"), this);
    mThumbNailLabel->hide();

    // thumb layout
    auto *thumbWidget = new QWidget(this);
    auto *thumbLayout = new QHBoxLayout(thumbWidget);
    thumbLayout->setContentsMargins(0, 0, 0, 0);
    thumbLayout->addStretch();
    thumbLayout->addWidget(mThumbNailLabel);
    thumbLayout->addStretch();

    auto *widget = new QWidget(this);
    auto *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->addWidget(mFilterEdit);
    layout->addWidget(mTreeView);
    layout->addWidget(thumbWidget);
    setWidget(widget);
}

void DkMetaDataDock::onFilterTextChanged(const QString &filterText)
{
    if (!filterText.isEmpty())
        mTreeView->expandAll();

    mProxyModel->setFilterRegularExpression(
        QRegularExpression(QRegularExpression::escape(filterText), QRegularExpression::CaseInsensitiveOption));
}

void DkMetaDataDock::updateEntries(QSharedPointer<DkMetaDataT> metadata)
{
    int nr = mProxyModel->rowCount(QModelIndex());
    for (int idx = 0; idx < nr; idx++)
        getExpandedItemNames(mProxyModel->index(idx, 0, QModelIndex()), mExpandedNames);

    mModel->deleteLater();
    mModel = new DkMetaDataModel(this);
    mModel->addMetaData(metadata);
    mProxyModel->setSourceModel(mModel);

    mTreeView->setUpdatesEnabled(false);
    nr = mProxyModel->rowCount();
    for (int idx = 0; idx < nr; idx++)
        expandRows(mProxyModel->index(idx, 0, QModelIndex()), mExpandedNames);

    mTreeView->setUpdatesEnabled(true);

    // for values we should adjust the size at least to the currently visible rows...
    mTreeView->resizeColumnToContents(1);
    // if (treeView->columnWidth(1) > 1000)
    //	treeView->setColumnWidth(1, 1000);
}

void DkMetaDataDock::setImage(QSharedPointer<DkImageContainerT> imgC)
{
    if (!imgC) {
        mProxyModel->setSourceModel(new DkMetaDataModel(this));
        return;
    }

    const auto metadata = imgC->getMetaData();

    if (isVisible())
        updateEntries(metadata);

    // Only load the EXIF thumbnail, so do not use DkThumbLoader.
    // We already have the metadata, no need to read file again,
    // so we can do this in the main thread.
    if (!metadata) {
        mThumbNailLabel->hide();
        return;
    }

    const std::optional<ThumbnailFromMetadata> res = loadThumbnailFromMetadata(*metadata);
    if (!res) {
        mThumbNailLabel->hide();
        return;
    }

    QImage thumbImg = res->thumb;

    const QSize tSize = thumbImg.size();
    const qint64 tSizeBytes = thumbImg.sizeInBytes();
    thumbImg = thumbImg.scaled(tSize.boundedTo(QSize(mTreeView->width(), mTreeView->width())), Qt::KeepAspectRatio);

    mThumbNailLabel->setPixmap(QPixmap::fromImage(thumbImg));

    QString toolTip = tr("Embedded Thumbnail");
    toolTip += QString("\n%1: %2").arg(tr("Size")).arg(DkUtils::readableByte(tSizeBytes));
    toolTip += QString("\n%1: %2x%3").arg(tr("Resolution")).arg(tSize.width()).arg(tSize.height());
    toolTip += QString("\n%1: %2").arg(tr("Transformed")).arg(res->transformed ? tr("yes") : tr("no"));
    mThumbNailLabel->setToolTip(toolTip);

    mThumbNailLabel->show();
}

void DkMetaDataDock::getExpandedItemNames(const QModelIndex &index, QStringList &expandedNames)
{
    if (!mTreeView || !index.isValid())
        return;

    QString entryName = mProxyModel->data(index, Qt::DisplayRole).toString();

    if (mTreeView->isExpanded(index) && !expandedNames.contains(entryName))
        expandedNames.append(entryName);
    else if (!mTreeView->isExpanded(index))
        expandedNames.removeAll(mProxyModel->data(index, Qt::DisplayRole).toString());

    int rows = mProxyModel->rowCount(index);

    for (int idx = 0; idx < rows; idx++)
        getExpandedItemNames(mProxyModel->index(idx, 0, index), expandedNames);
}

void DkMetaDataDock::expandRows(const QModelIndex &index, const QStringList &expandedNames)
{
    if (!index.isValid())
        return;

    if (expandedNames.contains(mProxyModel->data(index).toString())) {
        mTreeView->setExpanded(index, true);
    }

    for (int idx = 0; idx < mProxyModel->rowCount(index); idx++) {
        QModelIndex cIndex = mProxyModel->index(idx, 0, index);

        if (expandedNames.contains(mProxyModel->data(cIndex).toString())) {
            mTreeView->setExpanded(cIndex, true);
            expandRows(cIndex, expandedNames);
        }
    }
}

// void DkMetaDataDock::setVisible(bool visible) {
//
//	if (visible)
//		updateEntries();
//
//	QDockWidget::setVisible(visible);
// }

// DkMetaDataSelection --------------------------------------------------------------------
DkMetaDataSelection::DkMetaDataSelection(const QSharedPointer<DkMetaDataT> metaData, QWidget *parent)
    : DkWidget(parent)
{
    setObjectName("DkMetaDataSelection");
    mMetaData = metaData;
    createLayout();
    selectionChanged();
}

void DkMetaDataSelection::createLayout()
{
    createEntries(mMetaData, mKeys, mValues);

    auto *lWidget = new QWidget(this);
    mLayout = new QGridLayout(lWidget);

    for (int idx = 0; idx < mKeys.size(); idx++) {
        appendGUIEntry(mKeys.at(idx), mValues.at(idx), idx);
    }

    mLayout->setColumnStretch(2, 10);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumSize(QSize(200, 200));
    scrollArea->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidget(lWidget);

    mCbCheckAll = new QCheckBox(tr("Check All"), this);
    mCbCheckAll->setTristate(true);
    connect(mCbCheckAll, &QCheckBox::clicked, this, &DkMetaDataSelection::checkAll);

    auto *l = new QVBoxLayout(this);
    l->addWidget(scrollArea);
    l->addWidget(mCbCheckAll);
}

void DkMetaDataSelection::appendGUIEntry(const QString &key, const QString &value, int idx)
{
    QString cleanKey = key;
    cleanKey = cleanKey.replace(".", " > ");

    auto *cb = new QCheckBox(cleanKey, this);
    connect(cb, &QCheckBox::clicked, this, &DkMetaDataSelection::selectionChanged);
    mSelection.append(cb);

    QString cleanValue = DkUtils::cleanFraction(value);
    QDateTime pd = DkUtils::getConvertableDate(cleanValue);

    if (!pd.isNull())
        cleanValue = pd.toString(Qt::TextDate);

    auto *label = new QLabel(cleanValue, this);
    label->setObjectName("DkMetadataValueLabel");

    if (idx == -1)
        idx = mKeys.size();

    mLayout->addWidget(cb, idx, 1);
    mLayout->addWidget(label, idx, 2);
}

void DkMetaDataSelection::checkAll(bool checked)
{
    for (QCheckBox *cb : mSelection)
        cb->setChecked(checked);
}

void DkMetaDataSelection::selectionChanged()
{
    bool sel = false;
    bool partial = false;
    mCbCheckAll->setTristate(false);

    for (int idx = 0; idx < mSelection.size(); idx++) {
        if (idx > 0 && sel != mSelection.at(idx)->isChecked()) {
            mCbCheckAll->setCheckState(Qt::PartiallyChecked);
            partial = true;
            break;
        }

        sel = mSelection.at(idx)->isChecked();
    }

    if (!partial)
        mCbCheckAll->setChecked(sel);

    qDebug() << "selection changed...";
}

void DkMetaDataSelection::setSelectedKeys(const QStringList &selKeys)
{
    for (QString key : selKeys) {
        int idx = mKeys.indexOf(key);

        if (idx != -1) {
            mSelection.at(idx)->setChecked(true);
        } else {
            // append entries that are not available in the current image
            mKeys.append(key);
            appendGUIEntry(key, "");
            mSelection.last()->setChecked(true);
        }
    }

    selectionChanged();
}

QStringList DkMetaDataSelection::getSelectedKeys() const
{
    QStringList selKeys;

    for (int idx = 0; idx < mSelection.size(); idx++) {
        if (mSelection.at(idx)->isChecked())
            selKeys.append(mKeys.at(idx));
    }

    return selKeys;
}

void DkMetaDataSelection::createEntries(QSharedPointer<DkMetaDataT> metaData,
                                        QStringList &outKeys,
                                        QStringList &outValues) const
{
    if (!metaData)
        return;

    metaData->getFileMetaData(outKeys, outValues);
    metaData->getAllMetaData(outKeys, outValues);
}

// DkMetaDataHUD --------------------------------------------------------------------
DkMetaDataHUD::DkMetaDataHUD(QWidget *parent)
    : DkFadeWidget(parent)
{
    setObjectName("DkMetaDataHUD");
    setCursor(Qt::ArrowCursor);

    // some inits
    mKeyValues = getDefaultKeys();
    loadSettings();

    if (mWindowPosition == pos_west || mWindowPosition == pos_east)
        mOrientation = Qt::Vertical;

    createLayout();
    createActions();
}

DkMetaDataHUD::~DkMetaDataHUD()
{
    // save settings
    saveSettings();
}

void DkMetaDataHUD::createLayout()
{
    auto *titleLabel = new QLabel(tr("Image Information"), this);
    titleLabel->setObjectName("DkMetaDataHUDTitle");

    auto *titleSeparator = new QLabel("", this);
    titleSeparator->setObjectName("DkSeparator");

    mTitleWidget = new QWidget(this);
    auto *titleLayout = new QVBoxLayout(mTitleWidget);
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(titleSeparator);

    QString scrollbarStyle = QString("QScrollBar:vertical {border: 1px solid "
                                     + DkUtils::colorToString(DkSettingsManager::param().display().hudFgdColor)
                                     + "; background: rgba(0,0,0,0); width: 7px; margin: 0 0 0 0;}")
        + QString("QScrollBar::handle:vertical {background: "
                  + DkUtils::colorToString(DkSettingsManager::param().display().hudFgdColor) + "; min-height: 0px;}")
        + QString("QScrollBar::add-line:vertical {height: 0px;}")
        + QString("QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {background: rgba(0,0,0,0); width: "
                  "1px;}")
        + QString("QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {height: 0;}")
        + QString("QScrollBar:horizontal {border: 1px solid "
                  + DkUtils::colorToString(DkSettingsManager::param().display().hudFgdColor)
                  + "; background: rgba(0,0,0,0); height: 7px; margin: 0 0 0 0;}") // horizontal
        + QString("QScrollBar::handle:horizontal {background: "
                  + DkUtils::colorToString(DkSettingsManager::param().display().hudFgdColor) + "; min-width: 0px;}")
        + QString("QScrollBar::add-line:horizontal {width: 0px;}")
        + QString("QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {background: rgba(0,0,0,0); "
                  "height: "
                  "1px;}")
        + QString("QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {width: 0;}");

    mScrollArea = new DkResizableScrollArea(this);
    mScrollArea->setObjectName("DkScrollAreaMetaData");
    mScrollArea->setWidgetResizable(true);
    mScrollArea->setStyleSheet(scrollbarStyle + mScrollArea->styleSheet());
    mScrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    mContentWidget = new QWidget(this);
    mContentWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    mContentLayout = new QGridLayout(mContentWidget);
    updateLabels();

    mScrollArea->setWidget(mContentWidget);

    auto *l = new QVBoxLayout(this);
    l->setSpacing(0);
    l->setContentsMargins(3, 3, 3, 3);
    l->addWidget(mScrollArea);
}

void DkMetaDataHUD::createActions()
{
    mActions.resize(action_end);

    mActions[action_change_keys] = new QAction(tr("Change Entries"), this);
    mActions[action_change_keys]->setStatusTip(tr("You can customize the entries displayed here."));
    connect(mActions[action_change_keys], &QAction::triggered, this, &DkMetaDataHUD::changeKeys);

    mActions[action_num_columns] = new QAction(tr("Number of Columns"), this);
    mActions[action_num_columns]->setStatusTip(tr("Select the desired number of columns."));
    connect(mActions[action_num_columns], &QAction::triggered, this, &DkMetaDataHUD::changeNumColumns);

    mActions[action_set_to_default] = new QAction(tr("Set to Default"), this);
    mActions[action_set_to_default]->setStatusTip(tr("Reset the metadata panel."));
    connect(mActions[action_set_to_default], &QAction::triggered, this, &DkMetaDataHUD::setToDefault);

    // orientations
    mActions[action_pos_west] = new QAction(tr("Show Left"), this);
    mActions[action_pos_west]->setStatusTip(tr("Shows the Metadata on the Left"));
    connect(mActions[action_pos_west], &QAction::triggered, this, &DkMetaDataHUD::newPosition);

    mActions[action_pos_north] = new QAction(tr("Show Top"), this);
    mActions[action_pos_north]->setStatusTip(tr("Shows the Metadata at the Top"));
    connect(mActions[action_pos_north], &QAction::triggered, this, &DkMetaDataHUD::newPosition);

    mActions[action_pos_east] = new QAction(tr("Show Right"), this);
    mActions[action_pos_east]->setStatusTip(tr("Shows the Metadata on the Right"));
    connect(mActions[action_pos_east], &QAction::triggered, this, &DkMetaDataHUD::newPosition);

    mActions[action_pos_south] = new QAction(tr("Show Bottom"), this);
    mActions[action_pos_south]->setStatusTip(tr("Shows the Metadata at the Bottom"));
    connect(mActions[action_pos_south], &QAction::triggered, this, &DkMetaDataHUD::newPosition);
}

void DkMetaDataHUD::loadSettings()
{
    DefaultSettings settings;

    settings.beginGroup(objectName());
    QStringList keyVals = settings.value("keyValues", QStringList()).toStringList();
    mNumColumns = settings.value("numColumns", mNumColumns).toInt();
    mWindowPosition = settings.value("windowPosition", mWindowPosition).toInt();
    settings.endGroup();

    if (!keyVals.isEmpty())
        mKeyValues = keyVals;
}

void DkMetaDataHUD::saveSettings() const
{
    if (mKeyValues.isEmpty())
        return;

    DefaultSettings settings;

    settings.beginGroup(objectName());
    settings.setValue("keyValues", mKeyValues);
    settings.setValue("numColumns", mNumColumns);
    settings.setValue("windowPosition", mWindowPosition);
    settings.endGroup();
}

int DkMetaDataHUD::getWindowPosition() const
{
    return mWindowPosition;
}

QStringList DkMetaDataHUD::getDefaultKeys() const
{
    QStringList keyValues;

    keyValues.append("File." + QObject::tr("Filename"));
    keyValues.append("File." + QObject::tr("Path"));
    keyValues.append("File." + QObject::tr("Size"));
    keyValues.append("Exif.Image.Make");
    keyValues.append("Exif.Image.Model");
    keyValues.append("Exif.Image.DateTime");
    keyValues.append("Exif.Image.ImageDescription");

    keyValues.append("Exif.Photo.UserComment");
    keyValues.append("Exif.Photo.ISO");
    keyValues.append("Exif.Photo.FocalLength");
    keyValues.append("Exif.Photo.ExposureTime");
    keyValues.append("Exif.Photo.Flash");
    keyValues.append("Exif.Photo.FNumber");

    return keyValues;
}

void DkMetaDataHUD::updateMetaData(const QSharedPointer<DkImageContainerT> cImg)
{
    if (cImg) {
        mMetaData = cImg->getMetaData();

        // only update if I am visible
        if (isVisible())
            updateMetaData(mMetaData);
    } else
        mMetaData = QSharedPointer<DkMetaDataT>();
}

void DkMetaDataHUD::updateMetaData(const QSharedPointer<DkMetaDataT> metaData)
{
    // clean up
    for (QLabel *cLabel : mEntryKeyLabels)
        delete cLabel;
    for (QLabel *cLabel : mEntryValueLabels)
        delete cLabel;

    mEntryKeyLabels.clear();
    mEntryValueLabels.clear();

    if (!metaData) {
        // create dummy entries
        for (QString cKey : mKeyValues) {
            mEntryKeyLabels.append(createKeyLabel(cKey));
        }
        return;
    }

    DkTimer dt;

    QStringList fileKeys, fileValues;
    metaData->getFileMetaData(fileKeys, fileValues);

    for (int idx = 0; idx < fileKeys.size(); idx++) {
        QString cKey = fileKeys.at(idx);
        if (mKeyValues.contains(cKey)) {
            mEntryKeyLabels.append(createKeyLabel(cKey));
            mEntryValueLabels.append(createValueLabel(fileValues.at(idx)));
        }
    }

    QStringList exifKeys = metaData->getExifKeys();

    for (int idx = 0; idx < exifKeys.size(); idx++) {
        QString cKey = exifKeys.at(idx);

        if (mKeyValues.contains(cKey)) {
            QString lastKey = cKey.split(".").last();
            QString exifValue = metaData->getNativeExifValue(exifKeys.at(idx), true);
            exifValue = DkMetaDataHelper::getInstance().resolveSpecialValue(metaData, lastKey, exifValue);

            mEntryKeyLabels.append(createKeyLabel(cKey));
            mEntryValueLabels.append(createValueLabel(exifValue));
        }
    }

    QStringList iptcKeys = metaData->getIptcKeys();

    for (int idx = 0; idx < iptcKeys.size(); idx++) {
        QString cKey = iptcKeys.at(idx);

        if (mKeyValues.contains(cKey)) {
            QString lastKey = iptcKeys.at(idx).split(".").last();
            QString exifValue = metaData->getIptcValue(iptcKeys.at(idx));
            exifValue = DkMetaDataHelper::getInstance().resolveSpecialValue(metaData, lastKey, exifValue);

            mEntryKeyLabels.append(createKeyLabel(cKey));
            mEntryValueLabels.append(createValueLabel(exifValue));
        }
    }

    QStringList xmpKeys = metaData->getXmpKeys();

    for (int idx = 0; idx < xmpKeys.size(); idx++) {
        QString cKey = xmpKeys.at(idx);

        if (mKeyValues.contains(cKey)) {
            QString lastKey = xmpKeys.at(idx).split(".").last();
            QString exifValue = metaData->getXmpValue(xmpKeys.at(idx));
            exifValue = DkMetaDataHelper::getInstance().resolveSpecialValue(metaData, lastKey, exifValue);

            mEntryKeyLabels.append(createKeyLabel(cKey));
            mEntryValueLabels.append(createValueLabel(exifValue));
        }
    }

    QStringList qtKeys = metaData->getQtKeys();

    for (int idx = 0; idx < qtKeys.size(); idx++) {
        QString cKey = qtKeys.at(idx);

        if (mKeyValues.contains(cKey)) {
            QString lastKey = cKey.split(".").last();
            QString exifValue = metaData->getQtValue(cKey);
            exifValue = DkMetaDataHelper::getInstance().resolveSpecialValue(metaData, lastKey, exifValue);

            mEntryKeyLabels.append(createKeyLabel(cKey));
            mEntryValueLabels.append(createValueLabel(exifValue));
        }
    }

    updateLabels();
}

void DkMetaDataHUD::updateLabels(int numColumns /* = -1 */)
{
    if (numColumns == -1 && mNumColumns == -1) {
        int numLines = 6;
        numColumns = ((float)mEntryKeyLabels.size() + numLines - 1) / numLines > 2
            ? qRound(((float)mEntryKeyLabels.size() + numLines - 1) / numLines)
            : 2;
    } else if (numColumns == -1) {
        numColumns = mNumColumns;
    }

    if (mOrientation == Qt::Vertical)
        numColumns = 1;

    int cIdx = 0;
    int rIdx = 0;
    int nRows = qCeil((float)(mEntryKeyLabels.size()) / numColumns);

    // stretch between labels
    // we need this for correct context menu handling
    int cS = (mOrientation == Qt::Horizontal) ? 10 : 0;
    mContentLayout->setColumnStretch(cIdx, cS);
    cIdx++;
    mContentLayout->setRowStretch(rIdx, cS);

    mTitleWidget->setVisible(mOrientation == Qt::Vertical);
    if (mOrientation == Qt::Vertical)
        mContentLayout->addWidget(mTitleWidget, 0, 0, 1, 4);

    for (int idx = 0; idx < mEntryKeyLabels.size(); idx++) {
        if (idx && idx % nRows == 0) {
            rIdx = 0;
            cIdx += 3;
            mContentLayout->setColumnStretch(cIdx - 1, cS);
        }

        mContentLayout->addWidget(mEntryKeyLabels.at(idx), rIdx + 1, cIdx, 1, 1, Qt::AlignTop);
        mContentLayout->addWidget(mEntryValueLabels.at(idx), rIdx + 1, cIdx + 1, 1, 1, Qt::AlignTop);
        rIdx++;
    }

    mContentLayout->setColumnStretch(cIdx + 1, cS);
    mContentLayout->setRowStretch(1000, 10); // stretch a reasonably high row (we assume to have less than 1000 entries)

    // remove old columnStretches
    for (int idx = cIdx + 2; idx < 40; idx++)
        mContentLayout->setColumnStretch(idx, 0);

    if (mOrientation == Qt::Vertical) {
        // some scroll area settings need to be adopted to the orientation
        mScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else {
        // some scroll area settings need to be adopted to the orientation
        mScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }

    // TODO: I give now up on this:
    // I do not understand why Qt does not simply resize according to the
    // child widget's constraints if Qt::ScrollBarAlwaysOff is set
    // to me, this would be intended behavior
    // resizing itself is fixed, however, on layout changes it won't
    // decrease it's size
}

QLabel *DkMetaDataHUD::createKeyLabel(const QString &key)
{
    QString labelString = key.split(".").last();
    labelString = DkMetaDataHelper::getInstance().translateKey(labelString);
    auto *keyLabel = new QLabel(labelString, this);
    keyLabel->setObjectName("DkMetaDataKeyLabel");
    keyLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    keyLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    return keyLabel;
}

QLabel *DkMetaDataHUD::createValueLabel(const QString &val)
{
    QString cleanValue = DkUtils::cleanFraction(val);
    QDateTime pd = DkUtils::getConvertableDate(cleanValue);

    if (!pd.isNull())
        cleanValue = pd.toString(Qt::TextDate);

    auto *valLabel = new QLabel(cleanValue.trimmed(), this);
    valLabel->setObjectName("DkMetaDataLabel");
    valLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    valLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    return valLabel;
}

// events
void DkMetaDataHUD::contextMenuEvent(QContextMenuEvent *event)
{
    if (!mContextMenu) {
        mContextMenu = new QMenu(tr("Metadata Menu"), this);
        mContextMenu->addActions(mActions.toList());
    }

    mContextMenu->exec(event->globalPos());
    event->accept();

    // DkFadeWidget::contextMenuEvent(event);
}

// public slots...
void DkMetaDataHUD::setVisible(bool visible, bool saveSetting /* = true */)
{
    DkFadeWidget::setVisible(visible, saveSetting);
    if (mSetWidgetVisible)
        return; // prevent recursion via fade()

    updateMetaData(mMetaData);
}

void DkMetaDataHUD::newPosition()
{
    const auto *sender = static_cast<QAction *>(QObject::sender());
    if (!sender)
        return;

    int pos = 0;
    Qt::Orientation orient = Qt::Horizontal;

    if (sender == mActions[action_pos_west]) {
        pos = pos_west;
        orient = Qt::Vertical;
    } else if (sender == mActions[action_pos_east]) {
        pos = pos_east;
        orient = Qt::Vertical;
    } else if (sender == mActions[action_pos_north]) {
        pos = pos_north;
        orient = Qt::Horizontal;
    } else {
        pos = pos_south;
        orient = Qt::Horizontal;
    }

    mWindowPosition = pos;
    mOrientation = orient;
    emit positionChangeSignal(mWindowPosition);

    updateLabels();
}

void DkMetaDataHUD::changeKeys()
{
    auto *dialog = new QDialog(this);
    dialog->setWindowTitle(tr("Change Metadata Entries"));

    auto *selWidget = new DkMetaDataSelection(mMetaData, this);
    selWidget->setSelectedKeys(mKeyValues);

    // mButtons
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
    connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    auto *layout = new QVBoxLayout(dialog);
    layout->addWidget(selWidget);
    layout->addWidget(buttons);

    int res = dialog->exec();

    if (res == QDialog::Accepted) {
        mKeyValues = selWidget->getSelectedKeys();
        updateMetaData(mMetaData);
    }

    dialog->deleteLater();
}

void DkMetaDataHUD::changeNumColumns()
{
    bool ok;
    int val = QInputDialog::getInt(this,
                                   tr("Number of Columns"),
                                   tr("Number of columns (-1 is default)"),
                                   mNumColumns,
                                   -1,
                                   20,
                                   1,
                                   &ok);

    if (ok) {
        mNumColumns = val;
        updateLabels(mNumColumns);
    }
}

void DkMetaDataHUD::setToDefault()
{
    mNumColumns = -1;
    mKeyValues = getDefaultKeys();
    updateMetaData(mMetaData);
}

// DkCommentTextEdit --------------------------------------------------------------------
DkCommentTextEdit::DkCommentTextEdit(QWidget *parent /* = 0 */)
    : QTextEdit(parent)
{
}

void DkCommentTextEdit::focusOutEvent(QFocusEvent *focusEvent)
{
    emit focusLost();
    QTextEdit::focusOutEvent(focusEvent);
}

void DkCommentTextEdit::paintEvent(QPaintEvent *e)
{
    if (toPlainText().isEmpty() && !viewport()->hasFocus()) {
        QPainter p(viewport());
        p.setOpacity(0.5);
        p.drawText(QRect(QPoint(), viewport()->size()),
                   Qt::AlignHCenter | Qt::AlignVCenter,
                   tr("Click here to add notes"));
    }

    QTextEdit::paintEvent(e);
}

// DkCommentWidget --------------------------------------------------------------------
DkCommentWidget::DkCommentWidget(QWidget *parent /* = 0 */, Qt::WindowFlags /* = 0 */)
    : DkFadeLabel("", parent)
{
    createLayout();
}

void DkCommentWidget::createLayout()
{
    setObjectName("DkCommentWidget");

    auto *titleLabel = new QLabel(tr("NOTES"), this);
    titleLabel->setObjectName("commentTitleLabel");

    // TODO: move to stylesheet.css
    QString scrollbarStyle = QString("QScrollBar:vertical {border: 1px solid "
                                     + DkUtils::colorToString(DkSettingsManager::param().display().hudFgdColor)
                                     + "; background: rgba(0,0,0,0); width: 7px; margin: 0 0 0 0;}")
        + QString("QScrollBar::handle:vertical {background: "
                  + DkUtils::colorToString(DkSettingsManager::param().display().hudFgdColor) + "; min-height: 0px;}")
        + QString("QScrollBar::add-line:vertical {height: 0px;}")
        + QString("QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {background: rgba(0,0,0,0); width: "
                  "1px;}")
        + QString("QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {height: 0;}");

    mCommentLabel = new DkCommentTextEdit(this);
    mCommentLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mCommentLabel->setStyleSheet(scrollbarStyle + mCommentLabel->styleSheet());
    mCommentLabel->setToolTip(tr("Enter your notes here. They will be saved to the image metadata."));
    connect(mCommentLabel, &DkCommentTextEdit::textChanged, this, &DkCommentWidget::onCommentLabelTextChanged);
    connect(mCommentLabel, &DkCommentTextEdit::focusLost, this, &DkCommentWidget::onCommentLabelFocusLost);

    auto *cancelButton = new QPushButton(this);
    cancelButton->setFlat(true);
    cancelButton->setIcon(
        DkImage::loadIcon(":/nomacs/img/trash.svg", QSize(), DkSettingsManager::param().display().hudFgdColor));
    cancelButton->setToolTip(tr("Discard Changes (ESC)"));
    connect(cancelButton, &QPushButton::clicked, this, &DkCommentWidget::onCancelButtonClicked);

    auto *saveButton = new QPushButton(this);
    saveButton->setFlat(true);
    saveButton->setIcon(
        DkImage::loadIcon(":/nomacs/img/save.svg", QSize(), DkSettingsManager::param().display().hudFgdColor));
    saveButton->setToolTip(tr("Save Note (CTRL + ENTER)"));
    connect(saveButton, &QPushButton::clicked, this, &DkCommentWidget::onSaveButtonClicked);

    auto *cancelAction = new QAction(this);
    cancelAction->setShortcut(Qt::Key_Escape);
    connect(cancelAction, &QAction::triggered, cancelButton, &QPushButton::animateClick);

    auto *saveAction = new QAction(this);
    saveAction->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(saveAction, &QAction::triggered, saveButton, &QPushButton::animateClick);

    auto *actionFilter = new DkActionEventFilter(this);
    actionFilter->addAction(cancelAction);
    actionFilter->addAction(saveAction);
    mCommentLabel->installEventFilter(actionFilter);

    auto *titleWidget = new QWidget(this);
    auto *titleLayout = new QHBoxLayout(titleWidget);
    titleLayout->setAlignment(Qt::AlignLeft);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(0);
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(cancelButton, 0, Qt::AlignVCenter);
    titleLayout->addWidget(saveButton, 0, Qt::AlignVCenter);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(titleWidget);
    layout->addWidget(mCommentLabel);

    setLayout(layout);
    setCursor(Qt::ArrowCursor);
}

void DkCommentWidget::setMetaData(QSharedPointer<DkMetaDataT> metaData)
{
    mMetaData = metaData;
    initComment(metaData->getDescription());
}

void DkCommentWidget::initComment(const QString &description)
{
    mOldText = description;
    resetComment();
}

void DkCommentWidget::resetComment()
{
    // First, reset comment text (triggering changed event, but not edited event)
    mOldText = mMetaData->getDescription();
    mCommentLabel->setText(mOldText);
    mCommentLabel->clearFocus();
    // Reset internal state (this panel only)
    mTextEdited = false;
    // Just like in any typical webform, "cancel"/"reset" shouldn't save anything
}

QString DkCommentWidget::text() const
{
    return mCommentLabel->toPlainText();
}

void DkCommentWidget::saveComment()
{
    if (mTextEdited && mCommentLabel->toPlainText() != mMetaData->getDescription() && mMetaData) {
        if (!mMetaData->setDescription(text()) && !text().isEmpty()) {
            emit showInfoSignal(tr("Sorry, I cannot save comments for this image format."));
            return;
        }
        initComment(text());

        emit commentSavedSignal();
    }
}

void DkCommentWidget::onCommentLabelTextChanged()
{
    mTextEdited = text() != mOldText;
}

void DkCommentWidget::onCommentLabelFocusLost()
{
    // We don't want to do anything when changing focus
}

void DkCommentWidget::onSaveButtonClicked()
{
    saveComment();
    mCommentLabel->clearFocus();
}

void DkCommentWidget::onCancelButtonClicked()
{
    resetComment();
}
}
