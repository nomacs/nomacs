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

#include "DkDockWidgets.h"

#include "DkBasicLoader.h"
#include "DkSettings.h"

#pragma warning(push, 0) // no warnings from includes
#include <QListWidget>
#include <QVBoxLayout>
#pragma warning(pop)

namespace nmc
{

DkHistoryDock::DkHistoryDock(const QString &title, QWidget *parent)
    : DkDockWidget(title, parent)
{
    setObjectName("DkHistoryDock");
    createLayout();
    QMetaObject::connectSlotsByName(this);
}

void DkHistoryDock::createLayout()
{
    mHistoryList = new QListWidget(this);
    mHistoryList->setObjectName("historyList");
    mHistoryList->setIconSize(QSize(DkSettingsManager::param().effectiveIconSize(), DkSettingsManager::param().effectiveIconSize()));

    QWidget *contentWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(contentWidget);
    layout->addWidget(mHistoryList);

    setWidget(contentWidget);
}

void DkHistoryDock::updateImage(QSharedPointer<DkImageContainerT> img)
{
    updateList(img);
    mImg = img;
}

void DkHistoryDock::updateList(QSharedPointer<DkImageContainerT> img)
{
    QVector<DkEditImage> *history = img->getLoader()->history();
    int hIdx = img->getLoader()->historyIndex();
    QVector<QListWidgetItem *> editItems;

    mHistoryList->clear();

    for (int idx = 0; idx < history->size(); idx++) {
        const DkEditImage &eImg = history->at(idx);
        QListWidgetItem *item = new QListWidgetItem(QIcon(":/nomacs/img/nomacs.svg"), eImg.editName());
        item->setFlags(idx <= hIdx ? Qt::ItemIsEnabled : Qt::NoItemFlags);

        mHistoryList->addItem(item);
    }

    if (mHistoryList->item(hIdx))
        mHistoryList->item(hIdx)->setSelected(true);
}

void DkHistoryDock::on_historyList_itemClicked(QListWidgetItem *item)
{
    if (!mImg)
        return;

    for (int idx = 0; idx < mHistoryList->count(); idx++) {
        if (item == mHistoryList->item(idx)) {
            mImg->setHistoryIndex(idx);
            break;
        }
    }
}

}
