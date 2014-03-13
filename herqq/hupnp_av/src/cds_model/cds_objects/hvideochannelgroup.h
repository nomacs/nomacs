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

#ifndef HVIDEOCHANNELGROUP_H_
#define HVIDEOCHANNELGROUP_H_

#include <HUpnpAv/HChannelGroup>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HVideoChannelGroupPrivate;

/*!
 * \brief This class is used to group together a set of items that correspond
 * to individual but related video broadcast channels.
 *
 * The class identifier specified by the AV Working Committee is
 * \c object.container.channelGroup.videoChannelGroup.
 *
 * \headerfile hvideochannelgroup.h HVideoChannelGroup
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HVideoChannelGroup :
    public HChannelGroup
{
Q_OBJECT
H_DISABLE_COPY(HVideoChannelGroup)
H_DECLARE_PRIVATE(HVideoChannelGroup)

protected:

    /*!
     * Constructs a new instance.
     *
     * \param clazz specifies the class identifier.
     *
     * \param cdsType specifies CDS type.
     */
    HVideoChannelGroup(const QString& clazz = sClass(), CdsType cdsType = sType());
    HVideoChannelGroup(HVideoChannelGroupPrivate&);

    // Documented in HClonable
    virtual HVideoChannelGroup* newInstance() const;

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
    HVideoChannelGroup(
        const QString& title,
        const QString& parentId,
        const QString& id = QString());

   /*!
     * \brief Destroys the instance.
     */
    virtual ~HVideoChannelGroup();

    /*!
     * \brief Returns the CdsType value of this class.
     *
     * \return The CdsType value of this class.
     */
    inline static CdsType sType() { return VideoChannelGroup; }

    /*!
     * \brief Returns the class identifier specified by the AV Working Committee.
     *
     * \return The class identifier specified by the AV Working Committee.
     */
    inline static QString sClass() { return "object.container.channelGroup.videoChannelGroup"; }

    /*!
     * Creates a new instance with no title or parentID.
     *
     * \return a pointer to the newly created instance.
     *
     * \remarks the ownership of the object is transferred to the caller. Make sure
     * to delete the object.
     */
    inline static HVideoChannelGroup* create() { return new HVideoChannelGroup(); }
};

}
}
}

#endif /* HVIDEOCHANNELGROUP_H_ */
