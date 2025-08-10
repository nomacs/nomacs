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
#endif

/* tristimulus reference: Adobe RGB (1998) Color Image Encoding
 * http://www.adobe.com/digitalimag/pdfs/AdobeRGB1998.pdf
 * D65 0.9505, 1.0000, 1.0891
 * D50 0.9642, 1.000, 0.8249
 */

static const qreal refX = 0.9642;
static const qreal refY = 1.0000;
static const qreal refZ = 0.8249;
static const qreal e = 216 / 24389;
static const qreal k = 24389 / 27;
static const qreal gammaPsd = 563 / 256; // 2.19921875

static QRgb xyzToRgb(qreal x, qreal y, qreal z, qreal alpha = 1.0)
{
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
    /* D65 */
    /* qreal varR = (x * 2.0413690) + (y * -0.5649464) + (z * -0.3446944);
     * qreal varG = (x * -0.9692660) + (y * 1.8760108) + (z * 0.0415560);
     * qreal varB = (x * 0.0134474) + (y * -0.1183897) + (z * 1.0154096);
     */

    /* D50 */
    qreal varR = (x * 1.9624274) + (y * -0.6105343) + (z * -0.3413404);
    qreal varG = (x * -0.9787684) + (y * 1.9161415) + (z * 0.0334540);
    qreal varB = (x * 0.0286869) + (y * -0.1406752) + (z * 1.3487655);

    /* gamma companding */
    qreal red = qPow(varR, 1 / gammaPsd);
    qreal green = qPow(varG, 1 / gammaPsd);
    qreal blue = qPow(varB, 1 / gammaPsd);

    /* values sometimes fall outside the range 0-1 */
    red = (red < 0) ? 0 : ((red > 1) ? 1 : red);
    green = (green < 0) ? 0 : ((green > 1) ? 1 : green);
    blue = (blue < 0) ? 0 : ((blue > 1) ? 1 : blue);

    /* QColor::fromRgbF accepts value from 0-1, therefore, there's
     * no need to multiply it by 255 and use qRgb()
     */
    return QColor::fromRgbF(red, green, blue, alpha).rgba();
}

static QRgb labToRgb(qreal lightness, qreal a, qreal b, quint8 alpha = 255)
{
    /* ranges:
     * lightness = 0 to 100
     * a = -128 to 127
     * b = -128 to 127
     */

    lightness /= 2.55;
    a -= 128;
    b -= 128;
    qreal fy = (lightness + 16.0) / 116.0;
    qreal fx = (a / 500.0) + fy;
    qreal fz = fy - (b / 200.0);

    qreal varY = (lightness > k * e) ? pow(fy, 3) : lightness / k;
    qreal varX = (pow(fx, 3) > e) ? pow(fx, 3) : ((116.0 * fx) - 16.0) / k;
    qreal varZ = (pow(fz, 3) > e) ? pow(fz, 3) : ((116.0 * fz) - 16.0) / k;

    return xyzToRgb(refX * varX, refY * varY, refZ * varZ, alpha / 255);
}

/**
 * @brief QPsdHandler::isValidSignature Validates if signature is 8BPS.
 * @param signature Signature.
 * @return bool.
 */
bool QPsdHandler::isValidSignature(quint32 signature)
{
    return signature == 0x38425053; //'8BPS'
}

/**
 * @brief QPsdHandler::isValidVersion Validates if version is 1(PSD) or 2(PSB).
 * @param version Version.
 * @return bool.
 */
bool QPsdHandler::isValidVersion(quint16 version)
{
    switch (version) {
    case 1:
        // check if format is empty or it is not psd
        if (format().isEmpty() || format() != "psd")
            setFormat("psd");
        return true;
        break;
    case 2:
        // check if format is empty or it is not psb
        if (format().isEmpty() || format() != "psb")
            setFormat("psb");
        return true;
        break;
    default:
        return false;
        break;
    }
}

/**
 * @brief QPsdHandler::isChannelCountSupported Validates if the number of channels is supported.
 *      Supported range is 1 to 56.
 * @param channels Number of channels.
 * @return bool.
 */
bool QPsdHandler::isChannelCountSupported(quint16 channels)
{
    //    return channels >= 1 && channels <= 56;
    //    NOTE: found a sample file with channels > 56 and Photoshop can still read it
    //    though the documentation says it should be within 1 to 56 channels
    return channels >= 1; // breaking "56-max rule"
}

