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

#ifndef HRATING_H_
#define HRATING_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HRatingPrivate;

/*!
 * \brief This class is used to represent rating types and values as defined in the
 * UPnP A/V ScheduledRecording:2 specification.
 *
 * \headerfile hrating.h HRating
 *
 * \ingroup hupnp_av_common
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HRating
{
private:

    QSharedDataPointer<HRatingPrivate> h_ptr;

public:

    /*!
     * \brief This enumeration specifies the different rating types available.
     */
    enum Type
    {
        /*!
         * The rating type is undefined.
         */
        UndefinedType = 0,

        /*!
         * The Motion Picture Association of America.
         */
        MPAA,

        /*!
         * The Recording Industry Association of America.
         */
        RIAA,

        /*!
         * The Entertainment Software Rating Board.
         */
        ESRB,

        /*!
         * TV Parental Guidelines.
         */
        TvGuidelines,

        /*!
         * The rating type is vendor-defined.
         */
        VendorDefined
    };

    /*!
     * \brief Returns a string representation of the HRating::Type value specified.
     *
     * \return a string representation of the HRating::Type value specified.
     */
    static QString toString(Type type);

    /*!
     * \brief This enumeration defines the rating values MPAA has defined.
     */
    enum MpaaValues
    {
        /*!
         * No MPAA value was defined.
         *
         * This value is used in error scenarios.
         */
        MPAA_Undefined = 0,

        /*!
         * "G"
         */
        MPAA_GeneralAudiences,

        /*!
         * "PG"
         */
        MPAA_ParentalGuidanceSuggested,

        /*!
         * "PG-13"
         */
        MPAA_ParentsStronglyCautioned,

        /*!
         * "R"
         */
        MPAA_Restricted,

        /*!
         * "NC-17"
         */
        MPAA_NoOneSeventeenAndUnderAdmitted,

        /*!
         * "NR"
         */
        MPAA_NotRatedYet
    };

    /*!
     * \brief Returns a string representation of the HRating::MpaaValues value specified.
     *
     * \return a string representation of the HRating::MpaaValues value specified.
     */
    static QString toString(MpaaValues type);

    /*!
     * \brief This enumeration defines the rating values RIAA has defined.
     */
    enum RiaaValues
    {
        /*!
         * No RIAA value was defined.
         *
         * This value is used in error scenarios.
         */
        RIAA_Undefined = 0,

        /*!
         * This value equals to an empty string.
         */
        RIAA_NonExplicitContent,

        /*!
         * "PA-EC"
         */
        RIAA_ExplicitContent
    };

    /*!
     * \brief Returns a string representation of the HRating::RiaaValues value specified.
     *
     * \return a string representation of the HRating::RiaaValues value specified.
     */
    static QString toString(RiaaValues type);

    /*!
     * \brief This enumeration defines the rating values ESRB has defined.
     */
    enum EsrbValues
    {
        /*!
         * No ESRB value was defined.
         *
         * This value is used in error scenarios.
         */
        ESRB_Undefined = 0,

        /*!
         * "EC"
         */
        ESRB_EarlyChildhood,

        /*!
         * "E"
         */
        ESRB_Everyone,

        /*!
         * "E10+"
         */
        ESRB_EveryoneTenAndOlder,

        /*!
         * "T"
         */
        ESRB_Teen,

        /*!
         * "M"
         */
        ESRB_Mature,

        /*!
         * "AO"
         */
        ESRB_AdultsOnly,

        /*!
         * "RP"
         */
        ESRB_RatingPending
    };

    /*!
     * \brief Returns a string representation of the HRating::EsrbValues value specified.
     *
     * \return a string representation of the HRating::EsrbValues value specified.
     */
    static QString toString(EsrbValues type);

    /*!
     * \brief This enumeration defines the rating values TV Guidelines has defined.
     */
    enum TvGuidelinesValues
    {
        /*!
         * No TV Guidelines value was defined.
         *
         * This value is used in error scenarios.
         */
        TVG_Undefined = 0,

        /*!
         * "TV-Y"
         */
        TVG_AllChildren,

        /*!
         * "TV-Y7"
         */
        TVG_OlderChildren,

        /*!
         * "TV-Y7FV"
         */
        TVG_OlderChildren_FantasyViolence,

        /*!
         * "TV-G"
         */
        TVG_GeneralAudience,

        /*!
         * "TV-PG"
         */
        TVG_ParentalGuidanceSuggested,

        /*!
         * "TV-14"
         */
        TVG_ParentsStronglyCautioned,

        /*!
         * "TV-MA"
         */
        TVG_MatureAudienceOnly
    };

    /*!
     * \brief Returns a string representation of the HRating::TvGuidelinesValues value specified.
     *
     * \return a string representation of the HRating::TvGuidelinesValues value specified.
     */
    static QString toString(TvGuidelinesValues type);

    /*!
     * Creates a new, invalid instance.
     *
     * sa isValid()
     */
    HRating();

    /*!
     * \brief Creates a new instance.
     *
     * \param value specifies the rating value.
     *
     * \param type specifies the type of rating. This is optional and if it isn't
     * provided, the value is matched against all the rating values defined by
     * MPAA, RIAA, ESRB and Tv Guidelines. If a match is found the type is
     * set accordingly. If a match is not found and the specified \a value is
     * not empty, the type is set to HRating::VendorDefined.
     *
     * \sa isValid()
     */
    HRating(const QString& value, const QString& type = QString());

    /*!
     * \brief Creates a new instance.
     *
     * \param arg specifies the rating as defined by MPAA.
     *
     * \sa isValid()
     */
    HRating(MpaaValues arg);

    /*!
     * \brief Creates a new instance.
     *
     * \param arg specifies the rating as defined by RIAA.
     *
     * \sa isValid()
     */
    HRating(RiaaValues arg);

    /*!
     * \brief Creates a new instance.
     *
     * \param arg specifies the rating as defined by ESRB.
     *
     * \sa isValid()
     */
    HRating(EsrbValues arg);

    /*!
     * \brief Creates a new instance.
     *
     * \param arg specifies the rating as defined by TV-Guidelines.
     *
     * \sa isValid()
     */
    HRating(TvGuidelinesValues arg);

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HRating(const HRating&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HRating& operator=(const HRating&);

    /*!
     * \brief Destroys the instance.
     *
     * sa isValid()
     */
    ~HRating();

    /*!
     * \brief Returns the rating type this instance uses.
     *
     * \return The rating type this instance uses.
     */
    Type type() const;

    /*!
     * \brief Returns the rating type this instance uses as a string.
     *
     * \return The rating type this instance uses as a string.
     */
    QString typeAsString() const;

    /*!
     * \brief Returns the rating value this instance uses, as defined by the MPAA.
     *
     * \return The rating value this instance uses, as defined by the MPAA.
     */
    MpaaValues mpaaValue() const;

    /*!
     * \brief Returns the rating value this instance uses, as defined by the RIAA.
     *
     * \return The rating value this instance uses, as defined by the RIAA.
     */
    RiaaValues riaaValue() const;

    /*!
     * \brief Returns the rating value this instance uses, as defined by the ESRB.
     *
     * \return The rating value this instance uses, as defined by the ESRB.
     */
    EsrbValues esrbValue() const;

    /*!
     * \brief Returns the rating value this instance uses, as defined by the TV-Guidelines.
     *
     * \return The rating value this instance uses, as defined by the TV-Guidelines.
     */
    TvGuidelinesValues tvGuidelinesValue() const;

    /*!
     * \brief Returns the rating value.
     *
     * \return The rating value.
     */
    QString value() const;

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true if the object is valid, i.e. both the value() and type()
     * are appropriately defined.
     */
    bool isValid() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HRating
 */
H_UPNP_AV_EXPORT bool operator==(const HRating& obj1, const HRating& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HRating
 */
inline bool operator!=(const HRating& obj1, const HRating& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

Q_DECLARE_METATYPE(Herqq::Upnp::Av::HRating)

#endif /* HRATING_H_ */
