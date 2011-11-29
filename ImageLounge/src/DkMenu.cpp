/*******************************************************************************************************
 DkMenu.cpp
 Created on:	09.08.2011
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011 Florian Kleber <florian@nomacs.org>

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
	qDebug() << "showing menu\n";
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
