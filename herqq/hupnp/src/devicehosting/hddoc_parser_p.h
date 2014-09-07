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

#ifndef HDDOC_PARSER_P_H_
#define HDDOC_PARSER_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../general/hupnp_defs.h"
#include "../general/hupnp_fwd.h"
#include "../general/hupnp_global.h"
#include "../dataelements/hserviceid.h"
#include "../dataelements/hactioninfo.h"
#include "../dataelements/hserviceinfo.h"
#include "../dataelements/hstatevariableinfo.h"

#include "../devicemodel/hactionarguments.h"
#include "../devicemodel/hactions_setupdata.h"

#include <QtCore/QUrl>
#include <QtCore/QSet>
#include <QtCore/QList>
#include <QtCore/QString>

class QDomElement;
class QDomDocument;

namespace Herqq
{

namespace Upnp
{

//
//
//
enum DocumentErrorTypes
{
    NoError,
    InvalidDeviceDescriptionError,
    InvalidServiceDescriptionError
};

//
// The class that creates the HUPnP's device model from description files
//
class HDocParser
{
H_DISABLE_COPY(HDocParser)

private:

    QList<QUrl> parseIconList(
        const QDomElement& iconListElement);

    bool parseActionArguments(
        const QDomElement& argListElement,
        const QHash<QString, HStateVariableInfo>&,
        QVector<HActionArgument>* inArgs,
        QVector<HActionArgument>* outArgs,
        bool* hasRetVal);

    HStateVariableInfo parseStateVariableInfo_str(
        const QString& name,
        const QVariant& defValue,
        const QDomElement& svElement,
        HStateVariableInfo::EventingType,
        HInclusionRequirement);

    HStateVariableInfo parseStateVariableInfo_numeric(
        const QString& name,
        const QVariant& defValue,
        const QDomElement& svElement,
        HStateVariableInfo::EventingType,
        HInclusionRequirement,
        HUpnpDataTypes::DataType dataTypeEnumValue);

private:

    const QByteArray m_loggingIdentifier;
    HValidityCheckLevel m_cLevel;
    QString m_lastErrorDescription;
    DocumentErrorTypes m_lastError;

public:

    HDocParser(const QByteArray& loggingIdentifier, HValidityCheckLevel);

    inline QString lastErrorDescription() const { return m_lastErrorDescription; }
    inline DocumentErrorTypes lastError() const { return m_lastError; }

    bool parseRoot(const QString& doc, QDomDocument*, QDomElement*);
    qint32 readConfigId(const QDomElement&);
    bool parseDeviceInfo(const QDomElement&, HDeviceInfo*);
    bool parseServiceInfo(const QDomElement& serviceDefinition, HServiceInfo*);

    bool parseServiceDescription(
        const QString& docStr,
        QDomDocument* doc,
        QDomElement* stateVarElement,
        QDomElement* actionElement);

    bool parseStateVariable(
        const QDomElement& stateVariableElement,
        HStateVariableInfo*);

    bool parseActionInfo(
        const QDomElement& actionElement,
        const QHash<QString, HStateVariableInfo>&,
        HActionInfo*);

    bool verifySpecVersion(const QDomElement&, QString* err = 0);
};

//
//
//
class HDeviceValidator
{
private:

    QString m_lastErrorDescription;
    DocumentErrorTypes m_lastError;

public:

    inline QString lastErrorDescription() const { return m_lastErrorDescription; }
    inline DocumentErrorTypes lastError() const { return m_lastError; }

