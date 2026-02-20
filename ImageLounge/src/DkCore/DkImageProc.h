/*******************************************************************************************************
 DkImageProc.h
 Created on:	02.16.2026

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2026 Scrubs <scrubbbbs@gmail.org>

 This file is part of nomacs.

 nomacs is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 nomacs is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 *******************************************************************************************************/

#pragma once

#include <QImage>
#include <QtConcurrentMap>

#ifdef WITH_OPENCV
#include "opencv2/imgproc/imgproc.hpp"
#endif

namespace nmc
{
/**
 * This scheme does not support big-endian systems for the moment (PowerPC, some rare RiscV)
 * - For simplicity we assume bytes 0-2 are RGB or BGR, and byte 3 (if any) is alpha
 * - The ARGB32/RGB32 byte order on big-endian is: (byte 1-3 is rgb, 0 is alpha)
 */
static_assert(Q_BYTE_ORDER == Q_LITTLE_ENDIAN, "unsupported architecture");

/**
 *
 * @brief Pixel formats we can directly map between QImage and cv::Mat
 *
 * These are all aligned packed pixel formats, that is the channels are aligned to sizeof(channel),
 * and are adjacent in memory.
 *
 * These roughly correspond to Qt pixel formats of the similar name, except:
 * - RGB32/RGBX8888/RGBX64 are supported but use the alpha variant of the format
 * - This is fine since the X variant stores a saturated alpha (0xFF,etc)
 *   which we can simply process as if it was opaque alpha (and take small performance hit)
 *
 * @note Float16 is not here because most cv operations can't deal with it (intended as GPU format),
 */
enum class ImgFmt {
    Invalid = 0,
    Gray8,
    Gray16,
    BGR888,
    RGB888,
    ARGB32,
    RGBA8888,
    RGBA64,
    RGBAFP32,
    NFormats // last
};

enum class ImgType {
    bgr, // memory order is B-G-R(-A)
    rgb, // memory order is R-G-B(-A)
    gray, // one channel
    // cmyk, // not implemented
};

/**
 * @brief describe a pixel format with some helpers
 *
 * @note image kernels have required constants/types imported:
 *
 *  template<typename Format> // << typename==PixFormat<>
 *  kernel() { mat.ptr<Format::ChannelType>(); }
 *
 *  For more extensive format support this might contain alpha channel usage,
 *  masks and shifts for packed formats, premultiply helpers etc.
 */
template<ImgType type, typename T, int channels>
struct PixFormat {
    using ChannelType = T; // channel data type, always unsigned integer or float

    static constexpr ImgType Type = (ImgType)type; // basic pixel format/layout
    static constexpr int Channels = channels; // rgb(a) channels per pixel (grayscale == 1 channel)
    static constexpr int Scale = // scale factor from float [0,1]
        std::is_floating_point_v<T> ? 1 : std::numeric_limits<T>::max();

#ifdef WITH_OPENCV
    static bool isCompatibleWith(const cv::Mat &mat) // basic check for mismatched cv::Mat (channel order ignored)
    {
        return sizeof(T) * channels == mat.elemSize();
    }
#endif

    static bool isCompatibleWith(const QImage &img)
    {
        return sizeof(T) * channels == img.depth() / 8;
    }

    // load pixel to float {r,g,b,a} [0,1]
    // HDR (fp32) images may have values outside of [0,1]
    static std::tuple<float, float, float, float> loadFloat(const T *rgba)
    {
        T sb, sr, sg;
        if constexpr (Type == ImgType::gray) {
            sb = rgba[0];
            sg = sr = sb;
        } else if constexpr (Type == ImgType::rgb) {
            sr = rgba[0], sg = rgba[1], sb = rgba[2];
        } else {
            static_assert(Type == ImgType::bgr);
            sb = rgba[0], sg = rgba[1], sr = rgba[2];
        }

        constexpr float invScale = 1.0f / Scale;
        float b = sb * invScale, g = sg * invScale, r = sr * invScale;
        float a = Channels > 3 ? rgba[3] * invScale : 1.0f;

        return {r, g, b, a};
    }

