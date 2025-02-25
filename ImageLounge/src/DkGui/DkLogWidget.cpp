/*******************************************************************************************************
flowView is a visualization and auto-gating tool for FCS data (Flow Cytometry Data).
This software is part of the EU Project AutoFLOW [1] and
is developed at the Computer Vision Lab [2] at TU Wien.

Copyright (C) 2014-2016 Markus Diem <diemmarkus@gmail.com>

This file is part of flowView.

flowView is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

flowView is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

related links:
[1] http://www.autoflow-project.eu/
[2] http://www.caa.tuwien.ac.at/cvl/
[3] http://nomacs.org/
*******************************************************************************************************/

#include "DkLogWidget.h"

#include "DkUtils.h"

#pragma warning(push, 0) // no warnings from includes
#include <QAction>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#pragma warning(pop)

namespace nmc
{

QSharedPointer<DkMessageQueuer> msgQueuer = QSharedPointer<DkMessageQueuer>();

// -------------------------------------------------------------------- DkLogWidget
DkLogWidget::DkLogWidget(QWidget *parent)
    : DkWidget(parent)
{
    setObjectName("logWidget");
    createLayout();

    if (!msgQueuer)
        msgQueuer = QSharedPointer<DkMessageQueuer>(new DkMessageQueuer());

    connect(msgQueuer.data(), &DkMessageQueuer::message, this, &DkLogWidget::log, Qt::QueuedConnection);

    qInstallMessageHandler(widgetMessageHandler);
}

void DkLogWidget::log(const QString &msg)
{
    mTextEdit->append(msg);
}

void DkLogWidget::onClearButtonPressed()
{
    mTextEdit->clear();
}

void DkLogWidget::createLayout()
{
    mTextEdit = new QTextEdit(this);
    mTextEdit->setReadOnly(true);
    mTextEdit->setFocusPolicy(Qt::ClickFocus);

    // we can't change text colors in qss so also fix the background color
    mTextEdit->setStyleSheet("QTextEdit { font-family: monospace; background-color: #000; }");

    // invisible clear button ?!
    QPushButton *clearButton = new QPushButton(this);
    clearButton->setFlat(true);
    clearButton->setFixedSize(QSize(32, 32));
    clearButton->setFocusPolicy(Qt::NoFocus);
    connect(clearButton, &QPushButton::clicked, this, &DkLogWidget::onClearButtonPressed);

    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mTextEdit, 1, 1);
    layout->addWidget(clearButton, 1, 1, Qt::AlignRight | Qt::AlignTop);
}

/// <summary>
/// Saves log messages to a temporary log file.
/// Log messages are saved to DkUtils::instance().app().logPath() if
/// DkUtils::instance().app().useLogFile ist true.
/// </summary>
/// <param name="type">The message type (QtDebugMsg are not written to the log).</param>
/// <param name=""></param>
/// <param name="msg">The message.</param>
void widgetMessageHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    if (msgQueuer) {
        msgQueuer->log(type, msg);
    }

    DkUtils::logToFile(type, msg);
}

// -------------------------------------------------------------------- DkLogDock
DkLogDock::DkLogDock(const QString &title, QWidget *parent, Qt::WindowFlags flags)
    : DkDockWidget(title, parent, flags)
{
    setObjectName("logDock");
    createLayout();
}

void DkLogDock::createLayout()
{
    DkLogWidget *logWidget = new DkLogWidget(this);
    logWidget->setFocusPolicy(Qt::ClickFocus);
    setWidget(logWidget);
}

DkMessageQueuer::DkMessageQueuer()
{
}

void DkMessageQueuer::log(QtMsgType type, const QString &msg)
{
    QString txt;

    switch (type) {
    case QtDebugMsg:
        txt = "<span style=\"color: #ddd\"><i>" + msg + "</i></span>";
        break;
    case QtInfoMsg:
        txt = "<span style=\"color: #66cce1\">" + msg + "</span>";
        break;
    case QtWarningMsg:
        txt = "<span style=\"color: #faa23d\">[Warning] " + msg + "</span>";
        break;
    case QtCriticalMsg:
        txt = "<span style=\"color: #ff5794\">[Critical] " + msg + "</span>";
        break;
    case QtFatalMsg:
        txt = "<span style=\"color: #ff5794\">[FATAL] " + msg + "</span>";
        break;
    default:
        return;
    }

    emit message(txt);
}

}
