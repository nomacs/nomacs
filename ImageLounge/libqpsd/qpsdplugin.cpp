/*
Copyright (c) 2012-2013 Ronie Martinez (ronmarti18@gmail.com)
Copyright (c) 2013 Yuezhao Huang (huangezhao@gmail.com)
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU Lesser General Public License for more
details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301  USA
*/

#include "qpsdplugin.h"
#include "qpsdhandler.h"

QPSDPlugin::QPSDPlugin(QObject *parent) :
    QImageIOPlugin(parent)
{
}

QPSDPlugin::~QPSDPlugin()
{
}

QStringList QPSDPlugin::keys() const
{
    return QStringList() << "psd";//TODO: add PSB (Photoshop Big) support
}

QImageIOPlugin::Capabilities QPSDPlugin::capabilities(
    QIODevice *device, const QByteArray &format) const
{
    if (format == "psd")
        return Capabilities(CanRead);//TODO: add CanWrite support
    if (!(format.isEmpty() && device->isOpen()))
        return 0;

    Capabilities cap;
    if (device->isReadable() && QPSDHandler::canRead(device))
        cap |= CanRead;
    // if (device->isWritable())
    //     cap |= CanWrite;
    return cap;
}

QImageIOHandler *QPSDPlugin::create(
    QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new QPSDHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(QPSD, QPSDPlugin)
#endif // QT_VERSION < 0x050000
