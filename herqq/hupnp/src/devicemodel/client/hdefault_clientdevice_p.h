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

#ifndef HDEFAULT_CLIENTDEVICE_P_H_
#define HDEFAULT_CLIENTDEVICE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpCore/HClientDevice>
#include <HUpnpCore/HDeviceStatus>

#include <QtCore/QTimer>

namespace Herqq
{

namespace Upnp
{

class HDefaultClientService;

//
//
//
class HDefaultClientDevice :
    public HClientDevice
{
Q_OBJECT
H_DISABLE_COPY(HDefaultClientDevice)

private:

    bool m_timedout;
    QScopedPointer<QTimer> m_statusNotifier;
    QScopedPointer<HDeviceStatus> m_deviceStatus;
    qint32 m_configId;

private Q_SLOTS:

    void timeout_();

public:

    HDefaultClientDevice(
        const QString& description,
        const QList<QUrl>& locations,
        const HDeviceInfo&,
        qint32 deviceTimeoutInSecs,
        HDefaultClientDevice* parentDev);

    void setServices(const QList<HDefaultClientService*>&);
    void setEmbeddedDevices(const QList<HDefaultClientDevice*>&);
    inline void setConfigId(qint32 configId) { m_configId = configId; }

public:

    enum SearchCriteria
    {
        ThisOnly = 0,
        EmbeddedDevices = 1,
        Services = 2,
        All = 3
    };

    quint32 deviceTimeoutInSecs() const;

    inline HDeviceStatus* deviceStatus() const
    {
        if (!parentDevice()) { return m_deviceStatus.data(); }
        return static_cast<HDefaultClientDevice*>(rootDevice())->deviceStatus();
    }

    void startStatusNotifier(SearchCriteria searchCriteria);
    void stopStatusNotifier(SearchCriteria searchCriteria);

    bool addLocation(const QUrl& location);
    void addLocations(const QList<QUrl>& locations);
    void clearLocations();
    HDefaultClientDevice* rootDevice() const;
    bool isTimedout(SearchCriteria searchCriteria) const;

Q_SIGNALS:

    void statusTimeout(HDefaultClientDevice* source);
    void locationsChanged();

};

}
}

#endif /* HDEFAULT_CLIENTDEVICE_P_H_ */
