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

#ifndef HCONTENTDIRECTORY_INFO_H_
#define HCONTENTDIRECTORY_INFO_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpCore/HUpnp>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This class provides general information of a UPnP A/V ContentDirectory and
 * information related to the operations of ContentDirectory
 * implementations provided by HUPnPAv.
 *
 * \headerfile hcontentdirectory_info.h HContentDirectoryInfo
 *
 * \ingroup hupnp_av_cds
 *
 * \remarks This class is thread-safe.
 */
class H_UPNP_AV_EXPORT HContentDirectoryInfo
{
H_FORCE_SINGLETON(HContentDirectoryInfo)

public:

    /*!
     * \brief This enumeration specifies the available browse options for a browse
     * operation.
     *
     * \sa browseFlagToString(), browseFlagFromString()
     */
    enum BrowseFlag
    {
        /*!
         * This value is used in error scenarios.
         */
        Undefined,

        /*!
         * The browse operation should return the metadata of the object specified.
         */
        BrowseMetadata,

        /*!
         * The browse operation should return metadata of the children of the
         * object specified.
         */
        BrowseDirectChildren
    };

    /*!
     * \brief Returns a string representation of the specified \a flag value.
     *
     * \param flag specifies the browse option to be converted.
     *
     * \return a string representation of the specified BrowseFlag.
     */
    static QString browseFlagToString(BrowseFlag flag);

    /*!
     * \brief Returns a BrowseFlag value corresponding to the specified string, if any.
     *
     * \param arg specifies the string to be converted.
     *
     * \return The corresponding BrowseFlag value.
     */
    static BrowseFlag browseFlagFromString(const QString& arg);

    /*!
     * \brief Returns the type of the latest ContentDirectory specification the
     * HUPnPAv implementations support.
     *
     * \return The type of the latest ContentDirectory specification the
     * HUPnPAv implementations support.
     */
    static const HResourceType& supportedServiceType();

    /*!
     * \brief Returns information about the actions specified up until the
     * ContentDirectory specification supported by HUPnPAv implementations.
     *
     * \sa supportedServiceType(), stateVariablesSetupData()
     */
    static HActionsSetupData actionsSetupData();

    /*!
     * \brief Returns information about the state variables specified up until the
     * ContentDirectory specification supported by HUPnPAv implementations.
     *
     * \sa supportedServiceType(), actionsSetupData()
     */
    static HStateVariablesSetupData stateVariablesSetupData();

    /*!
     * \brief This enumeration describes the error codes that have been
     * defined by the UPnP forum to the ContentDirectory:3 service.
     *
     * \note the descriptions are taken from the ContentDirectory:3 specification.
     */
    enum ErrorCode
    {
        /*!
         * An operation failed due to an invalid object ID.
         */
        InvalidObjectId = 701,

        /*!
         * An operation failed, because one or more of the specified current tag
         * values do not match with the current state of the service.
         */
        InvalidCurrentTagValue = 702,

        /*!
         * An operation failed, because one or more of the specified new tag
         * values has an unsupported or malformed property value.
         */
        InvalidNewTagValue = 703,

        /*!
         * The action failed because the request included an implicit request
         * to delete a required tag.
         */
        RequiredTag = 704,

        /*!
         * An operation failed, because the request would have modified a
         * read-only tag.
         */
        ReadOnlyTag = 705,

        /*!
         * The action failed because two separate references to the number of
         * tag/value pairs (including empty placeholders) do not match.
         */
        ParameterMismatch = 706,

        /*!
         * The specified search criteria was either unsupported or malformed.
         */
        InvalidSearchCriteria = 708,

        /*!
         The specified sort criteria was either unsupported or malformed.
         */
        InvalidSortCriteria = 709,

        /*!
         * An operation failed due to an invalid container ID.
         */
        NoSuchContainer = 710,

        /*!
         * A write operation failed, because the specified CDS object is
         * \e restricted.
         */
        RestrictedObject = 711,

        /*!
         * An operation failed due to invalid metadata.
         */
        BadMetadata = 712,

        /*!
         * A write operation failed, because the specified CDS object is a child
         * of a restricted CDS container.
         */
        RestrictedParentObject = 713,

        /*!
         * An operation failed, because the resource referenced by the CDS object
         * could not be identified.
         */
        NoSuchResource = 714,

        /*!
         * An operation failed, because the resource referenced by the CDS object
         * could not be accessed.
         */
        SourceResourceAccessDenied = 715,

        /*!
         * The action failed because a specified resource refuses to perform
         * another file transfer.
         */
        TransferBusy = 716,

        /*!
         * The action failed because a specified file transfer task does not
         * exist.
         */
        NoSuchFileTransfer = 717,

        /*!
         * The action failed because a specified destination resource cannot
         * be identified.
         */
        NoSuchDestinationResource = 718,

        /*!
         * The action failed because a specified destination resource is busy.
         */
        DestinationResourceAccessDenied = 719,

        /*!
         * An operation failed, because the service was unable to process
         * the request within the time restraints.
         */
        CannotProcessTheRequest = 720,

        /*!
         * The action failed because the @restricted property of the source
         * parent container of the object to move is set to true.
         */
        RestrictedSourceParentObject = 721,

        /*!
         * The action failed because the class of the object to move is not
         * compatible with the upnp:createClass property of the destination
         * parent container.
         */
        IncompatibleParentClass = 722,

        /*!
         * The action failed because it would create an illegal configuration.
         */
        IllegalDestination = 723,

        /*!
         * The request failed because the value specified in the CDSView
         * argument is not supported or is invalid.
         */
        UnsupportedOrInvalidCDSView = 724,

        /*!
         * The request failed because the XQuery XML document specified
         * in the QueryRequest argument is invalid.
         */
        InvalidQueryRequest = 725,

        /*!
         * The request failed because the XQuery XML document specified in
         * the QueryRequest argument contains unsupported instructions for
         * this particular implementation.
         */
        UnsupportedQueryRequestInstruction = 726
    };
};

}
}
}

#endif /* HCONTENTDIRECTORY_INFO_H_ */
