/*
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP (HUPnP) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HASYNCOP_P_H_
#define HASYNCOP_P_H_

#include <HUpnpCore/HUpnp>

#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{



//
// Implementation details of HAsyncOp
//
class H_UPNP_CORE_EXPORT HAsyncOpPrivate
{
H_DISABLE_COPY(HAsyncOpPrivate)

private:

    const unsigned int m_id;

public:

    int m_refCount;

    int m_returnValue;
    void* m_userData;
    QString* m_errorDescription;

    inline HAsyncOpPrivate() :
        m_id(0), m_refCount(1), m_returnValue(0), m_userData(0),
        m_errorDescription(0)
    {
    }

    inline HAsyncOpPrivate(int id) :
        m_id(id), m_refCount(1), m_returnValue(0), m_userData(0),
        m_errorDescription(0)
    {
    }

    static unsigned int genId();

    virtual ~HAsyncOpPrivate();

    inline unsigned int id() const { return m_id; }
};

}
}

#endif /* HASYNCOP_P_H_ */
