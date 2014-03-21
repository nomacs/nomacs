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

#ifndef HABSTRACT_CDS_DATASOURCE_H_
#define HABSTRACT_CDS_DATASOURCE_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QObject>

class QIODevice;

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HAbstractCdsDataSourcePrivate;

/*!
 * \brief This class is used to store instances of the HUPnPAv CDS object model.
 *
 * \headerfile habstract_cds_datasource.h HAbstractCdsDataSource
 *
 * \ingroup hupnp_av_cds_ds
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HAbstractCdsDataSourceConfiguration
 */
class H_UPNP_AV_EXPORT HAbstractCdsDataSource :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HAbstractCdsDataSource)
H_DECLARE_PRIVATE(HAbstractCdsDataSource)

private Q_SLOTS:

    void objectModified_(
        Herqq::Upnp::Av::HObject* source,
        const Herqq::Upnp::Av::HObjectEventInfo& eventInfo);

    void containerModified_(
        Herqq::Upnp::Av::HContainer* source,
        const Herqq::Upnp::Av::HContainerEventInfo& eventInfo);

public:

    /*!
     * \brief This enumeration specifies the different modes of adding objects into
     * the data source.
     */
    enum AddFlag
    {
        /*!
         * Add an object or objects only if their IDs are not found in the data
         * source already.
         */
        AddNewOnly,

        /*!
         * Add an object or objects even if their IDs are already found in the
         * data source.
         *
         * If an object with an already existing ID is to be added, the existing
         * object is deleted first.
         */
        AddAndOverwrite
    };

    /*!
     * \brief This enumeration specifies the error situations the HAbstractCdsDataSource might
     * encounter.
     */
    enum DataSourceError
    {
        /*!
         * No error has occurred.
         */
        NoError = 0,

        /*!
         * No information about the error is available.
         */
        UndefinedError
    };

protected:

    HAbstractCdsDataSourcePrivate* h_ptr;

    //
    // \internal
    //
    HAbstractCdsDataSource(HAbstractCdsDataSourcePrivate& dd, QObject* parent = 0);

    /*!
     * \brief Sets the type and description of the last occurred error.
     *
     * \param error specifies the error type.
     * \param errorDescr specifies a human readable description of the error.
     *
     * \sa error(), errorDescription()
     */
    void setLastError(DataSourceError error, const QString& errorDescr);

    /*!
     * \brief Creates a new instance.
     *
     * \param parent specifies the parent \c QObject.
     *
     * \sa init(), isInitialized()
     *
     * \remarks a default configuration is created for the data source.
     */
    HAbstractCdsDataSource(QObject* parent = 0);

    /*!
     * \brief Creates a new instance.
     *
     * \param conf specifies the configuration of the data source.
     *
     * \param parent specifies the parent \c QObject.
     *
     * \sa init(), isInitialized()
     */
    HAbstractCdsDataSource(const HCdsDataSourceConfiguration& conf, QObject* parent = 0);

    /*!
     * Performs the initialization of a derived class.
     *
     * The \c %HAbstractCdsDataSource uses two-phase initialization in which the user
     * first constructs an instance and then calls init() in order to ready
     * the object for use. This method is called by the \c %HAbstractCdsDataSource
     * during its private initialization after all the private data structures
     * are constructed.
     *
     * You can override this method to perform any further initialization of a
     * derived class.
     *
     * \return \e true if and only if the initialization succeeded.
     * If \e false is returned the initialization of the data source is
     * aborted. In addition, it is advised that you call setError()
     * before returning.
     *
     * \remarks the default implementation does nothing and returns \e true
     * always.
     *
     * \sa init()
     */
    virtual bool doInit();

    /*!
     * \brief Returns the configuration of the data source.
     *
     * \return The configuration of the data source.
     *
     * \remarks
     * \li this is not a copy and the ownership of the returned pointer to
     * object is not transferred to the caller.
     *
     * \li this method always returns a valid pointer, even if the user did
     * not provide a configuration upon construction. In such a case a
     * default configuration object is created.
     */
    virtual const HCdsDataSourceConfiguration* configuration() const;

    /*!
     * Removes and deletes all the objects in the data source.
     *
     * \sa count()
     */
    virtual void clear();

    /*!
     * Adds CDS objects to the data source.
     *
     * \param objects specifies the objects to add.
     *
     * \param addFlag specifies whether to add and overwrite possibly existing
     * objects with equal identifiers.
     *
     * \return a list of objects that were not added into the data source.
     * \note The ownership of these objects remains at the caller.
     *
     * \remarks The data source takes the ownership of the provided HObjects that
     * were successfully added.
     *
     * \sa HObject::id()
     */
    HObjects add(const HObjects& objects, AddFlag addFlag=AddNewOnly);

    /*!
     * Adds the specified CDS object to the data source.
     *
     * \param object specifies the object to add.
     *
     * \param addFlag specifies whether to add and overwrite possibly existing
     * object with equal identifier.
     *
     * \return \e true in case the object was successfully added.
     *
     * \remarks The data source takes the ownership of the provided HObject \b if
     * it is successfully added.
     *
     * \sa HObject::id()
     */
    bool add(HObject* object, AddFlag addFlag=AddNewOnly);

    /*!
     * Removes the specified object from the data source.
     *
     * \param object specifies the object to be removed.
     *
     * \return \e true if the object was found and removed.
     *
     * \remarks This is a constant-time operation.
     */
    bool remove(const QString& id);

    /*!
     * Removes the specified objects from the data source.
     *
     * \param objects specifies the objects to be removed.
     *
     * \return The number of removed objects.
     */
    qint32 remove(const HObjects& objects);

    /*!
     * Removes the specified objects from the data source.
     *
     * \param ids specifies the IDs of the objects to be removed.
     *
     * \return The number of removed objects.
     */
    qint32 remove(const QSet<QString>& ids);

