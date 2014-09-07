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

#ifndef HACTIONARGUMENTS_H_
#define HACTIONARGUMENTS_H_

#include <HUpnpCore/HUpnpDataTypes>
#include <HUpnpCore/HStateVariableInfo>

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QExplicitlySharedDataPointer>

template<typename T, typename U>
class QHash;

template<typename T>
class QVector;

namespace Herqq
{

namespace Upnp
{

class HActionArgumentPrivate;

/*!
 * \brief This is a class that represents an argument used in a UPnP action invocation.
 *
 * A UPnP argument is defined in the UPnP service description within
 * an action. If you picture a UPnP action as a function, then an
 * action argument is a parameter to the function. In that sense a UPnP
 * \e input \e argument is a single \b constant parameter that provides
 * input for the function. An input argument is never modified during action
 * invocation. On the other hand, a UPnP \e output \e argument relays information
 * back from the callee to the caller and thus it is often modified during
 * action invocation.
 *
 * \section actionargument_basicuse Basic Use
 *
 * A UPnP argument has an unique name() within the definition
 * of the action that contains it. A UPnP argument contains a value, which you
 * can retrieve using value() and which you can set using setValue(). Note, the
 * value of a UPnP argument is bound by its dataType().
 *
 * A somewhat unusual aspect of a UPnP argument is the concept of a
 * <em>related state variable</em>. According to the UDA specification, a
 * UPnP argument is \b always associated with a state variable, even if the
 * state variable does not serve any other purpose besides that.
 * This type of a state variable is used to describe the data type of a
 * UPnP argument and thus the value of a UPnP argument is bound by the
 * data type of its related state variable. The dataType() method introduced
 * in this class is equivalent for calling relatedStateVariable()->dataType().
 *
 * \note
 * relatedStateVariable() returns a const reference to an HStateVariableInfo
 * object, rather than a reference or a pointer to an actual state variable.
 * HStateVariableInfo is an object with value semantics that details information
 * of a state variable.
 *
 * Due to the strict typing of UPnP arguments, HUPnP attempts to make sure that
 * invalid values are not entered into a UPnP argument. Because of this, you can
 * call isValidValue() to check if a value you wish to set using setValue()
 * will be accepted. In addition, setValue() returns \e false in case the value
 * was not accepted. It is advised that you make sure your values are properly
 * set before attempting to invoke an action, because the invocation may fail
 * in case any of the provided arguments is invalid.
 *
 * Finally, you can use isValid() to check if the object itself is valid, which
 * is true if the object was constructed with a proper name and valid related state
 * variable information.
 *
 * \note Since it is common for actions to use both input and output arguments
 * that are defined only for the duration of the action invocation,
 * there are bound to be numerous state variables that exist only for
 * UPnP action invocation. It is defined in the UDA specification
 * that these types of state variables have to have a name that includes the
 * prefix \b A_ARG_TYPE.
 *
 * \section actionargument_copysemantics Copy Semantics
 *
 * HActionArgument is designed to be used by value. However, the class uses
 * \e explicit \e sharing, which essentially means that every copy of an
 * HActionArgument instance accesses and modifies the same data until detach()
 * is called. The detach() function effectively modifies an HActionArgument instance to use
 * a "private" copy of the underlying data until the instance is copied
 * via a copy constructor or an assignment operator.
 *
 * \remarks This class is not thread-safe.
 *
 * \headerfile hactionarguments.h HActionArgument
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HActionArguments
 */
class H_UPNP_CORE_EXPORT HActionArgument
{
friend H_UPNP_CORE_EXPORT bool operator==(
    const HActionArgument&, const HActionArgument&);

private:

    QExplicitlySharedDataPointer<HActionArgumentPrivate> h_ptr;

public:

    /*!
     * Constructs a new, empty instance.
     *
     * \remarks Object constructed using this method is always invalid.
     *
     * \sa isValid()
     */
    HActionArgument();

