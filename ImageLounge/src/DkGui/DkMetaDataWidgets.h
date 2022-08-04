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

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QTextEdit>
#include <QAbstractTableModel>
#include <QDockWidget>
#include <QSortFilterProxyModel>
#pragma warning(pop)		// no warnings from includes - end

#include "DkBaseWidgets.h"
#include "DkImageContainer.h"

// Qt defines
class QTreeView;
class QLabel;
class QPushButton;
class QGridLayout;
class QCheckBox;
class QVBoxLayout;
class QLineEdit;

namespace nmc {

// nomacs defines
class TreeItem;

class DkMetaDataModel : public QAbstractItemModel {
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

	virtual Qt::ItemFlags flags(const QModelIndex& index) const;
	//virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

	virtual void addMetaData(QSharedPointer<DkMetaDataT> metaData);
	void clear();

protected:
	TreeItem* rootItem;

	void createItem(const QString& key, const QString& keyName, const QString& value);
};

class DkMetaDataProxyModel : public QSortFilterProxyModel {
	Q_OBJECT

public:
	DkMetaDataProxyModel(QObject * parent = 0);
	virtual ~DkMetaDataProxyModel() {}

protected:
	virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

};

class DkMetaDataDock : public DkDockWidget {
	Q_OBJECT

public:
	DkMetaDataDock(const QString& title, QWidget* parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
	~DkMetaDataDock();

public slots:
	void setImage(QSharedPointer<DkImageContainerT> imgC);
	void thumbLoaded(bool loaded);
	void on_filter_textChanged(const QString& filterText);

protected:
	void createLayout();
	void updateEntries();
	void writeSettings();
	void readSettings();

	void getExpandedItemNames(const QModelIndex& index, QStringList& expandedNames);
	void expandRows(const QModelIndex& index, const QStringList& expandedNames);

	QSharedPointer<DkImageContainerT> mImgC;
	QTreeView* mTreeView = 0;
	DkMetaDataProxyModel* mProxyModel = 0;
	QLineEdit* mFilterEdit = 0;
	DkMetaDataModel* mModel = 0;
	QLabel* mThumbNailLabel = 0;
	QSharedPointer<DkThumbNailT> mThumb;
	QStringList mExpandedNames;
};

class DkMetaDataSelection : public DkWidget {
	Q_OBJECT

public:
	DkMetaDataSelection(const QSharedPointer<DkMetaDataT> metaData, QWidget* parent = 0);

	void setSelectedKeys(const QStringList& selKeys);
	QStringList getSelectedKeys() const;

public slots:
	void checkAll(bool checked);
	void selectionChanged();

protected:
	void createLayout();
	void createEntries(QSharedPointer<DkMetaDataT> metaData, QStringList& outKeys, QStringList& outValues) const;
	void appendGUIEntry(const QString& key, const QString& value, int idx = -1);

	QSharedPointer<DkMetaDataT> mMetaData;

	QStringList mSelectedKeys;
	QStringList mKeys;
	QStringList mValues;

	QVector<QCheckBox*> mSelection;
	QCheckBox* mCbCheckAll;
	QGridLayout* mLayout;
};

class DkMetaDataHUD : public DkFadeWidget {
	Q_OBJECT

public:
	DkMetaDataHUD(QWidget* parent = 0);
	~DkMetaDataHUD();

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
	virtual void setVisible(bool visible, bool saveSetting = true) override;

signals:
	void positionChangeSignal(int newPos) const;

protected:
	void createLayout();
	void createActions();
	void loadSettings();
	void saveSettings() const;
	QStringList getDefaultKeys() const;
	QLabel* createKeyLabel(const QString& key);
	QLabel* createValueLabel(const QString& val);

	void contextMenuEvent(QContextMenuEvent *event) override;

	// current metadata
	QSharedPointer<DkMetaDataT> mMetaData;
	QStringList mKeyValues;

	// gui elements
	QVector<QLabel*> mEntryKeyLabels;
	QVector<QLabel*> mEntryValueLabels;
	QGridLayout* mContentLayout = 0;
	QWidget* mContentWidget = 0;
	DkResizableScrollArea* mScrollArea = 0;
	QWidget* mTitleWidget = 0;

	QMenu* mContextMenu = 0;
	QVector<QAction*> mActions;

	int mNumColumns = -1;
	int mWindowPosition = pos_south;
	Qt::Orientation mOrientation = Qt::Horizontal;
};

class DkCommentTextEdit : public QTextEdit {
	Q_OBJECT

public:
	DkCommentTextEdit(QWidget* parent = 0);

signals:
	void focusLost() const;

protected:
	void focusOutEvent(QFocusEvent* focusEvent) override;
	void paintEvent(QPaintEvent* e) override;
};

class DkCommentWidget : public DkFadeLabel {
	Q_OBJECT

public:
	DkCommentWidget(QWidget* parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
	~DkCommentWidget() {};
	
	void setMetaData(QSharedPointer<DkMetaDataT> metaData);
	QString text() const;

public slots:
	void on_CommentLabel_textChanged();
	void on_CommentLabel_focusLost();
	void on_saveButton_clicked();
	void on_cancelButton_clicked();

	void initComment(const QString& description);
	void resetComment();
	void saveComment();

signals:
	void showInfoSignal(const QString& msg) const;
	void commentEditedSignal() const;
	void commentSavedSignal() const;
	void commentSavedSignal(const QString&) const;

protected:
	void createLayout();

	DkCommentTextEdit* mCommentLabel;
	QSharedPointer<DkMetaDataT> mMetaData;
	bool mTextEdited = false;
	QString mOldText;
};

}
