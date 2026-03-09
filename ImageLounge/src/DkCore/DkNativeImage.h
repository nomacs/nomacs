#pragma once
#include <QImage>

#ifdef WITH_OPENCV
#include "opencv2/core/mat.hpp"
#endif

namespace nmc
{
#ifdef WITH_OPENCV

/**
 * @brief Wrapper for sharing image buffers between Qt and OpenCV
 *
 * DkNativeImage represents a shared image buffer that is accessible as a `const QImage&`
 * and exposes a `cv::Mat&` view that can be processed by OpenCV.
 * During construction, the image may be converted to a format that is supported by
 * both QImage and cv::Mat.
 *
 * The exposed references remain valid as long as the lifetime of
 * the DkNativeImage instance.
 */
class DkNativeImage
{
    QImage mImg{};
    cv::Mat mMat{};
    bool mReadOnly = false;

    static int compatibleCvFormat(QImage::Format qtFormat, int options = map_anyrgb);
    static DkNativeImage fromImageInner(QImage &&img, int options, bool readOnly);

    DkNativeImage(QImage &&img, cv::Mat &&mat, bool readOnly)
        : mImg(std::move(img))
        , mMat(std::move(mat))
        , mReadOnly{readOnly}
    {
    }

public:
    DkNativeImage() = default;

    /**
     * @brief View to the underlying image data as QImage
     *
     * The QImage& is valid as long as the DkNativeImage.
     *
     * @warning When constructed from cv::Mat(), use QImage::copy() to persist a copy.
     * @return A reference to the image data
     */
    [[nodiscard]] const QImage &img() const // return const; any write to mImg will break link with mat
    {
        return mImg;
    }

    /**
     * @brief Mutable view to the underlying image data as OpenCV matrix
     *
     * The cv::Mat& is valid as long as the DkNativeImage instance.
     * Use cv::Mat::clone() to persist a copy.
     *
     * @warning Accessing this on a read-only image will trigger a fatal error.
     */
    cv::Mat &mat()
    {
        if (mReadOnly) {
            qFatal("attempt to access mutable mat of read only DkNativeImage");
        }

        return mMat;
    }

    /**
     * @brief Immutable view to the underlying image data as OpenCV matrix
     *
     * The const cv::Mat& is valid as long as the DkNativeImage instance. Use
     * cv::Mat::clone() to persist a copy.
     *
     * @warning Some OpenCV types cast away const and will mutate buffer if
     * care is not taken.
     */
    [[nodiscard]] const cv::Mat &constMat() const
    {
        return mMat;
    }

    /**
     * @brief Check if this instance is read-only.
     *
     * When the return value is true, calling `mat()` will trigger a fatal error.
     */
    [[nodiscard]] bool readOnly() const
    {
        return mReadOnly;
    }

    // options for converting to native formats
    enum {
        map_bgr = 0x1, // cv::Mat is BGR(A) (or grayscale one channel) -- cv::cvtColor() wants this usually
        map_rgb = 0x2, // cv::Mat is RGB(A) (or grayscale one channel)
        map_anyrgb = map_bgr | map_rgb, // either order acceptable
    };

    /**
     * @brief Construct mutable DkNativeImage from QImage, converting if necessary
     *
     * This factory method is used to construct a DkNativeImage and always
     * returns an instance that is mutable via the reference returned from the
     * `mat()` method.
     *
     * DkNativeImage takes ownership of `img` argument.
     *
     * The image may be converted if the format is not supported by both QImage
     * and cv::Mat, or the data could be copied if image refcount > 1.
     *
     * Since ownership is transferred, you must use `DkNativeImage::img()` to
     * get the image back out after any mutation.
     *
     * The internal image is guaranteed to be detached from any other QImage
     * that share the same buffer. Therefore, any mutation will not affect any
     * other QImage. (Unless the QImage data is not managed by Qt, please avoid
     * passing those in.)
     *
     * @param img input image It will be moved into the container.
     * @param options map_* options
     * @return A mutable DkNativeImage.
     */
    static DkNativeImage fromImage(QImage &&img, int options = map_anyrgb);

    /**
     * @brief Construct from QImage, converting if necessary
     *
     * This factory method is used to construct a (usually) immutable
     * DkNativeImage from a `const QImage&`.
     *
     * Conditioning on whether the image is converted, the resulting
     * DkNativeImage may or may not allow mutation via `mat()`. The caller must
     * check with `readOnly()` before attempting to call `mat()`.
     *
     * If the conversion is not required, this method will not copy the image
     * data. Due to the detach mechanisms of QImage, any modification of the
     * same implicitly shared image during the lifetime of this instance will
     * create a deep copy and have no effect on `img()`. (Unless the QImage
     * data is not managed by Qt, please avoid passing those in.)
     *
     * @param img input image
     * @param options map_* options
     * @return A DkNativeImage, may be mutable or immutable.
     */
    static DkNativeImage fromConstImage(const QImage &img, int options = map_anyrgb);

    /**
     * @brief Construct from cv::Mat, without conversions
     *
     * Unlike fromImage() this will not perform any conversions into the
     * non-native Qt pixel formats. To modify the QImage you must take a deep
     * copy.
     *
     * The supported pattern is to use the read-only QImage or copy it out with .copy()
     *
     * @note This will probably go away in favor of fromImage() and allocateFrom()
     *
     * @param mat input mat, always takes a shallow copy, now COW semantics
     * @param srcImg image for restoring/setting the format and colorspace
     * @param options map_* options
     * @return see map_* options
     */
    static DkNativeImage fromMat(cv::Mat &mat, const QImage &srcImg, int options = map_bgr);

    /**
     * @brief Construct an empty image with the same properties (format, colorspace, etc)
     * @param size use this size, otherwise use the size of this image
     * @return A mutable DkNativeImage
     */
    DkNativeImage allocateLike(const QSize &size = {}) const;
};

#endif // WITH_OPENCV

}
