/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Solutions component.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QT_NO_IMAGEFORMATPLUGIN
#include <QImageIOHandler>
#include <QStringList>

#include "qtjp2imageformatplugin.h"
#include "qtjp2imagehandler.h"

/*!
    \class QJPeg2Plugin

    \brief This class implements the QImageIOPlugin inteface for the \c
    JPEG 2000 image format.
*/

/*!
    Returns a QStringList containing the values "jp2" and "j2k".
*/
QStringList QJPeg2Plugin::keys() const
{
    return QStringList() << "jp2" << "j2k";
}

/*! \reimp
*/
QJPeg2Plugin::Capabilities QJPeg2Plugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "jp2" || format == "j2k")
        return Capabilities(CanRead | CanWrite);
    if (!format.isEmpty())
        return 0;
    if (!device->isOpen())
        return 0;

    Capabilities cap;
    if (device->isReadable() && QtJP2ImageHandler::canRead(device, 0))
        cap |= CanRead;
    if (device->isWritable())
        cap |= CanWrite;
    return cap;
}
/*! \reimp
*/
QImageIOHandler *QJPeg2Plugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new QtJP2ImageHandler(device);

    handler->setFormat(format);
    return handler;
}

Q_EXPORT_PLUGIN2(qtjp2, QJPeg2Plugin)
#endif

