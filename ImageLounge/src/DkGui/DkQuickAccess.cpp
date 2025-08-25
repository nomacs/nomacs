/*******************************************************************************************************
 DkQuickAccess.cpp
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

#include "DkQuickAccess.h"

#include "DkActionManager.h"
#include "DkFileInfo.h"
#include "DkImageStorage.h"
#include "DkUtils.h"

#include <QAction>
#include <QCompleter>
#include <QKeyEvent>
#include <QStandardItemModel>

namespace nmc
{

// DkQuickFilterCompleter --------------------------------------------------------------------
DkQuickAccess::DkQuickAccess(QObject *parent /* = 0 */)
    : QObject(parent)
{
    mModel = new QStandardItemModel(this);
    mModel->setColumnCount(1);
}

void DkQuickAccess::addActions(const QVector<QAction *> &actions)
{
    // initialize the model
    int nRows = mModel->rowCount();
    mModel->setRowCount(nRows + actions.size());

    for (int rIdx = 0; rIdx < actions.size(); rIdx++) {
        if (!actions[rIdx]) {
            qDebug() << "WARNING: empty action detected...";
            continue;
        }

        QAction *a = actions[rIdx];
        QIcon icon = a->icon().isNull() ? DkImage::loadIcon(":/nomacs/img/dummy.svg") : a->icon();

        QString text = a->text().remove("&");
        auto *item = new QStandardItem(text);
        item->setSizeHint(QSize(18, 18));
        item->setIcon(icon);
        item->setToolTip(a->toolTip());
        mModel->setItem(nRows + rIdx, 0, item);
    }

    // we assume they are unique
    mActions << actions;
}

void DkQuickAccess::addFiles(const QStringList &filePaths)
{
    addItems(filePaths, DkImage::loadIcon(":/nomacs/img/nomacs-bg.svg"));
}

void DkQuickAccess::addDirs(const QStringList &dirPaths)
{
    addItems(dirPaths, DkImage::loadIcon(":/nomacs/img/dir.svg"));
}

void DkQuickAccess::addItems(const QStringList &itemTexts, const QIcon &icon)
{
    int nRows = mModel->rowCount();
    mModel->setRowCount(nRows + itemTexts.size());

    for (int rIdx = 0; rIdx < itemTexts.size(); rIdx++) {
        QString text = itemTexts.at(rIdx);

        if (mFilePaths.contains(text))
            continue;

        auto *item = new QStandardItem(text);
        item->setIcon(icon);
        item->setSizeHint(QSize(18, 18));
        mModel->setItem(nRows + rIdx, 0, item);
        mFilePaths.append(text);
    }
}

bool DkQuickAccess::execute(const QString &cmd) const
{
    qDebug() << "executing" << cmd;

    // use tryExists because file paths come from recent files history, which
    // could contain disconnected network shares
    if (DkUtils::tryExists(DkFileInfo(cmd))) {
        emit loadFileSignal(cmd);
        return true;
    }

    for (QAction *a : mActions) {
        QString aKey = a->text().remove("&");

        if (aKey == cmd) {
            if (a->isEnabled())
                a->trigger();
            return true;
        }
    }

    return false;
}

// DkQuickAcessEdit --------------------------------------------------------------------
DkQuickAccessEdit::DkQuickAccessEdit(QWidget *parent)
    : QLineEdit("", parent)
{
    QString shortcutText = DkActionManager::instance()
                               .action(DkActionManager::menu_file_quick_launch)
                               ->shortcut()
                               .toString();
    setPlaceholderText(tr("Quick Launch (%1)").arg(shortcutText));
    setMinimumWidth(150);
    setMaximumWidth(350);
    hide();

    mCompleter = new QCompleter(this);

    mCompleter->setFilterMode(Qt::MatchContains);

    mCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    setCompleter(mCompleter);

    connect(this, &DkQuickAccessEdit::returnPressed, this, &DkQuickAccessEdit::editConfirmed);
}

void DkQuickAccessEdit::setModel(QStandardItemModel *model)
{
    mCompleter->setModel(model);
    clear();
    show();

    setFocus(Qt::MouseFocusReason);
}

void DkQuickAccessEdit::editConfirmed()
{
    QString fp = text();

    // check if we can directly load what is there (this is nice to load the parent dir of a recent dir)
    if (DkUtils::tryExists(DkFileInfo(fp)))
        emit executeSignal(text());
    else if (!mCompleter->currentCompletion().isNull())
        emit executeSignal(mCompleter->currentCompletion());
    else
        // do something usefull if our completer has no idea anymore...
        emit executeSignal(text());

    clearAccess();
}

void DkQuickAccessEdit::focusOutEvent(QFocusEvent *ev)
{
    QLineEdit::focusOutEvent(ev);
    clearAccess();
}

void DkQuickAccessEdit::keyReleaseEvent(QKeyEvent *ev)
{
    QLineEdit::keyReleaseEvent(ev);

    if (ev->key() == Qt::Key_Escape) {
        clearAccess();
    }
}

void DkQuickAccessEdit::clearAccess()
{
    clear();
    hide();
}

}
