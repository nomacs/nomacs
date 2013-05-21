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
/* For debugging purposes ONLY
#include <QDebug>
#include <QElapsedTimer>
*/

/* tristimulus reference: Adobe RGB (1998) Color Image Encoding
 * http://www.adobe.com/digitalimag/pdfs/AdobeRGB1998.pdf
 * D65 0.9505, 1.0000, 1.0891
 * D50 0.9642, 1.000, 0.8249
 */

const qreal ref_x = 96.05;
const qreal ref_y = 100.00;
const qreal ref_z = 108.91;
const qreal e = 216/24389;
const qreal k = 24389/27;
const qreal gamma = 563/256; //2.19921875

QRgb aXyzToRgba(quint8 alpha, qreal x, qreal y, qreal z)
{
    qreal var_x = x / 100.0;
    qreal var_y = y / 100.0;
    qreal var_z = z / 100.0;

    /* http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
     * Adobe RGB
     * D65
         2.0413690 -0.5649464 -0.3446944
        -0.9692660  1.8760108  0.0415560
         0.0134474 -0.1183897  1.0154096

     * D50
         1.9624274 -0.6105343 -0.3413404
        -0.9787684  1.9161415  0.0334540
         0.0286869 -0.1406752  1.3487655
    */

    qreal var_r = (var_x * 2.0413690) + (var_y * -0.5649464) + (var_z * -0.3446944);
    qreal var_g = (var_x * -0.9692660) + (var_y * 1.8760108) + (var_z * 0.0415560);
    qreal var_b = (var_x * 0.0134474) + (var_y * -0.1183897) + (var_z * 1.0154096);

    /* gamma companding */
    int red = pow(var_r, 1/gamma) * 255.0;
    int green = pow(var_g, 1/gamma) * 255.0;
    int blue = pow(var_b, 1/gamma) * 255.0;

    /* FIXME: there is a bug: red/green/blue sometimes fall outside the range 0-255
     * bug minimized but not totally solved and color is somewhat different
     * from what photoshop renders */

    red = (red < 0)?0:((red > 255)?255:red);
    green = (green < 0)?0:((green > 255)?255:green);
    blue = (blue < 0)?0:((blue > 255)?255:blue);

    /* FIXME: (for Lab color space with alpha channel)
     * I used photoshop to check if the image is viewed correctly
     * and found out that it is transparent at value = 255 so I assumed
     * they inverted it but there are still differences, the algorithm is
     * very closed and there might some kind of computation they used for
     * the alpha and I cannot find any documentation on it */
    alpha = 255 - alpha;
    return qRgba(red, green, blue, alpha);
}

QRgb XyzToRgb(qreal x, qreal y, qreal z)
{
    return aXyzToRgba(0, x, y, z);
}

QRgb aLabToRgb(quint8 alpha, qint32 L, qint32 a, qint32 b)
{
    /* ranges:
     * L* = 0 to 100
     * a = -128 to 127
     * b = -128 to 127
     */

    //L = L * 100 >> 8;
    L = L / 2.55;
    a -= 128;
    b -= 128;
    qreal x, y, z, var_y, var_x, var_z;
    qreal fy = ( L + 16.0 ) / 116.0;
    qreal fx = (a / 500.0) + fy;
    qreal fz = fy - (b / 200.0);

    if ( L > k*e )
        var_y = pow(fy, 3);
    else
        var_y = L/k;

    if ( pow(fx, 3) > e )
        var_x = pow(fx, 3);
    else
        var_x = ( (116.0*fx) - 16.0 ) / k;

    if ( pow(fz, 3) > e )
        var_z = pow(fz, 3);
    else
        var_z = ( (116.0*fz) - 16.0 ) / k;

    x = ref_x * var_x;
    y = ref_y * var_y;
    z = ref_z * var_z;

    return aXyzToRgba(alpha, x, y, z);
}

