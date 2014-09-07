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

#ifndef HRENDERINGCONTROL_INFO_H_
#define HRENDERINGCONTROL_INFO_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/HUpnp>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This class provides general information of a UPnP A/V RenderingControl and
 * information related to the operations of RenderingControl implementations
 * provided by HUPnPAv.
 *
 * \headerfile hrenderingcontrol_info.h HRenderingControlInfo
 *
 * \ingroup hupnp_av_rcs
 *
 * \remarks This class is thread-safe.
 */
class H_UPNP_AV_EXPORT HRenderingControlInfo
{
H_FORCE_SINGLETON(HRenderingControlInfo)

public:

    /*!
     * \brief This enumeration describes the error codes that have been
     * defined by the UPnP forum to the RenderingControl:2 service.
     *
     * \note the descriptions are taken from the RenderingControl:2 specification.
     */
    enum ErrorCode
    {
        /*!
         * The specified name is not a valid preset name.
         */
        InvalidName = 701,

        /*!
         * The specified instance ID is invalid.
         */
        InvalidInstanceId = 702,

        /*!
         * The specified channel is invalid.
         */
        InvalidChannel = 703,

        /*!
         * One or more of the specified state variables are invalid.
         */
        InvalidStateVariableList = 704,

        /*!
         * The specified CSV list is invalid.
         */
        IllFormedCsvList = 705,

        /*!
         * The value of a state variable value pair is invalid.
         */
        InvalidStateVariableValue = 706,

        /*!
         * The specified MediaRenderer UDN is invalid.
         */
        InvalidMediaRendererUdn = 707,

        /*!
         * The specified service type is invalid.
         */
        InvalidServiceType = 708,

        /*!
         * The specified service ID is invalid.
         */
        InvalidServiceId = 709,

        /*!
         * The specified list of state variable value pairs includes variables
         * that are not allowed to be set.
         */
        StateVariablesSpecifiedImproperly = 710
    };

    /*!
     * \brief Returns the type of the latest RenderingControl specification the
     * HUPnPAv implementations support.
     *
     * \return The type of the latest RenderingControl specification the
     * HUPnPAv implementations support.
     */
    static const HResourceType& supportedServiceType();

    /*!
     * \brief Returns information about the actions specified up until the
     * RenderingControl specification supported by HUPnPAv implementations.
     *
     * \sa supportedServiceType(), stateVariablesSetupData()
     */
    static HActionsSetupData actionsSetupData();

    /*!
     * \brief Returns information about the state variables specified up until the
     * RenderingControl specification supported by HUPnPAv implementations.
     *
     * \sa supportedServiceType(), actionsSetupData()
     */
    static HStateVariablesSetupData stateVariablesSetupData();
};

}
}
}

#endif /* HRENDERINGCONTROL_INFO_H_ */
