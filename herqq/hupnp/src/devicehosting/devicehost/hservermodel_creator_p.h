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

#ifndef HSERVERMODEL_CREATOR_P_H_
#define HSERVERMODEL_CREATOR_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../hddoc_parser_p.h"
#include "../hmodelcreation_p.h"
#include "../../devicemodel/hactioninvoke.h"

namespace Herqq
{

namespace Upnp
{

//
//
//
class HServerModelCreationArgs :
    public HModelCreationArgs
{
private:

    HDeviceModelCreator* m_deviceModelCreator;
    // Not owned.

    HDeviceModelInfoProvider* m_infoProvider;
    // Not owned.

    QString m_ddPostFix;

public:

    HServerModelCreationArgs(HDeviceModelCreator*);
    HServerModelCreationArgs(const HServerModelCreationArgs&);

    virtual ~HServerModelCreationArgs();

    HServerModelCreationArgs& operator=(const HServerModelCreationArgs&);

    inline void setInfoProvider(HDeviceModelInfoProvider* arg)
    {
        m_infoProvider = arg;
    }

    inline HDeviceModelCreator* creator() const
    {
        return m_deviceModelCreator;
    }

    inline HDeviceModelInfoProvider* infoProvider() const
    {
        return m_infoProvider;
    }

    inline void setDeviceDescriptionPostfix(const QString& arg)
    {
        m_ddPostFix = arg;
    }

    inline QString deviceDescriptionPostfix() const
    {
        return m_ddPostFix;
    }
};

//
// The class that creates the HUPnP's device model from description files
//
class HServerModelCreator
{
H_DISABLE_COPY(HServerModelCreator)

public:

    enum ErrorType
    {
        NoError,
        FailedToGetDataError,
        InvalidServiceDescription,
        InvalidDeviceDescription,
        UndefinedTypeError,
        UnimplementedAction,
        InitializationError,
        UndefinedError
    };

private:

    QScopedPointer<HServerModelCreationArgs> m_creationParameters;

    HDocParser m_docParser;
    QString m_lastErrorDescription;
    ErrorType m_lastError;

private:

    HStateVariablesSetupData getStateVariablesSetupData(HServerService*);
    HActionsSetupData getActionsSetupData(HServerService*);
    HServicesSetupData getServicesSetupData(HServerDevice*);
    HDevicesSetupData getDevicesSetupData(HServerDevice*);

    QList<QPair<QUrl, QByteArray> > parseIconList(
        const QDomElement& iconListElement);

    bool parseStateVariables(
        HServerService* service, QDomElement stateVariableElement);

    bool parseActions(
        HServerService* service,
        QDomElement actionElement,
        const HStateVariableInfos& svInfos);

    bool parseServiceDescription(HServerService*);

    bool parseServiceList(
        const QDomElement& serviceListElement, HServerDevice*,
        QList<HServerService*>* retVal);

    HServerDevice* parseDevice(
        const QDomElement& deviceElement, HServerDevice* parentDevice);

    inline ErrorType convert(DocumentErrorTypes type)
    {
        switch(type)
        {
        case InvalidDeviceDescriptionError:
            return InvalidDeviceDescription;
        case InvalidServiceDescriptionError:
            return InvalidServiceDescription;
        case NoError:
            return NoError;
        default:
            return UndefinedError;
        }
    }

public:

    HServerModelCreator(const HServerModelCreationArgs&);
    HServerDevice* createRootDevice();

    inline ErrorType lastError() const { return m_lastError; }
    inline QString lastErrorDescription() const { return m_lastErrorDescription; }
};

}
}

#endif /* HSERVERMODEL_CREATOR_P_H_ */
