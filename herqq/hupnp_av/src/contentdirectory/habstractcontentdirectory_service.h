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

#ifndef HABSTRACTCONTENTDIRECTORY_SERVICE_H_
#define HABSTRACTCONTENTDIRECTORY_SERVICE_H_

#include <HUpnpAv/HContentDirectoryInfo>
#include <HUpnpCore/HServerService>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HAbstractContentDirectoryServicePrivate;

/*!
 * This is an abstract base class for implementing the standardized UPnP service type
 * ContentDirectory:3.
 *
 * \brief This class marshals the action invocations through the HServerAction interface to
 * virtual methods. It is an ideal choice for a base class when you wish to implement
 * the ContentDirectory service in full. However, you may want to check
 * HContentDirectoryService class as well, which provides a standard-compliant
 * implementation of the ContentDirectory:3 specification.
 *
 * For more information, see the
 * <a href=http://upnp.org/specs/av/UPnP-av-ContentDirectory-v3-Service.pdf>
 * UPnP ContentDirectory:3 specification</a>.
 *
 * \headerfile habstractcontentdirectory_service.h HAbstractContentDirectoryService
 *
 * \ingroup hupnp_av_cds
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HContentDirectoryService
 */
class H_UPNP_AV_EXPORT HAbstractContentDirectoryService :
    public HServerService
{
Q_OBJECT
H_DECLARE_PRIVATE(HAbstractContentDirectoryService)
H_DISABLE_COPY(HAbstractContentDirectoryService)

protected:

    /*!
     * \brief Creates a new instance.
     */
    HAbstractContentDirectoryService();

    //
    // \internal
    //
    HAbstractContentDirectoryService(HAbstractContentDirectoryServicePrivate& dd);

    // Documented in HServerService
    virtual HActionInvokes createActionInvokes();

    // Documented in HServerService
    virtual bool finalizeInit(QString* errDescription);

public:

    /*!
     * \brief Destroys the instance.
     *
     * \sa ~HServerService()
     */
    virtual ~HAbstractContentDirectoryService() = 0;

    /*!
     * \brief Retrieves the search capabilities supported by the device.
     *
     * \param outArg specifies a pointer to a \c QStringList that will contain
     * the search capabilities supported by the service.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     */
    virtual qint32 getSearchCapabilities(QStringList* outArg) const = 0;

    /*!
     * \brief Retrieves the sort capabilities supported by the service.
     *
     * These are the arguments that can be used as the \e sort \e criteria
     * when browsing or searching.
     *
     * \param outArg specifies a pointer to a \c QStringList that will contain
     * the sort capabilities supported by the service.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     *
     * \sa browse(), search()
     */
    virtual qint32 getSortCapabilities(QStringList* outArg) const = 0;

    /*!
     * \brief Retrieves the sort modifiers supported by the service.
     *
     * \param outArg specifies a pointer to a \c QStringList that will contain
     * the sort modifiers supported by the service.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     *
     * \remarks This is an optional action.
     */
    virtual qint32 getSortExtensionCapabilities(QStringList* outArg) const;

    /*!
     * \brief Retrieves the optional CDS features the service supports, if any.
     *
     * \param outArg specifies a pointer to a \c QString that will contain
     * a <em>Features XML document</em>, which defines the optional
     * CDS features the service supports, if any.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     */
    virtual qint32 getFeatureList(QString* outArg) const = 0;

    /*!
     * \brief Retrieves the current value of the state variable \c SystemUpdateId.
     *
     * \param outArg specifies a pointer to a \c quint32 that will contain
     * the current value of the state variable \c SystemUpdateId.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     */
    virtual qint32 getSystemUpdateId(quint32*) const = 0;

    /*!
     * \brief Retrieves the current value of the staet variable \c ServiceResetToken.
     *
     * \param outArg specifies a pointer to a \c QString that will contain
     * the current value of the state variable \c ServiceResetToken.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     */
    virtual qint32 getServiceResetToken(QString*) const = 0;

    /*!
     * Browses the CDS objects contained by this instance.
     *
     * \param objectId the ID of the object to be browsed. A value \c "0" corresponds
     * to the root object that is always defined.
     *
     * \param browseFlag specifies whether the browse targets the metadata of the
     * specified object or the metadata of the children of the specified object.
     *
     * \param filter specifies the metadata properties to be returned. Unless
     * the filter contains a single value \c "*" indicating that everything should
     * be returned, each value in the filter has to contain the standard
     * namespace prefix for that property, except for the DIDL-Lite namespace.
     * That is, excluding properties in the DIDL-Lite namespace a property name
     * has to be fully qualified.
     *
     * \param startingIndex zero-based offset for browsing the children of the
     * specified object. This value has to be \c 0 in case the browseFlag is set to
     * HContentDirectoryInfo::BrowseMetadata. In addition, the \a objectId has
     * to identify a \e container object.
     *
     * \param requestedCount specifies the maximum number objects to be browsed.
     * A value of \c "0" indicates that all objects under the specified \a objectId
     * should be browsed.
     *
     * \param sortCriteria specifies the sort criteria to be applied to the
     * returned object information.
     *
     * \param result specifies a pointer to a HSearchResult object that will
     * contain the result of the operation.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HContentDirectoryInfo::ErrorCode
     */
    virtual qint32 browse(
        const QString& objectId,
        HContentDirectoryInfo::BrowseFlag browseFlag,
        const QSet<QString>& filter,
        quint32 startingIndex,
        quint32 requestedCount,
        const QStringList& sortCriteria,
        HSearchResult* result) = 0;

    /*!
     * Searches for objects using a query string search criteria.
     *
     * \param containerId specifies the ID of the \e container to which the
     * search the search is applied. A value of \c "0" corresponds to the root
     * object that is always defined.
     *
     * \param searchCriteria specifies the criteria used for querying the service.
     *
     * \param filter specifies the metadata properties to be returned. Unless
     * the filter contains a single value "*" indicating that everything should
     * be returned, each value in the filter has to contain the standard
     * namespace prefix for that property, except for the DIDL-Lite namespace.
     * That is, excluding properties in the DIDL-Lite namespace a property name
     * has to be fully qualified.
     *
     * \param startingIndex zero-based offset for browsing the children of the
     * specified object. This value has to be 0 in case the browseFlag is set to
     * HContentDirectoryInfo::BrowseMetadata. In addition, the \a objectId has
     * to identify a \e container object.
     *
     * \param requestedCount specifies the maximum number objects to be browsed.
     * A value of \c "0" indicates that all objects under the specified \a objectId
     * should be browsed.
     *
     * \param sortCriteria specifies the sort criteria to be applied to the
     * returned object information.
     *
     * \param result specifies a pointer to a HSearchResult object that will
     * contain the result of the operation.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HContentDirectoryInfo::ErrorCode

     * \remarks
     * This is an optional action.
     */
    virtual qint32 search(
        const QString& containerId,
        const QString& searchCriteria,
        const QSet<QString>& filter,
        quint32 startingIndex,
        quint32 requestedCount,
        const QStringList& sortCriteria,
        HSearchResult* result);

    /*!
     * Creates a new object under the specified container.
     *
     * \param containerId specifies the parent container for the new object.
     *
     * \param elements specifies a DIDL-Lite XML document that describes the
     * new object.
     *
     * \param result is a pointer to \c HCreateObjectResult that will contain
     * the result of the operation.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HContentDirectoryInfo::ErrorCode
     *
     * \remarks
     * This is an optional action.
     */
    virtual qint32 createObject(
        const QString& containerId,
        const QString& elements,
        HCreateObjectResult* result);

    /*!
     * Destroys and removes the specified object.
     *
     * \param objectId specifies the object to be removed.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HContentDirectoryInfo::ErrorCode
     *
     * \remarks
     * This is an optional action.
     */
    virtual qint32 destroyObject(const QString& objectId);

    /*!
     * Modifies the metadata of the specified object.
     *
     * \param objectId specifies the object to be modified.
     *
     * \param currentTagValues specifies the properties to be modified.
     *
     * \param newTagValues specifies the new values for properties to be modified.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HContentDirectoryInfo::ErrorCode
     *
     * \remarks
     * This is an optional action.
     */
    virtual qint32 updateObject(
        const QString& objectId,
        const QStringList& currentTagValues,
        const QStringList& newTagValues);

    /*!
     * Moves an CDS object within the service to a different location in the
     * CDS object hierarchy.
     *
     * \param objectId specifies the object to be moved.
     *
     * \param newParentId specifies the new parent container.
     *
     * \param newObjectId specifies a pointer to a \c QString that contains the
     * new ID of the object.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HContentDirectoryInfo::ErrorCode
     *
     * \remarks
     * This is an optional action.
     */
    virtual qint32 moveObject(
        const QString& objectId,
        const QString& newParentId,
        QString* newObjectId);

    /*!
     * Transfers a resource from an external source to a local destination
     * accessible by the service.
     *
     * \param source specifies the URL of the resource to be imported.
     *
     * \param destination specifies the location where the resource is imported.
     *
     * \param transferId specifies a pointer to a \c quint32 that will contain
     * the ID of the transfer when the transfer is successfully started.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HContentDirectoryInfo::ErrorCode
     *
     * \remarks
     * This is an optional action.
     */
    virtual qint32 importResource(
        const QUrl& source, const QUrl& destination, quint32* transferId);

    /*!
     * Transfers a local resource to an external destination using HTTP POST.
     *
     * \param source specifies the URL of the resource to be exported.
     *
     * \param destination specifies the location where the resource is exported.
     *
     * \param transferId specifies a pointer to a \c quint32 that will contain
     * the ID of the transfer when the transfer is successfully started.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HContentDirectoryInfo::ErrorCode
     *
     * \remarks
     * This is an optional action.
     */
    virtual qint32 exportResource(
        const QUrl& source, const QUrl& destination, quint32* transferId);

    /*!
     * Removes a \c res property with a value equal to \a resourceUri
     * of all CDS objects.
     *
     * \param resourceUri specifies a value of a \c res property to be removed.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HContentDirectoryInfo::ErrorCode
     *
     * \remarks
     * This is an optional action.
     */
    virtual qint32 deleteResource(const QUrl& resourceUri);

    /*!
     * Stops the resource transfer initiated either by importResource() or
     * exportResource().
     *
     * \param transferId specifies the ID of the transfer to be canceled.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HContentDirectoryInfo::ErrorCode
     *
     * \remarks
     * This is an optional action.
     */
    virtual qint32 stopTransferResource(quint32 transferId);

    /*!
     * Queries the progress of a resource transfer.
     *
     * \param transferId specifies the transfer.
     *
     * \param transferInfo specifies a pointer to HTransferProgressInfo,
     * which will contain progress information about the specified transfer.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HContentDirectoryInfo::ErrorCode
     *
     * \remarks
     * This is an optional action.
     */
    virtual qint32 getTransferProgress(
        quint32 transferId, HTransferProgressInfo* transferInfo);

    /*!
     * Creates a reference to an existing item.
     *
     * \param containerId specifies the parent container.
     *
     * \param objectId specifies the target of the reference.
     *
     * \param newId specifies a pointer to \c QString, which will contain the
     * ID of the newly created reference object.
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HContentDirectoryInfo::ErrorCode
     *
     * \remarks
     * This is an optional action.
     */
    virtual qint32 createReference(
        const QString& containerId, const QString& objectId,
        QString* newId);

    /*!
     * Runs an XQuery request.
     *
     * \param containerId
     *
     * \param cdsView
     *
     * \param queryRequest
     *
     * \param result
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HContentDirectoryInfo::ErrorCode
     *
     * \remarks
     * This is an optional action.
     */
    virtual qint32 freeFormQuery(
        const QString& containerId, quint32 cdsView,
        const QString& queryRequest, HFreeFormQueryResult* result);

    /*!
     * \brief Retrieves the properties and their associated namespaces that can be
     * used in an XQuery request on this CDS instance.
     *
     * \param ffqCapabilities
     *
     * \retval Herqq::Upnp::UpnpErrorCode
     * \retval HContentDirectoryInfo::ErrorCode
     *
     * \remarks
     * This is an optional action.
     */
    virtual qint32 getFreeFormQueryCapabilities(QString* ffqCapabilities);
};

}
}
}

#endif /* HABSTRACTCONTENTDIRECTORY_SERVICE_H_ */
