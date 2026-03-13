#pragma once

#include "DkFileInfo.h"
#include "DkImageStorage.h"

#include <QImageReader>

namespace nmc
{
/**
 * @brief Thumbnail disk cache following XDG specification
 *
 * @note This object is reentrant, we can construct and load/save
 *       thumbs from any thread.
 */
class DkCachedThumb
{
public:
    /**
     * @brief Async cleanup of the cache
     */
    static void DllCoreExport cleanup();

    /**
     * @brief Return true if shared cache enabled on XDG-capable system
     * @return
     */
    static bool isXdgCompliant();

    /**
     * @brief setup for loading or saving thumb from/to cache
     * @param fileInfo original file
     * @param size minimum thumbnail size wrt constraint
     * @param constraint how the thumbnail will be resized
     */
    DkCachedThumb(DkFileInfo &fileInfo, int size, ScaleConstraint constraint);

    /**
     * @brief read from thumbnail cache
     * @note the returned image could be larger than needed, but never smaller (to reduce blur)
     * @return valid thumbnail (all freshness checks passed) or null image
     */
    QImage load();

    /**
     * @brief write to thumbnail cache, if requirements are satisfied
     * @param img image to be scaled and saved to cache
     * @param loadedBinSize if >0, the bin this image came from; allows saving smaller thumbs from larger ones
     */
    void save(const QImage &img, int loadedBinSize = 0);

private:
    // Delete old cache files
    static void cleanupSync();

    // Top-level cache directory ($HOME/.cache/thumbnails)
    static QString cacheHome();

    // Return true if image size(sz) is large enough to meet size/constraint
    bool isLargeEnough(const QSize &sz) const;

    // Return modified date of original file, best effort if archived/zipped
    QDateTime lastModified() const;

    // xdg folder size and filesystem name
    struct XdgBin {
        int size{0};
        const char *name{"invalid"};
    };

    static constexpr std::array<XdgBin, 4> kXdgBins = {
        {{128, "normal"}, {256, "large"}, {512, "x-large"}, {1024, "xx-large"}}};

    const DkFileInfo mFileInfo;
    const int mSize;
    const ScaleConstraint mConstraint;

    QByteArray mUri; // original file's uri
    QByteArray mCacheFileName; // hash of uri + file extension
};

}
