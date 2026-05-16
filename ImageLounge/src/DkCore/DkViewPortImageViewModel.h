// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QSharedPointer>
#include <QSize>
#include <qimage.h>
#include <qstringview.h>
#include <variant>

#include "DkImageContainer.h"
#include "DkImageStorage.h"
#include "nmc_config.h"

class QMovie;
class QSvgRenderer;
class QBuffer;

namespace nmc
{
class DllCoreExport DkViewPortImageViewModel : public QObject
{
    Q_OBJECT

public:
    explicit DkViewPortImageViewModel()
    {
        connect(&mImgStorage, &DkImageStorage::imageUpdated, this, &DkViewPortImageViewModel::imageUpdated);
        connect(&mImgStorage,
                &DkImageStorage::antiAliasingChanged,
                this,
                &DkViewPortImageViewModel::antiAliasingChanged);
    }
    struct RasterImage {
        // This serves as a tag to fall back to use DkImageStorage.
    };

    struct SVG {
        QByteArray data;
    };

    struct Movie {
        QByteArray data;
        QByteArray format;
        QString fileName; // For logging
    };

    using ContentState = std::variant<RasterImage, SVG, Movie>;

    void setRasterImage(const QImage &img);
    void setSVG(const QByteArray &data);
    void setMovie(const QByteArray &data, const QByteArray &format, const QString &fileName);
    void fallBackToRaster();

    // TODO: this is temporary to preserve some behavior without exposing the image storage.
    // Should be removed in the future.
    void setImageOnly(const QImage &img);

    [[nodiscard]] const ContentState &contentState() const
    {
        return mContentState;
    }

    [[nodiscard]] bool isEmpty() const;

    // TODO: make this const
    [[nodiscard]] bool alphaChannelUsed();

    [[nodiscard]] QImage image() const;

    [[nodiscard]] QImage downsampled(const QSize &size,
                                     const QColorSpace &colorSpace,
                                     QImage::Format format,
                                     int options = DkImageStorage::process_async) &;
signals:
    void imageUpdated();
    void contentStateChanged();
    void antiAliasingChanged(bool enabled);

private:
    DkImageStorage mImgStorage;
    ContentState mContentState = RasterImage{};
};
}
