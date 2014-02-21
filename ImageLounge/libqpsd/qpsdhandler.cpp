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

//FIXME: not sure if making "static" will speed up conversion
static const qreal refX = 0.9642;
static const qreal refY = 1.0000;
static const qreal refZ = 0.8249;
static const qreal e = 216 / 24389;
static const qreal k = 24389 / 27;
static const qreal gammaPsd = 563 / 256; //2.19921875

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

#include <QDebug>

bool QPsdHandler::read(QImage *image)
{
	
    QDataStream input(device());
    quint32 signature, height, width, colorModeDataLength, imageResourcesLength;
    quint16 version, channels, depth, colorMode, compression;
    QByteArray colorData;

    input.setByteOrder(QDataStream::BigEndian);

    input >> signature;
    if (signature != 0x38425053) //'8BPS'
	//if (signature == '8BPS')
        return false;

    input >> version; //version should be 1(PSD) or 2(PSB)
    switch (version) {
    case 1:
        //check if format is empty or it is not psd
        if (format().isEmpty() || format() != "psd")
            setFormat("psd");
        break;
    case 2:
        //check if format is empty or it is not psb
        if (format().isEmpty() || format() != "psb")
            setFormat("psb");
        break;
    default: return false;
        break;
    }

    input.skipRawData(6); //reserved bytes should be 6-byte in size

    input >> channels; //Supported range is 1 to 56
    /* found a sample file with channels > 56 and Photoshop can still read it
     * though the documentation says it should be within 1 to 56 channels
     */
    //if (channels < 1 || channels > 56)
    if (channels < 1) //breaking "56-max rule"
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

    //QElapsedTimer timer;
    //timer.start();
    QByteArray imageData;
    quint64 totalBytesPerChannel = width * height * depth / 8;

    switch (compression) {
    case 0: /*RAW IMAGE DATA - UNDER TESTING*/
    {
        /* NOTE: This algorithm might be inaccurate and was based ONLY
         * on a psd file I obtained (no references) */

        /* This code is faster than the alternative below */
        int size = channels * totalBytesPerChannel;
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
        /* The RLE-compressed data is proceeded by a 2-byte(psd) or 4-byte(psb)
         * data count for each row in the data
         */
        if (format() == "psd")
            input.skipRawData(height * channels * 2);
        else if (format() == "psb")
            input.skipRawData(height * channels * 4);

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
                for (quint8 i = 0; i <= count; ++i) {
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
                imageData.resize(size + count);
                input.readRawData(imageData.data() + size, count);
            }
        }
    }
        break;
    case 2:/*ZIP WITHOUT PREDICTION - UNIMPLEMENTED*/
        return false;
        break;
    case 3:/*ZIP WITH PREDICTION - UNIMPLEMENTED*/
        return false;
        break;
    }
    //qDebug() << timer.nsecsElapsed();

    if (input.status() != QDataStream::Ok)
        return false;

    if (imageData.size() != channels * totalBytesPerChannel)
        return false;

    /* NOTE: this section was made for verification.
     * for developers use ONLY */
    /*
    qDebug() << endl
             << "format: " << format()
             << "\ncolor mode: " << colorMode
             << "\ndepth: " << depth
             << "\nchannels: " << channels
             << "\ncompression: " << compression
             << "\nwidth: " << width
             << "\nheight: " << height
             << "\ntotalBytesPerChannel: " << totalBytesPerChannel
             << "\nimage data: " << imageData.size();
    */

    switch (colorMode) {
    case 0: /*BITMAP*/
    {
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
        switch (depth) {
        case 8:
            switch (channels) {
            case 1:
            {
                QImage result(width, height, QImage::Format_RGB32);
                quint8 *data = (quint8*)imageData.constData();
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        *p = qRgb(*data, *data, *data);
                        ++p; ++data;
                    }
                }
                *image = result;
                return true;
            }
                break;
            /* graycale with alpha channel */
            default: //excess channels other than Gray are considered alphas
            {
                QImage result(width, height, QImage::Format_ARGB32);

                quint8 *data = (quint8*)imageData.constData();
                quint8 *alpha = data + totalBytesPerChannel;
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
                return true;
            }
                break;
            }
            break;
        case 16:
        {
            const qreal scale = (qPow(2, 8) -1 ) / (qPow(2, 16) - 1);
            switch (channels) {
            case 1:
            {
                quint16 data16;
                QImage result(width, height, QImage::Format_RGB32);
                quint8 *data8 = (quint8*)imageData.constData();
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        data16 = ((*data8 << 8) + *(data8 + 1)) * scale;
                        *p = qRgb(data16, data16, data16);
                        ++p; data8 += 2;
                    }
                }
                *image = result;
                return true;
            }
                break;
            /* graycale with alpha channel */
            default: //excess channels other than Gray are considered alphas
            {
                quint16 data16, alpha16;
                QImage result(width, height, QImage::Format_ARGB32);
                quint8 *data8 = (quint8*)imageData.constData();
                quint8 *alpha8 = data8 + totalBytesPerChannel;
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        data16 = ((*data8 << 8) + *(data8 + 1)) * scale;
                        alpha16 = ((*alpha8 << 8) + *(alpha8 + 1)) * scale;
                        *p = qRgba(data16, data16, data16, alpha16);
                        ++p; data8 += 2; alpha8 += 2;
                    }
                }
                *image = result;
                return true;
            }
                break;
            }
        }
            break;
        case 32:
        {

			//BUG: "scale" is almost close... obtained through trial and error
			const qreal scale = (qPow(2, 8) - 1) *  (qPow(2, 8) - 1) / (qPow(2, 32) - 1);
			switch (channels) {
			case 1:
				{
					quint32 data32;
					QImage result(width, height, QImage::Format_RGB32);
					quint8 *data8 = (quint8*)imageData.constData();
					QRgb  *p, *end;
					for (quint32 y = 0; y < height; ++y) {
						p = (QRgb *)result.scanLine(y);
						end = p + width;
						while (p < end) {
							data32 = ((*data8 << 24) + (*(data8 + 1) << 16) +
								(*(data8 + 2) << 8) + *(data8 + 3)) * scale;
							*p = qRgb(data32, data32, data32);
							++p; data8 += 4;
						}
					}
					*image = result;
					return true;
				}
				break;
				/* graycale with alpha channel */
			default: //excess channels other than Gray are considered alphas
				{
					quint32 data32, alpha32;
					QImage result(width, height, QImage::Format_ARGB32);
					quint8 *data8 = (quint8*)imageData.constData();
					quint8 *alpha8 = data8 + totalBytesPerChannel;
					QRgb  *p, *end;
					for (quint32 y = 0; y < height; ++y) {
						p = (QRgb *)result.scanLine(y);
						end = p + width;
						while (p < end) {
							data32 = ((*data8 << 24) + (*(data8 + 1) << 16) +
								(*(data8 + 2) << 8) + *(data8 + 3)) * scale;
							alpha32 = ((*alpha8 << 24) + (*(alpha8 + 1) << 16) +
								(*(alpha8 + 2) << 8) + *(alpha8 + 3)) * scale;
							*p = qRgba(data32, data32, data32, alpha32);
							++p; data8 += 4; alpha8 += 4;
						}
					}
					*image = result;
					return true;
				}
				break;
			}

			
			////32 bpc (HDR)... requires tonemapping
   //         return false;
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
                for (int i = 0; i < indexCount; ++i) {
                    result.setColor(i, qRgb(*red, *green, *blue));
                    ++red; ++green; ++blue;
                }

                quint8 *data = (quint8*)imageData.constData();
                for (quint32 i = 0; i < height; ++i) {
                    for (quint32 j = 0; j < width; ++j) {
                        result.setPixel(j, i, *data);
                        ++data;
                    }
                }
                *image = result;
                return true;
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
        switch (depth) {
        case 8:
            switch (channels) {
            case 3:
            {
                QImage result(width, height, QImage::Format_RGB32);
                quint8 *red = (quint8*)imageData.constData();
                quint8 *green = red + totalBytesPerChannel;
                quint8 *blue = green + totalBytesPerChannel;
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
                return true;
            }
                break;
            default: //excess channels other than RGB are considered alphas
            {
                QImage result(width, height, QImage::Format_ARGB32);
                quint8 *red = (quint8*)imageData.constData();
                quint8 *green = red + totalBytesPerChannel;
                quint8 *blue = green + totalBytesPerChannel;
                quint8 *alpha = blue + totalBytesPerChannel;
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
                return true;
            }
                break;
            }
            break;
        case 16:
        {
            const qreal scale = (qPow(2, 8) - 1) / (qPow(2, 16) - 1);
            switch (channels) {
            case 3:
            {
                QImage result(width, height, QImage::Format_RGB32);
                quint16 red16, blue16, green16;
                quint8 *red8 = (quint8*)imageData.constData();
                quint8 *green8 = red8 + totalBytesPerChannel;
                quint8 *blue8 = green8 + totalBytesPerChannel;
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        red16 = ((*red8 << 8) + *(red8 + 1)) * scale;
                        green16 = ((*green8 << 8) + *(green8 + 1)) * scale;
                        blue16 = ((*blue8 << 8) + *(blue8 + 1)) * scale;
                        *p = qRgb(red16, green16, blue16);
                        ++p;  red8 += 2; green8 += 2; blue8 += 2;
                    }
                }
                *image = result;
                return true;
            }
                break;
            default: //excess channels other than RGB are considered alphas
            {
                QImage result(width, height, QImage::Format_ARGB32);
                quint16 red16, blue16, green16, alpha16;
                quint8 *red8 = (quint8*)imageData.constData();
                quint8 *green8 = red8 + totalBytesPerChannel;
                quint8 *blue8 = green8 + totalBytesPerChannel;
                quint8 *alpha8 = blue8 + totalBytesPerChannel;
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        red16 = ((*red8 << 8) + *(red8 + 1)) * scale;
                        green16 = ((*green8 << 8) + *(green8 + 1)) * scale;
                        blue16 = ((*blue8 << 8) + *(blue8 + 1)) * scale;
                        alpha16 = ((*alpha8 << 8) + *(alpha8 + 1)) * scale;
                        *p = qRgba((quint8)red16, (quint8)green16, (quint8)blue16, (quint8)alpha16);
                        ++p;  red8 += 2; green8 += 2; blue8 += 2; alpha8 += 2;
                    }
                }
                *image = result;
                return true;
            }
                break;
            }
        }
            break;
        case 32:
        {
			
			//BUG: "scale" is almost close... obtained through trial and error
			const qreal scale = (qPow(2, 8) - 1) *  (qPow(2, 8) - 1) / (qPow(2, 32) - 1);
			switch (channels) {
			case 3:
				{
					QImage result(width, height, QImage::Format_RGB32);
					quint32 red32, blue32, green32;
					quint8 *red8 = (quint8*)imageData.constData();
					quint8 *green8 = red8 + totalBytesPerChannel;
					quint8 *blue8 = green8 + totalBytesPerChannel;
					QRgb  *p, *end;
					for (quint32 y = 0; y < height; ++y) {
						p = (QRgb *)result.scanLine(y);
						end = p + width;
						while (p < end) {
							red32 = ((*red8 << 24) + (*(red8 + 1) << 16) +
								(*(red8 + 2) << 8) + *(red8 + 3)) * scale;
							green32 = ((*green8 << 24) + (*(green8 + 1) << 16) +
								(*(green8 + 2) << 8) + *(green8 + 3)) * scale;
							blue32 = ((*blue8 << 24) + (*(blue8 + 1) << 16) +
								(*(blue8 + 2) << 8) + *(blue8 + 3)) * scale;
							*p = qRgb(red32, green32, blue32);
							++p;  red8 += 4; green8 += 4; blue8 += 4;
						}
					}
					*image = result;
					return true;
				}
				break;
			default: //excess channels other than RGB are considered alphas
				{
					QImage result(width, height, QImage::Format_ARGB32);
					quint32 red32, blue32, green32, alpha32;
					quint8 *red8 = (quint8*)imageData.constData();
					quint8 *green8 = red8 + totalBytesPerChannel;
					quint8 *blue8 = green8 + totalBytesPerChannel;
					quint8 *alpha8 = blue8 + totalBytesPerChannel;
					QRgb  *p, *end;
					for (quint32 y = 0; y < height; ++y) {
						p = (QRgb *)result.scanLine(y);
						end = p + width;
						while (p < end) {
							red32 = ((*red8 << 24) + (*(red8 + 1) << 16) +
								(*(red8 + 2) << 8) + *(red8 + 3)) * scale;
							green32 = ((*green8 << 24) + (*(green8 + 1) << 16) +
								(*(green8 + 2) << 8) + *(green8 + 3)) * scale;
							blue32 = ((*blue8 << 24) + (*(blue8 + 1) << 16) +
								(*(blue8 + 2) << 8) + *(blue8 + 1)) * scale;
							alpha32 = ((*alpha8 << 24) + (*(alpha8 + 1) << 16) +
								(*(alpha8 + 2) << 8) + *(alpha8 + 3)) * scale;
							*p = qRgba(red32, green32, blue32, alpha32);
							++p;  red8 += 4; green8 += 4; blue8 += 4; alpha8 += 4;
						}
					}
					*image = result;
					return true;
				}
				break;
			}
        }
            break;
        default:
            break;
        }
    }
        break;
    /* Mixed CMYK and Multichannel logic due to similarities*/
    case 4: /*CMYK*/
    case 7: /*MULTICHANNEL*/
    {
        switch (depth) {
        case 8:
        {
            switch (channels) {
            /* Reference: http://help.adobe.com/en_US/photoshop/cs/using/WSfd1234e1c4b69f30ea53e41001031ab64-73eea.html#WSfd1234e1c4b69f30ea53e41001031ab64-73e5a
             * Converting a CMYK image to Multichannel mode creates cyan, magenta, yellow, and black spot channels.
             * Converting an RGB image to Multichannel mode creates cyan, magenta, and yellow spot channels.
             */
            case 3:
            {
                QImage result(width, height, QImage::Format_RGB32);
                quint8 *cyan = (quint8*)imageData.constData();
                quint8 *magenta = cyan + totalBytesPerChannel;
                quint8 *yellow = magenta + totalBytesPerChannel;
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        *p = QColor::fromCmyk(255 - *cyan, 255 - *magenta,
                                              255 - *yellow, 0).rgba();
                        ++p; ++cyan; ++magenta; ++yellow;;
                    }
                }
                *image = result;
                return true;
            }
                break;
            case 4:
            {
                QImage result(width, height, QImage::Format_RGB32);
                quint8 *cyan = (quint8*)imageData.constData();
                quint8 *magenta = cyan + totalBytesPerChannel;
                quint8 *yellow = magenta + totalBytesPerChannel;
                quint8 *key = yellow + totalBytesPerChannel;
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        *p = QColor::fromCmyk(255 - *cyan, 255 - *magenta,
                                              255 - *yellow, 255 - *key).rgba();
                        ++p; ++cyan; ++magenta; ++yellow; ++key;
                    }
                }
                *image = result;
                return true;
            }
                break;
            default: //excess channels other than CMYK are considered alphas
            {
                QImage result(width, height, QImage::Format_ARGB32);
                quint8 *cyan = (quint8*)imageData.constData();
                quint8 *magenta = cyan + totalBytesPerChannel;
                quint8 *yellow = magenta + totalBytesPerChannel;
                quint8 *key = yellow + totalBytesPerChannel;
                quint8 *alpha = key + totalBytesPerChannel;
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        *p = QColor::fromCmyk(255 - *cyan, 255 - *magenta,
                                              255 - *yellow, 255 - *key,
                                              *alpha).rgba();
                        ++p; ++alpha; ++cyan; ++magenta; ++yellow; ++key;
                    }
                }
                *image = result;
                return true;
            }
                break;
            }
        }
            break;
        case 16:
        {
            const qreal scale = (qPow(2, 8) -1 ) / (qPow(2, 16) - 1);
            switch (channels) {
            case 4:
            {
                QImage result(width, height, QImage::Format_RGB32);
                quint16 cyan16, magenta16, yellow16, key16;
                quint8 *cyan8 = (quint8*)imageData.constData();
                quint8 *magenta8 = cyan8 + totalBytesPerChannel;
                quint8 *yellow8 = magenta8 + totalBytesPerChannel;
                quint8 *key8 = yellow8 + totalBytesPerChannel;
                QRgb  *p, *end;
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
                                              255 - (quint8)key16).rgba();
                        ++p;  cyan8 += 2; magenta8 += 2; yellow8 += 2; key8 += 2;
                    }
                }
                *image = result;
                return true;
            }
                break;
            default: //excess channels other than CMYK are considered alphas
            {
                QImage result(width, height, QImage::Format_ARGB32);
                quint16 cyan16, magenta16, yellow16, key16, alpha16;
                quint8 *cyan8 = (quint8*)imageData.constData();
                quint8 *magenta8 = cyan8 + totalBytesPerChannel;
                quint8 *yellow8 = magenta8 + totalBytesPerChannel;
                quint8 *key8 = yellow8 + totalBytesPerChannel;
                quint8 *alpha8 = key8 + totalBytesPerChannel;
                QRgb  *p, *end;
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
                                              255 - (quint8)alpha16).rgba();
                        ++p;  cyan8 += 2; magenta8 += 2; yellow8 += 2; key8 += 2, alpha8 += 2;
                    }
                }
                *image = result;
                return true;
            }
                break;
            }
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
                QImage result(width, height, QImage::Format_RGB32);
                quint8 *data = (quint8*)imageData.constData();
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        *p = qRgb(*data, *data, *data);
                        ++p; ++data;
                    }
                }
                *image = result;
                return true;
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
    case 9: /*LAB*/
    {
        switch (depth) {
        case 8:
            switch (channels) {
            case 3:
            {
                QImage result(width, height, QImage::Format_RGB32);
                quint8 *lightness = (quint8*)imageData.constData();
                quint8 *a = lightness + totalBytesPerChannel;
                quint8 *b = a + totalBytesPerChannel;

                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        *p = labToRgb(*lightness, *a, *b);
                        ++p; ++lightness; ++a; ++b;
                    }
                }
                *image = result;
                return true;
            }
                break;
            default: //excess channels other than CMYK are considered alphas
            {
                QImage result(width, height, QImage::Format_ARGB32);
                quint8 *lightness = (quint8*)imageData.constData();
                quint8 *a = lightness + totalBytesPerChannel;
                quint8 *b = a + totalBytesPerChannel;
                quint8 *alpha = b + totalBytesPerChannel;

                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        *p = labToRgb(*lightness, *a, *b, *alpha);
                        ++p; ++alpha; ++lightness; ++a; ++b;
                    }
                }
                *image = result;
                return true;
            }
                break;
            }
            break;
        case 16:
        {
            const qreal scale = (qPow(2, 8) - 1 ) / (qPow(2, 16) - 1);
            switch (channels) {
            case 3:
            {
                quint16 lightness16, a16, b16;
                QImage result(width, height, QImage::Format_RGB32);
                quint8 *lightness8 = (quint8*)imageData.constData();
                quint8 *a8 = lightness8 + totalBytesPerChannel;
                quint8 *b8 = a8 + totalBytesPerChannel;

                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        lightness16 = ((*lightness8 << 8) + *(lightness8 + 1)) * scale;
                        a16 = ((*a8 << 8) + *(a8 + 1)) * scale;
                        b16 = ((*b8 << 8) + *(b8 + 1)) * scale;
                        *p = labToRgb(lightness16, a16, b16);
                        ++p; lightness8 += 2; a8 += 2; b8 += 2;
                    }
                }
                *image = result;
                return true;
            }
                break;
            default: //excess channels other than CMYK are considered alphas
            {
                quint16 lightness16, a16, b16, alpha16;
                QImage result(width, height, QImage::Format_ARGB32);
                quint8 *lightness8 = (quint8*)imageData.constData();
                quint8 *a8 = lightness8 + totalBytesPerChannel;
                quint8 *b8 = a8 + totalBytesPerChannel;
                quint8 *alpha8 = b8 + totalBytesPerChannel;

                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        lightness16 = ((*lightness8 << 8) + *(lightness8 + 1)) * scale;
                        a16 = ((*a8 << 8) + *(a8 + 1)) * scale;
                        b16 = ((*b8 << 8) + *(b8 + 1)) * scale;
                        alpha16 = ((*alpha8 << 8) + *(alpha8 + 1)) * scale;
                        *p = labToRgb(lightness16, a16, b16, alpha16);
                        ++p; lightness8 += 2; a8 += 2; b8 += 2; alpha8 += 2;
                    }
                }
                *image = result;
                return true;
            }
                break;
            }
        }
            break;
        default:
            return false;
            break;
        }
    }
        break;
    default:
        return false;
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
        input.skipRawData(6); //reserved bytes should be 6-byte in size
        input >> channels >> height >> width >> depth >> colorMode;
        if (input.status() == QDataStream::Ok && signature == 0x38425053 &&
                (version == 1 || version == 2))
            return QSize(width, height);
    }
    return QVariant();
}
