/*
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of an application named HUpnpSimpleTestApp
 *  used for demonstrating how to use the Herqq UPnP (HUPnP) library.
 *
 *  HUpnpSimpleTestApp is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  HUpnpSimpleTestApp is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with HUpnpSimpleTestApp. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INVOKEACTIONDIALOG_H
#define INVOKEACTIONDIALOG_H

#include "i_dataholder.h"

#include <HUpnpCore/HUpnp>
#include <HUpnpCore/HAsyncOp>

#include <QHash>
#include <QtCore/QString>
#include <QtGui/QDialog>

struct QUuid;

namespace Ui {
    class InvokeActionDialog;
}

//
//
//
class InvokeActionDialog :
    public QDialog
{
Q_OBJECT
Q_DISABLE_COPY(InvokeActionDialog)

private:

    Ui::InvokeActionDialog* m_ui;
    Herqq::Upnp::HClientAction* m_action;
    QHash<QString, IDataHolder*> m_inputWidgets;

    void setupArgumentWidgets();
    IDataHolder* createDataHolder(const Herqq::Upnp::HStateVariableInfo&);

public:

    explicit InvokeActionDialog(
        Herqq::Upnp::HClientAction*, QWidget* parent = 0);

    virtual ~InvokeActionDialog();

public Q_SLOTS:

    void contentSourceRemoved(Herqq::Upnp::HClientDevice*);

protected:

    virtual void changeEvent(QEvent*);

private slots:

    void on_invokeButton_clicked();
    void invokeComplete(
        Herqq::Upnp::HClientAction*, const Herqq::Upnp::HClientActionOp&);
};

#endif // INVOKEACTIONDIALOG_H
