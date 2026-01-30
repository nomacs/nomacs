#include "DkImageStorage.h"
#include "DkNativeImage.h"

#include "QMetaEnum"
#include "gtest/gtest.h"

using namespace nmc;

#if WITH_OPENCV
#include "opencv2/imgproc.hpp"

template<typename T>
static std::string qEnumToString(T value)
{
    return QMetaEnum::fromType<QImage::Format>().valueToKey(value);
}

static std::string qColorToString(const QColor &color)
{
    return "ARGB(" + color.name(QColor::HexArgb).toStdString() + ")";
}

// set a pixel before any conversions take place (rgb: 255 blue, others: white)
static void setPixel(QImage &img)
{
    if (img.format() == QImage::Format_Alpha8) {
        img.fill(255);
    } else if (img.format() == QImage::Format_Indexed8) {
        QList<QRgb> table{256};
        table.fill(qRgb(0, 0, 0));
        table[0] = qRgb(0, 0, 255);
        img.setColorTable(table);
        img.fill(0);
    } else if (img.format() == QImage::Format_Mono) {
        img.fill(1);
    } else if (img.format() == QImage::Format_MonoLSB) {
        img.fill(1);
    } else if (img.pixelFormat().colorModel() == QPixelFormat::Grayscale) {
        img.fill(QColor(Qt::white));
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    } else if (img.format() == QImage::Format_CMYK8888) {
        auto tmp = QImage(img.size(), QImage::Format_RGB32);
        tmp.fill(QColor(Qt::blue));
        img = tmp.convertToFormat(img.format());
#endif
    } else {
        img.fill(QColor(Qt::blue));
    }
}

// check after conversions matched setPixel()
static ::testing::AssertionResult checkPixel(const QImage &img)
{
    // this is simpler as it already went through imageToMat
    if (img.format() == QImage::Format_Alpha8) {
        QRgb pixel = img.pixel(0, 0);
        QRgb expected = qRgba(0, 0, 0, 255);
        if (pixel != expected)
            return ::testing::AssertionFailure() << "expected " << expected << " but got " << pixel;
    } else if (img.pixelFormat().colorModel() == QPixelFormat::Grayscale) {
        QColor pixel = img.pixelColor(0, 0);
        QColor expected = Qt::white;
        if (pixel != expected)
            return ::testing::AssertionFailure()
                << "expected: " << qColorToString(expected) << " but got " << qColorToString(pixel);
    } else {
        QColor pixel = img.pixelColor(0, 0);
        QColor expected = Qt::blue;
        if (pixel != expected)
            return ::testing::AssertionFailure()
                << "expected: " << qColorToString(expected) << " but got " << qColorToString(pixel);
    }

    return ::testing::AssertionSuccess();
}

// check after conversions matched setPixel()
static ::testing::AssertionResult checkPixel(const cv::Mat &mat)
{
    int channel = 0;
    switch (mat.depth()) {
    case CV_8U: {
        int pixel = mat.ptr<uint8_t>()[channel];
        int expected = 0xFF;
        if (pixel != expected) {
            return ::testing::AssertionFailure() << "expected " << expected << " but got " << pixel;
        }
        break;
    }
    case CV_16U: {
        int pixel = mat.ptr<uint16_t>()[channel];
        int expected = 0xFFFF;
        if (pixel != expected) {
            return ::testing::AssertionFailure() << "expected " << expected << " but got " << pixel;
        }
        break;
    }
    case CV_32F: {
        float pixel = mat.ptr<float>()[channel];
        float expected = 1.0f;
        if (pixel != expected) {
            return ::testing::AssertionFailure() << "expected " << expected << " but got " << pixel;
        }
        break;
    }
    default:
        return ::testing::AssertionFailure() << "unexpected matrix depth:" << mat.depth();
    }
    return ::testing::AssertionSuccess();
}

