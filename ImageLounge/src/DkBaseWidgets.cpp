/*******************************************************************************************************
 DkBaseWidgets.cpp
 Created on:	11.02.2015
 
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

#include "DkBaseWidgets.h"
#include "DkSettings.h"
#include "DkUtils.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QGraphicsEffect>
#include <QAction>
#include <QDebug>
#include <QTimer>
#include <QPainter>
#pragma warning(pop)	// no warnings from includes - end

namespace nmc {

DkWidget::DkWidget(QWidget* parent, Qt::WindowFlags flags) : QWidget(parent, flags) {
	init();
}

void DkWidget::init() {

	setMouseTracking(true);

	bgCol = (DkSettings::app.appMode == DkSettings::mode_frameless) ?
		DkSettings::display.bgColorFrameless :
	DkSettings::display.bgColorWidget;

	showing = false;
	hiding = false;
	blocked = false;
	displaySettingsBits = 0;
	opacityEffect = 0;

	// painter problems if the widget is a child of another that has the same graphicseffect
	// widget starts on hide
	opacityEffect = new QGraphicsOpacityEffect(this);
	opacityEffect->setOpacity(0);
	opacityEffect->setEnabled(false);
	setGraphicsEffect(opacityEffect);

	setVisible(false);
}

void DkWidget::registerAction(QAction* action) {
	connect(this, SIGNAL(visibleSignal(bool)), action, SLOT(setChecked(bool)));
}

void DkWidget::block(bool blocked) {
	this->blocked = blocked;
	setVisible(false);
}

void DkWidget::setDisplaySettings(QBitArray* displayBits) {
	displaySettingsBits = displayBits;
}

bool DkWidget::getCurrentDisplaySetting() {


	if (!displaySettingsBits)
		return false;

	if (DkSettings::app.currentAppMode < 0 || DkSettings::app.currentAppMode >= displaySettingsBits->size()) {
		qDebug() << "[WARNING] illegal app mode: " << DkSettings::app.currentAppMode;
		return false;
	}

	return displaySettingsBits->testBit(DkSettings::app.currentAppMode);
};

bool DkWidget::isHiding() const {
	return hiding;
}

void DkWidget::show(bool saveSetting) {

	// here is a strange problem if you add a DkWidget to another DkWidget -> painters crash
	if (!blocked && !showing) {
		hiding = false;
		showing = true;
		setVisible(true, saveSetting);
		animateOpacityUp();
	}
}

void DkWidget::hide(bool saveSetting) {

	if (!hiding) {
		hiding = true;
		showing = false;
		animateOpacityDown();

		// set display bit here too -> since the final call to setVisible takes a few seconds
		if (saveSetting && displaySettingsBits && displaySettingsBits->size() > DkSettings::app.currentAppMode) {
			displaySettingsBits->setBit(DkSettings::app.currentAppMode, false);
		}
	}
}

void DkWidget::setVisible(bool visible, bool saveSetting) {

	if (blocked) {
		QWidget::setVisible(false);
		return;
	}

	if (visible && !isVisible() && !showing)
		opacityEffect->setOpacity(100);

	QWidget::setVisible(visible);
	emit visibleSignal(visible);	// if this gets slow -> put it into hide() or show()

	if (saveSetting && displaySettingsBits && displaySettingsBits->size() > DkSettings::app.currentAppMode) {
		displaySettingsBits->setBit(DkSettings::app.currentAppMode, true);
	}

}

void DkWidget::animateOpacityUp() {

	if (!showing)
		return;

	opacityEffect->setEnabled(true);
	if (opacityEffect->opacity() >= 1.0f || !showing) {
		opacityEffect->setOpacity(1.0f);
		showing = false;
		opacityEffect->setEnabled(false);
		return;
	}

	QTimer::singleShot(20, this, SLOT(animateOpacityUp()));
	opacityEffect->setOpacity(opacityEffect->opacity()+0.05);
}

void DkWidget::animateOpacityDown() {

	if (!hiding)
		return;

	opacityEffect->setEnabled(true);
	if (opacityEffect->opacity() <= 0.0f) {
		opacityEffect->setOpacity(0.0f);
		hiding = false;
		setVisible(false, false);	// finally hide the widget
		opacityEffect->setEnabled(false);
		return;
	}

	QTimer::singleShot(20, this, SLOT(animateOpacityDown()));
	opacityEffect->setOpacity(opacityEffect->opacity()-0.05);
}

// DkLabel --------------------------------------------------------------------
DkLabel::DkLabel(QWidget* parent, const QString& text) : QLabel(text, parent) {

	bgCol = (DkSettings::app.appMode == DkSettings::mode_frameless) ?
		DkSettings::display.bgColorFrameless :
	DkSettings::display.bgColorWidget;

	setMouseTracking(true);
	this->parent = parent;
	this->text = text;
	init();
	hide();
}

DkLabel::~DkLabel() {
	if (timer) delete timer;
	timer = 0;
}

void DkLabel::init() {

	time = -1;
	fixedWidth = -1;
	fontSize = 17;
	textCol = QColor(255, 255, 255);
	blocked = false;

	timer = new QTimer();
	timer->setSingleShot(true);
	connect(timer, SIGNAL(timeout()), this, SLOT(hide()));

	// default look and feel
	QFont font;
	font.setPixelSize(fontSize);
	QLabel::setFont(font);
	QLabel::setTextInteractionFlags(Qt::TextSelectableByMouse);

	QLabel::setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	updateStyleSheet();
}

void DkLabel::hide() {
	time = 0;
	QLabel::hide();
}

void DkLabel::setText(const QString& msg, int time) {

	this->text = msg;
	this->time = time;

	if (!time || msg.isEmpty()) {
		hide();
		return;
	}

	setTextToLabel();
	show();

	if (time != -1)
		timer->start(time);
}

void DkLabel::showTimed(int time) {

	this->time = time;

	if (!time) {
		hide();
		return;
	}

	show();

	if (time != -1)
		timer->start(time);

}


QString DkLabel::getText() {
	return this->text;
}

void DkLabel::setFontSize(int fontSize) {

	this->fontSize = fontSize;

	QFont font;
	font.setPixelSize(fontSize);
	QLabel::setFont(font);
	QLabel::adjustSize();
}

void DkLabel::stop() {
	timer->stop();
	hide();
}

void DkLabel::updateStyleSheet() {
	QLabel::setStyleSheet("QLabel{color: " + textCol.name() + "; margin: " + 
		QString::number(margin.y()) + "px " +
		QString::number(margin.x()) + "px " +
		QString::number(margin.y()) + "px " +
		QString::number(margin.x()) + "px;}");
}

void DkLabel::paintEvent(QPaintEvent *event) {

	if (blocked || !time)	// guarantee that the label is hidden after the time is up
		return;

	QPainter painter(this);
	draw(&painter);
	painter.end();

	QLabel::paintEvent(event);
}

void DkLabel::draw(QPainter* painter) {

	drawBackground(painter);
}

void DkLabel::setFixedWidth(int fixedWidth) {

	this->fixedWidth = fixedWidth;
	setTextToLabel();
}

void DkLabel::setTextToLabel() {

	if (fixedWidth == -1) {
		QLabel::setText(text);
		QLabel::adjustSize();
	}
	else {
		setToolTip(text);
		QLabel::setText(fontMetrics().elidedText(text, Qt::ElideRight, fixedWidth-2*margin.x()));
		QLabel::resize(fixedWidth, height());
	}

}

DkLabelBg::DkLabelBg(QWidget* parent, const QString& text) : DkLabel(parent, text) {

	bgCol = (DkSettings::app.appMode == DkSettings::mode_frameless) ?
		DkSettings::display.bgColorFrameless :
	DkSettings::display.bgColorWidget;

	setAttribute(Qt::WA_TransparentForMouseEvents);	// labels should forward mouse events

	setObjectName("DkLabelBg");
	updateStyleSheet();

	margin = QPoint(7,2);
	setMargin(margin);
}

void DkLabelBg::updateStyleSheet() {

	QLabel::setStyleSheet("QLabel#DkLabelBg{color: " + textCol.name() + "; padding: " + 
		QString::number(margin.y()) + "px " +
		QString::number(margin.x()) + "px " +
		QString::number(margin.y()) + "px " +
		QString::number(margin.x()) + "px; " +
		"background-color: " + DkUtils::colorToString(bgCol) + ";}");	// background
}

// DkFadeLabel --------------------------------------------------------------------
DkFadeLabel::DkFadeLabel(QWidget* parent, const QString& text) : DkLabel(parent, text) {
	init();
}

void DkFadeLabel::init() {

	bgCol = (DkSettings::app.appMode == DkSettings::mode_frameless) ?
		DkSettings::display.bgColorFrameless :
	DkSettings::display.bgColorWidget;

	showing = false;
	hiding = false;
	blocked = false;
	displaySettingsBits = 0;

	// widget starts on hide
	opacityEffect = new QGraphicsOpacityEffect(this);
	opacityEffect->setOpacity(0);
	opacityEffect->setEnabled(false);	// default disabled -> otherwise we get problems with children having the same effect
	setGraphicsEffect(opacityEffect);

	setVisible(false);
}

void DkFadeLabel::block(bool blocked) {
	this->blocked = blocked;
	setVisible(false);
}

void DkFadeLabel::registerAction(QAction* action) {
	connect(this, SIGNAL(visibleSignal(bool)), action, SLOT(setChecked(bool)));
}

void DkFadeLabel::setDisplaySettings(QBitArray* displayBits) {
	displaySettingsBits = displayBits;
}

bool DkFadeLabel::getCurrentDisplaySetting() {

	if (!displaySettingsBits)
		return false;

	if (DkSettings::app.currentAppMode < 0 || DkSettings::app.currentAppMode >= displaySettingsBits->size()) {
		qDebug() << "[WARNING] illegal app mode: " << DkSettings::app.currentAppMode;
		return false;
	}

	return displaySettingsBits->testBit(DkSettings::app.currentAppMode);
}

void DkFadeLabel::show(bool saveSettings) {

	if (!blocked && !showing) {
		hiding = false;
		showing = true;
		setVisible(true, saveSettings);
		animateOpacityUp();
	}
}

void DkFadeLabel::hide(bool saveSettings) {

	if (!hiding) {
		hiding = true;
		showing = false;
		animateOpacityDown();
	}

	if (saveSettings && displaySettingsBits && displaySettingsBits->size() > DkSettings::app.currentAppMode) {
		displaySettingsBits->setBit(DkSettings::app.currentAppMode, false);
	}
}

void DkFadeLabel::setVisible(bool visible, bool saveSettings) {

	if (blocked) {
		DkLabel::setVisible(false);
		return;
	}

	if (visible && !isVisible() && !showing)
		opacityEffect->setOpacity(100);

	emit visibleSignal(visible);
	DkLabel::setVisible(visible);

	if (saveSettings && displaySettingsBits && displaySettingsBits->size() > DkSettings::app.currentAppMode) {
		displaySettingsBits->setBit(DkSettings::app.currentAppMode, visible);
	}

}

void DkFadeLabel::animateOpacityUp() {

	if (!showing)
		return;

	opacityEffect->setEnabled(true);
	if (opacityEffect->opacity() >= 1.0f || !showing) {
		opacityEffect->setOpacity(1.0f);
		opacityEffect->setEnabled(false);
		showing = false;
		return;
	}

	QTimer::singleShot(20, this, SLOT(animateOpacityUp()));
	opacityEffect->setOpacity(opacityEffect->opacity()+0.05);
}

void DkFadeLabel::animateOpacityDown() {

	if (!hiding)
		return;

	opacityEffect->setEnabled(true);
	if (opacityEffect->opacity() <= 0.0f) {
		opacityEffect->setOpacity(0.0f);
		hiding = false;
		opacityEffect->setEnabled(false);
		setVisible(false, false);	// finally hide the widget
		return;
	}

	QTimer::singleShot(20, this, SLOT(animateOpacityDown()));
	opacityEffect->setOpacity(opacityEffect->opacity()-0.05);
}

}
