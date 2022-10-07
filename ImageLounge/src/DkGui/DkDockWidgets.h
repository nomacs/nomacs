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
#include "DkImageContainer.h"

#pragma warning(push, 0) // no warnings from includes
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
class QListWidget;
class QListWidgetItem;

namespace nmc
{

// nomacs defines
class DllCoreExport DkHistoryDock : public DkDockWidget
{
    Q_OBJECT

public:
    DkHistoryDock(const QString &title = "", QWidget *parent = 0);

public slots:
    void updateImage(QSharedPointer<DkImageContainerT> img);
    void on_historyList_itemClicked(QListWidgetItem *item);

protected:
    void createLayout();
    void updateList(QSharedPointer<DkImageContainerT> img);

    QSharedPointer<DkImageContainerT> mImg;
    QListWidget *mHistoryList;
};

}