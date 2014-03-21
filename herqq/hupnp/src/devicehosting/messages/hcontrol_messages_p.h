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

#ifndef HCONTROL_MESSAGES_H_
#define HCONTROL_MESSAGES_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <QtCore/QUrl>
#include <QtCore/QString>

#include <QtSoapMessage>

namespace Herqq
{

namespace Upnp
{

//
//
//
class HInvokeActionRequest
{
private:

    QString       m_soapAction;
    QtSoapMessage m_soapMsg;
    QUrl          m_serviceUrl;

public:

    HInvokeActionRequest();
    HInvokeActionRequest(
        const QString& soapAction, const QtSoapMessage& soapMsg,
        const QUrl& serviceUrl);

    ~HInvokeActionRequest();

    inline QString soapAction() const
    {
        return m_soapAction;
    }

    inline const QtSoapMessage* soapMsg() const
    {
        return &m_soapMsg;
    }

    inline QUrl serviceUrl() const
    {
        return m_serviceUrl;
    }
};

}
}

#endif /* HCONTROL_MESSAGES_H_ */
