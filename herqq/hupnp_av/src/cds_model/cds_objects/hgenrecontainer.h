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

#ifndef HGENRECONTAINER_H_
#define HGENRECONTAINER_H_

#include <HUpnpAv/HContainer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HGenreContainerPrivate;

/*!
 * \brief This class represents an unordered collection of objects that all
 * belong to the same genre.
 *
 * The class identifier specified by the AV Working Committee is
 * \c object.container.genre.
 *
 * \headerfile hgenrecontainer.h HGenreContainer
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HGenreContainer :
    public HContainer
{
Q_OBJECT
H_DISABLE_COPY(HGenreContainer)
H_DECLARE_PRIVATE(HGenreContainer)

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
    HGenreContainer(const QString& clazz = sClass(), CdsType cdsType = sType());
    HGenreContainer(HGenreContainerPrivate&);

    // Documented in HClonable
    virtual HGenreContainer* newInstance() const;

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
    HGenreContainer(
        const QString& title,
        const QString& parentId,
        const QString& id = QString());

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HGenreContainer();

    /*!
     * \brief Returns the genres to which this item belongs.
     *
     * upnp:genre, ContentDirectory:3, Appendix B.4.1.
     *
     * \return The genres to which this item belongs.
     *
     * \sa setGenres()
     */
    QList<HGenre> genres() const;

    /*!
     * \brief Returns a few lines of description of the content item.
     *
     * upnp:longDescripion, ContentDirectory:3, Appendix B.7.2.
     *
     * \return a few lines of description of the content item.
     *
     * \sa setLongDescription()
     */
    QString longDescription() const;

    /*!
     * \brief Returns a brief description of the content item.
     *
     * dc:description, ContentDirectory:3, Appendix B.7.1.
     *
     * \return a brief description of the content item.
     *
     * \sa setDescription()
     */
    QString description() const;

    /*!
     * \brief Specifies the genres to which this item belongs.
     *
     * \param arg specifies the genres to which this item belongs.
     *
     * \sa genres()
     */
    void setGenres(const QList<HGenre>& arg);

    /*!
     * \brief Specifies a few lines of description of the content item.
     *
     * \param arg specifies a few lines of description of the content item.
     *
     * \sa longDescription()
     */
    void setLongDescription(const QString& arg);

     /*!
     * \brief Specifies a brief description of the content item.
     *
     * \param arg specifies a brief description of the content item.
     *
     * \sa description()
     */
    void setDescription(const QString& arg);

    /*!
     * \brief Returns the CdsType value of this class.
     *
     * \return The CdsType value of this class.
     */
    inline static CdsType sType() { return Genre; }

    /*!
     * \brief Returns the class identifier specified by the AV Working Committee.
     *
     * \return The class identifier specified by the AV Working Committee.
     */
    inline static QString sClass() { return "object.container.genre"; }

    /*!
     * Creates a new instance with no title or parentID.
     *
     * \return a pointer to the newly created instance.
     *
     * \remarks the ownership of the object is transferred to the caller. Make sure
     * to delete the object.
     */
    inline static HGenreContainer* create() { return new HGenreContainer(); }
};

}
}
}

#endif /* HGENRECONTAINER_H_ */
