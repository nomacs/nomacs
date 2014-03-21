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

#include "hdeviceinfo.h"
#include "hdeviceinfo_p.h"

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HDeviceInfoPrivate
 ******************************************************************************/
HDeviceInfoPrivate::HDeviceInfoPrivate() :
    m_deviceType      (), m_friendlyName(), m_manufacturer(), m_manufacturerUrl(),
    m_modelDescription(), m_modelName   (), m_modelNumber (), m_modelUrl       (),
    m_serialNumber    (), m_udn         (), m_upc         (), m_presentationUrl(),
    m_icons()
{
}

HDeviceInfoPrivate::~HDeviceInfoPrivate()
{
}

bool HDeviceInfoPrivate::setDeviceType(const HResourceType& deviceType)
{
    if (!deviceType.isValid())
    {
        return false;
    }

    if (deviceType.type() != HResourceType::StandardDeviceType &&
        deviceType.type() != HResourceType::VendorSpecifiedDeviceType)
    {
        return false;
    }

    m_deviceType = deviceType;
    return true;
}

bool HDeviceInfoPrivate::setFriendlyName(const QString& friendlyName)
{
    HLOG(H_AT, H_FUN);

    if (friendlyName.isEmpty())
    {
        return false;
    }

    if (friendlyName.size() > 64)
    {
        HLOG_WARN(QString(
            "friendlyName [%1] longer than 64 characters").arg(friendlyName));
    }

    m_friendlyName = friendlyName;
    return true;
}

bool HDeviceInfoPrivate::setManufacturer(const QString& manufacturer)
{
    HLOG(H_AT, H_FUN);

    if (manufacturer.isEmpty())
    {
        return false;
    }

    if (manufacturer.size() > 64)
    {
        HLOG_WARN(QString(
            "manufacturer [%1] longer than 64 characters").arg(manufacturer));
    }

    m_manufacturer = manufacturer;
    return true;
}

bool HDeviceInfoPrivate::setModelDescription(const QString& modelDescription)
{
    HLOG(H_AT, H_FUN);

    if (modelDescription.size() > 128)
    {
        HLOG_WARN(QString(
            "modelDescription [%1] longer than 64 characters").arg(modelDescription));
    }

    m_modelDescription = modelDescription;
    return true;
}

bool HDeviceInfoPrivate::setModelName(const QString& modelName)
{
    HLOG(H_AT, H_FUN);

    if (modelName.isEmpty())
    {
        return false;
    }

    if (modelName.size() > 32)
    {
        HLOG_WARN(QString(
            "modelName [%1] longer than 32 characters: [%1]").arg(modelName));
    }

    m_modelName = modelName;
    return true;
}

bool HDeviceInfoPrivate::setModelNumber(const QString& modelNumber)
{
    HLOG(H_AT, H_FUN);

    if (modelNumber.size() > 32)
    {
        HLOG_WARN(QString(
            "modelNumber [%1] longer than 32 characters: [%1]").arg(modelNumber));
    }

    m_modelNumber = modelNumber;
    return true;
}

bool HDeviceInfoPrivate::setSerialNumber(const QString& serialNumber)
{
    HLOG(H_AT, H_FUN);

    if (serialNumber.size() > 64)
    {
        HLOG_WARN(QString(
            "serialNumber [%1] longer than 64 characters: [%1]").arg(serialNumber));
    }

    m_serialNumber = serialNumber;
    return true;
}

bool HDeviceInfoPrivate::setUpc(const QString& upc)
{
    HLOG(H_AT, H_FUN);

    if (upc.isEmpty())
    {
        // UPC is optional, so if it is not provided at all, that is okay.
        return false;
    }

    // even if something is provided, we only warn the user of possible error.
    // (since upc is optional)

    if (upc.size() > 13 || upc.size() < 12)
    {
        // a white-space and a hyphen in the middle are acceptable
        HLOG_WARN_NONSTD(QString(
            "UPC should be 12-digit, all-numeric code. Encountered: [%1].").arg(
                upc));
    }
    else
    {
        for(qint32 i = 0; i < upc.size(); ++i)
        {
            QChar ch = upc[i];

            if ((i == 6 && !ch.isSpace() && ch != '-' && upc.size() == 13) ||
                !ch.isDigit())
            {
                HLOG_WARN_NONSTD(QString(
                    "UPC should be 12-digit, all-numeric code. "
                    "Ignoring invalid value [%1].").arg(upc));

                break;
            }
        }
    }

    m_upc = upc;
    return true;
}