TEST(ImageToMat, Default)
{
    // test we don't blow up on invalid input / null image
    QImage nullImg;

    int options[] = {DkNativeImage::map_bgr, DkNativeImage::map_rgb};

    {
        DkNativeImage native;
        EXPECT_TRUE(native.img().isNull());
        EXPECT_TRUE(native.mat().empty());

        DkNativeImage copy = native;
        EXPECT_TRUE(copy.img().isNull());
        EXPECT_TRUE(copy.mat().empty());
    }

    for (int flags : options) {
        SCOPED_TRACE("flags: " + QString::number(flags).toStdString());
        auto native = DkNativeImage::fromImage(nullImg, flags);
        EXPECT_TRUE(native.img().isNull());
        EXPECT_TRUE(native.mat().empty());
    }
    for (int flags : options) {
        SCOPED_TRACE("flags: " + QString::number(flags).toStdString());
        auto native = DkConstNativeImage::fromImage(nullImg, flags);
        EXPECT_TRUE(native.img().isNull());
        EXPECT_TRUE(native.mat().empty());
    }

    {
        SCOPED_TRACE("img2mat");
        auto mat = DkImage::qImage2Mat(nullImg);
        EXPECT_TRUE(mat.empty());
    }

    {
        SCOPED_TRACE("mat2img");
        cv::Mat mat;
        EXPECT_TRUE(mat.empty());
        auto img = DkImage::mat2QImage(mat, nullImg);
        EXPECT_TRUE(img.isNull());
    }
}

TEST(ImageToMat, Mapped)
{
    // test the no-copy direct-map capability
    constexpr std::array<QImage::Format, 13> directFormats = {
        QImage::Format_Alpha8,
        QImage::Format_Grayscale8,
        QImage::Format_Grayscale16,
        QImage::Format_BGR888,
        QImage::Format_RGB888,
        QImage::Format_ARGB32,
        QImage::Format_RGB32,
        QImage::Format_RGBA8888,
        QImage::Format_RGBX8888,
        QImage::Format_RGBA64,
        QImage::Format_RGBX64,
        QImage::Format_RGBA32FPx4,
        QImage::Format_RGBX32FPx4,
    };

    for (auto format : directFormats) {
        SCOPED_TRACE(qEnumToString(format));
        QImage img{1, 1, format};
        setPixel(img);

        auto origBits = img.constBits();

        QImage outerCopy;

        // normal mode, no copying
        {
            auto native = DkNativeImage::fromImage(img);
            EXPECT_EQ(native.img().constBits(), native.mat().ptr<uchar>(0)); // correct linkage
            EXPECT_EQ(native.img().constBits(), origBits); // no copy should occur, img.refcount==1

            auto copy = native;
            EXPECT_EQ(copy.img().constBits(), copy.mat().ptr<uchar>(0)); // copy construction; no issues as well
            EXPECT_EQ(copy.img().constBits(), origBits);

            // create another shallow copy to test what happens when chaining ::fromImage()
            outerCopy = copy.img();
            EXPECT_EQ(outerCopy.constBits(), origBits);

            img = native.img();
        }
        EXPECT_EQ(origBits, img.constBits()); // copied back to itself; no deep copy

        img = {}; // now outerCopy is the only reference, should be no deep copy once again
        {
            auto native = DkNativeImage::fromImage(outerCopy);
            EXPECT_EQ(native.img().constBits(), native.mat().ptr<uchar>(0)); // copy construction; no issues as well
            EXPECT_EQ(native.img().constBits(), outerCopy.constBits());
            EXPECT_EQ(native.img().constBits(), origBits);
        }

        // reset since we wiped it out
        img = QImage{1, 1, format};
        setPixel(img);
        origBits = img.constBits();

        // normal mode, copying required
        QImage cowImg = img; // refcount of img data is now > 1, forces deep copy
        EXPECT_EQ(cowImg.constBits(), img.constBits());
        {
            auto native = DkNativeImage::fromImage(cowImg);
            EXPECT_EQ(native.img().constBits(), native.mat().ptr<uchar>(0));
            EXPECT_EQ(native.img().constBits(), cowImg.constBits()); // no copy/correct linkage
            EXPECT_NE(native.img().constBits(), origBits); // COW enforced, deep copy of img

            auto copy = native;
            EXPECT_EQ(copy.img().constBits(), copy.mat().ptr<uchar>(0)); // copy construction; no issues as well
            EXPECT_EQ(copy.img().constBits(), cowImg.constBits());
        }

        // read-only mode, no copying
        {
            auto native = DkConstNativeImage::fromImage(img);
            EXPECT_EQ(native.img().constBits(), native.mat().ptr<uchar>(0));
            EXPECT_EQ(native.img().constBits(), img.constBits());
            EXPECT_EQ(native.img().constBits(), origBits);

            auto copy = native;
            EXPECT_EQ(copy.img().constBits(), copy.mat().ptr<uchar>(0)); // copy construction; no issues as well
            EXPECT_EQ(copy.img().constBits(), img.constBits());

            // native.img().fill(0); // does not compile

            // this will mutate mat/img, nothing we can do except don't use cv::Mat
            // cv::rectangle(native.mat(), cv::Rect{0, 0, 1, 1}, cv::Scalar{255, 255, 255, 255}, cv::FILLED);
        }

        // read-only mode from potentially COW image, also no copying
        cowImg = img;
        EXPECT_EQ(cowImg.constBits(), img.constBits());
        {
            auto native = DkConstNativeImage::fromImage(cowImg);
            EXPECT_EQ(native.img().constBits(), native.mat().ptr<uchar>(0));
            EXPECT_EQ(native.img().constBits(), cowImg.constBits());
            EXPECT_EQ(native.img().constBits(), origBits); // COW relaxed: not allowed to mutate img/mat
        }
    }
}