    // store float [0,1] to pixel
    static void store(T *pixel, float r, float g, float b, float a)
    {
        float scaleF = Scale;

        if constexpr (Type == ImgType::gray) {
            pixel[0] = r * scaleF;
        } else if constexpr (Type == ImgType::rgb) {
            pixel[0] = r * scaleF, pixel[1] = g * scaleF, pixel[2] = b * scaleF;
        } else {
            static_assert(Type == ImgType::bgr);
            pixel[0] = b * scaleF, pixel[1] = g * scaleF, pixel[2] = r * scaleF;
        }

        if constexpr (Channels == 4) {
            pixel[3] = a * scaleF;
        }
    }

    // store float [0,1] to pixel, ignore alpha channel
    static void store(T *pixel, float r, float g, float b)
    {
        float scaleF = Scale;

        if constexpr (Type == ImgType::gray) {
            pixel[0] = r * scaleF;
        } else if constexpr (Type == ImgType::rgb) {
            pixel[0] = r * scaleF, pixel[1] = g * scaleF, pixel[2] = b * scaleF;
        } else {
            static_assert(Type == ImgType::bgr);
            pixel[0] = b * scaleF, pixel[1] = g * scaleF, pixel[2] = r * scaleF;
        }
    }
};

// types we will specialize kernels with
using PixFmt_Gray8 = PixFormat<ImgType::gray, uint8_t, 1>;
using PixFmt_Gray16 = PixFormat<ImgType::gray, uint16_t, 1>;
using PixFmt_BGR888 = PixFormat<ImgType::bgr, uint8_t, 3>;
using PixFmt_RGB888 = PixFormat<ImgType::rgb, uint8_t, 3>;
using PixFmt_ARGB32 = PixFormat<ImgType::bgr, uint8_t, 4>;
using PixFmt_RGBA8888 = PixFormat<ImgType::rgb, uint8_t, 4>;
using PixFmt_RGBA64 = PixFormat<ImgType::rgb, uint16_t, 4>;
using PixFmt_RGBAFP32 = PixFormat<ImgType::rgb, float, 4>;

// clang-format off
// convert ImgFmt enum to PixFmt_*
template<ImgFmt> struct ImgFmtToPixFormat;
template<> struct ImgFmtToPixFormat<ImgFmt::Gray8>    { using type = PixFmt_Gray8; };
template<> struct ImgFmtToPixFormat<ImgFmt::Gray16>   { using type = PixFmt_Gray16; };
template<> struct ImgFmtToPixFormat<ImgFmt::BGR888>   { using type = PixFmt_BGR888; };
template<> struct ImgFmtToPixFormat<ImgFmt::RGB888>   { using type = PixFmt_RGB888; };
template<> struct ImgFmtToPixFormat<ImgFmt::ARGB32>   { using type = PixFmt_ARGB32; };
template<> struct ImgFmtToPixFormat<ImgFmt::RGBA8888> { using type = PixFmt_RGBA8888; };
template<> struct ImgFmtToPixFormat<ImgFmt::RGBA64>   { using type = PixFmt_RGBA64; };
template<> struct ImgFmtToPixFormat<ImgFmt::RGBAFP32> { using type = PixFmt_RGBAFP32; };
// clang-format on

/**
 * @brief convert QImage format to something in our short list
 *
 * @note DkNativeImage::fromImage() converts everything else to these Qt formats
 *
 * @note Trying to treat the RGB(X) formats the same as RGB(A), it seems like
 *       it will work since the X channel is always fully saturated (255 etc).
 */
static ImgFmt qtImageFormatToNative(QImage::Format format)
{
    // clang-format off
    switch (format) {
    case QImage::Format_Grayscale8:  return ImgFmt::Gray8;
    case QImage::Format_Grayscale16: return ImgFmt::Gray16;
    case QImage::Format_BGR888:      return ImgFmt::BGR888;
    case QImage::Format_RGB888:      return ImgFmt::RGB888;
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:       return ImgFmt::ARGB32;
    case QImage::Format_RGBA8888:
    case QImage::Format_RGBX8888:    return ImgFmt::RGBA8888;
    case QImage::Format_RGBA64:
    case QImage::Format_RGBX64:      return ImgFmt::RGBA64;
    case QImage::Format_RGBA32FPx4:
    case QImage::Format_RGBX32FPx4:  return ImgFmt::RGBAFP32;
    default:                         return ImgFmt::Invalid;
    }
    // clang-format on
}

static QImage::Format nativeFormatToQtFormat(ImgFmt fmt)
{
    // clang-format off
    switch (fmt) {
    case ImgFmt::Gray8:    return QImage::Format_Grayscale8;
    case ImgFmt::Gray16:   return QImage::Format_Grayscale16;
    case ImgFmt::BGR888:   return QImage::Format_BGR888;
    case ImgFmt::RGB888:   return QImage::Format_RGB888;
    case ImgFmt::ARGB32:   return QImage::Format_ARGB32;
    case ImgFmt::RGBA8888: return QImage::Format_RGBA8888;
    case ImgFmt::RGBA64:   return QImage::Format_RGBA64;
    case ImgFmt::RGBAFP32: return QImage::Format_RGBA32FPx4;
    default:               return QImage::Format_Invalid;
    }
    // clang-format on
}

// work range for parallel stuff
struct DkWorkRange {
    int begin = 0, end = 0;

