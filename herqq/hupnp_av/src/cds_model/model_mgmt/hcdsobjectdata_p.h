/*
 *  Copyright (C) 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP Av (HUPnPAv) library.
 *
 *  Herqq UPnP Av is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP Av is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Herqq UPnP Av. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HCDS_OBJECTDATA_P_H_
#define HCDS_OBJECTDATA_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

//
//
//
class HCdsObjectData
{
H_DISABLE_COPY(HCdsObjectData)

private:

    QString m_dataPath;
    HObject* m_cdsObject;

public:

    //
    // Creates a new instance.
    //
    // \param cdsObj specifies the CDS object.
    //
    // \param path specifies the file system path to the data of the CDS object.
    // This parameter is optional.
    //
    HCdsObjectData(HObject* cdsObj, const QString& path = "");

    //
    // Destroys the instance.
    //
    // \remarks if the object has the ownership of the CDS object the CDS object
    // will be destroyed.
    //
    // \sa takeObject()
    //
    ~HCdsObjectData();

    //
    // Returns the absolute path to the file that contains the data represented
    // by the CDS object.
    //
    // \return the absolute path to the file that contains the data represented
    // by the CDS object.
    //
    inline QString dataPath() const { return m_dataPath; }

    //
    // Returns the CDS object.
    //
    // \return the CDS object.
    //
    // \remarks the ownership of the returned object is not transferred.
    //
    // \sa takeObject()
    //
    inline HObject* object() const { return m_cdsObject; }

    //
    // Releases ownership of the CDS object and returns it.
    //
    // \return the CDS object. The ownership is transferred to the caller.
    //
    // \sa object()
    //
    inline HObject* takeObject()
    {
        HObject* tmp = m_cdsObject;
        m_cdsObject = 0;
        return tmp;
    }
};
}
}
}

#endif /* HCDS_OBJECTDATA_P_H_ */
