/*
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP (HUPnP) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HSTATEVARIABLEINFO_H_
#define HSTATEVARIABLEINFO_H_

#include <HUpnpCore/HUpnp>
#include <HUpnpCore/HUpnpDataTypes>

#include <QtCore/QSharedDataPointer>

class QString;
class QVariant;

namespace Herqq
{

namespace Upnp
{

class HStateVariableInfoPrivate;

/*!
 * \brief This class is used to contain information of a UPnP state variable
 * found in a UPnP service description document.
 *
 * UPnP service description documents specify the actions and state variables
 * of the service. An instance of this class contain the information of a
 * state variable found in a service description document, such as the
 * name() and the dataType().
 *
 * In addition to the information found in the service description document,
 * the UPnP service containing the state variable that is depicted by the
 * HStateVariableInfo object may have specified additional information
 * about the state variable:
 *
 * - inclusionRequirement() details whether the state variable is considered as
 * mandatory or optional.
 * - maxEventRate() specifies the maximum rate at which an evented
 * state variable may send events.
 *
 * Further, the class contains a few helper methods:
 * - isConstrained() indicates if the state variable is restricted either by
 * a value range or a value list.
 * - isValidValue() checks if a specified \c QVariant contains a value that could be
 * inserted into the state variable taking into consideration the data types
 * of the state variable and the specified value as well as any possible
 * constraint set to the state variable.
 *
 * \headerfile hstatevariableinfo.h HStateVariableInfo
 *
 * \ingroup hupnp_common
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HDeviceInfo, HServiceInfo and HActionInfo.
 */
class H_UPNP_CORE_EXPORT HStateVariableInfo
{
friend H_UPNP_CORE_EXPORT bool operator==(
    const HStateVariableInfo&, const HStateVariableInfo&);

public:

    /*!
     * \brief Specifies different types of eventing.
     *
     * \sa hupnp_devicehosting
     */
    enum EventingType
    {
        /*!
         * The state variable is not evented and it will never emit
         * valueChanged() signal.
         */
        NoEvents = 0,

        /*!
         * The state variable is evented, valueChanged() signal is emitted upon
         * value change and the HUPnP will propagate events over network
         * to registered listeners through unicast only.
         */
        UnicastOnly = 1,

        /*!
         * The state variable is evented, valueChanged() signal is emitted upon
         * value change and the HUPnP will propagate events over network
         * using uni- and multicast.
         */
        UnicastAndMulticast = 2
    };

private:

    QSharedDataPointer<HStateVariableInfoPrivate> h_ptr;

public:

    /*!
     * \brief Creates a new, empty instance.
     *
     * \sa isValid()
     */
    HStateVariableInfo();

    /*!
     * \brief Creates a new instance.
     *
     * \param name specifies the name of the state variable.
     *
     * \param dataType specifies the UPnP data type of the state variable.
     *
     * \param incReq specifies whether the service is required or optional.
     * This parameter is optional.
     *
     * \param err specifies a pointer to a \c QString that will contain
     * an error description in case the construction failed. This is optional.
     *
     * \sa isValid()
     */
    HStateVariableInfo(
        const QString& name,
        HUpnpDataTypes::DataType dataType,
        HInclusionRequirement incReq,
        QString* err = 0);

    /*!
     * \brief Creates a new instance.
     *
     * \param name specifies the name of the state variable.
     *
     * \param dataType specifies the UPnP data type of the state variable.
     *
     * \param eventingType specifies the type of eventing used with the
     * state variable. This is optional.
     *
     * \param incReq specifies whether the service is required or optional.
     * This parameter is optional.
     *
     * \param err specifies a pointer to a \c QString that will contain
     * an error description in case the construction failed. This is optional.
     *
     * \sa isValid()
     */
    HStateVariableInfo(
        const QString& name,
        HUpnpDataTypes::DataType dataType,
        EventingType eventingType = NoEvents,
        HInclusionRequirement incReq = InclusionMandatory,
        QString* err = 0);

    /*!
     * \brief Creates a new instance.
     *
     * \param name specifies the name of the state variable.
     *
     * \param dataType specifies the UPnP data type of the state variable.
     *
     * \param defaultValue specifies the default value.
     *
     * \param eventingType specifies the type of eventing used with the
     * state variable. This is optional.
     *
     * \param incReq specifies whether the service is required or optional.
     * This parameter is optional.
     *
     * \param err specifies a pointer to a \c QString that will contain
     * an error description in case the construction failed. This is optional.
     *
     * \sa isValid()
     */
    HStateVariableInfo(
        const QString& name,
        HUpnpDataTypes::DataType dataType,
        const QVariant& defaultValue,
        EventingType eventingType = NoEvents,
        HInclusionRequirement incReq = InclusionMandatory,
        QString* err = 0);

