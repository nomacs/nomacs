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

#ifndef HFILESYSTEM_DATASOURCE_H_
#define HFILESYSTEM_DATASOURCE_H_

#include <HUpnpAv/HCdsDataSource>
#include <HUpnpAv/HFileSystemDataSourceConfiguration>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HFileSystemDataSourcePrivate;

/*!
 * \brief This class is used to create and store instances of the HUPnPAv CDS
 * object model from the files and directories on the local file system.
 *
 * \headerfile hfsys_datasource.h HFileSystemDataSource
 *
 * \ingroup hupnp_av_cds_ds
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HFileSystemDataSourceConfiguration
 */
class H_UPNP_AV_EXPORT HFileSystemDataSource :
    public HAbstractCdsDataSource
{
Q_OBJECT
H_DISABLE_COPY(HFileSystemDataSource)
H_DECLARE_PRIVATE(HFileSystemDataSource)

protected:

    //
    // \internal
    //
    HFileSystemDataSource(HFileSystemDataSourcePrivate& dd, QObject* parent = 0);

    // Documented in HAbstractCdsDataSource
    virtual bool doInit();

public:

    /*!
     * \brief Creates a new instance.
     *
     * \param parent specifies the \c QObject parent.
     */
    HFileSystemDataSource(QObject* parent = 0);

    /*!
     * \brief Creates a new instance.
     *
     * \param conf specifies the configuration for the data source.
     *
     * \param parent specifies the \c QObject parent.
     */
    HFileSystemDataSource(
        const HFileSystemDataSourceConfiguration& conf, QObject* parent = 0);

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HFileSystemDataSource();

    // Documented in HAbstractCdsDataSource
    virtual const HFileSystemDataSourceConfiguration* configuration() const;

    // Documented in HAbstractCdsDataSource
    virtual bool supportsLoading() const;

    // Documented in HAbstractCdsDataSource
    virtual bool isLoadable(const QString& itemId) const;

    // Documented in HAbstractCdsDataSource
    virtual QIODevice* loadItemData(const QString& itemId);

    // Documented in HAbstractCdsDataSource
    virtual void clear();

    /*!
     * Adds a new CDS container to the data source.
     *
     * \param cdsContainer specifies the container to be added.
     *
     * \param addFlag specifies the addition mode.
     *
     * \return \e true in case the container was successfully added.
     *
     * \remarks The data source takes the ownership of the provided HContainer \b if
     * it is successfully added.
     */
    bool add(HContainer* cdsContainer, AddFlag addFlag=AddNewOnly);

    /*!
     * Creates and adds a new CDS item to the data source.
     *
     * The function attempts to create the correct HItem* derivative based on
     * the provided file path. For example, if the path is "photo.png" the
     * function returns an instance of HPhoto.
     *
     * \param path specifies the absolute path to the file in the local
     * file system that contains the data the new item is supposed to represent.
     *
     * \param parentId specifies the ID of the CDS container that should be the
     * logical parent of the new item.
     *
     * \param addFlag specifies the addition mode.
     *
     * \return the item that was created and added into the data source,
     * or null if either the creation or addition failed.
     *
     * \remarks The data source takes the ownership of the returned HItem.
     */
    HItem* add(const QString& path, const QString& parentId, AddFlag addFlag=AddNewOnly);

    /*!
     * Adds a new CDS object to the data source.
     *
     * \param cdsItem specifies the item to be added.
     *
     * \param path specifies the absolute path to the file in the local
     * file system that contains the data the \a cdsItem represents.
     *
     * \param addFlag specifies the addition mode.
     *
     * \return \e true in case the item was successfully added.
     *
     * \remarks The data source takes the ownership of the provided HItem \b if
     * it is successfully added.
     */
    bool add(HItem* cdsItem, const QString& path, AddFlag addFlag=AddNewOnly);

    /*!
     * Adds a new CDS object to the data source.
     *
     * \param rootDir specifies the directory to be scanned for content.
     *
     * \param addFlag specifies the addition mode.
     *
     * \return the number CDS objects that were found, created and added.
     */
    qint32 add(const HRootDir& rootDir, AddFlag addFlag=AddNewOnly);

    /*!
     * \brief Returns the absolute path to the file in the local file system corresponding
     * the specified CDS object ID.
     *
     * \param itemId specifies the CDS item, which absolute path should be
     * returned.
     *
     * \return the absolute path to the file in the local file system corresponding
     * the specified CDS item ID. The returned string is empty, in case there's no CDS item
     * with the specified ID or the specified item is not associated with
     * any local file at the time of the call.
     */
    QString getPath(const QString& itemId) const;
};

}
}
}

#endif /* HFILESYSTEM_DATASOURCE_H_ */