/**
 * @brief QPsdHandler::isValidWidthOrHeight Validates if width or height is within valid range.
 *      Supported range is 1 to 30,000 for PSD and 1 to 300,000 for PSB.
 * @param version Version.
 * @param value Value of width or height.
 * @return bool.
 */
bool QPsdHandler::isValidWidthOrHeight(quint16 version, quint32 value)
{
    if (version == 1 && (value > 30000 || value == 0))
        return false;
    if (version == 2 && (value > 300000 || value == 0))
        return false;
    return true;
}

/**
 * @brief QPsdHandler::isSupportedDepth Validates if depth is supported.
 * Supported values are 1, 8, 16 and 32.
 * @param depth Depth.
 * @return bool.
 */
bool QPsdHandler::isSupportedDepth(quint16 depth)
{
    switch (depth) {
    case 1:
    case 8:
    case 16:
    case 32:
        return true;
        break;
    default:
        return false;
        break;
    }
}

/**
 * @brief QPsdHandler::isSupportedColorMode Validates if color mode is supported.
 *      Supported values are Bitmap = 0; Grayscale = 1; Indexed = 2; RGB = 3; CMYK = 4;
 *      Multichannel = 7; Duotone = 8; Lab = 9
 * @param colorMode
 * @return
 */
bool QPsdHandler::isSupportedColorMode(quint16 colorMode)
{
    switch (colorMode) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 7:
    case 8:
    case 9:
        return true;
        break;
    default:
        return false;
        break;
    }
}

/**
 * @brief QPsdHandler::getColorData Extracts the color data from the input stream.
 * @param input QDataStream.
 * @return bool.
 */
QByteArray QPsdHandler::readColorData(QDataStream &input)
{
    quint32 length;
    QByteArray colorData;
    input >> length;
    if (length != 0) {
        colorData.resize(length);
        input.readRawData(colorData.data(), length);
    }
    return colorData;
}

/**
 * @brief QPsdHandler::skipImageResources Skips the image resources section.
 * @param input QDataStream.
 */
void QPsdHandler::skipImageResources(QDataStream &input)
{
    quint32 length;
    input >> length;
    input.skipRawData(length);
}

/**
 * @brief QPsdHandler::skipLayerAndMaskSection Skips the layer and mask section.
 *      The size of Layer and Mask Section is stored in 4 bytes for PSD files
 *      and 8 bytes for PSB files.
 * @param input QDataStream.
 */
void QPsdHandler::skipLayerAndMaskSection(QDataStream &input)
{
    if (format() == "psd") {
        quint32 layerAndMaskInfoLength;
        input >> layerAndMaskInfoLength;
        input.skipRawData(layerAndMaskInfoLength);
    } else if (format() == "psb") {
        quint64 layerAndMaskInfoLength;
        input >> layerAndMaskInfoLength;
        input.skipRawData(layerAndMaskInfoLength);
    }
}

QByteArray QPsdHandler::readImageData(QDataStream &input, QPsdHandler::Compression compression, quint64 size)
{
    QByteArray imageData;
    switch (compression) {
    case RAW: {
        imageData.resize(size);
        input.readRawData(imageData.data(), size);
    } break;
    case RLE: /*RLE COMPRESSED DATA*/
    {
        quint8 byte, count;
        /* Code based on PackBits implementation which is primarily used by
         * Photoshop for RLE encoding/decoding */
        while (!input.atEnd()) {
            input >> byte;
            if (byte > 128) {
                count = 256 - byte;
                input >> byte;
                for (quint8 i = 0; i <= count; ++i) {
                    imageData.append(byte);
                }
            } else if (byte < 128) {
                count = byte + 1;
                qsizetype sz = imageData.size();
                imageData.resize(sz + count);
                input.readRawData(imageData.data() + sz, count);
            }
        }
    } break;
    case ZIP_WITHOUT_PREDICTION:
        /*NOT IMPLEMENTED*/
        break;
    case ZIP_WITH_PREDICTION:
        /*NOT IMPLEMENTED*/
        break;
    }
    return imageData;
}

/**
 * @brief QPsdHandler::processBitmap Generates bitmap image from imageData.
 * @param imageData QByteArray.
 * @param width quint32.
 * @param height quint32.
 * @return QImage.
 */
