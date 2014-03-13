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

#ifndef HSTATEVARIABLECOLLECTION_H_
#define HSTATEVARIABLECOLLECTION_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpAv/HChannel>

#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * Encapsulates the state of a single state variable.
 *
 * \headerfile hstatevariablecollection.h HStateVariableData
 *
 * \ingroup hupnp_av_cds_common
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HStateVariableData
{
private:

    QString m_svName, m_svValue;
    HChannel m_channel;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HStateVariableData();

    /*!
     * Creates a new, invalid instance.
     *
     * \param name specifies the name of the state variable.
     *
     * \param value specifies the value of the state variable.
     *
     * \sa isValid()
     */
    HStateVariableData(
        const QString& name, const QString& value, const HChannel& = HChannel());

   /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true if the object is valid, i.e. name() is defined.
     */
    bool isValid() const;

    /*!
     * \brief Returns the name of the state variable.
     *
     * \return The name of the state variable.
     */
    inline QString name() const
    {
        return m_svName;
    }

    /*!
     * \brief Returns the value of the state variable.
     *
     * \return The value of the state variable.
     */
    inline QString value() const
    {
        return m_svValue;
    }

    /*!
     * \brief Returns the associated channel of audio stream, if any.
     *
     * \return The associated channel of audio stream, if any.
     */
    inline const HChannel& channel() const
    {
        return m_channel;
    }

    /*!
     * \brief Specifies the name of the state variable.
     *
     * \param arg specifies the name of the state variable.
     */
    void setName(const QString& arg);

    /*!
     * \brief Specifies the value of the state variable.
     *
     * \param arg specifies the value of the state variable.
     */
    void setValue(const QString& arg);

    /*!
     * \brief Specifies the associated channel of audio stream.
     *
     * \param arg specifies the associated channel of audio stream.
     */
    void setChannel(const HChannel& arg);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HStateVariableData
 */
H_UPNP_AV_EXPORT bool operator==(const HStateVariableData& obj1, const HStateVariableData& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HStateVariableData
 */
inline bool operator!=(const HStateVariableData& obj1, const HStateVariableData& obj2)
{
    return !(obj1 == obj2);
}

class HStateVariableCollectionPrivate;

/*!
 * \brief This class is used to encapsulate the state of a set of state variables.
 *
 * \headerfile hstatevariablecollection.h HStateVariableCollection
 *
 * \ingroup hupnp_av_cds_common
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HStateVariableCollection
{
private:

    QSharedDataPointer<HStateVariableCollectionPrivate> h_ptr;

public:

    /*!
     * \brief This enumeration specifies whether a RenderingControl service instance
     * is pre-mix or post-mix.
     *
     * \see ContentDirectory:3, Appendix B.13.5.2.
     */
    enum RcsInstanceType
    {
        /*!
         * The RCS instance type is not defined.
         */
        Undefined,

        /*!
         * Pre-mix.
         */
        PreMix,

        /*!
         * Post-mix.
         */
        PostMix
    };

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HStateVariableCollection();

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HStateVariableCollection(
        const QString& serviceName, RcsInstanceType rcsInstanceType = Undefined);

    /*!
     * \brief Destroys the instance.
     */
    ~HStateVariableCollection();

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HStateVariableCollection(const HStateVariableCollection&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HStateVariableCollection& operator=(const HStateVariableCollection&);

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true if the object is valid, i.e. serviceName() is defined.
     */
    bool isValid() const;

    /*!
     * Converts the specified RcsInstanceType value to string.
     *
     * \param type specifies the RcsInstanceType value to be converted to string.
     *
     * \return a string representation of the specified RcsInstanceType value.
     */
    static QString toString(RcsInstanceType type);

    /*!
     * \brief Returns a RcsInstanceType value corresponding to the specified string, if any.
     *
     * \param type specifies the RcsInstanceType as string.
     *
     * \return a RcsInstanceType value corresponding to the specified string, if any.
     */
    static RcsInstanceType fromString(const QString& type);

    /*!
     * \brief Returns the name of the service from which the state variables were retrieved.
     *
     * \return The name of the service from which the state variables were retrieved.
     */
    QString serviceName() const;

    /*!
     * \brief Indicates whether a RenderingControl service instance
     * is pre-mix or post-mix.
     *
     * \return a value indicating whether a RenderingControl service instance
     * is pre-mix or post-mix.
     */
    RcsInstanceType rcsInstanceType() const;

    /*!
     * \brief Returns the state variable data.
     *
     * \return The state variable data.
     *
     * \sa setStateVariables()
     */
    QList<HStateVariableData> stateVariables() const;

    /*!
     * \brief Specifies whether a RenderingControl service instance
     * is pre-mix or post-mix.
     *
     * \param arg specifies whether a RenderingControl service instance
     * is pre-mix or post-mix.
     *
     * \sa rcsInstanceType()
     */
    void setRcsInstanceType(RcsInstanceType arg);

    /*!
     * \brief Specifies the state variable data.
     *
     * \param arg specifies the state variable data.
     *
     * \sa stateVariables()
     */
    void setStateVariables(const QList<HStateVariableData>& arg);

    /*!
     * Adds a new state variable data instance to the collection.
     *
     * \param arg specifies the state variable data instance to add.
     *
     * \sa setStateVariables()
     *
     * \remarks only valid HStateVariableData instances are added.
     */
    void add(const HStateVariableData& arg);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HStateVariableCollection
 */
H_UPNP_AV_EXPORT bool operator==(const HStateVariableCollection& obj1, const HStateVariableCollection& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HStateVariableCollection
 */
inline bool operator!=(const HStateVariableCollection& obj1, const HStateVariableCollection& obj2)
{
    return !(obj1 == obj2);
}

}
}
}

Q_DECLARE_METATYPE(Herqq::Upnp::Av::HStateVariableCollection)

#endif /* HSTATEVARIABLECOLLECTION_H_ */
