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

#ifndef HCDSPROPERTY_H_
#define HCDSPROPERTY_H_

#include <HUpnpAv/HCdsPropertyInfo>

#include <HUpnpCore/HFunctor>

#include <QtCore/QSharedDataPointer>

class QXmlStreamWriter;
class QXmlStreamReader;

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * This is a type definition for a <em>callable entity</em> that is used
 * for serializing CDS property data to QXmlStreamWriter.
 *
 * You can create \c %HOutSerializer objects using normal functions, functors and
 * member functions that follow the signature of
 *
 * <tt>
 *
 * bool function(const QString&, const QVariant&, QXmlStreamWriter&);
 *
 * </tt>
 *
 * The following example demonstrates how you can instantiate the \c %HOutSerializer
 * for a normal function, functor and a member function.
 *
 * \code
 *
 * #include <HUpnpAv/HOutSerializer>
 *
 * #include "myclass.h" // your code that contains declaration for MyClass
 *
 * namespace
 * {
 * bool freefun(const QString&, const QVariant&, QXmlStreamWriter&)
 * {
 *     return true;
 * }
 *
 * class MyFunctor
 * {
 * public:
 *     bool operator()(const QString&, const QVariant&, QXmlStreamWriter&)
 *     {
 *         return true;
 *     }
 * };
 * }
 *
 * bool MyClass::memfun(const QString&, const QVariant&, QXmlStreamWriter&)
 * {
 *     return true;
 * }
 *
 * void MyClass::example()
 * {
 *     Herqq::Upnp::Av::HOutSerializer usingFreeFunction(freefun);
 *
 *     MyFunctor myfunc;
 *     Herqq::Upnp::Av::HOutSerializer usingFunctor(myfunc);
 *
 *     Herqq::Upnp::Av::HOutSerializer usingMemberFunction(this, &MyClass::memfun);
 * }
 *
 * \endcode
 *
 * You can test if the object can be invoked simply by issuing
 * <tt>if (serializerObject) { ... } </tt>
 *
 * \headerfile hcdsproperty.h HOutSerializer
 *
 * \ingroup hupnp_av_cds_om_mgmt
 */
typedef Functor<bool, H_TYPELIST_3(
    const QString&, const QVariant&, QXmlStreamWriter&)> HOutSerializer;

/*!
 * This is a type definition for a <em>callable entity</em> that is used
 * for serializing CDS property data from QXmlStreamReader.
 *
 * You can create \c %HInSerializer objects using normal functions, functors and
 * member functions that follow the signature of
 *
 * <tt>
 *
 * bool function(const QString&, QVariant*, QXmlStreamReader*);
 *
 * </tt>
 *
 * The following example demonstrates how you can instantiate the \c %HInSerializer
 * for a normal function, functor and a member function.
 *
 * \code
 *
 * #include <HUpnpAv/HInSerializer>
 *
 * #include "myclass.h" // your code that contains declaration for MyClass
 *
 * namespace
 * {
 * bool freefun(const QString&, QVariant*, QXmlStreamReader*)
 * {
 *     return true;
 * }
 *
 * class MyFunctor
 * {
 * public:
 *     bool operator()(const QString&, QVariant*, QXmlStreamReader*)
 *     {
 *         return true;
 *     }
 * };
 * }
 *
 * bool MyClass::memfun(const QString&, QVariant*, QXmlStreamReader*)
 * {
 *     return true;
 * }
 *
 * void MyClass::example()
 * {
 *     Herqq::Upnp::Av::HInSerializer usingFreeFunction(freefun);
 *
 *     MyFunctor myfunc;
 *     Herqq::Upnp::Av::HInSerializer usingFunctor(myfunc);
 *
 *     Herqq::Upnp::Av::HInSerializer usingMemberFunction(this, &MyClass::memfun);
 * }
 *
 * \endcode
 *
 * You can test if the object can be invoked simply by issuing
 * <tt>if (serializerObject) { ... } </tt>
 *
 * \headerfile hcdsproperty.h HInSerializer
 *
 * \ingroup hupnp_av_cds_om_mgmt
 */
typedef Functor<bool, H_TYPELIST_3(
    const QString&, QVariant*, QXmlStreamReader*)> HInSerializer;

