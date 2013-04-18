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

#include "qpsdhandler.h"

QRgb psd_axyz_to_color(quint8 alpha, qreal x, qreal y, qreal z)
{
    qreal var_x = x / 100.0;
    qreal var_y = y / 100.0;
    qreal var_z = z / 100.0;

    qreal var_r = var_x * 3.2406 + var_y * -1.5372 + var_z * -0.4986;
    qreal var_g = var_x * -0.9689 + var_y * 1.8758 + var_z * 0.0415;
    qreal var_b = var_x * 0.0557 + var_y * -0.2040 + var_z * 1.0570;

    int red, green, blue;

    if ( var_r > 0.0031308 )
        var_r = 1.055 *  pow(var_r, 1/2.4)  - 0.055;
    else
        var_r = 12.92 * var_r;

    if ( var_g > 0.0031308 )
        var_g = 1.055 *  pow(var_g, 1/2.4)  - 0.055;
    else
        var_g = 12.92 * var_g;

    if ( var_b > 0.0031308 )
        var_b = 1.055 *  pow(var_b, 1/2.4) - 0.055;
    else
        var_b = 12.92 * var_b;

    red = var_r * 256.0;
    green = var_g * 256.0;
    blue = var_b * 256.0;

    return qRgba(red, green, blue, alpha);
}

QRgb psd_xyz_to_color(qreal x, qreal y, qreal z)
{
    return psd_axyz_to_color(255, x, y, z);
}

QRgb psd_alab_to_color(quint8 alpha, qint32 lightness, qint32 a, qint32 b)
{
    // For the conversion we first convert values to XYZ and then to RGB
    // ref_X = 95.047, ref_Y = 100.000, ref_Z = 108.883
    qreal x, y, z;
    const qreal ref_x = 95.047;
    const qreal ref_y = 100.000;
    const qreal ref_z = 108.883;

    qreal var_y = ( lightness + 16.0 ) / 116.0;
    qreal var_x = a / 500.0 + var_y;
    qreal var_z = var_y - b / 200.0;

    if ( pow(var_y, 3) > 0.008856 )
        var_y = pow(var_y, 3);
    else
        var_y = ( var_y - 16 / 116 ) / 7.787;

    if ( pow(var_x, 3) > 0.008856 )
        var_x = pow(var_x, 3);
    else
        var_x = ( var_x - 16 / 116 ) / 7.787;

    if ( pow(var_z, 3) > 0.008856 )
        var_z = pow(var_z, 3);
    else
        var_z = ( var_z - 16 / 116 ) / 7.787;

    x = ref_x * var_x;
    y = ref_y * var_y;
    z = ref_z * var_z;

    return psd_axyz_to_color(alpha, x, y, z);
}

QRgb psd_lab_to_color(qint32 lightness, qint32 a, qint32 b)
{
    return psd_alab_to_color(255, lightness, a, b);
}

QPSDHandler::QPSDHandler()
{
}

QPSDHandler::~QPSDHandler()
{
}

bool QPSDHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("psd");
        return true;
    }
    return false;
}

bool QPSDHandler::canRead(QIODevice *device)
{
    return device->peek(4) == QByteArray("\x38\x42\x50\x53"); //8BPS
}

