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

#include "invokeactiondialog.h"
#include "ui_invokeactiondialog.h"

#include "genericinput.h"
#include "allowedvaluelist_input.h"


#include <HUpnpCore/HActionInfo>
#include <HUpnpCore/HClientAction>
#include <HUpnpCore/HClientDevice>
#include <HUpnpCore/HClientService>
#include <HUpnpCore/HActionArguments>
#include <HUpnpCore/HActionArguments>
#include <HUpnpCore/HClientActionOp>
#include <HUpnpCore/HClientStateVariable>

#include <QUuid>
#include <QMessageBox>
#include <QIntValidator>
#include <QDoubleValidator>

#include <limits>

using namespace Herqq::Upnp;

InvokeActionDialog::InvokeActionDialog(HClientAction* action, QWidget* parent) :
    QDialog(parent),
        m_ui(new Ui::InvokeActionDialog), m_action(action), m_inputWidgets()
{
    Q_ASSERT(action);

    m_ui->setupUi(this);
    setupArgumentWidgets();

    bool ok = connect(
        action, SIGNAL(invokeComplete(Herqq::Upnp::HClientAction*, Herqq::Upnp::HClientActionOp)),
        this, SLOT(invokeComplete(Herqq::Upnp::HClientAction*, Herqq::Upnp::HClientActionOp)));

    Q_ASSERT(ok); Q_UNUSED(ok)
}

void InvokeActionDialog::invokeComplete(
    HClientAction*, const HClientActionOp& eventInfo)
{
    if (eventInfo.returnValue() == UpnpSuccess)
    {
        HActionArguments outArgs = eventInfo.outputArguments();
        for(qint32 i = 0; i < outArgs.size(); ++i)
        {
            m_ui->outputArguments->item(i, 2)->setText(
                outArgs[i].value().toString());
        }
    }
    else
    {
        QMessageBox* msgBox = new QMessageBox(this);

        msgBox->setText(QString("Action invocation [id: %1] failed: %2").arg(
            QString::number(eventInfo.id()),
            upnpErrorCodeToString(eventInfo.returnValue())));

        msgBox->show();
    }

    m_ui->invokeButton->setEnabled(true);
}

void InvokeActionDialog::setupArgumentWidgets()
{
    HActionArguments inputArgs = m_action->info().inputArguments();

    m_ui->inputArguments->setRowCount(inputArgs.size());

    for(qint32 i = 0; i < inputArgs.size(); ++i)
    {
        HActionArgument inputArg = inputArgs[i];
        const HStateVariableInfo& stateVar = inputArg.relatedStateVariable();

        QTableWidgetItem* item =
            new QTableWidgetItem(HUpnpDataTypes::toString(stateVar.dataType()));

        item->setFlags(Qt::NoItemFlags);

        m_ui->inputArguments->setItem(i, 0, item);

        item = new QTableWidgetItem(stateVar.name());
        item->setFlags(Qt::NoItemFlags);

        m_ui->inputArguments->setItem(i, 1, item);

        IDataHolder* dh = createDataHolder(stateVar);
        Q_ASSERT(dh);
        m_inputWidgets[inputArg.name()] = dh;

        m_ui->inputArguments->setCellWidget(i, 2, dh);
        //m_ui->inputArguments->resizeColumnsToContents();
    }

    HActionArguments outputArgs = m_action->info().outputArguments();

    m_ui->outputArguments->setRowCount(outputArgs.size());

    for(qint32 i = 0; i < outputArgs.size(); ++i)
    {
        HActionArgument outputArg = outputArgs[i];
        const HStateVariableInfo& stateVar = outputArg.relatedStateVariable();

        QTableWidgetItem* item =
            new QTableWidgetItem(HUpnpDataTypes::toString(stateVar.dataType()));

        item->setFlags(Qt::NoItemFlags);

        m_ui->outputArguments->setItem(i, 0, item);

        item = new QTableWidgetItem(stateVar.name());
        item->setFlags(Qt::NoItemFlags);

        m_ui->outputArguments->setItem(i, 1, item);

        item = new QTableWidgetItem();
        item->setFlags(Qt::NoItemFlags);

        m_ui->outputArguments->setItem(i, 2, item);
    }
}

void InvokeActionDialog::contentSourceRemoved(HClientDevice* device)
{
    Q_ASSERT(device);
    if (device == m_action->parentService()->parentDevice()->rootDevice())
    {
        done(0);
    }
}

