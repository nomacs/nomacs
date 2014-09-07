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

#ifndef HMEDIASERVER_INFO_H_
#define HMEDIASERVER_INFO_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/HUpnp>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This class provides general information of a UPnP Media Server and
 * information related to the operations of MediaServer
 * implementations provided by HUPnPAv.
 *
 * \headerfile hmediaserver_info.h HMediaServerInfo
 *
 * \ingroup hupnp_av_mediaserver
 *
 * \remarks This class is thread-safe.
 */
class H_UPNP_AV_EXPORT HMediaServerInfo
{
H_FORCE_SINGLETON(HMediaServerInfo)

public:

    /*!
     * \brief Returns the type of the latest MediaServer specification the
     * HUPnPAv implementations support.
     *
     * \return The type of the latest MediaServer specification the
     * HUPnPAv implementations support.
     */
    static const HResourceType& supportedDeviceType();

    /*!
     * \brief Returns information about the services specified up until the
     * MediaServer specification supported by HUPnPAv implementations.
     *
     * \return information about the services specified up until the
     * MediaServer specification supported by HUPnPAv implementations.
     */
    static HServicesSetupData servicesSetupData();

    /*!
     * \brief Returns the default UPnP Service ID for the Content Directory, when it is
     * included in a standard UPnP Media Server.
     *
     * \return The default UPnP Service ID for the Content Directory, when it is
     * included in a standard UPnP Media Server.
     */
    static const HServiceId& defaultContentDirectoryId();

    /*!
     * \brief Returns the default UPnP Service ID for the Connection Manager, when it is
     * included in a standard UPnP Media Server.
     *
     * \return The default UPnP Service ID for the Connection Manager, when it is
     * included in a standard UPnP Media Server.
     */
    static const HServiceId& defaultConnectionManagerId();

    /*!
     * \brief Returns the default UPnP Service ID for the AV Transport, when it is
     * included in a standard UPnP Media Server.
     *
     * \return The default UPnP Service ID for the AV Transport, when it is
     * included in a standard UPnP Media Server.
     */
    static const HServiceId& defaultAvTransportId();
};

}
}
}

#endif /* HMEDIASERVER_INFO_H_ */