TEST(ImageToMat, ChannelOrder)
{
    // test the channel order options : map_bgr , map_rgb for all Qt image formats
    for (int fmt = QImage::Format_Invalid + 1; fmt < QImage::NImageFormats; fmt++) {
        auto format = static_cast<QImage::Format>(fmt);
        SCOPED_TRACE("src format: " + qEnumToString(format));
        QImage img{1, 1, format};

        if (img.pixelFormat().channelCount() == 1) {
            continue;
        }

        QColor colors[] = {Qt::blue, Qt::red};
        int options[] = {DkNativeImage::map_bgr, DkNativeImage::map_rgb};

        for (int i = 0; i < 2; ++i) {
            img.setPixelColor(0, 0, colors[i]); // 0xFF => channel[0]

            auto native = DkConstNativeImage::fromImage(img, options[i]);
            SCOPED_TRACE("native format: " + qEnumToString(native.img().format()));
            SCOPED_TRACE("native option: " + QString::number(options[i]).toStdString());

            EXPECT_TRUE(checkPixel(native.mat())); // check channel[0] is set to 0xFF
        }
    }
}

TEST(ImageToMat, AllocateLike)
{
    for (int fmt = QImage::Format_Invalid + 1; fmt < QImage::NImageFormats; fmt++) {
        auto format = static_cast<QImage::Format>(fmt);
        SCOPED_TRACE("src format: " + qEnumToString(format));
        QImage src{1, 1, format};

        {
            const auto in = DkConstNativeImage::fromImage(src);
            const auto out = in.allocateLike();
            EXPECT_EQ(out.img().size(), in.img().size());
            EXPECT_EQ(out.img().format(), in.img().format());
        }
    }
}

TEST(ImageToMat, Scaled)
{
    // integration test, scaling an image with minimal copies/conversions
    for (int fmt = QImage::Format_Invalid + 1; fmt < QImage::NImageFormats; fmt++) {
        auto format = static_cast<QImage::Format>(fmt);
        SCOPED_TRACE("src format: " + qEnumToString(format));
        QImage src{1024, 768, format};
        setPixel(src);

        {
            auto in = DkConstNativeImage::fromImage(src);

            QSize size{640, 480};
            auto out = in.allocateLike(size);
            EXPECT_EQ(out.img().size(), size);

            cv::resize(in.mat(), out.mat(), cv::Size(size.width(), size.height()), 0, 0, cv::INTER_NEAREST);
            src = out.img();
        }
        EXPECT_TRUE(checkPixel(src));
        break;
    }
}

TEST(MatToImage, Default)
{
    for (int fmt = QImage::Format_Invalid + 1; fmt < QImage::NImageFormats; fmt++) {
        auto format = static_cast<QImage::Format>(fmt);
        SCOPED_TRACE("src format: " + qEnumToString(format));

        QImage img{1, 1, format};
        setPixel(img);

        QImage out;
        {
            auto mat = DkImage::qImage2Mat(img);
            EXPECT_TRUE(checkPixel(mat));
            EXPECT_NE(img.constBits(), mat.ptr<uchar>()); // deep copy

            auto qImg = DkImage::mat2QImage(mat, img);
            EXPECT_TRUE(checkPixel(qImg));
            EXPECT_NE(qImg.constBits(), mat.ptr<uchar>()); // deep copy

            out = qImg;
        }
        EXPECT_TRUE(checkPixel(out));
    }
}

#endif // WITH_OPENCV