namespace
{
void minMaxValues(HUpnpDataTypes::DataType dt, qint32* max, qint32* min)
{
    switch(dt)
    {
        case HUpnpDataTypes::ui1:
            *max = std::numeric_limits<unsigned char>::max();
            *min = std::numeric_limits<unsigned char>::min();
            break;
        case HUpnpDataTypes::ui2:
            *max = std::numeric_limits<unsigned short>::max();
            *min = std::numeric_limits<unsigned short>::min();
            break;
        case HUpnpDataTypes::ui4:
            *max = std::numeric_limits<int>::max();
            *min = std::numeric_limits<int>::min();
            // for this example, the signed int range is acceptable.
            break;

        case HUpnpDataTypes::i1:
            *max = std::numeric_limits<char>::max();
            *min = std::numeric_limits<char>::min();
            break;
        case HUpnpDataTypes::i2:
            *max = std::numeric_limits<short>::max();
            *min = std::numeric_limits<short>::min();
            break;
        case HUpnpDataTypes::i4:
            *max = std::numeric_limits<int>::max();
            *min = std::numeric_limits<int>::min();
            break;

        default:
            Q_ASSERT(false);
    }
}

void minMaxValues(HUpnpDataTypes::DataType dt, qreal* max, qreal* min)
{
    switch(dt)
    {
        case HUpnpDataTypes::r4:
        case HUpnpDataTypes::fp:
            *max = std::numeric_limits<float>::max();
            *min = std::numeric_limits<float>::min();
            break;
        case HUpnpDataTypes::r8:
        case HUpnpDataTypes::number:
        case HUpnpDataTypes::fixed_14_4:
            *max = std::numeric_limits<double>::max();
            *min = std::numeric_limits<double>::min();
            break;
        default:
            Q_ASSERT(false);
    }
}
}

IDataHolder* InvokeActionDialog::createDataHolder(
    const HStateVariableInfo& stateVar)
{
    IDataHolder* content = 0;

    if (HUpnpDataTypes::isInteger(stateVar.dataType()))
    {
        if (stateVar.isConstrained())
        {
            content = new GenericInput(
                new QIntValidator(
                    stateVar.minimumValue().toInt(),
                    stateVar.maximumValue().toInt(),
                    0));
        }
        else
        {
            qint32 max = 0, min = 0;
            minMaxValues(stateVar.dataType(), &max, &min);
            content = new GenericInput(new QIntValidator(min, max, 0));
        }
    }
    else if (HUpnpDataTypes::isRational(HUpnpDataTypes::string))
    {
        if (stateVar.isConstrained())
        {
            content = new GenericInput(
                new QDoubleValidator(
                    stateVar.minimumValue().toDouble(),
                    stateVar.maximumValue().toDouble(),
                    0,
                    0));
        }
        else
        {
            qreal max = 0, min = 0;
            minMaxValues(stateVar.dataType(), &max, &min);
            content = new GenericInput(new QDoubleValidator(min, max, 0, 0));
        }
    }
    else if (stateVar.dataType() == HUpnpDataTypes::string)
    {
        if (stateVar.isConstrained())
        {
            content = new AllowedValueListInput(stateVar.allowedValueList());
        }
        else
        {
            content = new GenericInput();
        }
    }
    else if (stateVar.dataType() == HUpnpDataTypes::boolean)
    {
        QStringList allowedValues;
        allowedValues.append("True");
        allowedValues.append("False");

        content = new AllowedValueListInput(allowedValues);
    }
    else
    {
        content = new GenericInput();
    }

    return content;
}

InvokeActionDialog::~InvokeActionDialog()
{
    delete m_ui;
}

void InvokeActionDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void InvokeActionDialog::on_invokeButton_clicked()
{
    HActionArguments inputArgs = m_action->info().inputArguments();

    for(qint32 i = 0; i < inputArgs.size(); ++i)
    {
        HActionArgument inputArg = inputArgs[i];

        IDataHolder* dataHolder = m_inputWidgets[inputArg.name()];

        QVariant data = dataHolder->data();
        if (inputArg.isValidValue(data))
        {
            bool ok = inputArg.setValue(data);
            Q_ASSERT(ok); Q_UNUSED(ok)
        }
        else
        {
            QMessageBox mbox;
            mbox.setText(QObject::tr("Check your arguments!"));
            mbox.setWindowTitle(QObject::tr("Error"));

            mbox.exec();
            return;
        }
    }

    m_action->beginInvoke(inputArgs);
    m_ui->invokeButton->setEnabled(false);
}
