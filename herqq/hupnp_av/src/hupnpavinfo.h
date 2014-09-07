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

#ifndef HUPNPAVINFO_H_
#define HUPNPAVINFO_H_

#include <HUpnpAv/HUpnpAv>

/*!
 * \file
 * This file contains information of the HUPnPAv library.
 */

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * This is the major version of the HUPnPAv library with which this header
 * file was provided.
 *
 * This value is useful for compile time checks.
 *
 * \sa Herqq::Upnp::Av::hupnpAvVersion()
 */
#define HUPNP_AV_MAJOR_VERSION 0

/*!
 * This is the minor version of the HUPnPAv library with which this header
 * file was provided.
 *
 * This value is useful for compile time checks.
 *
 * \sa Herqq::Upnp::Av::hupnpAvVersion()
 */
#define HUPNP_AV_MINOR_VERSION 0

/*!
 * This is the patch version of the HUPnPAv library with which this header
 * file was provided.
 *
 * This value is useful for compile time checks.
 *
 * \sa Herqq::Upnp::Av::hupnpAvVersion()
 */
#define HUPNP_AV_PATCH_VERSION 2

/*!
 * This is the version of the HUPnPAv library with which this header
 * file was provided.
 *
 * This value is useful for compile time checks.
 *
 * \sa Herqq::Upnp::Av::hupnpAvVersion()
 */
#define HUPNP_AV_VERSION STRX(HUPNP_AV_MAJOR_VERSION.HUPNP_AV_MINOR_VERSION.HUPNP_AV_PATCH_VERSION)

/*!
 * \brief Returns the runtime version of the HUPnPAv library as a string.
 *
 * You can use this function to query the version of the HUPnPAv library
 * at runtime.
 *
 * For compile time checks you may use the macros:
 * - HUPNP_AV_MAJOR_VERSION,
 * - HUPNP_AV_MINOR_VERSION,
 * - HUPNP_AV_PATCH_VERSION and
 * - HUPNP_AV_VERSION.
 *
 * \return The runtime version of the HUPnPAv library as a string.
 * The format is <c>major.minor.patch</c>.
 *
 * \remarks The returned string is statically allocated. You should never
 * delete or free it manually.
 */
H_UPNP_AV_EXPORT const char* hupnpAvVersion();

}
}
}

#endif /* HUPNPAVINFO_H_ */
