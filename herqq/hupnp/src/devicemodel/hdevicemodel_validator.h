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

#ifndef HDEVICEMODEL_VALIDATOR_H_
#define HDEVICEMODEL_VALIDATOR_H_

#include <HUpnpCore/HUpnp>

namespace Herqq
{

namespace Upnp
{

class HDeviceModelValidatorPrivate;

/*!
 * \brief This class is used to validate components of the UPnP Device Architecture.
 *
 * \headerfile hdevicemodel_validator.h HDeviceModelValidator
 *
 * \ingroup hupnp_devicemodel
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HDeviceModelValidator
{
H_DISABLE_COPY(HDeviceModelValidator)

private:

    HDeviceModelValidatorPrivate* h_ptr;

public:

    /*!
     * \brief Creates a new instance.
     */
    HDeviceModelValidator();

    /*!
     * \brief Destroys the instance.
     */
    ~HDeviceModelValidator();

    /*!
     * \brief Specifies the object that provides information to the validator during
     * the validation process.
     *
     * \param provider specifies the \e info \e provider.
     *
     * \sa infoProvider()
     */
    void setInfoProvider(const HDeviceModelInfoProvider& provider);

    /*!
     * \brief Returns the object that provides information to the validator during
     * the validation process.
     *
     * \return the object that provides information to the validator during
     * the validation process.
     *
     * \sa setInfoProvider()
     */
    const HDeviceModelInfoProvider* infoProvider() const;

    /*!
     * \brief This enumeration specifies the return values the validation methods
     * may return.
     */
    enum ValidationResult
    {
        /*!
         * The validation could not be run, since the \e info \e provider had not
         * been specified.
         */
        InfoProviderMissing,

        /*!
         * The validation failed, because a mandatory UPnP action was missing.
         */
        MandatoryActionMissing,

        /*!
         * The validation failed, because a mandatory UPnP action argument was missing.
         */
        MandatoryActionArgumentMissing,

        /*!
         * The validation failed, because a mandatory UPnP state variable was missing.
         */
        MandatoryStateVariableMissing,

        /*!
         * The validation failed, because a mandatory UPnP service was missing.
         */
        MandatoryServiceMissing,

        /*!
         * The validation failed, because a mandatory UPnP device was missing.
         */
        MandatoryDeviceMissing,

        /*!
         * The validation failed, because the definition of a UPnP action
         * argument was invalid.
         */
        InvalidArgumentDefinition,

        /*!
         * The validation failed, because the definition of a UPnP state
         * variable was invalid.
         */
        InvalidStateVariableDefinition,

        /*!
         * The validation succeeded.
         */
        ValidationSucceeded
    };

    /*!
     * Validates the specified client device.
     *
     * \param device specifies the client device to validate.
     *
     * \return a value indicating whether the validation succeeded.
     *
     * \sa lastErrorDescription()
     */
    ValidationResult validateDevice(HClientDevice* device);

    /*!
     * Validates the specified server device.
     *
     * \param device specifies the server device to validate.
     *
     * \return a value indicating whether the validation succeeded.
     *
     * \sa lastErrorDescription()
     */
    ValidationResult validateDevice(HServerDevice* device);

    /*!
     * Validates the specified client service.
     *
     * \param service specifies the client service to validate.
     *
     * \return a value indicating whether the validation succeeded.
     *
     * \sa lastErrorDescription()
     */
    ValidationResult validateService(HClientService* service);

    /*!
     * Validates the specified server service.
     *
     * \param service specifies the server service to validate.
     *
     * \return a value indicating whether the validation succeeded.
     *
     * \sa lastErrorDescription()
     */
    ValidationResult validateService(HServerService* service);

    /*!
     * Validates the specified client action.
     *
     * \param action specifies the client action to validate.
     *
     * \return a value indicating whether the validation succeeded.
     *
     * \sa lastErrorDescription()
     */
    ValidationResult validateAction(HClientAction* action);

    /*!
     * Validates the specified server action.
     *
     * \param action specifies the server action to validate.
     *
     * \return a value indicating whether the validation succeeded.
     *
     * \sa lastErrorDescription()
     */
    ValidationResult validateAction(HServerAction* action);

    /*!
     * Validates the specified client state variable.
     *
     * \param stateVar specifies the client state variable to validate.
     *
     * \return a value indicating whether the validation succeeded.
     *
     * \sa lastErrorDescription()
     */
    ValidationResult validateStateVariable(HClientStateVariable* stateVar);

    /*!
     * Validates the specified server state variable.
     *
     * \param stateVar specifies the server state variable to validate.
     *
     * \return a value indicating whether the validation succeeded.
     *
     * \sa lastErrorDescription()
     */
    ValidationResult validateStateVariable(HServerStateVariable* stateVar);

    /*!
     * \brief Returns a description of the last error that occurred.
     *
     * \return a description of the last error that occurred.
     */
    QString lastErrorDescription() const;
};

}
}

#endif /* HDEVICEMODEL_VALIDATOR_H_ */