    /*!
     * Creates a new instance with the data type set to \c HUpnpDataTypes::string.
     *
     * \param name specifies the name of the state variable.
     *
     * \param defaultValue specifies the default value.
     *
     * \param allowedValueList specifies the values the state variable
     * accepts.
     *
     * \param eventingType specifies the type of eventing used with the
     * state variable. This is optional.
     *
     * \param incReq specifies whether the service is required or optional.
     * This parameter is optional.
     *
     * \param err specifies a pointer to a \c QString that will contain
     * an error description in case the construction failed. This is optional.
     *
     * \sa isValid()
     */
    HStateVariableInfo(
        const QString& name,
        const QVariant& defaultValue,
        const QStringList& allowedValueList,
        EventingType eventingType = NoEvents,
        HInclusionRequirement incReq = InclusionMandatory,
        QString* err = 0);

    /*!
     * \param name specifies the name of the state variable.
     *
     * \param dataType specifies the UPnP data type of the state variable.
     *
     * \param defaultValue specifies the default value.
     *
     * \param minimumValue specifies the inclusive lower bound of an
     * acceptable value. This cannot be larger than the \c maximumValue.
     *
     * \param maximumValue specifies the inclusive upper bound of an
     * acceptable value. This cannot be smaller than the \c minimumValue.
     *
     * \param stepValue specifies the step value. This value cannot be
     * larger than the subtraction of the maximum and minimum values.
     *
     * \param eventingType specifies the type of eventing used with the
     * state variable. This is optional.
     *
     * \param incReq specifies whether the service is required or optional.
     * This parameter is optional.
     *
     * \param err specifies a pointer to a \c QString that will contain
     * an error description in case the construction failed. This is optional.
     *
     */
    HStateVariableInfo(
        const QString& name,
        HUpnpDataTypes::DataType dataType,
        const QVariant& defaultValue,
        const QVariant& minimumValue,
        const QVariant& maximumValue,
        const QVariant& stepValue,
        EventingType eventingType = NoEvents,
        HInclusionRequirement incReq = InclusionMandatory,
        QString* err = 0);

    /*!
     * \brief Copy constructor.
     *
     * Creates a new instance identical to the \c other object.
     */
    HStateVariableInfo(const HStateVariableInfo&);

    /*!
     * \brief Assignment operator.
     *
     * Assigns the contents of the \c other to this.
     */
    HStateVariableInfo& operator=(const HStateVariableInfo&);

    /*!
     * \brief Destroys the instance.
     */
    ~HStateVariableInfo();

    /*!
     * \brief Returns the UPnP service version in which the state variable
     * was first specified.
     *
     * \return The UPnP service version in which the state variable
     * was first specified or \c -1, if the version is not defined.
     *
     * \remarks It is perfectly normal that the version information is not
     * defined.
     *
     * \sa setVersion()
     */
    qint32 version() const;

    /*!
     * \brief Specifies the UPnP service version in which the state variable
     * was first specified.
     *
     * \param version specifies the UPnP service version in which the
     * state variable was first specified. If a value smaller than \c -1 is
     * given, the version value will be set to \c -1, which means that the
     * version() is not defined.
     *
     * \sa version()
     */
    void setVersion(qint32 version);

    /*!
     * \brief Returns the name of the action.
     *
     * This is the name specified in the corresponding service description file.
     *
     * \return The name of the action.
     */
    QString name() const;

    /*!
     * \brief Returns the type of the action, i.e. is it required or optional.
     *
     * This is the name specified in the corresponding service description file.
     *
     * \return The type of the action.
     */
    HInclusionRequirement inclusionRequirement() const;

    /*!
     * \brief Specifies whether the depicted state variable is required or optional.
     *
     * \param arg specifies whether the service is required or optional.
     */
    void setInclusionRequirement(HInclusionRequirement arg);

    /*!
     * \brief Returns the maximum rate at which an evented state variable may send
     * events.
     *
     * \return The maximum rate at which an evented state variable may send
     * events. The returned value is -1 if the state variable is not evented or
     * the maximum rate has not been defined.
     *
     * \sa setMaxEventRate(), eventingType()
     */
    qint32 maxEventRate() const;

    /*!
     * \brief Sets the maximum rate at which an evented state variable may send
     * events.
     *
     * \param arg specifies the maximum rate at which an evented state
     * variable may send events. The rate is not set if the state variable is
     * not evented.
     *
     * \sa maxEventRate(), eventingType()
     */
    void setMaxEventRate(qint32 arg);

    /*!
     * \brief Returns the data type of the state variable.
     *
     * \return The data type of the state variable.
     */
    HUpnpDataTypes::DataType dataType() const;

    /*!
     * \brief Returns the type of eventing the state variable supports, if any.
     *
     * \return The type of eventing the state variable supports, if any.
     */
    EventingType eventingType() const;

    /*!
     * \brief Sets the type of eventing the state variable supports, if any.
     *
     * \param arg specifies the type of eventing the state variable supports, if any.
     */
    void setEventingType(EventingType arg);

    /*!
     * \brief Returns the list of allowed values.
     *
     * \return The list of allowed values if the contained data type is string
     * or empty list otherwise.
     *
     * \remarks This is only applicable on state variables, which data type is
     * HUpnpDataTypes::string.
     *
     * \sa setAllowedValueList(), dataType()
     */
    QStringList allowedValueList() const;

