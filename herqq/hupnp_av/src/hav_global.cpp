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

#include "hav_global.h"
#include <HUpnpAv/HStateVariableCollection>

#include <HUpnpCore/private/hlogger_p.h>

/*!
 * \namespace Herqq::Upnp::Av
 * The main namespace that contains all of the HUPnP A/V enumerations, typedefs,
 * functions and classes.
 */

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \defgroup hupnp_av HUPnP A/V
 * HUPnPAv is a library that provides implementations of various components
 * defined in the <a href=http://upnp.org/specs/av/UPnP-av-AVArchitecture-v1.pdf>
 * UPnP AV Architecture document</a>.
 *
 * UPnP AV Architecture is about the
 * <a href=http://upnp.org/specs/av/UPnP-av-MediaServer-v3-Device.pdf>
 * Media Server</a>,
 * <a href=http://upnp.org/specs/av/UPnP-av-MediaRenderer-v2-Device.pdf>
 * Media Renderer</a> and their use from the client-side. The main purpose of a
 * \e Media \e Server is to make media content available to others and the main purpose
 * of a \e Media \e Renderer is to play that content. While that may not sound
 * too complicated, the UPnP AV Architecture is actually fairly large and complex.
 *
 * The purpose of HUPnPAv is to provide a fully standard-compliant
 * implementation of the UPnP AV Architecture with a simple and extendable API.
 * HUPnPAv implements all the mandatory functionality of the UPnP AV Architecture
 * and many of the optional features too. Furthermore, HUPnPAv provides extensive
 * support for using the UPnP AV devices from the client-side.
 *
 * \section clientside Client-Side Components
 *
 * To browse a ContentDirectory or to control an AVTransport of a Media Renderer
 * you first need to find the appropriate UPnP devices from the network.
 * HAvControlPoint is most often the best choice for this, although you can
 * do this by using the HControlPoint from the HUPnP core library as well. Regardless,
 * once you have found an appropriate UPnP device for your needs, you can take
 * advantage of the convenience classes HUPnPAv provides to simplify
 * the usage of the device.
 *
 * \subsection cds_browsing_example ContentDirectory browsing example
 *
 * Let's say you want to find Media Servers on the network and
 * browse their contents. At this point, you are not interested in rendering
 * content, so Media Renderers should be ignored. To do this you can setup an
 * HAvControlPoint to discover Media Servers on the network:
 *
 * \code
 *
 *  Herqq::Upnp::Av::HAvControlPointConfiguration configuration;
 *  configuration.setInterestingDeviceTypes(HAvControlPointConfiguration::MediaServer);
 *  Herqq::Upnp::Av::HAvControlPoint controlPoint(configuration);
 *  controlPoint.init();
 *
 * \endcode
 *
 * Once the HAvControlPoint is setup you can wait for its
 * HAvControlPoint::mediaServerOnline() signal to be emitted, or you can
 * call HAvControlPoint::mediaServers() to retrieve already discovered
 * Media Servers. Assuming you have found a Media Server device, you can
 * use the HMediaServerAdapter class to retrieve an instance of HContentDirectoryAdapter,
 * which you can use to browse the server-side ContentDirectory service:
 *
 * \code
 *
 * Herqq::Upnp::Av::HAvControlPoint* cp = ...;
 * Herqq::Upnp::Av::HMediaServerAdapters mediaServers = cp->mediaServers();
 * if (mediaServers.size() == 0)
 * {
 *     return;
 * }
 *
 * Herqq::Upnp::Av::HContentDirectoryAdapter* cds = mediaServer.contentDirectory();
 *
 * \endcode
 *
 * HContentDirectoryAdapter provides asynchronous helper methods for invoking
 * every action defined in the <c>ContentDirectory:3</c> specification.
 * So for instance an asynchronous \e browse could be done
 * with HContentDirectoryAdapter::browse(). In any case, HContentDirectoryAdapter
 * provides a semantically identical interface with the interface defined in the
 * <c>ContentDirectory:3</c> specification. This may be what you wanted, but it requires
 * that you have a fair understanding of the ContentDirectory specification.
 * Some operations also take some work, as is the case with browse. For instance,
 * if you wanted to browse the entire contents of a ContentDirectory, the number
 * of HContentDirectoryAdapter::browse() calls depends of the hierarchy of
 * CDS objects in the ContentDirectory. You also have to interpret and quite
 * possibly serialize the retrieved DIDL-Lite documents into some type of an
 * object model to actually work with the data.
 *
 * As browsing a ContentDirectory is a very common operation to do, HUPnPAv
 * provides a convenience class for it, HMediaBrowser:
 *
 * \code

 * Herqq::Upnp::Av::HContentDirectoryAdapter* cd = ...;
 *
 * Herqq::Upnp::Av::HMediaBrowser mediaBrowser;
 * if (!mediaBrowser.reset(cd))
 * {
 *     // ERROR: the provided HContentDirectoryAdapter could not be used.
 *     return;
 * }
 *
 * if (!mediaBrowser.browseAll())
 * {
 *     // ERROR: the operation could not be started
 * }
 *
 * \endcode
 *
 * HMediaBrowser emits a HMediaBrowser::objectsBrowsed() signal when progress
 * has been made and a HMediaBrowser::browseComplete() signal when a browse
 * operation has been successfully completed. HMediaBrowser caches the retrieved
 * data, but rather than storing a bunch of XML documents, HMediaBrowser utilizes
 * the HUPnPAv's CDS Object Model and a class for storing instances of it,
 * HCdsDataSource:
 *
 * \code
 *
 * Herqq::Upnp::Av::HMediaBrowser* mb = ...
 *
 * Herqq::Upnp::Av::HCdsDataSource* ds = mb->dataSource();
 *
 * Herqq::Upnp::Av::HObjects cdsObjects = ds->objects;
 *
 * \endcode
 *
 * With the help of HCdsDataSource you can easily browse through all the objects
 * the HMediaBrowser has retrieved from the ContentDirectory service. For more
 * information about the HUPnPAv's CDS object model, see \ref hupnp_av_cds_objects.
 *
 * \note Although the example above was mostly about HUPnPAv's convenience classes
 * relating to a ContentDirectory of a Media Server, HUPnPAv provides similar
 * convenience classes for other services and to the use of a Media Renderer
 * as well, which is the topic of the next example.
 *
 * \subsection controlling_mr_example Controlling MediaRenderer example
 *
 * As with the example above, you start by creating and initializing
 * HAvControlPoint. However, this time you are most likely interested in both
 * MediaServers and MediaRenderers, as controlling MediaRenderers often involves
 * specifying what content they should play and how.
 * Because this is the default behavior of HAvControlPoint you can use the
 * default configuration, unless you wish to modify some of the other parameters.
 *
 * \code
 *
 * Herqq::Upnp::Av::HAvControlPoint controlPoint;
 * controlPoint.init();
 *
 * \endcode
 *
 * When the control point detects a MediaRenderer on the network it emits
 * HAvControlPoint::mediaRendererOnline() signal. You can also call
 * HAvControlPoint::mediaRenderers() to get all the MediaRenderers the control
 * point has already discovered:
 *
 * \code
 *
 * Herqq::Upnp::Av::HAvControlPoint* cp = ...
 * Herqq::Upnp::Av::HMediaRendererAdapters mediaRenderers = cp->mediaRenderers();
 *
 * \endcode
 *
 * You can use HMediaRendererAdapter to get an HConnection, which represents a connection
 * to the MediaRenderer device and from which you can retrieve the virtual
 * AVTransport and RenderingControl instances. The virtual AVT and RCS instances
 * in turn provide the actions for controlling the MediaRenderer device.
 *
 * \code
 *
 * Herqq::Upnp::Av::HMediaRendererAdapter* mediaRenderer = ...
 * if (!mediaRenderer->getCurrentConnections())
 * {
 *     // ERROR: the asynchronous operation failed to start.
 * }
 *
 * // The above call attempts to retrieve every existing connection from the
 * // device. For each existing connection the
 * // HMediaRendererAdapter::connectionReady() signal is emitted once.
 * // After the HMediaRendererAdapter has retrieved one or more connections, you can
 * // get them with HMediaRendererAdapter::takeConnections():
 *
 * Herqq::Upnp::Av::HConnections connections = mediaRenderer->takeConnections();
 *
 * // Assuming the list is not empty:
 * Herqq::Upnp::Av::HConnection* connection = connections.at(0);
 *
 * Herqq::Upnp::Av::HAvTransportAdapter* virtualAvt = connection->transport();
 * Herqq::Upnp::Av::HRenderingControlAdapter* virtualRcs = connection->renderingControl();
 *
 * \endcode
 *
 * HAvTransportAdapter and HRenderingControlAdapter provide asynchronous
 * helper methods for invoking every action defined in the AVTransport:2
 * and RenderingControl:2 specifications. They both provide semantically identical
 * interfaces with the interfaces defined in their respective specifications.
 *
 * \section serverside Server-Side Components
 *
 * For most common use cases, HUPnPAv provides the HAvDeviceModelCreator class, which
 * provides implementations for a MediaRenderer and MediaServer devices.
 * \ref hupnp_devicehosting details how to host a UPnP device and how a
 * <em>device model creator</em> relates to that. However, setting up a Media
 * Server or a Media Renderer is a bit more involved than that. With a
 * Media Server you have to configure a ContentDirectory for operation, which
 * means that you have to provide at least the data it is supposed to serve.
 * With a Media Renderer you have to provide a component that does the actual
 * rendering of media content.
 *
 * As an example of setting up a Media Server,
 *
 * \code
 *
 * #include <HUpnpAv/HRootDir>
 * #include <HUpnpAv/HFileSystemDataSource>
 * #include <HUpnpAv/HMediaServerDeviceConfiguration>
 * #include <HUpnpAv/HFileSystemDataSourceConfiguration>

 * #include <HUpnpCore/HDeviceHost>
 *
 * namespace
 * {
 *
 * // This function returns the information the HUPnPAv's default Media Server
 * // implementation requires.
 * Herqq::Upnp::Av::HMediaServerDeviceConfiguration getMediaServerConfig()
 * {
 *     Herqq::Upnp::Av::HFileSystemDataSourceConfiguration dataSourceConfig;
 *     dataSourceConfig.addRootDir(Herqq::Upnp::Av::HRootDir(
 *          QDir("C:/Herqq/MediaServerTestData"), Herqq::Upnp::Av::HRootDir::RecursiveScan));
 *
 *     // The above directory will be scanned recursively for media content
 *     // when the HMediaServerDevice is initialized by the HDeviceHost.
 *
 *     Herqq::Upnp::Av::HFileSystemDataSource* dataSource =
 *         new Herqq::Upnp::Av::HFileSystemDataSource(dataSourceConfig);
 *
 *     // As the name implies, this data source scans the local file system
 *     // for media content. However, there could be other data sources that
 *     // retrieve the content from elsewhere.
 *     // See Herqq::Upnp::Av::HAbstractCdsDataSource for more information.
 *
 *     HContentDirectoryServiceConfiguration cdsConf;
 *     cdsConf.setDataSource(dataSource, true);
 *     // This associates the created data source with a ContentDirectory, and it
 *     // specifies that the ContentDirectory implementation should take the
 *     // ownership of the data source.
 *
 *     Herqq::Upnp::Av::HMediaServerDeviceConfiguration mediaServerConfig;
 *     mediaServerConfig.setContentDirectoryConfiguration(cdsConf);
 *
 *     return mediaServerConfig;
 * }
 *
 * Herqq::Upnp::HDeviceHost* createDeviceHost()
 * {
 *     Herqq::Upnp::HDeviceConfiguration deviceConf;
 *     deviceConf.setPathToDeviceDescription("mediaserver_description.xml");
 *
 *     Herqq::Upnp::HDeviceHostConfiguration hostConfig;
 *
 *     Herqq::Upnp::Av::HAvDeviceModelCreator avCreator;
 *     avCreator.setMediaServerConfiguration(getMediaServerConfig());
 *
 *     hostConfig.setDeviceModelCreator(avCreator);
 *     hostConfig.add(deviceConf);
 *
 *     Herqq::Upnp::HDeviceHost* deviceHost = new HDeviceHost();
 *     if (!deviceHost->init(hostConfig))
 *     {
 *         // the initialization failed. perhaps something should be done?
 *         // call error() to check the type of the error and errorDescription()
 *         // to get a human-readable description of the error.
 *     }
 *
 *     return deviceHost;
 * }
 * }
 *
 * \endcode
 *
 * And an example of setting up a Media Renderer,
 *
 * \code
 *
 * #include "myrendererconnection.h" // this would be your code
 *
 * #include <HUpnpAv/HMediaRendererDevice>
 * #include <HUpnpAv/HRendererConnectionManager>
 * #include <HUpnpAv/HMediaRendererDeviceConfiguration>

 * #include <HUpnpCore/HDeviceHost>
 *
 * namespace
 * {
 *
 * class MyRendererConnectionManager :
 *   public Herqq::Upnp::Av::HRendererConnectionManager
 * {
 * protected:
 *
 *   Herqq::Upnp::Av::HRendererConnection* doCreate(const QString&, qint32)
 *   {
 *       return new MyRendererConnectionManager();
 *   }
 *
 * public:
 *
 *   MyRendererConnectionManager()
 *   {
 *   }
 * };
 *
 * // This function returns the information the HUPnPAv's default Media Renderer
 * // implementation requires.
 * Herqq::Upnp::Av::HMediaRendererDeviceConfiguration getMediaRendererConfig()
 * {
 *     Herqq::Upnp::Av::HMediaRendererDeviceConfiguration mediaRendererConfig;
 *     mediaRendererConfig.setRendererConnectionManager(new MyRendererConnectionManager(), true);
 *
 *     // The above two lines are about associating a media renderer device configuration
 *     // with an object capable of creating HRendererConnection instances, which
 *     // ultimately are the objects doing the actual rendering of media content.
 *
 *     return mediaRendererConfig;
 * }
 *
 * Herqq::Upnp::HDeviceHost* createDeviceHost()
 * {
 *     Herqq::Upnp::HDeviceConfiguration deviceConf;
 *     deviceConf.setPathToDeviceDescription("mediarenderer_description.xml");
 *
 *     Herqq::Upnp::HDeviceHostConfiguration hostConfig;
 *
 *     Herqq::Upnp::Av::HAvDeviceModelCreator avCreator;
 *     avCreator.setMediaRendererConfiguration(getMediaRendererConfig());
 *
 *     hostConfig.setDeviceModelCreator(avCreator);
 *     hostConfig.add(deviceConf);
 *
 *     Herqq::Upnp::HDeviceHost* deviceHost = new HDeviceHost();
 *     if (!deviceHost->init(hostConfig))
 *     {
 *         // the initialization failed. perhaps something should be done?
 *         // call error() to check the type of the error and errorDescription()
 *         // to get a human-readable description of the error.
 *     }
 *
 *     return deviceHost;
 * }
 * }
 *
 * \endcode
 *
 * See \ref hupnp_devicehosting and HDeviceHost for more information about hosting
 * a UPnP device.
 *
 * In addition to the default A/V device implementations, HUPnPAv provides
 * base classes for each of the services defined in
 * the <c>MediaServer:3</c> and <c>MediaRenderer:2</c> specifications. These
 * classes marshal action requests into virtual method calls and user provided
 * values back into action responses, so as a user you only need to override a
 * virtual method to provide an implementation for an action. These classes are
 * ideal when you want to have full control of a Media Server or a Media Renderer.
 * The obvious downside is that this may require much more work from your part.
 *
 * For instance, if you are supposed to implement a custom ConnectionManager,
 * you should consider using HAbstractConnectionManagerService as a base class.
 * As defined in the <c>ConnectionManager:2</c> specification,
 * HAbstractConnectionManagerService has three pure virtual methods
 * (representing mandatory UPnP actions) to be implemented:
 * HAbstractConnectionManagerService::getProtocolInfo(),
 * HAbstractConnectionManagerService::getCurrentConnectionIDs() and
 * HAbstractConnectionManagerService::getCurrentConnectionInfo(). The other two
 * virtual methods, HAbstractConnectionManagerService::prepareForConnection() and
 * HAbstractConnectionManagerService::connectionComplete() are defined
 * in the specification as optional and as such HAbstractConnectionManagerService
 * provides a default implementation for them, always responding to the caller
 * that the "actions" are not implemented.
 *
 * \code
 *
 * // myconnectionmanager.h
 *
 * #include <HUpnpAv/HAbstractConnectionManagerService>
 *
 * class MyConnectionManager :
 *     public Herqq::Upnp::Av::HAbstractConnectionManagerService
 * {
 *    public:
 *        virtual qint32 getProtocolInfo(HProtocolInfoResult* result);
 *        virtual qint32 getCurrentConnectionIDs(QList<quint32>* outArg);
 *        virtual qint32 getCurrentConnectionInfo(
 *            qint32 connectionId, Herqq::Upnp::Av::HConnectionInfo* outArg);
 *
 * };
 *
 * // myconnectionmanager.cpp
 *
 * qint32 MyConnectionManager::getProtocolInfo(HProtocolInfoResult* result)
 * {
 *     // provide an implementation for this action
 *     return Herqq::Upnp::UpnpSuccess;
 * }
 *
 * qint32 MyConnectionManager::getCurrentConnectionIDs(QList<quint32>* outArg)
 * {
 *     // provide an implementation for this action
 *     return Herqq::Upnp::UpnpSuccess;
 * }
 *
 * qint32 MyConnectionManager::getCurrentConnectionInfo(
 *     qint32 connectionId, Herqq::Upnp::Av::HConnectionInfo* outArg)
 * {
 *     // provide an implementation for this action
 *     return Herqq::Upnp::UpnpSuccess;
 * }
 *
 * \endcode
 *
 * To put the implementations of classes such as the just shown \c MyConnectionManager
 * into action, you need to to create a custom HDeviceModelCreator that knows
 * how and when to create instances of your classes and provide that to the
 * HDeviceHost as discussed previously. Certainly you could derive
 * from HAvDeviceModelCreator to take advantage of the default device and service
 * implementations provided by HUPnPAv too.
 *
 * \section where_to_now What Next?
 *
 * HUPnPAv types are organized by functionality and domain, so for instance if you
 * want to implement a server-side ContentDirectory or you wish to use one from
 * the client-side, see \ref hupnp_av_cds for more information. Inside such a
 * module, you'll find:
 * - generic types that are used both the server and client-side,
 * - types that have the suffix \e Device,
 * - types that have the suffix \e Service and
 * - types that have the suffix \e Adapter.
 *
 * In short, types that have the suffix \e Device represent \b server-side
 * UPnP A/V device implementations. Types that have the suffix \e Service represent
 * \b server-side UPnP A/V service implementations. Types that have the suffix
 * \e Adapter represent \b client-side classes used for interacting with
 * server-side components.
 */

