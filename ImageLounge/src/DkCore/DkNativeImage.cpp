
#include "DkNativeImage.h"

#include <QColorSpace>

namespace nmc
{

#if WITH_OPENCV
// There are a limited number of Qt pixel formats that can
// be directly mapped to cv::Mat. OpenCV functions usually
// expect a bgra byte order, but many don't care. So we
// support rgba as well.
static constexpr struct {
    QImage::Format qtFormat;
    int cvFormat;
} kBgrCvFormats[]{
    {QImage::Format_ARGB32, CV_8UC4},
    {QImage::Format_RGB32, CV_8UC4},
    {QImage::Format_BGR888, CV_8UC3},
};

static constexpr struct {
    QImage::Format qtFormat;
    int cvFormat;
} kRgbCvFormats[]{
    {QImage::Format_RGB888, CV_8UC3},
    {QImage::Format_RGBA8888, CV_8UC4},
    {QImage::Format_RGBX8888, CV_8UC4},
    {QImage::Format_RGBA64, CV_16UC4},
    {QImage::Format_RGBX64, CV_16UC4},
    {QImage::Format_RGBA32FPx4, CV_32FC4},
    {QImage::Format_RGBX32FPx4, CV_32FC4},
};

static constexpr struct {
    QImage::Format qtFormat;
    int cvFormat;
} kGrayCvFormats[]{
    {QImage::Format_Grayscale8, CV_8UC1},
    {QImage::Format_Grayscale16, CV_16UC1},
    {QImage::Format_Alpha8, CV_8UC1},
};

int DkNativeImage::compatibleCvFormat(QImage::Format qtFormat, int options)
{
    if (options & map_bgr) {
        for (auto &fmt : kBgrCvFormats) {
            if (fmt.qtFormat == qtFormat) {
                return fmt.cvFormat;
            }
        }
    }

    if (options & map_rgb) {
        for (auto &fmt : kRgbCvFormats) {
            if (fmt.qtFormat == qtFormat) {
                return fmt.cvFormat;
            }
        }
    }

    for (auto &fmt : kGrayCvFormats) {
        if (fmt.qtFormat == qtFormat) {
            return fmt.cvFormat;
        }
    }

    return -1;
}

DkNativeImage DkNativeImage::fromImage(QImage &img, int options)
{
    Q_ASSERT(options & (map_bgr | map_rgb)); // choose one; if both, prefer bgr

    if (img.isNull()) {
        return {};
    }

    int cvFormat = compatibleCvFormat(img.format(), options);

    QImage converted;
    if (cvFormat < 0) {
        // direct map is not possible, convert image to one that is
        QImage::Format newFormat = QImage::Format_Invalid;

        const QPixelFormat pixFormat = img.pixelFormat();
        bool usesAlpha = pixFormat.alphaUsage() == QPixelFormat::UsesAlpha;

        if (img.format() == QImage::Format_Indexed8) {
            if (img.allGray()) { // check color table for r==g==b
                newFormat = QImage::Format_Grayscale8;
                cvFormat = CV_8UC1;
            } else {
                usesAlpha = img.hasAlphaChannel(); // check color table for transparency
            }
        } else if (pixFormat.typeInterpretation() == QPixelFormat::TypeInterpretation::FloatingPoint) {
            // FP16, and future float formats, should go to FP32 to avoid clipping HDR images
            if (options & map_rgb) {
                newFormat = usesAlpha ? QImage::Format_RGBA32FPx4 : QImage::Format_RGBX32FPx4;
                cvFormat = CV_32FC4;
            }
        } else {
            switch (pixFormat.bitsPerPixel()) {
            case 1:
            case 8:
                newFormat = QImage::Format_Grayscale8;
                cvFormat = CV_8UC1;
                break;
            case 16:
            case 24:
            case 32:
                // note: 10-bit formats can't be converted to 64-bit (Qt 6.10.0) but 32-bit works
                break;
            case 64:
                if (options & map_rgb) {
                    newFormat = usesAlpha ? QImage::Format_RGBA64 : QImage::Format_RGBX64;
                    cvFormat = CV_16UC4;
                }
                break;
            case 128:
                if (options & map_rgb) {
                    newFormat = usesAlpha ? QImage::Format_RGBA32FPx4 : QImage::Format_RGBX32FPx4;
                    cvFormat = CV_32FC4;
                }
                break;
            }
        }

        // if there is no other conversion use 32-bit RGB
        if (newFormat == QImage::Format_Invalid) {
            if (options & map_bgr) {
                newFormat = usesAlpha ? QImage::Format_ARGB32 : QImage::Format_BGR888;
            } else if (options & map_rgb) {
                newFormat = usesAlpha ? QImage::Format_RGBA8888 : QImage::Format_RGB888;
            }
            cvFormat = usesAlpha ? CV_8UC4 : CV_8UC3;

            if (pixFormat.greenSize() > 8) {
                qWarning() << "[NativeImage] narrowing conversion from" << img.format() << "to" << newFormat;
            }
        }

        qDebug() << "[NativeImage] converting" << img.format() << "to" << newFormat;
        converted = img.convertToFormat(newFormat);
    }

    try {
        QImage &useImg = converted.isNull() ? img : converted; // careful not to refcount img: forces a deep copy
        uchar *bits = options & map_readonly
            ? const_cast<uchar *>(useImg.constBits()) // never deep copy, unsafe for writable views
            : static_cast<uchar *>(useImg.bits()); // deep copy if img is a shallow copy
        auto stride = static_cast<size_t>(useImg.bytesPerLine());
        cv::Mat mat{useImg.height(), useImg.width(), cvFormat, bits, stride};
        return {useImg, mat};
    } catch (...) {
        qWarning() << "[NativeImage] could not allocate cv::Mat";
    }

    return {};
}

DkNativeImage DkNativeImage::fromMat(cv::Mat &mat, const QImage &srcImg, int options)
{
    int cvFormat = -1;
    QImage::Format qtFormat = srcImg.format(), firstQtFormat = QImage::Format_Invalid;
    if (options & map_bgr) {
        for (auto &c : kBgrCvFormats) {
            if (c.cvFormat == mat.type() && firstQtFormat == QImage::Format_Invalid) {
                firstQtFormat = c.qtFormat;
            }

            if (c.qtFormat == srcImg.format()) {
                cvFormat = c.cvFormat;
                break;
            }
        }
    }

    if (cvFormat < 0 && (options & map_rgb)) {
        for (auto &c : kRgbCvFormats) {
            if (c.cvFormat == mat.type() && firstQtFormat == QImage::Format_Invalid) {
                firstQtFormat = c.qtFormat;
            }

            if (c.qtFormat == qtFormat) {
                cvFormat = c.cvFormat;
                break;
            }
        }
    }

    if (cvFormat < 0) {
        for (auto &c : kGrayCvFormats) {
            if (c.cvFormat == mat.type() && firstQtFormat == QImage::Format_Invalid) {
                firstQtFormat = c.qtFormat;
            }

            if (c.qtFormat == qtFormat) {
                cvFormat = c.cvFormat;
                break;
            }
        }
    }

    if (cvFormat != mat.type()) {
        if (firstQtFormat != QImage::Format_Invalid) {
            qWarning() << "[NativeImage] no direct mapping from" << cv::typeToString(mat.type()).c_str() << "to"
                       << srcImg.format() << ": using" << firstQtFormat;
            qtFormat = firstQtFormat;
        } else {
            // TODO: no fallback here, do we need one?
            qWarning() << "[NativeImage] no available conversion from" << cv::typeToString(mat.type()).c_str() << "to"
                       << srcImg.format();
            return {};
        }
    }

    QImage qImg(mat.data, mat.cols, mat.rows, mat.step, qtFormat);
    qImg.setColorSpace(srcImg.colorSpace());

    return {qImg, mat};
}

DkConstNativeImage DkConstNativeImage::fromImage(const QImage &img, int options)
{
    auto &nonConst = const_cast<QImage &>(img);
    auto view = DkNativeImage::fromImage(nonConst, options | map_readonly);
    return DkConstNativeImage{std::move(view)};
}

DkNativeImage DkNativeImage::allocateLike(const QSize &size) const
{
    QImage tmp(size.isEmpty() ? mImg.size() : size, mImg.format());
    tmp.setColorSpace(mImg.colorSpace());
    return fromImage(tmp, map_anyrgb);
}

#endif // WITH_OPENCV

}
