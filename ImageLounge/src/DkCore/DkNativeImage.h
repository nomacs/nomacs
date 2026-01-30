#pragma once
#include <QImage>

#ifdef WITH_OPENCV
#include "opencv2/core/mat.hpp"
#endif

namespace nmc
{
#ifdef WITH_OPENCV

// container for image in a "native" format supported by nomacs, which
// for the moment means it can map directly to/from cv::Mat
class DkNativeImage
{
protected:
    QImage mImg{};
    cv::Mat mMat{};

    DkNativeImage(const QImage &img, const cv::Mat &mat)
        : mImg(img)
        , mMat(mat)
    {
    }

public:
    DkNativeImage() = default;

    // note: if constructed from cv::Mat, must use .copy() to persist a copy
    const QImage &img() const // return const; any write to mImg will break link with mat
    {
        return mImg;
    }

    // note: if constructed from QImage, must use .clone() to persist a copy
    cv::Mat &mat()
    {
        return mMat;
    }

    // options for converting to native formats
    enum {
        map_bgr = 0x1, // cv::Mat is BGR(A) (or grayscale one channel) -- cv::cvtColor() wants this usually
        map_rgb = 0x2, // cv::Mat is RGB(A) (or grayscale one channel)
        map_readonly = 0x4, // returned mat/image should not be modified
        map_anyrgb = map_bgr | map_rgb, // either order acceptable
    };

    /**
     * @brief construct from QImage, converting if necessary
     * @param img input reference to avoid taking a deep copy (see: QImage::bits())
     * @param options map_* options
     *
     * @note  If the image refcount > 1, a deep copy of pixels is performed,
     *        and the image is unmodified. So it is not possible to mutate other copies.
     *
     * @note If a direct mapping is not supported, there is a conversion, and
     *       the passed image is unmodified; always use the "img" member to
     *       get the QImage back
     *
     * @return see map_* options
     */
    static DkNativeImage fromImage(QImage &img, int options = map_anyrgb);

    /**
     * @brief construct from QImage, converting if necessary
     * @param mat input mat, always takes a shallow, non-COW copy (reference counted by cv::Mat)
     * @param srcImg image for restoring/setting the format and colorspace
     * @param options map_* options
     *
     * @note Unlike fromImage() this will not perform any conversions into the non-native
     *       Qt pixel formats. To modify the qimage you must take a deep copy.
     *
     * @note The supported pattern is to use the read-only QImage or copy it out with .copy()
     *
     * @note This will probably go away in favor of fromImage() on pre-allocated QImage,
     *       avoid using it
     *
     * @return see map_* options
     */
    static DkNativeImage fromMat(cv::Mat &mat, const QImage &srcImg, int options = map_bgr);

    DkNativeImage allocateLike(const QSize &size = {}) const;

protected:
    static int compatibleCvFormat(QImage::Format qtFormat, int options = map_anyrgb);
};

// const-ified DkNativeImage (provides limited footgun protection on cv::Mat)
class DkConstNativeImage : public DkNativeImage
{
protected:
    DkConstNativeImage(DkNativeImage &&img)
        : DkNativeImage(std::move(img))
    {
    }

public:
    const cv::Mat &mat() const
    {
        return mMat;
    }

    /**
     * @brief immutable version
     *
     * @note use this if you will not mutate the cv::Mat, as there is rarely
     *       any copying of the image buffer (for common formats)
     *
     * @warning OpenCV OutputArray parameters will write into const cv::Mat&,
     *          be sure not to use it in this way or otherwise cast away
     *          const.
     */
    static DkConstNativeImage fromImage(const QImage &img, int options = map_anyrgb);
};

#endif // WITH_OPENCV

}
