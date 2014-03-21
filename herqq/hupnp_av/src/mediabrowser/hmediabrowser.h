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

#ifndef HMEDIABROWSER_H_
#define HMEDIABROWSER_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/HUpnp>

#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtCore/QObject>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HBrowseParamsPrivate;

/*!
 * \brief This class is used to configure a \e browse operation run by a HMediaBrowser.
 *
 * \headerfile hmediabrowser.h HBrowseParams
 *
 * \ingroup hupnp_av_cds_browsing
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HMediaBrowser::browse()
 */
class H_UPNP_AV_EXPORT HBrowseParams
{
public:

    /*!
     * \brief This enumeration defines the different browse operations available.
     */
    enum BrowseType
    {
        /*!
         * The browse targets a single CDS object.
         */
        SingleItem,

        /*!
         * The browse targets the children of the CDS container.
         */
        DirectChildren,

        /*!
         * The browse targets a CDS container \b and its children. Contrast this
         * to \c DirectChildren, which targets \b only the children.
         */
        ObjectAndDirectChildren,

        /*!
         * The browse targets a CDS container \b and its children recursively.
         */
        ObjectAndChildrenRecursively
    };

private:

    HBrowseParamsPrivate* h_ptr;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HBrowseParams();

    /*!
     * \brief Creates a new instance.
     *
     * \param browseType specifies the type of the browse operation.
     *
     * \sa isValid()
     */
    HBrowseParams(BrowseType browseType);

    /*!
     * \brief Creates a new instance.
     *
     * \param objectId specifies the ID of the target object.
     *
     * \param browseType specifies the type of the browse operation.
     *
     * \sa isValid()
     */
    HBrowseParams(const QString& objectId, BrowseType loadType);

    /*!
     * \brief Destroys the instance.
     */
    ~HBrowseParams();

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HBrowseParams(const HBrowseParams&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HBrowseParams& operator=(const HBrowseParams&);

    /*!
     * \brief Specifies the ID of the target object.
     *
     * \param id specifies the ID of the target object. In case of a recursive
     * browse, this is the ID of the root object.
     *
     * \sa objectId()
     */
    void setObjectId(const QString& id);

    /*!
     * \brief Specifies the type of the browse operation.
     *
     * \param type specifies the type of the browse operation.
     *
     * \sa browseType()
     */
    void setBrowseType(BrowseType type);

    /*!
     * \brief Specifies the \e filter for the operation.
     *
     * A filter specifies the CDS metadata properties that should be returned
     * by the server. A special value of \c "*" means that every available
     * metadata property should be returned. For more information, see the
     * ContentDirectory:3 specification, section 2.3.15.
     *
     * \param filter specifies the CDS metadata properties that the server
     * should include in the response.
     *
     * \sa filter()
     */
    void setFilter(const QSet<QString>& filter);

    /*!
     * \brief Indicates the validity of the object.
     *
     * \return \e true in case the object is valid, i.e. at least objectId()
     * is defined.
     */
    bool isValid() const;

    /*!
     * \brief Returns the ID of the target object.
     *
     * \return The ID of the target object.
     *
     * \sa setObjectId()
     */
    QString objectId() const;

    /*!
     * \brief Returns the type of the browse operation.
     *
     * \return The type of the browse operation.
     *
     * \sa setBrowseType()
     */
    BrowseType browseType() const;

    /*!
     * \brief Returns the CDS metadata properties that the server
     * should include in the response.
     *
     * \return The the CDS metadata properties that the server
     * should include in the response.
     *
     * \sa setFilter()
     */
    QSet<QString> filter() const;
};

class HMediaBrowserPrivate;

/*!
 * \brief This class provides a simple API for browsing a ContentDirectory service and
 * caching the contents of it.
 *
 * \headerfile hmediabrowser.h HMediaBrowser
 *
 * \ingroup hupnp_av_cds_browsing
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HMediaBrowser :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HMediaBrowser)
H_DECLARE_PRIVATE(HMediaBrowser)

private:

    HMediaBrowserPrivate* h_ptr;

public:

    /*!
     * \brief Creates a new instance.
     *
     * \param parent specifies parent \c QObject.
     *
     * \sa reset()
     */
    explicit HMediaBrowser(QObject* parent = 0);

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HMediaBrowser();

    /*!
     * Resets the ContentDirectory service being browsed.
     *
     * \param cds specifies the ContentDirectory service object to be browsed.
     *
     * \return \e true if the CDS object was successfully set and it is ready
     * to be browsed.
     *
     * \sa isReady()
     */
    bool reset(HClientService* cds);

    /*!
     * Resets the ContentDirectory service being browsed.
     *
     * \param cds specifies the ContentDirectory service object to be browsed.
     *
     * \param takeOwnership specifies whether the \c %HMediaBrowser instance
     * takes the ownership of the specified ContentDirectory instance.
     *
     * \return \e true if the CDS object was successfully set and it is ready
     * to be browsed.
     *
     * \sa isReady()
     */
    bool reset(HContentDirectoryAdapter* cds, bool takeOwnership);

