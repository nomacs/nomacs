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

#pragma warning(push, 0) // no warnings from includes
#include <QWidget>
#pragma warning(pop)

#ifndef DllExport
#ifdef DK_DLL_EXPORT
#define DllExport Q_DECL_EXPORT
#elif defined(DK_DLL_IMPORT)
#define DllExport Q_DECL_IMPORT
#else
#define DllExport
#endif
#endif

class QTextEdit;

namespace nmc
{

void widgetMessageHandler(QtMsgType type, const QMessageLogContext &, const QString &msg);

class DkMessageQueuer : public QObject
{
    Q_OBJECT

public:
    DkMessageQueuer();

    void log(QtMsgType type, const QString &msg);

signals:
    void message(const QString &msg);
};

class DkLogDock : public DkDockWidget
{
    Q_OBJECT

public:
    DkLogDock(const QString &title, QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());

protected:
    void createLayout();
};

class DkLogWidget : public DkWidget
{
    Q_OBJECT

public:
    DkLogWidget(QWidget *parent = 0);

public slots:
    void log(const QString &msg);
    void on_clearButton_pressed();

protected:
    void createLayout();

    QTextEdit *mTextEdit;
};

}