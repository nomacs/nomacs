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

#ifndef HCONNECTIONMANAGER_SERVICECONFIGURATION_H_
#define HCONNECTIONMANAGER_SERVICECONFIGURATION_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/HClonable>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HConnectionManagerServiceConfigurationPrivate;

/*!
 * \brief This class contains the configuration data passed to a HConnectionManagerService.
 *
 * \headerfile hconnectionmanager_serviceconfiguration.h HConnectionManagerServiceConfiguration
 *
 * \ingroup hupnp_av_cm
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HConnectionManagerService
 */
class H_UPNP_AV_EXPORT HConnectionManagerServiceConfiguration :
    public HClonable
{
H_DISABLE_COPY(HConnectionManagerServiceConfiguration)
H_DECLARE_PRIVATE(HConnectionManagerServiceConfiguration)

protected:

    HConnectionManagerServiceConfigurationPrivate* h_ptr;
    HConnectionManagerServiceConfiguration(HConnectionManagerServiceConfigurationPrivate& dd);

    // Documented in HClonable
    virtual void doClone(HClonable* target) const;
    // Documented in HClonable
    virtual HConnectionManagerServiceConfiguration* newInstance() const;

public:

    /*!
     * \brief Creates a new instance.
     */
    HConnectionManagerServiceConfiguration();

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HConnectionManagerServiceConfiguration();

    // Documented in HClonable
    virtual HConnectionManagerServiceConfiguration* clone() const;

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true if the object is valid.
     */
    virtual bool isValid() const;

    /*!
     *
     */
    void setSupportedContentInfo(const HProtocolInfos&);

    /*!
     *
     */
    const HProtocolInfos& supportedContentInfo() const;
};

}
}
}

#endif /* HCONTENTDIRECTORY_SERVICECONFIGURATION_H_ */