/*!
 * \defgroup hupnp_av_common Common
 * \ingroup hupnp_av
 */

QString toString(HDayOfWeek arg, HDayOfWeekFormat format)
{
    QString retVal;
    switch (arg)
    {
    case Monday:
        retVal = format == Full ? "Monday" : "Mon";
        break;
    case Tuesday:
        retVal = format == Full ? "Tuesday" : "Tue";
        break;
    case Wednesday:
        retVal = format == Full ? "Wedneday" : "Wed";
        break;
    case Thursday:
        retVal = format == Full ? "Thursday" : "Thu";
        break;
    case Friday:
        retVal = format == Full ? "Friday" : "Fri";
        break;
    case Saturday:
        retVal = format == Full ? "Saturday" : "Sat";
        break;
    case Sunday:
        retVal = format == Full ? "Sunday" : "Sun";
        break;
    default:
        retVal = "Undefined";
        break;
    }
    return retVal;
}

HDayOfWeek dayOfWeekFromString(const QString& arg)
{
    HDayOfWeek retVal = Undefined_DayOfWeek;
    if (arg.startsWith("mon", Qt::CaseInsensitive))
    {
        retVal = Monday;
    }
    else if (arg.startsWith("Tue", Qt::CaseInsensitive))
    {
        retVal = Tuesday;
    }
    else if (arg.startsWith("wed", Qt::CaseInsensitive))
    {
        retVal = Wednesday;
    }
    else if (arg.startsWith("thu", Qt::CaseInsensitive))
    {
        retVal = Thursday;
    }
    else if (arg.startsWith("fri", Qt::CaseInsensitive))
    {
        retVal = Friday;
    }
    else if (arg.startsWith("sat", Qt::CaseInsensitive))
    {
        retVal = Saturday;
    }
    else if (arg.startsWith("sun", Qt::CaseInsensitive))
    {
        retVal = Sunday;
    }
    return retVal;
}

