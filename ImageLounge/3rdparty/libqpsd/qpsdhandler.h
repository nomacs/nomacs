/*
Copyright (c) 2012-2019 Ronie Martinez (ronmarti18@gmail.com)
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

#ifndef QPSDHANDLER_H
#define QPSDHANDLER_H

#include <QColor>
#include <QImage>
#include <QImageIOHandler>
#include <QVariant>
#include <qmath.h>

class QPsdHandler : public QImageIOHandler
{
public:
    QPsdHandler();
    ~QPsdHandler();

    bool canRead() const;
    bool read(QImage *image);
    // bool write(const QImage &image);

    static bool canRead(QIODevice *device);

    QVariant option(ImageOption option) const;
    // void setOption(ImageOption option, const QVariant &value);
    bool supportsOption(ImageOption option) const;

private:
    bool isValidSignature(quint32 signature);
    bool isValidVersion(quint16 version);
    bool isChannelCountSupported(quint16 channel);
    bool isValidWidthOrHeight(quint16 version, quint32 value);
    bool isSupportedDepth(quint16 depth);
    bool isSupportedColorMode(quint16 colorMode);
    QByteArray readColorData(QDataStream &input);
    void skipImageResources(QDataStream &input);
    void skipLayerAndMaskSection(QDataStream &input);
    enum Compression {
        RAW = 0,
        RLE = 1,
        ZIP_WITHOUT_PREDICTION = 2,
        ZIP_WITH_PREDICTION = 3
    };
    QByteArray readImageData(QDataStream &input, Compression compression, quint64 size = 0);
    enum ColorMode {
        BITMAP = 0,
        GRAYSCALE = 1,
        INDEXED = 2,
        RGB = 3,
        CMYK = 4,
        MULTICHANNEL = 7,
        DUOTONE = 8,
        LAB = 9,
    };
    QImage processBitmap(QByteArray &imageData, quint32 width, quint32 height);
    QImage processGrayscale8(QByteArray &imageData, quint32 width, quint32 height);
    QImage processGrayscale8WithAlpha(QByteArray &imageData,
                                      quint32 width,
                                      quint32 height,
                                      quint64 totalBytesPerChannel);
    QImage processGrayscale16(QByteArray &imageData, quint32 width, quint32 height);
    QImage processGrayscale16WithAlpha(QByteArray &imageData,
                                       quint32 width,
                                       quint32 height,
                                       quint64 totalBytesPerChannel);
    QImage processIndexed(QByteArray &colorData, QByteArray &imageData, quint32 width, quint32 height);
    QImage processRGB8(QByteArray &imageData, quint32 width, quint32 height, quint64 totalBytesPerChannel);
    QImage processRGB16(QByteArray &imageData, quint32 width, quint32 height, quint64 totalBytesPerChannel);
    QImage processRGB8WithAlpha(QByteArray &imageData, quint32 width, quint32 height, quint64 totalBytesPerChannel);
    QImage processRGB16WithAlpha(QByteArray &imageData, quint32 width, quint32 height, quint64 totalBytesPerChannel);
    QImage processCMY8(QByteArray &imageData, quint32 width, quint32 height, quint64 totalBytesPerChannel);
    QImage processCMYK8(QByteArray &imageData, quint32 width, quint32 height, quint64 totalBytesPerChannel);
    QImage processCMYK8WithAlpha(QByteArray &imageData, quint32 width, quint32 height, quint64 totalBytesPerChannel);
    QImage processCMYK16(QByteArray &imageData, quint32 width, quint32 height, quint64 totalBytesPerChannel);
    QImage processCMYK16WithAlpha(QByteArray &imageData, quint32 width, quint32 height, quint64 totalBytesPerChannel);
    QImage processDuotone(QByteArray &imageData, quint32 width, quint32 height);
    QImage processLAB8(QByteArray &imageData, quint32 width, quint32 height, quint64 totalBytesPerChannel);
    QImage processLAB8WithAlpha(QByteArray &imageData, quint32 width, quint32 height, quint64 totalBytesPerChannel);
    QImage processLAB16(QByteArray &imageData, quint32 width, quint32 height, quint64 totalBytesPerChannel);
    QImage processLAB16WithAlpha(QByteArray &imageData, quint32 width, quint32 height, quint64 totalBytesPerChannel);
};

#endif // QPSDHANDLER_H