    bool isWithin(int x0, int xN) const // [begin,end) is within [x0,xN)
    {
        return begin >= x0 && begin < end && end <= xN;
    }

    QList<DkWorkRange> partition() const // partiton by cpu thread count
    {
        QList<DkWorkRange> parts;
        int partSize = qMax(1, (end - begin) / QThreadPool::globalInstance()->maxThreadCount());
        for (int y0 = begin; y0 < end; y0 += partSize) {
            parts.append({y0, qMin(end, y0 + partSize)});
        }
        qDebug() << "parts=" << parts.length();
        return parts;
    }
};

/**
 * @brief parent class for image processing kernels
 * @abstract
 *
 *   This class helps with multithreading and dispatching the correct
 *   kernel for the desired image format, the rest is up to subclasses,
 *   to keep things flexible.
 *
 *   This class (and supporting types/helpers) is all about making
 *   multiformat, multithreaded image processing manageable and performant.
 *
 *   The main idea is that kernels do not need a switch/case in the inner section
 *   of the kernel to put a format-specific operation. The PixFormat::load()/store()
 *   helpers take care of conversions for the most part to process in float samples,
 *   but for performant kernels one has the option to bifurcate if needed,
 *   e.g. one path for integer and different for float (see DkLutKernel for example)
 *
 *   Subclasses only need to create a simple function exactly like:
 *
 *   template<typename Format>
 *   static bool kernel(Name& self, const Range& range);
 *
 *   OR
 *
 *   template<typename SrcFormat, typename DstFormat>
 *   static bool kernel(Name& self, const Range& range);
 *
 *   The Format parameter(s) are a PixFmt_* type, which provide all of the
 *   constants and types needed to handle the format.
 *
 *   constexpr stuff is used here to prevent compiling kernel variants we don't use
 */
template<typename Kernel>
class DkKernelBase
{
public:
    // setup image formats etc and call dispatch()
    // return false on errors (input unsuitable, out of memory, perhaps cancelled operation etc)
    virtual bool run() = 0;

    // returns image result
    virtual QImage result() const = 0;

    virtual ~DkKernelBase() = default;

protected:
    using EntryPoint = bool (*)(Kernel &, const DkWorkRange &); // signature of kernel entry point
    using DispatchTable = std::array<EntryPoint, (int)ImgFmt::NFormats>; // map format to kernel entry point

    using FmtList = std::array<ImgFmt, (int)ImgFmt::NFormats>; // fixed-size required for constexpr, we'll ignore null
    using FmtPair = std::array<ImgFmt, 2>;
    using FmtMap = std::array<FmtPair, (int)ImgFmt::NFormats>;

    // flags to specify what a kernel supports/requires
    enum KernelCapability {
        cap_bgr = 0x1, // kernel supports b-g-r-(a) byte order
        cap_rgb = 0x2, // kernel supports r-g-b-(a) byte order
        cap_gray = 0x4, // kernel supports grayscale images
        cap_rgb_invariant = 0x8, // kernel doesn't depend on rgb channel order
    };

