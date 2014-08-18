/*******************************************************************************************************
 DkMetaDataWidgets.h
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

#pragma once

#include <QDockWidget>
#include <QTreeView>
#include <QLabel>

#include "DkMetaData.h"
#include "DkImageContainer.h"
#include "DkThumbs.h"

namespace nmc {

class DkMetaDataModel : public QAbstractTableModel {
	Q_OBJECT

public:
	DkMetaDataModel(QObject* parent = 0);
	~DkMetaDataModel();

	QModelIndex index(int row, int column, const QModelIndex &parent) const;
	QModelIndex parent(const QModelIndex &index) const;

	// return item of the model
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	//virtual Qt::ItemFlags flags(const QModelIndex& index) const;
	//virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

	virtual void addMetaData(QSharedPointer<DkMetaDataT> metaData);
	void clear();

protected:
	TreeItem* rootItem;

	void createItem(const QString& key, const QString& keyName, const QString& value);

};

class DkMetaDataDock : public QDockWidget {
	Q_OBJECT

public:
	DkMetaDataDock(const QString& title, QWidget* parent = 0, Qt::WindowFlags flags = 0 );
	~DkMetaDataDock();

public slots:
	void setImage(QSharedPointer<DkImageContainerT> imgC);
	void thumbLoaded(bool loaded);
	//virtual void setVisible(bool visible);

protected:
	void createLayout();
	void updateEntries();
	void writeSettings();
	void readSettings();

	void getExpandedItemNames(const QModelIndex& index, QStringList& expandedNames);
	void expandRows(const QModelIndex& index, const QStringList& expandedNames);

	QSharedPointer<DkImageContainerT> imgC;
	QTreeView* treeView;
	DkMetaDataModel* model;
	QLabel* thumbNailLabel;
	QSharedPointer<DkThumbNailT> thumb;
};

};