QImage QPsdHandler::processBitmap(QByteArray &imageData, quint32 width, quint32 height)
{
#ifdef QT_DEBUG
    qDebug() << "Bitmap";
#endif
    QString head = QString("P4\n%1 %2\n").arg(width).arg(height);
    QByteArray buffer(head.toUtf8());
    buffer.append(imageData);
    return QImage::fromData(buffer);
}

QImage QPsdHandler::processGrayscale8(QByteArray &imageData, quint32 width, quint32 height)
{
#ifdef QT_DEBUG
    qDebug() << "8-bit Grayscale";
#endif
    QImage result(width, height, QImage::Format_RGB32);
    const auto *data = (const quint8 *)imageData.constData();
    QRgb *p, *end;
    for (quint32 y = 0; y < height; ++y) {
        p = (QRgb *)result.scanLine(y);
        end = p + width;
        while (p < end) {
            *p = qRgb(*data, *data, *data);
            ++p;
            ++data;
        }
    }
    return result;
}

QImage QPsdHandler::processGrayscale8WithAlpha(QByteArray &imageData,
                                               quint32 width,
                                               quint32 height,
                                               quint64 totalBytesPerChannel)
{
#ifdef QT_DEBUG
    qDebug() << "8-bit Grayscale with Alpha";
#endif
    QImage result(width, height, QImage::Format_ARGB32);
    const auto *data = (const quint8 *)imageData.constData();
    const quint8 *alpha = data + totalBytesPerChannel;
    QRgb *p, *end;
    for (quint32 y = 0; y < height; ++y) {
        p = (QRgb *)result.scanLine(y);
        end = p + width;
        while (p < end) {
            *p = qRgba(*data, *data, *data, *alpha);
            ++p;
            ++data;
            ++alpha;
        }
    }
    return result;
}

QImage QPsdHandler::processGrayscale16(QByteArray &imageData, quint32 width, quint32 height)
{
#ifdef QT_DEBUG
    qDebug() << "16-bit Grayscale";
#endif
    const qreal scale = (qPow(2, 8) - 1) / (qPow(2, 16) - 1);
    quint16 data16;
    QImage result(width, height, QImage::Format_RGB32);
    const auto *data8 = (const quint8 *)imageData.constData();
    QRgb *p, *end;
    for (quint32 y = 0; y < height; ++y) {
        p = (QRgb *)result.scanLine(y);
        end = p + width;
        while (p < end) {
            data16 = ((*data8 << 8) + *(data8 + 1)) * scale;
            *p = qRgb(data16, data16, data16);
            ++p;
            data8 += 2;
        }
    }
    return result;
}

QImage QPsdHandler::processGrayscale16WithAlpha(QByteArray &imageData,
                                                quint32 width,
                                                quint32 height,
                                                quint64 totalBytesPerChannel)
{
#ifdef QT_DEBUG
    qDebug() << "8-bit Grayscale with Alpha";
#endif
    const qreal scale = (qPow(2, 8) - 1) / (qPow(2, 16) - 1);
    quint16 data16, alpha16;
    QImage result(width, height, QImage::Format_ARGB32);
    const auto *data8 = (const quint8 *)imageData.constData();
    const quint8 *alpha8 = data8 + totalBytesPerChannel;
    QRgb *p, *end;
    for (quint32 y = 0; y < height; ++y) {
        p = (QRgb *)result.scanLine(y);
        end = p + width;
        while (p < end) {
            data16 = ((*data8 << 8) + *(data8 + 1)) * scale;
            alpha16 = ((*alpha8 << 8) + *(alpha8 + 1)) * scale;
            *p = qRgba(data16, data16, data16, alpha16);
            ++p;
            data8 += 2;
            alpha8 += 2;
        }
    }
    return result;
}

QImage QPsdHandler::processIndexed(QByteArray &colorData, QByteArray &imageData, quint32 width, quint32 height)
{
#ifdef QT_DEBUG
    qDebug() << "Indexed";
#endif
    QImage result(width, height, QImage::Format_Indexed8);
    int indexCount = colorData.size() / 3;
    const auto *red = (const quint8 *)colorData.constData();
    const quint8 *green = red + indexCount;
    const quint8 *blue = green + indexCount;
    for (int i = 0; i < indexCount; ++i) {
        result.setColor(i, qRgb(*red, *green, *blue));
        ++red;
        ++green;
        ++blue;
    }
    const auto *data = (const quint8 *)imageData.constData();
    for (quint32 i = 0; i < height; ++i) {
        for (quint32 j = 0; j < width; ++j) {
            result.setPixel(j, i, *data);
            ++data;
        }
    }
    return result;
}