    /*!
     * Initializes a new instance with the specified name and related state variable.
     *
     * \param name specifies the name of the argument.
     *
     * \param stateVariableInfo specifies the related state variable.
     *
     * \param err specifies a pointer to a \c QString, which will contain an
     * error description in case the provided arguments were not valid. This
     * is optional
     *
     * \remarks in case the name parameter fails the criteria specified for
     * UPnP action arguments in UPnP Device Architecture 1.1 specification
     * or the stateVariable is null, the object is constructed as "invalid";
     * isValid() always returns false.
     *
     * \sa isValid()
     */
    HActionArgument(
        const QString& name,
        const HStateVariableInfo& stateVariableInfo,
        QString* err = 0);

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HActionArgument(const HActionArgument&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HActionArgument& operator=(const HActionArgument&);

    /*!
     * \brief Destroys the instance.
     */
    ~HActionArgument();

    /*!
     * Creates a deep copy of the instance, if necessary.
     *
     * If the underlying reference count of this instance is greater than one,
     * this function creates a deep copy of the shared data and modifies this
     * instance to refer to the copied data.
     */
    void detach();

    /*!
     * \brief Returns the name of the argument.
     *
     * \return The name of the argument. The return value is an empty string in
     * case the object is invalid.
     *
     * \sa isValid()
     */
    QString name() const;

    /*!
     * \brief Returns information about the state variable that is associated
     * with this action argument.
     *
     * \return information about the state variable that is associated
     * with this action argument or a null pointer in case the object is invalid.
     *
     * \sa isValid()
     */
    const HStateVariableInfo& relatedStateVariable() const;

    /*!
     * Helper method for accessing the data type of the related state variable
     * info object directly.
     *
     * \return The data type of the state variable. The data type is
     * HUpnpDataTypes::Undefined in case the object is invalid.
     *
     * \sa isValid()
     */
    HUpnpDataTypes::DataType dataType() const;

    /*!
     * \brief Returns the value of the argument.
     *
     * \return The value of the argument. The returned \c QVariant has a type of
     * \c QVariant::Invalid in case the object is invalid.
     *
     * \sa isValid()
     */
    QVariant value() const;

    /*!
     * \brief Sets the value of the argument if the object is valid and the new value is
     * of right type.
     *
     * \param value specifies the new value of the argument.
     *
     * \return \e true in case the new value was successfully set.
     */
    bool setValue(const QVariant& value);

    /*!
     * \brief Indicates if the object is constructed with a proper name and a state
     * variable.
     *
     * \return \e true in case the object has a proper name and the object refers
     * to a valid state variable.
     */
    bool isValid() const;

    /*!
     * \brief Indicates whether or not the object is considered as invalid.
     *
     * This is the opposite for calling isValid().
     *
     * \return \e true in case the object is invalid.
     *
     * \sa isValid()
     */
    bool operator!() const;

    /*!
     * \brief Returns a string representation of the object.
     *
     * The format of the return value is \c "name: theValue".
     *
     * \return a string representation of the object. An empty string is returned
     * if the object is invalid.
     */
    QString toString() const;

    /*!
     * \brief Indicates if the provided value can be set into this input argument
     * successfully.
     *
     * A value is considered \e valid, when:
     * \li the argument object is valid, i.e. isValid() returns true and
     * \li the data type of the provided value matches the data type of the argument or
     * \li the data type of the provided value can be converted to the data type
     * of the argument.
     *
     * \param value specifies the value to be checked.
     *
     * \return \e true in case the provided value can be set into this input argument
     * successfully.
     */
    bool isValidValue(const QVariant& value);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the object are logically equivalent.
 *
 * \relates HActionArgument
 */
H_UPNP_CORE_EXPORT bool operator==(
    const HActionArgument&, const HActionArgument&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HActionArgument
 */
H_UPNP_CORE_EXPORT bool operator!=(
    const HActionArgument&, const HActionArgument&);

class HActionArgumentsPrivate;

/*!
 * A storage class for HActionArgument instances.
 *
 * Instances of this class are used to contain the input and output arguments
 * for an action invocation.
 *
 * \note
 * The class provides iterative and keyed access to the stored HActionArgument
 * instances. The order of action arguments during iteration is the order
 * in which the HActionArgument objects are provided to the instance.
 * If the class is instantiated by HUPnP, the order of the contained arguments
 * during iteration is the order in which they are defined in the service
 * description document.
 *
 * \headerfile hactionarguments.h HActionArguments
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HActionArgument
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HActionArguments
{
friend H_UPNP_CORE_EXPORT bool operator==(
    const HActionArguments&, const HActionArguments&);

private:

    HActionArgumentsPrivate* h_ptr;

public:

    typedef HActionArgument* iterator;
    typedef const HActionArgument* const_iterator;

    /*!
     * Swaps the contents of the two containers.
     *
     * Swaps the contents of the two containers.
     *
     * \relates HActionArguments
     */
    friend H_UPNP_CORE_EXPORT void swap(HActionArguments&, HActionArguments&);

    /*!
     * \brief Creates a new, empty instance.
     *
     * \sa isEmpty()
     */
    HActionArguments();

    /*!
     * Creates a new instance from the specified input arguments.
     *
     * \param args specifies the action argument objects this instance will
     * contain.
     *
     * \sa isEmpty()
     */
    HActionArguments(const QVector<HActionArgument>& args);

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HActionArguments(const HActionArguments&);

    /*!
     * \brief Destroys the instance.
     */
    ~HActionArguments();

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     *
     * \return a reference to this object.
     */
    HActionArguments& operator=(const HActionArguments&);

    /*!
     * \brief Indicates if the object contains an argument with the specified name.
     *
     * \param argumentName specifies the name of the action argument.
     *
     * \return \e true in case the object contains an argument with the specified name.
     *
     * \remarks This is a \e constant-time operation.
     */
    bool contains(const QString& argumentName) const;

    /*!
     * \brief Returns an action argument with the specified name.
     *
     * \param argumentName specifies the name of the argument to be retrieved.
     *
     * \return an action argument with the specified name. If no argument is found
     * with the specified name, the returned instance is invalid, i.e.
     * HActionArgument::isValid() returns \e false.
     *
     * \remarks This is a \e constant-time operation.
     */
    HActionArgument get(const QString& argumentName) const;

    /*!
     * \brief Returns an action argument at the specified index.
     *
     * \param index specifies the index of the action argument to return. The
     * index has to be valid position in the container, i.e. it must be
     * 0 <= i < size().
     *
     * \return an action argument at the specified \a index.
     *
     * \remarks This is a \e constant-time operation.
     */
    HActionArgument get(qint32 index) const;

    /*!
     * \brief Returns a const STL-style iterator pointing to the first item.
     *
     * \return a const STL-style iterator pointing to the first item.
     */
    HActionArguments::const_iterator constBegin() const;

    /*!
     * \brief Returns a const STL-style iterator pointing to the
     * imaginary item after the last item.
     *
     * \return a const STL-style iterator pointing to the
     * imaginary item after the last item.
     */
    HActionArguments::const_iterator constEnd() const;

    /*!
     * \brief Returns an STL-style iterator pointing to the first item.
     *
     * \return an STL-style iterator pointing to the first item.
     */
    HActionArguments::iterator begin();

    /*!
     * \overload
     *
     * \return an STL-style iterator pointing to the first item.
     */
    HActionArguments::const_iterator begin() const;

    /*!
     * \brief Returns an STL-style iterator pointing to the imaginary item
     * after the last item.
     *
     * \return an STL-style iterator pointing to the imaginary item
     * after the last item.
     */
    HActionArguments::iterator end();

    /*!
     * \overload
     *
     * \return an STL-style iterator pointing to the imaginary item
     * after the last item.
     */
    HActionArguments::const_iterator end() const;

    /*!
     * \brief Returns the number of contained action arguments.
     *
     * \return The number of contained action arguments.
     */
    qint32 size() const;

    /*!
     * \brief Returns an action argument at the specified index.
     *
     * This is the same as calling get() with the specified index. This method is
     * provided for convenience.
     *
     * \param index specifies the index of the action argument to return. The
     * index has to be valid position in the container, i.e. it must be
     * 0 <= i < size().
     *
     * \return an action argument at the specified index.
     */
    HActionArgument operator[](qint32 index) const;

    /*!
     * \brief Returns an action argument with the specified name.
     *
     * This is the same as calling get() with the specified argument name.
     * This method is provided for convenience.
     *
     * \param argName specifies the name of the argument to be retrieved.
     *
     * \return an action argument with the specified name. If no argument is found
     * with the specified name, the returned instance is invalid, i.e.
     * HActionArgument::isValid() returns \e false.
     *
     * \remarks This is a \e constant-time operation.
     */
    HActionArgument operator[](const QString& argName) const;

    /*!
     * \brief Returns the names of all the contained action arguments.
     *
     * \return The names of all the contained action arguments.
     */
    QStringList names() const;

    /*!
     * \brief Indicates if the object is empty, i.e. it has no action arguments.
     *
     * \return \e true when the object has no action arguments.
     */
    bool isEmpty() const;

    /*!
     * Removes every contained HActionArgument from this instance.
     *
     * \remarks
     * A call to this function makes active iterators invalid and
     */
    void clear();

    /*!
     * Removes an HActionArgument with the specified name.
     *
     * \param name specifies the name of the HActionArgument to be removed.
     *
     * \return \e true if an HActionArgument was found and removed.
     *
     * \remarks
     * A call to this function makes active iterators invalid and
     */
    bool remove(const QString& name);

    /*!
     * Inserts an HActionArgument to this instance.
     *
     * \param arg specifies the HActionArgument to be added.
     *
     * \return \e true if the specified argument was added. The action argument
     * will not be added if the instance already contains an action argument
     * instance with the same name or the provided instance is invalid.
     *
     * \remarks
     * A call to this function makes active iterators invalid and
     */
    bool append(const HActionArgument& arg);

    /*!
     * \brief Returns the value of the specified state variable, if such exists.
     *
     * This is a convenience method for retrieving the value of the specified
     * state variable. Semantically this call is comparable to
     * <c>get("stateVariable_name").value()</c>.
     *
     * \param name specifies the name of the state variable.
     *
     * \param ok specifies a pointer to \c bool, which will be \e true if
     * the specified state variable was found. This parameter is optional.
     *
     * \return The value of the specified state variable, if such exists.
     * Otherwise the returned \c QVariant is invalid.
     */
    QVariant value(const QString& name, bool* ok = 0) const;

    /*!
     * Attempts to set the value of the specified state variable.
     *
     * This is a convenience method for setting the value of the specified
     * state variable. Semantically this call is comparable to
     * <c>get("stateVariable_name").setValue(value)</c>.
     *
     * \param name specifies the name of the state variable.
     *
     * \param value specifies the value of the state variable.
     *
     * \return \e true in case the value of the specified state variable was
     * found and its value was changed.
     */
    bool setValue(const QString& name, const QVariant& value);

    /*!
     * \brief Returns a string representation of the object.
     *
     * \return a string representation of the object. The
     * returned string contains all the arguments represented as strings and
     * separated from each other by a new-line. The string representation of
     * an argument is retrieved using HActionArgument::toString().
     *
     * \remarks
     *  An empty string is returned if the object is invalid.
     */
    QString toString() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the object are logically equivalent.
 *
 * \relates HActionArguments
 */
H_UPNP_CORE_EXPORT bool operator==(
    const HActionArguments&, const HActionArguments&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HActionArguments
 */
inline bool operator!=(
    const HActionArguments& obj1, const HActionArguments& obj2)
{
    return !(obj1 == obj2);
}

}
}
#endif /* HACTIONARGUMENTS_H_ */