    /*!
     * \brief Specifies the values the state variable accepts.
     *
     * \param arg specifies the values the state variable accepts.
     *
     * \remarks This is only applicable on state variables, which data type is
     * HUpnpDataTypes::string.
     *
     * \sa allowedValueList(), dataType()
     */
    bool setAllowedValueList(const QStringList& arg);

    /*!
     * \brief Returns the minimum value of the specified value range.
     *
     * \return The minimum value of the specified value range.
     *
     * \remarks This is only applicable on state variables, which data type is
     * numeric. In addition, it is optional and it may not be defined.
     *
     * \sa dataType()
     */
    QVariant minimumValue() const;

    /*!
     * \brief Returns the maximum value of the specified value range.
     *
     * \return The maximum value of the specified value range.
     *
     * \remarks This is only applicable on state variables, which data type is
     * numeric. In addition, it is optional and it may not be defined.
     *
     * \sa dataType()
     */
    QVariant maximumValue() const;

    /*!
     * \brief Returns the step value of the specified value range.
     *
     * \return The step value of the specified value range.
     *
     * \remarks This is only applicable on state variables, which data type is
     * numeric. In addition, it is optional and it may not be defined.
     *
     * \sa dataType()
     */
    QVariant stepValue() const;

    /*!
     * \brief Sets the allowed value range.
     *
     * \param minimumValue specifies the inclusive lower bound of an
     * acceptable value. This cannot be larger than the \c maximumValue.
     *
     * \param maximumValue specifies the inclusive upper bound of an
     * acceptable value. This cannot be smaller than the \c minimumValue.
     *
     * \param stepValue specifies the step value. This value cannot be
     * larger than the subtraction of the maximum and minimum values.
     *
     * \param err specifies a pointer to a \c QString, which contains an
     * error description in case the any of the provided values is invalid. This
     * parameter is optional.
     *
     * \remarks This is only applicable on state variables, which data type is
     * numeric. In addition, it is optional and it may not be defined.
     *
     * \return \e true in case the values were successfully set.
     */
    bool setAllowedValueRange(
        const QVariant& minimumValue, const QVariant& maximumValue,
        const QVariant& stepValue, QString* err = 0);

    /*!
     * \brief Returns the default value of the state variable.
     *
     * \return The default value of the state variable. If no default has been
     * specified, QVariant::Invalid is returned.
     */
    QVariant defaultValue() const;

    /*!
     * \brief Sets the default value.
     *
     * \param arg specifies the default value. If the value range has been
     * specified the value has to be within the specified range.
     *
     * \param err specifies a pointer to a \c QString, which contains an
     * error description in case the value is invalid. This
     * parameter is optional.
     *
     * \return \e true in case the default value was successfully set.
     */
    bool setDefaultValue(const QVariant& arg, QString* err = 0);

    /*!
     * \brief Indicates if the state variable's value is constrained either by minimum,
     * maximum or by a list of allowed values.
     *
     * \return true in case the state variable's value is constrained either by minimum,
     * maximum or by a list of allowed values.
     *
     * \sa minimumValue(), maximumValue(), allowedValueList()
     */
    bool isConstrained() const;

    /*!
     * \brief Indicates whether or not the value is valid in terms of this particular
     * state variable.
     *
     * \param value specifies the value to be checked.
     *
     * \param convertedValue specifies a pointer to a \c QVariant that contains
     * the value as a variant of the correct type. This is optional.
     * Further, it will not be set if the value is invalid.
     *
     * \param err specifies a pointer to a \c QString, which contains an
     * error description in case the value is invalid. This
     * parameter is optional.
     *
     * \retval \e true in case the specified value is valid in terms of the
     * state variable this info object depicts. In other words, setValue() will
     * succeed with the value.
     *
     * \retval false otherwise.
     */
    bool isValidValue(
        const QVariant& value, QVariant* convertedValue = 0, QString* err = 0) const;

    /*!
     * \brief Indicates if the object is valid.
     *
     * \return \e true in case the object is valid.
     */
    bool isValid() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the object are logically equivalent.
 *
 * \relates HStateVariableInfo
 */
H_UPNP_CORE_EXPORT bool operator==(
    const HStateVariableInfo&, const HStateVariableInfo&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HStateVariableInfo
 */
inline bool operator!=(
    const HStateVariableInfo& obj1, const HStateVariableInfo& obj2)
{
    return !(obj1 == obj2);
}

/*!
 * \brief Returns a value that can be used as a unique key in a hash-map identifying
 * the object.
 *
 * \param key specifies the HStateVariableInfo object from which the hash value
 * is created.
 *
 * \return a value that can be used as a unique key in a hash-map identifying
 * the object.
 *
 * \relates HStateVariableInfo
 */
H_UPNP_CORE_EXPORT quint32 qHash(const HStateVariableInfo& key);

}
}

#endif /* HSTATEVARIABLEINFO_H_ */