QImage QPsdHandler::processRGB8(QByteArray &imageData, quint32 width, quint32 height, quint64 totalBytesPerChannel)
{
#ifdef QT_DEBUG
    qDebug() << "8-bit RGB";
#endif
    QImage result(width, height, QImage::Format_RGB32);
    const auto *red = (const quint8 *)imageData.constData();
    const quint8 *green = red + totalBytesPerChannel;
    const quint8 *blue = green + totalBytesPerChannel;
    QRgb *p, *end;
    for (quint32 y = 0; y < height; ++y) {
        p = (QRgb *)result.scanLine(y);
        end = p + width;
        while (p < end) {
            *p = qRgb(*red, *green, *blue);
            ++p;
            ++red;
            ++green;
            ++blue;
        }
    }
    return result;
}

QImage QPsdHandler::processRGB16(QByteArray &imageData, quint32 width, quint32 height, quint64 totalBytesPerChannel)
{
#ifdef QT_DEBUG
    qDebug() << "16-bit RGB";
#endif
    const qreal scale = (qPow(2, 8) - 1) / (qPow(2, 16) - 1);
    QImage result(width, height, QImage::Format_RGB32);
    quint16 red16, blue16, green16;
    const auto *red8 = (const quint8 *)imageData.constData();
    const quint8 *green8 = red8 + totalBytesPerChannel;
    const quint8 *blue8 = green8 + totalBytesPerChannel;
    QRgb *p, *end;
    for (quint32 y = 0; y < height; ++y) {
        p = (QRgb *)result.scanLine(y);
        end = p + width;
        while (p < end) {
            red16 = ((*red8 << 8) + *(red8 + 1)) * scale;
            green16 = ((*green8 << 8) + *(green8 + 1)) * scale;
            blue16 = ((*blue8 << 8) + *(blue8 + 1)) * scale;
            *p = qRgb(red16, green16, blue16);
            ++p;
            red8 += 2;
            green8 += 2;
            blue8 += 2;
        }
    }
    return result;
}

QImage QPsdHandler::processRGB8WithAlpha(QByteArray &imageData,
                                         quint32 width,
                                         quint32 height,
                                         quint64 totalBytesPerChannel)
{
#ifdef QT_DEBUG
    qDebug() << "8-bit RGB with Alpha";
#endif
    QImage result(width, height, QImage::Format_ARGB32);
    const auto *red = (const quint8 *)imageData.constData();
    const quint8 *green = red + totalBytesPerChannel;
    const quint8 *blue = green + totalBytesPerChannel;
    const quint8 *alpha = blue + totalBytesPerChannel;
    QRgb *p, *end;
    for (quint32 y = 0; y < height; ++y) {
        p = (QRgb *)result.scanLine(y);
        end = p + width;
        while (p < end) {
            // Fix for blending image with white
            if (*alpha != 0) {
                quint8 r = *red;
                quint8 g = *green;
                quint8 b = *blue;
                quint8 a = *alpha;

                quint8 rFixed = (((r + a) - 255) * 255) / a;
                quint8 gFixed = (((g + a) - 255) * 255) / a;
                quint8 bFixed = (((b + a) - 255) * 255) / a;

                *p = qRgba(rFixed, gFixed, bFixed, a);
            } else {
                *p = qRgba(*red, *green, *blue, *alpha);
            }
            ++p;
            ++red;
            ++green;
            ++blue;
            ++alpha;
        }
    }
    return result;
}

