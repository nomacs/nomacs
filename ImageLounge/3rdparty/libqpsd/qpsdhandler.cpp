/*
Copyright (c) 2012-2017 Ronie Martinez (ronmarti18@gmail.com)
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
#include <QTextStream>

#ifdef QT_DEBUG
#include <QDebug>
#include <QElapsedTimer>
#endif


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
        else if (version == 2)
            setFormat("psb");
        else return false;
        return true;
    }
    return false;
}

bool QPsdHandler::read(QImage *image)
{
    QSharedPointer<QDataStream> input(new QDataStream(device()));

	input->setByteOrder(QDataStream::BigEndian);

	QBufferHandler bh(input);

    *input >> bh.signature;
    if (!isValidSignature(bh.signature)) {
        return false;
    }

    *input >> bh.version;
    if (!isValidVersion(bh.version)) {
        return false;
    }

    input->skipRawData(6); //reserved bytes should be 6-byte in size

    *input >> bh.channels;
    if (!isChannelCountSupported(bh.channels)) {
        return false;
    }

    *input >> bh.height;
    if (!isValidWidthOrHeight(bh.version, bh.height)) {
        return false;
    }

    *input >> bh.width;
    if (!isValidWidthOrHeight(bh.version, bh.width)) {
        return false;
    }

    *input >> bh.depth;
    if (!isSupportedDepth(bh.depth)) {
        return false;
    }

	quint16 cm = 0;
    *input >> cm;
	bh.colorMode = (QPsdHandler::ColorMode)cm;
    if (!isSupportedColorMode(bh.colorMode)) {
        return false;
    }

    QByteArray colorData = readColorData(*input);
    skipImageResources(*input);
    skipLayerAndMaskSection(*input);

	quint16 cp = 0;
    *input >> cp;

	bh.compression = (QPsdHandler::Compression)cp;

	// currently, we support only raw for files > 4GB
	if (!bh.singleLoad() && bh.compression != QPsdHandler::RAW)
		return false;

    switch (bh.compression) {
    case RLE:
//        The RLE-compressed data is preceeded by a 2-byte(psd) or 4-byte(psb)
//        data count for each row in the data
        if (format() == "psd")
            input->skipRawData(bh.height * bh.channels * 2);
        else if (format() == "psb")
            input->skipRawData(bh.height * bh.channels * 4);
			break;
    case RAW:
		//	imageData = readImageData(input, (Compression)compression, size);
        break;
    default:
        return false;
        break;
    }

    if (input->status() != QDataStream::Ok)
        return false;


#ifdef QT_DEBUG
	qDebug().noquote() << bh.toString();
#endif


    switch (bh.colorMode) {
    case BITMAP:
    {
        QImage result = processBitmap(bh);
        if (result.isNull())
            return false;
        else {
            *image = result;
            return true;
        }
    }
        break;
    case GRAYSCALE:
    {
        switch (bh.depth) {
        case 8:
            if (1 == bh.channels) {
                *image = processGrayscale8(bh);
                return true;
            } else {
                //excess channels other than Gray are considered alphas
                *image = processGrayscale8WithAlpha(bh);
                return true;
            }
            break;
        case 16:
            if (1 == bh.channels) {
                *image = processGrayscale16(bh);
                return true;
            } else {
                //excess channels other than Gray are considered alphas
                *image = processGrayscale16WithAlpha(bh);
                return true;
            }
            break;
        case 32: // FIXME: 32 bpc (HDR)... requires tonemapping
        default:
            return false;
            break;
        }
    }
        break;
    case INDEXED:
        if (8 == bh.depth && 1 == bh.channels) {
            *image = processIndexed(colorData, bh);
            return true;
        } else {
            return false;
        }
        break;
    case RGB:
    {
        switch (bh.depth) {
        case 8:
            if (3 == bh.channels) {
                *image = processRGB8(bh);
                return true;
            } else {
                //excess channels other than RGB are considered alphas
                *image = processRGB8WithAlpha(bh);
                return true;
            }
            break;
        case 16:
            if (3 == bh.channels) {
                *image = processRGB16(bh);
                return true;
            } else {
                //excess channels other than RGB are considered alphas
                *image = processRGB16WithAlpha(bh);
                return true;
            }
            break;
        case 32: //FIXME: 32 bpc (HDR)... requires tonemapping
        default:
            return false;
            break;
        }
    }
        break;
    /* Mixed CMYK and Multichannel logic due to similarities*/
    case CMYK:
    case MULTICHANNEL:
    {
//        Reference: http://help.adobe.com/en_US/photoshop/cs/using/WSfd1234e1c4b69f30ea53e41001031ab64-73eea.html#WSfd1234e1c4b69f30ea53e41001031ab64-73e5a
//        Converting a CMYK image to Multichannel mode creates cyan, magenta, yellow, and black spot channels.
//        Converting an RGB image to Multichannel mode creates cyan, magenta, and yellow spot channels.
        switch (bh.depth) {
        case 8:
            if (3 == bh.channels) {
                *image = processCMY8(bh);
                return true;
            } else if (4 == bh.channels) {
                *image = processCMYK8(bh);
                return true;
            } else {
                *image = processCMYK8WithAlpha(bh);
                return true;
            }
            break;
        case 16:
            if (4 == bh.channels) {
                *image = processCMYK16(bh);
                return true;
            } else {
                *image = processCMYK16WithAlpha(bh);
                return true;
            }
            break;
        default:
            return false;
            break;
        }
    }
        break;
    case DUOTONE:
