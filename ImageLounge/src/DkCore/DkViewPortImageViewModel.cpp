// SPDX-License-Identifier: GPL-3.0-or-later

#include "DkViewPortImageViewModel.h"

namespace nmc
{

void DkViewPortImageViewModel::setRasterImage(const QImage &img)
{
    mImgStorage.setImage(img);
    mContentState = RasterImage{};
    emit contentStateChanged();
}

void DkViewPortImageViewModel::setSVG(const QByteArray &data)
{
    mContentState = SVG{data};
    emit contentStateChanged();
}

void DkViewPortImageViewModel::setMovie(const QByteArray &data, const QByteArray &format, const QString &fileName)
{
    mContentState = Movie{data, format, fileName};
    emit contentStateChanged();
}

void DkViewPortImageViewModel::fallBackToRaster()
{
    // Clears SVG or movie and fall back to raster image.
    mContentState = RasterImage{};
    emit contentStateChanged();
}

bool DkViewPortImageViewModel::isEmpty() const
{
    return mImgStorage.isEmpty();
}

bool DkViewPortImageViewModel::alphaChannelUsed()
{
    return mImgStorage.alphaChannelUsed();
}

QImage DkViewPortImageViewModel::image() const
{
    return mImgStorage.image();
}

QImage DkViewPortImageViewModel::downsampled(const QSize &size,
                                             const QColorSpace &colorSpace,
                                             QImage::Format format,
                                             int options) &
{
    return mImgStorage.downsampled(size, colorSpace, format, options);
}

void DkViewPortImageViewModel::setImageOnly(const QImage &img)
{
    mImgStorage.setImage(img);
    emit contentStateChanged();
}
}
