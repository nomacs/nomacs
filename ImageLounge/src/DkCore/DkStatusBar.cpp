/*******************************************************************************************************
 DkStatusBar.cpp
 Created on:	12.01.2016

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

 *******************************************************************************************************/

#include "DkStatusBar.h"

#include "DkActionManager.h"
#include "DkSettings.h"

#pragma warning(push, 0) // no warnings from includes - begin
#include <QLabel>
#pragma warning(pop) // no warnings from includes - end

namespace nmc
{

DkStatusBar::DkStatusBar(QWidget *parent)
    : QStatusBar(parent)
{
    createLayout();
}

void DkStatusBar::createLayout()
{
    mLabels.resize(status_end);
    setObjectName("DkStatusBar");

    for (int idx = 0; idx < mLabels.size(); idx++) {
        mLabels[idx] = new QLabel(this);
        mLabels[idx]->setObjectName("statusBarLabel");
        mLabels[idx]->hide();

        if (idx == 0) {
            // mLabels[idx]->setToolTip(tr("CTRL activates the crosshair cursor"));
            addWidget(mLabels[idx]);
        } else
            addPermanentWidget(mLabels[idx]);
    }

    hide();
}

void DkStatusBar::setMessage(const QString &msg, StatusLabel which)
{
    if (which < 0 || which >= mLabels.size())
        return;

    mLabels[which]->setVisible(!msg.isEmpty());
    mLabels[which]->setText(msg);
}

// DkStatusBarManager --------------------------------------------------------------------
DkStatusBarManager::DkStatusBarManager()
{
    mStatusBar = new DkStatusBar();
}

DkStatusBarManager &DkStatusBarManager::instance()
{
    static DkStatusBarManager inst;
    return inst;
}

void DkStatusBarManager::setMessage(const QString &msg, DkStatusBar::StatusLabel which)
{
    mStatusBar->setMessage(msg, which);
}

void DkStatusBarManager::show(bool show, bool permanent)
{
    if (statusbar()->isVisible() == show)
        return;

    if (permanent)
        DkSettingsManager::param().app().showStatusBar = show;
    DkActionManager::instance().action(DkActionManager::menu_panel_statusbar)->setChecked(DkSettingsManager::param().app().showStatusBar);

    statusbar()->setVisible(show);
}

DkStatusBar *DkStatusBarManager::statusbar()
{
    return mStatusBar;
}

}
