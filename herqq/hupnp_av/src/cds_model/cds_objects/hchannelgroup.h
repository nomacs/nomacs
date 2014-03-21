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

#ifndef HCHANNELGROUP_H_
#define HCHANNELGROUP_H_

#include <HUpnpAv/HContainer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HChannelGroupPrivate;

/*!
 * \brief This class is used to group together a set of items that correspond to
 * individual but related broadcast channels.
 *
 * The class identifier specified by the AV Working Committee is
 * \c object.container.channelGroup.
 *
 * \headerfile hchannelgroup.h HChannelGroup
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HChannelGroup :
    public HContainer
{
Q_OBJECT
H_DISABLE_COPY(HChannelGroup)
H_DECLARE_PRIVATE(HChannelGroup)

protected:

    /*!
     * Constructs a new instance.
     *
     * \param clazz specifies the UPnP class of the object. This cannot be empty.
     *
     * \param cdsType specifies the CDS type of the object. This cannot be
     * HObject::UndefinedCdsType.
     *
     * \sa isInitialized()
     */
    HChannelGroup(const QString& clazz = sClass(), CdsType cdsType = sType());
    HChannelGroup(HChannelGroupPrivate&);

    // Documented in HClonable
    virtual HChannelGroup* newInstance() const;

public:

    /*!
     * \brief Creates a new instance.
     *
     * \param title specifies the title of the object.
     *
     * \param parentId specifies the ID of the object that contains this
     * object. If the object has no parent, this has to be left empty.
     *
     * \param id specifies the ID of this object. If this is not specified,
     * a unique identifier within the running process is created for the object.
     *
     * \sa isValid()
     */
    HChannelGroup(
        const QString& title,
        const QString& parentId,
        const QString& id = QString());

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HChannelGroup();

    /*!
     * \brief Returns the user friendly name of the channel group.
     *
     * upnp:channelGroupName, ContentDirectory:3, Appendix B.9.1.
     *
     * \return The user friendly name of the channel group.
     *
     * \sa setChannelGroupName()
     */
    HChannelGroupName channelGroupName() const;

    /*!
     * \brief Returns the name of the Electronic Program Guide service provider.
     *
     * upnp:epgProviderName, ContentDirectory:3, Appendix B.9.7.
     *
     * \return The name of the Electronic Program Guide service provider.
     *
     * \sa setEpgProviderName()
     */
    QString epgProviderName() const;

    /*!
     * \brief Returns the user friendly name of the service provider of this content.
     *
     * upnp:serviceProvider, ContentDirectory:3, Appendix B.9.4.
     *
     * \return The user friendly name of the service provider of this content.
     *
     * \sa setServiceProvider()
     */
    QString serviceProvider() const;

    /*!
     * \brief Returns a location to an icon that can be used as a descriptive short-cut
     * to display the content.
     *
     * upnp:icon, ContentDirectory:3, Appendix B.7.3.
     *
     * \return a location to an icon that can be used as a descriptive short-cut
     * to display the content.
     *
     * \sa setIcon()
     */
    QUrl icon() const;

    /*!
     * \brief Returns some information of the region, associated with the source of the
     * object.
     *
     * upnp:region, ContentDirectory, Appendix B.7.4.
     *
     * \return some information of the region, associated with the source of the
     * object.
     *
     * \sa setRegion()
     */
    QString region() const;

    /*!
     * \brief Specifies the user friendly name of the channel group.
     *
     * \param arg specifies the user friendly name of the channel group.
     *
     * \sa channelGroupName()
     */
    void setChannelGroupName(const HChannelGroupName& arg);

    /*!
     * \brief Specifies the name of the Electronic Program Guide service provider.
     *
     * \param arg specifies the name of the Electronic Program Guide service provider.
     *
     * \sa epgProviderName()
     */
    void setEpgProviderName(const QString& arg);

    /*!
     * \brief Specifies the user friendly name of the service provider of this content.
     *
     * \param arg specifies the user friendly name of the service provider of this content.
     *
     * \sa serviceProvider()
     */
    void setServiceProvider(const QString& arg);

    /*!
     * \brief Specifies a location to an icon that can be used as a descriptive short-cut
     * to display the content.
     *
     * \param arg specifies a location to an icon that can be used as a descriptive short-cut
     * to display the content.
     *
     * \sa icon()
     */
    void setIcon(const QUrl& arg);

    /*!
     * \brief Specifies some information of the region, associated with the source of the
     * object.
     *
     * \param arg specifies some information of the region, associated with the source of the
     * object.
     *
     * \sa region()
     */
    void setRegion(const QString& arg);

    /*!
     * \brief Returns the CdsType value of this class.
     *
     * \return The CdsType value of this class.
     */
    inline static CdsType sType() { return ChannelGroup; }

    /*!
     * \brief Returns the class identifier specified by the AV Working Committee.
     *
     * \return The class identifier specified by the AV Working Committee.
     */
    inline static QString sClass() { return "object.container.channelGroup"; }

    /*!
     * Creates a new instance with no title or parentID.
     *
     * \return a pointer to the newly created instance.
     *
     * \remarks the ownership of the object is transferred to the caller. Make sure
     * to delete the object.
     */
    inline static HChannelGroup* create() { return new HChannelGroup(); }
};

}
}
}

#endif /* HCHANNELGROUP_H_ */
