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

#ifndef HGENRE_H_
#define HGENRE_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QString>
#include <QtCore/QMetaType>
#include <QtCore/QStringList>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \brief This class contains information of a particular \e genre.
 *
 * \headerfile hgenre.h HGenre
 *
 * \ingroup hupnp_av_cds_common
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HGenre
{
private:

    QString m_name;
    QString m_id;
    QStringList m_extended;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HGenre();

    /*!
     * Creates a new, invalid instance.
     *
     * \param name specifies the name of the genre.
     *
     * \sa isValid()
     */
    HGenre(const QString& name);

    /*!
     * Creates a new, invalid instance.
     *
     * \param name specifies the name of the genre.
     *
     * \param id specifies the scheme, which defines the set of available
     * genre names.
     *
     * \param extended specifies increasingly precise sub-genres, where the first
     * entry specifies the most general genre. Thus, the first entry must equal
     * to \a name, or the list has to be empty. In addition, the list cannot
     * contain empty or whitespace-only entries. In such a case the entire list
     * is ignored.
     *
     * \sa isValid()
     */
    HGenre(const QString& name, const QString& id, const QStringList& extended);

    /*!
     * \brief Destroys the instance.
     */
    ~HGenre();

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true if the object is valid, i.e. the name() is defined.
     */
    bool isValid() const;

    /*!
     * \brief Returns the name of the genre.
     *
     * \return The name of the genre.
     */
    inline QString name() const { return m_name; }

    /*!
     * \brief Returns the genre scheme, which defines the set of available genre names.
     *
     * \return The genre scheme, which defines the set of available genre names.
     */
    inline QString id() const { return m_id; }

    /*!
     * \brief Returns the genre name and its sub-genres, if any.
     *
     * \return The genre name and its sub-genres, if any.
     */
    inline QStringList extended() const { return m_extended; }
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HGenre
 */
H_UPNP_AV_EXPORT bool operator==(const HGenre& obj1, const HGenre& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HGenre
 */
inline bool operator!=(const HGenre& obj1, const HGenre& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

Q_DECLARE_METATYPE(Herqq::Upnp::Av::HGenre)

#endif /* HGENRE_H_ */
