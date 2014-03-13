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

#ifndef HCLIENTMODEL_CREATOR_P_H_
#define HCLIENTMODEL_CREATOR_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../hddoc_parser_p.h"
#include "../hmodelcreation_p.h"

#include <QtXml/QDomElement>

class QNetworkAccessManager;

namespace Herqq
{

namespace Upnp
{

class HDefaultClientDevice;

//
//
//
class HClientModelCreationArgs :
    public HModelCreationArgs
{
public:

    QNetworkAccessManager* m_nam;

    HClientModelCreationArgs(QNetworkAccessManager* nam);
    virtual ~HClientModelCreationArgs();

    HClientModelCreationArgs(const HClientModelCreationArgs&);
    HClientModelCreationArgs& operator=(const HClientModelCreationArgs&);
};

class HDefaultClientService;

//
// The class that creates the HUPnP's device model from description files
//
class HClientModelCreator
{
H_DISABLE_COPY(HClientModelCreator)

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

    QScopedPointer<HClientModelCreationArgs> m_creationParameters;

    HDocParser m_docParser;
    QString m_lastErrorDescription;
    ErrorType m_lastError;

private:

    QList<QPair<QUrl, QByteArray> > parseIconList(
        const QDomElement& iconListElement);

    bool parseStateVariables(
        HDefaultClientService* service, QDomElement stateVariableElement);

    bool parseActions(
        HDefaultClientService*, QDomElement actionElement,
        const HStateVariableInfos& svInfos);

    bool parseServiceDescription(HDefaultClientService*);

    bool parseServiceList(
        const QDomElement& serviceListElement, HDefaultClientDevice*,
        QList<HDefaultClientService*>* retVal);

    HDefaultClientDevice* parseDevice(
        const QDomElement& deviceElement, HDefaultClientDevice* parentDevice);

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

    HClientModelCreator(const HClientModelCreationArgs&);
    HDefaultClientDevice* createRootDevice();

    inline ErrorType lastError() const { return m_lastError; }
    inline QString lastErrorDescription() const { return m_lastErrorDescription; }
};

}
}

#endif /* HCLIENTMODEL_CREATOR_P_H_ */
