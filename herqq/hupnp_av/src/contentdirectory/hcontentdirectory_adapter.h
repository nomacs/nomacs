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

#ifndef HCONTENTDIRECTORY_H_
#define HCONTENTDIRECTORY_H_

#include <HUpnpAv/HSearchResult>
#include <HUpnpAv/HCreateObjectResult>
#include <HUpnpAv/HFreeFormQueryResult>
#include <HUpnpAv/HTransferProgressInfo>
#include <HUpnpAv/HContentDirectoryInfo>

#include <HUpnpCore/HClientAdapterOp>
#include <HUpnpCore/HClientServiceAdapter>

#include <QtCore/QStringList>

class QUrl;

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HContentDirectoryAdapterPrivate;

/*!
 * \brief This is a convenience class for using a ContentDirectory service.
 *
 * This is a convenience class that provides a simple asynchronous API for
 * accessing server-side ContentDirectory service from the client-side.
 * The class can be instantiated with a HClientService that
 * provides the mandatory functionality of a ContentDirectory.
 *
 * For more information, see the
 * <a href=http://upnp.org/specs/av/UPnP-av-ContentDirectory-v3-Service.pdf>
 * UPnP ContentDirectory:3 specification</a>.
 *
 * \headerfile hcontentdirectory.h HContentDirectoryAdapter
 *
 * \ingroup hupnp_av_cds
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HContentDirectoryAdapter :
    public HClientServiceAdapter
{
Q_OBJECT
H_DISABLE_COPY(HContentDirectoryAdapter)
H_DECLARE_PRIVATE(HContentDirectoryAdapter)

private Q_SLOTS:

    void lastChange(
        const Herqq::Upnp::HClientStateVariable*,
        const Herqq::Upnp::HStateVariableEvent&);

protected:

    // Documented in HClientServiceAdapter.
    virtual bool prepareService(HClientService* service);

public:

    /*!
     * \brief Creates a new instance.
     *
     * \param specifies the \c QObject parent.
     *
     * \sa isReady(), setService()
     */
    HContentDirectoryAdapter(QObject* parent = 0);

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HContentDirectoryAdapter();

    /*!
     * Indicates if the ContentDirectoryService supports \e Last \e Change.
     *
     * \return \e true if the ContentDirectoryService supports \e Last \e Change.
     *
     * \sa lastChangeReceived()
     */
    bool isLastChangeEnabled() const;

    /*!
     * \brief Retrieves the search capabilities supported by the device.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getSearchCapabilitiesCompleted()
     */
    HClientAdapterOp<QStringList> getSearchCapabilities();

    /*!
     * \brief Retrieves the sort capabilities supported by the device.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getSortExtensionCapabilitiesCompleted()
     */
    HClientAdapterOp<QStringList> getSortCapabilities();

    /*!
     * \brief Retrieves the sort modifiers supported by the device.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getSortExtensionCapabilitiesCompleted()
     */
    HClientAdapterOp<QStringList> getSortExtensionCapabilities();

    /*!
     * \brief Retrieves the optional CDS features the service supports, if any.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getFeatureListCompleted()
     */
    HClientAdapterOp<QString> getFeatureList();

    /*!
     * \brief Retrieves the current value of the state variable \c SystemUpdateId.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getSystemUpdateIdCompleted()
     */
    HClientAdapterOp<quint32> getSystemUpdateId();

    /*!
     * \brief Retrieves the current value of the state variable \c ServiceResetToken.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getServiceResetTokenCompleted()
     */
    HClientAdapterOp<QString> getServiceResetToken();

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
     * HContentDirectoryAdapterInfo::BrowseMetadata. In addition, the \a objectId has
     * to identify a \e container object.
     *
     * \param requestedCount specifies the maximum number objects to be browsed.
     * A value of \c "0" indicates that all objects under the specified \a objectId
     * should be browsed.
     *
     * \param sortCriteria specifies the sort criteria to be applied to the
     * returned object information.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), browseCompleted()
     */
    HClientAdapterOp<HSearchResult> browse(
        const QString& objectId,
        HContentDirectoryInfo::BrowseFlag browseFlag,
        const QSet<QString>& filter,
        quint32 startingIndex,
        quint32 requestedCount,
        const QStringList& sortCriteria);

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
     * HContentDirectoryAdapterInfo::BrowseMetadata. In addition, the \a objectId has
     * to identify a \e container object.
     *
     * \param requestedCount specifies the maximum number objects to be browsed.
     * A value of \c "0" indicates that all objects under the specified \a objectId
     * should be browsed.
     *
     * \param sortCriteria specifies the sort criteria to be applied to the
     * returned object information.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), searchCompleted()
     */
     HClientAdapterOp<HSearchResult> search(
        const QString& containerId,
        const QString& searchCriteria,
        const QSet<QString>& filter,
        quint32 startingIndex,
        quint32 requestedCount,
        const QStringList& sortCriteria);

    /*!
     * Creates a new object under the specified container.
     *
     * \param containerId specifies the parent container for the new object.
     *
     * \param elements specifies a DIDL-Lite XML document that describes the
     * new object.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), createObjectCompleted()
     */
    HClientAdapterOp<HCreateObjectResult> createObject(
        const QString& containerId,
        const QString& elements);

    /*!
     * Destroys and removes the specified object.
     *
     * \param objectId specifies the object to be removed.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), destroyObjectCompleted()
     */
    HClientAdapterOpNull destroyObject(const QString& objectId);

    /*!
     * Modifies the metadata of the specified object.
     *
     * \param objectId specifies the object to be modified.
     *
     * \param currentTagValues specifies the properties to be modified.
     *
     * \param newTagValues specifies the new values for properties to be modified.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), updateObjectCompleted()
     */
    HClientAdapterOpNull updateObject(
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
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), moveObjectCompleted()
     */
    HClientAdapterOp<QString> moveObject(
        const QString& objectId,
        const QString& newParentId);

    /*!
     * Transfers a resource from an external source to a local destination
     * accessible by the service.
     *
     * \param source specifies the URL of the resource to be imported.
     *
     * \param destination specifies the location where the resource is imported.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), importResourceCompleted()
     */
    HClientAdapterOp<quint32> importResource(
        const QUrl& source, const QUrl& destination);

    /*!
     * Transfers a local resource to an external destination using HTTP POST.
     *
     * \param source specifies the URL of the resource to be exported.
     *
     * \param destination specifies the location where the resource is exported.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), exportResourceCompleted()
     */
    HClientAdapterOp<quint32> exportResource(
        const QUrl& source, const QUrl& destination);

    /*!
     * Removes a \c res property with a value equal to \a resourceUri
     * of all CDS objects.
     *
     * \param resourceUri specifies a value of a \c res property to be removed.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), deleteResourceCompleted()
     */
    HClientAdapterOpNull deleteResource(const QUrl& resourceUrl);

    /*!
     * Stops the resource transfer initiated either by importResource() or
     * exportResource().
     *
     * \param transferId specifies the ID of the transfer to be canceled.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), stopTransferResourceCompleted()
     */
    HClientAdapterOpNull stopTransferResource(quint32 transferId);

    /*!
     * Queries the progress of a resource transfer.
     *
     * \param transferId specifies the transfer.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getTransferProgressCompleted()
     */
    HClientAdapterOp<HTransferProgressInfo> getTransferProgress(quint32 transferId);

    /*!
     * Creates a reference to an existing item.
     *
     * \param containerId specifies the parent container.
     *
     * \param objectId specifies the target of the reference.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), createReferenceCompleted()
     */
    HClientAdapterOp<QString> createReference(
        const QString& containerId, const QString& objectId);

    /*!
     * Runs an XQuery request.
     *
     * \param containerId
     *
     * \param cdsView
     *
     * \param queryRequest
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), freeFormQueryCompleted()
     */
    HClientAdapterOp<HFreeFormQueryResult> freeFormQuery(
        const QString& containerId, quint32 cdsView,
        const QString& queryRequest);

    /*!
     * \brief Retrieves the properties and their associated namespaces that can be
     * used in an XQuery request on this CDS instance.
     *
     * \return The ID used to identify the asynchronous operation. The returned
     * object is invalid in case the object is not ready for use or the target
     * UPnP device does not support this optional action.
     *
     * \remarks This is an asynchronous method.
     *
     * \sa isReady(), getFreeFormQueryCapabilitiesCompleted()
     */
    HClientAdapterOp<QString> getFreeFormQueryCapabilities();

