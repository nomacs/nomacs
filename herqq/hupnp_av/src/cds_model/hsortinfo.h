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

#ifndef HSORTINFO_H_
#define HSORTINFO_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QString>
#include <QtCore/QSharedDataPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This is a convenience class for working with sort modifiers
 * defined in the ContentDirectory:3 specification.
 *
 * \headerfile HSortModifier.h HSortModifier
 *
 * \ingroup hupnp_av_cds_common
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HSortModifier
{

public:

    /*!
     * \brief This enumeration specifies the \e sort \e modifier values defined
     * in the ContentDirectory:3 specification.
     */
    enum Type
    {
        /*!
         * The sort modifier is not defined.
         */
        Undefined,

        /*!
         * The sort order is ascending considering the value of the associated
         * CDS property.
         */
        AscendingByValue,

        /*!
         * The sort order is descending considering the value of the associated
         * CDS property.
         */
        DescendingByValue,

        /*!
         * The sort order is ascending considering \b only the time part of the
         * associated date format property. An example of such a property is
         * <c>dc:date</c>.
         */
        AscendingByTime,

        /*!
         * The sort order is descending considering \b only the time part of the
         * associated date format property. An example of such a property is
         * <c>dc:date</c>.
         */
        DescendingByTime,

        /*!
         * Vendor-defined.
         */
        VendorDefined
    };

    QString m_typeAsString;
    Type m_type;

public:

    /*!
     * Creates a new instance, with the type() set to HSortInfo::AscendingByValue.
     *
     * \sa isValid()
     */
    HSortModifier();

    /*!
     * \brief Creates a new instance.
     *
     * \param type specifies the sort modifier type.
     *
     * \sa isValid()
     */
    HSortModifier(Type type);

    /*!
     * Creates a new instance from the specified string.
     *
     * \param arg specifies the sort modifier type. If the specified string does not
     * correspond to any Type value and the string is not empty, the type() is set to
     * HSortModifier::VendorDefined.
     *
     * \sa isValid()
     */
    HSortModifier(const QString& arg);

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true in case the object is valid, i.e. the type() is
     * not HSortModifier::Undefined.
     */
    inline bool isValid() const { return m_type != Undefined; }

    /*!
     * \brief Returns a string representation of the object.
     *
     * \return a string representation of the object.
     */
    inline QString toString() const { return m_typeAsString; }

    /*!
     * \brief Returns the type value.
     *
     * \return The type value.
     */
    inline Type type() const { return m_type; }

    /*!
     * \brief Indicates if the sort order is \e ascending.
     *
     * \param ok specifies a pointer to \c bool, which is set to \e true, when
     * the sort order was successfully determined. This is an optional parameter.
     *
     * \return \e true if the sort is \e ascending. Otherwise the sort order is
     * considered to be descending. Note, \e false is returned also in case the
     * sort order could not be determined.
     */
    bool ascending(bool* ok = 0) const;

    /*!
     * Converts the specified Type value to string.
     *
     * \param type specifies the Type value to be converted to string.
     *
     * \return a string representation of the specified Type value.
     */
    static QString toString(Type type);

    /*!
     * \brief Returns a Type value corresponding to the specified string, if any.
     *
     * \param type specifies the Type as string.
     *
     * \return a Type value corresponding to the specified string, if any.
     */
    static Type fromString(const QString& type);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HSortModifier
 */
H_UPNP_AV_EXPORT bool operator==(const HSortModifier& obj1, const HSortModifier& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HSortModifier
 */
inline bool operator!=(const HSortModifier& obj1, const HSortModifier& obj2)
{
    return !(obj1 == obj2);
}

class HSortInfoPrivate;

/*!
 * \brief This class is used to convey sorting-related information to ContentDirectory's
 * \e Browse and \e Search actions.
 *
 * \headerfile hsortinfo.h HSortInfo
 *
 * \ingroup hupnp_av_cds_common
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HContentDirectoryService::browse(), HContentDirectoryService::search(),
 * HContentDirectoryAdapter::browse(), HContentDirectoryAdapter::search().
 */
class H_UPNP_AV_EXPORT HSortInfo
{
private:

    QSharedDataPointer<HSortInfoPrivate> h_ptr;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HSortInfo();

    /*!
     * \brief Creates a new instance.
     *
     * \param property specifies the name of CDS property to which the sort
     * information applies.
     *
     * \param modifier specifies the sort modifier to use. The default sort
     * modifier is to sort in ascending order using the value of the specified
     * property as such.
     */
    HSortInfo(
        const QString& property,
        const HSortModifier& modifier = HSortModifier());

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HSortInfo(const HSortInfo&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HSortInfo& operator=(const HSortInfo&);

    /*!
     * \brief Destroys the instance.
     */
    ~HSortInfo();

    /*!
     * \brief Indicates if the object contains valid information for a
     * \e sort \e criteria.
     *
     * \return \e true if the object contains valid information for a
     * \e sort \e criteria, i.e. at least the property() is defined.
     */
    bool isValid() const;

    /*!
     * \brief Returns the name of CDS property to which the sort information applies.
     *
     * \return The name of CDS property to which the sort information applies.
     *
     * \sa setProperty()
     */
    QString property() const;

    /*!
     * \brief Returns the sort modifier.
     *
     * \return The sort modifier.
     *
     * \sa setSortModifier()
     */
    HSortModifier sortModifier() const;

    /*!
     * \brief Specifies the CDS property to which this sort criteria applies.
     *
     * \param arg specifies the CDS property to which this sort criteria applies.
     *
     * \sa property()
     */
    void setProperty(const QString& arg);

    /*!
     * \brief Specifies the sort modifier.
     *
     * \param arg specifies the sort modifier.
     *
     * \sa sortModifier()
     */
    void setSortModifier(const HSortModifier& arg);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HSortInfo
 */
H_UPNP_AV_EXPORT bool operator==(const HSortInfo& obj1, const HSortInfo& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HSortInfo
 */
inline bool operator!=(const HSortInfo& obj1, const HSortInfo& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* HSORTINFO_H_ */
