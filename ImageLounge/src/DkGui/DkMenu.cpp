/*******************************************************************************************************
 DkMenu.cpp
 Created on:	09.08.2011

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2013 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2013 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2013 Florian Kleber <florian@nomacs.org>

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

#include "DkMenu.h"
#include "DkNetwork.h"
#include "DkSettings.h"

#include <QAction>
#include <QDebug>
#include <QFileInfo>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QStringBuilder>
#include <QTimer>

namespace nmc
{

// DkMenu --------------------------------------------------------------------
DkMenuBar::DkMenuBar(QWidget *parent, int timeToShow)
    : QMenuBar(parent)
{
    mTimeToShow = timeToShow; // default: 5 seconds

    mTimerMenu = new QTimer(this);
    mTimerMenu->setSingleShot(true);
    connect(mTimerMenu, &QTimer::timeout, this, &DkMenuBar::hideMenu);

    // uncomment if you want to show menu on start-up
    // if (timeToShow != -1)
    //	timerMenu->start(timeToShow);
}

QAction *DkMenuBar::addMenu(QMenu *menu)
{
    mMenus.append(menu);

    return QMenuBar::addMenu(menu);
}

QMenu *DkMenuBar::addMenu(const QString &title)
{
    QMenu *newMenu = QMenuBar::addMenu(title);
    mMenus.append(newMenu);

    return newMenu;
}

QMenu *DkMenuBar::addMenu(const QIcon &icon, const QString &title)
{
    QMenu *newMenu = QMenuBar::addMenu(icon, title);
    mMenus.append(newMenu);

    return newMenu;
}

void DkMenuBar::showMenu()
{
    // if (mTimeToShow == -1)
    //	return;

    if (isVisible()) {
        mTimerMenu->stop();
        hideMenu();
        return;
    }

    if (mTimeToShow != -1)
        mTimerMenu->start(mTimeToShow);

    show();
}

void DkMenuBar::hideMenu()
{
    if (mTimeToShow == -1)
        return;

    // ok we have a mouseover
    if (mActive)
        return;

    for (int idx = 0; idx < mMenus.size(); idx++) {
        // ok, a child is active -> wait for it
        if (mMenus.at(idx)->isVisible()) {
            mTimerMenu->start(mTimeToShow);
            return;
        }
    }

    if (!mActive)
        hide();
}

void DkMenuBar::setTimeToShow(int timeToShow)
{
    mTimeToShow = timeToShow;
}

void DkMenuBar::enterEvent(DkEnterEvent *event)
{
    if (mTimeToShow == -1)
        return;

    mActive = true;

    QMenuBar::enterEvent(event);
}

void DkMenuBar::leaveEvent(QEvent *event)
{
    if (mTimeToShow == -1)
        return;

    mActive = false;
    mTimerMenu->start(mTimeToShow);

    QMenuBar::leaveEvent(event);
}

// DkTcpMenu --------------------------------------------------------------------
DkTcpMenu::DkTcpMenu(const QString &title, QWidget *parent)
    : QMenu(title, parent)
{
    connect(this, &DkTcpMenu::aboutToShow, this, &DkTcpMenu::updatePeers);
    connect(this, &DkTcpMenu::synchronizeWithSignal, DkSyncManager::inst().client(), &DkClientManager::synchronizeWith);
}

DkTcpMenu::~DkTcpMenu() = default;

void DkTcpMenu::addTcpAction(QAction *tcpAction)
{
    mTcpActions.append(tcpAction);
}

void DkTcpMenu::showNoClientsFound(bool show)
{
    mNoClientsFound = show;
}

void DkTcpMenu::clear()
{
    QMenu::clear();
    mTcpActions.clear();
}

void DkTcpMenu::enableActions(bool enable, bool local)
{
    updatePeers();

    if (local)
        return;

    bool anyConnected = enable;

    // let's see if any other connection is there
    if (!anyConnected) {
        for (int idx = 0; idx < mTcpActions.size(); idx++) {
            if (mTcpActions.at(idx)->objectName() == "tcpAction" && mTcpActions.at(idx)->isChecked()) {
                anyConnected = true;
                break;
            }
        }
    }

    for (int idx = 0; idx < mTcpActions.size(); idx++) {
        if (mTcpActions.at(idx)->objectName() == "serverAction")
            mTcpActions.at(idx)->setEnabled(!anyConnected);
    }
}

void DkTcpMenu::updatePeers()
{ // find other clients on paint

    auto ct = DkSyncManager::inst().client();
    QList<DkPeer *> newPeers = ct->getPeerList(); // TODO: remove old style

    // just update if the peers have changed...
    QMenu::clear();

    // show dummy action
    if (newPeers.empty() && mNoClientsFound) {
        auto *defaultAction = new QAction(tr("no clients found"), this);
        defaultAction->setEnabled(false);
        addAction(defaultAction);
        return;
    }

    if (!mNoClientsFound || !newPeers.empty()) {
        for (int idx = 0; idx < mTcpActions.size(); idx++) {
            addAction(mTcpActions.at(idx));
        }
    }

    for (int idx = 0; idx < newPeers.size(); idx++) {
        DkPeer *currentPeer = newPeers[idx];

        QString title = (mNoClientsFound) ? currentPeer->mTitle
                                          : currentPeer->mClientName % QString(": ") % currentPeer->mTitle;

        auto *peerEntry = new DkTcpAction(currentPeer, title, this);
        if (!mNoClientsFound)
            peerEntry->setTcpActions(&mTcpActions);

        connect(peerEntry, &DkTcpAction::synchronizeWithSignal, ct, &DkClientManager::synchronizeWith);
        connect(peerEntry, &DkTcpAction::disableSynchronizeWithSignal, ct, &DkClientManager::stopSynchronizeWith);
        connect(peerEntry, &DkTcpAction::enableActions, this, [this](bool enable) {
            this->enableActions(enable);
        });

        addAction(peerEntry);
    }
}

// DkTcpAction --------------------------------------------------------------------
DkTcpAction::DkTcpAction()
    : QAction(nullptr)
{
}

DkTcpAction::DkTcpAction(DkPeer *peer, QObject *parent)
    : QAction(parent)
{
    mPeer = peer;
    init();
}

DkTcpAction::DkTcpAction(DkPeer *peer, const QString &text, QObject *parent)
    : QAction(text, parent)
{
    mPeer = peer;
    init();
}

DkTcpAction::DkTcpAction(DkPeer *peer, const QIcon &icon, const QString &text, QObject *parent)
    : QAction(icon, text, parent)
{
    mPeer = peer;
    init();
}

DkTcpAction::~DkTcpAction() = default;

void DkTcpAction::init()
{
    mTcpActions = nullptr;
    setObjectName("tcpAction");
    setCheckable(true);
    setChecked(mPeer->isSynchronized());
    connect(this, &DkTcpAction::triggered, this, &DkTcpAction::synchronize);
}

void DkTcpAction::setTcpActions(QList<QAction *> *actions)
{
    mTcpActions = actions;
}

void DkTcpAction::synchronize(bool checked)
{
    if (checked)
        emit synchronizeWithSignal(mPeer->mPeerId);
    else
        emit disableSynchronizeWithSignal(mPeer->mPeerId);

    emit enableActions(checked);
}

}