    // build array of ImgFmt based on kernel caps
    static constexpr FmtList listForKernelCaps(int caps /* KernelCapability */)
    {
        FmtList list = {}; // fixed-size list for consteval; end of list is 0/Fmt::Invalid
        size_t i = 0;
        if (caps & cap_gray) {
            list[i++] = ImgFmt::Gray8;
            list[i++] = ImgFmt::Gray16;
        }
        if (caps & cap_bgr) {
            list[i++] = ImgFmt::BGR888;
            list[i++] = ImgFmt::ARGB32;
        }
        if (caps & cap_rgb) {
            list[i++] = ImgFmt::RGB888;
            list[i++] = ImgFmt::RGBA8888;
            list[i++] = ImgFmt::RGBA64;
            list[i++] = ImgFmt::RGBAFP32;
        }
        if (caps & cap_rgb_invariant) {
            list[i++] = ImgFmt::BGR888;
            list[i++] = ImgFmt::ARGB32;
            list[i++] = ImgFmt::RGBA64;
            list[i++] = ImgFmt::RGBAFP32;
        }

        if (i >= list.size()) {
            throw;
        }

        return list;
    }

    // find dst fmt for src fmt
    static constexpr ImgFmt findFormat(const FmtMap &map, ImgFmt srcFmt)
    {
        for (auto &pair : map) {
            if (pair[0] == srcFmt) {
                return pair[1];
            }
        }
        return ImgFmt::Invalid;
    }

    // build dispatch table; because it is constexpr the compiler only emits templates kernel requires
    static constexpr DispatchTable makeTable(FmtList formats)
    {
        DispatchTable table = {}; // unused entries are nullptr
        for (auto f : formats) {
            int i = (int)f;
            // clang-format off
            switch(f) {
            case ImgFmt::Invalid:  table[i] = nullptr; break;
            case ImgFmt::Gray8:    table[i] = &Kernel::template kernel<PixFmt_Gray8>; break;
            case ImgFmt::Gray16:   table[i] = &Kernel::template kernel<PixFmt_Gray16>; break;
            case ImgFmt::BGR888:   table[i] = &Kernel::template kernel<PixFmt_BGR888>; break;
            case ImgFmt::RGB888:   table[i] = &Kernel::template kernel<PixFmt_RGB888>; break;
            case ImgFmt::ARGB32:   table[i] = &Kernel::template kernel<PixFmt_ARGB32>; break;
            case ImgFmt::RGBA8888: table[i] = &Kernel::template kernel<PixFmt_RGBA8888>; break;
            case ImgFmt::RGBA64:   table[i] = &Kernel::template kernel<PixFmt_RGBA64>; break;
            case ImgFmt::RGBAFP32: table[i] = &Kernel::template kernel<PixFmt_RGBAFP32>; break;
            default: throw; // bad input or missing case label
            }
            // clang-format on
        }
        return table;
    }

    // mapped dispatch helper, for kernels with two template parameters
    template<ImgFmt SrcFmt>
    static constexpr void setTablePair(DispatchTable &table, ImgFmt srcFmt, ImgFmt dstFmt)
    {
        using SrcType = typename ImgFmtToPixFormat<SrcFmt>::type;

        int i = (int)srcFmt;

        // clang-format off
        switch (dstFmt) {
        case ImgFmt::Gray8:    table[i] = &Kernel::template kernel<SrcType, PixFmt_Gray8>; break;
        case ImgFmt::Gray16:   table[i] = &Kernel::template kernel<SrcType, PixFmt_Gray16>; break;
        case ImgFmt::BGR888:   table[i] = &Kernel::template kernel<SrcType, PixFmt_BGR888>; break;
        case ImgFmt::RGB888:   table[i] = &Kernel::template kernel<SrcType, PixFmt_RGB888>; break;
        case ImgFmt::ARGB32:   table[i] = &Kernel::template kernel<SrcType, PixFmt_ARGB32>; break;
        case ImgFmt::RGBA8888: table[i] = &Kernel::template kernel<SrcType, PixFmt_RGBA8888>; break;
        case ImgFmt::RGBA64:   table[i] = &Kernel::template kernel<SrcType, PixFmt_RGBA64>; break;
        case ImgFmt::RGBAFP32: table[i] = &Kernel::template kernel<SrcType, PixFmt_RGBAFP32>; break;
        default: throw; // bad input or missing case label
        }
        // clang-format on
    }