QImage QPsdHandler::processRGB16WithAlpha(QByteArray &imageData,
                                          quint32 width,
                                          quint32 height,
                                          quint64 totalBytesPerChannel)
{
#ifdef QT_DEBUG
    qDebug() << "16-bit RGB with Alpha";
#endif
    //    FIXME: blending image with white (see QPsdHandler::processRGB8WithAlpha)
    const qreal scale = (qPow(2, 8) - 1) / (qPow(2, 16) - 1);
    QImage result(width, height, QImage::Format_ARGB32);
    quint16 red16, blue16, green16, alpha16;
    const auto *red8 = (const quint8 *)imageData.constData();
    const quint8 *green8 = red8 + totalBytesPerChannel;
    const quint8 *blue8 = green8 + totalBytesPerChannel;
    const quint8 *alpha8 = blue8 + totalBytesPerChannel;
    QRgb *p, *end;
    for (quint32 y = 0; y < height; ++y) {
        p = (QRgb *)result.scanLine(y);
        end = p + width;
        while (p < end) {
            red16 = ((*red8 << 8) + *(red8 + 1)) * scale;
            green16 = ((*green8 << 8) + *(green8 + 1)) * scale;
            blue16 = ((*blue8 << 8) + *(blue8 + 1)) * scale;
            alpha16 = ((*alpha8 << 8) + *(alpha8 + 1)) * scale;
            *p = qRgba((quint8)red16, (quint8)green16, (quint8)blue16, (quint8)alpha16);
            ++p;
            red8 += 2;
            green8 += 2;
            blue8 += 2;
            alpha8 += 2;
        }
    }
    return result;
}

QImage QPsdHandler::processCMY8(QByteArray &imageData, quint32 width, quint32 height, quint64 totalBytesPerChannel)
{
#ifdef QT_DEBUG
    qDebug() << "8-bit CMY";
#endif
    QImage result(width, height, QImage::Format_RGB32);
    const auto *cyan = (const quint8 *)imageData.constData();
    const quint8 *magenta = cyan + totalBytesPerChannel;
    const quint8 *yellow = magenta + totalBytesPerChannel;
    QRgb *p, *end;
    for (quint32 y = 0; y < height; ++y) {
        p = (QRgb *)result.scanLine(y);
        end = p + width;
        while (p < end) {
            *p = QColor::fromCmyk(255 - *cyan, 255 - *magenta, 255 - *yellow, 0).rgba();
            ++p;
            ++cyan;
            ++magenta;
            ++yellow;
            ;
        }
    }
    return result;
}

QImage QPsdHandler::processCMYK8(QByteArray &imageData, quint32 width, quint32 height, quint64 totalBytesPerChannel)
{
#ifdef QT_DEBUG
    qDebug() << "8-bit CMYK";
#endif
    QImage result(width, height, QImage::Format_RGB32);
    const auto *cyan = (const quint8 *)imageData.constData();
    const quint8 *magenta = cyan + totalBytesPerChannel;
    const quint8 *yellow = magenta + totalBytesPerChannel;
    const quint8 *key = yellow + totalBytesPerChannel;
    QRgb *p, *end;
    for (quint32 y = 0; y < height; ++y) {
        p = (QRgb *)result.scanLine(y);
        end = p + width;
        while (p < end) {
            *p = QColor::fromCmyk(255 - *cyan, 255 - *magenta, 255 - *yellow, 255 - *key).rgba();
            ++p;
            ++cyan;
            ++magenta;
            ++yellow;
            ++key;
        }
    }
    return result;
}

QImage QPsdHandler::processCMYK8WithAlpha(QByteArray &imageData,
                                          quint32 width,
                                          quint32 height,
                                          quint64 totalBytesPerChannel)
{
#ifdef QT_DEBUG
    qDebug() << "8-bit CMYK with Alpha";
#endif
    QImage result(width, height, QImage::Format_ARGB32);
    const auto *cyan = (const quint8 *)imageData.constData();
    const quint8 *magenta = cyan + totalBytesPerChannel;
    const quint8 *yellow = magenta + totalBytesPerChannel;
    const quint8 *key = yellow + totalBytesPerChannel;
    const quint8 *alpha = key + totalBytesPerChannel;
    QRgb *p, *end;
    for (quint32 y = 0; y < height; ++y) {
        p = (QRgb *)result.scanLine(y);
        end = p + width;
        while (p < end) {
            *p = QColor::fromCmyk(255 - *cyan, 255 - *magenta, 255 - *yellow, 255 - *key, *alpha).rgba();
            ++p;
            ++alpha;
            ++cyan;
            ++magenta;
            ++yellow;
            ++key;
        }
    }
    return result;
}

