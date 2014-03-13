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

#include "hclientdevice_adapter.h"
#include "hclientdevice_adapter_p.h"
#include "hclientservice_adapter.h"
#include "hclientdevice.h"

#include "../hdevicemodel_validator.h"
#include "../hdevicemodel_infoprovider.h"

#include "../../dataelements/hdeviceinfo.h"

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HClientDeviceAdapterPrivate
 ******************************************************************************/
HClientDeviceAdapterPrivate::HClientDeviceAdapterPrivate(const HResourceType& rt) :
    HAbstractClientAdapterPrivate(rt),
        m_device(0), q_ptr(0)
{
}

HClientDeviceAdapterPrivate::~HClientDeviceAdapterPrivate()
{
}

bool HClientDeviceAdapterPrivate::validate(HClientDevice* device)
{
    HDeviceModelValidator validator;
    validator.setInfoProvider(*m_infoProvider.data());
    if (validator.validateDevice(device) != HDeviceModelValidator::ValidationSucceeded)
    {
        m_lastErrorDescription = validator.lastErrorDescription();
        return false;
    }
    return true;
}

/*******************************************************************************
 * HClientDeviceAdapter
 ******************************************************************************/
HClientDeviceAdapter::HClientDeviceAdapter(
    const HResourceType& deviceType, QObject* parent) :
        QObject(parent),
            h_ptr(new HClientDeviceAdapterPrivate(deviceType))
{
    h_ptr->q_ptr = this;
    Q_ASSERT(deviceType.isValid());
}

HClientDeviceAdapter::HClientDeviceAdapter(
    HClientDeviceAdapterPrivate& dd, QObject* parent) :
        QObject(parent),
            h_ptr(&dd)
{
    h_ptr->q_ptr = this;
}

HClientDeviceAdapter::~HClientDeviceAdapter()
{
    delete h_ptr;
}

bool HClientDeviceAdapter::prepareDevice(HClientDevice*)
{
    return true;
}

void HClientDeviceAdapter::setDeviceModelInfoProvider(
    const HDeviceModelInfoProvider& infoProvider)
{
    h_ptr->m_infoProvider.reset(infoProvider.clone());
}

const HDeviceModelInfoProvider* HClientDeviceAdapter::deviceModelInfoProvider() const
{
    return h_ptr->m_infoProvider.data();
}

void HClientDeviceAdapter::setLastErrorDescription(const QString& arg)
{
    h_ptr->m_lastErrorDescription = arg;
}

bool HClientDeviceAdapter::setDevice(HClientDevice* device, ValidationType vt)
{
    HResourceType dt = device->info().deviceType();

    if (!device || !dt.compare(deviceType(), HResourceType::Inclusive))
    {
        setLastErrorDescription(QString("Unsupported device type: [%1]").arg(dt.toString()));
        return false;
    }

    if (vt == FullValidation)
    {
        if (deviceModelInfoProvider())
        {
            if (!h_ptr->validate(device))
            {
                return false;
            }
        }
    }

    if (!prepareDevice(device))
    {
        return false;
    }

    h_ptr->m_device = device;

    return true;
}

QString HClientDeviceAdapter::lastErrorDescription() const
{
    return h_ptr->m_lastErrorDescription;
}

HClientDevice* HClientDeviceAdapter::device() const
{
    return h_ptr->m_device;
}

bool HClientDeviceAdapter::isReady() const
{
    return h_ptr->m_device;
}

const HResourceType& HClientDeviceAdapter::deviceType() const
{
    return h_ptr->m_resourceType;
}

}
}
