/*******************************************************************************************************
 DkSettings.cpp
 Created on:	23.10.2018

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

#pragma once

#include "DkBaseWidgets.h"

class QTextEdit;

namespace nmc
{

void widgetMessageHandler(QtMsgType type, const QMessageLogContext &, const QString &msg);

class DkMessageQueuer : public QObject
{
    Q_OBJECT

public:
    DkMessageQueuer();

    // called via qt message log callback
    void log(QtMsgType type, const QString &msg);

signals:
    // must use QueuedConnection since log() must be thread-safe
    void message(const QString &msg);
};

class DkLogDock : public DkDockWidget
{
    Q_OBJECT

public:
    explicit DkLogDock(const QString &title, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

protected:
    void createLayout();
};

class DkLogWidget : public DkWidget
{
    Q_OBJECT

public:
    explicit DkLogWidget(QWidget *parent = nullptr);

public slots:
    void log(const QString &msg);
    void onClearButtonPressed();

protected:
    void createLayout();

    QTextEdit *mTextEdit;
};

}
