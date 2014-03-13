/*
 *  Copyright (C) 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP Av (HUPnPAv) library.
 *
 *  Herqq UPnP Av is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP Av is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Herqq UPnP Av. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HCONNECTIONMANAGER_SOURCESERVICE_P_H_
#define HCONNECTIONMANAGER_SOURCESERVICE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../connectionmanager/hconnectionmanager_service_p.h"

#include <HUpnpCore/private/hhttp_server_p.h>

namespace Herqq
{

namespace Upnp
{

class HMessagingInfo;

namespace Av
{

//
//
//
class HHttpStreamer :
    public QObject
{
Q_OBJECT

private Q_SLOTS:

    void bytesWritten(qint64 written);

private:

    const int m_bufSize;
    char* m_buf;

    QIODevice* m_dataToSend;
    HMessagingInfo* m_mi;
    QByteArray m_header;

    qint64 m_read;
    qint64 m_written;

public:

    HHttpStreamer(
        HMessagingInfo*, const QByteArray& header, QIODevice* data,
        QObject* parent = 0);

    virtual ~HHttpStreamer();

    void send();

};

class HConnectionManagerSourceService;

//
//
//
class HConnectionManagerHttpServer :
    public HHttpServer
{
Q_OBJECT
H_DISABLE_COPY(HConnectionManagerHttpServer)

private:

    HConnectionManagerSourceService* m_owner;

protected:

    virtual void incomingUnknownGetRequest(
        HMessagingInfo*, const HHttpRequestHeader&);

public:

    HConnectionManagerHttpServer(
        const QByteArray& loggingId, HConnectionManagerSourceService* owner);

    virtual ~HConnectionManagerHttpServer();
};

class HConnectionManagerSourceServicePrivate;

//
//
//
class HConnectionManagerSourceService :
    public HConnectionManagerService
{
Q_OBJECT
H_DISABLE_COPY(HConnectionManagerSourceService)
friend class HConnectionManagerHttpServer;

private Q_SLOTS:

    void objectModified(
        Herqq::Upnp::Av::HObject*, const Herqq::Upnp::Av::HObjectEventInfo&);

    void containerModified(
        Herqq::Upnp::Av::HContainer*, const Herqq::Upnp::Av::HContainerEventInfo&);

private:

    HAbstractCdsDataSource* m_dataSource;
    // ^^ this is only used. this class never has the ownership of the data source.

    HConnectionManagerHttpServer* m_httpServer;

    void addLocation(HItem*);

protected:

    virtual bool finalizeInit(QString* errDescription);

public:

    HConnectionManagerSourceService(HAbstractCdsDataSource*);
    virtual ~HConnectionManagerSourceService();

    bool init();
    bool isInitialized() const;
};

}
}
}

#endif /* HCONNECTIONMANAGER_SOURCESERVICE_P_H_ */
