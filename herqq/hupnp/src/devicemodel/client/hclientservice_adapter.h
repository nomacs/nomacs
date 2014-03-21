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

#ifndef HCLIENTSERVICE_ADAPTER_H_
#define HCLIENTSERVICE_ADAPTER_H_

#include <HUpnpCore/HUpnp>

#include <QtCore/QObject>

namespace Herqq
{

namespace Upnp
{

class HClientServiceAdapterPrivate;

/*!
 * This is an abstract base class for classes that compose HClientService instances
 * and provide alternate interfaces for interacting with HClientServices.
 *
 * \headerfile hclientservice_adapter.h HClientServiceAdapter
 *
 * \ingroup hupnp_devicemodel
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HClientServiceAdapter :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HClientServiceAdapter)
H_DECLARE_PRIVATE(HClientServiceAdapter)

protected:

    HClientServiceAdapterPrivate* h_ptr;

    /*!
     * \brief Creates a new instance.
     *
     * \param serviceType specifies the UPnP service type this adapter can handle.
     * Note, every HClientService to be associated with this adapter has to be
     * of the specified service type.
     *
     * \param parent specifies the parent \c QObject.
     */
    HClientServiceAdapter(const HResourceType& serviceType, QObject* parent = 0);

    //
    // \internal
    //
    HClientServiceAdapter(
        HClientServiceAdapterPrivate& dd, QObject* parent = 0);

    /*!
     * Prepares the instance to be associated with the specified HClientService.
     *
     * \param service specifies the service to be associated with this adapter.
     *
     * \return \e true if the instance was successfully prepared to be used
     * with the specified HClientService. If \e false is returned the
     * specified HClientService was not associated with this instance and
     * isReady() will return \e false.
     */
    virtual bool prepareService(HClientService* service);

    /*!
     * \brief Specifies the device model info provider the adapter should use to validate
     * HClientService instances with which this instance is associated.
     *
     * \param infoProvider specifies the device model info provider to be used.
     */
    void setDeviceModelInfoProvider(const HDeviceModelInfoProvider& infoProvider);

    /*!
     * \brief Returns the device model info provider the adapter uses to validate
     * HClientService instances with which this instance is associated.
     *
     * \return The device model info provider the adapter uses to validate
     * HClientService instances with which this instance is associated.
     */
    const HDeviceModelInfoProvider* deviceModelInfoProvider() const;

    /*!
     * Attempts to retrieve an action with the specified name.
     *
     * \param name specifies the name of the action to retrieve.
     *
     * \param rc specifies a pointer to \c qint32 that will contain the return
     * code of the call.
     *
     * \return The action instance that has the specified name, if any.
     */
    HClientAction* getAction(const QString& name, qint32* rc = 0) const;

    /*!
     * \brief Specifies a description of the last error that occurred.
     *
     * \param errDescr specifies a description of the last error that occurred.
     *
     * \sa lastErrorDescription()
     */
    void setLastErrorDescription(const QString& errDescr);

public:

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HClientServiceAdapter() = 0;

    /*!
     * \brief This enumeration specifies the different methods of validating
     * HClientService instances.
     */
    enum ValidationType
    {
        /*!
         * Only the service type should be checked to be correct.
         *
         * \sa serviceType()
         */
        MinimalValidation,

        /*!
         * The service type is checked and the service should be validated
         * using the deviceModelInfoProvider().
         */
        FullValidation
    };

    /*!
     * Associates the specified \c HClientService with this instance.
     *
     * \param service specifies the service to be associated with this instance.
     *
     * \param vt specifies how the \a service should be validated.
     *
     * \return \e true in case the \c service was successfully associated with
     * this instance.
     *
     * \sa service()
     */
    bool setService(HClientService* service, ValidationType vt = FullValidation);

    /*!
     * \brief Returns a description of the last error that occurred.
     *
     * \return a description of the last error that occurred.
     */
    QString lastErrorDescription() const;

    /*!
     * \brief Returns the \c HClientService associated with this instance.
     *
     * \return The \c HClientService associated with this instance, if any.
     * A null pointer is returned in case setService() has not been called
     * successfully.
     *
     * \sa setService()
     */
    HClientService* service() const;

    /*!
     * \brief Indicates if the HClientService associated with this instance implements
     * an action with the specified name.
     *
     * \param name specifies the name of the action to be checked.
     *
     * \return \e true in case the associated HClientService implements the
     * specified action.
     */
    bool implementsAction(const QString& name) const;

    /*!
     * \brief Indicates if the HClientService associated with this instance implements
     * a state variable with the specified name.
     *
     * \param name specifies the name of the state variable to be checked.
     *
     * \return \e true in case the associated HClientService implements the
     * specified state variable.
     */
    bool implementsStateVariable(const QString& name) const;

    /*!
     * \brief Indicates if the object is ready to be used.
     *
     * \return \e true if the object is ready to be used.
     */
    bool isReady() const;

    /*!
     * \brief Returns the service type this adapter abstracts.
     *
     * \return The service type this adapter abstracts.
     */
    const HResourceType& serviceType() const;
};

}
}

#endif /* HCLIENTSERVICE_ADAPTER_H_ */
