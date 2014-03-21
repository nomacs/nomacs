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

#include "hdiscoverytype.h"

#include "../dataelements/hudn.h"
#include "../dataelements/hresourcetype.h"

#include "../general/hlogger_p.h"

#include <QtCore/QString>
#include <QtCore/QStringList>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HDiscoveryTypePrivate
 ******************************************************************************/
class HDiscoveryTypePrivate :
    public QSharedData
{
public:

    HDiscoveryType::Type m_type;
    QString       m_contents;

    HUdn          m_udn;
    HResourceType m_resourceType;

public:

    HDiscoveryTypePrivate() :
        m_type(HDiscoveryType::Undefined), m_contents(), m_udn(),
        m_resourceType()
    {
    }

    bool parse(const HResourceType& rt)
    {
        if (!rt.isValid())
        {
            return false;
        }

        m_resourceType = rt;
        return true;
    }

    bool parse(const QString& arg, HValidityCheckLevel checkLevel)
    {
        HLOG(H_AT, H_FUN);

        QString tmp(arg.simplified());

        HUdn udn;
        qint32 indx = tmp.indexOf("::");
        if (indx == 41) // the length of "uuid:UUID" is 41
        {
            udn = HUdn(tmp.left(41));
            if (!udn.isValid(checkLevel))
            {
                return false;
            }

            if (tmp.size() > 43)
            {
                tmp = tmp.mid(43);
            }
            else
            {
                m_udn = udn;
                m_type = HDiscoveryType::SpecificDevice;
                m_contents = udn.toString();
                return true;
            }
        }

        QStringList parsed = tmp.split(':');
        if (parsed.size() < 2)
        {
            HLOG_WARN(QString("Invalid resource identifier: %1").arg(arg));
            return false;
        }

        if (!udn.isValid(checkLevel))
        {
            if (parsed[0] == "ssdp" && parsed[1] == "all")
            {
                m_type = HDiscoveryType::All;
                m_contents = "ssdp:all";
                return true;
            }
        }

        if (parsed[0] == "upnp" && parsed[1] == "rootdevice")
        {
            m_udn = udn;

            if (m_udn.isValid(checkLevel))
            {
                m_type = HDiscoveryType::SpecificRootDevice;
                m_contents = QString("%1::upnp:rootdevice").arg(udn.toString());
            }
            else
            {
                m_type = HDiscoveryType::RootDevices;
                m_contents = "upnp:rootdevice";
            }
            return true;
        }
        else if (parsed[0] == "uuid")
        {
            udn = HUdn(parsed[1]);
            if (udn.isValid(checkLevel))
            {
                m_udn = udn;
                m_type = HDiscoveryType::SpecificDevice;
                m_contents = udn.toString();
                return true;
            }
        }

        HResourceType resourceType(tmp);
        if (parse(resourceType))
        {
            m_udn = udn;
            if (m_udn.isValid(checkLevel))
            {
                m_type = resourceType.isDeviceType() ?
                     HDiscoveryType::SpecificDeviceWithType :
                     HDiscoveryType::SpecificServiceWithType;

                m_contents = QString("%1::%2").arg(
                    udn.toString(), resourceType.toString());
            }
            else
            {
                m_type = resourceType.isDeviceType() ?
                     HDiscoveryType::DeviceType :
                     HDiscoveryType::ServiceType;

                m_contents = QString("%1").arg(resourceType.toString());
            }

            return true;
        }

        HLOG_WARN(QString("Invalid resource identifier: %1").arg(arg));
        return false;
    }

    void setState(
        const HUdn& udn, const HResourceType& rt, HValidityCheckLevel checkLevel)
    {
        if (udn.isValid(checkLevel))
        {
            switch(rt.type())
            {
            case HResourceType::Undefined:
                m_udn = udn;
                m_type = HDiscoveryType::SpecificDevice;
                m_resourceType = rt;
                m_contents = udn.toString();
                return;

            case HResourceType::StandardDeviceType:
            case HResourceType::VendorSpecifiedDeviceType:
                m_type = HDiscoveryType::SpecificDeviceWithType;
                break;

            case HResourceType::StandardServiceType:
            case HResourceType::VendorSpecifiedServiceType:
                m_type = HDiscoveryType::SpecificServiceWithType;
                break;

            default:
                Q_ASSERT(false);
            }

            m_contents = QString("%1::%2").arg(udn.toString(), rt.toString());
        }
        else
        {
            switch(rt.type())
            {
            case HResourceType::Undefined:
                m_udn = udn;
                m_type = HDiscoveryType::Undefined;
                m_resourceType = rt;
                m_contents = QString();
                return;

            case HResourceType::StandardDeviceType:
            case HResourceType::VendorSpecifiedDeviceType:
                m_type = HDiscoveryType::DeviceType;
                break;

            case HResourceType::StandardServiceType:
            case HResourceType::VendorSpecifiedServiceType:
                m_type = HDiscoveryType::ServiceType;
                break;

            default:
                Q_ASSERT(false);
            }

            m_contents = QString("%1").arg(rt.toString());
        }

        m_udn = udn;
        m_resourceType = rt;
    }
};

