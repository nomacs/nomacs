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

#ifndef HTRANSFERPROGRESSINFO_H_
#define HTRANSFERPROGRESSINFO_H_

#include <HUpnpAv/HUpnpAv>
#include <QtCore/QSharedDataPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HTransferProgressInfoPrivate;

/*!
 * \brief This is a class that contains progress information of a transfer.
 *
 * \headerfile htransferinfo.h HTransferProgressInfo
 *
 * \ingroup hupnp_av_cds
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HAbstractContentDirectoryService::getTransferProgress()
 * \sa HContentDirectoryAdapter::getTransferProgress()
 */
class H_UPNP_AV_EXPORT HTransferProgressInfo
{
public:

    /*!
     * \brief This enumeration specifies the different states of a transfer.
     */
    enum Status
    {
        /*!
         * An error has occured and the transfer has been aborted.
         */
        Error,

        /*!
         * The transfer is in progress.
         */
        InProgress,

        /*!
         * The transfer has been stopped.
         */
        Stopped,

        /*!
         * The transfer has been successfully completed.
         */
        Completed
    };

    /*!
     * \brief Returns a Status value corresponding to the specified string.
     *
     * \param arg specifies the string.
     *
     * \return The corresponding Status value.
     */
    static Status fromString(const QString& arg);

private:

    QSharedDataPointer<HTransferProgressInfoPrivate> h_ptr;

public:

    /*!
     * \brief Creates a new, empty instance.
     *
     * \sa isEmpty()
     */
    HTransferProgressInfo();

    /*!
     * \brief Creates a new instance.
     *
     * \param length specifies the number of bytes that have been transferred.
     *
     * \param status specifies the status of the transfer.
     *
     * \param total specifies the total number of bytes that is expected to be transferred.
     *
     * \sa isEmpty()
     */
    HTransferProgressInfo(quint32 length, Status status, quint32 total);

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HTransferProgressInfo(const HTransferProgressInfo&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     *
     * \return a reference to this.
     */
    HTransferProgressInfo& operator=(const HTransferProgressInfo&);

    /*!
     * \brief Destroys the instance.
     */
    ~HTransferProgressInfo();

    /*!
     * \brief Indicates if the object is empty.
     *
     * \return \e true if the object is empty, i.e. none of the attributes
     * is defined.
     */
    bool isEmpty() const;

    /*!
     * \brief Returns the number of bytes that have been transferred.
     *
     * \return The number of bytes that have been transferred.
     */
    quint32 length() const;

    /*!
     * \brief Returns the status of the transfer.
     *
     * \return The status of the trasnfer.
     */
    Status status() const;

    /*!
     * \brief Returns the total number of bytes that is expected to be transferred.
     *
     * \return The total number of bytes that is expected to be transferred.
     */
    quint32 total() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HTransferProgressInfo
 */
H_UPNP_AV_EXPORT bool operator==(
    const HTransferProgressInfo& obj1, const HTransferProgressInfo& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HTransferProgressInfo
 */
inline bool operator!=(
    const HTransferProgressInfo& obj1, const HTransferProgressInfo& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

#endif /* HTRANSFERPROGRESSINFO_H_ */
