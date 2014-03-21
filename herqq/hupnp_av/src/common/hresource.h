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

#ifndef HRESOURCE_H_
#define HRESOURCE_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>

template<typename T, typename U>
class QHash;

class QUrl;
class QString;

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HResourcePrivate;

/*!
 * \brief This class depicts resource information associated with a \e CDS \e item.
 *
 * \headerfile hresource.h HResource
 *
 * \ingroup hupnp_av_common
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HResource
{
friend H_UPNP_AV_EXPORT bool operator==(const HResource&, const HResource&);

private: // attributes

    QSharedDataPointer<HResourcePrivate> h_ptr;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HResource();

    /*!
     * \brief Creates a new instance.
     *
     * \param protocolInfo specifies the Protocol Info of the object.
     *
     * \sa isValid()
     */
    HResource(const HProtocolInfo& protocolInfo);

    /*!
     * \brief Creates a new instance.
     *
     * \param location specifies the URL of the referenced object.
     *
     * \param protocolInfo specifies the Protocol Info of the object.
     *
     * \sa isValid()
     */
    HResource(const QUrl& location, const HProtocolInfo& protocolInfo);

    /*!
     * \brief Destroys the instance.
     */
    ~HResource();

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HResource(const HResource&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HResource& operator=(const HResource&);

    /*!
     * \brief Returns the URL of the referenced object.
     *
     * \return The URL of the referenced object. This is where the object
     * can be retrieved.
     *
     * \remarks This may not be set.
     */
    QUrl location() const;

    /*!
     * \brief Returns the protocol information.
     *
     * \return The protocol information.
     *
     * \sa setProtocolInfo()
     */
    const HProtocolInfo& protocolInfo() const;

    /*!
     * \brief Returns the media information associated with the resource.
     *
     * \return The media information associated with the resource.
     *
     * \sa setMediaInfo()
     */
    const QHash<QString, QString>& mediaInfo() const;

    /*!
     * \brief Returns the number of times a change was made to the content
     * referenced by this resource.
     *
     * \return The number of times a change was made to the content
     * referenced by this resource.
     *
     * \sa setUpdateCount()
     */
    quint32 updateCount() const;

    /*!
     * \brief Indicates if the TCO (Track Changes Option) is enabled for this resource.
     *
     * \return \e true if the TCO (Track Changes Option) is enabled for this resource.
     *
     * \sa enableTrackChangesOption()
     */
    bool trackChangesOptionEnabled() const;

    /*!
     * \brief Sets the URL of the referenced object.
     *
     * \param arg specifies the URL of the referenced object.
     *
     * \sa location()
     */
    void setLocation(const QUrl& arg);

    /*!
     * \brief Sets the protocol info.
     *
     * \param arg specifies the ProtocolInfo.
     *
     * \sa protocolInfo()
     */
    void setProtocolInfo(const HProtocolInfo& arg);

    /*!
     * \brief Specifies the media information associated with the resource.
     *
     * \param arg specifies the media information associated with the resource.
     *
     * \sa mediaInfo()
     */
    void setMediaInfo(const QHash<QString, QString>& arg);

    /*!
     * \brief Specifies the number of times a change was made to the content
     * referenced by this resource.
     *
     * \param arg specifies the number of times a change was made to the content
     * referenced by this resource.
     *
     * \sa updateCount()
     */
    void setUpdateCount(quint32 arg);

    /*!
     * \brief Specifies whether the TCO (Track Changes Option) is enabled for
     * this resource.
     *
     * \param arg specifies whether the TCO (Track Changes Option) is enabled
     * for this resource.
     *
     * \sa trackChangesOptionEnabled()
     */
    void enableTrackChangesOption(bool arg);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the object are logically equivalent.
 *
 * \relates HResource
 */
H_UPNP_AV_EXPORT bool operator==(const HResource&, const HResource&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HResource
 */
inline bool operator!=(const HResource& obj1, const HResource& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

Q_DECLARE_METATYPE(Herqq::Upnp::Av::HResource)

#endif /* HRESOURCE_H_ */
