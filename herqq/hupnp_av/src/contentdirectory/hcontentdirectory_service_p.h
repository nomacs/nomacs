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

#ifndef HCONTENTDIRECTORY_SERVICE_P_H_
#define HCONTENTDIRECTORY_SERVICE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "habstractcontentdirectory_service_p.h"

#include "../cds_model/cds_objects/hitem.h"
#include "../cds_model/cds_objects/hcontainer.h"
#include "../cds_model/datasource/hcds_datasource.h"

#include <QtCore/QTimer>
#include <QtCore/QPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

//
//
//
class HModificationEvent
{
H_DISABLE_COPY(HModificationEvent)

public:

    enum Type
    {
        ObjectModification,
        ContainerModification
    };

private:

    Type m_type;
    QPointer<HObject> m_source;

public:

    union
    {
        HObjectEventInfo* m_objectEventInfo;
        HContainerEventInfo* m_containerEventInfo;
    };

    HModificationEvent(HObject* source, const HObjectEventInfo& info) :
        m_type(ObjectModification), m_source(source),
        m_objectEventInfo(new HObjectEventInfo(info))
    {
    }

    HModificationEvent(HContainer* source, const HContainerEventInfo& info) :
        m_type(ContainerModification), m_source(source),
        m_containerEventInfo(new HContainerEventInfo(info))
    {
    }

    ~HModificationEvent()
    {
        if (m_type == ObjectModification)
        {
            delete m_objectEventInfo;
        }
        else
        {
            delete m_containerEventInfo;
        }
    }

    inline Type type() const { return m_type; }
    inline HContainerEventInfo containerEvent() const
    {
        Q_ASSERT(type() == ContainerModification);
        return *m_containerEventInfo;
    }
    inline HObjectEventInfo objectEvent() const
    {
        Q_ASSERT(type() == ObjectModification);
        return *m_objectEventInfo;
    }
    inline HObject* source() const
    {
        return m_source;
    }
};

//
// Implementation details of HContentDirectoryService
//
class H_UPNP_AV_EXPORT HContentDirectoryServicePrivate :
    public HAbstractContentDirectoryServicePrivate
{
H_DECLARE_PUBLIC(HContentDirectoryService)
H_DISABLE_COPY(HContentDirectoryServicePrivate)

private:

    qint32 sort(const QStringList& sortCriteria, QList<HObject*>& objects);

    qint32 browseDirectChildren(
        const QString& containerId,
        const QSet<QString>& filter,
        const QStringList& sortCriteria,
        quint32 startingIndex,
        quint32 requestedCount,
        HSearchResult*);

    qint32 browseMetadata(
        const QString& objectId,
        const QSet<QString>& filter,
        quint32 startingIndex,
        HSearchResult*);

    void enableChangeTracking();
    QString generateLastChange();

public:

    QPointer<HAbstractCdsDataSource> m_dataSource;
    bool m_lastEventSent;
    QTimer m_timer;

    QList<HModificationEvent*> m_modificationEvents;

public:

    HContentDirectoryServicePrivate();
    virtual ~HContentDirectoryServicePrivate();

    void init();
};

}
}
}

#endif /* HCONTENTDIRECTORY_SERVICE_P_H_ */
