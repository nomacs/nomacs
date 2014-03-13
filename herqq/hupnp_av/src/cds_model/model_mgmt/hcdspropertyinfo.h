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

#ifndef HCDSPROPERTYINFO_H_
#define HCDSPROPERTYINFO_H_

#include <HUpnpAv/HCdsProperties>

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QSharedDataPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HCdsPropertyInfoPrivate;

/*!
 * \brief This class is used to contain information of a UPnP A/V CDS property.
 *
 * \headerfile hcdspropertyinfo.h HCdsPropertyInfo
 *
 * \ingroup hupnp_av_cds_om_mgmt
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HCdsPropertyInfo
{
friend class HCdsProperties;

public:

    /*!
     * \brief This enumeration is used to define additional information of a CDS property.
     */
    enum PropertyFlag
    {
        /*!
         * No additional information was specified.
         */
        None = 0,

        /*!
         * The property is defined by the UPnP A/V committee.
         */
        StandardType = 1,

        /*!
         * The property is multi valued.
         */
        MultiValued = 2,

        /*!
         * The property is mandatory for the specified CDS object.
         */
        Mandatory = 4,

        /*!
         * The property can be disabled.
         */
        Disableable = 8
    };
    Q_DECLARE_FLAGS(PropertyFlags, PropertyFlag)

private:

    QSharedDataPointer<HCdsPropertyInfoPrivate> h_ptr;

    static const HCdsPropertyInfo s_empty;

    static HCdsPropertyInfo* create(
        const QString& name, HCdsProperties::Property prop,
        const QVariant& defaultValue, PropertyFlags = None);

public:

    /*!
     * \brief Creates a new, invalid instance.
     *
     * \sa isEmpty()
     */
    HCdsPropertyInfo();

    /*!
     * \brief Creates a new instance.
     *
     * \param name specifies the name of the CDS property.
     *
     * \param defaultValue specifies the default value of the property.
     *
     * \param pfs specifies additional information of the property.
     *
     * \remarks The value HCdsPropertyInfo::StandardType in \a pfs is ignored.
     *
     * \sa isValid()
     */
    HCdsPropertyInfo(const QString& name, const QVariant& defaultValue, PropertyFlags pfs=None);

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HCdsPropertyInfo(const HCdsPropertyInfo&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HCdsPropertyInfo& operator=(const HCdsPropertyInfo&);

    /*!
     * Destroys the instance.
     */
    ~HCdsPropertyInfo();

    /*!
     * \brief Indicates if the instance contains property information.
     *
     * \return \e true if the instance is valid, i.e. at least name() is defined.
     */
    bool isValid() const;

    /*!
     * \brief Returns the name of the CDS property.
     *
     * \return The name of the CDS property.
     *
     * \sa type()
     */
    const QString& name() const;

    /*!
     * \brief Returns the type of the property, if available.
     *
     * \return the type of the property, if available. Note, this is set to
     * HCdsProperties::undefined if the property type is not recognized by
     * HUPnPAv, but the property may still be valid.
     *
     * \sa isValid()
     */
    HCdsProperties::Property type() const;

    /*!
     * \brief Returns the default value of the property.
     *
     * \return The default value of the property.
     */
    const QVariant& defaultValue() const;

    /*!
     * \brief Returns additional information of the property.
     *
     * \return additional information of the property.
     */
    PropertyFlags propertyFlags() const;

    /*!
     * Returns a const-reference to statically created empty instance.
     *
     * \return a const-reference to statically created empty instance.
     */
    inline static const HCdsPropertyInfo& empty()
    {
        return s_empty;
    }
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HCdsPropertyInfo
 */
H_UPNP_AV_EXPORT bool operator==(const HCdsPropertyInfo& obj1, const HCdsPropertyInfo& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HCdsPropertyInfo
 */
inline bool operator!=(const HCdsPropertyInfo& obj1, const HCdsPropertyInfo& obj2)
{
    return !(obj1 == obj2);
}

Q_DECLARE_OPERATORS_FOR_FLAGS(HCdsPropertyInfo::PropertyFlags)

}
}
}

#endif /* HCDSPROPERTYINFO_H_ */