//        Duotone images: color data contains the duotone specification
//        (the format of which is not documented). Other applications that
//        read Photoshop files can treat a duotone image as a gray image,
//        and just preserve the contents of the duotone information when
//        reading and writing the file.
//        TODO: find a way to actually get the duotone, tritone, and quadtone colors
        if (8 == bh.depth && 1 == bh.channels) {
            *image = processDuotone(bh);
            return true;
        } else {
            return false;
        }
        break;
    case LAB:
    {
        switch (bh.depth) {
        case 8:
            if (3 == bh.channels) {
                *image = processLAB8(bh);
                return true;
            } else {
                *image = processLAB8WithAlpha(bh);
                return true;
            }
            break;
        case 16:
            if (3 == bh.channels) {
                *image = processLAB16(bh);
                return true;
            } else {
                *image = processLAB16WithAlpha(bh);
                return true;
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
    return input->status() == QDataStream::Ok;
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
        input >> signature >> version ;
        input.skipRawData(6); //reserved bytes should be 6-byte in size
        input >> channels >> height >> width >> depth >> colorMode;
        if (input.status() == QDataStream::Ok && signature == 0x38425053 &&
                (version == 1 || version == 2))
            return QSize(width, height);
    }
    return QVariant();
}

bool QPsdHandler::supportsOption(ImageOption option) const
{
    return option == Size;
}

QBufferHandler::QBufferHandler(const QSharedPointer<QDataStream>& stream) 
{
	mStream = stream;
}

QDataStream* QBufferHandler::stream() const 
{
	return mStream.data();
}

bool QBufferHandler::singleLoad() const 
{
	return streamSize() < std::numeric_limits<int>::max();
}

quint64 QBufferHandler::streamSize() const 
{
	return (quint64)channels * totalBytesPerChannel();
}

quint64 QBufferHandler::totalBytesPerChannel() const 
{
	return (quint64)width * height * depth / 8;
}

QString QBufferHandler::toString() const 
{

	QString msg;
	QTextStream out(&msg);
	out << "\ncolor mode: " << colorMode
		<< "\ndepth: " << depth
		<< "\nchannels: " << channels
		<< "\ncompression: " << compression
		<< "\nwidth: " << width
		<< "\nheight: " << height
		<< "\ntotalBytesPerChannel: " << totalBytesPerChannel();

	return msg;
}

int QBufferHandler::chunkSize() const {
	return numRowsPerChunk() * width * depth / 8;
}

int QBufferHandler::numRowsPerChunk() const {
	return 1024;
}

void QBufferHandler::readChannel(quint8 * chPtr) const {
	
	quint8* ptr = chPtr;

	quint64 bpc = totalBytesPerChannel();
	for (quint64 idx = 0; idx < bpc; idx += chunkSize()) {

		mStream->readRawData((char*)ptr, std::min((quint64)chunkSize(), bpc - idx));
		ptr += chunkSize();
	}
}
 