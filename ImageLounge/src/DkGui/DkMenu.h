/*******************************************************************************************************
 DkMenu.h
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

#pragma once

#include <QMenuBar>

#include "DkQt5Compat.h"

class QTimer;

namespace nmc
{
class DkPeer;

/**
 * This class perfectly acts like a QMenuBar.
 * However, if timeToShow != -1 it is hidden
 * just like Windows 7 menus.
 * You need to install a shortcut that calls
 * showMenu() on Key_Alt events.
 * Note: if the menu is hidden, no actions
 * are propagated. So you should register
 * your actions additionally in the QMainWindow.
 **/
class DkMenuBar : public QMenuBar
{
    Q_OBJECT

public:
    /**
     * Creates a DkMenuBar.
     * @param parent the parent widget.
     * @param timeToShow the time to show in ms. If set to -1 it acts like a QMenu.
     **/
    explicit DkMenuBar(QWidget *parent = nullptr, int timeToShow = 5000);

    /**
     * Override method.
     * @param menu a menu.
     * @return QAction* the action returned by QMenuBar::addAction().
     **/
    QAction *addMenu(QMenu *menu);

    /**
     * Override method.
     * @param title the menu title. (& indicates the shortcut key)
     * @return QMenu* the menu returned by QMenuBar::addAction().
     **/
    QMenu *addMenu(const QString &title);

    /**
     * Override method.
     * @param icon the menu icon.
     * @param title the menu title.
     * @return QMenu* the menu returned by QMenuBar::addAction().
     **/
    QMenu *addMenu(const QIcon &icon, const QString &title);

    void setTimeToShow(int setTimeToShow);

public slots:

    /**
     * Shows or hides the menu.
     * If the menu is shown, the timer for the hide event is started.
     **/
    void showMenu();

    /**
     * Hides the menu if it is not active.
     * In this context, active means that the mouse is over the menubar
     * or one of its children is visible.
     **/
    void hideMenu();

protected:
    void enterEvent(DkEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QList<QMenu *> mMenus;
    bool mActive = false;
    int mTimeToShow = 5000;
    QTimer *mTimerMenu = nullptr;
};

class DkTcpAction : public QAction
{
    Q_OBJECT

public:
    DkTcpAction();
    explicit DkTcpAction(DkPeer *peer, QObject *parent = nullptr);
    DkTcpAction(DkPeer *peer, const QString &text, QObject *parent = nullptr);
    DkTcpAction(DkPeer *peer, const QIcon &icon, const QString &text, QObject *parent);
    ~DkTcpAction() override;

    void init();
    void setTcpActions(QList<QAction *> *actions);

signals:
    void synchronizeWithSignal(quint16) const;
    void disableSynchronizeWithSignal(quint16) const;
    void enableActions(bool enable) const;

public slots:
    void synchronize(bool checked);

protected:
    DkPeer *mPeer;
    QList<QAction *> *mTcpActions;
};

class DkTcpMenu : public QMenu
{
    Q_OBJECT

public:
    explicit DkTcpMenu(const QString &title = QString(), QWidget *parent = nullptr);
    ~DkTcpMenu() override;

    void addTcpAction(QAction *tcpAction);
    void showNoClientsFound(bool show);
    void clear();

signals:
    void synchronizeWithSignal(quint16) const;

public slots:
    void enableActions(bool enable = false, bool local = false);

protected slots:
    void updatePeers();

protected:
    QList<QAction *> mTcpActions;
    bool mNoClientsFound = false;
};
}
