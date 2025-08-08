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

#include "qpsdhandler.h"
#ifdef QT_DEBUG
#include <QDebug>
#include <QElapsedTimer>
#endif

QPsdHandler::QPsdHandler() = default;

QPsdHandler::~QPsdHandler() = default;

bool QPsdHandler::canRead() const
{
    if (canRead(device())) {
        QByteArray bytes = device()->peek(6);
        QDataStream input(bytes);
        input.setByteOrder(QDataStream::BigEndian);
        quint32 signature;
        quint16 version;
        input >> signature >> version;
        if (version == 1)
            setFormat("psd");
        else if (version == 2)
            setFormat("psb");
        else
            return false;
        return true;
    }
    return false;
}

bool QPsdHandler::read(QImage *image)
{
    QDataStream input(device());
    quint32 signature, height, width;
    quint16 version, channels, depth, colorMode, compression;

    input.setByteOrder(QDataStream::BigEndian);

    input >> signature;
    if (!isValidSignature(signature)) {
        return false;
    }

    input >> version;
    if (!isValidVersion(version)) {
        return false;
    }

    input.skipRawData(6); // reserved bytes should be 6-byte in size

    input >> channels;
    if (!isChannelCountSupported(channels)) {
        return false;
    }

    input >> height;
    if (!isValidWidthOrHeight(version, height)) {
        return false;
    }

    input >> width;
    if (!isValidWidthOrHeight(version, width)) {
        return false;
    }

    input >> depth;
    if (!isSupportedDepth(depth)) {
        return false;
    }

    input >> colorMode;
    if (!isSupportedColorMode(colorMode)) {
        return false;
    }

    QByteArray colorData = readColorData(input);
    skipImageResources(input);
    skipLayerAndMaskSection(input);

    input >> compression;

    quint64 totalBytesPerChannel = (quint64)width * height * depth / 8;
    quint64 size = (quint64)channels * totalBytesPerChannel;
    QByteArray imageData;

    switch (compression) {
    case RLE:
        //        The RLE-compressed data is preceeded by a 2-byte(psd) or 4-byte(psb)
        //        data count for each row in the data
        if (format() == "psd")
            input.skipRawData(height * channels * 2);
        else if (format() == "psb")
            input.skipRawData(height * channels * 4);
        [[fallthrough]];
    case RAW:
        imageData = readImageData(input, (Compression)compression, size);
        break;
    default:
        return false;
        break;
    }

    if (input.status() != QDataStream::Ok)
        return false;

    if ((quint64)imageData.size() != size)
        return false;

#ifdef QT_DEBUG
    qDebug() << "format: " << format() << "\ncolor mode: " << colorMode << "\ndepth: " << depth
             << "\nchannels: " << channels << "\ncompression: " << compression << "\nwidth: " << width
             << "\nheight: " << height << "\ntotalBytesPerChannel: " << totalBytesPerChannel
             << "\nimage data: " << imageData.size();
#endif

    switch (colorMode) {
    case BITMAP: {
        QImage result = processBitmap(imageData, width, height);
        if (result.isNull())
            return false;
        else {
            *image = result;
            return true;
        }
    } break;
    case GRAYSCALE: {
        switch (depth) {
        case 8:
            if (1 == channels) {
                *image = processGrayscale8(imageData, width, height);
                return true;
            } else {
                // excess channels other than Gray are considered alphas
                *image = processGrayscale8WithAlpha(imageData, width, height, totalBytesPerChannel);
                return true;
            }
            break;
        case 16:
            if (1 == channels) {
                *image = processGrayscale16(imageData, width, height);
                return true;
            } else {
                // excess channels other than Gray are considered alphas
                *image = processGrayscale16WithAlpha(imageData, width, height, totalBytesPerChannel);
                return true;
            }
            break;
        case 32: // FIXME: 32 bpc (HDR)... requires tonemapping
        default:
            return false;
            break;
        }
    } break;
    case INDEXED:
        if (8 == depth && 1 == channels) {
            *image = processIndexed(colorData, imageData, width, height);
            return true;
        } else {
            return false;
        }
        break;
    case RGB: {
        switch (depth) {
        case 8:
            if (3 == channels) {
                *image = processRGB8(imageData, width, height, totalBytesPerChannel);
                return true;
            } else {
                // excess channels other than RGB are considered alphas
                *image = processRGB8WithAlpha(imageData, width, height, totalBytesPerChannel);
                return true;
            }
            break;
        case 16:
            if (3 == channels) {
                *image = processRGB16(imageData, width, height, totalBytesPerChannel);
                return true;
            } else {
                // excess channels other than RGB are considered alphas
                *image = processRGB16WithAlpha(imageData, width, height, totalBytesPerChannel);
                return true;
            }
            break;
        case 32: // FIXME: 32 bpc (HDR)... requires tonemapping
        default:
            return false;
            break;
        }
    } break;
    /* Mixed CMYK and Multichannel logic due to similarities*/
    case CMYK:
    case MULTICHANNEL: {
        //        Reference:
        //        http://help.adobe.com/en_US/photoshop/cs/using/WSfd1234e1c4b69f30ea53e41001031ab64-73eea.html#WSfd1234e1c4b69f30ea53e41001031ab64-73e5a
        //        Converting a CMYK image to Multichannel mode creates cyan, magenta, yellow, and black spot channels.
        //        Converting an RGB image to Multichannel mode creates cyan, magenta, and yellow spot channels.
        switch (depth) {
        case 8:
            if (3 == channels) {
                *image = processCMY8(imageData, width, height, totalBytesPerChannel);
                return true;
            } else if (4 == channels) {
                *image = processCMYK8(imageData, width, height, totalBytesPerChannel);
                return true;
            } else {
                *image = processCMYK8WithAlpha(imageData, width, height, totalBytesPerChannel);
                return true;
            }
            break;
        case 16:
            if (4 == channels) {
                *image = processCMYK16(imageData, width, height, totalBytesPerChannel);
                return true;
            } else {
                *image = processCMYK16WithAlpha(imageData, width, height, totalBytesPerChannel);
                return true;
            }
            break;
        default:
            return false;
            break;
        }
    } break;
    case DUOTONE:
        //        Duotone images: color data contains the duotone specification
        //        (the format of which is not documented). Other applications that
        //        read Photoshop files can treat a duotone image as a gray image,
        //        and just preserve the contents of the duotone information when
        //        reading and writing the file.
        //        TODO: find a way to actually get the duotone, tritone, and quadtone colors
        if (8 == depth && 1 == channels) {
            *image = processDuotone(imageData, width, height);
            return true;
        } else {
            return false;
        }
        break;
    case LAB: {
        switch (depth) {
        case 8:
            if (3 == channels) {
                *image = processLAB8(imageData, width, height, totalBytesPerChannel);
                return true;
            } else {
                *image = processLAB8WithAlpha(imageData, width, height, totalBytesPerChannel);
                return true;
            }
            break;
        case 16:
            if (3 == channels) {
                *image = processLAB16(imageData, width, height, totalBytesPerChannel);
                return true;
            } else {
                *image = processLAB16WithAlpha(imageData, width, height, totalBytesPerChannel);
                return true;
            }
            break;
        default:
            return false;
            break;
        }
    } break;
    default:
        return false;
        break;
    }
    return input.status() == QDataStream::Ok;
}

bool QPsdHandler::canRead(QIODevice *device)
{
    return device->peek(4) == "8BPS";
}

QVariant QPsdHandler::option(ImageOption option) const
{
    if (option == Size) {
        QByteArray bytes = device()->peek(26);
        QDataStream input(bytes);
        quint32 signature, height, width;
        quint16 version, channels, depth, colorMode;
        input.setByteOrder(QDataStream::BigEndian);
        input >> signature >> version;
        input.skipRawData(6); // reserved bytes should be 6-byte in size
        input >> channels >> height >> width >> depth >> colorMode;
        if (input.status() == QDataStream::Ok && signature == 0x38425053 && (version == 1 || version == 2))
            return QSize(width, height);
    }
    return QVariant();
}

bool QPsdHandler::supportsOption(ImageOption option) const
{
    return option == Size;
}