QString toString(HEpisodeType arg)
{
    QString retVal;
    switch (arg)
    {
    case EpisodeTypeAll:
        retVal = "ALL";
        break;
    case EpisodeTypeFirstRun:
        retVal = "FIRST-RUN";
        break;
    case EpisodeTypeRepeat:
        retVal = "REPEAT";
        break;
    default:
        break;
    }
    return retVal;
}

HEpisodeType episodeTypeFromString(const QString& arg)
{
    HEpisodeType retVal = EpisodeTypeUndefined;
    if (arg == "ALL")
    {
        retVal = EpisodeTypeAll;
    }
    else if (arg == "FIRST-RUN")
    {
        retVal = EpisodeTypeFirstRun;
    }
    else if (arg == "REPEAT")
    {
        retVal = EpisodeTypeRepeat;
    }
    return retVal;
}

QString toString(HDaylightSaving arg)
{
    QString retVal;
    switch (arg)
    {
    case DaylightSaving:
        retVal = "DAYLIGHTSAVING";
        break;
    case Standard_DaylightSaving:
        retVal = "STANDARD";
        break;
    case Unknown_DaylightSaving:
    default:
        retVal = "UNKNOWN";
        break;
    }
    return retVal;
}

HDaylightSaving daylightSavingFromString(const QString& arg)
{
    HDaylightSaving retVal = Unknown_DaylightSaving;
    if (arg == "DAYLIGHTSAVING")
    {
        retVal = DaylightSaving;
    }
    else if (arg == "STANDARD")
    {
        retVal = Standard_DaylightSaving;
    }
    return retVal;
}