    /*!
     * Initiates a browse operation based on the provided parameters.
     *
     * \params specifies the parameters for the browse operation.
     *
     * \return \e true when the operation was successfully dispatched.
     *
     * \remarks
     * This is an asynchronous operation.
     *
     * \sa browseComplete(), objectsBrowsed(), cancel()
     */
    bool browse(const HBrowseParams& params);

    /*!
     * Attempts to browse everything the ContentDirectory service exposes.
     *
     * This is a convenience method.
     *
     * \return \e true when the operation was successfully dispatched.
     *
     * \remarks
     * This is an asynchronous operation.
     *
     * \sa browse(), browseComplete(), objectsBrowsed(), cancel()
     */
    bool browseAll();

    /*!
     * Attempts to browse the metadata of a root container and everything directly
     * underneath it.
     *
     * This is a convenience method.
     *
     * \return \e true when the operation was successfully dispatched.
     *
     * \remarks
     * This is an asynchronous operation.
     *
     * \sa browse(), browseComplete()
     */
    bool browseRoot();

    /*!
     * \brief Returns the data source that contains all the loaded data.
     *
     * \return The data source that contains all the loaded data. The returned
     * pointer is \b never null and the ownership of the data source is
     * \b never transferred to the caller.
     */
    HCdsDataSource* dataSource() const;

    /*!
     * \brief Returns the ContentDirectory that is the target of browsing.
     *
     * \return The ContentDirectory that is the target of browsing.
     * This is null when the ContentDirectory service has not been set.
     *
     * \sa reset()
     */
    HContentDirectoryAdapter* contentDirectory() const;

    /*!
     * \brief Indicates if the object is ready for browsing.
     *
     * \return \e true when the object is ready for browsing.
     *
     * \sa reset(), isActive()
     */
    bool isReady() const;

    /*!
     * \brief Indicates if an browse operation is currently ongoing.
     *
     * \return \e true  if an browse operation is currently active.
     */
    bool isActive() const;

    /*!
     * Returns the last error code that occurred.
     *
     * \return the last error code that occurred. This is set to zero if no
     * error has occurred.
     *
     * \sa lastErrorDescription()
     */
    qint32 lastErrorCode() const;

    /*!
     * Returns the last error description that occurred.
     *
     * \return the last error description that occurred. The returned string is
     * empty if no error has occurred.
     *
     * \sa lastErrorDescription()
     */
    QString lastErrorDescription() const;

    /*!
     * Cancels currently active browse operation.
     *
     * This function does nothing if there is no browse operation currently active.
     */
    void cancel();

    /*!
     * Indicates if the object should automatically process LastChange events and
     * attempt to update its data source.
     *
     * \return \e true if the object should automatically process LastChange events and
     * attempt to update its data source.
     *
     * \sa setAutoUpdate()
     */
    bool isAutoUpdateEnabled();

    /*!
     * Specifies whether the object should automatically process LastChange events and
     * attempt to update its data source.
     *
     * \param enable specifies whether the object should automatically process
     * LastChange events and attempt to update its data source.
     *
     * \sa isAutoUpdateEnabled()
     */
    void setAutoUpdate(bool enable);

Q_SIGNALS:

    /*!
     * \brief This signal is emitted when the previously started load operation failed
     * before completion.
     *
     * \param source specifies the source of the event.
     *
     * \sa browseComplete()
     */
    void browseFailed(Herqq::Upnp::Av::HMediaBrowser* source);

    /*!
     * \brief This signal is emitted when a previously started browse operation is
     * successfully completed.
     *
     * \param source specifies the source of the event.
     *
     * \sa browseFailed(), objectsBrowsed()
     */
    void browseComplete(Herqq::Upnp::Av::HMediaBrowser* source);

    /*!
     * \brief This signal is emitted when new objects have been browsed and cached
     * by the instance.
     *
     * \brief This signal is emitted whenever the contents of a single CDS container
     * have been browsed and cached. This signal is especially useful in situations
     * where the browse operation involves multiple CDS containers, as it enables
     * progressive processing of the results while the operation is running
     * (before the browseComplete() is emitted).
     *
     * \param source specifies the source of the event.
     *
     * \param ids specifies the IDs of the objects browsed. You can now retrieve
     * these objects from the dataSource().
     *
     * \sa browseComplete(), browseFailed()
     */
    void objectsBrowsed(Herqq::Upnp::Av::HMediaBrowser* source, const QSet<QString>& ids);

    /*!
     * This signal is emitted when the instance has received LastChange data
     * from the ContentDirectoryService.
     *
     * \param data specifies the received LastChange data.
     */
    void receivedLastChange(const Herqq::Upnp::Av::HCdsLastChangeInfos& data);
};

}
}
}

#endif /* HMEDIABROWSER_H_ */
