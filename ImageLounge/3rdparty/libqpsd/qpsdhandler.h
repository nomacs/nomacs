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

#ifndef QPSDHANDLER_H
#define QPSDHANDLER_H

#include <QImageIOHandler>
#include <QImage>
#include <QColor>
#include <QVariant>
#include <qmath.h>

class QBufferHandler;

class QPsdHandler : public QImageIOHandler
{
public:
    QPsdHandler();
    ~QPsdHandler();

    bool canRead() const;
    bool read(QImage *image);
    //bool write(const QImage &image);

    static bool canRead(QIODevice *device);

    QVariant option(ImageOption option) const;
    //void setOption(ImageOption option, const QVariant &value);
    bool supportsOption(ImageOption option) const;

	enum Compression {
		RAW = 0,
		RLE = 1,
		ZIP_WITHOUT_PREDICTION = 2,
		ZIP_WITH_PREDICTION = 3
	};

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

private:
    bool isValidSignature(quint32 signature);
    bool isValidVersion(quint16 version);
    bool isChannelCountSupported(quint16 channel);
    bool isValidWidthOrHeight(quint16 version, quint32 value);
    bool isSupportedDepth(quint16 depth);
    bool isSupportedColorMode(quint16 colorMode);
	QByteArray readColorData(QDataStream& input);
	void skipImageResources(QDataStream& input);
	void skipLayerAndMaskSection(QDataStream& input);

	QByteArray readImageData(QDataStream& input, Compression compression, quint64 size=0);

	QImage processBitmap(const QBufferHandler& bh);
    QImage processGrayscale8(const QBufferHandler& bh);
    QImage processGrayscale8WithAlpha(const QBufferHandler& bh);
    QImage processGrayscale16(const QBufferHandler& bh);
    QImage processGrayscale16WithAlpha(const QBufferHandler& bh);
    QImage processIndexed(QByteArray& colorData, const QBufferHandler& bh);
    QImage processRGB8(const QBufferHandler& bh);
    QImage processRGB16(const QBufferHandler& bh);
    QImage processRGB8WithAlpha(const QBufferHandler& bh);
    QImage processRGB16WithAlpha(const QBufferHandler& bh);
    QImage processCMY8(const QBufferHandler& bh);
    QImage processCMYK8(const QBufferHandler& bh);
    QImage processCMYK8WithAlpha(const QBufferHandler& bh);
    QImage processCMYK16(const QBufferHandler& bh);
    QImage processCMYK16WithAlpha(const QBufferHandler& bh);
    QImage processDuotone(const QBufferHandler& bh);
    QImage processLAB8(const QBufferHandler& bh);
    QImage processLAB8WithAlpha(const QBufferHandler& bh);
    QImage processLAB16(const QBufferHandler& bh);
    QImage processLAB16WithAlpha(const QBufferHandler& bh);
};

class QBufferHandler {

public:
	QBufferHandler(const QSharedPointer<QDataStream>& stream = QSharedPointer<QDataStream>());

	QDataStream* stream() const;

	bool singleLoad() const;

	quint64 streamSize() const;
	quint64 totalBytesPerChannel() const;

	QString toString() const;

	int chunkSize() const;
	int numRowsPerChunk() const;

	void readChannel(quint8* chPtr) const;

	quint32 signature = 0;
	quint32 height = 0;
	quint32 width = 0;

	quint16 version = 0;
	quint16 channels = 0;
	quint16 depth = 0;
	QPsdHandler::ColorMode colorMode = QPsdHandler::BITMAP;
	QPsdHandler::Compression compression = QPsdHandler::RAW;

private:
	QSharedPointer<QDataStream> mStream;

	quint64 mStreamSize = 0;
};

#endif // QPSDHANDLER_H