QImage QPsdHandler::processCMYK16(QByteArray &imageData, quint32 width, quint32 height, quint64 totalBytesPerChannel)
{
#ifdef QT_DEBUG
    qDebug() << "16-bit CMYK";
#endif
    const qreal scale = (qPow(2, 8) - 1) / (qPow(2, 16) - 1);
    QImage result(width, height, QImage::Format_RGB32);
    quint16 cyan16, magenta16, yellow16, key16;
    const auto *cyan8 = (const quint8 *)imageData.constData();
    const quint8 *magenta8 = cyan8 + totalBytesPerChannel;
    const quint8 *yellow8 = magenta8 + totalBytesPerChannel;
    const quint8 *key8 = yellow8 + totalBytesPerChannel;
    QRgb *p, *end;
    for (quint32 y = 0; y < height; ++y) {
        p = (QRgb *)result.scanLine(y);
        end = p + width;
        while (p < end) {
            cyan16 = ((*cyan8 << 8) + *(cyan8 + 1)) * scale;
            magenta16 = ((*magenta8 << 8) + *(magenta8 + 1)) * scale;
            yellow16 = ((*yellow8 << 8) + *(yellow8 + 1)) * scale;
            key16 = ((*key8 << 8) + *(key8 + 1)) * scale;
            *p = QColor::fromCmyk(255 - (quint8)cyan16,
                                  255 - (quint8)magenta16,
                                  255 - (quint8)yellow16,
                                  255 - (quint8)key16)
                     .rgba();
            ++p;
            cyan8 += 2;
            magenta8 += 2;
            yellow8 += 2;
            key8 += 2;
        }
    }
    return result;
}

QImage QPsdHandler::processCMYK16WithAlpha(QByteArray &imageData,
                                           quint32 width,
                                           quint32 height,
                                           quint64 totalBytesPerChannel)
{
#ifdef QT_DEBUG
    qDebug() << "16-bit CMYK with Alpha";
#endif
    const qreal scale = (qPow(2, 8) - 1) / (qPow(2, 16) - 1);
    QImage result(width, height, QImage::Format_ARGB32);
    quint16 cyan16, magenta16, yellow16, key16, alpha16;
    const auto *cyan8 = (const quint8 *)imageData.constData();
    const quint8 *magenta8 = cyan8 + totalBytesPerChannel;
    const quint8 *yellow8 = magenta8 + totalBytesPerChannel;
    const quint8 *key8 = yellow8 + totalBytesPerChannel;
    const quint8 *alpha8 = key8 + totalBytesPerChannel;
    QRgb *p, *end;
    for (quint32 y = 0; y < height; ++y) {
        p = (QRgb *)result.scanLine(y);
        end = p + width;
        while (p < end) {
            cyan16 = ((*cyan8 << 8) + *(cyan8 + 1)) * scale;
            magenta16 = ((*magenta8 << 8) + *(magenta8 + 1)) * scale;
            yellow16 = ((*yellow8 << 8) + *(yellow8 + 1)) * scale;
            key16 = ((*key8 << 8) + *(key8 + 1)) * scale;
            alpha16 = ((*alpha8 << 8) + *(alpha8 + 1)) * scale;
            *p = QColor::fromCmyk(255 - (quint8)cyan16,
                                  255 - (quint8)magenta16,
                                  255 - (quint8)yellow16,
                                  255 - (quint8)key16,
                                  255 - (quint8)alpha16)
                     .rgba();
            ++p;
            cyan8 += 2;
            magenta8 += 2;
            yellow8 += 2;
            key8 += 2, alpha8 += 2;
        }
    }
    return result;
}

QImage QPsdHandler::processDuotone(QByteArray &imageData, quint32 width, quint32 height)
{
#ifdef QT_DEBUG
    qDebug() << "Duotone";
#endif
    QImage result(width, height, QImage::Format_RGB32);
    const auto *data = (const quint8 *)imageData.constData();
    QRgb *p, *end;
    for (quint32 y = 0; y < height; ++y) {
        p = (QRgb *)result.scanLine(y);
        end = p + width;
        while (p < end) {
            *p = qRgb(*data, *data, *data);
            ++p;
            ++data;
        }
    }
    return result;
}

