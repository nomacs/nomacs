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

#ifndef HAUDIOPROGRAM_H_
#define HAUDIOPROGRAM_H_

#include <HUpnpAv/HEpgItem>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HAudioProgramPrivate;

/*!
 * \brief This class represents a broadcast audio program such as a radio show or a
 * series of programs.
 *
 * The class identifier specified by the AV Working Committee is
 * \c object.item.epgItem.audioProgram.
 *
 * \headerfile haudioprogram.h HAudioProgram
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HAudioProgram :
    public HEpgItem
{
Q_OBJECT
H_DISABLE_COPY(HAudioProgram)
H_DECLARE_PRIVATE(HAudioProgram)

protected:

    /*!
     * Constructs a new instance.
     *
     * \param clazz specifies the UPnP class of the object. This cannot be empty.
     *
     * \param cdsType specifies the CDS type of the object. This cannot be
     * HObject::UndefinedCdsType.
     *
     * \sa isInitialized()
     */
    HAudioProgram(const QString& clazz = sClass(), CdsType cdsType = sType());
    HAudioProgram(HAudioProgramPrivate&);

    // Documented in HClonable
    virtual HAudioProgram* newInstance() const;

public:

    /*!
     * \brief Creates a new instance.
     *
     * \param title specifies the title of the object.
     *
     * \param parentId specifies the ID of the object that contains this
     * object. If the object has no parent, this has to be left empty.
     *
     * \param id specifies the ID of this object. If this is not specified,
     * a unique identifier within the running process is created for the object.
     *
     * \sa isValid()
     */
    HAudioProgram(
        const QString& title,
        const QString& parentId,
        const QString& id = QString());

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HAudioProgram();

    /*!
     * \brief Returns the call sign of the source radio station.
     *
     *  upnp:radioCallSign, ContentDirectory, Appendix B.10.1
     *
     * \return The call sign of the source radio station.
     *
     * \sa setRadioCallSign()
     */
    QString radioCallSign() const;

    /*!
     * \brief Returns some identification information of the radio station.
     *
     * upnp:radioStationID, ContentDirectory, Appendix B.10.2.
     *
     * \return some identification information of the radio station.
     *
     * \sa setRadioStationId()
     */
    QString radioStationId() const;

    /*!
     * \brief Returns the radio band type of the radio station.
     *
     * upnp:radioBand, ContentDirectory, Appendix B.10.3.
     *
     * \return The radio band type of the radio station.
     *
     * \sa setRadioBand()
     */
    HRadioBand radioBand() const;

    /*!
     * \brief Specifies the call sign of the source radio station.
     *
     * \param arg specifies the call sign of the source radio station.
     *
     * \sa radioCallSign()
     */
    void setRadioCallSign(const QString& arg);

    /*!
     * \brief Specifies some identification information of the radio station.
     *
     * \param arg specifies some identification information of the radio station.
     *
     * \sa radioStationId()
     */
    void setRadioStationId(const QString& arg);

    /*!
     * \brief Specifies the radio band type of the radio station.
     *
     * \param arg specifies the radio band type of the radio station.
     *
     * \sa radioBand()
     */
    void setRadioBand(const HRadioBand& arg);

    /*!
     * \brief Returns the CdsType value of this class.
     *
     * \return The CdsType value of this class.
     */
    inline static CdsType sType() { return AudioProgram; }

    /*!
     * \brief Returns the class identifier specified by the AV Working Committee.
     *
     * \return The class identifier specified by the AV Working Committee.
     */
    inline static QString sClass() { return "object.item.epgItem.audioProgram"; }

    /*!
     * Creates a new instance with no title or parentID.
     *
     * \return a pointer to the newly created instance.
     *
     * \remarks the ownership of the object is transferred to the caller. Make sure
     * to delete the object.
     */
    inline static HAudioProgram* create() { return new HAudioProgram(); }
};

}
}
}

#endif /* HAUDIOPROGRAM_H_ */
