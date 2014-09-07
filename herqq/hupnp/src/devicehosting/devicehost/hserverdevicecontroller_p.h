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

#ifndef HSERVERDEVICECONTROLLER_P_H_
#define HSERVERDEVICECONTROLLER_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpCore/HUpnp>

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>

class QTimer;

namespace Herqq
{

namespace Upnp
{

class HServerDevice;
class HEventNotifier;

//
// This is an internal class that provides features HDeviceHost requires
// in its operations
//
class H_UPNP_CORE_EXPORT HServerDeviceController :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HServerDeviceController)

private:

    bool m_timedout;
    QScopedPointer<QTimer> m_statusNotifier;
    QScopedPointer<HDeviceStatus> m_deviceStatus;

private Q_SLOTS:

    void timeout_();

public:

    HServerDevice* m_device;
    qint32 m_configId;

public:

    HServerDeviceController(
        HServerDevice* device, qint32 deviceTimeoutInSecs, QObject* parent = 0);

    virtual ~HServerDeviceController();

    qint32 deviceTimeoutInSecs() const;

    inline HDeviceStatus* deviceStatus() const
    {
        return m_deviceStatus.data();
    }

    void startStatusNotifier();
    void stopStatusNotifier();

    bool isTimedout() const;

Q_SIGNALS:

    void statusTimeout(HServerDeviceController* source);
};

}
}


#endif /* HSERVERDEVICECONTROLLER_P_H_ */
