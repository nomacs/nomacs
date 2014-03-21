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

#ifndef HUPNP_GLOBAL_H_
#define HUPNP_GLOBAL_H_

#include <HUpnpCore/public/hupnp_fwd.h>
#include <HUpnpCore/public/hupnp_defs.h>

/*!
 * \file
 * This file contains public functions and enumerations.
 */

namespace Herqq
{

namespace Upnp
{

/*!
 * \brief This enumeration specifies the generic error codes that UPnP action invocation
 * may return.
 *
 * These values correspond to the values defined in the UDA, excluding
 * \c NotImplemented and \c UndefinedFailure, which are defined for the purposes
 * of HUPnP.
 *
 * \note These are only the generic error codes. Many UPnP devices define
 * and use domain specific error codes that cannot be specified here.
 */
enum UpnpErrorCode
{
    /*!
     * \brief Action invocation succeeded.
     *
     * Action invocation succeeded.
     */
    UpnpSuccess = 200,

    /*!
     * Invalid action.
     *
     * The specified action was not found.
     */
    UpnpInvalidAction = 401,

    /*!
     * Action invocation failed due to:
     * \li not enough arguments,
     * \li arguments in wrong order,
     * \li one or more arguments have wrong data type
     */
    UpnpInvalidArgs = 402,

    /*!
     * \brief The current state of the service prevents the action invocation.
     *
     * The current state of the service prevents the action invocation.
     */
    UpnpActionFailed = 501,

    /*!
     * \brief Action invocation failed due to an invalid argument value.
     *
     * Action invocation failed due to an invalid argument value.
     */
    UpnpArgumentValueInvalid = 600,

    /*!
     * Action invocation failed due to:
     * \li an argument value is less than the minimum of the allowed value range,
     * \li an argument value is more than the maximum of the allowed value range,
     * \li an argument value is not in the allowed value list
     */
    UpnpArgumentValueOutOfRange = 601,

    /*!
     * Action invocation failed due to the requested action being optional
     * and not implemented by the device.
     */
    UpnpOptionalActionNotImplemented = 602,

    /*!
     * Action invocation failed due to insufficient memory.
     *
     * The device does not have sufficient memory available to complete the action.
     * This MAY be a temporary condition; the control point MAY choose to retry the
     * unmodified request again later and it MAY succeed if memory is available.
     */
    UpnpOutOfMemory = 603,

    /*!
     * The device has encountered an error condition which it cannot resolve itself
     * and required human intervention such as a reset or power cycle. See the device
     * display or documentation for further guidance.
     */
    UpnpHumanInterventionRequired = 604,

    /*!
     * Action invocation failed due to a string argument being
     * too long for the device to handle properly.
     */
    UpnpStringArgumentTooLong = 605,

    /*!
     * The action invocation is in progress.
     *
     * \remarks
     * This value is defined and used by HUPnP in-process only.
     */
    UpnpInvocationInProgress = 0x00f00000,

    /*!
     * \brief Action invocation failed, but the exact cause could not be determined.
     *
     * Action invocation failed, but the exact cause could not be determined.
     *
     * \remarks
     * This value is defined and used by HUPnP in-process only.
     */
    UpnpUndefinedFailure = 0x0ff00000,

    /*!
     * The action invocation was aborted by user.
     *
     * \remarks
     * This value is defined and used by HUPnP in-process only.
     */
    UpnpInvocationAborted = 0x00f00001
};

/*!
 * \brief Returns a string representation of the specified error code.
 *
 * \param errCode specififes the error code.
 *
 * \return a string representation of the specified error code.
 */
QString H_UPNP_CORE_EXPORT upnpErrorCodeToString(qint32 errCode);

/*!
 * \brief This enumeration specifies how a device tree should be traversed given a
 * starting node.
 *
 * HUPnP \ref hupnp_devicemodel is organized into a tree that has a root
 * device, which may contain embedded devices as its children and they may contain
 * embedded devices as their children recursively.
 *
 * \brief This enumeration is used to specify how a device and its children are traversed.
 */
enum DeviceVisitType
{
    /*!
     * This value is used to indicate that only the device in question is visited.
     */
    VisitThisOnly = 0,

    /*!
     * This value is used to indicate that this device and its embedded devices
     * are visited.
     */
    VisitThisAndDirectChildren,

    /*!
     * This value is used to indicate that this device and all of its child
     * devices are visited recursively.
     */
    VisitThisRecursively
};

/*!
 * \brief This enumeration specifies the device types that are considered as
 * \e targets of an operation.
 */
enum TargetDeviceType
{
    /*!
     * This value is used to indicate that \b all devices, both root and
     * embedded are the targets of an operation.
     */
    AllDevices,

    /*!
     * This value is used to indicate that \b only embedded devices are the
     * targets of an operation.
     */
    EmbeddedDevices,

    /*!
     * This value is used to indicate that \b only root devices are the
     * targets of an operation.
     */
    RootDevices
};

/*!
 * \brief This enumeration specifies the type of a device location URL.
 */
enum LocationUrlType
{
    /*!
     * The absolute URL for the device description.
     */
    AbsoluteUrl,

