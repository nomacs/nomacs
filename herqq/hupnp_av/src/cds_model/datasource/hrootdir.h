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

#ifndef HROOTDIR_H_
#define HROOTDIR_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QDir>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This class is used to contain information about a directory
 * the HFileSystemDataSource requires in order to scan the directory for content.
 *
 * \headerfile hrootdir.h HRootDir
 *
 * \ingroup hupnp_av_cds_ds
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HRootDir
{
public:

    /*!
     * \brief This enumeration specifies whether the HFileSystemDataSource should
     * watch the specified root directory for changes.
     */
    enum WatchMode
    {
        /*!
         * The data source should not monitor the specified directory (tree).
         */
        NoWatch,

        /*!
         * The data source should monitor the specified directory (tree).
         */
        WatchForChanges
    };

    /*!
     * \brief This enumeration specifies the methods for scanning an HRootDir
     * for content.
     */
    enum ScanMode
    {
        /*!
         * Only the contents found directly underneath the specified directory
         * should be scanned.
         */
        SingleDirectoryScan,

        /*!
         * The directory specified should be scanned recursively for content.
         */
        RecursiveScan
    };

private:

    QDir m_dir;
    ScanMode m_scanMode;
    WatchMode m_watchMode;

public:

    /*!
     * Creates a new instance.
     */
    HRootDir();

    /*!
     * \brief Creates a new instance.
     *
     * \param dir specifies the directory in the file system.
     *
     * \param scanMode specifies how the directory will be scanned.
     *
     * \param watchMode
     *
     * \sa isValid()
     */
    HRootDir(
        const QDir& dir,
        ScanMode scanMode = SingleDirectoryScan,
        WatchMode watchMode = NoWatch);

    /*!
     * \brief Returns the root directory.
     *
     * \return The root directory.
     *
     * \sa setDir()
     */
    inline QDir dir() const
    {
        return m_dir;
    }

    /*!
     * \brief Returns the scan mode.
     *
     * \return The scan mode.
     *
     * \sa setScanMode()
     */
    inline ScanMode scanMode() const
    {
        return m_scanMode;
    }

    /*!
     * \brief Returns the watch mode.
     *
     * \return The watch mode.
     *
     * \sa setWatchMode()
     *
     * \note Watch-mode is not currently implemented.
     */
    inline WatchMode watchMode() const
    {
        return m_watchMode;
    }

    /*!
     * Indicates if this instance overlaps the other root directory.
     *
     * An instance is considered to overlap another root directory instance when:
     * - they both refer to the same directory or
     * - \e the directory represented by this instance contains the other directory
     * and this instance is configured to scan sub-directories so that it encompasses
     * the directory represented by the other instance.
     *
     * \return \e true if this instance overlaps the other root directory.
     */
    bool overlaps(const HRootDir& other) const;

    /*!
     * Specifies the root directory.
     *
     * \param dir specifies the root directory.
     *
     * \return \e true if the directory exists.
     *
     * \sa dir()
     */
    bool setDir(const QDir& dir);

    /*!
     * Specifies the scan mode.
     *
     * \param smode specifies the scan mode.
     *
     * \sa scanMode()
     */
    void setScanMode(ScanMode smode);

    /*!
     * Specifies the watch mode.
     *
     * \param wmode specifies the watch mode.
     *
     * \sa watchMode()
     *
     * \note Watch-mode is not currently implemented.
     */
    void setWatchMode(WatchMode wmode);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HRootDir
 */
H_UPNP_AV_EXPORT bool operator==(const HRootDir& obj1, const HRootDir& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HRootDir
 */
inline bool operator!=(const HRootDir& obj1, const HRootDir& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* HROOTDIR_H_ */
