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
#include "DkSettings.h"
#include "DkNetwork.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#include <QFileInfo>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QStringBuilder>
#include <QDebug>
#pragma warning(pop)		// no warnings from includes - end

#ifdef QT_NO_DEBUG_OUTPUT
#pragma warning(disable: 4127)		// no 'conditional expression is constant' if qDebug() messages are removed
#endif

namespace nmc {

// DkMenu --------------------------------------------------------------------
DkMenuBar::DkMenuBar(QWidget *parent, int timeToShow) : QMenuBar(parent) {

	this->timeToShow = timeToShow;	// default: 5 seconds
	active = false;

	timerMenu = new QTimer(this);
	timerMenu->setSingleShot(true);
	connect(timerMenu, SIGNAL(timeout()), this, SLOT(hideMenu()));

	if (timeToShow != -1)
		timerMenu->start(timeToShow);
}

QAction* DkMenuBar::addMenu(QMenu* menu) {

	menus.append(menu);

	return QMenuBar::addMenu(menu);
}

QMenu* DkMenuBar::addMenu(const QString& title) {

	QMenu* newMenu = QMenuBar::addMenu(title);
	menus.append(newMenu);

	return newMenu;
}

QMenu* DkMenuBar::addMenu(const QIcon& icon, const QString& title) {

	QMenu* newMenu = QMenuBar::addMenu(icon, title);
	menus.append(newMenu);

	return newMenu;
}

void DkMenuBar::showMenu() {

	if (timeToShow == -1)
		return;

	if (isVisible()) {
		timerMenu->stop();
		hideMenu();
		return;
	}

	timerMenu->start(timeToShow);
	show();
	qDebug() << "mShowing menu\n";
}

void DkMenuBar::hideMenu() {

	if (timeToShow == -1)
		return;

	// ok we have a mouseover
	if (active)
		return;

	for (int idx = 0; idx < menus.size(); idx++) {

		// ok, a child is active -> wait for it
		if (menus.at(idx)->isVisible()) {
			timerMenu->start(timeToShow);
			return;
		}
	}

	if (!active)
		hide();
}

void DkMenuBar::setTimeToShow(int timeToShow) {

	this->timeToShow = timeToShow;
}

void DkMenuBar::enterEvent(QEvent* event) {

	if (timeToShow == -1)
		return;

	active = true;

	QMenuBar::enterEvent(event);
}

void DkMenuBar::leaveEvent(QEvent* event) {

	if (timeToShow == -1)
		return;

	active = false;
	timerMenu->start(timeToShow);

	QMenuBar::leaveEvent(event);

}

// DkTcpMenu --------------------------------------------------------------------
DkTcpMenu::DkTcpMenu(QWidget* parent, DkManagerThread* clientThread) : QMenu(parent) {
	this->clientThread = clientThread;

	connect(this, SIGNAL(aboutToShow()), this, SLOT(updatePeers()));

	if (clientThread)
		connect(this, SIGNAL(synchronizeWithSignal(quint16)), clientThread, SLOT(synchronizeWith(quint16)));

	noClientsFound = false;
}

DkTcpMenu::DkTcpMenu(const QString& title, QWidget* parent, DkManagerThread* clientThread) : QMenu(title, parent) {
	this->clientThread = clientThread;

	connect(this, SIGNAL(aboutToShow()), this, SLOT(updatePeers()));

	if (clientThread)
		connect(this, SIGNAL(synchronizeWithSignal(quint16)), clientThread, SLOT(synchronizeWith(quint16)));

	noClientsFound = false;
}

DkTcpMenu::~DkTcpMenu() {}

void DkTcpMenu::setClientManager(DkManagerThread* clientThread) {
	this->clientThread = clientThread;
	if (clientThread)
		connect(this, SIGNAL(synchronizeWithSignal(quint16)), clientThread, SLOT(synchronizeWith(quint16)));
}

void DkTcpMenu::addTcpAction(QAction* tcpAction) {
	tcpActions.append(tcpAction);
}

void DkTcpMenu::showNoClientsFound(bool show) {
	noClientsFound = show;
}

void DkTcpMenu::clear() {
	QMenu::clear();
	peers.clear();
	clients.clear();
	tcpActions.clear();
}

void DkTcpMenu::enableActions(bool enable, bool local) {

	updatePeers();

	if (local)
		return;

	bool anyConnected = enable;

	// let's see if any other connection is there
	if (!anyConnected) {

		for (int idx = 0; idx < tcpActions.size(); idx++) {

			if (tcpActions.at(idx)->objectName() == "tcpAction" && tcpActions.at(idx)->isChecked()) {
				anyConnected = true;
				break;
			}
		}
	}

	for (int idx = 0; idx < tcpActions.size(); idx++) {

		if (tcpActions.at(idx)->objectName() == "serverAction")
			tcpActions.at(idx)->setEnabled(!anyConnected);
		if (tcpActions.at(idx)->objectName() == "sendImageAction" && DkSettings::sync.syncMode == DkSettings::sync_mode_default)
			tcpActions.at(idx)->setEnabled(anyConnected);
	}

}

void DkTcpMenu::updatePeers() {	// find other clients on paint

	if (!clientThread)
		return;

	QList<DkPeer*> newPeers = clientThread->getPeerList();	// TODO: remove old style

	// just update if the peers have changed...
	QMenu::clear();

	// show dummy action
	if (newPeers.empty() && noClientsFound) {
		qDebug() << "dummy node...\n";
		QAction* defaultAction = new QAction(tr("no clients found"), this);
		defaultAction->setEnabled(false);
		addAction(defaultAction);
		return;
	}

	if (!noClientsFound || !newPeers.empty()) {

		for (int idx = 0; idx < tcpActions.size(); idx++) {

			if (tcpActions.at(idx)->objectName() != "sendImageAction")
				addAction(tcpActions.at(idx));
		}

		//QList<QAction*>::iterator actionIter = tcpActions.begin();
		//while (actionIter != tcpActions.end()) {
		//	addAction(*actionIter);
		//	actionIter++;
		//}
	}

	for (int idx = 0; idx < newPeers.size(); idx++) {

		DkPeer* currentPeer = newPeers[idx];

		QString title = (noClientsFound) ? currentPeer->title : currentPeer->clientName % QString(": ") % currentPeer->title;

		DkTcpAction* peerEntry = new DkTcpAction(currentPeer, title, this);
		if (!noClientsFound) 
			peerEntry->setTcpActions(&tcpActions);

		connect(peerEntry, SIGNAL(synchronizeWithSignal(quint16)), clientThread, SLOT(synchronizeWith(quint16)));
		connect(peerEntry, SIGNAL(disableSynchronizeWithSignal(quint16)), clientThread, SLOT(stopSynchronizeWith(quint16)));
		connect(peerEntry, SIGNAL(enableActions(bool)), this, SLOT(enableActions(bool)));

		addAction(peerEntry);

	}

	peers = newPeers;
}

// DkTcpAction --------------------------------------------------------------------
DkTcpAction::DkTcpAction() : QAction(0) {}

DkTcpAction::DkTcpAction(DkPeer* peer, QObject* parent) : QAction(parent) {
	this->peer = peer;
	init();
}

DkTcpAction::DkTcpAction(DkPeer* peer, const QString& text, QObject* parent) : QAction(text, parent) {
	this->peer = peer;
	init();
}

DkTcpAction::DkTcpAction(DkPeer* peer, const QIcon& icon, const QString& text, QObject* parent) : QAction(icon, text, parent) {
	this->peer = peer;
	init();
}

DkTcpAction::~DkTcpAction() {}

void DkTcpAction::init() {
	tcpActions = 0;
	setObjectName("tcpAction");
	setCheckable(true);
	setChecked(peer->isSynchronized());
	connect(this, SIGNAL(triggered(bool)), this, SLOT(synchronize(bool)));
}

void DkTcpAction::setTcpActions(QList<QAction*>* actions) {
	tcpActions = actions;
}

void DkTcpAction::synchronize(bool checked) {

	if (checked)
		emit synchronizeWithSignal(peer->peerId);
	else
		emit disableSynchronizeWithSignal(peer->peerId);

	emit enableActions(checked);
	qDebug() << "emitted a synchronize message...\n";
}

}
