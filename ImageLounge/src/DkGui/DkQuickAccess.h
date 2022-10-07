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

#pragma warning(push, 0) // no warnings from includes - begin
#include <QFileInfo>
#include <QLineEdit>
#include <QObject>
#include <QStringList>
#include <QVector>
#pragma warning(pop) // no warnings from includes - end

class QAction;
class QStandardItemModel;
class QIcon;
class QModelIndex;

namespace nmc
{

class DkQuickAccess : public QObject
{
    Q_OBJECT

public:
    DkQuickAccess(QObject *parent = 0);

    void addActions(const QVector<QAction *> &actions);
    void addDirs(const QStringList &dirPaths);
    void addFiles(const QStringList &filePaths);
    void addItems(const QStringList &itemTexts, const QIcon &icon);

    QStandardItemModel *getModel() const
    {
        return mModel;
    };

public slots:
    bool execute(const QString &cmd) const;

signals:
    void loadFileSignal(const QString &filePath) const;

protected:
    QStandardItemModel *mModel = 0;

    QVector<QAction *> mActions;
    QStringList mFilePaths;
};

class DkQuickAccessEdit : public QLineEdit
{
    Q_OBJECT

public:
    DkQuickAccessEdit(QWidget *parent = 0);

    void setModel(QStandardItemModel *model);

signals:
    void executeSignal(const QString &cmd) const;

public slots:
    void clearAccess();
    void editConfirmed();

protected:
    void focusOutEvent(QFocusEvent *ev) override;
    void keyReleaseEvent(QKeyEvent *ev) override;

    QCompleter *mCompleter;
};

}