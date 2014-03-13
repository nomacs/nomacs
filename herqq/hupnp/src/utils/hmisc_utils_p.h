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

#ifndef HMISC_UTILS_P_H_
#define HMISC_UTILS_P_H_

#include <HUpnpCore/HUpnp>

class QHostAddress;

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

namespace Herqq
{

/* djb2
 *
 * This algorithm was first reported by Dan Bernstein
 * many years ago in comp.lang.c
 */
H_UPNP_CORE_EXPORT unsigned long hash(const char* str, int n);

H_UPNP_CORE_EXPORT QHostAddress findBindableHostAddress();

H_UPNP_CORE_EXPORT bool toBool(const QString&, bool* ok);

}

#endif /* HMISC_UTILS_P_H_ */
