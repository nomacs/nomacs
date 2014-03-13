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

#ifndef HMEDIARENDERER_INFO_H_
#define HMEDIARENDERER_INFO_H_

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
 * information related to the operations of MediaRenderer
 * implementations provided by HUPnPAv.
 *
 * \headerfile hmediarenderer_info.h HMediaRendererInfo
 *
 * \ingroup hupnp_av_mediarenderer
 *
 * \remarks This class is thread-safe.
 */
class H_UPNP_AV_EXPORT HMediaRendererInfo
{
H_FORCE_SINGLETON(HMediaRendererInfo)

public:

    /*!
     * \brief Returns the type of the latest MediaRenderer specification the
     * HUPnPAv implementations support.
     *
     * \return The type of the latest MediaRenderer specification the
     * HUPnPAv implementations support.
     */
    static const HResourceType& supportedDeviceType();

    /*!
     * \brief Returns information about the services specified up until the
     * MediaRenderer specification supported by HUPnPAv implementations.
     *
     * \return information about the services specified up until the
     * MediaRenderer specification supported by HUPnPAv implementations.
     */
    static HServicesSetupData servicesSetupData();

    /*!
     * \brief Returns the default UPnP Service ID for the ConnectionManager, when it is
     * included in a standard UPnP Media Renderer.
     *
     * \return The default UPnP Service ID for the ConnectionManager, when it is
     * included in a standard UPnP Media Renderer.
     */
    static const HServiceId& defaultConnectionManagerId();

    /*!
     * \brief Returns the default UPnP Service ID for the RenderingControl, when it is
     * included in a standard UPnP Media Renderer.
     *
     * \return The default UPnP Service ID for the RenderingControl, when it is
     * included in a standard UPnP Media Renderer.
     */
    static const HServiceId& defaultRenderingControlId();

    /*!
     * \brief Returns the default UPnP Service ID for the AV Transport, when it is
     * included in a standard UPnP Media Renderer.
     *
     * \return The default UPnP Service ID for the AV Transport, when it is
     * included in a standard UPnP Media Renderer.
     */
    static const HServiceId& defaultAvTransportId();
};

}
}
}

#endif /* HMEDIARENDERER_INFO_H_ */
