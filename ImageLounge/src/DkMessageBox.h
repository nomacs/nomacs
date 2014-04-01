/*******************************************************************************************************
 DkMessageBox.h
 Created on:	28.03.2014
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2014 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2014 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2014 Florian Kleber <florian@nomacs.org>

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

#include <QDialog>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QCheckBox>

// needed in cpp
#include <QSettings>
#include <QLabel>
#include <QStyle>
#include <QGridLayout>
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QPushButton>

namespace nmc {

class DkMessageBox : public QDialog {
	Q_OBJECT

public:
	DkMessageBox(QMessageBox::Icon icon, 
		const QString& title, 
		const QString& text, 
		QMessageBox::StandardButtons buttons = QMessageBox::NoButton,
		QWidget* parent = 0, 
		Qt::WindowFlags f = Qt::Dialog);
	DkMessageBox(QWidget* parent = 0);

	~DkMessageBox();

	virtual void setVisible(bool visible);
	void setDefaultButton(QMessageBox::StandardButton button);

public slots:
	void buttonClicked(QAbstractButton* button);
	int exec();

protected:

	QLabel* iconLabel;
	QLabel* textLabel;
	QMessageBox::Icon icon;
	QDialogButtonBox* buttonBox;
	QCheckBox* showAgain;

	void createLayout(const QMessageBox::Icon& userIcon, const QString& userText, QMessageBox::StandardButtons buttons);
	void updateSize();
};

}