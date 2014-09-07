/*4
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP Av (HUPnPAv) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HCONTENTDIRECTORY_SERVICE_H_
#define HCONTENTDIRECTORY_SERVICE_H_

#include <HUpnpAv/HAbstractContentDirectoryService>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HContentDirectoryServicePrivate;

/*!
 * \brief This class provides an implementation of the standardized UPnP service type
 * ContentDirectory:3.
 *
 * For more information, see the
 * <a href=http://upnp.org/specs/av/UPnP-av-ContentDirectory-v3-Service.pdf>
 * UPnP ContentDirectory:3 specification</a>.
 *
 * \headerfile hcontentdirectory_service.h HContentDirectoryService
 *
 * \ingroup hupnp_av_cds
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HAbstractMediaServerDevice, HAbstractContentDirectoryService
 */
class H_UPNP_AV_EXPORT HContentDirectoryService :
    public HAbstractContentDirectoryService
{
Q_OBJECT
H_DISABLE_COPY(HContentDirectoryService)
H_DECLARE_PRIVATE(HContentDirectoryService)

private Q_SLOTS:

    void timeout();

    void objectModified(
        Herqq::Upnp::Av::HObject*, const Herqq::Upnp::Av::HObjectEventInfo&);

    void containerModified(
        Herqq::Upnp::Av::HContainer*, const Herqq::Upnp::Av::HContainerEventInfo&);

    void independentObjectAdded(Herqq::Upnp::Av::HObject* source);

protected:

    //
    // \internal
    //
    HContentDirectoryService(HContentDirectoryServicePrivate& dd);

public:

    /*!
     * \brief Creates a new instance.
     *
     * \param dataSource specifies the data source the content directory
     * uses.
     *
     * \sa init()
     *
     * \remarks The ownership of the \a datasource is not transferred;
     * the content directory service only uses it and it will never delete it.
     * Make sure the data source is not deleted before this object is deleted.
     */
    HContentDirectoryService(HAbstractCdsDataSource* dataSource);

    /*!
     * \brief Destroys the instance.
     *
     * \sa ~HAbstractContentDirectoryService()
     */
    virtual ~HContentDirectoryService();

    /*!
     * Initializes the instance.
     *
     * \return \e true if the initialization succeeded.
     */
    bool init();

    // All the following functions are documented in HAbstractContentDirectoryService
    virtual qint32 getSearchCapabilities(QStringList* outArg) const;
    virtual qint32 getSortCapabilities(QStringList* outArg) const;
    virtual qint32 getSortExtensionCapabilities(QStringList* outArg) const;
    virtual qint32 getFeatureList(QString* outArg) const;
    virtual qint32 getSystemUpdateId(quint32*) const;
    virtual qint32 getServiceResetToken(QString*) const;
    virtual qint32 browse(
        const QString& objectId,
        HContentDirectoryInfo::BrowseFlag browseFlag,
        const QSet<QString>& filter,
        quint32 startingIndex,
        quint32 requestedCount,
        const QStringList& sortCriteria,
        HSearchResult* result);

    virtual qint32 search(
        const QString& containerId,
        const QString& searchCriteria,
        const QSet<QString>& filter,
        quint32 startingIndex,
        quint32 requestedCount,
        const QStringList& sortCriteria,
        HSearchResult* result);
};

}
}
}

#endif /* HCONTENTDIRECTORY_SERVICE_H_ */
