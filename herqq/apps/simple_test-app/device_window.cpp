/*
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of an application named HUpnpSimpleTestApp
 *  used for demonstrating how to use the Herqq UPnP (HUPnP) library.
 *
 *  HUpnpSimpleTestApp is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  HUpnpSimpleTestApp is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with HUpnpSimpleTestApp. If not, see <http://www.gnu.org/licenses/>.
 */

#include "device_window.h"
#include "ui_device_window.h"

#include <HUpnpCore/HUpnp>
#include <HUpnpCore/HServiceId>
#include <HUpnpCore/HDeviceHost>
#include <HUpnpCore/HDeviceInfo>
#include <HUpnpCore/HActionInfo>
#include <HUpnpCore/HServiceInfo>
#include <HUpnpCore/HServerAction>
#include <HUpnpCore/HActionArguments>
#include <HUpnpCore/HServerStateVariable>
#include <HUpnpCore/HDeviceHostConfiguration>

#include <QtDebug>
#include <QDateTime>

using namespace Herqq::Upnp;

/*******************************************************************************
 * HTestService
 *******************************************************************************/
HTestService::HTestService()
{
}

HTestService::~HTestService()
{
}

HServerService::HActionInvokes HTestService::createActionInvokes()
{
    HActionInvokes retVal;

    //
    // This is where it is defined what are to be called when the actions
    // identified by their names are invoked.

    // In this example, public member functions are used.
    retVal.insert(
        "Echo", HActionInvoke(this, &HTestService::echoAction));

    retVal.insert(
        "Register", HActionInvoke(this, &HTestService::registerAction));

    retVal.insert(
        "Chargen", HActionInvoke(this, &HTestService::chargenAction));

    return retVal;
}

qint32 HTestService::echoAction(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    // Simple implementation of the echo service:
    // merely echos the received message back to the invoker.

    QString echoMsg = inArgs["MessageIn"].value().toString();
    (*outArgs)["MessageOut"].setValue(echoMsg);

    emit actionInvoked(
        "Echo", QString("Argument was set to [%1].").arg(echoMsg));

    // This signal is sent so that the user interface can react to this
    // particular invocation somehow.

    return UpnpSuccess;
}

qint32 HTestService::registerAction(
    const HActionArguments& /*inArgs*/, HActionArguments* /*outArgs*/)
{
    // Simple implementation of the Register service:
    // modifies an evented state variable, which causes events to be sent to
    // all registered listeners.

    bool ok = false;
    HServerStateVariable* sv = stateVariables().value("RegisteredClientCount");

    Q_ASSERT(sv);
    // fetch the state variable we want to modify

    quint32 count = sv->value().toUInt(&ok);
    Q_ASSERT(ok);
    // check its current value

    ok = sv->setValue(++count);
    Q_ASSERT(ok);
    // and increment it

    emit actionInvoked(
        "Register",
        QString("Register invoked %1 times.").arg(QString::number(count)));

    // This signal is sent so that the user interface can react to this
    // particular invocation somehow.

    return UpnpSuccess;
}

qint32 HTestService::chargenAction(
    const HActionArguments& inArgs,
    HActionArguments* outArgs)
{
    qint32 charCount = inArgs["Count"].value().toInt();
    (*outArgs)["Characters"].setValue(QString(charCount, 'z'));

    emit actionInvoked(
        "Chargen",
        QString("Character count set to %1.").arg(
            QString::number(charCount)));

    // This signal is sent so that the user interface can react to this
    // particular invocation somehow.

    return UpnpSuccess;
}

/*******************************************************************************
 * HTestDevice
 ******************************************************************************/
HTestDevice::HTestDevice() :
    HServerDevice()
{
}

HTestDevice::~HTestDevice()
{
}

/*******************************************************************************
 * DeviceWindow
 *******************************************************************************/
namespace
{
class Creator :
    public HDeviceModelCreator
{
protected:

    virtual Creator* newInstance() const
    {
        return new Creator();
    }

public:

    virtual HServerDevice* createDevice(const HDeviceInfo& info) const
    {
        if (info.deviceType().toString() == "urn:herqq-org:device:HTestDevice:1")
        {
            return new HTestDevice();
        }

        return 0;
    }

    virtual HServerService* createService(
        const HServiceInfo& serviceInfo, const HDeviceInfo&) const
    {
        if (serviceInfo.serviceType().toString() == "urn:herqq-org:service:HTestService:1")
        {
            return new HTestService();
        }

        return 0;
    }
};
}

DeviceWindow::DeviceWindow(QWidget *parent) :
    QMainWindow(parent),
        m_ui(new Ui::DeviceWindow), m_deviceHost(0), m_testDevice()
{
    m_ui->setupUi(this);

    HDeviceHostConfiguration hostConfiguration;

    Creator creator;
    hostConfiguration.setDeviceModelCreator(creator);

    HDeviceConfiguration config;
    config.setPathToDeviceDescription(
        "./descriptions/hupnp_testdevice.xml");
    // the path to the device description file we want to be instantiated

    config.setCacheControlMaxAge(30);

    hostConfiguration.add(config);

    m_deviceHost = new HDeviceHost(this);

    if (!m_deviceHost->init(hostConfiguration))
    {
        qWarning() << m_deviceHost->errorDescription();
        Q_ASSERT(false);
        return;
    }

    m_testDevice = m_deviceHost->rootDevices().at(0);
    // since we know there is at least one device if the initialization succeeded...

    HServerService* service =
        m_testDevice->serviceById(HServiceId("urn:herqq-org:serviceId:HTestService"));

    // our user interface is supposed to react when our actions are invoked, so
    // let's connect the signal introduced in HTestService to this class.
    // (note that the connection works although the static type of our "service"
    // is not HTestService during the connect() call)

    bool ok = connect(
        service, SIGNAL(actionInvoked(QString, QString)),
        this, SLOT(actionInvoked(QString, QString)));

    Q_ASSERT(ok); Q_UNUSED(ok)
}

DeviceWindow::~DeviceWindow()
{
    delete m_ui;
    delete m_deviceHost;
}

void DeviceWindow::actionInvoked(const QString& actionName, const QString& text)
{
    //
    // okay, one of our actions was invoked, let's display something.

    QString textToDisplay = QString("%1 Action [%2] invoked: %3").arg(
        QDateTime::currentDateTime().toString(), actionName, text);

    m_ui->statusDisplay->append(textToDisplay);
}

void DeviceWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);

    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void DeviceWindow::closeEvent(QCloseEvent*)
{
    emit closed();
}
