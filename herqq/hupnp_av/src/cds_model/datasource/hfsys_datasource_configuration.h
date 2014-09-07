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

#ifndef HFILESYSTEM_DATASOURCE_CONFIGURATION_H_
#define HFILESYSTEM_DATASOURCE_CONFIGURATION_H_

#include <HUpnpAv/HCdsDataSourceConfiguration>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HFileSystemDataSourceConfigurationPrivate;

/*!
 * \brief This class contains the configuration data passed to a HFileSystemDataSource.
 *
 * \headerfile hfsys_datasource_configuration.h HFileSystemDataSourceConfiguration
 *
 * \ingroup hupnp_av_cds_ds
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HFileSystemDataSource
 */
class H_UPNP_AV_EXPORT HFileSystemDataSourceConfiguration :
    public HCdsDataSourceConfiguration
{
H_DISABLE_COPY(HFileSystemDataSourceConfiguration)
H_DECLARE_PRIVATE(HFileSystemDataSourceConfiguration)

protected:

    //
    // \internal
    //
    HFileSystemDataSourceConfiguration(
        HFileSystemDataSourceConfigurationPrivate& dd);

    // Documented in HClonable
    virtual void doClone(HClonable* target) const;

    // Documented in HClonable
    virtual HFileSystemDataSourceConfiguration* newInstance() const;

public:

    /*!
     * \brief Creates a new instance.
     */
    HFileSystemDataSourceConfiguration();

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HFileSystemDataSourceConfiguration();

    // Documented in HClonable
    virtual HFileSystemDataSourceConfiguration* clone() const;

    /*!
     * \brief Returns the <em>root directories</em> contained by this object.
     *
     * A <em>root directory</em> represents a directory in the file system that
     * the HFileSystemDataSource scans for content.
     *
     * \return The root directories contained by this object.
     *
     * \sa setRootDirs(), addRootDir()
     */
    HRootDirs rootDirs() const;

    /*!
     * Adds a <em>root directory</em> to the configuration.
     *
     * A <em>root directory</em> represents a directory in the file system that
     * the HFileSystemDataSource scans for content.
     *
     * \return \e true if the directory was successfully added. The object is
     * added if it is valid, i.e. HRootDir::isValid() returns \e true.
     *
     * \sa rootDirs(), removeRootDir()
     */
    bool addRootDir(const HRootDir& dir);

    /*!
     * Removes a <em>root directory</em> from the configuration.
     *
     * \return \e true if the directory was found and removed.
     *
     * \sa rootDirs(), addRootDir()
     */
    bool removeRootDir(const HRootDir& dir);

    /*!
     * \brief Sets the <em>root directories</em> for this object.
     *
     * A <em>root directory</em> represents a directory in the file system that
     * the HFileSystemDataSource will scan for content.
     *
     * \param dirs specifies the root directories for this object.
     *
     * \return \e true if the directories were successfully set. This requires
     * that each HRootDir object is valid, i.e. HRootDir::isValid() returns
     * \e true.
     *
     * \sa rootDirs(), addRootDir()
     */
    bool setRootDirs(const HRootDirs& dirs);

    /*!
     * Clears the state of the object, such as removes all root directories.
     */
    void clear();
};

}
}
}

#endif /* HFILESYSTEM_DATASOURCE_CONFIGURATION_H_ */
