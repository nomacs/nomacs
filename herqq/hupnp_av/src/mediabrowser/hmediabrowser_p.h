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

#ifndef H_MEDIABROWSER_P_H_
#define H_MEDIABROWSER_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hmediabrowser.h"

#include <HUpnpAv/HSearchResult>
#include <HUpnpCore/HClientAdapterOp>

#include <QtCore/QQueue>
#include <QtCore/QScopedPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

//
//
//
class HBrowseOp
{
HBrowseOp& operator=(const HBrowseOp&);

public:

    HBrowseParams m_loadParams;
    HObjects m_loadedObjects;
    qint32 m_indexUnderProcessing;
    QScopedPointer<HClientAdapterOp<HSearchResult> > m_currentOp;

    HBrowseOp() :
        m_loadParams(),
        m_loadedObjects(),
        m_indexUnderProcessing(-1),
        m_currentOp(0)
    {
    }

    HBrowseOp(const HBrowseParams& arg) :
        m_loadParams(arg),
        m_loadedObjects(),
        m_indexUnderProcessing(arg.browseType() != HBrowseParams::DirectChildren ? -1 : 0),
        m_currentOp(0)
    {
    }

    HBrowseOp(const HBrowseOp& other) :
        m_loadParams(other.m_loadParams),
        m_loadedObjects(other.m_loadedObjects),
        m_indexUnderProcessing(other.m_indexUnderProcessing),
        m_currentOp(new HClientAdapterOp<HSearchResult>(*m_currentOp))
    {
    }
};

//
// Implementation details of HMediaBrowser
//
class H_UPNP_AV_EXPORT HMediaBrowserPrivate :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HMediaBrowserPrivate)

private Q_SLOTS:

    void browseCompleted(
        Herqq::Upnp::Av::HContentDirectoryAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HSearchResult>& op);

    void lastChangeReceived(
        Herqq::Upnp::Av::HContentDirectoryAdapter* source, const QString& data);

public:

    HContentDirectoryAdapter* m_contentDirectory;
    bool m_hasOwnershipOfCds;
    bool m_autoUpdateEnabled;

    HCdsDataSource* m_dataSource;

    QScopedPointer<HBrowseOp> m_currentUserOp;
    QScopedPointer<HBrowseOp> m_currentAutoOp;
    QQueue<HBrowseOp*> m_autoOpQueue;

    qint32 m_lastErrorCode;
    QString m_lastErrorDescription;

    HMediaBrowser* q_ptr;

    explicit HMediaBrowserPrivate(HMediaBrowser* parent);
    virtual ~HMediaBrowserPrivate();

    void checkNextAutoOp();

    void autoBrowse(const HBrowseParams&);

    void update(const HCdsLastChangeInfos&);

    bool browse(HBrowseOp*);
    void reset();

    HObjects browseChildren(const QString& id);

    inline HMediaBrowser* owner() const
    {
        return static_cast<HMediaBrowser*>(parent());
    }

    void browseComplete(HBrowseOp*);
    void browseFailed(HBrowseOp* op, const QString& errorDescription,
        qint32 errorCode = UpnpUndefinedFailure);

    inline void setLastError(qint32 code, const QString& description)
    {
        m_lastErrorCode = code;
        m_lastErrorDescription = description;
    }
};

}
}
}

#endif /* H_MEDIABROWSER_P_H_ */
