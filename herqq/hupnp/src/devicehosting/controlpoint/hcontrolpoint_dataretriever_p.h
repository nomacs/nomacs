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

#ifndef HCONTROLPOINT_DATARETRIEVER_H_
#define HCONTROLPOINT_DATARETRIEVER_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../../general/hupnp_defs.h"

#include <QtCore/QByteArray>
#include <QtCore/QEventLoop>
#include <QtNetwork/QNetworkAccessManager>

class QUrl;
class QNetworkReply;

namespace Herqq
{

namespace Upnp
{

//
//
//
class HDataRetriever :
    public QEventLoop
{
Q_OBJECT
H_DISABLE_COPY(HDataRetriever)

private slots:

    void finished();

private:

    const QByteArray m_loggingIdentifier;
    QNetworkAccessManager m_nam;
    QNetworkReply* m_reply;
    QString m_lastError;

    bool m_success;

private:

    bool retrieveData(const QUrl& baseUrl, const QUrl& query, QByteArray*);

protected:

    virtual void timerEvent(QTimerEvent*);

public:

    HDataRetriever(const QByteArray& loggingId);

    inline QString lastError() const
    {
        return m_lastError;
    }

    bool retrieveServiceDescription(
        const QUrl& deviceLocation, const QUrl& scpdUrl, QString*);

    bool retrieveIcon(
        const QUrl& deviceLocation, const QUrl& iconUrl, QByteArray*);

    bool retrieveDeviceDescription(const QUrl& deviceLocation, QString*);
};

}
}

#endif /* HCONTROLPOINT_DATARETRIEVER_H_ */
