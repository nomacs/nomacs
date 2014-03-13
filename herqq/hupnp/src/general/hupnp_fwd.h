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

#ifndef HUPNPFWD_H_
#define HUPNPFWD_H_

class QString;

template<typename T>
class QList;

template<typename T, typename U>
class QHash;

namespace Herqq
{

namespace Upnp
{

/*!
 * \file
 * This file contains forward-declarations to every public class HUPnP exposes
 * and a few common type definitions.
 */

class HClientAction;
class HClientDevice;
class HClientService;
class HClientActionOp;
class HClientStateVariable;

struct HNullValue;

template<typename T>
class HClientAdapterOp;

typedef HClientAdapterOp<HNullValue> HClientAdapterOpNull;

class HClientDeviceAdapter;
class HClientServiceAdapter;

class HServerAction;
class HServerDevice;
class HServerService;
class HServerStateVariable;

class HActionSetup;
class HDeviceSetup;
class HServiceSetup;
class HDevicesSetupData;
class HActionsSetupData;
class HServicesSetupData;
class HServicesSetupData;
class HStateVariablesSetupData;

class HActionInfo;
class HDeviceInfo;
class HServiceInfo;
class HActionArgument;
class HActionArguments;
class HStateVariableInfo;
class HStateVariableEvent;

class HUdn;
class HEndpoint;
class HServiceId;
class HResourceType;
class HProductToken;
class HDeviceStatus;
class HProductTokens;
class HDiscoveryType;

class HSsdp;
class HResourceUpdate;
class HDiscoveryRequest;
class HDiscoveryResponse;
class HResourceAvailable;
class HResourceUnavailable;

class HDeviceModelCreator;
class HDeviceModelValidator;
class HDeviceModelInfoProvider;

class HAsyncOp;
class HExecArgs;
class HControlPoint;
class HControlPointConfiguration;

class HDeviceHost;
class HDeviceConfiguration;
class HDeviceHostConfiguration;
class HDeviceHostRuntimeStatus;

/*!
 * This is a type definition to a list of Herqq::Upnp::HEndpoint instances.
 *
 * \ingroup hupnp_common
 *
 * \sa HEndpoint
 */
typedef QList<HEndpoint> HEndpoints;

/*!
 * This is a type definition to a list of pointers to
 * Herqq::Upnp::HClientService instances.
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HClientService
 */
typedef QList<HClientService*> HClientServices;

/*!
 * This is a type definition to a list of pointers to
 * Herqq::Upnp::HServerService instances.
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HServerService
 */
typedef QList<HServerService*> HServerServices;

/*!
 * This is a type definition to a list of pointers to
 * Herqq::Upnp::HClientDevice instances.
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HClientDevice
 */
typedef QList<HClientDevice*> HClientDevices;

/*!
 * This is a type definition to a list of pointers to
 * Herqq::Upnp::HServerDevice instances.
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HServerDevice
 */
typedef QList<HServerDevice*> HServerDevices;

/*!
 * This is a type definition to a hash table of pointers to
 * const Herqq::Upnp::HServerStateVariable instances keyed with the
 * state variable names.
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HClientStateVariable
 */
typedef QHash<QString, const HClientStateVariable*> HClientStateVariables;

/*!
 * This is a type definition to a hash table of pointers to
 * Herqq::Upnp::HServerStateVariable instances keyed with the state variable names.
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HServerStateVariable
 */
typedef QHash<QString, HServerStateVariable*> HServerStateVariables;

/*!
 * This is a type definition to a hash table of
 * Herqq::Upnp::HStateVariableInfo instances keyed with the state variable names.
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HStateVariableInfo
 */
typedef QHash<QString, HStateVariableInfo> HStateVariableInfos;

/*!
 * This is a type definition to a hash table of pointers to
 * Herqq::Upnp::HClientAction instances keyed with the action names.
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HClientAction
 */
typedef QHash<QString, HClientAction*> HClientActions;

/*!
 * This is a type definition to a hash table of pointers to
 * Herqq::Upnp::HServerAction instances keyed with the action names.
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HServerAction
 */
typedef QHash<QString, HServerAction*> HServerActions;

}
}

#endif /* HUPNPFWD_H_ */
