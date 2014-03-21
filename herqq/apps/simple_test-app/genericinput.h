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

#ifndef GENERICINPUT_H
#define GENERICINPUT_H

#include "i_dataholder.h"

#include <QWidget>

namespace Ui {
    class GenericInput;
}

class QValidator;

//
//
//
class GenericInput :
    public IDataHolder
{
Q_OBJECT
Q_DISABLE_COPY(GenericInput)

private:

    Ui::GenericInput* m_ui;
    QValidator* m_inputValidator;

protected:

    void changeEvent(QEvent *e);

public:

    GenericInput(QValidator* inputValidator = 0, QWidget* parent = 0);
    virtual ~GenericInput();

    virtual QVariant data() const;
};

#endif // GENERICINPUT_H
