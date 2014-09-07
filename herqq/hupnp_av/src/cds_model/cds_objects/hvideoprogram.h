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

#ifndef HVIDEOPROGRAM_H_
#define HVIDEOPROGRAM_H_

#include <HUpnpAv/HEpgItem>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HVideoProgramPrivate;

/*!
 * \brief This class represents a video program such as a single TV show or a
 * series of programs.
 *
 * The class identifier specified by the AV Working Committee is
 * \c object.item.epgItem.videoProgram.
 *
 * \headerfile hvideoprogram.h HVideoProgram
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HVideoProgram :
    public HEpgItem
{
Q_OBJECT
H_DISABLE_COPY(HVideoProgram)
H_DECLARE_PRIVATE(HVideoProgram)

protected:

    /*!
     * Constructs a new instance.
     *
     * \param clazz specifies the class identifier.
     *
     * \param cdsType specifies CDS type.
     */
    HVideoProgram(const QString& clazz = sClass(), CdsType cdsType = sType());
    HVideoProgram(HVideoProgramPrivate&);

    // Documented in HClonable
    virtual HVideoProgram* newInstance() const;

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
    HVideoProgram(
        const QString& title,
        const QString& parentId,
        const QString& id = QString());

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HVideoProgram();

    /*!
     * \brief Returns the CdsType value of this class.
     *
     * \return The CdsType value of this class.
     */
    inline static CdsType sType() { return VideoProgram; }

    /*!
     * \brief Returns the class identifier specified by the AV Working Committee.
     *
     * \return The class identifier specified by the AV Working Committee.
     */
    inline static QString sClass() { return "object.item.epgItem.videoProgram"; }

    /*!
     * Creates a new instance with no title or parentID.
     *
     * \return a pointer to the newly created instance.
     *
     * \remarks the ownership of the object is transferred to the caller. Make sure
     * to delete the object.
     */
    inline static HVideoProgram* create() { return new HVideoProgram(); }
};

}
}
}

#endif /* HVIDEOPROGRAM_H_ */
