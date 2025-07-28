/**
 * DRIF - Developers RAW image format
 *
 * MIT License
 * Copyright (c) 2018 Ovidiu Ionescu
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdint.h>
#include <stdio.h>

#if defined(Q_OS_MAC) || defined(Q_OS_OPENBSD) || defined(Q_OS_FREEBSD)
#include <stdlib.h>
#else
#include <malloc.h>
#endif

// DRIF Public API

#ifndef DRIF_API_H_INC
#define DRIF_API_H_INC

// DRIF version
#define DRIF_VER (0x01 | (0x00 << 8) | (0x00 << 16) | ('S' << 24))

// footer size reserved at end of the file
#define DRIF_FOOTER_SZ 512

// DRIF magic
#define DRIF_MAGIC ('D' | ('R' << 8) | ('I' << 16) | ('F' << 24))

#ifndef DRIFAPI
#ifdef __cplusplus
#define DRIFAPI extern "C"
#else
#define DRIFAPI extern
#endif
#endif

// DRIF error codes
enum {
    DRIF_OK = 0,
    DRIF_ERR_NULLPTR,
    DRIF_ERR_FMT,
    DRIF_ERR_MAX_SIZE,
    DRIF_ERR_MIN_SIZE,
    DRIF_ERR_READ_FILE,
    DRIF_ERR_WRITE_FILE,
    DRIF_ERR_NO_MEMORY,
};

// DRIF formats
enum {
    DRIF_FMT_YUV = (1 << 0),
    DRIF_FMT_RGB = (1 << 1),
};

#define DRIF_MKFMT(fmt_type, fmt_id) (((fmt_type) << 16) | (fmt_id))

enum {
    DRIF_FMT_RGB888 = DRIF_MKFMT(DRIF_FMT_RGB, 0),
    DRIF_FMT_BGR888 = DRIF_MKFMT(DRIF_FMT_RGB, 1),
    DRIF_FMT_RGB888P = DRIF_MKFMT(DRIF_FMT_RGB, 2),
    DRIF_FMT_BGR888P = DRIF_MKFMT(DRIF_FMT_RGB, 3),

    DRIF_FMT_RGBA8888 = DRIF_MKFMT(DRIF_FMT_RGB, 4),
    DRIF_FMT_BGRA8888 = DRIF_MKFMT(DRIF_FMT_RGB, 5),
    DRIF_FMT_RGBA8888P = DRIF_MKFMT(DRIF_FMT_RGB, 6),
    DRIF_FMT_BGRA8888P = DRIF_MKFMT(DRIF_FMT_RGB, 7),

    DRIF_FMT_GRAY = DRIF_MKFMT(DRIF_FMT_YUV, 0),

    DRIF_FMT_YUV420P = DRIF_MKFMT(DRIF_FMT_YUV, 1),
    DRIF_FMT_YVU420P = DRIF_MKFMT(DRIF_FMT_YUV, 2),
    DRIF_FMT_NV12 = DRIF_MKFMT(DRIF_FMT_YUV, 3),
    DRIF_FMT_NV21 = DRIF_MKFMT(DRIF_FMT_YUV, 4),
};

// DRIF max / min dimensions
#define DRIF_MAX_W (1 << 15)
#define DRIF_MAX_H (1 << 15)

#define DRIF_MIN_W (1)
#define DRIF_MIN_H (1)

// DRIF Api functions
DRIFAPI uint8_t *drifLoadImg(const char *szFileName, uint32_t *w, uint32_t *h, uint32_t *f);
DRIFAPI int drifSaveImg(const char *szFileName, uint32_t w, uint32_t h, uint32_t f, const void *data);
DRIFAPI void drifFreeImg(void *ptr_from_load);
DRIFAPI uint32_t drifGetSize(const uint32_t w, const uint32_t h, const uint32_t f);

#endif // DRIF_API_H_INC

// DRIF Private
#ifdef DRIF_IMAGE_IMPL

// DRIF footer structure
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t w;
    uint32_t h;
    uint32_t f;

} drif_footer_t;

static int isDrifFmtValid(uint32_t f)
{
    switch (f) {
    case DRIF_FMT_RGB888:
    case DRIF_FMT_BGR888:
    case DRIF_FMT_RGB888P:
    case DRIF_FMT_BGR888P:
    case DRIF_FMT_RGBA8888:
    case DRIF_FMT_BGRA8888:
    case DRIF_FMT_RGBA8888P:
    case DRIF_FMT_BGRA8888P:
    case DRIF_FMT_GRAY:
    case DRIF_FMT_YUV420P:
    case DRIF_FMT_YVU420P:
    case DRIF_FMT_NV12:
    case DRIF_FMT_NV21:
        return 1;
    }

    return 0;
}

uint8_t *drifLoadImg(const char *szFileName, uint32_t *w, uint32_t *h, uint32_t *f)
{
    // check input params
    if (NULL == szFileName || NULL == w || NULL == h || NULL == f)
        return NULL;

    // open file for reading
    FILE *fp = fopen(szFileName, "rb");

    // check if file opened ok
    if (NULL == fp)
        return NULL;

    // move to footer offeset
    fseek(fp, 0L, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, sz - DRIF_FOOTER_SZ, SEEK_SET);

    // read footer
    char buffer[DRIF_FOOTER_SZ] = {0};
    fread((void *)buffer, DRIF_FOOTER_SZ, 1, fp);

    drif_footer_t *footer = (drif_footer_t *)buffer;

    // check magic & format
    if (DRIF_MAGIC != footer->magic || !isDrifFmtValid(footer->f)) {
        fclose(fp);
        return NULL;
    }

    // read image
    uint32_t dataSize = drifGetSize(footer->w, footer->h, footer->f);
    void *img = malloc(dataSize);

    if (NULL == img) {
        fclose(fp);
        return NULL;
    }

    // move at file start
    fseek(fp, 0L, SEEK_SET);
    size_t nElem = fread(img, dataSize, 1, fp);
    fclose(fp);

    if (1 != nElem) {
        free(img);
        return NULL;
    }

    // export image info
    *w = footer->w;
    *h = footer->h;
    *f = footer->f;

    return (uint8_t *)img;
}

int drifSaveImg(const char *szFileName, const uint32_t w, const uint32_t h, const uint32_t f, const void *data)
{
    // check input params
    if (NULL == szFileName || NULL == data)
        return DRIF_ERR_NULLPTR;

    // check params
    if (!isDrifFmtValid(f))
        return DRIF_ERR_FMT;

    // check for max dimensions
    if (w > DRIF_MAX_W || h > DRIF_MAX_H)
        return DRIF_ERR_MAX_SIZE;

    // check for min dimensions
    if (w < DRIF_MIN_W || h < DRIF_MIN_H)
        return DRIF_ERR_MIN_SIZE;

    // open file for writing
    FILE *fp = fopen(szFileName, "wb");

    // check if file opened ok
    if (NULL == fp)
        return DRIF_ERR_WRITE_FILE;

    // prepare footer
    char buffer[DRIF_FOOTER_SZ] = {0};
    drif_footer_t *footer = (drif_footer_t *)buffer;
    footer->magic = DRIF_MAGIC;
    footer->version = DRIF_VER;
    footer->w = w;
    footer->h = h;
    footer->f = f;

    // compute image size
    uint32_t dataSize = drifGetSize(w, h, f);
    // write image
    if (1 != fwrite(data, dataSize, 1, fp))
        return DRIF_ERR_WRITE_FILE;

    // write footer
    if (1 != fwrite((void *)buffer, DRIF_FOOTER_SZ, 1, fp))
        return DRIF_ERR_WRITE_FILE;

    // close file and flush contents
    fclose(fp);

    return DRIF_OK;
}

void drifFreeImg(void *ptr_from_load)
{
    free(ptr_from_load);
}

uint32_t drifGetSize(const uint32_t w, const uint32_t h, const uint32_t f)
{
    switch (f) {
    case DRIF_FMT_RGB888:
    case DRIF_FMT_BGR888:
    case DRIF_FMT_RGB888P:
    case DRIF_FMT_BGR888P:
        return w * h * 3;

    case DRIF_FMT_RGBA8888:
    case DRIF_FMT_BGRA8888:
    case DRIF_FMT_RGBA8888P:
    case DRIF_FMT_BGRA8888P:
        return w * h * 4;

    case DRIF_FMT_YUV420P:
    case DRIF_FMT_YVU420P:
    case DRIF_FMT_NV12:
    case DRIF_FMT_NV21:
        return w * h * 3 / 2;

    case DRIF_FMT_GRAY:
        return w * h;

    default:
        break;
    }

    return 0;
}

#endif