bool HDeviceInfoPrivate::setIcons(const QList<QUrl>& icons)
{
    m_icons = icons;
    return true;
}

/*******************************************************************************
 * HDeviceInfo
 ******************************************************************************/
HDeviceInfo::HDeviceInfo() :
    h_ptr(new HDeviceInfoPrivate())
{
}

HDeviceInfo::HDeviceInfo(const HDeviceInfo& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HDeviceInfo& HDeviceInfo::operator=(const HDeviceInfo& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

HDeviceInfo::HDeviceInfo(
    const HResourceType& deviceType,
    const QString& friendlyName,
    const QString& manufacturer,
    const QString& modelName,
    const HUdn&    udn,
    HValidityCheckLevel checkLevel,
    QString* err) :
        h_ptr(new HDeviceInfoPrivate())
{
    QScopedPointer<HDeviceInfoPrivate> tmp(new HDeviceInfoPrivate());

    QString errTmp;
    if (!tmp->setDeviceType(deviceType))
    {
        errTmp = QString("Invalid device type: [%1]").arg(deviceType.toString());
    }
    else if (!tmp->setFriendlyName(friendlyName))
    {
        errTmp = QString("Invalid friendly name: [%1]").arg(friendlyName);
    }
    else if (!tmp->setManufacturer(manufacturer))
    {
        errTmp = QString("Invalid manufacturer: [%1]").arg(manufacturer);
    }
    else if (!tmp->setModelName(modelName))
    {
        errTmp = QString("Invalid model name: [%1]").arg(modelName);
    }
    else if (!tmp->setUdn(udn, checkLevel))
    {
        errTmp = QString("Invalid UDN: [%1]").arg(udn.toString());
    }

    if (!errTmp.isEmpty())
    {
        if (err)
        {
            *err = errTmp;
        }
    }
    else
    {
        h_ptr = tmp.take();
    }
}

HDeviceInfo::HDeviceInfo(
    const HResourceType& deviceType,
    const QString& friendlyName,
    const QString& manufacturer,
    const QUrl&    manufacturerUrl,
    const QString& modelDescription,
    const QString& modelName,
    const QString& modelNumber,
    const QUrl&    modelUrl,
    const QString& serialNumber,
    const HUdn&    udn,
    const QString& upc,
    const QList<QUrl>& icons,
    const QUrl&    presentationUrl,
    HValidityCheckLevel checkLevel,
    QString* err) :
        h_ptr(new HDeviceInfoPrivate())
{
    QScopedPointer<HDeviceInfoPrivate> tmp(new HDeviceInfoPrivate());

    QString errTmp;
    if (!tmp->setDeviceType(deviceType))
    {
        errTmp = QString("Invalid device type: [%1]").arg(deviceType.toString());
    }
    else if (!tmp->setFriendlyName(friendlyName))
    {
        errTmp = QString("Invalid friendly name: [%1]").arg(friendlyName);
    }
    else if (!tmp->setManufacturer(manufacturer))
    {
        errTmp = QString("Invalid manufacturer: [%1]").arg(manufacturer);
    }
    else if (!tmp->setModelName(modelName))
    {
        errTmp = QString("Invalid model name: [%1]").arg(modelName);
    }
    else if (!tmp->setUdn(udn, checkLevel))
    {
        errTmp = QString("Invalid UDN: [%1]").arg(udn.toString());
    }

    if (!errTmp.isEmpty())
    {
        if (err)
        {
            *err = errTmp;
        }
    }
    else
    {
        h_ptr = tmp.take();
    }

    // these are optional ==> no need to be strict
    h_ptr->setManufacturerUrl (manufacturerUrl.toString());
    h_ptr->setModelDescription(modelDescription);

    h_ptr->setModelNumber     (modelNumber);
    h_ptr->setModelUrl        (modelUrl.toString());
    h_ptr->setSerialNumber    (serialNumber);
    h_ptr->setUpc             (upc);
    h_ptr->setIcons           (icons);
    h_ptr->setPresentationUrl (presentationUrl.toString());
}

HDeviceInfo::~HDeviceInfo()
{
}

bool HDeviceInfo::isValid(HValidityCheckLevel level) const
{
    return h_ptr->m_deviceType.isValid() &&
           h_ptr->m_udn.isValid(level);
}

void HDeviceInfo::setManufacturerUrl(const QUrl& arg)
{
    h_ptr->setManufacturerUrl(arg);
}

void HDeviceInfo::setModelDescription(const QString& arg)
{
    h_ptr->setModelDescription(arg);
}

void HDeviceInfo::setModelNumber(const QString& arg)
{
    h_ptr->setModelNumber(arg);
}

void HDeviceInfo::setModelUrl(const QUrl& arg)
{
    h_ptr->setModelUrl(arg);
}

void HDeviceInfo::setSerialNumber(const QString& arg)
{
    h_ptr->setSerialNumber(arg);
}

void HDeviceInfo::setUpc(const QString& arg)
{
    h_ptr->setUpc(arg);
}

void HDeviceInfo::setIcons(const QList<QUrl>& arg)
{
    h_ptr->setIcons(arg);
}

void HDeviceInfo::setPresentationUrl(const QUrl& arg)
{
    h_ptr->setPresentationUrl(arg);
}

const HResourceType& HDeviceInfo::deviceType() const
{
    return h_ptr->m_deviceType;
}

QString HDeviceInfo::friendlyName() const
{
    return h_ptr->m_friendlyName;
}

QString HDeviceInfo::manufacturer() const
{
    return h_ptr->m_manufacturer;
}

QUrl HDeviceInfo::manufacturerUrl() const
{
    return h_ptr->m_manufacturerUrl;
}

QString HDeviceInfo::modelDescription() const
{
    return h_ptr->m_modelDescription;
}

QString HDeviceInfo::modelName () const
{
    return h_ptr->m_modelName;
}

QString HDeviceInfo::modelNumber() const
{
    return h_ptr->m_modelNumber;
}

QUrl HDeviceInfo::modelUrl() const
{
    return h_ptr->m_modelUrl;
}

QString HDeviceInfo::serialNumber() const
{
    return h_ptr->m_serialNumber;
}

const HUdn& HDeviceInfo::udn() const
{
    return h_ptr->m_udn;
}

QString HDeviceInfo::upc() const
{
    return h_ptr->m_upc;
}

QList<QUrl> HDeviceInfo::icons() const
{
    return h_ptr->m_icons;
}

QUrl HDeviceInfo::presentationUrl() const
{
    return h_ptr->m_presentationUrl;
}

bool operator==(const HDeviceInfo& obj1, const HDeviceInfo& obj2)
{
    return   obj1.h_ptr->m_deviceType         == obj2.h_ptr->m_deviceType       &&
             obj1.h_ptr->m_friendlyName       == obj2.h_ptr->m_friendlyName     &&
             obj1.h_ptr->m_manufacturer       == obj2.h_ptr->m_manufacturer     &&
             obj1.h_ptr->m_manufacturerUrl    == obj2.h_ptr->m_manufacturerUrl  &&
             obj1.h_ptr->m_modelDescription   == obj2.h_ptr->m_modelDescription &&
             obj1.h_ptr->m_modelName          == obj2.h_ptr->m_modelName        &&
             obj1.h_ptr->m_modelNumber        == obj2.h_ptr->m_modelNumber      &&
             obj1.h_ptr->m_modelUrl           == obj2.h_ptr->m_modelUrl         &&
             obj1.h_ptr->m_serialNumber       == obj2.h_ptr->m_serialNumber     &&
             obj1.h_ptr->m_udn                == obj2.h_ptr->m_udn              &&
             obj1.h_ptr->m_upc                == obj2.h_ptr->m_upc              &&
             obj1.h_ptr->m_presentationUrl    == obj2.h_ptr->m_presentationUrl  &&
             obj1.h_ptr->m_icons              == obj2.h_ptr->m_icons;
}

}
}
