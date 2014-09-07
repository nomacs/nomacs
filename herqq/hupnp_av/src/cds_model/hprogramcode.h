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

#ifndef HPROGRAMCODE_H_
#define HPROGRAMCODE_H_

#include <HUpnpAv/HUpnpAv>

#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HProgramCodePrivate;

/*!
 * \brief This class is used to represent the concept of a \e program \e code defined
 * in the ScheduledRecording:2 specification.
 *
 * \headerfile hprogramcode.h HProgramCode
 *
 * \ingroup hupnp_av_cds_common
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HProgramCode
{
private:

    QSharedDataPointer<HProgramCodePrivate> h_ptr;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HProgramCode();

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HProgramCode(const QString& value, const QString& type);

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HProgramCode(const HProgramCode&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HProgramCode& operator=(const HProgramCode&);

    /*!
     * \brief Destroys the instance.
     */
    ~HProgramCode();

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true if the object is valid, i.e. value() and type() are both
     * defined.
     */
    bool isValid() const;

    /*!
     * \brief Returns the value of the program code.
     *
     * \return The value of the program code.
     */
    QString value() const;

    /*!
     * \brief Returns the type of the program code.
     *
     * \return The type of the program code.
     */
    QString type() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HProgramCode
 */
H_UPNP_AV_EXPORT bool operator==(const HProgramCode& obj1, const HProgramCode& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HProgramCode
 */
inline bool operator!=(const HProgramCode& obj1, const HProgramCode& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

Q_DECLARE_METATYPE(Herqq::Upnp::Av::HProgramCode)

#endif /* HPROGRAMCODE_H_ */