QRgb LabToRgb(qint32 L, qint32 a, qint32 b)
{
    return aLabToRgb(255, L, a, b);
}

QPsdHandler::QPsdHandler()
{
}

QPsdHandler::~QPsdHandler()
{
}

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
        else if (version == 1)
            setFormat("psb");
        else return false;
        return true;
    }
    return false;
}

bool QPsdHandler::canRead(QIODevice *device)
{
    return device->peek(4) == "8BPS";
}

bool QPsdHandler::read(QImage *image)
{
    QDataStream input(device());
    quint32 signature, height, width, colorModeDataLength, imageResourcesLength;
    quint16 version, channels, depth, colorMode, compression;
    QByteArray colorData;

    input.setByteOrder(QDataStream::BigEndian);

    /* checking for validity of the file/device are executed
     * after reading EACH "important" info and NOT after reading
     * ALL of them - I think this will save time and increase speed */
    input >> signature;
    if (signature != 0x38425053)
        return false;

    input >> version; //version should be 1(PSD) or 2(PSB)
    switch (version) {
    case 1:
    case 2:
        break;
    default: return false;
        break;
    }

    input.skipRawData(6); //reserved bytes should be 6-byte in size

    input >> channels; //Supported range is 1 to 56
    if (channels < 1 || channels > 56)
        return false;

    input >> height; //Supported range is 1 to 30,000. (**PSB** max of 300,000.)
    if (version == 1 && (height > 30000 || height == 0))
        return false;
    if (version == 2 && (height > 300000 || height == 0))
        return false;

    input >> width; //Supported range is 1 to 30,000. (**PSB** max of 300,000.)
    if (version == 1 && (width > 30000 || width == 0))
        return false;
    if (version == 2 && (width > 300000 || width == 0))
        return false;

    input >> depth; //Supported values are 1, 8, 16 and 32
    switch (depth) {
    case 1:
    case 8:
    case 16:
    case 32:
        break;
    default: return false;
        break;
    }

    /* The color mode of the file. Supported values are:
     * Bitmap = 0; Grayscale = 1; Indexed = 2; RGB = 3; CMYK = 4;
     * Multichannel = 7; Duotone = 8; Lab = 9 */
    input >> colorMode;
    switch (colorMode) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 7:
    case 8:
    case 9:
        break;
    default: return false;
        break;
    }

    input >> colorModeDataLength;
    if (colorModeDataLength != 0) {
        /* Alternative 1:
        quint8 byte;
        for(quint32 i=0; i<colorModeDataLength; ++i) {
            input >> byte;
            colorData.append(byte);
        }
        */
        /* Alternative 2:
        char *temp = new char[colorModeDataLength];
        input.readRawData(temp, colorModeDataLength);
        colorData.append(temp, colorModeDataLength);
        delete [] temp;
        */
        /* This code is faster than the 2 alternatives above */
        colorData.resize(colorModeDataLength);
        input.readRawData(colorData.data(), colorModeDataLength);
    }

    input >> imageResourcesLength;
    input.skipRawData(imageResourcesLength);

    /* The size of Layer and Mask Section is 4 bytes for PSD files
     * and 8 bytes for PSB files */
    if (format() == "psd") {
        quint32 layerAndMaskInfoLength;
        input >> layerAndMaskInfoLength;
        input.skipRawData(layerAndMaskInfoLength);
    } else if (format() == "psb") {
        quint64 layerAndMaskInfoLength;
        input >> layerAndMaskInfoLength;
        input.skipRawData(layerAndMaskInfoLength);
    }

    input >> compression;

    if (input.status() != QDataStream::Ok)
        return false;

    //QElapsedTimer timer;
    //timer.start();
    QByteArray imageData;
    switch (compression) {
    case 0: /*RAW IMAGE DATA - UNDER TESTING*/
    {
        /* NOTE: This algorithm might be inaccurate and was based ONLY
         * on a psd file I obtained (no references) */

        /* This code is faster than the alternative below */
        int size = width * height * channels * (depth/8);
        if (colorMode == 0)
            size /= 8;
        imageData.resize(size);
        input.readRawData(imageData.data(), size);
        /* Alternative:
        quint8 byte;
        while (!input.atEnd()) {
            input >> byte;
            imageData.append(byte);
        }*/
    }
        break;
    case 1: /*RLE COMPRESSED DATA*/
    {
        /* The RLE-compressed data is proceeded by a 2-byte data count for each row in the data,
         * which we're going to just skip. */
        if (format() == "psd")
            input.skipRawData(height*channels*2);
        /* This section was NOT documented, but because of too much use of
         * qDebug() + caffeine, I solved it using the verification section
         * after this switch statement :)
         * Found out that the resulting image data was NOT of correct size */
        else if (format() == "psb")
            input.skipRawData(height*channels*4);

        quint8 byte,count;

        /* Code based on PackBits implementation which is primarily used by
         * Photoshop for RLE encoding/decoding */

        while (!input.atEnd()) {
            input >> byte;
            if (byte > 128) {
                count = 256 - byte ;
                input >>  byte;
                /* This code is still faster than the
                 * alternative below */
                for (quint8 i=0; i<=count; ++i) {
                    imageData.append(byte);
                }
                /* Alternative:
                ++count;
                imageData.append(QByteArray(count, char(byte)));
                */
            } else if (byte < 128) {
                count = byte + 1;
                /* Alternative 1:
                for(quint8 i=0; i<count; ++i) {
                    input >> byte;
                    imageData.append(byte);
                }
                */
                /* Alternative 2:
                char *temp = new char[count];
                input.readRawData(temp, count);
                imageData.append(temp, count);
                delete [] temp;
                */
                /* This code is faster than the 2 alternatives above */
                int size = imageData.size();
                imageData.resize(size+count);
                input.readRawData(imageData.data()+size, count);
            }
        }
    }
        break;
    case 2:/*ZIP WITHOUT PREDICTION - UNIMPLEMENTED*/
        break;
    case 3:/*ZIP WITH PREDICTION - UNIMPLEMENTED*/
        break;
    }
    //qDebug() << timer.nsecsElapsed();

    if (input.status() != QDataStream::Ok)
        return false;

    int totalBytes = width * height;

    /* this section was made for verification
     * for developers use ONLY */
    /*
    qDebug() << endl
             <<"color mode: " << colorMode
             << "\ndepth: " << depth
             << "\nchannels: " << channels
             << "\ncompression: " << compression
             << "\nwidth: " << width
             << "\nheight: " << height
             << "\ntotalBytes: " << totalBytes
             << "\nimage data: " << imageData.size();
    */

    switch (colorMode) {
    case 0: /*BITMAP*/
    {
        if (imageData.size() != (channels * totalBytes)/8)
            return false;

        QString head = QString("P4\n%1 %2\n").arg(width).arg(height);
        QByteArray buffer(head.toUtf8());
        buffer.append(imageData);
        QImage result = QImage::fromData(buffer);
        if (result.isNull())
            return false;
        else {
            *image = result;
            return true;
        }
    }
        break;
    case 1: /*GRAYSCALE*/
    {
        if (imageData.size() != channels * totalBytes)
            return false;

        switch (depth) {
        case 8:
            switch (channels) {
            case 1:
            {
                /*
                QImage result(width, height, QImage::Format_Indexed8);
                for (int i = 0; i < 256; ++i){
                    result.setColor(i, qRgb(i, i, i));
                }
                */
                QImage result(width, height, QImage::Format_RGB32);
                quint8 *data = (quint8*)imageData.constData();
                for (quint32 i=0; i < height; ++i) {
                    for (quint32 j=0; j < width; ++j) {
                        //result.setPixel(j, i, *data);
                        result.setPixel(j, i, qRgb(*data, *data, *data));
                        ++data;
                    }
                }
                *image = result;
            }
                break;
            case 2: /* graycale with alpha channel */
            {
                QImage result(width, height, QImage::Format_ARGB32);

                quint8 *data = (quint8*)imageData.constData();
                quint8 *alpha = data + totalBytes;
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        *p = qRgba(*data, *data, *data, *alpha);
                        ++p; ++data; ++alpha;
                    }
                }
                *image = result;
            }
                break;
            default:
                return false;
                break;
            }
            break;
        default:
            return false;
            break;
        }
    }
        break;
    case 2: /*INDEXED*/
    {
        if (imageData.size() != channels * totalBytes)
            return false;

        switch (depth) {
        case 8:
            switch (channels) {
            case 1:
            {
                QImage result(width, height, QImage::Format_Indexed8);
                int indexCount = colorData.size() / 3;
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

                quint8 *data = (quint8*)imageData.constData();
                for (quint32 i=0; i < height; ++i) {
                    for (quint32 j=0; j < width; ++j) {
                        result.setPixel(j,i,*data);
                        ++data;
                    }
                }
                *image=result;
            }
                break;
            default:
                return false;
                break;
            }
            break;
        default:
            return false;
            break;
        }
    }
        break;
    case 3: /*RGB*/
    {
        if (imageData.size() != channels * totalBytes * (depth/8))
            return false;

        switch (depth) {
        case 8:
            switch (channels) {
            case 3:
            {
                QImage result(width, height, QImage::Format_RGB32);
                quint8 *red = (quint8*)imageData.constData();
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
                quint8 *red = (quint8*)imageData.constData();
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
            default:
                return false;
                break;
            }
            break;
        case 16:
            switch (channels) {
            case 3:
            {
                QImage result(width, height, QImage::Format_RGB32);
                quint16 red16, blue16, green16;
                quint8 *red8 = (quint8*)imageData.constData();
                quint8 *green8 = red8 + totalBytes * 2;
                quint8 *blue8 = green8 + totalBytes * 2;
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        /* FIXME: find a cleaner, shorter method if possible
                         * steps:
                         * get high byte then left shift by 8 bits (color16 = *byte << 8)
                         * move pointers (++byte)
                         * get low byte then add it to high byte (color16 += *byte)
                         * convert 16-bit depth color to 8-bit depth color
                         *            by scaling it (color16 * 255 / 65535)
                         * move pointers (++byte)
                         */
                        red16 = *red8 << 8;
                        green16 = *green8 << 8;
                        blue16 = *blue8 << 8;
                        ++red8; ++green8; ++blue8;
                        red16 += *red8;
                        green16 += *green8;
                        blue16 += *blue8;
                        *p = qRgb(quint8(red16 * 255 / 65535),
                                   quint8(green16 * 255 / 65535),
                                   quint8(blue16 * 255 / 65535));
                        ++p; ++red8; ++green8; ++blue8;
                    }
                }
                    *image = result;
            }
                break;
            case 4:
            {
                QImage result(width, height, QImage::Format_ARGB32);
                quint16 red16, blue16, green16, alpha16;
                quint8 *red8 = (quint8*)imageData.constData();
                quint8 *green8 = red8 + totalBytes * 2;
                quint8 *blue8 = green8 + totalBytes * 2;
                quint8 *alpha8 = blue8 + totalBytes * 2;
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        /* FIXME: find a cleaner, shorter method if possible
                         * steps:
                         * get high byte then left shift by 8 bits (color16 = *byte << 8)
                         * move pointers (++byte)
                         * get low byte then add it to high byte (color16 += *byte)
                         * convert 16-bit depth color to 8-bit depth color
                         *            by scaling it (color16 * 255 / 65535)
                         * move pointers (++byte)
                         */
                        red16 = *red8 << 8;
                        green16 = *green8 << 8;
                        blue16 = *blue8 << 8;
                        alpha16 = *alpha8 << 8;
                        ++red8; ++green8; ++blue8; ++alpha8;
                        red16 += *red8;
                        green16 += *green8;
                        blue16 += *blue8;
                        alpha16 += *alpha8;
                        *p = qRgba(quint8(red16 * 255 / 65535),
                                   quint8(green16 * 255 / 65535),
                                   quint8(blue16 * 255 / 65535),
                                   quint8(alpha16 * 255 / 65535));
                        ++p; ++red8; ++green8; ++blue8; ++alpha8;
                    }
                }

                *image = result;
            }
                break;
            default:
                return false;
                break;
            }
            break;
        /* Removed temporarily... incorrect
        case 32:
            switch (channels) {
            case 3:
            {
                QImage result(width, height, QImage::Format_RGB32);
                quint32 red32 = 0;
                quint32 blue32 = 0;
                quint32 green32 = 0;
                quint8 *red8 = (quint8*)imageData.constData();
                quint8 *green8 = red8 + totalBytes * 4;
                quint8 *blue8 = green8 + totalBytes * 4;
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        for (int i = 0; i < 3; ++i) {
                            red32 += *red8;
                            green32 += *green8;
                            blue32 += *blue8;
                            red32 <<= 8;
                            green32 <<= 8;
                            blue32 <<= 8;
                            ++red8; ++green8; ++blue8;
                        }
                        *p = qRgb(quint8(red32 * 255 / 4294967295),
                                  quint8(green32 * 255 / 4294967295),
                                  quint8(blue32 * 255 / 4294967295));
                        ++p;
                    }
                }
                    *image = result;
            }
                break;
            case 4:
            {
                QImage result(width, height, QImage::Format_ARGB32);
                quint32 red32 = 0;
                quint32 blue32 = 0;
                quint32 green32 = 0;
                quint32 alpha32 = 0;
                quint8 *red8 = (quint8*)imageData.constData();
                quint8 *green8 = red8 + totalBytes * 4;
                quint8 *blue8 = green8 + totalBytes * 4;
                quint8 *alpha8 = blue8 + totalBytes * 4;
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        //4,294,967,295
                        for (int i = 0; i < 3; ++i) {
                            red32 += *red8;
                            green32 += *green8;
                            blue32 += *blue8;
                            alpha32 += *alpha8;
                            red32 <<= 8;
                            green32 <<= 8;
                            blue32 <<= 8;
                            alpha32 <<= 8;
                            ++red8; ++green8; ++blue8; ++alpha8;
                        }
                        *p = qRgba(quint8(red32 * 255 / 4294967295),
                                   quint8(green32 * 255 / 4294967295),
                                   quint8(blue32 * 255 / 4294967295),
                                   quint8(alpha32 * 255 / 4294967295));
                        ++p;
                    }
                }
                    *image = result;
            }
                break;
            default:
                return false;
                break;
            }
            break;*/
        default:
            break;
        }
    }
        break;
    case 4: /*CMYK*/
    {
        if (imageData.size() != channels * totalBytes)
            return false;

        switch (depth) {
        case 8:
            switch (channels) {
            case 4:
            {
                QImage result(width, height, QImage::Format_RGB32);
                quint8 *cyan = (quint8*)imageData.constData();
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
                quint8 *cyan = (quint8*)imageData.constData();
                quint8 *magenta = cyan + totalBytes;
                quint8 *yellow = magenta + totalBytes;
                quint8 *key = yellow + totalBytes;
                quint8 *alpha = key + totalBytes;
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
            default:
                return false;
                break;
            }
            break;
        default:
            return false;
            break;
        }
    }
        break;
    case 7: /*MULTICHANNEL - UNDER TESTING*/
    {
        if (imageData.size() != channels * totalBytes)
            return false;

        /* Reference: http://help.adobe.com/en_US/photoshop/cs/using/WSfd1234e1c4b69f30ea53e41001031ab64-73eea.html#WSfd1234e1c4b69f30ea53e41001031ab64-73e5a
         * Converting a CMYK image to Multichannel mode creates cyan, magenta, yellow, and black spot channels.
         * Converting an RGB image to Multichannel mode creates cyan, magenta, and yellow spot channels.
         */
        switch (depth) {
        case 8:
            switch (channels) {
            case 3:
            {
                QImage result(width, height, QImage::Format_RGB32);
                quint8 *cyan = (quint8*)imageData.constData();
                quint8 *magenta = cyan + totalBytes;
                quint8 *yellow = magenta + totalBytes;
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        *p = QColor::fromCmyk(255-*cyan, 255-*magenta,
                                              255-*yellow, 0).rgb();
                        ++p; ++cyan; ++magenta; ++yellow;;
                    }
                }
                *image = result;
            }
                break;
            case 4:
            {
                QImage result(width, height, QImage::Format_RGB32);
                quint8 *cyan = (quint8*)imageData.constData();
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
            default:
                return false;
                break;
            }
            break;
        default:
            return false;
            break;
        }
    }
        break;
    case 8: /*DUOTONE*/
    {
        if (imageData.size() != channels * totalBytes)
            return false;

        switch (depth) {
        case 8:
            switch (channels) {
            case 1:
            {
                /*
                 *Duotone images: color data contains the duotone specification
                 *(the format of which is not documented). Other applications that
                 *read Photoshop files can treat a duotone image as a gray image,
                 *and just preserve the contents of the duotone information when
                 *reading and writing the file.
                 *
                 *TODO: find a way to actually get the duotone, tritone, and quadtone colors
                 */
                QImage result(width, height, QImage::Format_Indexed8);
                for(int i = 0; i < 256; ++i){
                    result.setColor(i, qRgb(i, i, i));
                }
                quint8 *data = (quint8*)imageData.constData();
                for(quint32 i=0; i < height; ++i)
                {
                    for(quint32 j=0; j < width; ++j)
                    {
                        result.setPixel(j,i, *data);
                        ++data;
                    }
                }
                *image = result;
            }
                break;
            default:
                return false;
                break;
            }
            break;
        default:
            return false;
            break;
        }
    }
        break;
    case 9: /*LAB - UNDER TESTING*/
    {
        if (imageData.size() != channels * totalBytes)
            return false;

        /* FIXME: computation from Lab color mode to RGB has some minor bug
         * which results to pixels different from the correct conversion */
        switch (depth) {
        case 8:
            switch (channels) {
            case 3:
            {
                QImage result(width, height, QImage::Format_RGB32);
                quint8 *L = (quint8*)imageData.constData();
                quint8 *a = L + totalBytes;
                quint8 *b = a + totalBytes;

                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        *p = LabToRgb(*L, *a, *b);
                        ++p; ++L; ++a; ++b;
                    }
                }
                *image = result;
            }
                break;
            case 4:
            {
                QImage result(width, height, QImage::Format_ARGB32);
                quint8 *alpha = (quint8*)imageData.constData();
                quint8 *L = alpha + totalBytes;
                quint8 *a = L + totalBytes;
                quint8 *b = a + totalBytes;

                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        *p = aLabToRgb(*alpha, *L, *a, *b);
                        ++p; ++alpha; ++L; ++a; ++b;
                    }

                }
                *image = result;
            }
                break;
            default:
                return false;
                break;
            }
            break;
        default:
            return false;
            break;
        }
    }
        break;
    default: /*NOT DOCUMENTED*/
    {
        return false;
    }
        break;
    }
    return input.status() == QDataStream::Ok;
}


bool QPsdHandler::supportsOption(ImageOption option) const
{
    return option == Size;
}

QVariant QPsdHandler::option(ImageOption option) const
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
        if (input.status() == QDataStream::Ok && signature == 0x38425053 &&
                (version == 1 || version == 2))
            return QSize(width, height);
    }
    return QVariant();
}
