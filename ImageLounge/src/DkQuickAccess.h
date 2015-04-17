/*******************************************************************************************************
 DkQuickAccess.h
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

#pragma once

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QObject>
#include <QFileInfo>
#include <QVector>
#include <QStringList>
#pragma warning(pop)		// no warnings from includes - end

class QAction;
class QStandardItemModel;
class QIcon;
class QModelIndex;

namespace nmc {

class DkQuickAccess : public QObject {
	Q_OBJECT

public:
	DkQuickAccess(QObject* parent = 0);

	void addActions(const QVector<QAction*>& actions);
	void addDirs(const QStringList& dirPaths);
	void addFiles(const QStringList& filePaths);
	void addItems(const QStringList& itemTexts, const QIcon& icon);

	QStandardItemModel* getModel() const { return model; };
	
public slots:
	void fireAction(const QModelIndex& index) const;

signals:
	void loadFileSignal(const QFileInfo& fileInfo) const;

protected:
	QStandardItemModel* model;

	QVector<QAction*> actions;
	QStringList filePaths;
};


//// nomacs defines
//class TreeItem;
//
//class DkQuickAcessModel : public QAbstractItemModel {
//	Q_OBJECT
//
//public:
//	DkQuickAcessModel(QObject* parent = 0);
//	~DkQuickAcessModel();
//
//	QModelIndex index(int row, int column, const QModelIndex &parent) const;
//	QModelIndex parent(const QModelIndex &index) const;
//
//	// return item of the model
//	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
//	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
//	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
//	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
//
//	virtual Qt::ItemFlags flags(const QModelIndex& index) const;
//	//virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
//
//	virtual void addActions(const QVector<QAction*>& actions);
//	virtual void addPaths(const QStringList& filePaths);
//	void clear();
//
//protected:
//	TreeItem* rootItem;
//
//	void createItem(const QString& key, const QVariant& value);
//};




}