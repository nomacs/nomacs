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

#include "genericinput.h"
#include "ui_genericinput.h"

GenericInput::GenericInput(QValidator* inputValidator, QWidget* parent) :
    IDataHolder(parent),
        m_ui(new Ui::GenericInput), m_inputValidator(inputValidator)
{
    m_ui->setupUi(this);

    if (m_inputValidator)
    {
        m_inputValidator->setParent(this);
        m_ui->inputLineEdit->setValidator(m_inputValidator);
    }
}

GenericInput::~GenericInput()
{
    delete m_ui;
}

void GenericInput::changeEvent(QEvent* e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

QVariant GenericInput::data() const
{
    return m_ui->inputLineEdit->text();
}
