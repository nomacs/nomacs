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

#ifndef HCDSCLASSINFO_H_
#define HCDSCLASSINFO_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HCdsClassInfoPrivate;

/*!
 * \brief This class specifies information of a CDS class needed within some of
 * the operations of HContainer.
 *
 * \headerfile hcdsclassinfo.h HCdsClassInfo
 *
 * \ingroup hupnp_av_cds_common
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HContainer::searchClassInfos(), HContainer::createClassInfos()
 */
class H_UPNP_AV_EXPORT HCdsClassInfo
{
friend H_UPNP_AV_EXPORT bool operator==(const HCdsClassInfo&, const HCdsClassInfo&);

private: // attributes

    QSharedDataPointer<HCdsClassInfoPrivate> h_ptr;

public:

    /*!
     * \brief Creates a new, empty instance.
     *
     * \sa isValid()
     */
    HCdsClassInfo();

    /*!
     * \brief Creates a new instance.
     *
     * \param className specifies the class name (type).
     *
     * \sa isValid()
     */
    HCdsClassInfo(const QString& className);

    /*!
     * \brief Creates a new instance.
     *
     * \param className specifies the UPnP CDS class name (type), such as
     * \c object.item.photo.
     *
     * \param includeDerived specifies whether classes that are derived from the
     * specified \a className should be considered. That is, if this parameter
     * is set to \e true, the operation interpreting this information must
     * consider the specified UPnP CDS class type specified by \a className
     * \b AND types that are derived from it. Otherwise only the UPnP CDS
     * class specified should be considered.
     *
     * \param name specifies a friendly name of the class.
     *
     * \sa isValid()
     */
    HCdsClassInfo(
        const QString& className, bool includeDerived, const QString& name);

    /*!
     * \brief Destroys the instance.
     */
    ~HCdsClassInfo();

    /*!
     * \brief Copy constructor.
     *
     * Creates a new instance of other.
     */
    HCdsClassInfo(const HCdsClassInfo&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of other to this.
     */
    HCdsClassInfo& operator=(const HCdsClassInfo&);

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true when the object is valid, i.e. the className() is
     * defined.
     */
    bool isValid() const;

    /*!
     * \brief Returns the class name (type).
     *
     * \return The UPnP CDS class name (type), such as \c object.item.photo.
     *
     * \sa setClassName()
     */
    QString className() const;

    /*!
     * \brief Returns the friendly name.
     *
     * \return The friendly name.
     *
     * \sa setName()
     */
    QString name() const;

    /*!
     * \brief Indicates whether the classes that are derived from the specified className()
     * should be considered as a part of the operation.
     *
     * \return \e true if the classes that are derived from the specified className()
     * should be considered as a part of the operation.
     *
     * \sa setIncludeDerived()
     */
    bool includeDerived() const;

    /*!
     * \brief Sets the friendly name.
     *
     * \param arg specifies the friendly name.
     *
     * \sa name()
     */
    void setName(const QString& arg);

    /*!
     * \brief Specifies whether the classes that are derived from the specified className()
     * should be considered as a part of the operation.
     *
     * \param arg specifies whether the classes that are derived from the
     * specified className() should be considered as a part of the operation.
     *
     * \sa includeDerived()
     */
    void setIncludeDerived(bool arg);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HCdsClassInfo
 */
H_UPNP_AV_EXPORT bool operator==(const HCdsClassInfo&, const HCdsClassInfo&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HCdsClassInfo
 */
inline bool operator!=(const HCdsClassInfo& obj1, const HCdsClassInfo& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

Q_DECLARE_METATYPE(Herqq::Upnp::Av::HCdsClassInfo)

#endif /* HCDSCLASSINFO_H_ */