/*******************************************************************************
 * HDiscoveryType
 ******************************************************************************/
HDiscoveryType::HDiscoveryType() :
    h_ptr(new HDiscoveryTypePrivate())
{
}

HDiscoveryType::HDiscoveryType(
    const HUdn& udn, bool isRootDevice, HValidityCheckLevel checkLevel) :
        h_ptr(new HDiscoveryTypePrivate())
{
    if (udn.isValid(checkLevel))
    {
        if (isRootDevice)
        {
            h_ptr->m_type = HDiscoveryType::SpecificRootDevice;
            h_ptr->m_contents =
                QString("%1::upnp:rootdevice").arg(udn.toString());
        }
        else
        {
            h_ptr->m_type = HDiscoveryType::SpecificDevice;
            h_ptr->m_contents = udn.toString();
        }

        h_ptr->m_udn = udn;
    }
}

HDiscoveryType::HDiscoveryType(const HResourceType& resourceType) :
    h_ptr(new HDiscoveryTypePrivate())
{
    if (h_ptr->parse(resourceType))
    {
        h_ptr->m_contents = resourceType.toString();
        h_ptr->m_type = resourceType.isDeviceType() ?
            HDiscoveryType::DeviceType :
            HDiscoveryType::ServiceType;
    }
}

HDiscoveryType::HDiscoveryType(
    const HUdn& udn, const HResourceType& resourceType,
    HValidityCheckLevel checkLevel) :
        h_ptr(new HDiscoveryTypePrivate())
{
    if (h_ptr->parse(resourceType) && udn.isValid(checkLevel))
    {
        h_ptr->m_udn = udn;
        h_ptr->m_contents =
            QString("%1::%2").arg(udn.toString(), resourceType.toString());

        h_ptr->m_type = resourceType.isDeviceType() ?
            HDiscoveryType::SpecificDeviceWithType :
            HDiscoveryType::SpecificServiceWithType;
    }
}

HDiscoveryType::HDiscoveryType(
    const QString& resource, HValidityCheckLevel checkLevel) :
        h_ptr(new HDiscoveryTypePrivate())
{
    h_ptr->parse(resource, checkLevel);
}

HDiscoveryType::~HDiscoveryType()
{
}

HDiscoveryType::HDiscoveryType(const HDiscoveryType& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HDiscoveryType& HDiscoveryType::operator=(
    const HDiscoveryType& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

HDiscoveryType::Type HDiscoveryType::type() const
{
    return h_ptr->m_type;
}

const HUdn& HDiscoveryType::udn() const
{
    return h_ptr->m_udn;
}

void HDiscoveryType::setUdn(const HUdn& udn, HValidityCheckLevel checkLevel)
{
    h_ptr->setState(udn, h_ptr->m_resourceType, checkLevel);
}

const HResourceType& HDiscoveryType::resourceType() const
{
    return h_ptr->m_resourceType;
}

void HDiscoveryType::setResourceType(const HResourceType& resource)
{
    h_ptr->setState(h_ptr->m_udn, resource, LooseChecks);
}

QString HDiscoveryType::toString() const
{
    return h_ptr->m_contents;
}

HDiscoveryType HDiscoveryType::createDiscoveryTypeForRootDevices()
{
    static HDiscoveryType retVal("upnp:rootdevice");
    return retVal;
}

HDiscoveryType HDiscoveryType::createDiscoveryTypeForAllResources()
{
    static HDiscoveryType retVal("ssdp:all");
    return retVal;
}

bool operator==(const HDiscoveryType& obj1, const HDiscoveryType& obj2)
{
    return obj1.h_ptr->m_contents == obj2.h_ptr->m_contents;
}

}
}
