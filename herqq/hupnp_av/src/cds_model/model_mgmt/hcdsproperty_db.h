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

#ifndef HCDSPROPERTY_DB_H_
#define HCDSPROPERTY_DB_H_

#include <HUpnpAv/HUpnpAv>

class QMutex;

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HCdsPropertyDbPrivate;

/*!
 * \brief This class is used to contain HCdsProperty instances, which describe
 * a CDS property and contain the necessary functionality for serializing,
 * comparing and validating CDS object properties.
 *
 * \headerfile hcdsproperty_db.h HCdsPropertyDb
 *
 * \ingroup hupnp_av_cds_om_mgmt
 *
 * \remarks This class \b is thread-safe.
 */
class H_UPNP_AV_EXPORT HCdsPropertyDb
{
H_DISABLE_COPY(HCdsPropertyDb)

private:

    HCdsPropertyDbPrivate* h_ptr;

    static HCdsPropertyDb* s_instance;
    static QMutex* s_instanceLock;

    HCdsPropertyDb();
    ~HCdsPropertyDb();

public:

    /*!
     * \brief Returns the single property database instance.
     *
     * \return The single property database instance.
     */
    static HCdsPropertyDb& instance();

    /*!
     * Adds a new HCdsProperty to the instance, if a property with the same name
     * is not stored already.
     *
     * \param arg specifies the new CDS property object to add.
     *
     * \return \e true if the property was added.
     */
    bool registerProperty(const HCdsProperty& arg);

    /*!
     * Removes the specified HCdsProperty instance.
     *
     * \param name specifies the name of the CDS property to remove.
     *
     * \return \e true if a property with the specified \a name was unregistered.
     */
    bool unregisterProperty(const QString& name);

    /*!
     * \brief Returns the HCdsProperty instance matching the specified name, if any.
     *
     * \param name specifies the name of the CDS property.
     *
     * \return the HCdsProperty instance matching the specified name, if any.
     */
    HCdsProperty property(const QString& name) const;

    /*!
     * \brief Returns the names of all the properties that belong to the DIDL-Lite
     * namespace.
     *
     * \return The names of all the properties that belong to the DIDL-Lite
     * namespace.
     */
    QSet<QString> didlLiteDependentProperties() const;
};

}
}
}

#endif /* HCDSPROPERTY_DB_H_ */
