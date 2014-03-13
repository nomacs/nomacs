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

#ifndef ALLOWEDVALUELIST_INPUT_H_
#define ALLOWEDVALUELIST_INPUT_H_

#include "i_dataholder.h"

namespace Ui {
    class AllowedValueListInput;
}

class QStringList;

//
//
//
class AllowedValueListInput :
    public IDataHolder
{
Q_OBJECT

private:

    Ui::AllowedValueListInput* m_ui;

protected:

    void changeEvent(QEvent *e);

public:

    AllowedValueListInput(const QStringList& values, QWidget* parent = 0);

    virtual ~AllowedValueListInput();

    virtual QVariant data() const;
};

#endif // ALLOWEDVALUELIST_INPUT_H_
