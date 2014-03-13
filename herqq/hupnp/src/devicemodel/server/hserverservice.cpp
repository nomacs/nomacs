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

#include "hserverservice.h"
#include "hserverservice_p.h"

#include "../../general/hlogger_p.h"

#include <QtCore/QMetaMethod>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HServerServicePrivate
 ******************************************************************************/
HServerServicePrivate::HServerServicePrivate()
{
}

HServerServicePrivate::~HServerServicePrivate()
{
}

HServerServicePrivate::ReturnValue HServerServicePrivate::updateVariables(
    const QList<QPair<QString, QString> >& variables, bool sendEvent)
{
    ReturnValue rv =
        HServicePrivate<HServerService, HServerAction, HServerStateVariable>::updateVariables(variables);

    if (rv == Updated && sendEvent && m_evented)
    {
        emit q_ptr->stateChanged(q_ptr);
    }

    return rv;
}

/*******************************************************************************
 * HServerService
 ******************************************************************************/
HServerService::HServerService() :
    h_ptr(new HServerServicePrivate())
{
}

HServerService::HServerService(HServerServicePrivate& dd) :
    h_ptr(&dd)
{
}

HServerService::~HServerService()
{
    delete h_ptr;
}

bool HServerService::init(
    const HServiceInfo& info, HServerDevice* parentDevice)
{
    if (h_ptr->q_ptr)
    {
        return false;
    }

    Q_ASSERT_X(parentDevice, "parentDevice", "Parent device has to be defined.");
    setParent(reinterpret_cast<QObject*>(parentDevice));
    h_ptr->m_serviceInfo = info;
    h_ptr->q_ptr = this;

    return true;
}

namespace
{
class MetaMethodInvoker
{
private:

    const char* m_typeName;
    HServerService* m_methodOwner;
    QMetaMethod m_mm;

public:

    MetaMethodInvoker(
        HServerService* methodOwner, const QMetaMethod& mm,
        const char* typeName) :
            m_typeName(typeName), m_methodOwner(methodOwner), m_mm(mm)
    {
        Q_ASSERT(methodOwner);
    }

    int operator()(
        const HActionArguments& inArgs, HActionArguments* outArgs)
    {
        int retVal = UpnpSuccess;

        bool ok = m_mm.invoke(
            m_methodOwner,
            Qt::DirectConnection,
            //Q_RETURN_ARG(int, retVal),
            QGenericReturnArgument(m_typeName, static_cast<void*>(&retVal)),
            Q_ARG(Herqq::Upnp::HActionArguments, inArgs),
            Q_ARG(Herqq::Upnp::HActionArguments*, outArgs));

        Q_ASSERT(ok); Q_UNUSED(ok)

        // Q_RETURN_ARG is not used above, because it cannot handle typedefs
        // and in this case it is perfectly reasonable for the user to
        // use "int" or "qint32" as the return type. Certainly other typedefs
        // besides "qint32" should be valid too, but for now the typeName has to
        // be either "int" or "qint32". The restriction is enforced in the
        // createActionInvokes() method below.

        // For instance, a user could have one action defined as:
        // qint32 myAction(const HActionArguments&, HActionArguments*);
        // and another as:
        // int myAction2(const HActionArguments&, HActionArguments*);

        return retVal;
    }
};
}

HServerService::HActionInvokes HServerService::createActionInvokes()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    HActionInvokes retVal;

    const QMetaObject* mob = metaObject();

    for(int i = mob->methodOffset(); i < mob->methodCount(); ++i)
    {
        QMetaMethod mm = mob->method(i);

        QString typeName = mm.typeName();
        if (typeName != "int" && typeName != "qint32")
        {
            continue;
        }

        QList<QByteArray> parTypes = mm.parameterTypes();
        if (parTypes.size() != 2)
        {
            continue;
        }

        QString firstPar = parTypes.at(0);
        if (firstPar != "Herqq::Upnp::HActionArguments" &&
            firstPar != "HActionArguments")
        {
            continue;
        }

        QString secondPar = parTypes.at(1);
        if (secondPar != "Herqq::Upnp::HActionArguments*" &&
            secondPar != "HActionArguments*")
        {
            continue;
        }

        QString signature = mm.signature();
        signature = signature.left(signature.indexOf('('));

        Q_ASSERT(!retVal.contains(signature));

        retVal.insert(signature, MetaMethodInvoker(this, mm, mm.typeName()));
        // See the comment in MetaMethodInvoker why the typeName() is passed
        // there as well.
    }

    return retVal;
}

bool HServerService::finalizeInit(QString*)
{
    // intentionally empty.
    return true;
}

HServerDevice* HServerService::parentDevice() const
{
    return reinterpret_cast<HServerDevice*>(parent());
}

const HServiceInfo& HServerService::info() const
{
    return h_ptr->m_serviceInfo;
}

const QString& HServerService::description() const
{
    return h_ptr->m_serviceDescription;
}

const HServerActions& HServerService::actions() const
{
    return h_ptr->m_actions;
}

const HServerStateVariables& HServerService::stateVariables() const
{
    return h_ptr->m_stateVariables;
}

void HServerService::notifyListeners()
{
    if (h_ptr->m_evented)
    {
        emit stateChanged(this);
    }
}

bool HServerService::isEvented() const
{
    return h_ptr->m_evented;
}

QVariant HServerService::value(const QString& stateVarName, bool* ok) const
{
    return h_ptr->value(stateVarName, ok);
}

bool HServerService::setValue(const QString& stateVarName, const QVariant& value)
{
    return h_ptr->setValue(stateVarName, value);
}

}
}