    template<typename Device, typename Service>
    bool validateRootDevice(Device* device)
    {
        class DeviceValidator
        {
        private:

            QSet<QString> eventUrls;
            QSet<QString> controlUrls;
            QSet<QString> scpdUrls;
            QSet<HServiceId> serviceIds;

        public:

            QString m_lastErrorDescription;
            DocumentErrorTypes m_lastError;

            bool validateService(Service* service)
            {
                const HServiceId& serviceId = service->info().serviceId();

                if (serviceIds.contains(serviceId))
                {
                    m_lastError = InvalidDeviceDescriptionError;
                    m_lastErrorDescription = QString(
                        "ServiceId [%1] encountered more than once. "
                        "ServiceIDs MUST be unique within a device tree.").arg(
                            serviceId.toString());

                    return false;
                }
                else
                {
                    serviceIds.insert(serviceId);
                }

                QString eventUrl = service->info().eventSubUrl().toString();
                if (!eventUrl.isEmpty())
                {
                    if (eventUrls.contains(eventUrl))
                    {
                        m_lastError = InvalidDeviceDescriptionError;
                        m_lastErrorDescription = QString(
                            "EventSubUrl [%1] encountered more than once."
                            "EventSubUrls MUST be unique within a device tree.").arg(eventUrl);

                        return false;
                    }
                    else
                    {
                        eventUrls.insert(eventUrl);
                    }
                }

                QString scpdUrl = service->info().scpdUrl().toString();
                if (scpdUrls.contains(scpdUrl))
                {
                    m_lastError = InvalidDeviceDescriptionError;
                    m_lastErrorDescription = QString(
                        "ScpdUrl [%1] encountered more than once."
                        "ScpdUrls MUST be unique within a device tree.").arg(eventUrl);

                    return false;
                }
                else
                {
                    scpdUrls.insert(eventUrl);
                }

                QString controlUrl = service->info().controlUrl().toString();
                if (controlUrls.contains(controlUrl))
                {
                    m_lastError = InvalidDeviceDescriptionError;
                    m_lastErrorDescription = QString(
                        "ControlUrl [%1] encountered more than once. "
                        "ControlUrls MUST be unique within a device tree.").arg(controlUrl);

                    return false;
                }
                else
                {
                    controlUrls.insert(controlUrl);
                }

                return true;
            }

            bool validateDevice(Device* device)
            {
                serviceIds.clear();
                QList<Service*> services = device->services();
                for(qint32 i = 0; i < services.size(); ++i)
                {
                    Service* service = services[i];
                    if (!validateService(service))
                    {
                        return false;
                    }
                }

                QList<Device*> devices = device->embeddedDevices();
                for(qint32 i = 0; i < devices.size(); ++i)
                {
                    if (!validateDevice(devices[i]))
                    {
                        return false;
                    }
                }

                return true;
            }
        };

        DeviceValidator validator;
        if (!validator.validateDevice(device))
        {
            m_lastError = validator.m_lastError;
            m_lastErrorDescription = validator.m_lastErrorDescription;
            return false;
        }

        return true;
    }

    bool validate(const HStateVariableInfo& setupInfo,
                  const HStateVariableInfo&)
    {
        if (!setupInfo.isValid())
        {
            // The setup information was not defined, which means that no
            // further validation should be run.
            return true;
        }

        return true;
    }

    bool validate(const HActionSetup& setupInfo, const HActionInfo& createdInfo)
    {
        if (!setupInfo.isValid())
        {
            // The setup information was not defined, which means that no
            // further validation should be run.
            return true;
        }

        bool b = true;

        // the setup info does not necessarily contain information about the
        // action arguments, which is fine and it just means that the arguments
        // should not be validated.

        if (!setupInfo.inputArguments().isEmpty())
        {
            b = setupInfo.inputArguments() != createdInfo.inputArguments();
        }

        if (!b)
        {
            m_lastError = InvalidServiceDescriptionError;
            m_lastErrorDescription =
                "Invalid input arguments provided. Remember to check the "
                "order in which they are defined.";

            return false;
        }

        if (!setupInfo.outputArguments().isEmpty())
        {
            b = setupInfo.outputArguments() != createdInfo.outputArguments();
        }

        if (!b)
        {
            m_lastError = InvalidServiceDescriptionError;
            m_lastErrorDescription =
                "Invalid output arguments provided. Remember to check the "
                "order in which they are defined.";
        }

        return b;
    }
};

}
}

#endif /* HDDOC_PARSER_P_H_ */
