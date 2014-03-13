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

#ifndef HCLIENTADAPTER_OP_H_
#define HCLIENTADAPTER_OP_H_

#include <HUpnpCore/HAsyncOp>
#include <QtCore/QExplicitlySharedDataPointer>

namespace Herqq
{

namespace Upnp
{

//
//
//
struct H_UPNP_CORE_EXPORT HNullValue
{
};

class HClientAdapterOpPrivate;

template<typename T>
class HClientAdapterOp;

/*!
 * This is a type definition for a HClientAdapterOp that has no associated
 * return value.
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HClientAdapterOp
 */
typedef HClientAdapterOp<HNullValue> HClientAdapterOpNull;

class HAbstractClientAdapterOpPrivate;

/*!
 * This is an abstract base class for classes used to identify \e Client \e Adapter
 * operations.
 *
 * \headerfile hclientactionop.h HAbstractClientAdapterOp
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HClientDeviceAdapter, HClientServiceAdapter
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HAbstractClientAdapterOp :
    public HAsyncOp
{
protected:

    /*!
     * \brief Creates a new, empty instance.
     *
     * \sa isNull()
     */
    HAbstractClientAdapterOp();

    /*!
     * \brief Creates a new, invalid instance.
     *
     * \param returnCode specifies the return code of the operation.
     *
     * \param errorDescription specifies a human-readable error description.
     *
     * \sa isNull()
     */
    HAbstractClientAdapterOp(qint32 returnCode, const QString& errorDescription);

    /*!
     * \brief Copy constructor.
     *
     * Copies the contents of the \c other to this.
     */
    HAbstractClientAdapterOp(const HAbstractClientAdapterOp&);

    /*!
     * \brief Assigns the contents of the other object to this.
     *
     * \return reference to this object.
     */
    HAbstractClientAdapterOp& operator=(const HAbstractClientAdapterOp&);
};

/*!
 * This class is used to identify \e Client \e Adapter operation and
 * detail information of it.
 *
 * \tparam Value Specifies the return value of the operation. Note, this is not
 * the same as the UPnP \e return or \e error code, but rather an object, if any,
 * that is the result of the operation. For example, consider a derivative of
 * HClientServiceAdapter that has a method called \e getProtocolInfo(). Running
 * this method will always produce a UPnP return code, but if it succeeds, it
 * will also return an object that details the "Protocol Info", whatever that is.
 * \a Value represents that object and a concrete instantiation of this template
 * could be <c>HClientAdapterOp<MyProtocolInfo></c>.
 *
 * \note
 *
 * HClientAdapterOp is designed to be used by value. However, the class uses
 * \e explicit \e sharing, which essentially means that every copy of an
 * HClientAdapterOp instance accesses and modifies the same underlying data
 * and any change to that data is visible to all of the copies.
 *
 * \headerfile hclientadapterop.h HClientAdapterOp
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HClientDeviceAdapter, HClientServiceAdapter, HClientAdapterOpNull
 *
 * \remarks This class is not thread-safe.
 */
template<typename Value>
class HClientAdapterOp :
    public HAbstractClientAdapterOp
{

private:

    struct ValueContainer : public QSharedData
    {
        Value value;
        ValueContainer() : value() {}
        ValueContainer(const Value& value) : value(value) {}
    };

    QExplicitlySharedDataPointer<ValueContainer> m_value;

    //
    // \internal
    //
    HClientAdapterOp(qint32 returnCode, const QString& errorDescription) :
        HAbstractClientAdapterOp(returnCode, errorDescription)
    {
    }

public:

    /*!
     * \brief Creates a new, empty instance.
     *
     * \sa isNull()
     */
    HClientAdapterOp() :
        m_value(new ValueContainer())
    {
    }

    /*!
     * Creates a new instance and sets the value() as specified.
     *
     * \param value specifies the return value of the operation.
     *
     * \sa isNull()
     */
    explicit HClientAdapterOp(const Value& value) :
        HAbstractClientAdapterOp(), m_value(new ValueContainer(value))
    {
    }

    /*!
     * \brief Copy constructor.
     *
     * Copies the contents of the \c other to this.
     */
    HClientAdapterOp(const HClientAdapterOp& other) :
        HAbstractClientAdapterOp(other), m_value(other.m_value)
    {
    }

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HClientAdapterOp()
    {
    }

    /*!
     * \brief Assigns the contents of the other object to this.
     *
     * \param other specifies the other object.
     *
     * \return reference to this object.
     */
    HClientAdapterOp& operator=(const HClientAdapterOp& other)
    {
        HAbstractClientAdapterOp::operator=(other);
        m_value = other.m_value;
        return *this;
    }

    /*!
     * \brief Returns the return value of the operation.
     *
     * \return The return value of the operation.
     *
     * \sa setValue()
     */
    Value value() const
    {
        return m_value->value;
    }

    /*!
     * \brief Sets the return value of the operation.
     *
     * \param value specifies the return value of the operation.
     *
     * \sa value()
     */
    void setValue(const Value& value)
    {
        m_value->value = value;
    }

    /*!
     * Creates a new invalid instance.
     *
     * \param returnCode specifies the return code of the operation.
     *
     * \param errDescr specifies a human-readable error description.
     */
    static HClientAdapterOp createInvalid(int returnCode, const QString& errDescr = "")
    {
        return HClientAdapterOp<Value>(returnCode, errDescr);
    }
};

}
}

#endif /* HCLIENTADAPTER_OP_H_ */