public:

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HAbstractCdsDataSource() = 0;

    /*!
     * Initializes the data source. After a successful call, the data
     * source is ready to use.
     *
     * \return true on success.
     *
     * \sa isInitialized()
     */
    bool init();

    /*!
     * \brief Indicates whether the data source is appropriately initialized and
     * ready to be used.
     *
     * \return \e true in case the data source is appropriately initialized and
     * ready to be used.
     *
     * \sa init()
     */
    bool isInitialized() const;

    /*!
     * \brief Indicates if the data source supports loading the possible data associated
     * with CDS items.
     *
     * \return \e true in case the data source supports loading the possible
     * data associated with CDS items.
     *
     * \remarks The default implementation does not support loading of data.
     * Override this method in derived classes that support loading of data.
     */
    virtual bool supportsLoading() const;

    /*!
     * \brief Indicates if the specified itemId corresponds to an HItem, which data
     * can be loaded by this data source.
     *
     * \param itemId specifies the ID of the HItem to be checked.
     *
     * \return \e true if the specified itemId corresponds to an HItem, which data
     * can be loaded by this data source.
     *
     * \remarks The default implementation returns \c false always, as it doesn't
     * support loading of data. Override this method in derived classes that
     * support loading of data.
     */
    virtual bool isLoadable(const QString& itemId) const;

    /*!
     * Loads the data of the specified HItem.
     *
     * \param itemId specifies the ID of the HItem, which data is to be loaded.
     *
     * \return a pointer to a \c QIODevice opened for reading when the
     * load succeeds. The ownership of the pointer \b is transferred to the caller.
     * If the load fails a null pointer is returned.
     *
     * \remarks The default implementation returns a null pointer always, as it
     * doesn't support loading of data. Override this method in derived classes
     * that support loading of data.
     */
    virtual QIODevice* loadItemData(const QString& itemId);

    /*!
     * Attempts to find an object with the given object ID.
     *
     * \param objectId specifies the object to be searched.
     *
     * \return The object with the given object ID, or null, if no contained
     * object has the specified ID.
     *
     * \remarks
     * \li this is a constant-time operation.
     *
     * \li the ownership of the returned pointer to object is \b not transferred
     * to the caller.
     *
     * \sa findItem(), findContainer()
     */
    HObject* findObject(const QString& objectId);

    /*!
     * Attempts to find objects with the specified object IDs.
     *
     * \param objectIds specifies the object IDs to be searched.
     *
     * \return The objects with the given object IDs.
     *
     * \remarks
     * \li IDs that are not found are ignored and because of that the return value
     * may not contain an equal number of objects compared to the number of
     * IDs specified by the user.
     *
     * \li the ownership of the returned pointers are \b not transferred
     * to the caller.
     *
     * \sa findItems(), findContainers()
     */
    HObjects findObjects(const QSet<QString>& objectIds);

    /*!
     * Indicates if the datasource has a container with the specified title.
     *
     * \return true in case the datasource contains a container with the
     * specified title.
     */
    HContainer* findContainerWithTitle(const QString& title);

    /*!
     * \brief Returns all the objects this data source contains.
     *
     * \return all the objects this data source contains.
     *
     * \sa items(), containers()
     */
    HObjects objects() const;

    /*!
     * \brief Returns the number of objects this data source contains.
     *
     * \return The number of objects this data source contains.
     */
    qint32 count() const;

    /*!
     * Attempts to find an HItem with the given ID.
     *
     * \param itemId specifies the item to be searched.
     *
     * \return The HItem with the given ID, or null, if no contained
     * HItem has the specified ID.
     *
     * \remarks
     * \li this is a constant-time operation.
     *
     * \li the ownership of the returned pointer to object is \b not transferred
     * to the caller.
     *
     * \sa findObject(), findContainer()
     */
    HItem* findItem(const QString& itemId);

    /*!
     * Attempts to find HItems with the specified IDs.
     *
     * \param itemIDs specifies the IDs to be searched.
     *
     * \return The HItems with the given IDs.
     *
     * \remarks
     * \li IDs that are not found are ignored and because of that the return value
     * may not contain an equal number of items compared to the number of
     * IDs specified by the user.
     *
     * \li the ownership of the returned pointers are \b not transferred
     * to the caller.
     *
     * \sa findObjects(), findContainers()
     */
    HItems findItems(const QSet<QString>& itemIds);

    /*!
     * \brief Returns all the HItems this data source contains.
     *
     * \return all the HItems this data source contains.
     *
     * \sa objects(), containers()
     */
    HItems items() const;

    /*!
     * Attempts to find an HContainer with the given ID.
     *
     * \param itecontainerIdmId specifies the item to be searched.
     *
     * \return The HContainer with the given ID, or null, if no contained
     * HContainer has the specified ID.
     *
     * \remarks
     * \li this is a constant-time operation.
     *
     * \li the ownership of the returned pointer to object is \b not transferred
     * to the caller.
     *
     * \sa findItem(), findObject()
     */
    HContainer* findContainer(const QString& containerId);

    /*!
     * Attempts to find HContainers with the specified IDs.
     *
     * \param containerIds specifies the IDs to be searched.
     *
     * \return The HContainers with the given IDs.
     *
     * \remarks
     * \li IDs that are not found are ignored and because of that the return value
     * may not contain an equal number of items compared to the number of
     * IDs specified by the user.
     *
     * \li the ownership of the returned pointers are \b not transferred
     * to the caller.
     *
     * \sa findItems(), findObjects()
     */
    HContainers findContainers(const QSet<QString>& containerIds);

     /*!
     * \brief Returns all the HContainers this data source contains.
     *
     * \return all the HContainers this data source contains.
     *
     * \sa objects(), items()
     */
    HContainers containers() const;

Q_SIGNALS:

    /*!
     * \brief This signal is emitted when an object has been modified.
     *
     * \param source specifies the HObject that has been modified.
     *
     * \param eventInfo specifies information of the \e Object \e Modification that
     * occurred.
     */
    void objectModified(
        Herqq::Upnp::Av::HObject* source,
        const Herqq::Upnp::Av::HObjectEventInfo& eventInfo);

    /*!
     * \brief This signal is emitted when a container has been modified.
     *
     * \param source specifies the HContainer that has been modified.
     *
     * \param eventInfo specifies information of the \e Container \e Modification that
     * occurred.
     */
    void containerModified(
        Herqq::Upnp::Av::HContainer* source,
        const Herqq::Upnp::Av::HContainerEventInfo& eventInfo);

    /*!
     * \brief This signal is emitted when an object without a parent has been added
     * into the data source.
     *
     * \param source specifies the HObject that has been added.
     */
    void independentObjectAdded(Herqq::Upnp::Av::HObject* source);
};

}
}
}

#endif /* HABSTRACT_CDS_DATASOURCE_H_ */
