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

#pragma once

#include "DkBaseWidgets.h"
#include "DkUtils.h"

#pragma warning(push, 0) // no warnings from includes
#include <QAbstractItemModel>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QWidget>
#pragma warning(pop)

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

// Qt defines
class QTreeView;
class QLineEdit;

namespace nmc
{

class DllCoreExport DkSettingsEntry
{
public:
    DkSettingsEntry(const QString &key = QString(), const QVariant &value = QVariant());

    QString key() const;

    void setValue(const QVariant &value);
    QVariant value() const;

    static DkSettingsEntry fromSettings(const QString &key, const QSettings &settings);

protected:
    QString mKey;
    QVariant mValue;
};

class DllCoreExport DkSettingsGroup
{
public:
    DkSettingsGroup(const QString &name = QString());

    bool isEmpty() const;

    QString name() const;
    int size() const;
    QVector<DkSettingsEntry> entries() const;
    QVector<DkSettingsGroup> children() const;

    void addChild(const DkSettingsGroup &group);

    static DkSettingsGroup fromSettings(QSettings &settings, const QString &groupName = "");

protected:
    QString mGroupName;
    QVector<DkSettingsEntry> mEntries;
    QVector<DkSettingsGroup> mChildren;
};

class DkSettingsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    DkSettingsProxyModel(QObject *parent = 0);
    virtual ~DkSettingsProxyModel()
    {
    }

protected:
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

class DkSettingsModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    DkSettingsModel(QObject *parent = 0);
    ~DkSettingsModel();

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    // return item of the model
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // edit functions
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    void addSettingsGroup(const DkSettingsGroup &group, const QString &parentName = "");
    void clear();

signals:
    void settingChanged(const QString &key, const QVariant &value, const QStringList &groups) const;
    void settingRemoved(const QString &key, const QStringList &groups) const;

protected:
    TreeItem *mRootItem;
};

// nomacs defines
class DllCoreExport DkSettingsWidget : public DkWidget
{
    Q_OBJECT

public:
    DkSettingsWidget(QWidget *parent);

    void setSettingsPath(const QString &settings, const QString &parentName = "");
    void addSettingsGroup(const DkSettingsGroup &group);
    void clear();
    void filter(const QString &filterText);
    void expandAll();

    static void changeSetting(QSettings &settings,
                              const QString &key,
                              const QVariant &value,
                              const QStringList &groups);
    static void removeSetting(QSettings &settings, const QString &key, const QStringList &groups);

signals:
    void changeSettingSignal(const QString &key, const QVariant &value, const QStringList &groups);
    void removeSettingSignal(const QString &key, const QStringList &groups);

public slots:
    void onFilterTextChanged(const QString &text);
    void onSettingsModelSettingChanged(const QString &key, const QVariant &value, const QStringList &groups);
    void onSettingsModelSettingRemoved(const QString &key, const QStringList &groups);
    void onRemoveRowsTriggered();

protected:
    void createLayout();

    DkSettingsModel *mSettingsModel;
    DkSettingsProxyModel *mProxyModel;
    QLineEdit *mSettingsFilter;
    QTreeView *mTreeView;
};

}
