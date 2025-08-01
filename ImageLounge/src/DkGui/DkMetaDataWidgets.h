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

#include <QSortFilterProxyModel>
#include <QTextEdit>

#include "DkBaseWidgets.h"
#include "DkImageContainer.h"

class QTreeView;
class QLabel;
class QPushButton;
class QGridLayout;
class QCheckBox;
class QVBoxLayout;
class QLineEdit;

namespace nmc
{
class TreeItem;

class DkMetaDataModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit DkMetaDataModel(QObject *parent = nullptr);
    ~DkMetaDataModel() override;

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    // return item of the model
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    // virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

    virtual void addMetaData(QSharedPointer<DkMetaDataT> metaData);
    void clear();

protected:
    TreeItem *rootItem;

    void createItem(const QString &key, const QString &keyName, const QString &value);
};

class DkMetaDataProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit DkMetaDataProxyModel(QObject *parent = nullptr);
    ~DkMetaDataProxyModel() override = default;

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

class DkMetaDataDock : public DkDockWidget
{
    Q_OBJECT

public:
    explicit DkMetaDataDock(const QString &title, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~DkMetaDataDock() override;

public slots:
    void setImage(QSharedPointer<DkImageContainerT> imgC);
    void onFilterTextChanged(const QString &filterText);

protected:
    void createLayout();
    void updateEntries(QSharedPointer<DkMetaDataT> metadata);
    void writeSettings();
    void readSettings();

    void getExpandedItemNames(const QModelIndex &index, QStringList &expandedNames);
    void expandRows(const QModelIndex &index, const QStringList &expandedNames);

    QTreeView *mTreeView = nullptr;
    DkMetaDataProxyModel *mProxyModel = nullptr;
    QLineEdit *mFilterEdit = nullptr;
    DkMetaDataModel *mModel = nullptr;
    QLabel *mThumbNailLabel = nullptr;
    QStringList mExpandedNames;
};

class DkMetaDataSelection : public DkWidget
{
    Q_OBJECT

public:
    explicit DkMetaDataSelection(const QSharedPointer<DkMetaDataT> metaData, QWidget *parent = nullptr);

    void setSelectedKeys(const QStringList &selKeys);
    QStringList getSelectedKeys() const;

public slots:
    void checkAll(bool checked);
    void selectionChanged();

protected:
    void createLayout();
    void createEntries(QSharedPointer<DkMetaDataT> metaData, QStringList &outKeys, QStringList &outValues) const;
    void appendGUIEntry(const QString &key, const QString &value, int idx = -1);

    QSharedPointer<DkMetaDataT> mMetaData;

    QStringList mSelectedKeys;
    QStringList mKeys;
    QStringList mValues;

    QVector<QCheckBox *> mSelection;
    QCheckBox *mCbCheckAll;
    QGridLayout *mLayout;
};

class DkMetaDataHUD : public DkFadeWidget
{
    Q_OBJECT

public:
    explicit DkMetaDataHUD(QWidget *parent = nullptr);
    ~DkMetaDataHUD() override;

    void updateLabels(int numColumns = -1);

    int getWindowPosition() const;

    enum {
        action_change_keys,
        action_num_columns,
        action_set_to_default,

        action_pos_west,
        action_pos_north,
        action_pos_east,
        action_pos_south,

        action_end,
    };

public slots:
    void updateMetaData(const QSharedPointer<DkImageContainerT> cImg = QSharedPointer<DkImageContainerT>());
    void updateMetaData(const QSharedPointer<DkMetaDataT> cImg);
    void changeKeys();
    void changeNumColumns();
    void setToDefault();
    void newPosition();
    void setVisible(bool visible, bool saveSetting = true) override;

signals:
    void positionChangeSignal(int newPos) const;

protected:
    void createLayout();
    void createActions();
    void loadSettings();
    void saveSettings() const;
    QStringList getDefaultKeys() const;
    QLabel *createKeyLabel(const QString &key);
    QLabel *createValueLabel(const QString &val);

    void contextMenuEvent(QContextMenuEvent *event) override;

    // current metadata
    QSharedPointer<DkMetaDataT> mMetaData;
    QStringList mKeyValues;

    // gui elements
    QVector<QLabel *> mEntryKeyLabels;
    QVector<QLabel *> mEntryValueLabels;
    QGridLayout *mContentLayout = nullptr;
    QWidget *mContentWidget = nullptr;
    DkResizableScrollArea *mScrollArea = nullptr;
    QWidget *mTitleWidget = nullptr;

    QMenu *mContextMenu = nullptr;
    QVector<QAction *> mActions;

    int mNumColumns = -1;
    int mWindowPosition = pos_south;
    Qt::Orientation mOrientation = Qt::Horizontal;
};

class DkCommentTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    explicit DkCommentTextEdit(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *e) override;
};

class DkCommentWidget : public DkFadeLabel
{
    Q_OBJECT

public:
    explicit DkCommentWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~DkCommentWidget() override = default;

    void setText(const QString &comment);

public slots:
    void onCommentLabelTextChanged();
    void onSaveButtonClicked();
    void onCancelButtonClicked();

signals:
    void commentSavedSignal(const QString &comment) const;

private:
    void createLayout();
    void resetComment();

    DkCommentTextEdit *mCommentLabel = nullptr;
    QString mOldText;
};
}