    // mapped dispatch table, for kernels with two template parameters,
    // with a different source and destination pixel format
    static constexpr DispatchTable makeTable(FmtMap map)
    {
        DispatchTable table = {}; // unused entries are nullptr
        for (auto pair : map) {
            auto srcFmt = pair[0];
            auto dstFmt = pair[1];

            // clang-format off
            switch (srcFmt) {
            case ImgFmt::Invalid:  break; // already initialized to 0
            case ImgFmt::Gray8:    setTablePair<ImgFmt::Gray8>(table, srcFmt, dstFmt); break;
            case ImgFmt::Gray16:   setTablePair<ImgFmt::Gray16>(table, srcFmt, dstFmt); break;
            case ImgFmt::BGR888:   setTablePair<ImgFmt::BGR888>(table, srcFmt, dstFmt); break;
            case ImgFmt::RGB888:   setTablePair<ImgFmt::RGB888>(table, srcFmt, dstFmt); break;
            case ImgFmt::ARGB32:   setTablePair<ImgFmt::ARGB32>(table, srcFmt, dstFmt); break;
            case ImgFmt::RGBA8888: setTablePair<ImgFmt::RGBA8888>(table, srcFmt, dstFmt); break;
            case ImgFmt::RGBA64:   setTablePair<ImgFmt::RGBA64>(table, srcFmt, dstFmt); break;
            case ImgFmt::RGBAFP32: setTablePair<ImgFmt::RGBAFP32>(table, srcFmt, dstFmt); break;
            default:            throw; // bad input or missing case label
            }
            // clang-format on
        }
        return table;
    }

    // invoke kernel from dispatch table
    static bool dispatch(const DispatchTable &table,
                         QImage::Format qtFormat,
                         Kernel &kernel,
                         const DkWorkRange &range,
                         bool serial = false)
    {
        auto fmt = qtImageFormatToNative(qtFormat);

        EntryPoint fn = table[(int)fmt];
        if (!fn) {
            qWarning() << "[Kernel Dispatch] unsupported format" << qtFormat << (int)fmt;
            return false;
        }

        bool ok;
        if (serial) {
            ok = fn(kernel, range);
        } else {
            auto slices = range.partition();
            QFuture<bool> f = QtConcurrent::mappedReduced<bool>(
                slices,
                [&](const DkWorkRange &slice) {
                    return fn(kernel, slice);
                },
                [&](bool &accum, bool result) {
                    accum = accum && result; // if one slice fails, return false in the end
                },
                true,
                QtConcurrent::UnorderedReduce);
            ok = f.takeResult();
        }
        return ok;
    }
};

#ifdef WITH_OPENCV

// helper for per-channel processing
template<typename Format, typename Lambda>
static void forEachChannel(cv::Mat &mat, const DkWorkRange &range, Lambda func)
{
    Q_ASSERT(Format::isCompatibleWith(mat));
    Q_ASSERT(range.isWithin(0, mat.rows));

    using ChannelType = typename Format::ChannelType;
    const int channelsPerRow = mat.cols * Format::Channels;
    constexpr int numChannels = qMin(3, Format::Channels); // skip alpha channel (use per-pixel mode for that)

    for (int row = range.begin; row < range.end; ++row) {
        auto *rowPtr = mat.ptr<ChannelType>(row);
        auto *const endPtr = rowPtr + channelsPerRow;
        for (; rowPtr < endPtr; rowPtr += Format::Channels) {
            for (int channel = 0; channel < numChannels; ++channel) {
                func(rowPtr[channel], channel);
            }
        }
    }
}

// helper for per-pixel processing
template<typename Format, typename Lambda>
static void forEachPixel(cv::Mat &mat, const DkWorkRange &range, Lambda func)
{
    Q_ASSERT(Format::isCompatibleWith(mat));
    Q_ASSERT(range.isWithin(0, mat.rows));

    using ChannelType = typename Format::ChannelType;
    const int channelsPerRow = mat.cols * Format::Channels;

    for (int row = range.begin; row < range.end; ++row) {
        auto *rowPtr = mat.ptr<ChannelType>(row);
        auto *const endPtr = rowPtr + channelsPerRow;
        for (; rowPtr < endPtr; rowPtr += Format::Channels) {
            func(rowPtr);
        }
    }
}

#endif // WITH_OPENCV

} // namespace nmc
