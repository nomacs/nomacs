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

#pragma once

#include "DkUtils.h"

#pragma warning(push, 0)	// no warnings from includes
#include <QWidget>
#include <QSettings>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#pragma warning(pop)

#ifndef DllGuiExport
#ifdef DK_GUI_DLL_EXPORT
#define DllGuiExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllGuiExport Q_DECL_IMPORT
#else
#define DllGuiExport Q_DECL_IMPORT
#endif
#endif

// Qt defines
class QTreeView;
class QLineEdit;

namespace nmc {

class DkSettingsEntry {

public:
	DkSettingsEntry(const QString& key = QString(), const QVariant& value = QVariant());

	QString key() const;

	void setValue(const QVariant& value);
	QVariant value() const;

	static DkSettingsEntry fromSettings(const QString& key, const QSettings& settings);

protected:
	QString mKey;
	QVariant mValue;
};

class DkSettingsGroup {

public:
	DkSettingsGroup(const QString& name = QString());

	bool isEmpty() const;

	QString name() const;
	int size() const;
	QVector<DkSettingsEntry> entries() const;

	static DkSettingsGroup fromSettings(const QString& groupName, QSettings& settings);

protected:
	QString mGroupName;
	QVector<DkSettingsEntry> mEntries;
};

class DkSettingsProxyModel : public QSortFilterProxyModel {
	Q_OBJECT

public:
	DkSettingsProxyModel(QObject * parent = 0);
	virtual ~DkSettingsProxyModel() {}

protected:
	virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

};

class DkSettingsModel : public QAbstractItemModel {
	Q_OBJECT

public:
	DkSettingsModel(QObject* parent = 0);
	~DkSettingsModel();

	QModelIndex index(int row, int column, const QModelIndex &parent) const;
	QModelIndex parent(const QModelIndex &index) const;

	// return item of the model
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	virtual Qt::ItemFlags flags(const QModelIndex& index) const;
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

	void addSettingsGroup(const DkSettingsGroup& group, const QString& parentName = "");

	//void resetSettings();
	//void saveSettings() const;

protected:
	TreeItem* mRootItem;
	//QVector<DkSettingsGroup> mGroups;

};

// nomacs defines
class DllGuiExport DkSettingsWidget : public QWidget {
	Q_OBJECT

public:
	DkSettingsWidget(QWidget* parent);

	void setSettings(QSettings& settings, const QString& parentName = "");

public slots:
	void focusFilter();
	void on_Filter_textChanged(const QString& text);

protected:
	void createLayout();

	DkSettingsModel* mSettingsModel;
	DkSettingsProxyModel* mProxyModel;
	QLineEdit* mSettingsFilter;
	QTreeView* mTreeView;
};


}