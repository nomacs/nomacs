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

#ifndef HSERVERACTION_H_
#define HSERVERACTION_H_

#include <HUpnpCore/HUpnp>

#include <QtCore/QObject>

class QString;

namespace Herqq
{

namespace Upnp
{

class HServerActionPrivate;

/*!
 * \brief A class that represents a server-side UPnP action.
 *
 * \c %HServerAction is a core component of the HUPnP's server-side \ref hupnp_devicemodel
 * and it models a UPnP action. The UPnP Device Architecture specifies a UPnP
 * action as command, which takes one or more input or output arguments and that
 * may have a return value.
 *
 * \brief This class is used to invoke the server-side UPnP actions directly in process.
 * You can get information of the action using info(), which includes the action's
 * input and output arguments. You can invoke an action synchronously using
 * invoke().
 *
 * \headerfile hserveraction.h HServerAction
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HActionInfo, HServerService
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HServerAction :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HServerAction)
H_DECLARE_PRIVATE(HServerAction)

protected:

    HServerActionPrivate* h_ptr;

    /*!
     * \brief Creates a new instance.
     *
     * \param info specifies information of the action. This is usually read
     * from a service description document.
     *
     * \param parentService specifies the UPnP service instance that contains
     * this action.
     */
    HServerAction(const HActionInfo& info, HServerService* parentService);

public:

    /*!
     * \brief Destroys the instance.
     *
     * An \c %HServerAction is always destroyed by the \c %HServerService that
     * contains it when it is being deleted. Further, unless you hold the
     * ownership of the \c %HServerAction instance, you should never destroy it.
     */
    virtual ~HServerAction() = 0;

    /*!
     * \brief Returns the parent service of the action.
     *
     * \return The parent service of the action.
     *
     * \warning the pointer is guaranteed to point to a valid object as long
     * as the \c %HServerAction exists, which ultimately is as long as the
     * containing root device exists.
     *
     * \sa HServerDevice
     */
    HServerService* parentService() const;

    /*!
     * \brief Returns information about the action that is read from the
     * service description.
     *
     * \return information about the action that is read from the
     * service description.
     */
    const HActionInfo& info() const;

    /*!
     * Invokes the action synchronously.
     *
     * For example,
     *
     * \code
     *
     * Herqq::Upnp::HActionArguments inArgs = action->info().inputArguments();
     * inArgs.setValue("EchoInArgument", "Ping");
     *
     * Herqq::Upnp::HActionArguments outArgs;
     *
     * qint32 retVal = action->invoke(inArgs, &outArgs);
     * if (retVal == Herqq::Upnp::HServerAction::Success)
     * {
     *     qDebug() << outArgs.value("EchoOutArgument").toString();
     * }
     *
     * \endcode
     *
     * \param inArgs specifies the input arguments for the action.
     *
     * \param outArgs specifies a pointer to HActionArguments instance created
     * by the user. This can be null in which case the output arguments will not
     * be set even if the action has output arguments. If the parameter is specified
     * and the action has output arguments, the values of the arguments will be
     * set accordingly. If the action doesn't have output arguments,
     * the parameter is ignored.
     *
     * \return UpnpSuccess on success. Any other value indicates
     * that an error occurred.
     */
    qint32 invoke(
        const HActionArguments& inArgs, HActionArguments* outArgs = 0);
};

}
}

#endif /* HSERVERACTION_H_ */