    /*!
     * The base URL of the device. This is the URL with which the various
     * other URLs found in a device description are resolved.
     */
    BaseUrl
};

/*!
 * \brief This enumeration is used to specify the strictness of argument validation.
 *
 * \ingroup hupnp_common
 */
enum HValidityCheckLevel
{
    /*!
     * The arguments are validated strictly according to the UDA
     * v1.0 and v1.1 specifications.
     */
    StrictChecks,

    /*!
     * The validation allows slight deviations from the UDA specifications
     * in an attempt to improve interoperability. The accepted exceptions
     * have been encountered in other UPnP software that are popular enough
     * to warrant the exceptional behavior.
     */
    LooseChecks
};

/*!
 * \brief This enumeration specifies whether a component of the \ref hupnp_devicemodel is
 * mandatory within a specific UPnP device.
 *
 * In more detail, any component of the device model
 * (a device, a service, a state variable or an action) may be specified as
 * a mandatory or an optional part of a UPnP device; for example,
 * a UPnP device may have two mandatory embedded devices and one
 * optional embedded device. The same applies to the other components as well.
 *
 * When HUPnP builds an object model of a UPnP device, this information can be
 * used in validating a description document, or verifying that the provided
 * device tree accurately depicts a description document.
 *
 * For instance, if the author of a subclass of a HServerService has
 * specified that a particular action is mandatory, the user of the class,
 * who is the one that provides the description document, has to make sure that
 * the description document also contains the definition of the action.
 *
 * These types of mappings are optional, but they are highly useful in case
 * the component is to be used as a public part of a library.
 * They help to ensure that the implementation back-end reflects the used
 * description documents appropriately. This is important, as it is the
 * description documents that are transferred from servers to clients and it is
 * these documents that advertise what a particular UPnP device supports and
 * is capable of doing.
 *
 * From the client's perspective they are also useful in defining requirements
 * for device and service types. For instance, if you have a component that
 * expects a discovered UPnP device to contain certain services, state variables
 * and actions, HUPnP can use these requirements to filter devices that are
 * suitable in terms of advertised capabilities.
 *
 * \ingroup hupnp_common
 */
enum HInclusionRequirement
{
    /*!
     * This value indicates that the inclusion requirement for the component
     * is not specified.
     *
     * This value is used only in error situations.
     */
    InclusionRequirementUnknown = 0,

    /*!
     * This value indicates that the component has to be appropriately specified.
     * It is a critical error if the component is missing.
     */
    InclusionMandatory,

    /*!
     * This value indicates that the component is optional and may or may not be
     * specified.
     */
    InclusionOptional
};

 /*!
 * \brief This enumeration specifies the logging levels that can be used with the device host.
 *
 * \ingroup hupnp_common
 */
enum HLogLevel
{
    /*!
     * No logs are generated.
     *
     * \remark by default, HUPnP uses this logging level.
     */
    None = 0,

    /*!
     * Only fatal messages are logged. Most often a fatal message is
     * followed by termination of the application.
     */
    Fatal = 1,

    /*!
     * Only critical and fatal messages are logged. Most often a critical message
     * signals a severe runtime error.
     */
    Critical = 2,

    /*!
     * Messages with level set to warning, critical and fatal are logged.
     * A warning message usually signifies an error or exceptional situation
     * that should be noted. Most often the system stability is not at stake
     * when warning messages appear, but they may still indicate that some
     * component, internal or external, is not functioning correctly.
     * Usually the source of warnings should be investigated.
     */
    Warning = 3,

    /*!
     * All but debug level messages are logged. An informational message is used
     * to log status information of control flow. A good example of an informational
     * message is when a sizable component logs the start of an initialization procedure.
     */
    Information = 4,

    /*!
     * All up to the debug messages are output. This excludes only the function
     * enter and exit messages.
     *
     * \remark Enabling this level of logging has notable effect on performance.
     * This generally should be used only for debugging purposes.
     */
    Debug = 5,

    /*!
     * Every log message is output. This includes even the function enters
     * and exits.
     *
     * \remark Enabling this level of logging has severe effect on performance.
     * This is very rarely needed and usually the debug level is far more helpful.
     */
    All = 6
};

/*!
 * \brief Sets the logging level the HUPnP should use.
 *
 * \param level specifies the desired logging level.
 *
 * \remark
 * \li The new logging level will take effect immediately.
 * \li The function is thread-safe.
 *
 * \ingroup hupnp_common
 */
void H_UPNP_CORE_EXPORT SetLoggingLevel(HLogLevel level);

/*!
 * Enables / disables warnings that relate to non-standard behavior discovered
 * in other UPnP software.
 *
 * Most often if non-standard behavior in other UPnP software is discovered, it
 * isn't fatal or critical and it may be possible to inter-operate with the software.
 * However, deviations from the specifications and standards are unfortunate and such
 * \b errors should be fixed.
 *
 * Regardless, you may not want to be notified about these warnings in which
 * case you can specifically disable all the warnings that relate to non-standard
 * behavior.
 *
 * \param arg specifies whether to output warnings that are about non-standard
 * behavior in other UPnP software.
 *
 * \remark by default, the non standard behavior warnings are on.
 *
 * \ingroup hupnp_common
 */
void H_UPNP_CORE_EXPORT EnableNonStdBehaviourWarnings(bool arg);

}
}

#endif /* HUPNP_GLOBAL_H_ */