bool QPSDHandler::read(QImage *image)
{
    QDataStream input(device());
    quint32 signature, height, width, colorModeDataLength, imageResourcesLength, layerAndMaskInformationLength;
    quint16 version, channels, depth, colorMode, compression;
    QByteArray colorData;
    input.setByteOrder(QDataStream::BigEndian);
    input >> signature >> version ;
    input.skipRawData(6);//reserved bytes should be 6-byte in size
    input >> channels >> height >> width >> depth >> colorMode;
    input >> colorModeDataLength;
    if (colorModeDataLength != 0) {
        quint8 byte;
        for(quint32 i=0; i<colorModeDataLength; ++i) {
            input >> byte;
            colorData.append(byte);
        }
    }

    input >> imageResourcesLength;
    input.skipRawData(imageResourcesLength);
    input >> layerAndMaskInformationLength;
    input.skipRawData(layerAndMaskInformationLength);
    input >> compression;

    if (input.status() != QDataStream::Ok || signature != 0x38425053 || version != 0x0001)
        return false;

    QByteArray decompressed;
    switch (compression) {
    case 0: /*RAW IMAGE DATA - UNIMPLEMENTED*/
        break;
    case 1: /*RLE COMPRESSED DATA*/
        // The RLE-compressed data is proceeded by a 2-byte data count for each row in the data,
        // which we're going to just skip.
        input.skipRawData(height*channels*2);

        quint8 byte,count;
        decompressed.clear();

        /*Code based on PackBits implementation which is primarily used by Photoshop for RLE encoding/decoding*/
        while (!input.atEnd()) {
            input >> byte;
            if (byte > 128) {
                count=256-byte;
                input >>  byte;
                for (quint8 i=0; i<=count; ++i) {
                    decompressed.append(byte);
                }
            } else if (byte < 128) {
                count = byte + 1;
                for(quint8 i=0; i<count; ++i) {
                    input >> byte;
                    decompressed.append(byte);
                }
            }
        }
        break;
    case 2:/*ZIP WITHOUT PREDICTION - UNIMPLEMENTED*/
        break;
    case 3:/*ZIP WITH PREDICTION - UNIMPLEMENTED*/
        break;
    }

    int totalBytes = width * height;

    //FIXME: find better alternative
    switch (colorMode) {
    case 0: //for bitmap
        if (decompressed.size() != (channels * totalBytes)/8)
            return false;
        break;
    default: //for non-bitmap
        if (decompressed.size() != channels * totalBytes)
            return false;

        break;
    }

    switch (colorMode) {
    case 0: /*BITMAP*/
    {
        QString head = QString("P4\n%1 %2\n").arg(width).arg(height);
        //QByteArray buffer(head.toAscii());
        QByteArray buffer(head.toUtf8());
        buffer.append(decompressed);
        QImage result = QImage::fromData(buffer);
        if (result.isNull())
            return false;
        else
            *image = result;
    }

        break;
    case 1: /*GRAYSCALE*/
        switch (depth) {
        case 8:
            switch (channels) {
            case 1:
                QImage result(width, height, QImage::Format_Indexed8);
                const int IndexCount = 256;
                for (int i = 0; i < IndexCount; ++i){
                    result.setColor(i, qRgb(i, i, i));
                }

                quint8 *data = (quint8*)decompressed.constData();
                for (quint32 i=0; i < height; ++i) {
                    for (quint32 j=0; j < width; ++j) {
                        result.setPixel(j,i, *data);
                        ++data;
                    }
                }

                *image = result;
                break;
            }
        }

        break;
    case 2: /*INDEXED*/
        switch (depth) {
        case 8:
            switch (channels) {
            case 1:
                QImage result(width, height, QImage::Format_Indexed8);
                int indexCount = colorData.size() / 3;
                Q_ASSERT(indexCount == 256);
                quint8 *red = (quint8*)colorData.constData();
                quint8 *green = red + indexCount;
                quint8 *blue = green + indexCount;
                for (int i=0; i < indexCount; ++i) {
                    /*
                     * reference https://github.com/OpenImageIO/oiio/blob/master/src/psd.imageio/psdinput.cpp
                     * function bool PSDInput::indexed_to_rgb (char *dst)
                     */
                    result.setColor(i, qRgb(*red, *green, *blue));
                    ++red; ++green; ++blue;
                }

                quint8 *data = (quint8*)decompressed.constData();
                for (quint32 i=0; i < height; ++i) {
                    for (quint32 j=0; j < width; ++j) {
                        result.setPixel(j,i,*data);
                        ++data;
                    }
                }
                *image=result;
                break;
            }
        }
        break;
    case 3: /*RGB*/
        switch (depth) {
        case 1:
            break;
        case 8:
            switch(channels) {
            case 1:
                break;
            case 3:
            {
                QImage result(width, height, QImage::Format_RGB32);
                quint8 *red = (quint8*)decompressed.constData();
                quint8 *green = red + totalBytes;
                quint8 *blue = green + totalBytes;
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        *p = qRgb(*red, *green, *blue);
                        ++p; ++red; ++green; ++blue;
                    }
                }

                *image = result;
            }
                break;
            case 4:
            {
                QImage result(width, height, QImage::Format_ARGB32);
                quint8 *red = (quint8*)decompressed.constData();
                quint8 *green = red + totalBytes;
                quint8 *blue = green + totalBytes;
                quint8 *alpha = blue + totalBytes;
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        *p = qRgba(*red, *green, *blue, *alpha);
                        ++p; ++red; ++green; ++blue; ++alpha;
                    }
                }

                *image = result;
            }
                break;
            case 5:
                qDebug("5 channels of rgb mode");
                return false;
            }

            break;
        case 16:
            break;
        }
        break;
    case 4: /*CMYK*/
        switch (depth) {
        case 8:
            switch (channels) {
            case 4:
            {
                QImage result(width, height, QImage::Format_RGB32);
                quint8 *cyan = (quint8*)decompressed.constData();
                quint8 *magenta = cyan + totalBytes;
                quint8 *yellow = magenta + totalBytes;
                quint8 *key = yellow + totalBytes;
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        *p = QColor::fromCmyk(255-*cyan, 255-*magenta,
                                              255-*yellow, 255-*key).rgb();
                        ++p; ++cyan; ++magenta; ++yellow; ++key;
                    }
                }
                *image = result;
            }
                break;
            case 5:
            {
                QImage result(width, height, QImage::Format_ARGB32);
                quint8 *alpha = (quint8*)decompressed.constData();
                quint8 *cyan = alpha + totalBytes;
                quint8 *magenta = cyan + totalBytes;
                quint8 *yellow = magenta + totalBytes;
                quint8 *key = yellow + totalBytes;
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        *p = QColor::fromCmyk(255-*cyan, 255-*magenta,
                                              255-*yellow, 255-*key,
                                              *alpha).rgba();
                        ++p; ++alpha; ++cyan; ++magenta; ++yellow; ++key;
                    }
                }
                *image = result;
            }
                break;
            }
        }
        break;
    case 7: /*MULTICHANNEL - UNIMPLEMENTED*/
        return 0;
        break;
    case 8: /*DUOTONE*/
        switch (depth) {
        case 8:
            switch (channels) {
            case 1:
                /*
                 *Duotone images: color data contains the duotone specification
                 *(the format of which is not documented). Other applications that
                 *read Photoshop files can treat a duotone image as a gray image,
                 *and just preserve the contents of the duotone information when
                 *reading and writing the file.
                 *
                 *TODO: find a way to actually get the duotone, tritone, and quadtone colors
                 *Noticed the added "Curve" layer when using photoshop
                 */
                QImage result(width, height, QImage::Format_Indexed8);
                const int IndexCount = 256;
                for(int i = 0; i < IndexCount; ++i){
                    result.setColor(i, qRgb(i, i, i));
                }
                quint8 *data = (quint8*)decompressed.constData();
                for(quint32 i=0; i < height; ++i)
                {
                    for(quint32 j=0; j < width; ++j)
                    {
                        result.setPixel(j,i, *data);
                        ++data;
                    }
                }
                *image = result;
                break;
            }

            break;
        }
        break;
    case 9: /*LAB - UNDER TESTING*/
        switch (depth) {
        case 8:
            switch (channels) {
            case 3:
            {
                QImage result(width, height, QImage::Format_RGB32);
                quint8 *lightness = (quint8*)decompressed.constData();
                quint8 *a = lightness + totalBytes;
                quint8 *b = a + totalBytes;

                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        *p = psd_lab_to_color(*lightness * 100 >> 8, *a - 128, *b - 128);
                        ++p; ++lightness; ++a; ++b;
                    }

                }
                *image = result;
                break;
            }
            case 4:
            {
                QImage result(width, height, QImage::Format_RGB32);
                quint8 *alpha = (quint8*)decompressed.constData();
                quint8 *lightness = alpha + totalBytes;
                quint8 *a = lightness + totalBytes;
                quint8 *b = a + totalBytes;

                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        *p = psd_alab_to_color(*alpha, *lightness * 100 >> 8, *a - 128, *b - 128);
                        ++p; ++alpha; ++lightness; ++a; ++b;
                    }

                }
                *image = result;
                break;
            }
            }
            break;
        case 16:
            qDebug("depth is 16, unsupported mode");
            return false;
        }
        break;
    }

    return input.status() == QDataStream::Ok;
}


bool QPSDHandler::supportsOption(ImageOption option) const
{
    return option == Size;
}

QVariant QPSDHandler::option(ImageOption option) const
{
    if (option == Size) {
        QByteArray bytes = device()->peek(26);
        QDataStream input(bytes);
        quint32 signature, height, width;
        quint16 version, channels, depth, colorMode;
        input.setByteOrder(QDataStream::BigEndian);
        input >> signature >> version ;
        input.skipRawData(6);//reserved bytes should be 6-byte in size
        input >> channels >> height >> width >> depth >> colorMode;
        if (input.status() == QDataStream::Ok && signature == 0x38425053 && version == 0x0001)
            return QSize(width, height);
    }
    return QVariant();
}