QImage QPsdHandler::processLAB8(QByteArray &imageData, quint32 width, quint32 height, quint64 totalBytesPerChannel)
{
#ifdef QT_DEBUG
    qDebug() << "8-bit LAB";
#endif
    QImage result(width, height, QImage::Format_RGB32);
    const auto *lightness = (const quint8 *)imageData.constData();
    const quint8 *a = lightness + totalBytesPerChannel;
    const quint8 *b = a + totalBytesPerChannel;
    QRgb *p, *end;
    for (quint32 y = 0; y < height; ++y) {
        p = (QRgb *)result.scanLine(y);
        end = p + width;
        while (p < end) {
            *p = labToRgb(*lightness, *a, *b);
            ++p;
            ++lightness;
            ++a;
            ++b;
        }
    }
    return result;
}

QImage QPsdHandler::processLAB8WithAlpha(QByteArray &imageData,
                                         quint32 width,
                                         quint32 height,
                                         quint64 totalBytesPerChannel)
{
#ifdef QT_DEBUG
    qDebug() << "8-bit LAB with Alpha";
#endif
    QImage result(width, height, QImage::Format_ARGB32);
    const auto *lightness = (const quint8 *)imageData.constData();
    const quint8 *a = lightness + totalBytesPerChannel;
    const quint8 *b = a + totalBytesPerChannel;
    const quint8 *alpha = b + totalBytesPerChannel;
    QRgb *p, *end;
    for (quint32 y = 0; y < height; ++y) {
        p = (QRgb *)result.scanLine(y);
        end = p + width;
        while (p < end) {
            *p = labToRgb(*lightness, *a, *b, *alpha);
            ++p;
            ++alpha;
            ++lightness;
            ++a;
            ++b;
        }
    }
    return result;
}

QImage QPsdHandler::processLAB16(QByteArray &imageData, quint32 width, quint32 height, quint64 totalBytesPerChannel)
{
#ifdef QT_DEBUG
    qDebug() << "16-bit LAB";
#endif
    const qreal scale = (qPow(2, 8) - 1) / (qPow(2, 16) - 1);
    quint16 lightness16, a16, b16;
    QImage result(width, height, QImage::Format_RGB32);
    const auto *lightness8 = (quint8 *)imageData.constData();
    const quint8 *a8 = lightness8 + totalBytesPerChannel;
    const quint8 *b8 = a8 + totalBytesPerChannel;
    QRgb *p, *end;
    for (quint32 y = 0; y < height; ++y) {
        p = (QRgb *)result.scanLine(y);
        end = p + width;
        while (p < end) {
            lightness16 = ((*lightness8 << 8) + *(lightness8 + 1)) * scale;
            a16 = ((*a8 << 8) + *(a8 + 1)) * scale;
            b16 = ((*b8 << 8) + *(b8 + 1)) * scale;
            *p = labToRgb(lightness16, a16, b16);
            ++p;
            lightness8 += 2;
            a8 += 2;
            b8 += 2;
        }
    }
    return result;
}

QImage QPsdHandler::processLAB16WithAlpha(QByteArray &imageData,
                                          quint32 width,
                                          quint32 height,
                                          quint64 totalBytesPerChannel)
{
#ifdef QT_DEBUG
    qDebug() << "8-bit LAB with Alpha";
#endif
    const qreal scale = (qPow(2, 8) - 1) / (qPow(2, 16) - 1);
    quint16 lightness16, a16, b16, alpha16;
    QImage result(width, height, QImage::Format_ARGB32);
    const auto *lightness8 = (const quint8 *)imageData.constData();
    const quint8 *a8 = lightness8 + totalBytesPerChannel;
    const quint8 *b8 = a8 + totalBytesPerChannel;
    const quint8 *alpha8 = b8 + totalBytesPerChannel;
    QRgb *p, *end;
    for (quint32 y = 0; y < height; ++y) {
        p = (QRgb *)result.scanLine(y);
        end = p + width;
        while (p < end) {
            lightness16 = ((*lightness8 << 8) + *(lightness8 + 1)) * scale;
            a16 = ((*a8 << 8) + *(a8 + 1)) * scale;
            b16 = ((*b8 << 8) + *(b8 + 1)) * scale;
            alpha16 = ((*alpha8 << 8) + *(alpha8 + 1)) * scale;
            *p = labToRgb(lightness16, a16, b16, alpha16);
            ++p;
            lightness8 += 2;
            a8 += 2;
            b8 += 2;
            alpha8 += 2;
        }
    }
    return result;
}