namespace
{
void addNamespaces(QXmlStreamReader& reader)
{
    QXmlStreamNamespaceDeclaration def(
        "", "urn:schemas-upnp-org:av:avs");
    QXmlStreamNamespaceDeclaration xsi(
        "xsi", "http://www.w3.org/2001/XMLSchema-instance");
    QXmlStreamNamespaceDeclaration xsiSchemaLocation(
        "xsi:schemaLocation", "urn:schemas-upnp-org:av:avs\r\nhttp://www.upnp.org/schemas/av/avs.xsd");

    reader.addExtraNamespaceDeclaration(def);
    reader.addExtraNamespaceDeclaration(xsi);
    reader.addExtraNamespaceDeclaration(xsiSchemaLocation);
}
}

HStateVariableCollection parseStateVariableCollection(const QString& arg)
{
    HLOG(H_AT, H_FUN);

    HStateVariableCollection retVal;

    QXmlStreamReader reader(arg.trimmed());
    addNamespaces(reader);

    if (!reader.readNextStartElement() ||
        reader.name().compare("stateVariableValuePairs", Qt::CaseInsensitive) != 0)
    {
        return retVal;
    }

    while(!reader.atEnd() && reader.readNextStartElement())
    {
        QStringRef name = reader.name();
        if (name == "stateVariable")
        {
            QXmlStreamAttributes attrs = reader.attributes();
            if (!attrs.hasAttribute(QString("variableName")))
            {
                HLOG_WARN(QString(
                    "Ignoring state variable value pair definition that "
                    "lacks the [variableName] attribute."));
            }
            else
            {
                QString channel = attrs.value("channel").toString();
                QString svName = attrs.value("variableName").toString();
                QString value = reader.readElementText().trimmed();

                HStateVariableData element(svName, value, channel);
                if (element.isValid())
                {
                    retVal.add(element);
                }
            }
        }
        else
        {
            HLOG_WARN(QString("Encountered unknown XML element: [%1]").arg(name.toString()));
        }
    }

    return retVal;
}


}
}
}