/*!
 * This is a type definition for a <em>callable entity</em> that is used
 * for comparing the values of CDS properties against each other.
 *
 * You can create \c %HComparer objects using normal functions, functors and
 * member functions that follow the signature of
 *
 * <tt>
 *
 * bool function(const QVariant&, const QVariant&, qint32*);
 *
 * </tt>
 *
 * The following example demonstrates how you can instantiate the \c %HComparer
 * for a normal function, functor and a member function.
 *
 * \code
 *
 * #include <HUpnpAv/HComparer>
 *
 * #include "myclass.h" // your code that contains declaration for MyClass
 *
 * namespace
 * {
 * bool freefun(const QVariant&, const QVariant&, qint32*)
 * {
 *     return true;
 * }
 *
 * class MyFunctor
 * {
 * public:
 *     bool operator()(const QVariant&, const QVariant&, qint32*)
 *     {
 *         return true;
 *     }
 * };
 * }
 *
 * bool MyClass::memfun(const QVariant&, const QVariant&, qint32*)
 * {
 *     return true;
 * }
 *
 * void MyClass::example()
 * {
 *     Herqq::Upnp::Av::HComparer usingFreeFunction(freefun);
 *
 *     MyFunctor myfunc;
 *     Herqq::Upnp::Av::HComparer usingFunctor(myfunc);
 *
 *     Herqq::Upnp::Av::HComparer usingMemberFunction(this, &MyClass::memfun);
 * }
 *
 * \endcode
 *
 * You can test if the object can be invoked simply by issuing
 * <tt>if (serializerObject) { ... } </tt>
 *
 * \headerfile hcdsproperty.h HInSerializer
 *
 * \ingroup hupnp_av_cds_om_mgmt
 */
typedef Functor<bool, H_TYPELIST_3(
    const QVariant&, const QVariant&, qint32*)> HComparer;

/*!
 *
 * \ingroup hupnp_av_cds_om_mgmt
 */
typedef Functor<bool, H_TYPELIST_1(const QVariant&)> HValidator;

class HCdsPropertyHandlerPrivate;

/*!
 * \brief This class is used to serialize, compare and validate CDS objects.
 *
 * \headerfile hcdsproperty.h HCdsPropertyHandler
 *
 * \ingroup hupnp_av_cds_om_mgmt
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HCdsPropertyHandler
{
private:

    QSharedDataPointer<HCdsPropertyHandlerPrivate> h_ptr;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HCdsPropertyHandler();

    /*!
     * \brief Creates a new instance.
     *
     * \param oser specifies the \e output serializer.
     *
     * \param iser specifies the \e input serializer.
     *
     * \param comparer specifies the \e comparer.
     *
     * \param validator
     */
    HCdsPropertyHandler(
        const HOutSerializer& oser, const HInSerializer& iser,
        const HComparer& comparer, const HValidator& validator);

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HCdsPropertyHandler(const HCdsPropertyHandler&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HCdsPropertyHandler& operator=(const HCdsPropertyHandler&);

    /*!
     * \brief Destroys the instance.
     */
    ~HCdsPropertyHandler();

    /*!
     * \brief Indicates if the instance is valid.
     *
     * \return \e true if the instance is valid, i.e. at least() the
     * inSerializer() and outSerializer() are defined.
     */
    bool isValid() const;

    /*!
     * \brief Returns the output serializer.
     *
     * \return The output serializer.
     */
    HOutSerializer outSerializer() const;

    /*!
     * \brief Returns the input serializer.
     *
     * \return The input serializer.
     */
    HInSerializer inSerializer() const;

    /*!
     * \brief Returns the comparer.
     *
     * \return the comparer.
     */
    HComparer comparer() const;
    HValidator validator() const;

    /*!
     * \brief Specifies the output serializer.
     *
     * \param arg specifies the output serializer.
     */
    void setOutSerializer(const HOutSerializer& arg);

    /*!
     * \brief Specifies the input serializer.
     *
     * \param arg specifies the input serializer.
     */
    void setInSerializer(const HInSerializer& arg);

    /*!
     * Specifies the comparer.
     *
     * \param arg specifies the comparer.
     */
    void setComparer(const HComparer& arg);
    void setValidator(const HValidator& arg);
};

/*!
 * \brief This class depicts a CDS property from the implementation point of view.
 *
 * \headerfile hcdsproperty.h HCdsProperty
 *
 * \ingroup hupnp_av_cds_om_mgmt
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HCdsProperty
{

private:

    HCdsPropertyInfo m_info;
    HCdsPropertyHandler m_handler;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HCdsProperty();

    /*!
     * Creates a new type.
     *
     * \param info specifies information about the property.
     *
     * \param handler specifies the property handler object.
     *
     * \sa isValid()
     */
    HCdsProperty(const HCdsPropertyInfo& info, const HCdsPropertyHandler& handler);

    /*!
     * \brief Indicates if the object is empty.
     *
     * \return \e true in case the object is valid, i.e. both the property
     * handler and the information object are defined.
     */
    bool isValid() const;

    /*!
     * \brief Returns the property handler.
     *
     * \return The property handler.
     */
    inline const HCdsPropertyHandler& handler() const
    {
        return m_handler;
    }

    /*!
     * \brief Returns information about the property.
     *
     * \return information about the property.
     */
    inline const HCdsPropertyInfo& info() const
    {
        return m_info;
    }
};

}
}
}

#endif /* HCDSPROPERTY_H_ */