Q_SIGNALS:

    /*!
     * \brief This signal is emitted when getSearchCapabilities() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getSearchCapabilities().
     *
     * \sa getSearchCapabilities()
     */
    void getSearchCapabilitiesCompleted(
        Herqq::Upnp::Av::HContentDirectoryAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<QStringList>& op);

    /*!
     * \brief This signal is emitted when getSortCapabilities() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getSortCapabilities().
     *
     * \sa getSortCapabilities()
     */
    void getSortCapabilitiesCompleted(
        Herqq::Upnp::Av::HContentDirectoryAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<QStringList>& op);

    /*!
     * \brief This signal is emitted when getSortExtensionCapabilities() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getSortExtensionCapabilities().
     *
     * \sa getSortExtensionCapabilities()
     */
    void getSortExtensionCapabilitiesCompleted(
        Herqq::Upnp::Av::HContentDirectoryAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<QStringList>& op);

    /*!
     * \brief This signal is emitted when getFeatureList() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getFeatureList().
     *
     * The return value of the operation upon success is a \c QString that contains
     * a <em>Features XML document</em>, which defines the optional
     * CDS features the service supports, if any.
     *
     * \sa getFeatureList()
     */
    void getFeatureListCompleted(
        Herqq::Upnp::Av::HContentDirectoryAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<QString>& op);

    /*!
     * \brief This signal is emitted when getSystemUpdateId() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getSystemUpdateId().
     *
     * The return value of the operation upon success is a \c quint32 that will contain
     * the current value of the state variable \c SystemUpdateId.
     *
     * \sa getSystemUpdateId()
     */
    void getSystemUpdateIdCompleted(
        Herqq::Upnp::Av::HContentDirectoryAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<quint32>& op);

    /*!
     * \brief This signal is emitted when getServiceResetToken() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getServiceResetToken().
     *
     * The return value of the operation upon success is a \c QString that will contain
     * the current value of the state variable \c ServiceResetToken.
     *
     * \sa getServiceResetToken()
     */
    void getServiceResetTokenCompleted(
        Herqq::Upnp::Av::HContentDirectoryAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<QString>& op);

    /*!
     * \brief This signal is emitted when browse() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * browse().
     *
     * \sa browse()
     */
    void browseCompleted(
        Herqq::Upnp::Av::HContentDirectoryAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HSearchResult>& op);

    /*!
     * \brief This signal is emitted when search() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * search().
     *
     * \sa search()
     */
    void searchCompleted(
        Herqq::Upnp::Av::HContentDirectoryAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HSearchResult>& op);

    /*!
     * \brief This signal is emitted when createObject() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * createObject().
     *
     * \sa createObject()
     */
    void createObjectCompleted(
        Herqq::Upnp::Av::HContentDirectoryAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HCreateObjectResult>& op);

    /*!
     * \brief This signal is emitted when destroyObject() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * destroyObject().
     *
     * \sa destroyObject()
     */
    void destroyObjectCompleted(
        Herqq::Upnp::Av::HContentDirectoryAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when updateObject() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * updateObject().
     *
     * \sa updateObject()
     */
    void updateObjectCompleted(
        Herqq::Upnp::Av::HContentDirectoryAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when moveObject() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * moveObject().
     *
     * The return value of the operation upon success is a \c QString that
     * contains the new ID of the object.
     *
     * \sa moveObject()
     */
    void moveObjectCompleted(
        Herqq::Upnp::Av::HContentDirectoryAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<QString>& op);

    /*!
     * \brief This signal is emitted when importResource() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * importResource().
     *
     * The return value of the operation upon success is a \c quint32 that contains
     * the ID of the transfer.
     *
     * \sa importResource()
     */
    void importResourceCompleted(
        Herqq::Upnp::Av::HContentDirectoryAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<quint32>& op);

    /*!
     * \brief This signal is emitted when exportResource() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * exportResource().
     *
     * The return value of the operation upon success is a \c quint32 that contains
     * the ID of the transfer.
     *
     * \sa exportResource()
     */
    void exportResourceCompleted(
        Herqq::Upnp::Av::HContentDirectoryAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<quint32>& op);

    /*!
     * \brief This signal is emitted when deleteResource() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * deleteResource().
     *
     * \sa deleteResource()
     */
    void deleteResourceCompleted(
        Herqq::Upnp::Av::HContentDirectoryAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when stopTransferResource() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * stopTransferResource().
     *
     * \sa stopTransferResource()
     */
    void stopTransferResourceCompleted(
        Herqq::Upnp::Av::HContentDirectoryAdapter* source,
        const Herqq::Upnp::HClientAdapterOpNull& op);

    /*!
     * \brief This signal is emitted when getTransferProgress() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getTransferProgress().
     *
     * \sa getTransferProgress()
     */
    void getTransferProgressCompleted(
        Herqq::Upnp::Av::HContentDirectoryAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HTransferProgressInfo>& op);

    /*!
     * \brief This signal is emitted when createReference() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * createReference().
     *
     * The return value of the operation upon success is a \c QString that contains
     * the ID of the newly created reference object.
     *
     * \sa createReference()
     */
    void createReferenceCompleted(
        Herqq::Upnp::Av::HContentDirectoryAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<QString>& op);

    /*!
     * \brief This signal is emitted when freeFormQuery() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * freeFormQuery().
     *
     * \sa freeFormQuery()
     */
    void freeFormQueryCompleted(
        Herqq::Upnp::Av::HContentDirectoryAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<Herqq::Upnp::Av::HFreeFormQueryResult>& op);

    /*!
     * \brief This signal is emitted when getFreeFormQueryCapabilities() has completed.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param op specifies the asynchronous operation previously started by
     * getFreeFormQueryCapabilities().
     *
     * \sa getFreeFormQueryCapabilities()
     */
    void getFreeFormQueryCapabilitiesCompleted(
        Herqq::Upnp::Av::HContentDirectoryAdapter* source,
        const Herqq::Upnp::HClientAdapterOp<QString>& op);

    /*!
     * \brief This signal is emitted when a \e Last \e Change event has been
     * received from the ContentDirectoryService.
     *
     * \param source specifies the HContentDirectoryAdapter instance that
     * sent the event.
     *
     * \param data specifies the <em>LastChange XML Document</em> following
     * the schema available <a href="http://www.upnp.org/schemas/av/cds-event-v1.xsd">here</a>.
     *
     * \sa isLastChangeEnabled()
     */
    void lastChangeReceived(
        Herqq::Upnp::Av::HContentDirectoryAdapter* source, const QString& data);
};

}
}
}

#endif /* HCONTENTDIRECTORY_H_ */
