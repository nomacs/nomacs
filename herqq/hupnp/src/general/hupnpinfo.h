/*
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP (HUPnP) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HUPNPINFO_H_
#define HUPNPINFO_H_

#include <HUpnpCore/HUpnp>

/*!
 * \file
 * This file contains information of the HUPnP core library.
 */

namespace Herqq
{

namespace Upnp
{

/*!
 * This is the major version of the HUPnP Core library with which this header
 * file was provided.
 *
 * This value is useful for compile time checks.
 *
 * \sa Herqq::Upnp::hupnpCoreVersion()
 */
#define HUPNP_CORE_MAJOR_VERSION 2

/*!
 * This is the minor version of the HUPnP Core library with which this header
 * file was provided.
 *
 * This value is useful for compile time checks.
 *
 * \sa Herqq::Upnp::hupnpCoreVersion()
 */
#define HUPNP_CORE_MINOR_VERSION 0

/*!
 * This is the patch version of the HUPnP Core library with which this header
 * file was provided.
 *
 * This value is useful for compile time checks.
 *
 * \sa Herqq::Upnp::hupnpCoreVersion()
 */
#define HUPNP_CORE_PATCH_VERSION 0

/*!
 * This is the version of the HUPnP Core library with which this header
 * file was provided.
 *
 * This value is useful for compile time checks.
 *
 * \sa Herqq::Upnp::hupnpCoreVersion()
 */
#define HUPNP_CORE_VERSION STRX(HUPNP_CORE_MAJOR_VERSION.HUPNP_CORE_MINOR_VERSION.HUPNP_CORE_PATCH_VERSION)

/*!
 * \brief Returns the runtime version of the HUPnP Core library as a string.
 *
 * You can use this function to query the version of the HUPnP core library
 * at runtime.
 *
 * For compile time checks you may use the macros:
 * - HUPNP_CORE_MAJOR_VERSION,
 * - HUPNP_CORE_MINOR_VERSION,
 * - HUPNP_CORE_PATCH_VERSION and
 * - HUPNP_CORE_VERSION.
 *
 * \return The runtime version of the HUPnP Core library as a string.
 * The format is <c>major.minor.patch</c>.
 *
 * \remarks The returned string is statically allocated. You should never
 * delete or free it manually.
 */
H_UPNP_CORE_EXPORT const char* hupnpCoreVersion();

}
}

#endif /* HUPNPINFO_H_ */
