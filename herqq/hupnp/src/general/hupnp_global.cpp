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

#include "hupnp_global.h"
#include "hupnp_global_p.h"

#include "../socket/hendpoint.h"
#include "../dataelements/hproduct_tokens.h"

#include "../general/hupnpinfo.h"
#include "../general/hlogger_p.h"

#include <QtXml/QDomElement>
#include <QtCore/QTextStream>
#include <QtCore/QMutexLocker>
#include <QtNetwork/QNetworkInterface>

#if defined(Q_OS_LINUX)
#include <sys/utsname.h>
#endif

/*!
 * \namespace Herqq The main namespace of Herqq libraries. This namespace contains
 * the global enumerations, typedefs, functions and classes that are used
 * and referenced throughout the Herqq libraries.
 *
 * \namespace Herqq::Upnp The namespace that contains all of the Herqq UPnP
 * core functionality.
 */

namespace Herqq
{

namespace Upnp
{

/*! \mainpage Reference Documentation of the development versions of HUPnP and HUPnPAv
 *
 * \section introduction Introduction
 *
 * %Herqq UPnP Library (thereafter HUPnP) is a collection of reusable classes that
 * provide UPnP connectivity conforming to the
 * <a href="http://www.upnp.org/specs/arch/UPnP-arch-DeviceArchitecture-v1.1.pdf">
 * UPnP Device Architecture version 1.1</a>.
 *
 * Above everything else, HUPnP is designed to be simple to use and robust in operation.
 * HUPnP does its best to enable you, the user, to focus on the business logic
 * of your domain, rather than to the details of UPnP. However, not everything
 * can be hidden and some UPnP knowledge is required to fully understand
 * the system mechanics and the terms used in this documentation. To fill such
 * gaps you can check the aforementioned UDA specification and other documents
 * available at the <a href="http://www.upnp.org/resources/documents.asp">UPnP Forum</a>.
 *
 * HUPnP is tightly integrated into the <a href="http://qt.nokia.com/">Qt Framework</a> and
 * follows closely the very same design principles and programming practices Qt
 * follows. You will get the most out of HUPnP by using it alongside with Qt.
 * You can use HUPnP from other environments as well, assuming the appropriate
 * Qt headers and libraries are available.
 *
 * \section settingup Setting Up
 *
 * First, it is important to point out that at the moment HUPnP is
 * officially distributed in source code only. If you
 * come across a binary of HUPnP, it is not made by the author of HUPnP.
 * Second, HUPnP uses the
 * <a href="http://qt.nokia.com/products/appdev/add-on-products/catalog/4/Utilities/qtsoap">QtSoap library</a>
 * under the <a href="http://www.gnu.org/licenses/lgpl-2.1.html">LGPLv2.1</a> license.
 * The QtSoap library is distributed along the HUPnP in
 * source code and the library is built into a shared library during the compilation of HUPnP.
 * \attention At the moment, HUPnP uses a modified version of QtSoap version 2.7.
 * This is because the original version 2.7 contains few severe bugs in regard
 * to thread-safety. Until the errors are fixed in the official QtSoap release, the users
 * of HUPnP \b must use the modified version distributed with HUPnP.
 *
 * In order to use HUPnP, you need to build it first. By far the simplest way to do
 * this is to download the <a href="http://qt.nokia.com/downloads">Qt SDK</a>,
 * install it, start QtCreator and open the HUPnP project file \b herqq.pro
 * located in the root of the HUPnP package. To build HUPnP from the command-line
 * you need to run:
 * - <c>qmake -recursive herqq.pro</c> to create the make files. You'll find the
 * \c herqq.pro project file under the project's root directory.
 * - \c make to build HUPnP and
 * - \c make \c install to install HUPnP. This step is optional and can be omitted
 * if you do not want to install the HUPnP headers and binaries anywhere. See
 * \ref deployment for further information.
 *
 * The build produces two shared libraries to which you need to link in order to use
 * HUPnP. Currently, static linking is not an option. The created libraries are
 * placed in \c bin directory and they are named <c>[lib]HUPnP[-majVer.minVer.patchVer].x</c> and
 * <c>[lib]QtSolutions_SOAP-2.7.x</c>, where \c ".x" is the platform dependent suffix
 * for shared libraries. In addition, your compiler
 * must be aware of the HUPnP includes, which can be found in the \c include
 * directory. It is very important that you do \b not directly include anything that
 * is not found in the \c include directory. In any case, once your compiler
 * finds the HUPnP includes and your linker finds the HUPnP shared library,
 * you are good to go. \attention You do \b not need to include the QtSoap includes.
 * HUPnP does not expose the types declared in QtSoap.
 *
 * \section importantnotes Important notes
 *
 * Before starting to use HUPnP, there are a few things you have to know.
 *
 * \subsection include \#include
 *
 * HUPnP follows similar include pattern to that of Qt. When you want to use a
 * a class in HUPnP, you have to use \verbatim #include <HClassName> \endverbatim
 * where <c>HClassName</c> matches the name of the class you wish to use exactly.
 *
 * \subsection memorymanagement Memory management
 *
 * Some of the key classes in HUPnP are always instantiated by HUPnP and the
 * instances are always destroyed by HUPnP. You should never delete these.
 * The API documentation of HUPnP is clear about object ownership and
 * these classes are identified in documentation.
 *
 * \subsection herqqheader The HUpnp include file
 *
 * %HUpnp introduce a number of types, functions, enums and
 * type definitions under the root namespace \c Herqq. For instance, all the
 * HUPnP core types can be found under the namespace Herqq::Upnp.
 *
 * In several occasions, you do not need to include the full %HUPnP type definitions for your
 * code to work. More specifically, if the compiler doesn't need to see the layout
 * of a %HUPnP type to compile your code, you should only forward-declare such %HUPnP
 * types. In that case, you can include the
 * \c HUpnp file, which provides forward-declarations to every public
 * %HUPnP type and function.
 *
 * \subsection logging Logging
 *
 * In many situations it is useful to see some log output to have some idea what is
 * going on under the hood, especially when something appears
 * to be malfunctioning. You can enable logging in HUPnP by calling the
 * function Herqq::Upnp::SetLoggingLevel() with a desired \e level argument.
 * Include \c HUpnp to use the Herqq::Upnp::SetLoggingLevel().
 *
 * \subsection deployment Deployment
 *
 * You can run \c make \c install after compiling the project to copy the
 * public headers and created dynamic libraries into \c hupnp/deploy folder. More
 * specifically, \c hupnp/deploy/include will contain all the public headers and
 * \c hupnp/deploy/lib/ will contain the dynamic libraries. Alternatively, you can
 * run <c>qmake -recursive "PREFIX = /usr"</c> before running \c make \c install
 * to install the headers and binaries under the <c>/usr/include</c>
 * and <c>/usr/lib</c> respectively. This may be useful to you in case you
 * do not want to include the full HUPnP source tree with your software. You can
 * run \c make \c uninstall to remove HUPnP from the selected location.
 *
 * \subsection versioning Versioning
 *
 * Until the first stable release (1.0) is made, HUPnP follows a versioning practise
 * where the \e major component is always zero, the \e minor component is
 * incremented whenever an API or ABI breaking change is introduced and the
 * \e patch component is increment upon each update that preserves the
 * API and ABI.
 *
 * By including \c HUpnpInfo, you can call hupnpCoreVersion() to query the exact
 * version of a HUPnP Core library at runtime. At compile-time you can use the macros
 * HUPNP_CORE_MAJOR_VERSION, HUPNP_CORE_MINOR_VERSION, HUPNP_CORE_PATCH_VERSION
 * and HUPNP_CORE_VERSION for checking the version of the API.
 *
 * \section gettingstarted Getting Started
 *
 * Often the best explanation is demonstration.
 * So without further ado, the following links should get you started.
 *
 * \li \ref builddevice_tutorial shows how to build your own UPnP device using
 * HUPnP.
 * \li The API documentation of Herqq::Upnp::HControlPoint shows how to discover
 * and use UPnP devices on the network.
 * \li The API documentation of Herqq::Upnp::HDeviceHost shows how to host a Herqq::Upnp::HServerDevice.
 * This is how you setup a UPnP device to be found and used by UPnP control points.
 *
 * For more detailed information, you can check
 *
 * \li \ref hupnp_devicemodel for the details of the HUPnP device model and
 * \li \ref hupnp_devicehosting for the details of hosting a device.
 *
 * From there, the API reference is the way to go.
 */

/*!
 * \defgroup hupnp_devicemodel Device Model
 * \ingroup hupnp_core
 *
 * \brief This page explains the concept of HUPnP Device Model, which is the
 * logical object hierarchy of HUPnP representing the UPnP Device Architecture.
 *
 * \section notesaboutdesign A few notes about the design
 *
 * The main four components of the UPnP device model are
 * <em>a device</em>, <em>a service</em>, <em>a state variable</em> and
 * <em>an action</em>. These four components form a type of a tree in which
 * devices and services are contained by devices, and state variables
 * and actions are contained by services. This is called the <em>device tree</em>.
 * A device tree has a \e root \e device, which is a UPnP device that has
 * no parent, but may contain other UPnP devices. These contained devices
 * are called <em>embedded devices</em>.
 *
 * HUPnP's device model mimicts this design closely. At server-side the main
 * four components are HServerDevice, HServerService, HServerStateVariable and
 * HServerAction. At client-side the main four components are HClientDevice,
 * HClientService, HClientStateVariable and HClientAction.
 *
 * The purpose of the other classes part of the HUPnP's device model is to support the
 * initialization and use of the four core components both at the server and
 * client-side.
 *
 * \subsection The API differences between client and server sides
 *
 * The HUPnP device model is largely the same at the server and client sides.
 * That is, whether you are writing a custom UPnP device or
 * trying to interact with a UPnP device found in the network, you will be
 * interacting with the HUPnP device model in a similar manner. Regardless of that,
 * there are some important differences, which is why the server and
 * client-side define and use different types that do not share ancestors
 * provided by HUPnP.
 *
 * First and most importantly, server side contains the "businness" logic of
 * a UPnP device and clients only invoke or use it. This logic should not be
 * duplicated to the client-side, as it serves no purpose there. Having a design
 * that explicitly states this separation of concerns at type level should
 * certainly help in making the purpose of each type clear.
 *
 * Second, UPnP clients are unable to modify server-side state variables directly
 * and in a uniform manner. This is because UPnP Device Architecture does not
 * specify a mechanism for changing the value of a state variable from client-side.
 * Certainly the value of a state variable may be changeable, but that and the
 * way it is done depends of the service type in which the state variable is defined.
 * On the other hand, server-side has to have direct read-write access to the
 * state variables. This type of difference should be explicit in the design.
 *
 * Third, client-side action invocation should naturally be asynchronous
 * to the user, as the call most often involves network access. On the
 * server-side the action invocations are direct method calls and as such the
 * burden of running them "asynchronously" with the help of worker threads
 * should reside at the user code.
 *
 * \subsection lifetime_and_ownership The lifetime and ownership of objects
 *
 * Every \e device (HClientDevice and HServerDevice) has the ownership of all
 * of its embedded devices, services, actions and state variables and the
 * ownership is never released. This means that every device always manages
 * the memory used by the objects it owns. Hence, the owner of a \e root \e device
 * ultimately has the ownership of an entire device tree.
 *
 * This is a very important point to remember: <b>the lifetime of every object
 * contained by the root device depends of the lifetime of the root device</b>.
 *
 * In other words, when a root device is deleted, every
 * embedded device, service, state variable and action underneath it are deleted as well.
 * Furthermore, every \e root \e device is always owned by HUPnP and the
 * ownership is never released. Because of this you must never call \c delete
 * to any of the components of the device model that is setup by HUPnP.
 * Failing to follow this rule <b>will result in undefined behavior</b>.
 *
 * \note There are situations where you may want to instruct HUPnP to \c delete
 * a device. For instance, when a UPnP device is removed from the network
 * you may want your HControlPoint instance to remove the device that is no
 * longer available. This can be done through the \c %HControlPoint interface.
 * But note, HUPnP never deletes a device object without an explicit request from
 * a user.
 *
 * \section usage Usage
 *
 * Basic use is about interacting with already created objects that comprise the
 * device model. To get started you need to initialize either a device host or a
 * control point and retrieve a device or a list of devices from it.
 * See \ref hupnp_devicehosting for more information about device hosts and
 * control points. Once you have access to a device you can interact with any of its
 * embedded devices, services, state variables and actions until:
 * - you explicitly request the root device of the device tree to be deleted,
 * - \c delete the owner of a device tree, such as HControlPoint or HDeviceHost.
 *
 * See the corresponding classes for more information concerning their use.
 *
 * \note By default, HControlPoint keeps the state of the
 * state variables up-to-date. That is, using default configuration
 * an \c %HControlPoint automatically subscribes to events the UPnP services expose.
 * In such a case the state of a device tree the control point maintains reflects
 * the state of the corresponding device tree at server-side as accurately
 * as the server keeps sending events.
 *
 * If you wish to implement and host your own UPnP device things get
 * more involved. See \ref builddevice_tutorial to get started on building your
 * own UPnP devices using HUPnP.
 */


/*! \page builddevice_tutorial Tutorial for Building a UPnP Device
 *
 * \section settingup_descriptions Setting up the device and service descriptions
 *
 * Generally, building a UPnP device with HUPnP involves two main steps in your part.
 * First, you have to define a \em UPnP \em device \em description document following
 * the specifications set by the UPnP forum. Depending of your UPnP Device Description
 * document, you may need to define one or more \em UPnP \em service \em description documents
 * as well. Second, you may have to implement a class for your device and most often
 * one or more classes for each service your device contains.
 *
 * For example, if you want to implement a standard UPnP device named
 * \b BinaryLight:1, your device description could look something like this:
 *
 * \code
 * <?xml version="1.0"?>
 * <root xmlns="urn:schemas-upnp-org:device-1-0">
 *     <specVersion>
 *         <major>1</major>
 *         <minor>0</minor>
 *     </specVersion>
 *     <device>
 *         <deviceType>urn:schemas-upnp-org:device:BinaryLight:1</deviceType>
 *         <friendlyName>UPnP Binary Light</friendlyName>
 *         <manufacturer>MyCompany</manufacturer>
 *         <manufacturerURL>www.mywebsite.org</manufacturerURL>
 *         <modelDescription>New brilliant BinaryLight</modelDescription>
 *         <modelName>SuperWhiteLight 4000</modelName>
 *         <modelNumber>1</modelNumber>
 *         <UDN>uuid:138d3934-4202-45d7-bf35-8b50b0208139</UDN>
 *         <serviceList>
 *             <service>
 *                 <serviceType>urn:schemas-upnp-org:service:SwitchPower:1</serviceType>
 *                 <serviceId>urn:upnp-org:serviceId:SwitchPower:1</serviceId>
 *                 <SCPDURL>switchpower_scpd.xml</SCPDURL>
 *                 <controlURL>/control</controlURL>
 *                 <eventSubURL>/eventing</eventSubURL>
 *             </service>
 *         </serviceList>
 *     </device>
 * </root>
 * \endcode
 *
 * Note that the above is the standard device template for UPnP \b BinaryLight:1 filled
 * with imaginary information.
 *
 * Since the \b BinaryLight:1 defines a service, \b SwitchPower:1, you have to provide a
 * service description document that could look like this:
 *
 * \code
 * <?xml version="1.0"?>
 * <scpd xmlns="urn:schemas-upnp-org:service-1-0">
 *     <specVersion>
 *         <major>1</major>
 *         <minor>0</minor>
 *     </specVersion>
 *     <actionList>
 *         <action>
 *             <name>SetTarget</name>
 *             <argumentList>
 *                 <argument>
 *                     <name>newTargetValue</name>
 *                     <relatedStateVariable>Target</relatedStateVariable>
 *                     <direction>in</direction>
 *                 </argument>
 *             </argumentList>
 *          </action>
 *          <action>
 *              <name>GetTarget</name>
 *              <argumentList>
 *                  <argument>
 *                      <name>RetTargetValue</name>
 *                      <relatedStateVariable>Target</relatedStateVariable>
 *                      <direction>out</direction>
 *                  </argument>
 *              </argumentList>
 *          </action>
 *          <action>
 *              <name>GetStatus</name>
 *              <argumentList>
 *                  <argument>
 *                      <name>ResultStatus</name>
 *                      <relatedStateVariable>Status</relatedStateVariable>
 *                      <direction>out</direction>
 *                  </argument>
 *              </argumentList>
 *          </action>
 *      </actionList>
 *      <serviceStateTable>
 *          <stateVariable sendEvents="no">
 *              <name>Target</name>
 *              <dataType>boolean</dataType>
 *              <defaultValue>0</defaultValue>
 *          </stateVariable>
 *          <stateVariable sendEvents="yes">
 *              <name>Status</name>
 *              <dataType>boolean</dataType>
 *              <defaultValue>0</defaultValue>
 *          </stateVariable>
 *      </serviceStateTable>
 * </scpd>
 * \endcode
 *
 * The above description is the standard service description for the
 * \b SwitchPower:1 without any vendor specific declarations. For more information
 * about description documents, see the UDA specification, sections 2.3 and 2.5.
 *
 * \section creatingclasses Creating the necessary HUPnP classes
 *
 * HUPnP doesn't require any classes to be created in order to "host" a UPnP device
 * (make it visible for UPnP control points), but in order to plug in custom
 * functionality you often have to accompany the device and service descriptions with
 * corresponding classes.
 *
 * In our example we have to derive a class we have to derive a class from
 * Herqq::Upnp::HServerService for the \b SwitchPower:1 service description and
 * we \b can derive a class from Herqq::Upnp::HServerDevice for the
 * \b BinaryLight:1 device description. Note the last point, we do \b not have to
 * create a class for the \b BinaryLight:1, but we can. Furthermore,
 * if your service has no actions you do not need to create your own
 * HServerService type either.
 *
 * \note Omitting the creation of custom HServerDevice classes is common if there
 * is no need to define a type that orchestrates the use, initialization and
 * control of the contained services. Furthermore, such a type can specify
 * information about the embedded devices and services that have to be present
 * for the type to be initialized properly. And of course, it can offer features
 * that are not present in the default HServerDevice class.
 *
 * To create a custom Herqq::Upnp::HServerDevice you only need to derive from it.
 * There are no abstract member functions to override, but there are a few virtual
 * member functions that could be very useful to override in case you are
 * writing a type for other people to use. For more information of this, see
 * \ref hupnp_devicemodel.
 *
 * To create a concrete class from Herqq::Upnp::HServerService that exposes
 * custom actions you can either:
 * - Override Herqq::Upnp::HServerService::createActionInvokes(), which
 * purpose is to create <em>callable entities</em> that will
 * be called when the corresponding UPnP actions are invoked.
 * - Define \c Q_INVOKABLE methods in your custom type derived from HServerService
 * using the same method names as the action definitions in the service description
 * document.
 *
 * The first option is much more flexible, as you have full control over
 * what HUPnP should call when a particular action is invoked. In addition,
 * callable entities aren't tied to member functions. The second option may
 * be more convenient, as you don't have to implement
 * HServerService::createActionInvokes() and create the callable entities by
 * yourself. Whichever option you choose, every action implementation has to
 * have a signature of <c>action(const HActionArguments&, HActionArguments*)</c>
 * and \c int as a return type.
 *
 * \note
 * - The UPnP actions of a particular UPnP service are defined in the service's
 * description file and your service implementation has to implement all of them.
 * - The callable entities are used internally by HUPnP. HUPnP does not
 * otherwise expose them directly in the public API for action invocation.
 *
 * To continue with the example we will create two classes, one for the
 * \b BinaryLight:1 and one for the \b SwitchPowerService:1. Note, the class for
 * the BinaryLight:1 is \b not required, but it is done here for demonstration
 * purposes. Also note that for this example the class declarations are put
 * into the same header file, although in real code you might want to separate them.
 *
 * <c>mybinarylight.h</c>
 *
 * \code
 *
 * #include <HUpnpCore/HServerDevice>
 * #include <HUpnpCore/HServerService>
 *
 * class MyBinaryLightDevice :
 *    public Herqq::Upnp::HServerDevice
 * {
 *
 * public:
 *
 *    MyBinaryLightDevice();
 *    virtual ~MyBinaryLightDevice();
 * };
 *
 * class MySwitchPowerService :
 *    public Herqq::Upnp::HServerService
 * {
 * protected:
 *
 *     virtual HActionInvokes createActionInvokes();
 *
 * public:
 *
 *     MySwitchPowerService();
 *     virtual ~MySwitchPowerService();
 * };
 *
 * \endcode
 *
 * In turn, the implementation could look something like this:
 *
 * <c>mybinarylight.cpp</c>
 *
 * \code
 *
 * #include "mybinarylight.h"
 *
 * using namespace Herqq::Upnp;
 *
 * MyBinaryLightDevice::MyBinaryLightDevice()
 * {
 * }
 *
 * MyBinaryLightDevice::~MyBinaryLightDevice()
 * {
 * }
 *
 * MySwitchPowerService::MySwitchPowerService()
 * {
 * }
 *
 * MySwitchPowerService::~MySwitchPowerService()
 * {
 * }
 *
 * HServerService::HActionInvokes MySwitchPowerService::createActionInvokes()
 * {
 *     HActionInvokes retVal;
 *     return retVal;
 * }
 *
 * \endcode
 *
 * Those who know UPnP and paid close attention to the above example might have
 * noticed that something was off. Where are the actions?
 *
 * According to the UPnP Device Architecture (UDA), a service may have zero or
 * more actions. If a service has no actions, you don't have to create a custom
 * HServerService derivative in the first place, but even if you do,
 * similar class declaration and definition as shown above are enough.
 *
 * However, the standard \b BinaryLight:1 device type specifies the
 * \b SwitchPower:1 service type that has three actions defined
 * (look back in the service description document).
 * Namely these are \b SetTarget, \b GetTarget and \b GetStatus.
 * To make the example complete the <c>MySwitchPowerService</c> class
 * requires some additional work. Note that next example shows only one way
 * of making the service complete. There are a few other ways,
 * which will be discussed later in depth.
 *
 * The complete declaration for <c>MySwitchPowerService</c>:
 *
 * <c>mybinarylight.h</c>
 *
 * \code
 *
 * #include <HUpnpCore/HServerService>
 *
 * class MySwitchPowerService :
 *    public Herqq::Upnp::HServerService
 * {
 * protected:
 *
 *     virtual HActionInvokes createActionInvokes();
 *
 * public:
 *
 *     MySwitchPowerService();
 *     virtual ~MySwitchPowerService();
 *
 *     qint32 setTarget(
 *         const Herqq::Upnp::HActionArguments& inArgs,
 *         Herqq::Upnp::HActionArguments* outArgs);
 *
 *     qint32 getTarget(
 *         const Herqq::Upnp::HActionArguments& inArgs,
 *         Herqq::Upnp::HActionArguments* outArgs);
 *
 *     qint32 getStatus(
 *         const Herqq::Upnp::HActionArguments& inArgs,
 *         Herqq::Upnp::HActionArguments* outArgs);
 * };
 *
 * \endcode
 *
 * The complete definition for <c>MySwitchPowerService</c>:
 *
 * <c>mybinarylight.cpp</c>
 *
 * \code
 *
 * #include "mybinarylight.h"
 *
 * #include <HUpnpCore/HServerAction>
 * #include <HUpnpCore/HActionArguments>
 * #include <HUpnpCore/HServerStateVariable>
 *
 * MySwitchPowerService::MySwitchPowerService()
 * {
 * }
 *
 * MySwitchPowerService::~MySwitchPowerService()
 * {
 * }
 *
 * HServerService::HActionInvokes MySwitchPowerService::createActionInvokes()
 * {
 *     Herqq::Upnp::HServerService::HActionInvokes retVal;
 *
 *     retVal.insert(
 *         "SetTarget",
 *         Herqq::Upnp::HActionInvoke(this, &MySwitchPowerService::setTarget));
 *
 *     // The above lines map the MySwitchPowerService::setTarget() method to
 *     // the action that has the name SetTarget. In essence, this mapping instructs
 *     // HUPnP to call this method when the SetTarget action is invoked.
 *     // However, note that HActionInvoke accepts any "callable entity",
 *     // such as a normal function or a functor. Furthermore, if you use a
 *     // method the method does not have to be public.
 *
 *     retVal.insert(
 *         "GetTarget",
 *         Herqq::Upnp::HActionInvoke(this, &MySwitchPowerService::getTarget));
 *
 *     retVal.insert(
 *         "GetStatus",
 *         Herqq::Upnp::HActionInvoke(this, &MySwitchPowerService::getStatus));
 *
 *     return retVal;
 * }
 *
 * qint32 MySwitchPowerService::setTarget(
 *     const Herqq::Upnp::HActionArguments& inArgs, Herqq::Upnp::HActionArguments* outArgs)
 * {
 *     Herqq::Upnp::HActionArgument newTargetValueArg = inArgs.get("newTargetValue");
 *     if (!newTargetValueArg.isValid())
 *     {
 *         // If MySwitchPowerService class is not made for direct public use
 *         // this check is redundant, since in that case this method is called only by
 *         // HUPnP and HUPnP always ensures that the action arguments defined in the
 *         // service description are present when an action is invoked.
 *
 *         return Herqq::Upnp::UpnpInvalidArgs;
 *     }
 *
 *     bool newTargetValue = newTargetValueArg.value().toBool();
 *     stateVariables().value("Target")->setValue(newTargetValue);
 *
 *     // The above line modifies the state variable "Target", which reflects the
 *     // "target state" of a light device, i.e. if a user wants to turn off a light, the
 *     // "target state" is the light turned off whether the light can be turned
 *     // off or not.
 *
 *     //
 *     // Do here whatever that is required to turn on / off the light
 *     // (set it to the target state)
 *     //
 *
 *     //
 *     // If it succeeded, we should modify the Status state variable to reflect
 *     // the new state of the light.
 *     //
 *
 *     stateVariables().value("Status")->setValue(newTargetValue);
 *
 *     return Herqq::Upnp::UpnpSuccess;
 * }
 *
 * qint32 MySwitchPowerService::getTarget(
 *     const Herqq::Upnp::HActionArguments& inArgs, Herqq::Upnp::HActionArguments* outArgs)
 * {
 *     if (!outArgs)
 *     {
 *         // See the comments in MySwitchPowerService::setTarget why this
 *         // check is here. Basically, this check is redundant if this method
 *         // is called only by HUPnP, as HUPnP ensures proper arguments
 *         // are always provided when an action is invoked.
 *
 *         return Herqq::Upnp::UpnpInvalidArgs;
 *     }
 *
 *     bool b = stateVariables().value("Target")->value().toBool();
 *     outArgs->setValue("RetTargetValue", b);
 *
 *     return Herqq::Upnp::UpnpSuccess;
 * }
 *
 * qint32 MySwitchPowerService::getStatus(
 *     const Herqq::Upnp::HActionArguments& inArgs, Herqq::Upnp::HActionArguments* outArgs)
 * {
 *     if (!outArgs)
 *     {
 *         // See the comments in MySwitchPowerService::getTarget();
 *         return UpnpInvalidArgs;
 *     }
 *
 *     bool b = stateVariables().value("Status")->value().toBool();
 *     outArgs->setValue("ResultStatus", b);
 *
 *     return Herqq::Upnp::UpnpSuccess;
 * }
 *
 * \endcode
 *
 * The above example overrode the HServerService::createActionInvokes() and
 * did the action name - callable entity mapping. However, if you'd rather
 * have HUPnP do that automatically, you can mark your action implementations
 * as \c Q_INVOKABLE as follows:
 *
 * <c>mybinarylight.h</c>
 *
 * \code
 *
 * #include <HUpnpCore/HServerService>
 *
 * class MySwitchPowerService :
 *    public Herqq::Upnp::HServerService
 * {
 * Q_OBJECT
 *
 * public:
 *
 *     MySwitchPowerService();
 *     virtual ~MySwitchPowerService();
 *
 *     Q_INVOKABLE qint32 SetTarget(
 *         const Herqq::Upnp::HActionArguments& inArgs,
 *         Herqq::Upnp::HActionArguments* outArgs);
 *
 *     Q_INVOKABLE qint32 GetTarget(
 *         const Herqq::Upnp::HActionArguments& inArgs,
 *         Herqq::Upnp::HActionArguments* outArgs);
 *
 *     Q_INVOKABLE qint32 GetStatus(
 *         const Herqq::Upnp::HActionArguments& inArgs,
 *         Herqq::Upnp::HActionArguments* outArgs);
 * };
 *
 * \endcode
 *
 * Apart from changing the method names to start with capital letters,
 * the method definitions stay otherwise the same.
 *
 * \note
 * Using \c Q_INVOKABLE methods as action implementations you have to
 * ensure that the names of the member functions correspond \b exactly to the
 * action names defined in the service description document.
 *
 * \subsection some_notes_about_switchpower_example Some closing notes
 *
 * First of all, you may want to skim the discussion in
 * \ref hupnp_devicemodel and \ref hupnp_devicehosting to fully understand
 * the comments in the example above. Especially the section
 * \ref setting_up_the_devicemodel is useful if you want to learn the details of
 * building a custom UPnP device using HUPnP. That being said, perhaps the
 * most important issues of building a custom UPnP device using HUPnP
 * can be summarized to:
 *
 * - Every device description has to have a corresponding Herqq::Upnp::HServerDevice
 * and every service description has to have a corresponding Herqq::Upnp::HServerService.
 * However, you don't have to create a custom HServerDevice, in which case HUPnP
 * will create and use a default HServerDevice type. In addition, if a service has no actions
 * you don't have to create a custom HServerService. On the other hand, most
 * commonly a service has one or more actions, so this is something you'll be
 * doing often.
 *
 * - It is perfectly fine to create custom HServerDevice and HServerService classes
 * just to be hosted in a Herqq::Upnp::HDeviceHost. Such classes exist only to
 * run your code when UPnP control points interact with them over the network.
 * These types of classes are to be used directly only by HDeviceHost.
 *
 * - You can create more advanced HServerDevice and HServerService classes perhaps
 * to build a higher level public API or just to provide yourself a nicer interface for
 * doing something. This was the case with \c MySwitchPowerService class, which
 * extended the HServerService interface by providing the possibility of invoking
 * the actions of the service through the \c setTarget(), \c getTarget() and \c getStatus()
 * methods.
 *
 * - HUPnP allows direct (in-process) access to the hosted
 * HServerDevice and HServerService classes, which means you can interact with
 * your classes \b while they are being hosted and possibly used from external
 * processes. Custom HServerDevice and HServerService interfaces may be beneficial
 * in such a case.
 *
 * - The type behind an Herqq::Upnp::HActionInvoke can hold any
 * <em>callable entity</em>, such as a pointer to a normal function,
 * functor or a pointer to a member function.
 *
 * - A public callable entity should always strictly verify the input and
 * respond to illegal input accordingly. A "private" callable entity that
 * is called only by HUPnP can rest assured that HUPnP never passes a null input
 * argument or an argument that has an incorrect name or data type.
 *
 * - Before implementing your own device
 * and service types directly from HServerDevice and HServerService,
 * you should check if HUPnP provides more refined classes to suit your
 * requirements. For instance, HUPnP provides a base class
 * Herqq::Upnp::Lighting::HAbstractSwitchPower for simplifying the implementation
 * and use of \b SwitchPower:1.
 *
 * In any case, the above example demonstrates a fully standard-compliant implementation
 * of \b BinaryLight:1. The next step is to publish your HServerDevice in the network
 * for UPnP control points to discover. You can find the instructions for that
 * in HDeviceHost and \ref hupnp_devicehosting.
 */

/*! \page setting_up_the_devicemodel Setting Up the Device Model
 *
 * \section before_we_start Before We Start
 *
 * Please note that in case you are writing client-side
 * software you rarely need to worry about how the device model gets built. But
 * if you are implementing a server-side UPnP device, this is something you
 * should know. In addition, you should first skim through the
 * \ref builddevice_tutorial if you haven't already.
 *
 * \section how_devicehost_builds_a_device How HDeviceHost builds a device tree
 *
 * When you initialize an Herqq::Upnp::HDeviceHost you have to provide it:
 * - one or more device descriptions that represent the UPnP device(s) you want
 * to host and
 * - a <em>device model creator</em> that creates the C++ classes representing the
 * UPnP devices and services defined in the description documents.
 *
 * You can find more information about setting up the Herqq::Upnp::HDeviceHost in
 * \ref hupnp_devicehosting, but what is relevant here is that you provide two
 * pieces of information that \b must match; the C++ classes created by the
 * <em>device model creator</em> \b must reflect the description documents and vice
 * versa.
 *
 * During its initialization an \c %HDeviceHost scans the provided
 * description document(s) and whenever it encounters a definition of a
 * UPnP device it invokes the device model creator you have provided to create
 * an instance of HServerDevice matching the device definition in the
 * description document. Similarly, whenever it encounters a UPnP service it invokes
 * the device model creator to create an instance of HServerService.
 *
 * When creating a device the \c %HDeviceHost provides an HDeviceInfo object to
 * the creator, which contains the information read from the description document.
 * And again, when creating a service an HServiceInfo object is provided to the
 * creator. Based on the information in the \e info objects the device model
 * creator is expected to create an HServerDevice or HServerService instance
 * if it can, or return \c null otherwise. If the device model creator returns
 * \c null the \c %HDeviceHost creates an instance of a default type used to
 * represent the encountered UPnP device or service.
 *
 * Consider an example of a simple device model creator,
 *
 * \code
 *
 * #include <HUpnpCore/HDeviceModelCreator>
 *
 * class MyCreator : public Herqq::Upnp::HDeviceModelCreator
 * {
 *
 * private:
 *
 *   // overridden from HDeviceModelCreator
 *   virtual MyCreator* newInstance() const
 *   {
 *       return new MyCreator();
 *   }
 *
 * public:
 *
 *   // overridden from HDeviceModelCreator
 *   virtual MyHServerDevice* createDevice(const Herqq::Upnp::HDeviceInfo& info) const
 *   {
 *       if (info.deviceType().toString() == "urn:herqq-org:device:MyDevice:1")
 *       {
 *           return new MyHServerDevice();
 *       }
 *
 *       return 0;
 *   }
 *
 *   // overridden from HDeviceModelCreator
 *   virtual MyHServerService* createService(
 *       const Herqq::Upnp::HServiceInfo& serviceInfo,
 *       const Herqq::Upnp::HDeviceInfo& parentDeviceInfo) const
 *   {
 *       if (serviceInfo.serviceType().toString() == "urn:herqq-org:service:MyService:1")
 *       {
 *           return new HMyServerService();
 *       }
 *
 *       // Note, parentDeviceInfo is not needed in this case, but there are
 *       // scenarios when it is mandatory to know information of the parent
 *       // device to create the correct HServerService type.
 *
 *       return 0;
 *   }
 * };
 * \endcode
 *
 * \note The \c %HDeviceHost takes ownership of the returned objects. However,
 * the \c %HDeviceHost will not destroy any of them until it is being deleted.
 *
 * \section plugging_in_custom_functionality Plugging in Custom Functionality
 *
 * In the UPnP architecture the \e actions represent the functionality of a device.
 * Actions are contained by services and services are contained by devices.
 * Thus, custom functionality means custom actions, which are logically placed
 * into custom services. Custom services, on the other hand, don't have to reside
 * inside custom devices. This is because ultimately a device is only a
 * container for services and that is certainly something the default device
 * types used by HUPnP can handle.
 *
 * So, to plug-in custom functionality you need a custom service type, which
 * main purpose is to map <em>callable entities</em> to UPnP action names.
 *
 * \note A callable entity is a C++ concept that
 * is used to refer to anything that can be called with the \c operator(),
 * such as a normal function, functor or a member function.
 *
 * There are two ways to do the mapping. You can either override
 * Herqq::Upnp::HServerService::createActionInvokes() in your custom HServerService
 * type, or you can mark member functions of your custom HServerService as
 * \c Q_INVOKABLE.
 *
 * Consider an example of overriding the \c createActionInvokes(),
 *
 * \code
 * Herqq::Upnp::HServerService::HActionInvokes MyConnectionManagerService::createActionInvokes()
 * {
 *   Herqq::Upnp::HServerService::HActionInvokes retVal;
 *
 *   retVal.insert("GetProtocolInfo",
 *       HActionInvoke(this, &MyConnectionManagerService::getProtocolInfo));
 *
 *   retVal.insert("PrepareForConnection",
 *       HActionInvoke(this, &MyConnectionManagerService::prepareForConnection));
 *
 *   retVal.insert("ConnectionComplete",
 *       HActionInvoke(this, &MyConnectionManagerService::connectionComplete));
 *
 *   retVal.insert("GetCurrentConnectionIDs",
 *       HActionInvoke(this, &MyConnectionManagerService::getCurrentConnectionIDs));
 *
 *   retVal.insert("GetCurrentConnectionInfo",
 *       HActionInvoke(this, &MyConnectionManagerService::getCurrentConnectionInfo));
 *
 *   return retVal;
 * }
 * \endcode
 *
 * The above code maps five member functions of the class
 * \c MyConnectionManagerService to the five action names accordingly. Once the
 * device tree is fully set up and the HServerDevice containing the
 * \c MyConnectionManagerService is hosted by an HDeviceHost,
 * action invocations are ultimately directed to these mapped member functions.
 * In other words, in this case it is these member functions that have to
 * do whatever it is that these actions are expected to do.
 *
 * \note The callable entity concept detaches invocation logic from what is being
 * invoked and enables these "entities" to be handled by value. It is a very
 * powerful concept that allows you to map <em>anything that can be called</em>
 * following a certain signature under the same interface and copy these \e
 * entities around by-value.
 *
 * To give you an idea of the versatility of an callable entity, you could do
 * the above with normal functions too:
 *
 * \code
 * namespace
 * {
 * int getProtocolInfo(const Herqq::Upnp::HActionArguments& inArgs, Herqq::Upnp::HActionArguments* outArgs)
 * {
 * }
 *
 * int prepareForConnection(const Herqq::Upnp::HActionArguments& inArgs, Herqq::Upnp::HActionArguments* outArgs)
 * {
 * }
 *
 * int connectionComplete(const Herqq::Upnp::HActionArguments& inArgs, Herqq::Upnp::HActionArguments* outArgs)
 * {
 * }
 *
 * int getCurrentConnectionIDs(const Herqq::Upnp::HActionArguments& inArgs, Herqq::Upnp::HActionArguments* outArgs)
 * {
 * }
 *
 * int getCurrentConnectionInfo(const Herqq::Upnp::HActionArguments& inArgs, Herqq::Upnp::HActionArguments* outArgs)
 * {
 * }
 * }
 *
 * Herqq::Upnp::HServerService::HActionInvokes MyConnectionManagerService::createActionInvokes()
 * {
 *   Herqq::Upnp::HServerService::HActionInvokes retVal;
 *
 *   retVal.insert("GetProtocolInfo", Herqq::Upnp::HActionInvoke(getProtocolInfo));
 *   retVal.insert("PrepareForConnection", Herqq::Upnp::HActionInvoke(prepareForConnection));
 *   retVal.insert("ConnectionComplete", Herqq::Upnp::HActionInvoke(connectionComplete));
 *   retVal.insert("GetCurrentConnectionIDs", Herqq::Upnp::HActionInvoke(getCurrentConnectionIDs));
 *   retVal.insert("GetCurrentConnectionInfo", Herqq::Upnp::HActionInvoke(getCurrentConnectionInfo));
 *
 *   return retVal;
 * }
 * \endcode
 *
 * And it doesn't stop there. You can use functors as well.
 *
 * However, you can also let HUPnP to do the mapping and creation of the callable
 * entities, but to do that you need to:
 *
 * - Make sure your custom HServerService type uses the \c Q_OBJECT macro.
 * - Mark each member function to be used as an action implemention with \c Q_INVOKABLE
 * macro.
 * - Ensure that the member functions are named exactly as the corresponding
 * actions are defined in the service description document.
 *
 * Consider an example of using \c Q_INVOKABLE,
 *
 * \code
 *
 * class MyConnectionManagerService :
 *   public HServerService
 * {
 *  Q_OBJECT
 *
 *  public:
 *
 *    MyConnectionManagerService();
 *    virtual ~MyConnectionManagerService();
 *
 *    Q_INVOKABLE qint32 GetProtocolInfo(
 *        const HActionArguments& inArgs, HActionArguments* outArgs);
 *
 *    Q_INVOKABLE qint32 PrepareForConnection(
 *        const HActionArguments& inArgs, HActionArguments* outArgs);
 *
 *    Q_INVOKABLE qint32 ConnectionComplete(
 *        const HActionArguments& inArgs, HActionArguments* outArgs);
 *
 *   Q_INVOKABLE qint32 GetCurrentConnectionIDs(
 *        const HActionArguments& inArgs, HActionArguments* outArgs);
 *
 *   Q_INVOKABLE qint32 GetCurrentConnectionInfo(
 *        const HActionArguments& inArgs, HActionArguments* outArgs);
 * };
 *
 * \endcode
 *
 * Note, the method names are started with capital letters as are the
 * corresponding actions defined in the ConnectionManager specification.
 *
 * Once you have set up the action mappings, your custom HServerService
 * is ready to be used. However, there is much more you can do with
 * Herqq::Upnp::HDeviceModelInfoProvider::actionsSetupData() to ensure that the
 * service description containing the action definitions is correct. Similarly,
 * you can override Herqq::Upnp::HDeviceModelInfoProvider::stateVariablesSetupData()
 * to provide additional information to HUPnP in order to make sure that the
 * service description document is appropriately set up in terms of state
 * variables as well. This may not be important to you if you are writing a
 * "private" implementation of a service, but it could be very useful if you are
 * writing a "public" library of UPnP devices and services that have to make
 * sure they are appropriately used.
 *
 * \note
 * When implementing a custom \c %HServerService class you have to define the
 * implementations for the action definitions found in the corresponding
 * service description document. You are not required to do anything else.
 * However, you \b may provide additional information about the structure and
 * details of a UPnP service via HDeviceModelInfoProvider::actionsSetupData() and
 * HDeviceModelInfoProvider::stateVariablesSetupData(), but those are always optional.
 *
 * \section providing_more_info_to_model_setup Providing more information to the model setup process
 *
 * There are a few ways to provide in-depth information of a UPnP device to ensure
 * that a device model will get built as desired. All of the methods described
 * here require HDeviceModelInfoProvider. First, you can override
 * HDeviceModelInfoProvider::embedddedDevicesSetupData() and HDeviceModelInfoProvider::servicesSetupData()
 * methods to specify what embedded devices and services has to be defined in the
 * device description for an instance of the device type to function correctly.
 * Second, you can provide detailed information of expected actions and their
 * arguments when you override HDeviceModelInfoProvider::actionsSetupData(). Third,
 * you can provide detailed information of expected state variables when you override
 * HDeviceModelInfoProvider::stateVariablesSetupData().
 *
 * \note All of these methods and techniques described here are optional.
 *
 * The benefit of overriding these methods is that HUPnP can ensure that the
 * provided description documents provide everything the respective C++ classes
 * expect. The validation is done by HUPnP during the build of the device model
 * and the build succeeds only if the provided description documents are
 * appropriately defined. This way you never have to validate the device model
 * yourself and you do not have to check everywhere if an embedded device,
 * a service, a required action, an action argument or a state variable is provided.
 * But if your device and service types are created for internal or
 * otherwise controlled use only, implementing your own HDeviceModelInfoProvider
 * may be unnecessary.
 *
 * An example of \c servicesSetupData():
 *
 * \code
 *
 * Herqq::Upnp::HServicesSetupData MyDeviceModelInfoProvider::servicesSetupData(
 *     const Herqq::Upnp::HDeviceInfo& info)
 * {
 *  Herqq::Upnp::HServicesSetupData retVal;
 *
 *  QString type = info.deviceType().toString();
 *  if (type == "urn:schemas-upnp-org:device:DimmableLight:1")
 *  {
 *    retVal.insert(
 *       Herqq::Upnp::HServiceSetup(
 *           Herqq::Upnp::HServiceId("urn:schemas-upnp-org:serviceId:SwitchPower"),
 *           Herqq::Upnp::HResourceType("urn:schemas-upnp-org:service:SwitchPower:1")));
 *
 *    retVal.insert(
 *       Herqq::Upnp::HServiceSetup(
 *           Herqq::Upnp::HServiceId("urn:schemas-upnp-org:serviceId:Dimming"),
 *           Herqq::Upnp::HResourceType("urn:schemas-upnp-org:service:Dimming:1")));
 *  }
 *
 *   return retVal;
 * }
 * \endcode
 *
 * The above definition instructs HUPnP to ensure that when the \c DimmableLight:1
 * device type is being initialized those two specified services are found
 * in the device description document provided by the user. If either one of them
 * is missing or contains invalid information, the device model build
 * is aborted, the HDeviceHost::init() fails and HDeviceHost::error()
 * returns HDeviceHost::InvalidDeviceDescriptionError. See the documentation of
 * HServiceSetup for more information of what can be validated.
 *
 * An example of \c embedddedDevicesSetupData():
 *
 * \code
 * Herqq::Upnp::HDevicesSetupData MyDeviceModelInfoProvider::embedddedDevicesSetupData(
 *     const HDeviceInfo& info)
 * {
 *  Herqq::Upnp::HDevicesSetupData retVal;
 *
 *  QString type = info.deviceType().toString();
 *  if (type == "urn:custom-domain-org:device:MyDeviceType:1")
 *  {
 *    retVal.insert(
 *       Herqq::Upnp::HDeviceSetup(
 *           Herqq::Upnp::HResourceType("urn:my-domain-org:device:MyDevice_X:1")));
 *
 *    retVal.insert(
 *       Herqq::Upnp::HDeviceSetup(
 *           Herqq::Upnp::HResourceType("urn:my-domain-org:device:MyDevice_Y:1")));
 *  }
 *
 *   return retVal;
 * }
 * \endcode
 *
 * The above definition instructs HUPnP to ensure that when the \c MyDeviceType:1
 * device type is being initialized those two specified embedded devices are found
 * in the device description document provided by the user. If either one of them
 * is missing or contains invalid information, the device model build
 * is aborted, the HDeviceHost::init() fails and HDeviceHost::error()
 * returns HDeviceHost::InvalidDeviceDescriptionError. See the documentation of
 * HDeviceSetup for more information of what can be validated.
 *
 * An example of \c stateVariablesSetupData():
 *
 * \code
 * Herqq::Upnp::HStateVariablesSetupData MyDeviceModelInfoProvider::stateVariablesSetupData(
 *     const Herqq::Upnp::HServiceInfo& serviceInfo, const Herqq::Upnp::HDeviceInfo& parentDeviceInfo)
 * {
 *  Herqq::Upnp::HStateVariablesSetupData retVal;
 *
 *  if (info.serviceType().compare(
 *      HResourceType("urn:schemas-upnp-org:service:ConnectionManager:2"),
 *      HResourceType::Inclusive))
 *  {
 *    retVal.insert(HStateVariableInfo(
 *      "SourceProtocolInfo", HUpnpDataTypes::string));
 *
 *    retVal.insert(HStateVariableInfo(
 *      "SinkProtocolInfo", HUpnpDataTypes::string));
 *
 *    retVal.insert(HStateVariableInfo(
 *      "CurrentConnectionIDs", HUpnpDataTypes::string));
 *
 *    retVal.insert(HStateVariableInfo(
 *      "A_ARG_TYPE_ConnectionStatus", HUpnpDataTypes::string));
 *
 *    retVal.insert(HStateVariableInfo(
 *      "A_ARG_TYPE_ConnectionManager",HUpnpDataTypes::string));
 *
 *    retVal.insert(HStateVariableInfo(
 *      "A_ARG_TYPE_ProtocolInfo", HUpnpDataTypes::string));
 *
 *    retVal.insert(HStateVariableInfo(
 *      "A_ARG_TYPE_ConnectionID", HUpnpDataTypes::i4));
 *
 *    retVal.insert(HStateVariableInfo(
 *      "A_ARG_TYPE_AVTransportID", HUpnpDataTypes::i4));
 *
 *    retVal.insert(HStateVariableInfo(
 *      "A_ARG_TYPE_RcsID", HUpnpDataTypes::i4));
 *  }
 *
 *   return retVal;
 * }
 * \endcode
 *
 * The above definition instructs HUPnP to ensure that when the \c ConnectionManager:2
 * service type is being initialized all those specified state variables are found
 * in the service description document provided by the user. If any one of them
 * is missing or contains invalid information, the device model build
 * is aborted, the HDeviceHost::init() fails and HDeviceHost::error()
 * returns HDeviceHost::InvalidServiceDescriptionError. See the documentation of
 * HStateVariableInfo for more information of what can be validated.
 *
 * Finally, you can override HDeviceModelInfoProvider::actionsSetupData() to
 * provide detailed information about the action and its expected arguments.
 * See HActionSetup for more information of this.
 *
 * \note It is planned that in the future this same information could be used to
 * create instances of HUPnP's device model without description documents.
 *
 * \sa hupnp_devicehosting
 */

/*!
 * \defgroup hupnp_devicehosting Device Hosting
 * \ingroup hupnp_core
 *
 * \brief This page explains the concept of device hosts, which encapsulate the
 * technical details of UPnP networking.
 *
 * \section notesaboutdesign A few notes about the design
 *
 * The logical core of HUPnP is divided into two major modules; a collection of
 * classes that enable the \e hosting of UPnP device model and the collection of
 * classes that form up the \ref hupnp_devicemodel. The separation is very distinct. The
 * device hosts provide the technical foundation for the UPnP networking. They
 * encapsulate and implement the protocols the UPnP Device Architecture
 * specification details. The device model, on the other hand, is about the logical
 * structure of the UPnP core concepts, which is clearly independent of the
 * technical details of communication. Because of this HUPnP uses highly similar
 * device models both at the server and client side.
 *
 * HUPnP introduces two types of \e hosts.
 * \li The Herqq::Upnp::HDeviceHost is the class
 * that enables UPnP devices to be published for UPnP control points to use.
 * \li The Herqq::Upnp::HControlPoint is the class that enables the discovery
 * and use of UPnP devices that are available on the network.
 *
 * The difference between these two classes is important to notice.
 * You could picture an HDeviceHost as a server and an HControlPoint as a
 * client. The \c %HDeviceHost \e publishes instances of HServerDevice for
 * UPnP control points to use and the \c %HControlPoint \e uses instances of
 * HClientDevice to communicate with UPnP devices. But as implied,
 * the APIs of client and server side device models are very similar and once you
 * get familiar either one, using the other should be simple as well.
 *
 * \note While a Herqq::Upnp::HServerDevice published by a \c %HDeviceHost is always
 * usable by UPnP control points over the network, the same device can also
 * be accesed and used simultaneously in process.
 * See HDeviceHost for more information.
 *
 * \section basicuse Basic use
 *
 * The basic use of the \c %HDeviceHost is straightforward.
 * You only need to initialize it by providing information that enables one or
 * more UPnP devices to be created and hosted.
 *
 * In other words, you could create and initialize an HDeviceHost like this:
 *
 * \code
 *
 * #include <HUpnpCore/HDeviceHost>
 * #include <HUpnpCore/HDeviceModelCreator>
 *
 * #include "my_hserverdevice.h" // your code
 *
 * namespace
 * {
 * class MyDeviceModelCreator :
 *     public Herqq::Upnp::HDeviceModelCreator
 * {
 * private:
 *
 *     // overridden from HDeviceModelCreator
 *     virtual MyDeviceModelCreator* newInstance() const
 *     {
 *         return new MyDeviceModelCreator();
 *     }
 *
 * public:
 *
 *     // overridden from HDeviceModelCreator
 *     virtual Herqq::Upnp::HServerDevice* createDevice(
 *         const Herqq::Upnp::HDeviceInfo& info) const
 *     {
 *         // You should check the info object to see what object HUPnP wants
 *         // created and return null if your creator cannot create it.
 *         return new MyHServerDevice(); // your class derived from HServerDevice
 *     }
 *
 *     // overridden from HDeviceModelCreator
 *     virtual HServerService* createService(
 *         const Herqq::Upnp::HServiceInfo& serviceInfo,
 *         const Herqq::Upnp::HDeviceInfo& parentDeviceInfo) const
 *     {
 *         // You should check the info objects to see what object HUPnP wants
 *         // created and return null if your creator cannot create it.
 *         return new MyHServerService();
 *     }
 * };
 *
 * Herqq::Upnp::HDeviceHost* createDeviceHost()
 * {
 *     Herqq::Upnp::HDeviceHostConfiguration hostConf;
 *     hostConf.setDeviceModelCreator(MyDeviceModelCreator());
 *     // This specifies the factory type that the HDeviceHost uses to create
 *     // HServerDevice and HServerService instances.
 *
 *     Herqq::Upnp::HDeviceConfiguration deviceConf;
 *     deviceConf.setPathToDeviceDescription("my_hdevice_devicedescription.xml");
 *     // This is the device description for our custom UPnP device type
 *     // the device host uses this file to build the device tree.
 *
 *     hostConf.add(deviceConf);
 *     // The same HDeviceHost can host multiple UPnP root devices at the same time.
 *     // To do that you only need to create and add other HDeviceConfiguration
 *     // objects to the HDeviceHostConfiguration instance as shown above.
 *
 *     Herqq::Upnp::HDeviceHost* deviceHost = new HDeviceHost();
 *     if (!deviceHost->init(hostConf))
 *     {
 *         // The initialization failed. Perhaps something should be done?
 *         // You can call error() to check the type of the error and errorDescription()
 *         // to get a human-readable description of the error.
 *     }
 *
 *     return deviceHost;
 * }
 * }
 *
 * \endcode
 *
 * and an HControlPoint like this:
 *
 * \code
 *
 * #include <HUpnpCore/HControlPoint>
 *
 * Herqq::Upnp::HControlPoint* createControlPoint()
 * {
 *     Herqq::Upnp::HControlPoint* controlPoint = new HControlPoint();
 *
 *     if (!controlPoint->init())
 *     {
 *         // The initialization failed. Perhaps something should be done?
 *         // You can call error() to check the type of the error and errorDescription()
 *         // to get a human-readable description of the error.
 *     }
 *
 *     return controlPoint;
 * }
 * }
 *
 * \endcode
 *
 * The above shows the simplest way to initialize an HControlPoint instance.
 * However, you can configure the behavior of an \c %HControlPoint instance
 * in various ways by providing it an HControlPointConfiguration instance upon
 * construction.
 *
 * \sa Herqq::Upnp::HDeviceHost, Herqq::Upnp::HControlPoint, hupnp_devicemodel
 */

/*!
 * \defgroup hupnp_core HUPnP Core
 * HUPnP Core is a library that provides an implementation of the
 * <a href="http://www.upnp.org/specs/arch/UPnP-arch-DeviceArchitecture-v1.1.pdf">
 * UPnP Device Architecture version 1.1 specification</a>.
 */

/*!
 * \defgroup hupnp_common Common
 * \ingroup hupnp_core
 */

QString upnpErrorCodeToString(qint32 errCode)
{
    QString retVal;
    switch(errCode)
    {
    case UpnpSuccess:
        retVal = "Success";
        break;
    case UpnpInvalidAction:
        retVal = "InvalidAction";
        break;
    case UpnpInvalidArgs:
        retVal = "InvalidArgs";
        break;
    case UpnpActionFailed:
        retVal = "ActionFailed";
        break;
    case UpnpArgumentValueInvalid:
        retVal = "ArgumentValueInvalid";
        break;
    case UpnpArgumentValueOutOfRange:
        retVal = "ArgumentValueOutOfRange";
        break;
    case UpnpOptionalActionNotImplemented:
        retVal = "OptionalActionNotImplemented";
        break;
    case UpnpOutOfMemory:
        retVal = "OutOfMemory";
        break;
    case UpnpHumanInterventionRequired:
        retVal = "HumanInterventionRequired";
        break;
    case UpnpStringArgumentTooLong:
        retVal = "StringArgumentTooLong";
        break;
    case UpnpUndefinedFailure:
        retVal = "UndefinedFailure";
        break;
    default:
        retVal = QString::number(errCode);
        break;
    }
    return retVal;
}

void SetLoggingLevel(HLogLevel level)
{
    HLogger::setTraceLevel(static_cast<HLogger::HLogLevel>(level));
}

void EnableNonStdBehaviourWarnings(bool arg)
{
    HLogger::enableNonStdWarnings(arg);
}

QString readElementValue(
    const QString elementTagToSearch, const QDomElement& parentElement,
    bool* wasDefined)
{
    QDomElement element =
        parentElement.firstChildElement(elementTagToSearch);

    if (element.isNull())
    {
        if (wasDefined)
        {
            *wasDefined = false;
        }

        return "";
    }

    if (wasDefined)
    {
        *wasDefined = true;
    }

    return element.text();
}

QString toString(const QDomElement& e)
{
    QString buf;
    QTextStream ts(&buf, QIODevice::ReadWrite);
    e.save(ts, 0);

    return buf;
}

/*******************************************************************************
 * HSysInfo
 *******************************************************************************/
QScopedPointer<HSysInfo> HSysInfo::s_instance;
QMutex HSysInfo::s_initMutex;

HSysInfo::HSysInfo()
{
    createProductTokens();
    createLocalNetworks();
}

HSysInfo::~HSysInfo()
{
}

HSysInfo& HSysInfo::instance()
{
    QMutexLocker lock(&s_initMutex);

    if (s_instance)
    {
        return *s_instance;
    }

    s_instance.reset(new HSysInfo());
    return *s_instance;
}

void HSysInfo::createProductTokens()
{
#if defined(Q_OS_WIN)
    QString server = "MicrosoftWindows/";
    switch(QSysInfo::WindowsVersion)
    {
    case QSysInfo::WV_2000:
        server.append("5.0");
        break;
    case QSysInfo::WV_XP:
        server.append("5.1");
        break;
    case QSysInfo::WV_2003:
        server.append("5.2");
        break;
    case QSysInfo::WV_VISTA:
        server.append("6.0");
        break;
    case QSysInfo::WV_WINDOWS7:
        server.append("6.1");
        break;
    default:
        server.append("-1");
    }
#elif defined(Q_OS_DARWIN)
    QString server = "AppleMacOSX/";
    switch(QSysInfo::MacintoshVersion)
    {
    case QSysInfo::MV_10_3:
        server.append("10.3");
        break;
    case QSysInfo::MV_10_4:
        server.append("10.4");
        break;
    case QSysInfo::MV_10_5:
        server.append("10.5");
        break;
    case QSysInfo::MV_10_6:
        server.append("10.6");
        break;
    default:
        server.append("-1");
    }
#elif defined(Q_OS_LINUX)
    QString server;
    struct utsname sysinfo;
    if (!uname(&sysinfo))
    {
        server = QString("%1/%2").arg(sysinfo.sysname, sysinfo.release);
    }
    else
    {
        server = "Undefined/-1";
    }
#else
    QString server = "Undefined/-1";
#endif

    m_productTokens.reset(
        new HProductTokens(QString("%1 UPnP/1.1 HUPnP/%2.%3 DLNADOC/1.50").arg(
            server, STRX(HUPNP_CORE_MAJOR_VERSION), STRX(HUPNP_CORE_MINOR_VERSION))));
}

QList<QPair<quint32, quint32> > HSysInfo::createLocalNetworks()
{
    QList<QPair<quint32, quint32> > retVal;
    foreach(const QNetworkInterface& iface, QNetworkInterface::allInterfaces())
    {
        QList<QNetworkAddressEntry> entries = iface.addressEntries();
        foreach(const QNetworkAddressEntry& entry, entries)
        {
            QHostAddress ha = entry.ip();
            if (ha.protocol() != QAbstractSocket::IPv4Protocol)
            {
                continue;
            }

            quint32 nm = entry.netmask().toIPv4Address();
            retVal.append(qMakePair(ha.toIPv4Address() & nm, nm));
        }
    }
    return retVal;
}

bool HSysInfo::localNetwork(const QHostAddress& ha, quint32* retVal) const
{
    Q_ASSERT(retVal);

    QList<QPair<quint32, quint32> > localNetworks = createLocalNetworks();

    QList<QPair<quint32, quint32> >::const_iterator ci;
    for(ci = localNetworks.begin(); ci != localNetworks.end(); ++ci)
    {
        if ((ha.toIPv4Address() & ci->second) == ci->first)
        {
            *retVal = ci->first;
            return true;
        }
    }

    return false;
}

bool HSysInfo::isLocalAddress(const QHostAddress& ha) const
{
    quint32 tmp;
    return localNetwork(ha, &tmp);
}

bool HSysInfo::areLocalAddresses(const QList<QHostAddress>& addresses) const
{
    QList<QHostAddress> localAddresses = QNetworkInterface::allAddresses();
    foreach(const QHostAddress& ha, addresses)
    {
        bool matched = false;
        foreach(const QHostAddress& localAddress, localAddresses)
        {
            if (localAddress == ha)
            {
                matched = true;
                break;
            }
        }

        if (!matched)
        {
            return false;
        }
    }

    return true;
}

HEndpoints convertHostAddressesToEndpoints(const QList<QHostAddress>& addrs)
{
    HEndpoints retVal;
    foreach(const QHostAddress& ha, addrs)
    {
        retVal.append(HEndpoint(ha));
    }
    return retVal;
}

bool verifyName(const QString& name, QString* err)
{
    HLOG(H_AT, H_FUN);

    if (name.isEmpty())
    {
        if (err)
        {
            *err = "[name] cannot be empty";
        }
        return false;
    }

    if (!name[0].isLetterOrNumber() && name[0] != '_')
    {
        if (err)
        {
            *err = QString("[name: %1] has invalid first character").arg(name);
        }
        return false;
    }

    foreach(const QChar& c, name)
    {
        if (!c.isLetterOrNumber() && c != '_' && c != '.')
        {
            if (err)
            {
                *err = QString(
                    "[name: %1] contains invalid character(s)").arg(name);
            }
            return false;
        }
    }

    if (name.size() > 32)
    {
        HLOG_WARN(QString("[name: %1] longer than 32 characters").arg(name));
    }

    return true;
}

QString urlsAsStr(const QList<QUrl>& urls)
{
    QString retVal;

    for(qint32 i = 0; i < urls.size(); ++i)
    {
        retVal.append(QString("#%1 %2\n").arg(
            QString::number(i), urls[i].toString()));
    }

    return retVal;
}

QString extractBaseUrl(const QString& url)
{
    if (url.endsWith('/'))
    {
        return url;
    }
    else if (!url.contains('/'))
    {
        return "";
    }

    QString base = url.section('/', 0, -2, QString::SectionIncludeTrailingSep);
    return base;
}

QUrl resolveUri(const QUrl& baseUrl, const QUrl& other)
{
    QString otherReq(extractRequestPart(other));

    if (otherReq.startsWith('/'))
    {
        return QString("%1%2").arg(extractHostPart(baseUrl), otherReq);
    }

    QString basePath(baseUrl.toString());

    if (!basePath.endsWith('/'))  { basePath.append('/'); }
    if (otherReq.startsWith('/')) { otherReq.remove(0, 1); }

    basePath.append(otherReq);

    return basePath;
}

QUrl appendUrls(const QUrl& baseUrl, const QUrl& other)
{
    QString otherReq(extractRequestPart(other));

    QString basePath(baseUrl.toString());

    if (!basePath.endsWith('/'))  { basePath.append('/'); }
    if (otherReq.startsWith('/')) { otherReq.remove(0, 1); }

    basePath.append(otherReq);

    return basePath;
}

}
}
