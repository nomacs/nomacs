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

#ifndef HDEVICEHOST_P_H_
#define HDEVICEHOST_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hdevicehost.h"

#include "../hdevicestorage_p.h"

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>

class QTimer;
class QNetworkAccessManager;

namespace Herqq
{

namespace Upnp
{

class HDeviceHost;
class HServerDevice;
class HDeviceStatus;
class HEventNotifier;
class PresenceAnnouncer;
class HDeviceHostHttpServer;
class HDeviceHostSsdpHandler;
class HServerDeviceController;
class HDeviceHostConfiguration;


//
// Implementation details of HDeviceHost class
//
class HDeviceHostPrivate :
    public QObject
{
Q_OBJECT
H_DECLARE_PUBLIC(HDeviceHost)
H_DISABLE_COPY(HDeviceHostPrivate)

private:

    void connectSelfToServiceSignals(HServerDevice* device);

public: // attributes

    const QByteArray m_loggingIdentifier;
    // The prefix shown before the actual log output

    QScopedPointer<HDeviceHostConfiguration> m_config;
    // The configuration of this instance

    QList<HDeviceHostSsdpHandler*> m_ssdps;
    // An SSDP listener /sender for each configured network interface

    QScopedPointer<HDeviceHostHttpServer> m_httpServer;
    // A HTTP server for each configured network interface

    QScopedPointer<HEventNotifier> m_eventNotifier;
    // Handles the UPnP eventing

    QScopedPointer<PresenceAnnouncer> m_presenceAnnouncer;
    // Creates and sends the SSDP "presence announcement" messages

    QScopedPointer<HDeviceHostRuntimeStatus> m_runtimeStatus;
    //

    HDeviceHost* q_ptr;

    HDeviceHost::DeviceHostError m_lastError;

    QString m_lastErrorDescription;
    // description of the error that occurred last

    bool m_initialized;

    HDeviceStorage<HServerDevice, HServerService, HServerDeviceController> m_deviceStorage;
    // This contains the root devices and it provides lookup methods to the
    // contents of the device tree

    QNetworkAccessManager* m_nam;

public Q_SLOTS:

    void announcementTimedout(HServerDeviceController*);
    // called when it is about for the device to be re-advertised

public: // methods

    HDeviceHostPrivate();
    virtual ~HDeviceHostPrivate();

    void stopNotifiers();
    void startNotifiers(HServerDeviceController*);
    void startNotifiers();
    bool createRootDevice(const HDeviceConfiguration*);
    bool createRootDevices();

    inline static const QString& deviceDescriptionPostFix()
    {
        static QString retVal = "device_description.xml";
        return retVal;
    }
};

}
}


#endif /* HDEVICEHOST_P_H_ */
