/* -*- C++ -*-
 * File: libraw_cxx.cpp
 * Copyright 2008-2016 LibRaw LLC (info@libraw.org)
 * Created: Sat Mar  8 , 2008
 *
 * LibRaw C++ interface (implementation)

LibRaw is free software; you can redistribute it and/or modify
it under the terms of the one of two licenses as you choose:

1. GNU LESSER GENERAL PUBLIC LICENSE version 2.1
   (See file LICENSE.LGPL provided in LibRaw distribution archive for details).

2. COMMON DEVELOPMENT AND DISTRIBUTION LICENSE (CDDL) Version 1.0
   (See file LICENSE.CDDL provided in LibRaw distribution archive for details).

 */

#include <math.h>
#include <errno.h>
#include <float.h>
#include <new>
#include <exception>
#include <sys/types.h>
#include <sys/stat.h>
#if !defined(_WIN32) && !defined(__MINGW32__)
#include <netinet/in.h>
#else
#include <winsock2.h>
#endif
#define LIBRAW_LIBRARY_BUILD
#include "libraw/libraw.h"
#include "internal/defines.h"
#ifdef USE_ZLIB
#include <zlib.h>
#endif

#ifdef USE_RAWSPEED
#include "../RawSpeed/rawspeed_xmldata.cpp"
#include <RawSpeed/StdAfx.h>
#include <RawSpeed/FileMap.h>
#include <RawSpeed/RawParser.h>
#include <RawSpeed/RawDecoder.h>
#include <RawSpeed/CameraMetaData.h>
#include <RawSpeed/ColorFilterArray.h>
#endif

#ifdef USE_DNGSDK
#include "dng_host.h"
#include "dng_negative.h"
#include "dng_simple_image.h"
#include "dng_info.h"
#endif

#include "libraw_fuji_compressed.cpp"

#ifdef __cplusplus
extern "C" {
#endif
void default_memory_callback(void *, const char *file, const char *where)
{
  fprintf(stderr, "%s: Out of memory in %s\n", file ? file : "unknown file", where);
}

void default_data_callback(void *, const char *file, const int offset)
{
  if (offset < 0)
    fprintf(stderr, "%s: Unexpected end of file\n", file ? file : "unknown file");
  else
    fprintf(stderr, "%s: data corrupted at %d\n", file ? file : "unknown file", offset);
}
const char *libraw_strerror(int e)
{
  enum LibRaw_errors errorcode = (LibRaw_errors)e;
  switch (errorcode)
  {
  case LIBRAW_SUCCESS:
    return "No error";
  case LIBRAW_UNSPECIFIED_ERROR:
    return "Unspecified error";
  case LIBRAW_FILE_UNSUPPORTED:
    return "Unsupported file format or not RAW file";
  case LIBRAW_REQUEST_FOR_NONEXISTENT_IMAGE:
    return "Request for nonexisting image number";
  case LIBRAW_OUT_OF_ORDER_CALL:
    return "Out of order call of libraw function";
  case LIBRAW_NO_THUMBNAIL:
    return "No thumbnail in file";
  case LIBRAW_UNSUPPORTED_THUMBNAIL:
    return "Unsupported thumbnail format";
  case LIBRAW_INPUT_CLOSED:
    return "No input stream, or input stream closed";
  case LIBRAW_UNSUFFICIENT_MEMORY:
    return "Unsufficient memory";
  case LIBRAW_DATA_ERROR:
    return "Corrupted data or unexpected EOF";
  case LIBRAW_IO_ERROR:
    return "Input/output error";
  case LIBRAW_CANCELLED_BY_CALLBACK:
    return "Cancelled by user callback";
  case LIBRAW_BAD_CROP:
    return "Bad crop box";
  default:
    return "Unknown error code";
  }
}

#ifdef __cplusplus
}
#endif

#define Sigma_X3F 22

const double LibRaw_constants::xyz_rgb[3][3] = {
    {0.412453, 0.357580, 0.180423}, {0.212671, 0.715160, 0.072169}, {0.019334, 0.119193, 0.950227}};

const float LibRaw_constants::d65_white[3] = {0.950456f, 1.0f, 1.088754f};

#define P1 imgdata.idata
#define S imgdata.sizes
#define O imgdata.params
#define C imgdata.color
#define T imgdata.thumbnail
#define IO libraw_internal_data.internal_output_params
#define ID libraw_internal_data.internal_data

#define EXCEPTION_HANDLER(e)                                                                                           \
  do                                                                                                                   \
  {                                                                                                                    \
    /* fprintf(stderr,"Exception %d caught\n",e);*/                                                                    \
    switch (e)                                                                                                         \
    {                                                                                                                  \
    case LIBRAW_EXCEPTION_ALLOC:                                                                                       \
      recycle();                                                                                                       \
      return LIBRAW_UNSUFFICIENT_MEMORY;                                                                               \
    case LIBRAW_EXCEPTION_DECODE_RAW:                                                                                  \
    case LIBRAW_EXCEPTION_DECODE_JPEG:                                                                                 \
      recycle();                                                                                                       \
      return LIBRAW_DATA_ERROR;                                                                                        \
    case LIBRAW_EXCEPTION_DECODE_JPEG2000:                                                                             \
      recycle();                                                                                                       \
      return LIBRAW_DATA_ERROR;                                                                                        \
    case LIBRAW_EXCEPTION_IO_EOF:                                                                                      \
    case LIBRAW_EXCEPTION_IO_CORRUPT:                                                                                  \
      recycle();                                                                                                       \
      return LIBRAW_IO_ERROR;                                                                                          \
    case LIBRAW_EXCEPTION_CANCELLED_BY_CALLBACK:                                                                       \
      recycle();                                                                                                       \
      return LIBRAW_CANCELLED_BY_CALLBACK;                                                                             \
    case LIBRAW_EXCEPTION_BAD_CROP:                                                                                    \
      recycle();                                                                                                       \
      return LIBRAW_BAD_CROP;                                                                                          \
    default:                                                                                                           \
      return LIBRAW_UNSPECIFIED_ERROR;                                                                                 \
    }                                                                                                                  \
  } while (0)

const char *LibRaw::version() { return LIBRAW_VERSION_STR; }
int LibRaw::versionNumber() { return LIBRAW_VERSION; }
const char *LibRaw::strerror(int p) { return libraw_strerror(p); }

unsigned LibRaw::capabilities()
{
  unsigned ret = 0;
#ifdef USE_RAWSPEED
  ret |= LIBRAW_CAPS_RAWSPEED;
#endif
#ifdef USE_DNGSDK
  ret |= LIBRAW_CAPS_DNGSDK;
#endif
#ifdef LIBRAW_DEMOSAIC_PACK_GPL2
  ret |= LIBRAW_CAPS_DEMOSAICSGPL2;
#endif
#ifdef LIBRAW_DEMOSAIC_PACK_GPL3
  ret |= LIBRAW_CAPS_DEMOSAICSGPL3;
#endif
  return ret;
}

unsigned LibRaw::parse_custom_cameras(unsigned limit, libraw_custom_camera_t table[], char **list)
{
  if (!list)
    return 0;
  unsigned index = 0;
  for (int i = 0; i < limit; i++)
  {
    if (!list[i])
      break;
    if (strlen(list[i]) < 10)
      continue;
    char *string = (char *)malloc(strlen(list[i]) + 1);
    strcpy(string, list[i]);
    char *start = string;
    memset(&table[index], 0, sizeof(table[0]));
    for (int j = 0; start && j < 14; j++)
    {
      char *end = strchr(start, ',');
      if (end)
      {
        *end = 0;
        end++;
      } // move to next char
      while (isspace(*start) && *start)
        start++; // skip leading spaces?
      unsigned val = strtol(start, 0, 10);
      switch (j)
      {
      case 0:
        table[index].fsize = val;
        break;
      case 1:
        table[index].rw = val;
        break;
      case 2:
        table[index].rh = val;
        break;
      case 3:
        table[index].lm = val;
        break;
      case 4:
        table[index].tm = val;
        break;
      case 5:
        table[index].rm = val;
        break;
      case 6:
        table[index].bm = val;
        break;
      case 7:
        table[index].lf = val;
        break;
      case 8:
        table[index].cf = val;
        break;
      case 9:
        table[index].max = val;
        break;
      case 10:
        table[index].flags = val;
        break;
      case 11:
        strncpy(table[index].t_make, start, sizeof(table[index].t_make) - 1);
        break;
      case 12:
        strncpy(table[index].t_model, start, sizeof(table[index].t_model) - 1);
        break;
      case 13:
        table[index].offset = val;
        break;
      default:
        break;
      }
      start = end;
    }
    free(string);
    if (table[index].t_make[0])
      index++;
  }
  return index;
}

void LibRaw::derror()
{
  if (!libraw_internal_data.unpacker_data.data_error && libraw_internal_data.internal_data.input)
  {
    if (libraw_internal_data.internal_data.input->eof())
    {
      if (callbacks.data_cb)
        (*callbacks.data_cb)(callbacks.datacb_data, libraw_internal_data.internal_data.input->fname(), -1);
      throw LIBRAW_EXCEPTION_IO_EOF;
    }
    else
    {
      if (callbacks.data_cb)
        (*callbacks.data_cb)(callbacks.datacb_data, libraw_internal_data.internal_data.input->fname(),
                             libraw_internal_data.internal_data.input->tell());
      // throw LIBRAW_EXCEPTION_IO_CORRUPT;
    }
  }
  libraw_internal_data.unpacker_data.data_error++;
}

void LibRaw::dcraw_clear_mem(libraw_processed_image_t *p)
{
  if (p)
    ::free(p);
}

int LibRaw::is_sraw() { return load_raw == &LibRaw::canon_sraw_load_raw || load_raw == &LibRaw::nikon_load_sraw; }
int LibRaw::is_coolscan_nef() { return load_raw == &LibRaw::nikon_coolscan_load_raw; }

int LibRaw::is_nikon_sraw() { return load_raw == &LibRaw::nikon_load_sraw; }
int LibRaw::sraw_midpoint()
{
  if (load_raw == &LibRaw::canon_sraw_load_raw)
    return 8192;
  else if (load_raw == &LibRaw::nikon_load_sraw)
    return 2048;
  else
    return 0;
}

#ifdef USE_RAWSPEED
using namespace RawSpeed;
class CameraMetaDataLR : public CameraMetaData
{
public:
  CameraMetaDataLR() : CameraMetaData() {}
  CameraMetaDataLR(char *filename) : CameraMetaData(filename) {}
  CameraMetaDataLR(char *data, int sz);
};

CameraMetaDataLR::CameraMetaDataLR(char *data, int sz) : CameraMetaData()
{
  ctxt = xmlNewParserCtxt();
  if (ctxt == NULL)
  {
    ThrowCME("CameraMetaData:Could not initialize context.");
  }

  xmlResetLastError();
  doc = xmlCtxtReadMemory(ctxt, data, sz, "", NULL, XML_PARSE_DTDVALID);

  if (doc == NULL)
  {
    ThrowCME("CameraMetaData: XML Document could not be parsed successfully. Error was: %s", ctxt->lastError.message);
  }

  if (ctxt->valid == 0)
  {
    if (ctxt->lastError.code == 0x5e)
    {
      // printf("CameraMetaData: Unable to locate DTD, attempting to ignore.");
    }
    else
    {
      ThrowCME("CameraMetaData: XML file does not validate. DTD Error was: %s", ctxt->lastError.message);
    }
  }

  xmlNodePtr cur;
  cur = xmlDocGetRootElement(doc);
  if (xmlStrcmp(cur->name, (const xmlChar *)"Cameras"))
  {
    ThrowCME("CameraMetaData: XML document of the wrong type, root node is not cameras.");
    return;
  }

  cur = cur->xmlChildrenNode;
  while (cur != NULL)
  {
    if ((!xmlStrcmp(cur->name, (const xmlChar *)"Camera")))
    {
      Camera *camera = new Camera(doc, cur);
      addCamera(camera);

      // Create cameras for aliases.
      for (unsigned int i = 0; i < camera->aliases.size(); i++)
      {
        addCamera(new Camera(camera, i));
      }
    }
    cur = cur->next;
  }
  if (doc)
    xmlFreeDoc(doc);
  doc = 0;
  if (ctxt)
    xmlFreeParserCtxt(ctxt);
  ctxt = 0;
}

#define RAWSPEED_DATA_COUNT (sizeof(_rawspeed_data_xml) / sizeof(_rawspeed_data_xml[0]))
static CameraMetaDataLR *make_camera_metadata()
{
  int len = 0, i;
  for (i = 0; i < RAWSPEED_DATA_COUNT; i++)
    if (_rawspeed_data_xml[i])
    {
      len += strlen(_rawspeed_data_xml[i]);
    }
  char *rawspeed_xml = (char *)calloc(len + 1, sizeof(_rawspeed_data_xml[0][0]));
  if (!rawspeed_xml)
    return NULL;
  int offt = 0;
  for (i = 0; i < RAWSPEED_DATA_COUNT; i++)
    if (_rawspeed_data_xml[i])
    {
      int ll = strlen(_rawspeed_data_xml[i]);
      if (offt + ll > len)
        break;
      memmove(rawspeed_xml + offt, _rawspeed_data_xml[i], ll);
      offt += ll;
    }
  rawspeed_xml[offt] = 0;
  CameraMetaDataLR *ret = NULL;
  try
  {
    ret = new CameraMetaDataLR(rawspeed_xml, offt);
  }
  catch (...)
  {
    // Mask all exceptions
  }
  free(rawspeed_xml);
  return ret;
}

#endif

#define ZERO(a) memset(&a, 0, sizeof(a))

static void cleargps(libraw_gps_info_t *q)
{
  for (int i = 0; i < 3; i++)
    q->latitude[i] = q->longtitude[i] = q->gpstimestamp[i] = 0.f;
  q->altitude = 0.f;
  q->altref = q->latref = q->longref = q->gpsstatus = q->gpsparsed = 0;
}

LibRaw::LibRaw(unsigned int flags)
{
  double aber[4] = {1, 1, 1, 1};
  double gamm[6] = {0.45, 4.5, 0, 0, 0, 0};
  unsigned greybox[4] = {0, 0, UINT_MAX, UINT_MAX};
  unsigned cropbox[4] = {0, 0, UINT_MAX, UINT_MAX};
#ifdef DCRAW_VERBOSE
  verbose = 1;
#else
  verbose = 0;
#endif
  ZERO(imgdata);

  cleargps(&imgdata.other.parsed_gps);
  ZERO(libraw_internal_data);
  ZERO(callbacks);

  _rawspeed_camerameta = _rawspeed_decoder = NULL;
  dnghost = NULL;
  _x3f_data = NULL;

#ifdef USE_RAWSPEED
  CameraMetaDataLR *camerameta = make_camera_metadata(); // May be NULL in case of exception in make_camera_metadata()
  _rawspeed_camerameta = static_cast<void *>(camerameta);
#endif
  callbacks.mem_cb = (flags & LIBRAW_OPIONS_NO_MEMERR_CALLBACK) ? NULL : &default_memory_callback;
  callbacks.data_cb = (flags & LIBRAW_OPIONS_NO_DATAERR_CALLBACK) ? NULL : &default_data_callback;
  callbacks.exif_cb = NULL; // no default callback
  memmove(&imgdata.params.aber, &aber, sizeof(aber));
  memmove(&imgdata.params.gamm, &gamm, sizeof(gamm));
  memmove(&imgdata.params.greybox, &greybox, sizeof(greybox));
  memmove(&imgdata.params.cropbox, &cropbox, sizeof(cropbox));

  imgdata.params.bright = 1;
  imgdata.params.use_camera_matrix = 1;
  imgdata.params.user_flip = -1;
  imgdata.params.user_black = -1;
  imgdata.params.user_cblack[0] = imgdata.params.user_cblack[1] = imgdata.params.user_cblack[2] =
      imgdata.params.user_cblack[3] = -1000001;
  imgdata.params.user_sat = -1;
  imgdata.params.user_qual = -1;
  imgdata.params.output_color = 1;
  imgdata.params.output_bps = 8;
  imgdata.params.use_fuji_rotate = 1;
  imgdata.params.exp_shift = 1.0;
  imgdata.params.auto_bright_thr = LIBRAW_DEFAULT_AUTO_BRIGHTNESS_THRESHOLD;
  imgdata.params.adjust_maximum_thr = LIBRAW_DEFAULT_ADJUST_MAXIMUM_THRESHOLD;
  imgdata.params.use_rawspeed = 1;
  imgdata.params.use_dngsdk = LIBRAW_DNG_DEFAULT;
  imgdata.params.no_auto_scale = 0;
  imgdata.params.no_interpolation = 0;
  imgdata.params.raw_processing_options = LIBRAW_PROCESSING_DP2Q_INTERPOLATERG | LIBRAW_PROCESSING_DP2Q_INTERPOLATEAF |
                                          LIBRAW_PROCESSING_CONVERTFLOAT_TO_INT;
  imgdata.params.sony_arw2_posterization_thr = 0;
  imgdata.params.green_matching = 0;
  imgdata.params.custom_camera_strings = 0;
  imgdata.params.coolscan_nef_gamma = 1.0f;
  imgdata.parent_class = this;
  imgdata.progress_flags = 0;
  imgdata.color.baseline_exposure = -999.f;
  _exitflag = 0;
  tls = new LibRaw_TLS;
  tls->init();

  interpolate_bayer = 0;
  interpolate_xtrans = 0;
}

int LibRaw::set_rawspeed_camerafile(char *filename)
{
#ifdef USE_RAWSPEED
  try
  {
    CameraMetaDataLR *camerameta = new CameraMetaDataLR(filename);
    if (_rawspeed_camerameta)
    {
      CameraMetaDataLR *d = static_cast<CameraMetaDataLR *>(_rawspeed_camerameta);
      delete d;
    }
    _rawspeed_camerameta = static_cast<void *>(camerameta);
  }
  catch (...)
  {
    // just return error code
    return -1;
  }
#endif
  return 0;
}

LibRaw::~LibRaw()
{
  recycle();
  delete tls;
#ifdef USE_RAWSPEED
  if (_rawspeed_camerameta)
  {
    CameraMetaDataLR *cmeta = static_cast<CameraMetaDataLR *>(_rawspeed_camerameta);
    delete cmeta;
    _rawspeed_camerameta = NULL;
  }
#endif
}

void *LibRaw::malloc(size_t t)
{
  void *p = memmgr.malloc(t);
  if (!p)
    throw LIBRAW_EXCEPTION_ALLOC;
  return p;
}
void *LibRaw::realloc(void *q, size_t t)
{
  void *p = memmgr.realloc(q, t);
  if (!p)
    throw LIBRAW_EXCEPTION_ALLOC;
  return p;
}

void *LibRaw::calloc(size_t n, size_t t)
{
  void *p = memmgr.calloc(n, t);
  if (!p)
    throw LIBRAW_EXCEPTION_ALLOC;
  return p;
}
void LibRaw::free(void *p) { memmgr.free(p); }

void LibRaw::recycle_datastream()
{
  if (libraw_internal_data.internal_data.input && libraw_internal_data.internal_data.input_internal)
  {
    delete libraw_internal_data.internal_data.input;
    libraw_internal_data.internal_data.input = NULL;
  }
  libraw_internal_data.internal_data.input_internal = 0;
}

void x3f_clear(void *);

void LibRaw::recycle()
{
  recycle_datastream();
#define FREE(a)                                                                                                        \
  do                                                                                                                   \
  {                                                                                                                    \
    if (a)                                                                                                             \
    {                                                                                                                  \
      free(a);                                                                                                         \
      a = NULL;                                                                                                        \
    }                                                                                                                  \
  } while (0)

  FREE(imgdata.image);
  FREE(imgdata.thumbnail.thumb);
  FREE(libraw_internal_data.internal_data.meta_data);
  FREE(libraw_internal_data.output_data.histogram);
  FREE(libraw_internal_data.output_data.oprof);
  FREE(imgdata.color.profile);
  FREE(imgdata.rawdata.ph1_cblack);
  FREE(imgdata.rawdata.ph1_rblack);
  FREE(imgdata.rawdata.raw_alloc);
  FREE(imgdata.idata.xmpdata);
#undef FREE
  ZERO(imgdata.sizes);
  ZERO(imgdata.idata);
  ZERO(imgdata.makernotes);
  ZERO(imgdata.color);
  ZERO(imgdata.other);
  ZERO(imgdata.thumbnail);
  ZERO(imgdata.rawdata);
  imgdata.makernotes.olympus.OlympusCropID = -1;
  cleargps(&imgdata.other.parsed_gps);
  imgdata.color.baseline_exposure = -999.f;

  imgdata.makernotes.fuji.FujiExpoMidPointShift = -999.f;
  imgdata.makernotes.fuji.FujiDynamicRange = 0xffff;
  imgdata.makernotes.fuji.FujiFilmMode = 0xffff;
  imgdata.makernotes.fuji.FujiDynamicRangeSetting = 0xffff;
  imgdata.makernotes.fuji.FujiDevelopmentDynamicRange = 0xffff;
  imgdata.makernotes.fuji.FujiAutoDynamicRange = 0xffff;
  imgdata.makernotes.fuji.FocusMode = 0xffff;
  imgdata.makernotes.fuji.AFMode = 0xffff;
  imgdata.makernotes.fuji.FocusPixel[0] = imgdata.makernotes.fuji.FocusPixel[1] = 0xffff;
  imgdata.makernotes.fuji.ImageStabilization[0] = imgdata.makernotes.fuji.ImageStabilization[1] =
      imgdata.makernotes.fuji.ImageStabilization[2] = 0xffff;

  imgdata.makernotes.sony.SonyCameraType = 0xffff;
  imgdata.color.dng_color[0].illuminant = imgdata.color.dng_color[1].illuminant = 0xffff;

  for (int i = 0; i < 4; i++)
    imgdata.color.dng_levels.analogbalance[i] = 1.0f;

  ZERO(libraw_internal_data);
  ZERO(imgdata.lens);
  imgdata.lens.makernotes.CanonFocalUnits = 1;
  imgdata.lens.makernotes.LensID = 0xffffffffffffffffULL;
  ZERO(imgdata.shootinginfo);
  imgdata.shootinginfo.DriveMode = -1;
  imgdata.shootinginfo.FocusMode = -1;
  imgdata.shootinginfo.MeteringMode = -1;
  imgdata.shootinginfo.AFPoint = -1;
  imgdata.shootinginfo.ExposureMode = -1;
  imgdata.shootinginfo.ImageStabilization = -1;

  _exitflag = 0;
#ifdef USE_RAWSPEED
  if (_rawspeed_decoder)
  {
    RawDecoder *d = static_cast<RawDecoder *>(_rawspeed_decoder);
    delete d;
  }
  _rawspeed_decoder = 0;
#endif

  if (_x3f_data)
  {
    x3f_clear(_x3f_data);
    _x3f_data = 0;
  }

  memmgr.cleanup();
  imgdata.thumbnail.tformat = LIBRAW_THUMBNAIL_UNKNOWN;
  imgdata.progress_flags = 0;

  load_raw = thumb_load_raw = 0;

  tls->init();
}

const char *LibRaw::unpack_function_name()
{
  libraw_decoder_info_t decoder_info;
  get_decoder_info(&decoder_info);
  return decoder_info.decoder_name;
}

int LibRaw::get_decoder_info(libraw_decoder_info_t *d_info)
{
  if (!d_info)
    return LIBRAW_UNSPECIFIED_ERROR;
  d_info->decoder_name = 0;
  d_info->decoder_flags = 0;
  if (!load_raw)
    return LIBRAW_OUT_OF_ORDER_CALL;

  int rawdata = (imgdata.idata.filters || P1.colors == 1);
  // dcraw.c names order
  if (load_raw == &LibRaw::android_tight_load_raw)
  {
    d_info->decoder_name = "android_tight_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_FIXEDMAXC;
  }
  else if (load_raw == &LibRaw::android_loose_load_raw)
  {
    d_info->decoder_name = "android_loose_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_FIXEDMAXC;
  }
  else if (load_raw == &LibRaw::canon_600_load_raw)
  {
    d_info->decoder_name = "canon_600_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_FIXEDMAXC;
  }
  else if (load_raw == &LibRaw::fuji_compressed_load_raw)
  {
    d_info->decoder_name = "fuji_compressed_load_raw()";
  }
  else if (load_raw == &LibRaw::canon_load_raw)
  {
    d_info->decoder_name = "canon_load_raw()";
  }
  else if (load_raw == &LibRaw::lossless_jpeg_load_raw)
  {
    d_info->decoder_name = "lossless_jpeg_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_HASCURVE | LIBRAW_DECODER_TRYRAWSPEED;
  }
  else if (load_raw == &LibRaw::canon_sraw_load_raw)
  {
    d_info->decoder_name = "canon_sraw_load_raw()";
    // d_info->decoder_flags = LIBRAW_DECODER_TRYRAWSPEED;
  }
  else if (load_raw == &LibRaw::lossless_dng_load_raw)
  {
    d_info->decoder_name = "lossless_dng_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_HASCURVE | LIBRAW_DECODER_TRYRAWSPEED | LIBRAW_DECODER_ADOBECOPYPIXEL;
  }
  else if (load_raw == &LibRaw::packed_dng_load_raw)
  {
    d_info->decoder_name = "packed_dng_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_HASCURVE | LIBRAW_DECODER_TRYRAWSPEED | LIBRAW_DECODER_ADOBECOPYPIXEL;
  }
  else if (load_raw == &LibRaw::pentax_load_raw)
  {
    d_info->decoder_name = "pentax_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_TRYRAWSPEED;
  }
  else if (load_raw == &LibRaw::nikon_load_raw)
  {
    d_info->decoder_name = "nikon_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_TRYRAWSPEED;
  }
  else if (load_raw == &LibRaw::nikon_coolscan_load_raw)
  {
    d_info->decoder_name = "nikon_coolscan_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_FIXEDMAXC;
  }
  else if (load_raw == &LibRaw::nikon_load_sraw)
  {
    d_info->decoder_name = "nikon_load_sraw()";
    d_info->decoder_flags = LIBRAW_DECODER_HASCURVE | LIBRAW_DECODER_FIXEDMAXC;
  }
  else if (load_raw == &LibRaw::nikon_yuv_load_raw)
  {
    d_info->decoder_name = "nikon_load_yuv_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_HASCURVE | LIBRAW_DECODER_FIXEDMAXC;
  }
  else if (load_raw == &LibRaw::rollei_load_raw)
  {
    // UNTESTED
    d_info->decoder_name = "rollei_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_FIXEDMAXC;
  }
  else if (load_raw == &LibRaw::phase_one_load_raw)
  {
    d_info->decoder_name = "phase_one_load_raw()";
  }
  else if (load_raw == &LibRaw::phase_one_load_raw_c)
  {
    d_info->decoder_name = "phase_one_load_raw_c()";
  }
  else if (load_raw == &LibRaw::hasselblad_load_raw)
  {
    d_info->decoder_name = "hasselblad_load_raw()";
  }
  else if (load_raw == &LibRaw::leaf_hdr_load_raw)
  {
    d_info->decoder_name = "leaf_hdr_load_raw()";
  }
  else if (load_raw == &LibRaw::unpacked_load_raw)
  {
    d_info->decoder_name = "unpacked_load_raw()";
  }
  else if (load_raw == &LibRaw::unpacked_load_raw_reversed)
  {
    d_info->decoder_name = "unpacked_load_raw_reversed()";
    d_info->decoder_flags = LIBRAW_DECODER_FIXEDMAXC;
  }
  else if (load_raw == &LibRaw::sinar_4shot_load_raw)
  {
    // UNTESTED
    d_info->decoder_name = "sinar_4shot_load_raw()";
  }
  else if (load_raw == &LibRaw::imacon_full_load_raw)
  {
    d_info->decoder_name = "imacon_full_load_raw()";
  }
  else if (load_raw == &LibRaw::hasselblad_full_load_raw)
  {
    d_info->decoder_name = "hasselblad_full_load_raw()";
  }
  else if (load_raw == &LibRaw::packed_load_raw)
  {
    d_info->decoder_name = "packed_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_TRYRAWSPEED;
  }
  else if (load_raw == &LibRaw::broadcom_load_raw)
  {
    // UNTESTED
    d_info->decoder_name = "broadcom_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_FIXEDMAXC;
  }
  else if (load_raw == &LibRaw::nokia_load_raw)
  {
    // UNTESTED
    d_info->decoder_name = "nokia_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_FIXEDMAXC;
  }
  else if (load_raw == &LibRaw::canon_rmf_load_raw)
  {
    // UNTESTED
    d_info->decoder_name = "canon_rmf_load_raw()";
  }
  else if (load_raw == &LibRaw::panasonic_load_raw)
  {
    d_info->decoder_name = "panasonic_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_TRYRAWSPEED;
  }
  else if (load_raw == &LibRaw::olympus_load_raw)
  {
    d_info->decoder_name = "olympus_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_TRYRAWSPEED;
  }
  else if (load_raw == &LibRaw::minolta_rd175_load_raw)
  {
    // UNTESTED
    d_info->decoder_name = "minolta_rd175_load_raw()";
  }
  else if (load_raw == &LibRaw::quicktake_100_load_raw)
  {
    // UNTESTED
    d_info->decoder_name = "quicktake_100_load_raw()";
  }
  else if (load_raw == &LibRaw::kodak_radc_load_raw)
  {
    d_info->decoder_name = "kodak_radc_load_raw()";
  }
  else if (load_raw == &LibRaw::kodak_jpeg_load_raw)
  {
    // UNTESTED + RBAYER
    d_info->decoder_name = "kodak_jpeg_load_raw()";
  }
  else if (load_raw == &LibRaw::lossy_dng_load_raw)
  {
    // Check rbayer
    d_info->decoder_name = "lossy_dng_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_TRYRAWSPEED | LIBRAW_DECODER_HASCURVE;
  }
  else if (load_raw == &LibRaw::kodak_dc120_load_raw)
  {
    d_info->decoder_name = "kodak_dc120_load_raw()";
  }
  else if (load_raw == &LibRaw::eight_bit_load_raw)
  {
    d_info->decoder_name = "eight_bit_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_HASCURVE | LIBRAW_DECODER_FIXEDMAXC;
  }
  else if (load_raw == &LibRaw::kodak_c330_load_raw)
  {
    d_info->decoder_name = "kodak_yrgb_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_HASCURVE | LIBRAW_DECODER_FIXEDMAXC;
  }
  else if (load_raw == &LibRaw::kodak_c603_load_raw)
  {
    d_info->decoder_name = "kodak_yrgb_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_HASCURVE | LIBRAW_DECODER_FIXEDMAXC;
  }
  else if (load_raw == &LibRaw::kodak_262_load_raw)
  {
    d_info->decoder_name = "kodak_262_load_raw()"; // UNTESTED!
    d_info->decoder_flags = LIBRAW_DECODER_HASCURVE | LIBRAW_DECODER_FIXEDMAXC;
  }
  else if (load_raw == &LibRaw::kodak_65000_load_raw)
  {
    d_info->decoder_name = "kodak_65000_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_HASCURVE;
  }
  else if (load_raw == &LibRaw::kodak_ycbcr_load_raw)
  {
    // UNTESTED
    d_info->decoder_name = "kodak_ycbcr_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_HASCURVE | LIBRAW_DECODER_FIXEDMAXC;
  }
  else if (load_raw == &LibRaw::kodak_rgb_load_raw)
  {
    // UNTESTED
    d_info->decoder_name = "kodak_rgb_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_FIXEDMAXC;
  }
  else if (load_raw == &LibRaw::sony_load_raw)
  {
    d_info->decoder_name = "sony_load_raw()";
  }
  else if (load_raw == &LibRaw::sony_arw_load_raw)
  {
    d_info->decoder_name = "sony_arw_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_TRYRAWSPEED;
  }
  else if (load_raw == &LibRaw::sony_arw2_load_raw)
  {
    d_info->decoder_name = "sony_arw2_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_HASCURVE | LIBRAW_DECODER_TRYRAWSPEED | LIBRAW_DECODER_SONYARW2;
  }
  else if (load_raw == &LibRaw::samsung_load_raw)
  {
    d_info->decoder_name = "samsung_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_TRYRAWSPEED;
  }
  else if (load_raw == &LibRaw::samsung2_load_raw)
  {
    d_info->decoder_name = "samsung2_load_raw()";
  }
  else if (load_raw == &LibRaw::samsung3_load_raw)
  {
    d_info->decoder_name = "samsung3_load_raw()";
  }
  else if (load_raw == &LibRaw::smal_v6_load_raw)
  {
    // UNTESTED
    d_info->decoder_name = "smal_v6_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_FIXEDMAXC;
  }
  else if (load_raw == &LibRaw::smal_v9_load_raw)
  {
    // UNTESTED
    d_info->decoder_name = "smal_v9_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_FIXEDMAXC;
  }
  else if (load_raw == &LibRaw::redcine_load_raw)
  {
    d_info->decoder_name = "redcine_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_HASCURVE;
  }
  else if (load_raw == &LibRaw::x3f_load_raw)
  {
    d_info->decoder_name = "x3f_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_OWNALLOC | LIBRAW_DECODER_FIXEDMAXC | LIBRAW_DECODER_LEGACY_WITH_MARGINS;
  }
  else if (load_raw == &LibRaw::pentax_4shot_load_raw)
  {
    d_info->decoder_name = "pentax_4shot_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_OWNALLOC;
  }
  else if (load_raw == &LibRaw::deflate_dng_load_raw)
  {
    d_info->decoder_name = "deflate_dng_load_raw()";
    d_info->decoder_flags = LIBRAW_DECODER_OWNALLOC;
  }
  else if (load_raw == &LibRaw::nikon_load_striped_packed_raw)
  {
    d_info->decoder_name = "nikon_load_striped_packed_raw()";
  }
#ifdef LIBRAW_DEMOSAIC_PACK_GPL2
  else if (load_raw == &LibRaw::foveon_sd_load_raw)
  {
    d_info->decoder_name = "foveon_sd_load_raw()";
  }
  else if (load_raw == &LibRaw::foveon_dp_load_raw)
  {
    d_info->decoder_name = "foveon_dp_load_raw()";
  }
#endif
  else
  {
    d_info->decoder_name = "Unknown unpack function";
    d_info->decoder_flags = LIBRAW_DECODER_NOTSET;
  }
  return LIBRAW_SUCCESS;
}

int LibRaw::adjust_maximum()
{
  ushort real_max;
  float auto_threshold;

  if (O.adjust_maximum_thr < 0.00001)
    return LIBRAW_SUCCESS;
  else if (O.adjust_maximum_thr > 0.99999)
    auto_threshold = LIBRAW_DEFAULT_ADJUST_MAXIMUM_THRESHOLD;
  else
    auto_threshold = O.adjust_maximum_thr;

  real_max = C.data_maximum;
  if (real_max > 0 && real_max < C.maximum && real_max > C.maximum * auto_threshold)
  {
    C.maximum = real_max;
  }
  return LIBRAW_SUCCESS;
}

void LibRaw::merror(void *ptr, const char *where)
{
  if (ptr)
    return;
  if (callbacks.mem_cb)
    (*callbacks.mem_cb)(
        callbacks.memcb_data,
        libraw_internal_data.internal_data.input ? libraw_internal_data.internal_data.input->fname() : NULL, where);
  throw LIBRAW_EXCEPTION_ALLOC;
}

int LibRaw::open_file(const char *fname, INT64 max_buf_size)
{
#ifndef WIN32
  struct stat st;
  if (stat(fname, &st))
    return LIBRAW_IO_ERROR;
  int big = (st.st_size > max_buf_size) ? 1 : 0;
#else
  struct _stati64 st;
  if (_stati64(fname, &st))
    return LIBRAW_IO_ERROR;
  int big = (st.st_size > max_buf_size) ? 1 : 0;
#endif

  LibRaw_abstract_datastream *stream;
  try
  {
    if (big)
      stream = new LibRaw_bigfile_datastream(fname);
    else
      stream = new LibRaw_file_datastream(fname);
  }

  catch (std::bad_alloc)
  {
    recycle();
    return LIBRAW_UNSUFFICIENT_MEMORY;
  }
  if (!stream->valid())
  {
    delete stream;
    return LIBRAW_IO_ERROR;
  }
  ID.input_internal = 0; // preserve from deletion on error
  int ret = open_datastream(stream);
  if (ret == LIBRAW_SUCCESS)
  {
    ID.input_internal = 1; // flag to delete datastream on recycle
  }
  else
  {
    delete stream;
    ID.input_internal = 0;
  }
  return ret;
}

#if defined(_WIN32) && !defined(__MINGW32__) && defined(_MSC_VER) && (_MSC_VER > 1310)
int LibRaw::open_file(const wchar_t *fname, INT64 max_buf_size)
{
  struct _stati64 st;
  if (_wstati64(fname, &st))
    return LIBRAW_IO_ERROR;
  int big = (st.st_size > max_buf_size) ? 1 : 0;

  LibRaw_abstract_datastream *stream;
  try
  {
    if (big)
      stream = new LibRaw_bigfile_datastream(fname);
    else
      stream = new LibRaw_file_datastream(fname);
  }

  catch (std::bad_alloc)
  {
    recycle();
    return LIBRAW_UNSUFFICIENT_MEMORY;
  }
  if (!stream->valid())
  {
    delete stream;
    return LIBRAW_IO_ERROR;
  }
  ID.input_internal = 0; // preserve from deletion on error
  int ret = open_datastream(stream);
  if (ret == LIBRAW_SUCCESS)
  {
    ID.input_internal = 1; // flag to delete datastream on recycle
  }
  else
  {
    delete stream;
    ID.input_internal = 0;
  }
  return ret;
}
#endif

int LibRaw::open_buffer(void *buffer, size_t size)
{
  // this stream will close on recycle()
  if (!buffer || buffer == (void *)-1)
    return LIBRAW_IO_ERROR;

  LibRaw_buffer_datastream *stream;
  try
  {
    stream = new LibRaw_buffer_datastream(buffer, size);
  }
  catch (std::bad_alloc)
  {
    recycle();
    return LIBRAW_UNSUFFICIENT_MEMORY;
  }
  if (!stream->valid())
  {
    delete stream;
    return LIBRAW_IO_ERROR;
  }
  ID.input_internal = 0; // preserve from deletion on error
  int ret = open_datastream(stream);
  if (ret == LIBRAW_SUCCESS)
  {
    ID.input_internal = 1; // flag to delete datastream on recycle
  }
  else
  {
    delete stream;
    ID.input_internal = 0;
  }
  return ret;
}

int LibRaw::open_bayer(unsigned char *buffer, unsigned datalen,
	ushort _raw_width, ushort _raw_height, ushort _left_margin, ushort _top_margin, ushort _right_margin, ushort _bottom_margin,
	unsigned char procflags, unsigned char bayer_pattern, unsigned unused_bits, unsigned otherflags, unsigned black_level)
{
	// this stream will close on recycle()
	if (!buffer || buffer == (void *)-1)
		return LIBRAW_IO_ERROR;

	LibRaw_buffer_datastream *stream;
	try
	{
		stream = new LibRaw_buffer_datastream(buffer, datalen);
	}
	catch (std::bad_alloc)
	{
		recycle();
		return LIBRAW_UNSUFFICIENT_MEMORY;
	}
	if (!stream->valid())
	{
		delete stream;
		return LIBRAW_IO_ERROR;
	}
	ID.input = stream;
	SET_PROC_FLAG(LIBRAW_PROGRESS_OPEN);
	// From identify
	initdata();
	strcpy(imgdata.idata.make, "BayerDump");
	snprintf(imgdata.idata.model, sizeof(imgdata.idata.model)-1, "%u x %u pixels", _raw_width, _raw_height);
	S.flip = procflags >> 2;
	libraw_internal_data.internal_output_params.zero_is_bad = procflags & 2;
	libraw_internal_data.unpacker_data.data_offset = 0;
	S.raw_width = _raw_width;
	S.raw_height = _raw_height;
	S.left_margin = _left_margin;
	S.top_margin = _top_margin;
	S.width = S.raw_width - S.left_margin - _right_margin;
	S.height = S.raw_height - S.top_margin - _bottom_margin;

	imgdata.idata.filters = 0x1010101 * bayer_pattern;
	imgdata.idata.colors = 4 - !((imgdata.idata.filters & imgdata.idata.filters >> 1) & 0x5555);
	libraw_internal_data.unpacker_data.load_flags = otherflags;
	switch (libraw_internal_data.unpacker_data.tiff_bps = (datalen) * 8 / (S.raw_width * S.raw_height))
	{
	case 8:
		load_raw = &CLASS eight_bit_load_raw;
		break;
	case 10:
		if ((datalen) / S.raw_height * 3 >= S.raw_width * 4)
		{
			load_raw = &CLASS android_loose_load_raw;
			break;
		}
		else if (libraw_internal_data.unpacker_data.load_flags & 1)
		{
			load_raw = &CLASS android_tight_load_raw;
			break;
		}
	case 12:
		libraw_internal_data.unpacker_data.load_flags |= 128;
		load_raw = &CLASS packed_load_raw;
		break;
	case 16:
		libraw_internal_data.unpacker_data.order = 0x4949 | 0x404 * (libraw_internal_data.unpacker_data.load_flags & 1);
		libraw_internal_data.unpacker_data.tiff_bps -= libraw_internal_data.unpacker_data.load_flags >> 4;
		libraw_internal_data.unpacker_data.tiff_bps -= libraw_internal_data.unpacker_data.load_flags = libraw_internal_data.unpacker_data.load_flags >> 1 & 7;
		load_raw = &CLASS unpacked_load_raw;
	}
	C.maximum = (1 << libraw_internal_data.unpacker_data.tiff_bps) - (1 << unused_bits);
	C.black = black_level;
	S.iwidth = S.width;
	S.iheight = S.height;
	imgdata.idata.colors = 3;
	imgdata.idata.filters |= ((imgdata.idata.filters >> 2 & 0x22222222) | (imgdata.idata.filters << 2 & 0x88888888)) & imgdata.idata.filters << 1;

	imgdata.idata.raw_count = 1;
	for(int i = 0; i < 4; i++)
		imgdata.color.pre_mul[i] = 1.0;

	ID.input_internal = 1;
	SET_PROC_FLAG(LIBRAW_PROGRESS_IDENTIFY);
	return LIBRAW_SUCCESS;
}


#ifdef USE_ZLIB
inline unsigned int __DNG_HalfToFloat(ushort halfValue)
{
  int sign = (halfValue >> 15) & 0x00000001;
  int exponent = (halfValue >> 10) & 0x0000001f;
  int mantissa = halfValue & 0x000003ff;
  if (exponent == 0)
  {
    if (mantissa == 0)
    {
      return (unsigned int)(sign << 31);
    }
    else
    {
      while (!(mantissa & 0x00000400))
      {
        mantissa <<= 1;
        exponent -= 1;
      }
      exponent += 1;
      mantissa &= ~0x00000400;
    }
  }
  else if (exponent == 31)
  {
    if (mantissa == 0)
    {
      return (unsigned int)((sign << 31) | ((0x1eL + 127 - 15) << 23) | (0x3ffL << 13));
    }
    else
    {
      return 0;
    }
  }
  exponent += (127 - 15);
  mantissa <<= 13;
  return (unsigned int)((sign << 31) | (exponent << 23) | mantissa);
}

inline unsigned int __DNG_FP24ToFloat(const unsigned char *input)
{
  int sign = (input[0] >> 7) & 0x01;
  int exponent = (input[0]) & 0x7F;
  int mantissa = (((int)input[1]) << 8) | input[2];
  if (exponent == 0)
  {
    if (mantissa == 0)
    {
      return (unsigned int)(sign << 31);
    }
    else
    {
      while (!(mantissa & 0x00010000))
      {
        mantissa <<= 1;
        exponent -= 1;
      }
      exponent += 1;
      mantissa &= ~0x00010000;
    }
  }
  else if (exponent == 127)
  {
    if (mantissa == 0)
    {
      return (unsigned int)((sign << 31) | ((0x7eL + 128 - 64) << 23) | (0xffffL << 7));
    }
    else
    {
      // Nan -- Just set to zero.
      return 0;
    }
  }
  exponent += (128 - 64);
  mantissa <<= 7;
  return (uint32_t)((sign << 31) | (exponent << 23) | mantissa);
}

inline void DecodeDeltaBytes(unsigned char *bytePtr, int cols, int channels)
{
  if (channels == 1)
  {
    unsigned char b0 = bytePtr[0];
    bytePtr += 1;
    for (uint32_t col = 1; col < cols; ++col)
    {
      b0 += bytePtr[0];
      bytePtr[0] = b0;
      bytePtr += 1;
    }
  }
  else if (channels == 3)
  {
    unsigned char b0 = bytePtr[0];
    unsigned char b1 = bytePtr[1];
    unsigned char b2 = bytePtr[2];
    bytePtr += 3;
    for (int col = 1; col < cols; ++col)
    {
      b0 += bytePtr[0];
      b1 += bytePtr[1];
      b2 += bytePtr[2];
      bytePtr[0] = b0;
      bytePtr[1] = b1;
      bytePtr[2] = b2;
      bytePtr += 3;
    }
  }
  else if (channels == 4)
  {
    unsigned char b0 = bytePtr[0];
    unsigned char b1 = bytePtr[1];
    unsigned char b2 = bytePtr[2];
    unsigned char b3 = bytePtr[3];
    bytePtr += 4;
    for (uint32_t col = 1; col < cols; ++col)
    {
      b0 += bytePtr[0];
      b1 += bytePtr[1];
      b2 += bytePtr[2];
      b3 += bytePtr[3];
      bytePtr[0] = b0;
      bytePtr[1] = b1;
      bytePtr[2] = b2;
      bytePtr[3] = b3;
      bytePtr += 4;
    }
  }
  else
  {
    for (int col = 1; col < cols; ++col)
    {
      for (int chan = 0; chan < channels; ++chan)
      {
        bytePtr[chan + channels] += bytePtr[chan];
      }
      bytePtr += channels;
    }
  }
}

static void DecodeFPDelta(unsigned char *input, unsigned char *output, int cols, int channels, int bytesPerSample)
{
  DecodeDeltaBytes(input, cols * bytesPerSample, channels);
  int32_t rowIncrement = cols * channels;

  if (bytesPerSample == 2)
  {

#if LibRawBigEndian
    const unsigned char *input0 = input;
    const unsigned char *input1 = input + rowIncrement;
#else
    const unsigned char *input1 = input;
    const unsigned char *input0 = input + rowIncrement;
#endif
    for (int col = 0; col < rowIncrement; ++col)
    {
      output[0] = input0[col];
      output[1] = input1[col];
      output += 2;
    }
  }
  else if (bytesPerSample == 3)
  {
    const unsigned char *input0 = input;
    const unsigned char *input1 = input + rowIncrement;
    const unsigned char *input2 = input + rowIncrement * 2;
    for (int col = 0; col < rowIncrement; ++col)
    {
      output[0] = input0[col];
      output[1] = input1[col];
      output[2] = input2[col];
      output += 3;
    }
  }
  else
  {
#if LibRawBigEndian
    const unsigned char *input0 = input;
    const unsigned char *input1 = input + rowIncrement;
    const unsigned char *input2 = input + rowIncrement * 2;
    const unsigned char *input3 = input + rowIncrement * 3;
#else
    const unsigned char *input3 = input;
    const unsigned char *input2 = input + rowIncrement;
    const unsigned char *input1 = input + rowIncrement * 2;
    const unsigned char *input0 = input + rowIncrement * 3;
#endif
    for (int col = 0; col < rowIncrement; ++col)
    {
      output[0] = input0[col];
      output[1] = input1[col];
      output[2] = input2[col];
      output[3] = input3[col];
      output += 4;
    }
  }
}

static float expandFloats(unsigned char *dst, int tileWidth, int bytesps)
{
  float max = 0.f;
  if (bytesps == 2)
  {
    uint16_t *dst16 = (ushort *)dst;
    uint32_t *dst32 = (unsigned int *)dst;
    float *f32 = (float *)dst;
    for (int index = tileWidth - 1; index >= 0; --index)
    {
      dst32[index] = __DNG_HalfToFloat(dst16[index]);
      max = MAX(max, f32[index]);
    }
  }
  else if (bytesps == 3)
  {
    uint8_t *dst8 = ((unsigned char *)dst) + (tileWidth - 1) * 3;
    uint32_t *dst32 = (unsigned int *)dst;
    float *f32 = (float *)dst;
    for (int index = tileWidth - 1; index >= 0; --index, dst8 -= 3)
    {
      dst32[index] = __DNG_FP24ToFloat(dst8);
      max = MAX(max, f32[index]);
    }
  }
  else if (bytesps == 4)
  {
    float *f32 = (float *)dst;
    for (int index = 0; index < tileWidth; index++)
      max = MAX(max, f32[index]);
  }
  return max;
}

void LibRaw::deflate_dng_load_raw()
{
  struct tiff_ifd_t *ifd = &tiff_ifd[0];
  while (ifd < &tiff_ifd[libraw_internal_data.identify_data.tiff_nifds] &&
         ifd->offset != libraw_internal_data.unpacker_data.data_offset)
    ++ifd;
  if (ifd == &tiff_ifd[libraw_internal_data.identify_data.tiff_nifds])
  {
    throw LIBRAW_EXCEPTION_DECODE_RAW;
  }

  float *float_raw_image = 0;
  float max = 0.f;

  if (ifd->samples != 1 && ifd->samples != 3 && ifd->samples != 4)
    throw LIBRAW_EXCEPTION_DECODE_RAW; // Only float deflated supported

  if (libraw_internal_data.unpacker_data.tiff_samples != ifd->samples)
    throw LIBRAW_EXCEPTION_DECODE_RAW; // Wrong IFD

  size_t tilesH = (imgdata.sizes.raw_width + libraw_internal_data.unpacker_data.tile_width - 1) /
                  libraw_internal_data.unpacker_data.tile_width;
  size_t tilesV = (imgdata.sizes.raw_height + libraw_internal_data.unpacker_data.tile_length - 1) /
                  libraw_internal_data.unpacker_data.tile_length;
  size_t tileCnt = tilesH * tilesV;

  if (ifd->sample_format == 3)
  { // Floating point data
    float_raw_image = (float *)calloc(tileCnt * libraw_internal_data.unpacker_data.tile_length *
                                          libraw_internal_data.unpacker_data.tile_width * ifd->samples,
                                      sizeof(float));
    // imgdata.color.maximum = 65535;
    // imgdata.color.black = 0;
    // memset(imgdata.color.cblack,0,sizeof(imgdata.color.cblack));
  }
  else
    throw LIBRAW_EXCEPTION_DECODE_RAW; // Only float deflated supported

  int xFactor;
  switch (ifd->predictor)
  {
  case 3:
  default:
    xFactor = 1;
    break;
  case 34894:
    xFactor = 2;
    break;
  case 34895:
    xFactor = 4;
    break;
  }

  if (libraw_internal_data.unpacker_data.tile_length < INT_MAX)
  {
    if (tileCnt < 1 || tileCnt > 1000000)
      throw LIBRAW_EXCEPTION_DECODE_RAW;

    size_t *tOffsets = (size_t *)malloc(tileCnt * sizeof(size_t));
    for (int t = 0; t < tileCnt; ++t)
      tOffsets[t] = get4();

    size_t *tBytes = (size_t *)malloc(tileCnt * sizeof(size_t));
    unsigned long maxBytesInTile = 0;
    if (tileCnt == 1)
      tBytes[0] = maxBytesInTile = ifd->bytes;
    else
    {
      libraw_internal_data.internal_data.input->seek(ifd->bytes, SEEK_SET);
      for (size_t t = 0; t < tileCnt; ++t)
      {
        tBytes[t] = get4();
        maxBytesInTile = MAX(maxBytesInTile, tBytes[t]);
      }
    }
    unsigned tilePixels =
        libraw_internal_data.unpacker_data.tile_width * libraw_internal_data.unpacker_data.tile_length;
    unsigned pixelSize = sizeof(float) * ifd->samples;
    unsigned tileBytes = tilePixels * pixelSize;
    unsigned tileRowBytes = libraw_internal_data.unpacker_data.tile_width * pixelSize;

    unsigned char *cBuffer = (unsigned char *)malloc(maxBytesInTile);
    unsigned char *uBuffer = (unsigned char *)malloc(tileBytes + tileRowBytes); // extra row for decoding

    for (size_t y = 0, t = 0; y < imgdata.sizes.raw_height; y += libraw_internal_data.unpacker_data.tile_length)
    {
      for (size_t x = 0; x < imgdata.sizes.raw_width; x += libraw_internal_data.unpacker_data.tile_width, ++t)
      {
        libraw_internal_data.internal_data.input->seek(tOffsets[t], SEEK_SET);
        libraw_internal_data.internal_data.input->read(cBuffer, 1, tBytes[t]);
        unsigned long dstLen = tileBytes;
        int err = uncompress(uBuffer + tileRowBytes, &dstLen, cBuffer, tBytes[t]);
        if (err != Z_OK)
        {
          free(tOffsets);
          free(tBytes);
          free(cBuffer);
          free(uBuffer);
          throw LIBRAW_EXCEPTION_DECODE_RAW;
          return;
        }
        else
        {
          int bytesps = ifd->bps >> 3;
          size_t rowsInTile = y + libraw_internal_data.unpacker_data.tile_length > imgdata.sizes.raw_height
                                  ? imgdata.sizes.raw_height - y
                                  : libraw_internal_data.unpacker_data.tile_length;
          size_t colsInTile = x + libraw_internal_data.unpacker_data.tile_width > imgdata.sizes.raw_width
                                  ? imgdata.sizes.raw_width - x
                                  : libraw_internal_data.unpacker_data.tile_width;

          for (size_t row = 0; row < rowsInTile; ++row) // do not process full tile if not needed
          {
            unsigned char *dst = uBuffer + row * libraw_internal_data.unpacker_data.tile_width * bytesps * ifd->samples;
            unsigned char *src = dst + tileRowBytes;
            DecodeFPDelta(src, dst, libraw_internal_data.unpacker_data.tile_width / xFactor, ifd->samples * xFactor,
                          bytesps);
            float lmax = expandFloats(dst, libraw_internal_data.unpacker_data.tile_width * ifd->samples, bytesps);
            max = MAX(max, lmax);
            unsigned char *dst2 =
                (unsigned char *)&float_raw_image[((y + row) * imgdata.sizes.raw_width + x) * ifd->samples];
            memmove(dst2, dst, colsInTile * ifd->samples * sizeof(float));
          }
        }
      }
    }
    free(tOffsets);
    free(tBytes);
    free(cBuffer);
    free(uBuffer);
  }
  imgdata.color.fmaximum = max;

  // Set fields according to data format

  imgdata.rawdata.raw_alloc = float_raw_image;
  if (ifd->samples == 1)
  {
    imgdata.rawdata.float_image = float_raw_image;
    imgdata.rawdata.sizes.raw_pitch = imgdata.sizes.raw_pitch = imgdata.sizes.raw_width * 4;
  }
  else if (ifd->samples == 3)
  {
    imgdata.rawdata.float3_image = (float(*)[3])float_raw_image;
    imgdata.rawdata.sizes.raw_pitch = imgdata.sizes.raw_pitch = imgdata.sizes.raw_width * 12;
  }
  else if (ifd->samples == 4)
  {
    imgdata.rawdata.float4_image = (float(*)[4])float_raw_image;
    imgdata.rawdata.sizes.raw_pitch = imgdata.sizes.raw_pitch = imgdata.sizes.raw_width * 16;
  }

  if (imgdata.params.raw_processing_options & LIBRAW_PROCESSING_CONVERTFLOAT_TO_INT)
    convertFloatToInt(); // with default settings
}
#else
void LibRaw::deflate_dng_load_raw() { throw LIBRAW_EXCEPTION_DECODE_RAW; }
#endif

int LibRaw::is_floating_point()
{
  struct tiff_ifd_t *ifd = &tiff_ifd[0];
  while (ifd < &tiff_ifd[libraw_internal_data.identify_data.tiff_nifds] &&
         ifd->offset != libraw_internal_data.unpacker_data.data_offset)
    ++ifd;
  if (ifd == &tiff_ifd[libraw_internal_data.identify_data.tiff_nifds])
    return 0;

  return ifd->sample_format == 3;
}

int LibRaw::have_fpdata()
{
  return imgdata.rawdata.float_image || imgdata.rawdata.float3_image || imgdata.rawdata.float4_image;
}

void LibRaw::convertFloatToInt(float dmin /* =4096.f */, float dmax /* =32767.f */, float dtarget /*= 16383.f */)
{
  int samples = 0;
  float *data = 0;
  if (imgdata.rawdata.float_image)
  {
    samples = 1;
    data = imgdata.rawdata.float_image;
  }
  else if (imgdata.rawdata.float3_image)
  {
    samples = 3;
    data = (float *)imgdata.rawdata.float3_image;
  }
  else if (imgdata.rawdata.float4_image)
  {
    samples = 4;
    data = (float *)imgdata.rawdata.float4_image;
  }
  else
    return;

  ushort *raw_alloc = (ushort *)malloc(imgdata.sizes.raw_height * imgdata.sizes.raw_width *
                                       libraw_internal_data.unpacker_data.tiff_samples * sizeof(ushort));
  float tmax = MAX(imgdata.color.maximum, 1);
  float datamax = imgdata.color.fmaximum;

  tmax = MAX(tmax, datamax);
  tmax = MAX(tmax, 1.f);

  float multip = 1.f;
  if (tmax < dmin || tmax > dmax)
  {
    imgdata.rawdata.color.fnorm = imgdata.color.fnorm = multip = dtarget / tmax;
    imgdata.rawdata.color.maximum = imgdata.color.maximum = dtarget;
    imgdata.rawdata.color.black = imgdata.color.black = (float)imgdata.color.black * multip;
    for (int i = 0; i < sizeof(imgdata.color.cblack) / sizeof(imgdata.color.cblack[0]); i++)
      if (i != 4 && i != 5)
        imgdata.rawdata.color.cblack[i] = imgdata.color.cblack[i] = (float)imgdata.color.cblack[i] * multip;
  }
  else
    imgdata.rawdata.color.fnorm = imgdata.color.fnorm = 0.f;

  for (size_t i = 0;
       i < imgdata.sizes.raw_height * imgdata.sizes.raw_width * libraw_internal_data.unpacker_data.tiff_samples; ++i)
  {
    float val = MAX(data[i], 0.f);
    raw_alloc[i] = (ushort)(val * multip);
  }

  if (samples == 1)
  {
    imgdata.rawdata.raw_alloc = imgdata.rawdata.raw_image = raw_alloc;
    imgdata.rawdata.sizes.raw_pitch = imgdata.sizes.raw_pitch = imgdata.sizes.raw_width * 2;
  }
  else if (samples == 3)
  {
    imgdata.rawdata.raw_alloc = imgdata.rawdata.color3_image = (ushort(*)[3])raw_alloc;
    imgdata.rawdata.sizes.raw_pitch = imgdata.sizes.raw_pitch = imgdata.sizes.raw_width * 6;
  }
  else if (samples == 4)
  {
    imgdata.rawdata.raw_alloc = imgdata.rawdata.color4_image = (ushort(*)[4])raw_alloc;
    imgdata.rawdata.sizes.raw_pitch = imgdata.sizes.raw_pitch = imgdata.sizes.raw_width * 8;
  }
  free(data); // remove old allocation
  imgdata.rawdata.float_image = 0;
  imgdata.rawdata.float3_image = 0;
  imgdata.rawdata.float4_image = 0;
}

void LibRaw::pentax_4shot_load_raw()
{
  ushort *plane = (ushort *)malloc(imgdata.sizes.raw_width * imgdata.sizes.raw_height * sizeof(ushort));
  int alloc_sz = imgdata.sizes.raw_width * (imgdata.sizes.raw_height + 16) * 4 * sizeof(ushort);
  ushort(*result)[4] = (ushort(*)[4])malloc(alloc_sz);
  struct movement_t
  {
    int row, col;
  } _move[4] = {
      {1, 1}, {0, 1}, {0, 0}, {1, 0},
  };

  int tidx = 0;
  for (int i = 0; i < 4; i++)
  {
    int move_row, move_col;
    if (imgdata.params.p4shot_order[i] >= '0' && imgdata.params.p4shot_order[i] <= '3')
    {
      move_row = (imgdata.params.p4shot_order[i] - '0' & 2) ? 1 : 0;
      move_col = (imgdata.params.p4shot_order[i] - '0' & 1) ? 1 : 0;
    }
    else
    {
      move_row = _move[i].row;
      move_col = _move[i].col;
    }
    for (; tidx < 16; tidx++)
      if (tiff_ifd[tidx].t_width == imgdata.sizes.raw_width && tiff_ifd[tidx].t_height == imgdata.sizes.raw_height &&
          tiff_ifd[tidx].bps > 8 && tiff_ifd[tidx].samples == 1)
        break;
    if (tidx >= 16)
      break;
    imgdata.rawdata.raw_image = plane;
    ID.input->seek(tiff_ifd[tidx].offset, SEEK_SET);
    imgdata.idata.filters = 0xb4b4b4b4;
    libraw_internal_data.unpacker_data.data_offset = tiff_ifd[tidx].offset;
    (this->*pentax_component_load_raw)();
    for (int row = 0; row < imgdata.sizes.raw_height - move_row; row++)
    {
      int colors[2];
      for (int c = 0; c < 2; c++)
        colors[c] = COLOR(row, c);
      ushort *srcrow = &plane[imgdata.sizes.raw_width * row];
      ushort(*dstrow)[4] = &result[(imgdata.sizes.raw_width) * (row + move_row) + move_col];
      for (int col = 0; col < imgdata.sizes.raw_width - move_col; col++)
        dstrow[col][colors[col % 2]] = srcrow[col];
    }
    tidx++;
  }
  // assign things back:
  imgdata.sizes.raw_pitch = imgdata.sizes.raw_width * 8;
  imgdata.idata.filters = 0;
  imgdata.rawdata.raw_alloc = imgdata.rawdata.color4_image = result;
  free(plane);
  imgdata.rawdata.raw_image = 0;
}

void LibRaw::hasselblad_full_load_raw()
{
  int row, col;

  for (row = 0; row < S.height; row++)
    for (col = 0; col < S.width; col++)
    {
      read_shorts(&imgdata.image[row * S.width + col][2], 1); // B
      read_shorts(&imgdata.image[row * S.width + col][1], 1); // G
      read_shorts(&imgdata.image[row * S.width + col][0], 1); // R
    }
}

void LibRaw::nikon_load_striped_packed_raw()
{
  int vbits = 0, bwide, rbits, bite, row, col, val, i;

  UINT64 bitbuf = 0;
  unsigned load_flags = 24; // libraw_internal_data.unpacker_data.load_flags;
  unsigned tiff_bps = libraw_internal_data.unpacker_data.tiff_bps;
  int tiff_compress = libraw_internal_data.unpacker_data.tiff_compress;

  struct tiff_ifd_t *ifd = &tiff_ifd[0];
  while (ifd < &tiff_ifd[libraw_internal_data.identify_data.tiff_nifds] &&
         ifd->offset != libraw_internal_data.unpacker_data.data_offset)
    ++ifd;
  if (ifd == &tiff_ifd[libraw_internal_data.identify_data.tiff_nifds])
    throw LIBRAW_EXCEPTION_DECODE_RAW;

  if (!ifd->rows_per_strip || !ifd->strip_offsets_count)
    return; // not unpacked
  int stripcnt = 0;

  bwide = S.raw_width * tiff_bps / 8;
  bwide += bwide & load_flags >> 7;
  rbits = bwide * 8 - S.raw_width * tiff_bps;
  if (load_flags & 1)
    bwide = bwide * 16 / 15;
  bite = 8 + (load_flags & 24);
  for (row = 0; row < S.raw_height; row++)
  {
    checkCancel();
    if (!(row % ifd->rows_per_strip))
    {
      if (stripcnt >= ifd->strip_offsets_count)
        return; // run out of data
      libraw_internal_data.internal_data.input->seek(ifd->strip_offsets[stripcnt], SEEK_SET);
      stripcnt++;
    }
    for (col = 0; col < S.raw_width; col++)
    {
      for (vbits -= tiff_bps; vbits < 0; vbits += bite)
      {
        bitbuf <<= bite;
        for (i = 0; i < bite; i += 8)
          bitbuf |= (unsigned)(libraw_internal_data.internal_data.input->get_char() << i);
      }
      imgdata.rawdata.raw_image[(row)*S.raw_width + (col)] = bitbuf << (64 - tiff_bps - vbits) >> (64 - tiff_bps);
    }
    vbits -= rbits;
  }
}

struct foveon_data_t
{
  const char *make;
  const char *model;
  const int raw_width, raw_height;
  const int white;
  const int left_margin, top_margin;
  const int width, height;
} foveon_data[] = {
    {"Sigma", "SD9", 2304, 1531, 12000, 20, 8, 2266, 1510},
    {"Sigma", "SD9", 1152, 763, 12000, 10, 2, 1132, 755},
    {"Sigma", "SD10", 2304, 1531, 12000, 20, 8, 2266, 1510},
    {"Sigma", "SD10", 1152, 763, 12000, 10, 2, 1132, 755},
    {"Sigma", "SD14", 2688, 1792, 14000, 18, 12, 2651, 1767},
    {"Sigma", "SD14", 2688, 896, 14000, 18, 6, 2651, 883}, // 2/3
    {"Sigma", "SD14", 1344, 896, 14000, 9, 6, 1326, 883},  // 1/2
    {"Sigma", "SD15", 2688, 1792, 2900, 18, 12, 2651, 1767},
    {"Sigma", "SD15", 2688, 896, 2900, 18, 6, 2651, 883}, // 2/3 ?
    {"Sigma", "SD15", 1344, 896, 2900, 9, 6, 1326, 883},  // 1/2 ?
    {"Sigma", "DP1", 2688, 1792, 2100, 18, 12, 2651, 1767},
    {"Sigma", "DP1", 2688, 896, 2100, 18, 6, 2651, 883}, // 2/3 ?
    {"Sigma", "DP1", 1344, 896, 2100, 9, 6, 1326, 883},  // 1/2 ?
    {"Sigma", "DP1S", 2688, 1792, 2200, 18, 12, 2651, 1767},
    {"Sigma", "DP1S", 2688, 896, 2200, 18, 6, 2651, 883}, // 2/3
    {"Sigma", "DP1S", 1344, 896, 2200, 9, 6, 1326, 883},  // 1/2
    {"Sigma", "DP1X", 2688, 1792, 3560, 18, 12, 2651, 1767},
    {"Sigma", "DP1X", 2688, 896, 3560, 18, 6, 2651, 883}, // 2/3
    {"Sigma", "DP1X", 1344, 896, 3560, 9, 6, 1326, 883},  // 1/2
    {"Sigma", "DP2", 2688, 1792, 2326, 13, 16, 2651, 1767},
    {"Sigma", "DP2", 2688, 896, 2326, 13, 8, 2651, 883}, // 2/3 ??
    {"Sigma", "DP2", 1344, 896, 2326, 7, 8, 1325, 883},  // 1/2 ??
    {"Sigma", "DP2S", 2688, 1792, 2300, 18, 12, 2651, 1767},
    {"Sigma", "DP2S", 2688, 896, 2300, 18, 6, 2651, 883}, // 2/3
    {"Sigma", "DP2S", 1344, 896, 2300, 9, 6, 1326, 883},  // 1/2
    {"Sigma", "DP2X", 2688, 1792, 2300, 18, 12, 2651, 1767},
    {"Sigma", "DP2X", 2688, 896, 2300, 18, 6, 2651, 883},           // 2/3
    {"Sigma", "DP2X", 1344, 896, 2300, 9, 6, 1325, 883},            // 1/2
    {"Sigma", "SD1", 4928, 3264, 3900, 12, 52, 4807, 3205},         // Full size
    {"Sigma", "SD1", 4928, 1632, 3900, 12, 26, 4807, 1603},         // 2/3 size
    {"Sigma", "SD1", 2464, 1632, 3900, 6, 26, 2403, 1603},          // 1/2 size
    {"Sigma", "SD1 Merrill", 4928, 3264, 3900, 12, 52, 4807, 3205}, // Full size
    {"Sigma", "SD1 Merrill", 4928, 1632, 3900, 12, 26, 4807, 1603}, // 2/3 size
    {"Sigma", "SD1 Merrill", 2464, 1632, 3900, 6, 26, 2403, 1603},  // 1/2 size
    {"Sigma", "DP1 Merrill", 4928, 3264, 3900, 12, 0, 4807, 3205},
    {"Sigma", "DP1 Merrill", 2464, 1632, 3900, 12, 0, 2403, 1603}, // 1/2 size
    {"Sigma", "DP1 Merrill", 4928, 1632, 3900, 12, 0, 4807, 1603}, // 2/3 size
    {"Sigma", "DP2 Merrill", 4928, 3264, 3900, 12, 0, 4807, 3205},
    {"Sigma", "DP2 Merrill", 2464, 1632, 3900, 12, 0, 2403, 1603}, // 1/2 size
    {"Sigma", "DP2 Merrill", 4928, 1632, 3900, 12, 0, 4807, 1603}, // 2/3 size
    {"Sigma", "DP3 Merrill", 4928, 3264, 3900, 12, 0, 4807, 3205},
    {"Sigma", "DP3 Merrill", 2464, 1632, 3900, 12, 0, 2403, 1603}, // 1/2 size
    {"Sigma", "DP3 Merrill", 4928, 1632, 3900, 12, 0, 4807, 1603}, // 2/3 size
    {"Polaroid", "x530", 1440, 1088, 2700, 10, 13, 1419, 1059},
    // dp2 Q
    {"Sigma", "dp3 Quattro", 5888, 3672, 16383, 204, 24, 5446, 3624}, // full size
    {"Sigma", "dp3 Quattro", 2944, 1836, 16383, 102, 12, 2723, 1812}, // half size
    {"Sigma", "dp2 Quattro", 5888, 3672, 16383, 204, 24, 5446, 3624}, // full size
    {"Sigma", "dp2 Quattro", 2944, 1836, 16383, 102, 12, 2723, 1812}, // half size
    {"Sigma", "dp1 Quattro", 5888, 3672, 16383, 204, 24, 5446, 3624}, // full size
    {"Sigma", "dp1 Quattro", 2944, 1836, 16383, 102, 12, 2723, 1812}, // half size
    {"Sigma", "dp0 Quattro", 5888, 3672, 16383, 204, 24, 5446, 3624}, // full size
    {"Sigma", "dp0 Quattro", 2944, 1836, 16383, 102, 12, 2723, 1812}, // half size
    // Sigma sd Quattro
    {"Sigma", "sd Quattro", 5888, 3776, 16383, 204, 76, 5446, 3624}, // full size
    {"Sigma", "sd Quattro", 2944, 1888, 16383, 102, 38, 2723, 1812}, // half size
    // Sd Quattro H
    {"Sigma", "sd Quattro H", 6656, 4480, 16383, 224, 160, 6208, 4160}, // full size
    {"Sigma", "sd Quattro H", 3328, 2240, 16383, 112, 80, 3104, 2080},  // half size
    {"Sigma", "sd Quattro H", 5504, 3680, 16383, 0, 4, 5496, 3668},     // full size
    {"Sigma", "sd Quattro H", 2752, 1840, 16383, 0, 2, 2748, 1834},     // half size
};
const int foveon_count = sizeof(foveon_data) / sizeof(foveon_data[0]);

int LibRaw::open_datastream(LibRaw_abstract_datastream *stream)
{

  if (!stream)
    return ENOENT;
  if (!stream->valid())
    return LIBRAW_IO_ERROR;
  recycle();

  try
  {
    ID.input = stream;
    SET_PROC_FLAG(LIBRAW_PROGRESS_OPEN);

    identify();

    if (!strcasecmp(imgdata.idata.make, "Canon") && (load_raw == &LibRaw::canon_sraw_load_raw) &&
        imgdata.sizes.raw_width > 0)
    {
      float ratio = float(imgdata.sizes.raw_height) / float(imgdata.sizes.raw_width);
      if ((ratio < 0.57 || ratio > 0.75) && imgdata.makernotes.canon.SensorHeight > 1 &&
          imgdata.makernotes.canon.SensorWidth > 1)
      {
        imgdata.sizes.raw_width = imgdata.makernotes.canon.SensorWidth;
        imgdata.sizes.left_margin = imgdata.makernotes.canon.SensorLeftBorder;
        imgdata.sizes.iwidth = imgdata.sizes.width =
            imgdata.makernotes.canon.SensorRightBorder - imgdata.makernotes.canon.SensorLeftBorder + 1;
        imgdata.sizes.raw_height = imgdata.makernotes.canon.SensorHeight;
        imgdata.sizes.top_margin = imgdata.makernotes.canon.SensorTopBorder;
        imgdata.sizes.iheight = imgdata.sizes.height =
            imgdata.makernotes.canon.SensorBottomBorder - imgdata.makernotes.canon.SensorTopBorder + 1;
        libraw_internal_data.unpacker_data.load_flags |= 256; // reset width/height in canon_sraw_load_raw()
        imgdata.sizes.raw_pitch = 8 * imgdata.sizes.raw_width;
      }
      else if (imgdata.sizes.raw_width == 4032 && imgdata.sizes.raw_height == 3402 &&
               !strcasecmp(imgdata.idata.model, "EOS 80D")) // 80D hardcoded
      {
        imgdata.sizes.raw_width = 4536;
        imgdata.sizes.left_margin = 28;
        imgdata.sizes.iwidth = imgdata.sizes.width = imgdata.sizes.raw_width - imgdata.sizes.left_margin;
        imgdata.sizes.raw_height = 3024;
        imgdata.sizes.top_margin = 8;
        imgdata.sizes.iheight = imgdata.sizes.height = imgdata.sizes.raw_height - imgdata.sizes.top_margin;
        libraw_internal_data.unpacker_data.load_flags |= 256;
        imgdata.sizes.raw_pitch = 8 * imgdata.sizes.raw_width;
      }
    }

    // XTrans Compressed?
    if (!imgdata.idata.dng_version && !strcasecmp(imgdata.idata.make, "Fujifilm") &&
        (load_raw == &LibRaw::unpacked_load_raw))
    {
      if (imgdata.sizes.raw_width * imgdata.sizes.raw_height * 2 != libraw_internal_data.unpacker_data.data_size)
        parse_fuji_compressed_header();
      if (imgdata.idata.filters == 9)
      {
        // Adjust top/left margins for X-Trans
        int newtm = imgdata.sizes.top_margin % 6 ? (imgdata.sizes.top_margin / 6 + 1) * 6 : imgdata.sizes.top_margin;
        int newlm = imgdata.sizes.left_margin % 6 ? (imgdata.sizes.left_margin / 6 + 1) * 6 : imgdata.sizes.left_margin;
        if (newtm != imgdata.sizes.top_margin || newlm != imgdata.sizes.left_margin)
        {
          imgdata.sizes.height -= (newtm - imgdata.sizes.top_margin);
          imgdata.sizes.top_margin = newtm;
          imgdata.sizes.width -= (newlm - imgdata.sizes.left_margin);
          imgdata.sizes.left_margin = newlm;
          for (int c1 = 0; c1 < 6; c1++)
            for (int c2 = 0; c2 < 6; c2++)
              imgdata.idata.xtrans[c1][c2] = imgdata.idata.xtrans_abs[c1][c2];
        }
      }
    }

    // Fix DNG white balance if needed
    if (imgdata.idata.dng_version && (imgdata.idata.filters == 0) && imgdata.idata.colors > 1 &&
        imgdata.idata.colors < 5)
    {
      float delta[4] = {0.f, 0.f, 0.f, 0.f};
      for (int c = 0; c < imgdata.idata.colors; c++)
        delta[c] = imgdata.color.dng_levels.dng_whitelevel[c] - imgdata.color.dng_levels.dng_blacklevel[c];
      float mindelta = delta[0], maxdelta = delta[0];
      for (int c = 1; c < imgdata.idata.colors; c++)
      {
        if (mindelta > delta[c])
          mindelta = delta[c];
        if (maxdelta < delta[c])
          maxdelta = delta[c];
      }
      if (mindelta > 1 && maxdelta < (mindelta * 20)) // safety
      {
        for (int c = 0; c < imgdata.idata.colors; c++)
        {
          imgdata.color.cam_mul[c] /= (delta[c] / maxdelta);
          imgdata.color.pre_mul[c] /= (delta[c] / maxdelta);
        }
        imgdata.color.maximum = imgdata.color.cblack[0] + maxdelta;
      }
    }

    if (imgdata.idata.dng_version &&
        ((!strcasecmp(imgdata.idata.make, "Leica") && !strcasecmp(imgdata.idata.model, "D-LUX (Typ 109)")) ||
         (!strcasecmp(imgdata.idata.make, "Panasonic") && !strcasecmp(imgdata.idata.model, "LX100"))))
      imgdata.sizes.width = 4288;

    if (!strncasecmp(imgdata.idata.make, "Sony", 4) && imgdata.idata.dng_version)
    {
      if (S.raw_width == 3984)
        S.width = 3925;
      else if (S.raw_width == 4288)
        S.width = S.raw_width - 32;
      else if (S.raw_width == 4928 && S.height < 3280)
        S.width = S.raw_width - 8;
      else if (S.raw_width == 5504)
        S.width = S.raw_width - (S.height > 3664 ? 8 : 32);
      else if (S.raw_width == 6048)
      {
        S.width = S.raw_width - 24;
        if (strstr(imgdata.idata.model, "RX1") || strstr(imgdata.idata.model, "A99"))
          S.width -= 6;
      }
      else if (S.raw_width == 7392)
        S.width = S.raw_width - 30;
      else if (S.raw_width == 8000)
        S.width = S.raw_width - 32;
    }

    if (!strcasecmp(imgdata.idata.make, "Pentax") &&
        /*!strcasecmp(imgdata.idata.model,"K-3 II")  &&*/ imgdata.idata.raw_count == 4 &&
        (imgdata.params.raw_processing_options & LIBRAW_PROCESSING_PENTAX_PS_ALLFRAMES))
    {
      imgdata.idata.raw_count = 1;
      imgdata.idata.filters = 0;
      imgdata.idata.colors = 4;
      IO.mix_green = 1;
      pentax_component_load_raw = load_raw;
      load_raw = &LibRaw::pentax_4shot_load_raw;
    }

    if (!imgdata.idata.dng_version && !strcmp(imgdata.idata.make, "Leaf") && !strcmp(imgdata.idata.model, "Credo 50"))
    {
      imgdata.color.pre_mul[0] = 1.f / 0.3984f;
      imgdata.color.pre_mul[2] = 1.f / 0.7666f;
      imgdata.color.pre_mul[1] = imgdata.color.pre_mul[3] = 1.0;
    }

    // S3Pro DNG patch
    if (imgdata.idata.dng_version && !strcmp(imgdata.idata.make, "Fujifilm") && !strcmp(imgdata.idata.model, "S3Pro") &&
        imgdata.sizes.raw_width == 4288)
    {
      imgdata.sizes.left_margin++;
      imgdata.sizes.width--;
    }
    if (imgdata.idata.dng_version && !strcmp(imgdata.idata.make, "Fujifilm") && !strcmp(imgdata.idata.model, "S5Pro") &&
        imgdata.sizes.raw_width == 4288)
    {
      imgdata.sizes.left_margin++;
      imgdata.sizes.width--;
    }
    if (!imgdata.idata.dng_version && !strcmp(imgdata.idata.make, "Fujifilm") &&
        (!strncmp(imgdata.idata.model, "S20Pro", 6) || !strncmp(imgdata.idata.model, "F700", 4)))
    {
      imgdata.sizes.raw_width /= 2;
      load_raw = &LibRaw::unpacked_load_raw_fuji_f700s20;
    }
    if (load_raw == &LibRaw::packed_load_raw && !strcasecmp(imgdata.idata.make, "Nikon") &&
        !libraw_internal_data.unpacker_data.load_flags &&
        (!strncasecmp(imgdata.idata.model, "D810", 4) || !strcasecmp(imgdata.idata.model, "D4S")) &&
        libraw_internal_data.unpacker_data.data_size * 2 == imgdata.sizes.raw_height * imgdata.sizes.raw_width * 3)
    {
      libraw_internal_data.unpacker_data.load_flags = 80;
    }
    // Adjust BL for Sony A900/A850
    if (load_raw == &LibRaw::packed_load_raw &&
        !strcasecmp(imgdata.idata.make, "Sony")) // 12 bit sony, but metadata may be for 14-bit range
    {
      if (C.maximum > 4095)
        C.maximum = 4095;
      if (C.black > 256 || C.cblack[0] > 256)
      {
        C.black /= 4;
        for (int c = 0; c < 4; c++)
          C.cblack[c] /= 4;
        for (int c = 0; c < C.cblack[4] * C.cblack[5]; c++)
          C.cblack[6 + c] /= 4;
      }
    }
    if (load_raw == &LibRaw::nikon_yuv_load_raw) // Is it Nikon sRAW?
    {
      load_raw = &LibRaw::nikon_load_sraw;
      C.black = 0;
      memset(C.cblack, 0, sizeof(C.cblack));
      imgdata.idata.filters = 0;
      libraw_internal_data.unpacker_data.tiff_samples = 3;
      imgdata.idata.colors = 3;
      double beta_1 = -5.79342238397656E-02;
      double beta_2 = 3.28163551282665;
      double beta_3 = -8.43136004842678;
      double beta_4 = 1.03533181861023E+01;
      for (int i = 0; i <= 3072; i++)
      {
        double x = (double)i / 3072.;
        double y = (1. - exp(-beta_1 * x - beta_2 * x * x - beta_3 * x * x * x - beta_4 * x * x * x * x));
        if (y < 0.)
          y = 0.;
        imgdata.color.curve[i] = (y * 16383.);
      }
      for (int i = 0; i < 3; i++)
        for (int j = 0; j < 4; j++)
          imgdata.color.rgb_cam[i][j] = float(i == j);
    }
    // Adjust BL for Nikon 12bit
    if ((load_raw == &LibRaw::nikon_load_raw || load_raw == &LibRaw::packed_load_raw) &&
        !strcasecmp(imgdata.idata.make, "Nikon") && strncmp(imgdata.idata.model, "COOLPIX", 7)
        //	   && strncmp(imgdata.idata.model,"1 ",2)
        && libraw_internal_data.unpacker_data.tiff_bps == 12)
    {
      C.maximum = 4095;
      C.black /= 4;
      for (int c = 0; c < 4; c++)
        C.cblack[c] /= 4;
      for (int c = 0; c < C.cblack[4] * C.cblack[5]; c++)
        C.cblack[6 + c] /= 4;
    }

    // Adjust Highlight Linearity limit
    if (C.linear_max[0] < 0)
    {
      if (imgdata.idata.dng_version)
      {
        for (int c = 0; c < 4; c++)
          C.linear_max[c] = -1 * C.linear_max[c] + imgdata.color.cblack[c + 6];
      }
      else
      {
        for (int c = 0; c < 4; c++)
          C.linear_max[c] = -1 * C.linear_max[c] + imgdata.color.cblack[c];
      }
    }

    if (!strcasecmp(imgdata.idata.make, "Nikon") && (!C.linear_max[0]) && (C.maximum > 1024) &&
        (load_raw != &LibRaw::nikon_load_sraw))
    {
      C.linear_max[0] = C.linear_max[1] = C.linear_max[2] = C.linear_max[3] = (long)((float)(C.maximum) / 1.07f);
    }

    // Correct WB for Samsung GX20
    if (!strcasecmp(imgdata.idata.make, "Samsung") && !strcasecmp(imgdata.idata.model, "GX20"))
    {
      C.WB_Coeffs[LIBRAW_WBI_Daylight][2] = (int)((float)(C.WB_Coeffs[LIBRAW_WBI_Daylight][2]) * 2.56f);
      C.WB_Coeffs[LIBRAW_WBI_Shade][2] = (int)((float)(C.WB_Coeffs[LIBRAW_WBI_Shade][2]) * 2.56f);
      C.WB_Coeffs[LIBRAW_WBI_Cloudy][2] = (int)((float)(C.WB_Coeffs[LIBRAW_WBI_Cloudy][2]) * 2.56f);
      C.WB_Coeffs[LIBRAW_WBI_Tungsten][2] = (int)((float)(C.WB_Coeffs[LIBRAW_WBI_Tungsten][2]) * 2.56f);
      C.WB_Coeffs[LIBRAW_WBI_FL_D][2] = (int)((float)(C.WB_Coeffs[LIBRAW_WBI_FL_D][2]) * 2.56f);
      C.WB_Coeffs[LIBRAW_WBI_FL_N][2] = (int)((float)(C.WB_Coeffs[LIBRAW_WBI_FL_N][2]) * 2.56f);
      C.WB_Coeffs[LIBRAW_WBI_FL_W][2] = (int)((float)(C.WB_Coeffs[LIBRAW_WBI_FL_W][2]) * 2.56f);
      C.WB_Coeffs[LIBRAW_WBI_Flash][2] = (int)((float)(C.WB_Coeffs[LIBRAW_WBI_Flash][2]) * 2.56f);
      for (int c = 0; c < 64; c++)
      {
        if (imgdata.color.WBCT_Coeffs[c][0] > 0.0f)
        {
          imgdata.color.WBCT_Coeffs[c][3] *= 2.56f;
        }
      }
    }

    // Adjust BL for Panasonic
    if (load_raw == &LibRaw::panasonic_load_raw &&
        (!strcasecmp(imgdata.idata.make, "Panasonic") || !strcasecmp(imgdata.idata.make, "Leica") ||
         !strcasecmp(imgdata.idata.make, "YUNEEC")) &&
        ID.pana_black[0] && ID.pana_black[1] && ID.pana_black[2])
    {
      C.black = 0;
      C.cblack[0] = ID.pana_black[0] + ID.pana_black[3];
      C.cblack[1] = C.cblack[3] = ID.pana_black[1] + ID.pana_black[3];
      C.cblack[2] = ID.pana_black[2] + ID.pana_black[3];
      int i = C.cblack[3];
      for (int c = 0; c < 3; c++)
        if (i > C.cblack[c])
          i = C.cblack[c];
      for (int c = 0; c < 4; c++)
        C.cblack[c] -= i;
      C.black = i;
    }

    // Adjust sizes for X3F processing
    if (load_raw == &LibRaw::x3f_load_raw)
    {
      for (int i = 0; i < foveon_count; i++)
        if (!strcasecmp(imgdata.idata.make, foveon_data[i].make) &&
            !strcasecmp(imgdata.idata.model, foveon_data[i].model) &&
            imgdata.sizes.raw_width == foveon_data[i].raw_width &&
            imgdata.sizes.raw_height == foveon_data[i].raw_height)
        {
          imgdata.sizes.top_margin = foveon_data[i].top_margin;
          imgdata.sizes.left_margin = foveon_data[i].left_margin;
          imgdata.sizes.width = imgdata.sizes.iwidth = foveon_data[i].width;
          imgdata.sizes.height = imgdata.sizes.iheight = foveon_data[i].height;
          C.maximum = foveon_data[i].white;
          break;
        }
    }
#if 0
    size_t bytes = ID.input->size()-libraw_internal_data.unpacker_data.data_offset;
    float bpp = float(bytes)/float(S.raw_width)/float(S.raw_height);
    float bpp2 = float(bytes)/float(S.width)/float(S.height);
    printf("RawSize: %dx%d data offset: %d data size:%d bpp: %g bpp2: %g\n",S.raw_width,S.raw_height,libraw_internal_data.unpacker_data.data_offset,bytes,bpp,bpp2);
    if(!strcasecmp(imgdata.idata.make,"Hasselblad") && bpp == 6.0f)
      {
        load_raw = &LibRaw::hasselblad_full_load_raw;
        S.width = S.raw_width;
        S.height = S.raw_height;
        P1.filters = 0;
        P1.colors=3;
        P1.raw_count=1;
        C.maximum=0xffff;
        printf("3 channel hassy found\n");
      }
#endif
    if (C.profile_length)
    {
      if (C.profile)
        free(C.profile);
      C.profile = malloc(C.profile_length);
      merror(C.profile, "LibRaw::open_file()");
      ID.input->seek(ID.profile_offset, SEEK_SET);
      ID.input->read(C.profile, C.profile_length, 1);
    }

    SET_PROC_FLAG(LIBRAW_PROGRESS_IDENTIFY);
  }
  catch (LibRaw_exceptions err)
  {
    EXCEPTION_HANDLER(err);
  }
  catch (std::exception ee)
  {
    EXCEPTION_HANDLER(LIBRAW_EXCEPTION_IO_CORRUPT);
  }

  if (P1.raw_count < 1)
    return LIBRAW_FILE_UNSUPPORTED;

  write_fun = &LibRaw::write_ppm_tiff;

  if (load_raw == &LibRaw::kodak_ycbcr_load_raw)
  {
    S.height += S.height & 1;
    S.width += S.width & 1;
  }

  IO.shrink = P1.filters && (O.half_size || ((O.threshold || O.aber[0] != 1 || O.aber[2] != 1)));

  S.iheight = (S.height + IO.shrink) >> IO.shrink;
  S.iwidth = (S.width + IO.shrink) >> IO.shrink;

  // Save color,sizes and internal data into raw_image fields
  memmove(&imgdata.rawdata.color, &imgdata.color, sizeof(imgdata.color));
  memmove(&imgdata.rawdata.sizes, &imgdata.sizes, sizeof(imgdata.sizes));
  memmove(&imgdata.rawdata.iparams, &imgdata.idata, sizeof(imgdata.idata));
  memmove(&imgdata.rawdata.ioparams, &libraw_internal_data.internal_output_params,
          sizeof(libraw_internal_data.internal_output_params));

  SET_PROC_FLAG(LIBRAW_PROGRESS_SIZE_ADJUST);

  return LIBRAW_SUCCESS;
}

#ifdef USE_RAWSPEED
void LibRaw::fix_after_rawspeed(int bl)
{
  if (load_raw == &LibRaw::lossy_dng_load_raw)
    C.maximum = 0xffff;
  else if (load_raw == &LibRaw::sony_load_raw)
    C.maximum = 0x3ff0;
}
#else
void LibRaw::fix_after_rawspeed(int) {}
#endif

void LibRaw::clearCancelFlag()
{
#ifdef WIN32
  InterlockedExchange(&_exitflag, 0);
#else
  __sync_fetch_and_and(&_exitflag, 0);
#endif
#ifdef RAWSPEED_FASTEXIT
  if (_rawspeed_decoder)
  {
    RawDecoder *d = static_cast<RawDecoder *>(_rawspeed_decoder);
    d->resumeProcessing();
  }
#endif
}

void LibRaw::setCancelFlag()
{
#ifdef WIN32
  InterlockedExchange(&_exitflag, 1);
#else
  __sync_fetch_and_add(&_exitflag, 1);
#endif
#ifdef RAWSPEED_FASTEXIT
  if (_rawspeed_decoder)
  {
    RawDecoder *d = static_cast<RawDecoder *>(_rawspeed_decoder);
    d->cancelProcessing();
  }
#endif
}

void LibRaw::checkCancel()
{
#ifdef WIN32
  if (InterlockedExchange(&_exitflag, 0))
    throw LIBRAW_EXCEPTION_CANCELLED_BY_CALLBACK;
#else
  if (__sync_fetch_and_and(&_exitflag, 0))
    throw LIBRAW_EXCEPTION_CANCELLED_BY_CALLBACK;
#endif
}

int LibRaw::try_rawspeed()
{
#ifdef USE_RAWSPEED
  int ret = LIBRAW_SUCCESS;

  int rawspeed_ignore_errors = 0;
  if (imgdata.idata.dng_version && imgdata.idata.colors == 3 &&
      !strcasecmp(imgdata.idata.software, "Adobe Photoshop Lightroom 6.1.1 (Windows)"))
    rawspeed_ignore_errors = 1;

  // RawSpeed Supported,
  INT64 spos = ID.input->tell();
  void *_rawspeed_buffer = 0;
  try
  {
    //                printf("Using rawspeed\n");
    ID.input->seek(0, SEEK_SET);
    INT64 _rawspeed_buffer_sz = ID.input->size() + 32;
    _rawspeed_buffer = malloc(_rawspeed_buffer_sz);
    if (!_rawspeed_buffer)
      throw LIBRAW_EXCEPTION_ALLOC;
    ID.input->read(_rawspeed_buffer, _rawspeed_buffer_sz, 1);
    FileMap map((uchar8 *)_rawspeed_buffer, _rawspeed_buffer_sz);
    RawParser t(&map);
    RawDecoder *d = 0;
    CameraMetaDataLR *meta = static_cast<CameraMetaDataLR *>(_rawspeed_camerameta);
    d = t.getDecoder();
    if (!d)
      throw "Unable to find decoder";
    try
    {
      d->checkSupport(meta);
    }
    catch (const RawDecoderException &e)
    {
      imgdata.process_warnings |= LIBRAW_WARN_RAWSPEED_UNSUPPORTED;
      throw e;
    }
    d->interpolateBadPixels = FALSE;
    d->applyStage1DngOpcodes = FALSE;
    _rawspeed_decoder = static_cast<void *>(d);
    d->decodeRaw();
    d->decodeMetaData(meta);
    RawImage r = d->mRaw;
    if (r->errors.size() > 0 && !rawspeed_ignore_errors)
    {
      delete d;
      _rawspeed_decoder = 0;
      throw 1;
    }
    if (r->isCFA)
    {
      imgdata.rawdata.raw_image = (ushort *)r->getDataUncropped(0, 0);
    }
    else if (r->getCpp() == 4)
    {
      imgdata.rawdata.color4_image = (ushort(*)[4])r->getDataUncropped(0, 0);
      if (r->whitePoint > 0 && r->whitePoint < 65536)
        C.maximum = r->whitePoint;
    }
    else if (r->getCpp() == 3)
    {
      imgdata.rawdata.color3_image = (ushort(*)[3])r->getDataUncropped(0, 0);
      if (r->whitePoint > 0 && r->whitePoint < 65536)
        C.maximum = r->whitePoint;
    }
    else
    {
      delete d;
      _rawspeed_decoder = 0;
      ret = LIBRAW_UNSPECIFIED_ERROR;
    }
    if (_rawspeed_decoder)
    {
      // set sizes
      iPoint2D rsdim = r->getUncroppedDim();
      S.raw_pitch = r->pitch;
      S.raw_width = rsdim.x;
      S.raw_height = rsdim.y;
      // C.maximum = r->whitePoint;
      fix_after_rawspeed(r->blackLevel);
    }
    free(_rawspeed_buffer);
    _rawspeed_buffer = 0;
    imgdata.process_warnings |= LIBRAW_WARN_RAWSPEED_PROCESSED;
  }
  catch (const RawDecoderException &RDE)
  {
    imgdata.process_warnings |= LIBRAW_WARN_RAWSPEED_PROBLEM;
    if (_rawspeed_buffer)
    {
      free(_rawspeed_buffer);
      _rawspeed_buffer = 0;
    }
    const char *p = RDE.what();
    if (!strncmp(RDE.what(), "Decoder canceled", strlen("Decoder canceled")))
      throw LIBRAW_EXCEPTION_CANCELLED_BY_CALLBACK;
    ret = LIBRAW_UNSPECIFIED_ERROR;
  }
  catch (...)
  {
    // We may get here due to cancellation flag
    imgdata.process_warnings |= LIBRAW_WARN_RAWSPEED_PROBLEM;
    if (_rawspeed_buffer)
    {
      free(_rawspeed_buffer);
      _rawspeed_buffer = 0;
    }
    ret = LIBRAW_UNSPECIFIED_ERROR;
  }
  ID.input->seek(spos, SEEK_SET);

  return ret;
#else
  return LIBRAW_NOT_IMPLEMENTED;
#endif
}

int LibRaw::valid_for_dngsdk()
{
#ifndef USE_DNGSDK
  return 0;
#else
  if (!imgdata.idata.dng_version)
    return 0;
  if (!imgdata.params.use_dngsdk)
    return 0;
  if (load_raw == &LibRaw::lossy_dng_load_raw)
    return 0;
  if (is_floating_point() && (imgdata.params.use_dngsdk & LIBRAW_DNG_FLOAT))
    return 1;
  if (!imgdata.idata.filters && (imgdata.params.use_dngsdk & LIBRAW_DNG_LINEAR))
    return 1;
  if (libraw_internal_data.unpacker_data.tiff_bps == 8 && (imgdata.params.use_dngsdk & LIBRAW_DNG_8BIT))
    return 1;
  if (libraw_internal_data.unpacker_data.tiff_compress == 8 && (imgdata.params.use_dngsdk & LIBRAW_DNG_DEFLATE))
    return 1;
  if (libraw_internal_data.unpacker_data.tiff_samples == 2)
    return 0; // Always deny 2-samples (old fuji superccd)
  if (imgdata.idata.filters == 9 && (imgdata.params.use_dngsdk & LIBRAW_DNG_XTRANS))
    return 1;
  if (is_fuji_rotated())
    return 0; // refuse
  if (imgdata.params.use_dngsdk & LIBRAW_DNG_OTHER)
    return 1;
  return 0;
#endif
}

int LibRaw::is_curve_linear()
{
  for (int i = 0; i < 0x10000; i++)
    if (imgdata.color.curve[i] != i)
      return 0;
  return 1;
}

int LibRaw::try_dngsdk()
{
#ifdef USE_DNGSDK
  if (!dnghost)
    return LIBRAW_UNSPECIFIED_ERROR;

  dng_host *host = static_cast<dng_host *>(dnghost);

  try
  {
    libraw_dng_stream stream(libraw_internal_data.internal_data.input);

    AutoPtr<dng_negative> negative;
    negative.Reset(host->Make_dng_negative());

    dng_info info;
    info.Parse(*host, stream);
    info.PostParse(*host);

    if (!info.IsValidDNG())
    {
      return LIBRAW_DATA_ERROR;
    }
    negative->Parse(*host, stream, info);
    negative->PostParse(*host, stream, info);
    negative->ReadStage1Image(*host, stream, info);
    dng_simple_image *stage2 = (dng_simple_image *)negative->Stage1Image();
    if (stage2->Bounds().W() != S.raw_width || stage2->Bounds().H() != S.raw_height)
    {
      return LIBRAW_DATA_ERROR;
    }

    int pplanes = stage2->Planes();
    int ptype = stage2->PixelType();

    dng_pixel_buffer buffer;
    stage2->GetPixelBuffer(buffer);

    int pixels = stage2->Bounds().H() * stage2->Bounds().W() * pplanes;
    if (ptype == ttByte)
      imgdata.rawdata.raw_alloc = malloc(pixels * TagTypeSize(ttShort));
    else
      imgdata.rawdata.raw_alloc = malloc(pixels * TagTypeSize(ptype));

    if (ptype == ttShort && !is_curve_linear())
    {
      ushort *src = (ushort *)buffer.fData;
      ushort *dst = (ushort *)imgdata.rawdata.raw_alloc;
      for (int i = 0; i < pixels; i++)
        dst[i] = imgdata.color.curve[src[i]];
      S.raw_pitch = S.raw_width * pplanes * TagTypeSize(ptype);
    }
    else if (ptype == ttByte)
    {
      unsigned char *src = (unsigned char *)buffer.fData;
      ushort *dst = (ushort *)imgdata.rawdata.raw_alloc;
      if (is_curve_linear())
      {
        for (int i = 0; i < pixels; i++)
          dst[i] = src[i];
      }
      else
      {
        for (int i = 0; i < pixels; i++)
          dst[i] = imgdata.color.curve[src[i]];
      }
      S.raw_pitch = S.raw_width * pplanes * TagTypeSize(ttShort);
    }
    else
    {
      memmove(imgdata.rawdata.raw_alloc, buffer.fData, pixels * TagTypeSize(ptype));
      S.raw_pitch = S.raw_width * pplanes * TagTypeSize(ptype);
    }

    switch (ptype)
    {
    case ttFloat:
      if (pplanes == 1)
        imgdata.rawdata.float_image = (float *)imgdata.rawdata.raw_alloc;
      else if (pplanes == 3)
        imgdata.rawdata.float3_image = (float(*)[3])imgdata.rawdata.raw_alloc;
      else if (pplanes == 4)
        imgdata.rawdata.float4_image = (float(*)[4])imgdata.rawdata.raw_alloc;
      break;

    case ttByte:
    case ttShort:
      if (pplanes == 1)
        imgdata.rawdata.raw_image = (ushort *)imgdata.rawdata.raw_alloc;
      else if (pplanes == 3)
        imgdata.rawdata.color3_image = (ushort(*)[3])imgdata.rawdata.raw_alloc;
      else if (pplanes == 4)
        imgdata.rawdata.color4_image = (ushort(*)[4])imgdata.rawdata.raw_alloc;
      break;
    default:
      /* do nothing */
      break;
    }
  }
  catch (...)
  {
    return LIBRAW_UNSPECIFIED_ERROR;
  }
  return imgdata.rawdata.raw_alloc ? LIBRAW_SUCCESS : LIBRAW_UNSPECIFIED_ERROR;
#else
  return LIBRAW_UNSPECIFIED_ERROR;
#endif
}
void LibRaw::set_dng_host(void *p)
{
#ifdef USE_DNGSDK
  dnghost = p;
#endif
}

int LibRaw::unpack(void)
{
  CHECK_ORDER_HIGH(LIBRAW_PROGRESS_LOAD_RAW);
  CHECK_ORDER_LOW(LIBRAW_PROGRESS_IDENTIFY);
  try
  {

    if (!libraw_internal_data.internal_data.input)
      return LIBRAW_INPUT_CLOSED;

    RUN_CALLBACK(LIBRAW_PROGRESS_LOAD_RAW, 0, 2);
    if (O.shot_select >= P1.raw_count)
      return LIBRAW_REQUEST_FOR_NONEXISTENT_IMAGE;

    if (!load_raw)
      return LIBRAW_UNSPECIFIED_ERROR;

    // already allocated ?
    if (imgdata.image)
    {
      free(imgdata.image);
      imgdata.image = 0;
    }
    if (imgdata.rawdata.raw_alloc)
    {
      free(imgdata.rawdata.raw_alloc);
      imgdata.rawdata.raw_alloc = 0;
    }
    if (libraw_internal_data.unpacker_data.meta_length)
    {
      libraw_internal_data.internal_data.meta_data = (char *)malloc(libraw_internal_data.unpacker_data.meta_length);
      merror(libraw_internal_data.internal_data.meta_data, "LibRaw::unpack()");
    }

    libraw_decoder_info_t decoder_info;
    get_decoder_info(&decoder_info);

    int save_iwidth = S.iwidth, save_iheight = S.iheight, save_shrink = IO.shrink;

    int rwidth = S.raw_width, rheight = S.raw_height;
    if (!IO.fuji_width)
    {
      // adjust non-Fuji allocation
      if (rwidth < S.width + S.left_margin)
        rwidth = S.width + S.left_margin;
      if (rheight < S.height + S.top_margin)
        rheight = S.height + S.top_margin;
    }

    imgdata.rawdata.raw_image = 0;
    imgdata.rawdata.color4_image = 0;
    imgdata.rawdata.color3_image = 0;
    imgdata.rawdata.float_image = 0;
    imgdata.rawdata.float3_image = 0;

#ifdef USE_DNGSDK
    if (imgdata.idata.dng_version && dnghost && imgdata.idata.raw_count == 1 && valid_for_dngsdk() &&
        load_raw != &LibRaw::pentax_4shot_load_raw)
    {
      int rr = try_dngsdk();
    }
#endif

#ifdef USE_RAWSPEED
    if (!raw_was_read())
    {
      int rawspeed_enabled = 1;

      if (imgdata.idata.dng_version && libraw_internal_data.unpacker_data.tiff_samples == 2)
        rawspeed_enabled = 0;

      if (imgdata.idata.raw_count > 1)
        rawspeed_enabled = 0;
      if (!strncasecmp(imgdata.idata.software, "Magic", 5))
        rawspeed_enabled = 0;
      // Disable rawspeed for double-sized Oly files
      if (!strncasecmp(imgdata.idata.make, "Olympus", 7) &&
          ((imgdata.sizes.raw_width > 6000) || !strncasecmp(imgdata.idata.model, "SH-2", 4) ||
           !strncasecmp(imgdata.idata.model, "SH-3", 4) || !strncasecmp(imgdata.idata.model, "TG-4", 4)))
        rawspeed_enabled = 0;

      if (imgdata.idata.dng_version && imgdata.idata.filters == 0 &&
          libraw_internal_data.unpacker_data.tiff_bps == 8) // Disable for 8 bit
        rawspeed_enabled = 0;

      if (load_raw == &LibRaw::packed_load_raw && !strncasecmp(imgdata.idata.make, "Nikon", 5) &&
          (!strncasecmp(imgdata.idata.model, "E", 1) || !strncasecmp(imgdata.idata.model, "COOLPIX B", 9)))
        rawspeed_enabled = 0;

      // RawSpeed Supported,
      if (O.use_rawspeed && rawspeed_enabled &&
          !(is_sraw() &&
            (O.raw_processing_options & (LIBRAW_PROCESSING_SRAW_NO_RGB | LIBRAW_PROCESSING_SRAW_NO_INTERPOLATE))) &&
          (decoder_info.decoder_flags & LIBRAW_DECODER_TRYRAWSPEED) && _rawspeed_camerameta)
      {
        int rr = try_rawspeed();
      }
    }
#endif
    if (!raw_was_read()) // RawSpeed failed or not run
    {
      // Not allocated on RawSpeed call, try call LibRaow
      int zero_rawimage = 0;
      if (decoder_info.decoder_flags & LIBRAW_DECODER_OWNALLOC)
      {
        // x3f foveon decoder and DNG float
        // Do nothing! Decoder will allocate data internally
      }
      else if (imgdata.idata.filters || P1.colors == 1) // Bayer image or single color -> decode to raw_image
      {
        imgdata.rawdata.raw_alloc = malloc(rwidth * (rheight + 8) * sizeof(imgdata.rawdata.raw_image[0]));
        imgdata.rawdata.raw_image = (ushort *)imgdata.rawdata.raw_alloc;
        if (!S.raw_pitch)
          S.raw_pitch = S.raw_width * 2; // Bayer case, not set before
      }
      else // NO LEGACY FLAG if (decoder_info.decoder_flags & LIBRAW_DECODER_LEGACY)
      {
        // sRAW and old Foveon decoders only, so extra buffer size is just 1/4
        S.iwidth = S.width;
        S.iheight = S.height;
        IO.shrink = 0;
        if (!S.raw_pitch)
          S.raw_pitch =
              (decoder_info.decoder_flags & LIBRAW_DECODER_LEGACY_WITH_MARGINS) ? S.raw_width * 8 : S.width * 8;
        // allocate image as temporary buffer, size
        imgdata.rawdata.raw_alloc = 0;
        imgdata.image = (ushort(*)[4])calloc(unsigned(S.raw_width) * unsigned(S.raw_height), sizeof(*imgdata.image));
        if (!(decoder_info.decoder_flags & LIBRAW_DECODER_ADOBECOPYPIXEL))
        {
          imgdata.rawdata.raw_image = (ushort *)imgdata.image;
          zero_rawimage = 1;
        }
      }
      ID.input->seek(libraw_internal_data.unpacker_data.data_offset, SEEK_SET);

      unsigned m_save = C.maximum;
      if (load_raw == &LibRaw::unpacked_load_raw && !strcasecmp(imgdata.idata.make, "Nikon"))
        C.maximum = 65535;
      (this->*load_raw)();
      if (zero_rawimage)
        imgdata.rawdata.raw_image = 0;
      if (load_raw == &LibRaw::unpacked_load_raw && !strcasecmp(imgdata.idata.make, "Nikon"))
        C.maximum = m_save;
      if (decoder_info.decoder_flags & LIBRAW_DECODER_OWNALLOC)
      {
        // x3f foveon decoder only: do nothing
      }
      else if (!(imgdata.idata.filters || P1.colors == 1)) // legacy decoder, ownalloc handled above
      {
        // successfully decoded legacy image, attach image to raw_alloc
        imgdata.rawdata.raw_alloc = imgdata.image;
        imgdata.rawdata.color4_image = (ushort(*)[4])imgdata.rawdata.raw_alloc;
        imgdata.image = 0;
        // Restore saved values. Note: Foveon have masked frame
        // Other 4-color legacy data: no borders
        if (!(libraw_internal_data.unpacker_data.load_flags & 256))
        {
          S.raw_width = S.width;
          S.left_margin = 0;
          S.raw_height = S.height;
          S.top_margin = 0;
        }
      }
    }

    if (imgdata.rawdata.raw_image)
      crop_masked_pixels(); // calculate black levels

    // recover image sizes
    S.iwidth = save_iwidth;
    S.iheight = save_iheight;
    IO.shrink = save_shrink;

    // adjust black to possible maximum
    unsigned int i = C.cblack[3];
    unsigned int c;
    for (c = 0; c < 3; c++)
      if (i > C.cblack[c])
        i = C.cblack[c];
    for (c = 0; c < 4; c++)
      C.cblack[c] -= i;
    C.black += i;

    // Save color,sizes and internal data into raw_image fields
    memmove(&imgdata.rawdata.color, &imgdata.color, sizeof(imgdata.color));
    memmove(&imgdata.rawdata.sizes, &imgdata.sizes, sizeof(imgdata.sizes));
    memmove(&imgdata.rawdata.iparams, &imgdata.idata, sizeof(imgdata.idata));
    memmove(&imgdata.rawdata.ioparams, &libraw_internal_data.internal_output_params,
            sizeof(libraw_internal_data.internal_output_params));

    SET_PROC_FLAG(LIBRAW_PROGRESS_LOAD_RAW);
    RUN_CALLBACK(LIBRAW_PROGRESS_LOAD_RAW, 1, 2);

    return 0;
  }
  catch (LibRaw_exceptions err)
  {
    EXCEPTION_HANDLER(err);
  }
  catch (std::exception ee)
  {
    EXCEPTION_HANDLER(LIBRAW_EXCEPTION_IO_CORRUPT);
  }
}

void LibRaw::unpacked_load_raw_fuji_f700s20()
{
  int base_offset = 0;
  int row_size = imgdata.sizes.raw_width * 2; // in bytes
  if (imgdata.idata.raw_count == 2 && imgdata.params.shot_select)
  {
    libraw_internal_data.internal_data.input->seek(-row_size, SEEK_CUR);
    base_offset = row_size; // in bytes
  }
  unsigned char *buffer = (unsigned char *)malloc(row_size * 2);
  for (int row = 0; row < imgdata.sizes.raw_height; row++)
  {
    read_shorts((ushort *)buffer, imgdata.sizes.raw_width * 2);
    memmove(&imgdata.rawdata.raw_image[row * imgdata.sizes.raw_pitch / 2], buffer + base_offset, row_size);
  }
  free(buffer);
}

void LibRaw::nikon_load_sraw()
{
  // We're already seeked to data!
  unsigned char *rd = (unsigned char *)malloc(3 * (imgdata.sizes.raw_width + 2));
  if (!rd)
    throw LIBRAW_EXCEPTION_ALLOC;
  try
  {
    int row, col;
    for (row = 0; row < imgdata.sizes.raw_height; row++)
    {
      checkCancel();
      libraw_internal_data.internal_data.input->read(rd, 3, imgdata.sizes.raw_width);
      for (col = 0; col < imgdata.sizes.raw_width - 1; col += 2)
      {
        int bi = col * 3;
        ushort bits1 = (rd[bi + 1] & 0xf) << 8 | rd[bi];            // 3,0,1
        ushort bits2 = rd[bi + 2] << 4 | ((rd[bi + 1] >> 4) & 0xf); // 452
        ushort bits3 = ((rd[bi + 4] & 0xf) << 8) | rd[bi + 3];      // 967
        ushort bits4 = rd[bi + 5] << 4 | ((rd[bi + 4] >> 4) & 0xf); // ab8
        imgdata.image[row * imgdata.sizes.raw_width + col][0] = bits1;
        imgdata.image[row * imgdata.sizes.raw_width + col][1] = bits3;
        imgdata.image[row * imgdata.sizes.raw_width + col][2] = bits4;
        imgdata.image[row * imgdata.sizes.raw_width + col + 1][0] = bits2;
        imgdata.image[row * imgdata.sizes.raw_width + col + 1][1] = 2048;
        imgdata.image[row * imgdata.sizes.raw_width + col + 1][2] = 2048;
      }
    }
  }
  catch (...)
  {
    free(rd);
    throw;
  }
  free(rd);
  C.maximum = 0xfff; // 12 bit?
  if (imgdata.params.raw_processing_options & LIBRAW_PROCESSING_SRAW_NO_INTERPOLATE)
  {
    return; // no CbCr interpolation
  }
  // Interpolate CC channels
  int row, col;
  for (row = 0; row < imgdata.sizes.raw_height; row++)
  {
    checkCancel(); // will throw out
    for (col = 0; col < imgdata.sizes.raw_width; col += 2)
    {
      int col2 = col < imgdata.sizes.raw_width - 2 ? col + 2 : col;
      imgdata.image[row * imgdata.sizes.raw_width + col + 1][1] =
          (unsigned short)(int(imgdata.image[row * imgdata.sizes.raw_width + col][1] +
                               imgdata.image[row * imgdata.sizes.raw_width + col2][1]) /
                           2);
      imgdata.image[row * imgdata.sizes.raw_width + col + 1][2] =
          (unsigned short)(int(imgdata.image[row * imgdata.sizes.raw_width + col][2] +
                               imgdata.image[row * imgdata.sizes.raw_width + col2][2]) /
                           2);
    }
  }
  if (imgdata.params.raw_processing_options & LIBRAW_PROCESSING_SRAW_NO_RGB)
    return;

  for (row = 0; row < imgdata.sizes.raw_height; row++)
  {
    checkCancel(); // will throw out
    for (col = 0; col < imgdata.sizes.raw_width; col++)
    {
      float Y = float(imgdata.image[row * imgdata.sizes.raw_width + col][0]) / 2549.f;
      float Ch2 = float(imgdata.image[row * imgdata.sizes.raw_width + col][1] - 1280) / 1536.f;
      float Ch3 = float(imgdata.image[row * imgdata.sizes.raw_width + col][2] - 1280) / 1536.f;
      if (Y > 1.f)
        Y = 1.f;
      if (Y > 0.803f)
        Ch2 = Ch3 = 0.5f;
      float r = Y + 1.40200f * (Ch3 - 0.5f);
      if (r < 0.f)
        r = 0.f;
      if (r > 1.f)
        r = 1.f;
      float g = Y - 0.34414f * (Ch2 - 0.5f) - 0.71414 * (Ch3 - 0.5f);
      if (g > 1.f)
        g = 1.f;
      if (g < 0.f)
        g = 0.f;
      float b = Y + 1.77200 * (Ch2 - 0.5f);
      if (b > 1.f)
        b = 1.f;
      if (b < 0.f)
        b = 0.f;
      imgdata.image[row * imgdata.sizes.raw_width + col][0] = imgdata.color.curve[int(r * 3072.f)];
      imgdata.image[row * imgdata.sizes.raw_width + col][1] = imgdata.color.curve[int(g * 3072.f)];
      imgdata.image[row * imgdata.sizes.raw_width + col][2] = imgdata.color.curve[int(b * 3072.f)];
    }
  }
  C.maximum = 16383;
}

void LibRaw::free_image(void)
{
  if (imgdata.image)
  {
    free(imgdata.image);
    imgdata.image = 0;
    imgdata.progress_flags = LIBRAW_PROGRESS_START | LIBRAW_PROGRESS_OPEN | LIBRAW_PROGRESS_IDENTIFY |
                             LIBRAW_PROGRESS_SIZE_ADJUST | LIBRAW_PROGRESS_LOAD_RAW;
  }
}

void LibRaw::raw2image_start()
{
  // restore color,sizes and internal data into raw_image fields
  memmove(&imgdata.color, &imgdata.rawdata.color, sizeof(imgdata.color));
  memmove(&imgdata.sizes, &imgdata.rawdata.sizes, sizeof(imgdata.sizes));
  memmove(&imgdata.idata, &imgdata.rawdata.iparams, sizeof(imgdata.idata));
  memmove(&libraw_internal_data.internal_output_params, &imgdata.rawdata.ioparams,
          sizeof(libraw_internal_data.internal_output_params));

  if (O.user_flip >= 0)
    S.flip = O.user_flip;

  switch ((S.flip + 3600) % 360)
  {
  case 270:
    S.flip = 5;
    break;
  case 180:
    S.flip = 3;
    break;
  case 90:
    S.flip = 6;
    break;
  }

  // adjust for half mode!
  IO.shrink = P1.filters && (O.half_size || ((O.threshold || O.aber[0] != 1 || O.aber[2] != 1)));

  S.iheight = (S.height + IO.shrink) >> IO.shrink;
  S.iwidth = (S.width + IO.shrink) >> IO.shrink;
}

int LibRaw::is_phaseone_compressed()
{
  return (load_raw == &LibRaw::phase_one_load_raw_c || load_raw == &LibRaw::phase_one_load_raw);
}

int LibRaw::is_canon_600() { return load_raw == &LibRaw::canon_600_load_raw; }

int LibRaw::raw2image(void)
{

  CHECK_ORDER_LOW(LIBRAW_PROGRESS_LOAD_RAW);

  try
  {
    raw2image_start();

    if (is_phaseone_compressed())
    {
      phase_one_allocate_tempbuffer();
      int rc = phase_one_subtract_black((ushort *)imgdata.rawdata.raw_alloc, imgdata.rawdata.raw_image);
      if (rc == 0)
        rc = phase_one_correct();
      if (rc != 0)
      {
        phase_one_free_tempbuffer();
        return rc;
      }
    }

    // free and re-allocate image bitmap
    if (imgdata.image)
    {
      imgdata.image = (ushort(*)[4])realloc(imgdata.image, S.iheight * S.iwidth * sizeof(*imgdata.image));
      memset(imgdata.image, 0, S.iheight * S.iwidth * sizeof(*imgdata.image));
    }
    else
      imgdata.image = (ushort(*)[4])calloc(S.iheight * S.iwidth, sizeof(*imgdata.image));

    merror(imgdata.image, "raw2image()");

    libraw_decoder_info_t decoder_info;
    get_decoder_info(&decoder_info);

    // Move saved bitmap to imgdata.image
    if (imgdata.idata.filters || P1.colors == 1)
    {
      if (IO.fuji_width)
      {
        unsigned r, c;
        int row, col;
        for (row = 0; row < S.raw_height - S.top_margin * 2; row++)
        {
          for (col = 0; col < IO.fuji_width << !libraw_internal_data.unpacker_data.fuji_layout; col++)
          {
            if (libraw_internal_data.unpacker_data.fuji_layout)
            {
              r = IO.fuji_width - 1 - col + (row >> 1);
              c = col + ((row + 1) >> 1);
            }
            else
            {
              r = IO.fuji_width - 1 + row - (col >> 1);
              c = row + ((col + 1) >> 1);
            }
            if (r < S.height && c < S.width)
              imgdata.image[((r) >> IO.shrink) * S.iwidth + ((c) >> IO.shrink)][FC(r, c)] =
                  imgdata.rawdata.raw_image[(row + S.top_margin) * S.raw_pitch / 2 + (col + S.left_margin)];
          }
        }
      }
      else
      {
        int row, col;
        for (row = 0; row < S.height; row++)
          for (col = 0; col < S.width; col++)
            imgdata.image[((row) >> IO.shrink) * S.iwidth + ((col) >> IO.shrink)][fcol(row, col)] =
                imgdata.rawdata.raw_image[(row + S.top_margin) * S.raw_pitch / 2 + (col + S.left_margin)];
      }
    }
    else // if(decoder_info.decoder_flags & LIBRAW_DECODER_LEGACY)
    {
      if (imgdata.rawdata.color4_image)
      {
        if (S.width * 8 == S.raw_pitch)
          memmove(imgdata.image, imgdata.rawdata.color4_image, S.width * S.height * sizeof(*imgdata.image));
        else
        {
          for (int row = 0; row < S.height; row++)
            memmove(&imgdata.image[row * S.width],
                    &imgdata.rawdata.color4_image[(row + S.top_margin) * S.raw_pitch / 8 + S.left_margin],
                    S.width * sizeof(*imgdata.image));
        }
      }
      else if (imgdata.rawdata.color3_image)
      {
        unsigned char *c3image = (unsigned char *)imgdata.rawdata.color3_image;
        for (int row = 0; row < S.height; row++)
        {
          ushort(*srcrow)[3] = (ushort(*)[3]) & c3image[(row + S.top_margin) * S.raw_pitch];
          ushort(*dstrow)[4] = (ushort(*)[4]) & imgdata.image[row * S.width];
          for (int col = 0; col < S.width; col++)
          {
            for (int c = 0; c < 3; c++)
              dstrow[col][c] = srcrow[S.left_margin + col][c];
            dstrow[col][3] = 0;
          }
        }
      }
      else
      {
        // legacy decoder, but no data?
        throw LIBRAW_EXCEPTION_DECODE_RAW;
      }
    }

    // Free PhaseOne separate copy allocated at function start
    if (is_phaseone_compressed())
    {
      phase_one_free_tempbuffer();
    }
    // hack - clear later flags!

    if (load_raw == &CLASS canon_600_load_raw && S.width < S.raw_width)
    {
      canon_600_correct();
    }

    imgdata.progress_flags = LIBRAW_PROGRESS_START | LIBRAW_PROGRESS_OPEN | LIBRAW_PROGRESS_RAW2_IMAGE |
                             LIBRAW_PROGRESS_IDENTIFY | LIBRAW_PROGRESS_SIZE_ADJUST | LIBRAW_PROGRESS_LOAD_RAW;
    return 0;
  }
  catch (LibRaw_exceptions err)
  {
    EXCEPTION_HANDLER(err);
  }
}

void LibRaw::phase_one_allocate_tempbuffer()
{
  // Allocate temp raw_image buffer
  imgdata.rawdata.raw_image = (ushort *)malloc(S.raw_pitch * S.raw_height);
  merror(imgdata.rawdata.raw_image, "phase_one_prepare_to_correct()");
}
void LibRaw::phase_one_free_tempbuffer()
{
  free(imgdata.rawdata.raw_image);
  imgdata.rawdata.raw_image = (ushort *)imgdata.rawdata.raw_alloc;
}

int LibRaw::phase_one_subtract_black(ushort *src, ushort *dest)
{

  try
  {
    if (O.user_black < 0 && O.user_cblack[0] <= -1000000 && O.user_cblack[1] <= -1000000 &&
        O.user_cblack[2] <= -1000000 && O.user_cblack[3] <= -1000000)
    {
      if (!imgdata.rawdata.ph1_cblack || !imgdata.rawdata.ph1_rblack)
      {
        register int bl = imgdata.color.phase_one_data.t_black;
        for (int row = 0; row < S.raw_height; row++)
        {
          checkCancel();
          for (int col = 0; col < S.raw_width; col++)
          {
            int idx = row * S.raw_width + col;
            int val = int(src[idx]) - bl;
            dest[idx] = val > 0 ? val : 0;
          }
        }
      }
      else
      {
        register int bl = imgdata.color.phase_one_data.t_black;
        for (int row = 0; row < S.raw_height; row++)
        {
          checkCancel();
          for (int col = 0; col < S.raw_width; col++)
          {
            int idx = row * S.raw_width + col;
            int val = int(src[idx]) - bl +
                      imgdata.rawdata.ph1_cblack[row][col >= imgdata.rawdata.color.phase_one_data.split_col] +
                      imgdata.rawdata.ph1_rblack[col][row >= imgdata.rawdata.color.phase_one_data.split_row];
            dest[idx] = val > 0 ? val : 0;
          }
        }
      }
    }
    else // black set by user interaction
    {
      // Black level in cblack!
      for (int row = 0; row < S.raw_height; row++)
      {
        checkCancel();
        unsigned short cblk[16];
        for (int cc = 0; cc < 16; cc++)
          cblk[cc] = C.cblack[fcol(row, cc)];
        for (int col = 0; col < S.raw_width; col++)
        {
          int idx = row * S.raw_width + col;
          ushort val = src[idx];
          ushort bl = cblk[col & 0xf];
          dest[idx] = val > bl ? val - bl : 0;
        }
      }
    }
    return 0;
  }
  catch (LibRaw_exceptions err)
  {
    return LIBRAW_CANCELLED_BY_CALLBACK;
  }
}

void LibRaw::copy_fuji_uncropped(unsigned short cblack[4], unsigned short *dmaxp)
{
  int row;
#if defined(LIBRAW_USE_OPENMP)
#pragma omp parallel for default(shared)
#endif
  for (row = 0; row < S.raw_height - S.top_margin * 2; row++)
  {
    int col;
    unsigned short ldmax = 0;
    for (col = 0; col < IO.fuji_width << !libraw_internal_data.unpacker_data.fuji_layout; col++)
    {
      unsigned r, c;
      if (libraw_internal_data.unpacker_data.fuji_layout)
      {
        r = IO.fuji_width - 1 - col + (row >> 1);
        c = col + ((row + 1) >> 1);
      }
      else
      {
        r = IO.fuji_width - 1 + row - (col >> 1);
        c = row + ((col + 1) >> 1);
      }
      if (r < S.height && c < S.width)
      {
        unsigned short val = imgdata.rawdata.raw_image[(row + S.top_margin) * S.raw_pitch / 2 + (col + S.left_margin)];
        int cc = FC(r, c);
        if (val > cblack[cc])
        {
          val -= cblack[cc];
          if (val > ldmax)
            ldmax = val;
        }
        else
          val = 0;
        imgdata.image[((r) >> IO.shrink) * S.iwidth + ((c) >> IO.shrink)][cc] = val;
      }
    }
#if defined(LIBRAW_USE_OPENMP)
#pragma omp critical(dataupdate)
#endif
    {
      if (*dmaxp < ldmax)
        *dmaxp = ldmax;
    }
  }
}

void LibRaw::copy_bayer(unsigned short cblack[4], unsigned short *dmaxp)
{
  // Both cropped and uncropped
  int row;

#if defined(LIBRAW_USE_OPENMP)
#pragma omp parallel for default(shared)
#endif
  for (row = 0; row < S.height; row++)
  {
    int col;
    unsigned short ldmax = 0;
    for (col = 0; col < S.width; col++)
    {
      unsigned short val = imgdata.rawdata.raw_image[(row + S.top_margin) * S.raw_pitch / 2 + (col + S.left_margin)];
      int cc = fcol(row, col);
      if (val > cblack[cc])
      {
        val -= cblack[cc];
        if (val > ldmax)
          ldmax = val;
      }
      else
        val = 0;
      imgdata.image[((row) >> IO.shrink) * S.iwidth + ((col) >> IO.shrink)][cc] = val;
    }
#if defined(LIBRAW_USE_OPENMP)
#pragma omp critical(dataupdate)
#endif
    {
      if (*dmaxp < ldmax)
        *dmaxp = ldmax;
    }
  }
}

int LibRaw::raw2image_ex(int do_subtract_black)
{

  CHECK_ORDER_LOW(LIBRAW_PROGRESS_LOAD_RAW);

  try
  {
    raw2image_start();

    // Compressed P1 files with bl data!
    if (is_phaseone_compressed())
    {
      phase_one_allocate_tempbuffer();
      int rc = phase_one_subtract_black((ushort *)imgdata.rawdata.raw_alloc, imgdata.rawdata.raw_image);
      if (rc == 0)
        rc = phase_one_correct();
      if (rc != 0)
      {
        phase_one_free_tempbuffer();
        return rc;
      }
    }

    // process cropping
    int do_crop = 0;
    unsigned save_width = S.width;
    if (~O.cropbox[2] && ~O.cropbox[3]
#ifdef LIBRAW_DEMOSAIC_PACK_GPL2
        && load_raw != &LibRaw::foveon_sd_load_raw
#endif
        ) // Foveon SD to be cropped later
    {
      int crop[4], c, filt;
      for (int c = 0; c < 4; c++)
      {
        crop[c] = O.cropbox[c];
        if (crop[c] < 0)
          crop[c] = 0;
      }

      if (IO.fuji_width && imgdata.idata.filters >= 1000)
      {
        crop[0] = (crop[0] / 4) * 4;
        crop[1] = (crop[1] / 4) * 4;
        if (!libraw_internal_data.unpacker_data.fuji_layout)
        {
          crop[2] *= sqrt(2.0);
          crop[3] /= sqrt(2.0);
        }
        crop[2] = (crop[2] / 4 + 1) * 4;
        crop[3] = (crop[3] / 4 + 1) * 4;
      }
      else if (imgdata.idata.filters == 1)
      {
        crop[0] = (crop[0] / 16) * 16;
        crop[1] = (crop[1] / 16) * 16;
      }
      else if (imgdata.idata.filters == LIBRAW_XTRANS)
      {
        crop[0] = (crop[0] / 6) * 6;
        crop[1] = (crop[1] / 6) * 6;
      }
      do_crop = 1;

      crop[2] = MIN(crop[2], (signed)S.width - crop[0]);
      crop[3] = MIN(crop[3], (signed)S.height - crop[1]);
      if (crop[2] <= 0 || crop[3] <= 0)
        throw LIBRAW_EXCEPTION_BAD_CROP;

      // adjust sizes!
      S.left_margin += crop[0];
      S.top_margin += crop[1];
      S.width = crop[2];
      S.height = crop[3];

      S.iheight = (S.height + IO.shrink) >> IO.shrink;
      S.iwidth = (S.width + IO.shrink) >> IO.shrink;
      if (!IO.fuji_width && imgdata.idata.filters && imgdata.idata.filters >= 1000)
      {
        for (filt = c = 0; c < 16; c++)
          filt |= FC((c >> 1) + (crop[1]), (c & 1) + (crop[0])) << c * 2;
        imgdata.idata.filters = filt;
      }
    }

    int alloc_width = S.iwidth;
    int alloc_height = S.iheight;

    if (IO.fuji_width && do_crop)
    {
      int IO_fw = S.width >> !libraw_internal_data.unpacker_data.fuji_layout;
      int t_alloc_width = (S.height >> libraw_internal_data.unpacker_data.fuji_layout) + IO_fw;
      int t_alloc_height = t_alloc_width - 1;
      alloc_height = (t_alloc_height + IO.shrink) >> IO.shrink;
      alloc_width = (t_alloc_width + IO.shrink) >> IO.shrink;
    }
    int alloc_sz = alloc_width * alloc_height;

    if (imgdata.image)
    {
      imgdata.image = (ushort(*)[4])realloc(imgdata.image, alloc_sz * sizeof(*imgdata.image));
      memset(imgdata.image, 0, alloc_sz * sizeof(*imgdata.image));
    }
    else
      imgdata.image = (ushort(*)[4])calloc(alloc_sz, sizeof(*imgdata.image));
    merror(imgdata.image, "raw2image_ex()");

    libraw_decoder_info_t decoder_info;
    get_decoder_info(&decoder_info);

    // Adjust black levels
    unsigned short cblack[4] = {0, 0, 0, 0};
    unsigned short dmax = 0;
    if (do_subtract_black)
    {
      adjust_bl();
      for (int i = 0; i < 4; i++)
        cblack[i] = (unsigned short)C.cblack[i];
    }

    // Move saved bitmap to imgdata.image
    if (imgdata.idata.filters || P1.colors == 1)
    {
      if (IO.fuji_width)
      {
        if (do_crop)
        {
          IO.fuji_width = S.width >> !libraw_internal_data.unpacker_data.fuji_layout;
          int IO_fwidth = (S.height >> libraw_internal_data.unpacker_data.fuji_layout) + IO.fuji_width;
          int IO_fheight = IO_fwidth - 1;

          int row, col;
          for (row = 0; row < S.height; row++)
          {
            for (col = 0; col < S.width; col++)
            {
              int r, c;
              if (libraw_internal_data.unpacker_data.fuji_layout)
              {
                r = IO.fuji_width - 1 - col + (row >> 1);
                c = col + ((row + 1) >> 1);
              }
              else
              {
                r = IO.fuji_width - 1 + row - (col >> 1);
                c = row + ((col + 1) >> 1);
              }

              unsigned short val =
                  imgdata.rawdata.raw_image[(row + S.top_margin) * S.raw_pitch / 2 + (col + S.left_margin)];
              int cc = FCF(row, col);
              if (val > cblack[cc])
              {
                val -= cblack[cc];
                if (dmax < val)
                  dmax = val;
              }
              else
                val = 0;
              imgdata.image[((r) >> IO.shrink) * alloc_width + ((c) >> IO.shrink)][cc] = val;
            }
          }
          S.height = IO_fheight;
          S.width = IO_fwidth;
          S.iheight = (S.height + IO.shrink) >> IO.shrink;
          S.iwidth = (S.width + IO.shrink) >> IO.shrink;
          S.raw_height -= 2 * S.top_margin;
        }
        else
        {
          copy_fuji_uncropped(cblack, &dmax);
        }
      } // end Fuji
      else
      {
        copy_bayer(cblack, &dmax);
      }
    }
    else // if(decoder_info.decoder_flags & LIBRAW_DECODER_LEGACY)
    {
      if (imgdata.rawdata.color4_image)
      {
        if (S.raw_pitch != S.width * 8)
        {
          for (int row = 0; row < S.height; row++)
            memmove(&imgdata.image[row * S.width],
                    &imgdata.rawdata.color4_image[(row + S.top_margin) * S.raw_pitch / 8 + S.left_margin],
                    S.width * sizeof(*imgdata.image));
        }
        else
        {
          // legacy is always 4channel and not shrinked!
          memmove(imgdata.image, imgdata.rawdata.color4_image, S.width * S.height * sizeof(*imgdata.image));
        }
      }
      else if (imgdata.rawdata.color3_image)
      {
        unsigned char *c3image = (unsigned char *)imgdata.rawdata.color3_image;
        for (int row = 0; row < S.height; row++)
        {
          ushort(*srcrow)[3] = (ushort(*)[3]) & c3image[(row + S.top_margin) * S.raw_pitch];
          ushort(*dstrow)[4] = (ushort(*)[4]) & imgdata.image[row * S.width];
          for (int col = 0; col < S.width; col++)
          {
            for (int c = 0; c < 3; c++)
              dstrow[col][c] = srcrow[S.left_margin + col][c];
            dstrow[col][3] = 0;
          }
        }
      }
      else
      {
        // legacy decoder, but no data?
        throw LIBRAW_EXCEPTION_DECODE_RAW;
      }
    }

    // Free PhaseOne separate copy allocated at function start
    if (is_phaseone_compressed())
    {
      phase_one_free_tempbuffer();
    }
    if (load_raw == &CLASS canon_600_load_raw && S.width < S.raw_width)
    {
      canon_600_correct();
    }

    if (do_subtract_black)
    {
      C.data_maximum = (int)dmax;
      C.maximum -= C.black;
      //        ZERO(C.cblack);
      C.cblack[0] = C.cblack[1] = C.cblack[2] = C.cblack[3] = 0;
      C.black = 0;
    }

    // hack - clear later flags!
    imgdata.progress_flags = LIBRAW_PROGRESS_START | LIBRAW_PROGRESS_OPEN | LIBRAW_PROGRESS_RAW2_IMAGE |
                             LIBRAW_PROGRESS_IDENTIFY | LIBRAW_PROGRESS_SIZE_ADJUST | LIBRAW_PROGRESS_LOAD_RAW;
    return 0;
  }
  catch (LibRaw_exceptions err)
  {
    EXCEPTION_HANDLER(err);
  }
}

#if 1

libraw_processed_image_t *LibRaw::dcraw_make_mem_thumb(int *errcode)
{
  if (!T.thumb)
  {
    if (!ID.toffset && !(imgdata.thumbnail.tlength > 0 && load_raw == &LibRaw::broadcom_load_raw) // RPi
        )
    {
      if (errcode)
        *errcode = LIBRAW_NO_THUMBNAIL;
    }
    else
    {
      if (errcode)
        *errcode = LIBRAW_OUT_OF_ORDER_CALL;
    }
    return NULL;
  }

  if (T.tformat == LIBRAW_THUMBNAIL_BITMAP)
  {
    libraw_processed_image_t *ret = (libraw_processed_image_t *)::malloc(sizeof(libraw_processed_image_t) + T.tlength);

    if (!ret)
    {
      if (errcode)
        *errcode = ENOMEM;
      return NULL;
    }

    memset(ret, 0, sizeof(libraw_processed_image_t));
    ret->type = LIBRAW_IMAGE_BITMAP;
    ret->height = T.theight;
    ret->width = T.twidth;
    ret->colors = 3;
    ret->bits = 8;
    ret->data_size = T.tlength;
    memmove(ret->data, T.thumb, T.tlength);
    if (errcode)
      *errcode = 0;
    return ret;
  }
  else if (T.tformat == LIBRAW_THUMBNAIL_JPEG)
  {
    ushort exif[5];
    int mk_exif = 0;
    if (strcmp(T.thumb + 6, "Exif"))
      mk_exif = 1;

    int dsize = T.tlength + mk_exif * (sizeof(exif) + sizeof(tiff_hdr));

    libraw_processed_image_t *ret = (libraw_processed_image_t *)::malloc(sizeof(libraw_processed_image_t) + dsize);

    if (!ret)
    {
      if (errcode)
        *errcode = ENOMEM;
      return NULL;
    }

    memset(ret, 0, sizeof(libraw_processed_image_t));

    ret->type = LIBRAW_IMAGE_JPEG;
    ret->data_size = dsize;

    ret->data[0] = 0xff;
    ret->data[1] = 0xd8;
    if (mk_exif)
    {
      struct tiff_hdr th;
      memcpy(exif, "\xff\xe1  Exif\0\0", 10);
      exif[1] = htons(8 + sizeof th);
      memmove(ret->data + 2, exif, sizeof(exif));
      tiff_head(&th, 0);
      memmove(ret->data + (2 + sizeof(exif)), &th, sizeof(th));
      memmove(ret->data + (2 + sizeof(exif) + sizeof(th)), T.thumb + 2, T.tlength - 2);
    }
    else
    {
      memmove(ret->data + 2, T.thumb + 2, T.tlength - 2);
    }
    if (errcode)
      *errcode = 0;
    return ret;
  }
  else
  {
    if (errcode)
      *errcode = LIBRAW_UNSUPPORTED_THUMBNAIL;
    return NULL;
  }
}

// jlb
// macros for copying pixels to either BGR or RGB formats
#define FORBGR for (c = P1.colors - 1; c >= 0; c--)
#define FORRGB for (c = 0; c < P1.colors; c++)

void LibRaw::get_mem_image_format(int *width, int *height, int *colors, int *bps) const

{
  if (S.flip & 4)
  {
    *width = S.height;
    *height = S.width;
  }
  else
  {
    *width = S.width;
    *height = S.height;
  }
  *colors = P1.colors;
  *bps = O.output_bps;
}

int LibRaw::copy_mem_image(void *scan0, int stride, int bgr)

{
  // the image memory pointed to by scan0 is assumed to be in the format returned by get_mem_image_format
  if ((imgdata.progress_flags & LIBRAW_PROGRESS_THUMB_MASK) < LIBRAW_PROGRESS_PRE_INTERPOLATE)
    return LIBRAW_OUT_OF_ORDER_CALL;

  if (libraw_internal_data.output_data.histogram)
  {
    int perc, val, total, t_white = 0x2000, c;
    perc = S.width * S.height * O.auto_bright_thr;
    if (IO.fuji_width)
      perc /= 2;
    if (!((O.highlight & ~2) || O.no_auto_bright))
      for (t_white = c = 0; c < P1.colors; c++)
      {
        for (val = 0x2000, total = 0; --val > 32;)
          if ((total += libraw_internal_data.output_data.histogram[c][val]) > perc)
            break;
        if (t_white < val)
          t_white = val;
      }
    gamma_curve(O.gamm[0], O.gamm[1], 2, (t_white << 3) / O.bright);
  }

  int s_iheight = S.iheight;
  int s_iwidth = S.iwidth;
  int s_width = S.width;
  int s_hwight = S.height;

  S.iheight = S.height;
  S.iwidth = S.width;

  if (S.flip & 4)
    SWAP(S.height, S.width);
  uchar *ppm;
  ushort *ppm2;
  int c, row, col, soff, rstep, cstep;

  soff = flip_index(0, 0);
  cstep = flip_index(0, 1) - soff;
  rstep = flip_index(1, 0) - flip_index(0, S.width);

  for (row = 0; row < S.height; row++, soff += rstep)
  {
    uchar *bufp = ((uchar *)scan0) + row * stride;
    ppm2 = (ushort *)(ppm = bufp);
    // keep trivial decisions in the outer loop for speed
    if (bgr)
    {
      if (O.output_bps == 8)
      {
        for (col = 0; col < S.width; col++, soff += cstep)
          FORBGR *ppm++ = imgdata.color.curve[imgdata.image[soff][c]] >> 8;
      }
      else
      {
        for (col = 0; col < S.width; col++, soff += cstep)
          FORBGR *ppm2++ = imgdata.color.curve[imgdata.image[soff][c]];
      }
    }
    else
    {
      if (O.output_bps == 8)
      {
        for (col = 0; col < S.width; col++, soff += cstep)
          FORRGB *ppm++ = imgdata.color.curve[imgdata.image[soff][c]] >> 8;
      }
      else
      {
        for (col = 0; col < S.width; col++, soff += cstep)
          FORRGB *ppm2++ = imgdata.color.curve[imgdata.image[soff][c]];
      }
    }

    //            bufp += stride;           // go to the next line
  }

  S.iheight = s_iheight;
  S.iwidth = s_iwidth;
  S.width = s_width;
  S.height = s_hwight;

  return 0;
}
#undef FORBGR
#undef FORRGB

libraw_processed_image_t *LibRaw::dcraw_make_mem_image(int *errcode)

{
  int width, height, colors, bps;
  get_mem_image_format(&width, &height, &colors, &bps);
  int stride = width * (bps / 8) * colors;
  unsigned ds = height * stride;
  libraw_processed_image_t *ret = (libraw_processed_image_t *)::malloc(sizeof(libraw_processed_image_t) + ds);
  if (!ret)
  {
    if (errcode)
      *errcode = ENOMEM;
    return NULL;
  }
  memset(ret, 0, sizeof(libraw_processed_image_t));

  // metadata init
  ret->type = LIBRAW_IMAGE_BITMAP;
  ret->height = height;
  ret->width = width;
  ret->colors = colors;
  ret->bits = bps;
  ret->data_size = ds;
  copy_mem_image(ret->data, stride, 0);

  return ret;
}

#undef FORC
#undef FORCC
#undef SWAP
#endif

int LibRaw::dcraw_ppm_tiff_writer(const char *filename)
{
  CHECK_ORDER_LOW(LIBRAW_PROGRESS_LOAD_RAW);

  if (!imgdata.image)
    return LIBRAW_OUT_OF_ORDER_CALL;

  if (!filename)
    return ENOENT;
  FILE *f = fopen(filename, "wb");

  if (!f)
    return errno;

  try
  {
    if (!libraw_internal_data.output_data.histogram)
    {
      libraw_internal_data.output_data.histogram =
          (int(*)[LIBRAW_HISTOGRAM_SIZE])malloc(sizeof(*libraw_internal_data.output_data.histogram) * 4);
      merror(libraw_internal_data.output_data.histogram, "LibRaw::dcraw_ppm_tiff_writer()");
    }
    libraw_internal_data.internal_data.output = f;
    write_ppm_tiff();
    SET_PROC_FLAG(LIBRAW_PROGRESS_FLIP);
    libraw_internal_data.internal_data.output = NULL;
    fclose(f);
    return 0;
  }
  catch (LibRaw_exceptions err)
  {
    fclose(f);
    EXCEPTION_HANDLER(err);
  }
}

#define THUMB_READ_BEYOND 16384

void LibRaw::kodak_thumb_loader()
{
  INT64 est_datasize = T.theight * T.twidth / 3; // is 0.3 bytes per pixel good estimate?
  if (ID.toffset < 0)
    throw LIBRAW_EXCEPTION_IO_CORRUPT;

  if (ID.toffset + est_datasize > ID.input->size() + THUMB_READ_BEYOND)
    throw LIBRAW_EXCEPTION_IO_EOF;

  // some kodak cameras
  ushort s_height = S.height, s_width = S.width, s_iwidth = S.iwidth, s_iheight = S.iheight;
  ushort s_flags = libraw_internal_data.unpacker_data.load_flags;
  libraw_internal_data.unpacker_data.load_flags = 12;
  int s_colors = P1.colors;
  unsigned s_filters = P1.filters;
  ushort(*s_image)[4] = imgdata.image;

  S.height = T.theight;
  S.width = T.twidth;
  P1.filters = 0;

  if (thumb_load_raw == &CLASS kodak_ycbcr_load_raw)
  {
    S.height += S.height & 1;
    S.width += S.width & 1;
  }

  imgdata.image = (ushort(*)[4])calloc(S.iheight * S.iwidth, sizeof(*imgdata.image));
  merror(imgdata.image, "LibRaw::kodak_thumb_loader()");

  ID.input->seek(ID.toffset, SEEK_SET);
  // read kodak thumbnail into T.image[]
  try
  {
    (this->*thumb_load_raw)();
  }
  catch (...)
  {
    free(imgdata.image);
    imgdata.image = s_image;

    T.twidth = 0;
    S.width = s_width;

    S.iwidth = s_iwidth;
    S.iheight = s_iheight;

    T.theight = 0;
    S.height = s_height;

    T.tcolors = 0;
    P1.colors = s_colors;

    P1.filters = s_filters;
    T.tlength = 0;
    libraw_internal_data.unpacker_data.load_flags = s_flags;
    return;
  }

// copy-n-paste from image pipe
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define LIM(x, min, max) MAX(min, MIN(x, max))
#ifndef CLIP
#define CLIP(x) LIM(x, 0, 65535)
#endif
#define SWAP(a, b)                                                                                                     \
  {                                                                                                                    \
    a ^= b;                                                                                                            \
    a ^= (b ^= a);                                                                                                     \
  }

  // from scale_colors
  {
    double dmax;
    float scale_mul[4];
    int c, val;
    for (dmax = DBL_MAX, c = 0; c < 3; c++)
      if (dmax > C.pre_mul[c])
        dmax = C.pre_mul[c];

    for (c = 0; c < 3; c++)
      scale_mul[c] = (C.pre_mul[c] / dmax) * 65535.0 / C.maximum;
    scale_mul[3] = scale_mul[1];

    size_t size = S.height * S.width;
    for (unsigned i = 0; i < size * 4; i++)
    {
      val = imgdata.image[0][i];
      if (!val)
        continue;
      val *= scale_mul[i & 3];
      imgdata.image[0][i] = CLIP(val);
    }
  }

  // from convert_to_rgb
  ushort *img;
  int row, col;

  int(*t_hist)[LIBRAW_HISTOGRAM_SIZE] = (int(*)[LIBRAW_HISTOGRAM_SIZE])calloc(sizeof(*t_hist), 4);
  merror(t_hist, "LibRaw::kodak_thumb_loader()");

  float out[3],
      out_cam[3][4] = {{2.81761312, -1.98369181, 0.166078627, 0},
                       {-0.111855984, 1.73688626, -0.625030339, 0},
                       {-0.0379119813, -0.891268849, 1.92918086, 0}};

  for (img = imgdata.image[0], row = 0; row < S.height; row++)
    for (col = 0; col < S.width; col++, img += 4)
    {
      out[0] = out[1] = out[2] = 0;
      int c;
      for (c = 0; c < 3; c++)
      {
        out[0] += out_cam[0][c] * img[c];
        out[1] += out_cam[1][c] * img[c];
        out[2] += out_cam[2][c] * img[c];
      }
      for (c = 0; c < 3; c++)
        img[c] = CLIP((int)out[c]);
      for (c = 0; c < P1.colors; c++)
        t_hist[c][img[c] >> 3]++;
    }

  // from gamma_lut
  int(*save_hist)[LIBRAW_HISTOGRAM_SIZE] = libraw_internal_data.output_data.histogram;
  libraw_internal_data.output_data.histogram = t_hist;

  // make curve output curve!
  ushort(*t_curve) = (ushort *)calloc(sizeof(C.curve), 1);
  merror(t_curve, "LibRaw::kodak_thumb_loader()");
  memmove(t_curve, C.curve, sizeof(C.curve));
  memset(C.curve, 0, sizeof(C.curve));
  {
    int perc, val, total, t_white = 0x2000, c;

    perc = S.width * S.height * 0.01; /* 99th percentile white level */
    if (IO.fuji_width)
      perc /= 2;
    if (!((O.highlight & ~2) || O.no_auto_bright))
      for (t_white = c = 0; c < P1.colors; c++)
      {
        for (val = 0x2000, total = 0; --val > 32;)
          if ((total += libraw_internal_data.output_data.histogram[c][val]) > perc)
            break;
        if (t_white < val)
          t_white = val;
      }
    gamma_curve(O.gamm[0], O.gamm[1], 2, (t_white << 3) / O.bright);
  }

  libraw_internal_data.output_data.histogram = save_hist;
  free(t_hist);

  // from write_ppm_tiff - copy pixels into bitmap

  int s_flip = imgdata.sizes.flip;
  if (imgdata.params.raw_processing_options & LIBRAW_PROCESSING_NO_ROTATE_FOR_KODAK_THUMBNAILS)
    imgdata.sizes.flip = 0;

  S.iheight = S.height;
  S.iwidth = S.width;
  if (S.flip & 4)
    SWAP(S.height, S.width);

  if (T.thumb)
    free(T.thumb);
  T.thumb = (char *)calloc(S.width * S.height, P1.colors);
  merror(T.thumb, "LibRaw::kodak_thumb_loader()");
  T.tlength = S.width * S.height * P1.colors;

  // from write_tiff_ppm
  {
    int soff = flip_index(0, 0);
    int cstep = flip_index(0, 1) - soff;
    int rstep = flip_index(1, 0) - flip_index(0, S.width);

    for (int row = 0; row < S.height; row++, soff += rstep)
    {
      char *ppm = T.thumb + row * S.width * P1.colors;
      for (int col = 0; col < S.width; col++, soff += cstep)
        for (int c = 0; c < P1.colors; c++)
          ppm[col * P1.colors + c] = imgdata.color.curve[imgdata.image[soff][c]] >> 8;
    }
  }

  memmove(C.curve, t_curve, sizeof(C.curve));
  free(t_curve);

  // restore variables
  free(imgdata.image);
  imgdata.image = s_image;

  if (imgdata.params.raw_processing_options & LIBRAW_PROCESSING_NO_ROTATE_FOR_KODAK_THUMBNAILS)
    imgdata.sizes.flip = s_flip;

  T.twidth = S.width;
  S.width = s_width;

  S.iwidth = s_iwidth;
  S.iheight = s_iheight;

  T.theight = S.height;
  S.height = s_height;

  T.tcolors = P1.colors;
  P1.colors = s_colors;

  P1.filters = s_filters;
  libraw_internal_data.unpacker_data.load_flags = s_flags;
}
#undef MIN
#undef MAX
#undef LIM
#undef CLIP
#undef SWAP

// ������� thumbnail �� �����, ������ thumb_format � ������������ � ��������

int LibRaw::thumbOK(INT64 maxsz)
{
  if (!ID.input)
    return 0;
  if (!ID.toffset && !(imgdata.thumbnail.tlength > 0 && load_raw == &LibRaw::broadcom_load_raw) // RPi
      )
    return 0;
  INT64 fsize = ID.input->size();
  if (fsize > 0x7fffffffU)
    return 0; // No thumb for raw > 2Gb
  int tsize = 0;
  int tcol = (T.tcolors > 0 && T.tcolors < 4) ? T.tcolors : 3;
  if (write_thumb == &LibRaw::jpeg_thumb)
    tsize = T.tlength;
  else if (write_thumb == &LibRaw::ppm_thumb)
    tsize = tcol * T.twidth * T.theight;
  else if (write_thumb == &LibRaw::ppm16_thumb)
    tsize = tcol * T.twidth * T.theight * 2;
  else if (write_thumb == &LibRaw::x3f_thumb_loader)
  {
    tsize = x3f_thumb_size();
  }
  else // Kodak => no check
    tsize = 1;
  if (tsize < 0)
    return 0;
  if (maxsz > 0 && tsize > maxsz)
    return 0;
  return (tsize + ID.toffset <= fsize) ? 1 : 0;
}

int LibRaw::unpack_thumb(void)
{
  CHECK_ORDER_LOW(LIBRAW_PROGRESS_IDENTIFY);
  CHECK_ORDER_BIT(LIBRAW_PROGRESS_THUMB_LOAD);

  try
  {
    if (!libraw_internal_data.internal_data.input)
      return LIBRAW_INPUT_CLOSED;

    if (!ID.toffset && !(imgdata.thumbnail.tlength > 0 && load_raw == &LibRaw::broadcom_load_raw) // RPi
        )
    {
      return LIBRAW_NO_THUMBNAIL;
    }
    else if (thumb_load_raw)
    {
      kodak_thumb_loader();
      T.tformat = LIBRAW_THUMBNAIL_BITMAP;
      SET_PROC_FLAG(LIBRAW_PROGRESS_THUMB_LOAD);
      return 0;
    }
    else
    {
      if (write_thumb == &LibRaw::x3f_thumb_loader)
      {
        INT64 tsize = x3f_thumb_size();
        if (tsize < 2048 || INT64(ID.toffset) + tsize < 1)
          throw LIBRAW_EXCEPTION_IO_CORRUPT;

        if (INT64(ID.toffset) + tsize > ID.input->size() + THUMB_READ_BEYOND)
          throw LIBRAW_EXCEPTION_IO_EOF;
      }
      else
      {
        if (INT64(ID.toffset) + INT64(T.tlength) < 1)
          throw LIBRAW_EXCEPTION_IO_CORRUPT;

        if (INT64(ID.toffset) + INT64(T.tlength) > ID.input->size() + THUMB_READ_BEYOND)
          throw LIBRAW_EXCEPTION_IO_EOF;
      }

      ID.input->seek(ID.toffset, SEEK_SET);
      if (write_thumb == &LibRaw::jpeg_thumb)
      {
        if (T.thumb)
          free(T.thumb);
        T.thumb = (char *)malloc(T.tlength);
        merror(T.thumb, "jpeg_thumb()");
        ID.input->read(T.thumb, 1, T.tlength);
        T.thumb[0] = 0xff;
        T.thumb[1] = 0xd8;
        T.tcolors = 3;
        T.tformat = LIBRAW_THUMBNAIL_JPEG;
        SET_PROC_FLAG(LIBRAW_PROGRESS_THUMB_LOAD);
        return 0;
      }
      else if (write_thumb == &LibRaw::ppm_thumb)
      {
        T.tlength = T.twidth * T.theight * 3;
        if (T.thumb)
          free(T.thumb);

        T.thumb = (char *)malloc(T.tlength);
        merror(T.thumb, "ppm_thumb()");

        ID.input->read(T.thumb, 1, T.tlength);

        T.tformat = LIBRAW_THUMBNAIL_BITMAP;
        SET_PROC_FLAG(LIBRAW_PROGRESS_THUMB_LOAD);
        return 0;
      }
      else if (write_thumb == &LibRaw::ppm16_thumb)
      {
        T.tlength = T.twidth * T.theight * 3;
        ushort *t_thumb = (ushort *)calloc(T.tlength, 2);
        ID.input->read(t_thumb, 2, T.tlength);
        if ((libraw_internal_data.unpacker_data.order == 0x4949) == (ntohs(0x1234) == 0x1234))
          swab((char *)t_thumb, (char *)t_thumb, T.tlength * 2);

        if (T.thumb)
          free(T.thumb);
        T.thumb = (char *)malloc(T.tlength);
        merror(T.thumb, "ppm_thumb()");
        for (int i = 0; i < T.tlength; i++)
          T.thumb[i] = t_thumb[i] >> 8;
        free(t_thumb);
        T.tformat = LIBRAW_THUMBNAIL_BITMAP;
        SET_PROC_FLAG(LIBRAW_PROGRESS_THUMB_LOAD);
        return 0;
      }
      else if (write_thumb == &LibRaw::x3f_thumb_loader)
      {
        x3f_thumb_loader();
        SET_PROC_FLAG(LIBRAW_PROGRESS_THUMB_LOAD);
        return 0;
      }
#ifdef LIBRAW_DEMOSAIC_PACK_GPL2
      else if (write_thumb == &LibRaw::foveon_thumb)
      {
        foveon_thumb_loader();
        // may return with error, so format is set in
        // foveon thumb loader itself
        SET_PROC_FLAG(LIBRAW_PROGRESS_THUMB_LOAD);
        return 0;
      }
// else if -- all other write_thumb cases!
#endif
      else
      {
        return LIBRAW_UNSUPPORTED_THUMBNAIL;
      }
    }
    // last resort
    return LIBRAW_UNSUPPORTED_THUMBNAIL;
  }
  catch (LibRaw_exceptions err)
  {
    EXCEPTION_HANDLER(err);
  }
}

int LibRaw::dcraw_thumb_writer(const char *fname)
{
  //    CHECK_ORDER_LOW(LIBRAW_PROGRESS_THUMB_LOAD);

  if (!fname)
    return ENOENT;

  FILE *tfp = fopen(fname, "wb");

  if (!tfp)
    return errno;

  if (!T.thumb)
  {
    fclose(tfp);
    return LIBRAW_OUT_OF_ORDER_CALL;
  }

  try
  {
    switch (T.tformat)
    {
    case LIBRAW_THUMBNAIL_JPEG:
      jpeg_thumb_writer(tfp, T.thumb, T.tlength);
      break;
    case LIBRAW_THUMBNAIL_BITMAP:
      fprintf(tfp, "P6\n%d %d\n255\n", T.twidth, T.theight);
      fwrite(T.thumb, 1, T.tlength, tfp);
      break;
    default:
      fclose(tfp);
      return LIBRAW_UNSUPPORTED_THUMBNAIL;
    }
    fclose(tfp);
    return 0;
  }
  catch (LibRaw_exceptions err)
  {
    fclose(tfp);
    EXCEPTION_HANDLER(err);
  }
}

int LibRaw::adjust_sizes_info_only(void)
{
  CHECK_ORDER_LOW(LIBRAW_PROGRESS_IDENTIFY);

  raw2image_start();
  if (O.use_fuji_rotate)
  {
    if (IO.fuji_width)
    {
      IO.fuji_width = (IO.fuji_width - 1 + IO.shrink) >> IO.shrink;
      S.iwidth = (ushort)(IO.fuji_width / sqrt(0.5));
      S.iheight = (ushort)((S.iheight - IO.fuji_width) / sqrt(0.5));
    }
    else
    {
      if (S.pixel_aspect < 0.995)
        S.iheight = (ushort)(S.iheight / S.pixel_aspect + 0.5);
      if (S.pixel_aspect > 1.005)
        S.iwidth = (ushort)(S.iwidth * S.pixel_aspect + 0.5);
    }
  }
  SET_PROC_FLAG(LIBRAW_PROGRESS_FUJI_ROTATE);
  if (S.flip & 4)
  {
    unsigned short t = S.iheight;
    S.iheight = S.iwidth;
    S.iwidth = t;
    SET_PROC_FLAG(LIBRAW_PROGRESS_FLIP);
  }
  return 0;
}

int LibRaw::subtract_black()
{
  adjust_bl();
  return subtract_black_internal();
}

int LibRaw::subtract_black_internal()
{
  CHECK_ORDER_LOW(LIBRAW_PROGRESS_RAW2_IMAGE);

  try
  {
    if (!is_phaseone_compressed() &&
        (C.cblack[0] || C.cblack[1] || C.cblack[2] || C.cblack[3] || (C.cblack[4] && C.cblack[5])))
    {
#define BAYERC(row, col, c) imgdata.image[((row) >> IO.shrink) * S.iwidth + ((col) >> IO.shrink)][c]
      int cblk[4], i;
      for (i = 0; i < 4; i++)
        cblk[i] = C.cblack[i];

      int size = S.iheight * S.iwidth;
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define LIM(x, min, max) MAX(min, MIN(x, max))
#define CLIP(x) LIM(x, 0, 65535)
      int dmax = 0;
      if (C.cblack[4] && C.cblack[5])
      {
        for (i = 0; i < size * 4; i++)
        {
          int val = imgdata.image[0][i];
          val -= C.cblack[6 + i / 4 / S.iwidth % C.cblack[4] * C.cblack[5] + i / 4 % S.iwidth % C.cblack[5]];
          val -= cblk[i & 3];
          imgdata.image[0][i] = CLIP(val);
          if (dmax < val)
            dmax = val;
        }
      }
      else
      {
        for (i = 0; i < size * 4; i++)
        {
          int val = imgdata.image[0][i];
          val -= cblk[i & 3];
          imgdata.image[0][i] = CLIP(val);
          if (dmax < val)
            dmax = val;
        }
      }
      C.data_maximum = dmax & 0xffff;
#undef MIN
#undef MAX
#undef LIM
#undef CLIP
      C.maximum -= C.black;
      ZERO(C.cblack); // Yeah, we used cblack[6+] values too!
      C.black = 0;
#undef BAYERC
    }
    else
    {
      // Nothing to Do, maximum is already calculated, black level is 0, so no change
      // only calculate channel maximum;
      int idx;
      ushort *p = (ushort *)imgdata.image;
      int dmax = 0;
      for (idx = 0; idx < S.iheight * S.iwidth * 4; idx++)
        if (dmax < p[idx])
          dmax = p[idx];
      C.data_maximum = dmax;
    }
    return 0;
  }
  catch (LibRaw_exceptions err)
  {
    EXCEPTION_HANDLER(err);
  }
}

#define TBLN 65535

void LibRaw::exp_bef(float shift, float smooth)
{
  // params limits
  if (shift > 8)
    shift = 8;
  if (shift < 0.25)
    shift = 0.25;
  if (smooth < 0.0)
    smooth = 0.0;
  if (smooth > 1.0)
    smooth = 1.0;

  unsigned short *lut = (ushort *)malloc((TBLN + 1) * sizeof(unsigned short));

  if (shift <= 1.0)
  {
    for (int i = 0; i <= TBLN; i++)
      lut[i] = (unsigned short)((float)i * shift);
  }
  else
  {
    float x1, x2, y1, y2;

    float cstops = log(shift) / log(2.0f);
    float room = cstops * 2;
    float roomlin = powf(2.0f, room);
    x2 = (float)TBLN;
    x1 = (x2 + 1) / roomlin - 1;
    y1 = x1 * shift;
    y2 = x2 * (1 + (1 - smooth) * (shift - 1));
    float sq3x = powf(x1 * x1 * x2, 1.0f / 3.0f);
    float B = (y2 - y1 + shift * (3 * x1 - 3.0f * sq3x)) / (x2 + 2.0f * x1 - 3.0f * sq3x);
    float A = (shift - B) * 3.0f * powf(x1 * x1, 1.0f / 3.0f);
    float CC = y2 - A * powf(x2, 1.0f / 3.0f) - B * x2;
    for (int i = 0; i <= TBLN; i++)
    {
      float X = (float)i;
      float Y = A * powf(X, 1.0f / 3.0f) + B * X + CC;
      if (i < x1)
        lut[i] = (unsigned short)((float)i * shift);
      else
        lut[i] = Y < 0 ? 0 : (Y > TBLN ? TBLN : (unsigned short)(Y));
    }
  }
  for (int i = 0; i < S.height * S.width; i++)
  {
    imgdata.image[i][0] = lut[imgdata.image[i][0]];
    imgdata.image[i][1] = lut[imgdata.image[i][1]];
    imgdata.image[i][2] = lut[imgdata.image[i][2]];
    imgdata.image[i][3] = lut[imgdata.image[i][3]];
  }

  if (C.data_maximum <= TBLN)
    C.data_maximum = lut[C.data_maximum];
  if (C.maximum <= TBLN)
    C.maximum = lut[C.maximum];
  // no need to adjust the minumum, black is already subtracted
  free(lut);
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define LIM(x, min, max) MAX(min, MIN(x, max))
#define ULIM(x, y, z) ((y) < (z) ? LIM(x, y, z) : LIM(x, z, y))
#define CLIP(x) LIM(x, 0, 65535)

void LibRaw::convert_to_rgb_loop(float out_cam[3][4])
{
  int row, col, c;
  float out[3];
  ushort *img;
  memset(libraw_internal_data.output_data.histogram, 0, sizeof(int) * LIBRAW_HISTOGRAM_SIZE * 4);
  for (img = imgdata.image[0], row = 0; row < S.height; row++)
    for (col = 0; col < S.width; col++, img += 4)
    {
      if (!libraw_internal_data.internal_output_params.raw_color)
      {
        out[0] = out[1] = out[2] = 0;
        for (c = 0; c < imgdata.idata.colors; c++)
        {
          out[0] += out_cam[0][c] * img[c];
          out[1] += out_cam[1][c] * img[c];
          out[2] += out_cam[2][c] * img[c];
        }
        for (c = 0; c < 3; c++)
          img[c] = CLIP((int)out[c]);
      }
      for (c = 0; c < imgdata.idata.colors; c++)
        libraw_internal_data.output_data.histogram[c][img[c] >> 3]++;
    }
}

void LibRaw::scale_colors_loop(float scale_mul[4])
{
  unsigned size = S.iheight * S.iwidth;

  if (C.cblack[4] && C.cblack[5])
  {
    int val;
    for (unsigned i = 0; i < size * 4; i++)
    {
      if (!(val = imgdata.image[0][i]))
        continue;
      val -= C.cblack[6 + i / 4 / S.iwidth % C.cblack[4] * C.cblack[5] + i / 4 % S.iwidth % C.cblack[5]];
      val -= C.cblack[i & 3];
      val *= scale_mul[i & 3];
      imgdata.image[0][i] = CLIP(val);
    }
  }
  else if (C.cblack[0] || C.cblack[1] || C.cblack[2] || C.cblack[3])
  {
    for (unsigned i = 0; i < size * 4; i++)
    {
      int val = imgdata.image[0][i];
      if (!val)
        continue;
      val -= C.cblack[i & 3];
      val *= scale_mul[i & 3];
      imgdata.image[0][i] = CLIP(val);
    }
  }
  else // BL is zero
  {
    for (unsigned i = 0; i < size * 4; i++)
    {
      int val = imgdata.image[0][i];
      val *= scale_mul[i & 3];
      imgdata.image[0][i] = CLIP(val);
    }
  }
}

void LibRaw::adjust_bl()
{
  int clear_repeat = 0;
  if (O.user_black >= 0)
  {
    C.black = O.user_black;
    clear_repeat = 1;
  }
  for (int i = 0; i < 4; i++)
    if (O.user_cblack[i] > -1000000)
    {
      C.cblack[i] = O.user_cblack[i];
      clear_repeat = 1;
    }

  if (clear_repeat)
    C.cblack[4] = C.cblack[5] = 0;

  // Add common part to cblack[] early
  if (imgdata.idata.filters > 1000 && (C.cblack[4] + 1) / 2 == 1 && (C.cblack[5] + 1) / 2 == 1)
  {
    int clrs[4];
    int lastg = -1, gcnt = 0;
    for (int c = 0; c < 4; c++)
    {
      clrs[c] = FC(c / 2, c % 2);
      if (clrs[c] == 1)
      {
        gcnt++;
        lastg = c;
      }
    }
    if (gcnt > 1 && lastg >= 0)
      clrs[lastg] = 3;
    for (int c = 0; c < 4; c++)
      C.cblack[clrs[c]] += C.cblack[6 + c / 2 % C.cblack[4] * C.cblack[5] + c % 2 % C.cblack[5]];
    C.cblack[4] = C.cblack[5] = 0;
    // imgdata.idata.filters = sfilters;
  }
  else if (imgdata.idata.filters <= 1000 && C.cblack[4] == 1 && C.cblack[5] == 1) // Fuji RAF dng
  {
    for (int c = 0; c < 4; c++)
      C.cblack[c] += C.cblack[6];
    C.cblack[4] = C.cblack[5] = 0;
  }
  // remove common part from C.cblack[]
  int i = C.cblack[3];
  int c;
  for (c = 0; c < 3; c++)
    if (i > C.cblack[c])
      i = C.cblack[c];

  for (c = 0; c < 4; c++)
    C.cblack[c] -= i; // remove common part
  C.black += i;

  // Now calculate common part for cblack[6+] part and move it to C.black

  if (C.cblack[4] && C.cblack[5])
  {
    i = C.cblack[6];
    for (c = 1; c < C.cblack[4] * C.cblack[5]; c++)
      if (i > C.cblack[6 + c])
        i = C.cblack[6 + c];
    // Remove i from cblack[6+]
    int nonz = 0;
    for (c = 0; c < C.cblack[4] * C.cblack[5]; c++)
    {
      C.cblack[6 + c] -= i;
      if (C.cblack[6 + c])
        nonz++;
    }
    C.black += i;
    if (!nonz)
      C.cblack[4] = C.cblack[5] = 0;
  }
  for (c = 0; c < 4; c++)
    C.cblack[c] += C.black;
}

int LibRaw::dcraw_process(void)
{
  int quality, i;

  int iterations = -1, dcb_enhance = 1, noiserd = 0;
  int eeci_refine_fl = 0, es_med_passes_fl = 0;
  float cared = 0, cablue = 0;
  float linenoise = 0;
  float lclean = 0, cclean = 0;
  float thresh = 0;
  float preser = 0;
  float expos = 1.0;

  CHECK_ORDER_LOW(LIBRAW_PROGRESS_LOAD_RAW);
  //    CHECK_ORDER_HIGH(LIBRAW_PROGRESS_PRE_INTERPOLATE);

  try
  {

    int no_crop = 1;

    if (~O.cropbox[2] && ~O.cropbox[3])
      no_crop = 0;

    libraw_decoder_info_t di;
    get_decoder_info(&di);

    bool is_bayer = (imgdata.idata.filters || P1.colors == 1);
    int subtract_inline = !O.bad_pixels && !O.dark_frame && !O.wf_debanding && is_bayer && !IO.zero_is_bad;

    raw2image_ex(subtract_inline); // allocate imgdata.image and copy data!

    // Adjust sizes

    int save_4color = O.four_color_rgb;

    if (IO.zero_is_bad)
    {
      remove_zeroes();
      SET_PROC_FLAG(LIBRAW_PROGRESS_REMOVE_ZEROES);
    }

    if (O.bad_pixels && no_crop)
    {
      bad_pixels(O.bad_pixels);
      SET_PROC_FLAG(LIBRAW_PROGRESS_BAD_PIXELS);
    }

    if (O.dark_frame && no_crop)
    {
      subtract(O.dark_frame);
      SET_PROC_FLAG(LIBRAW_PROGRESS_DARK_FRAME);
    }

    if (O.wf_debanding)
    {
      wf_remove_banding();
    }

    quality = 2 + !IO.fuji_width;

    if (O.user_qual >= 0)
      quality = O.user_qual;

    if (!subtract_inline || !C.data_maximum)
    {
      adjust_bl();
      subtract_black_internal();
    }

    if (!(di.decoder_flags & LIBRAW_DECODER_FIXEDMAXC))
      adjust_maximum();

    if (O.user_sat > 0)
      C.maximum = O.user_sat;

    if (P1.is_foveon)
    {
      if (load_raw == &LibRaw::x3f_load_raw)
      {
        // Filter out zeroes
        for (int i = 0; i < S.height * S.width * 4; i++)
          if ((short)imgdata.image[0][i] < 0)
            imgdata.image[0][i] = 0;
      }
#ifdef LIBRAW_DEMOSAIC_PACK_GPL2
      else if (load_raw == &LibRaw::foveon_dp_load_raw)
      {
        for (int i = 0; i < S.height * S.width * 4; i++)
          if ((short)imgdata.image[0][i] < 0)
            imgdata.image[0][i] = 0;
      }
      else
      {
        foveon_interpolate();
      }
#endif
      SET_PROC_FLAG(LIBRAW_PROGRESS_FOVEON_INTERPOLATE);
    }

    if (O.green_matching && !O.half_size)
    {
      green_matching();
    }

    if (
#ifdef LIBRAW_DEMOSAIC_PACK_GPL2
        (!P1.is_foveon || (O.raw_processing_options & LIBRAW_PROCESSING_FORCE_FOVEON_X3F)) &&
#endif
        !O.no_auto_scale)
    {
      scale_colors();
      SET_PROC_FLAG(LIBRAW_PROGRESS_SCALE_COLORS);
    }

    pre_interpolate();

    SET_PROC_FLAG(LIBRAW_PROGRESS_PRE_INTERPOLATE);

    if (O.dcb_iterations >= 0)
      iterations = O.dcb_iterations;
    if (O.dcb_enhance_fl >= 0)
      dcb_enhance = O.dcb_enhance_fl;
    if (O.fbdd_noiserd >= 0)
      noiserd = O.fbdd_noiserd;
    if (O.eeci_refine >= 0)
      eeci_refine_fl = O.eeci_refine;
    if (O.es_med_passes > 0)
      es_med_passes_fl = O.es_med_passes;

    // LIBRAW_DEMOSAIC_PACK_GPL3

    if (!O.half_size && O.cfa_green > 0)
    {
      thresh = O.green_thresh;
      green_equilibrate(thresh);
    }
    if (O.exp_correc > 0)
    {
      expos = O.exp_shift;
      preser = O.exp_preser;
      exp_bef(expos, preser);
    }
    if (O.ca_correc > 0)
    {
      cablue = O.cablue;
      cared = O.cared;
      CA_correct_RT(cablue, cared);
    }
    if (O.cfaline > 0)
    {
      linenoise = O.linenoise;
      cfa_linedn(linenoise);
    }
    if (O.cfa_clean > 0)
    {
      lclean = O.lclean;
      cclean = O.cclean;
      cfa_impulse_gauss(lclean, cclean);
    }

    if (P1.filters && !O.no_interpolation)
    {
      if (noiserd > 0 && P1.colors == 3 && P1.filters)
        fbdd(noiserd);

      if (P1.filters > 1000 && interpolate_bayer)
        (this->*interpolate_bayer)();
      else if (P1.filters == 9 && interpolate_xtrans)
        (this->*interpolate_xtrans)();
      else if (quality == 0)
        lin_interpolate();
      else if (quality == 1 || P1.colors > 3)
        vng_interpolate();
      else if (quality == 2 && P1.filters > 1000)
        ppg_interpolate();
      else if (P1.filters == LIBRAW_XTRANS)
      {
        // Fuji X-Trans
        xtrans_interpolate(quality > 2 ? 3 : 1);
      }
      else if (quality == 3)
        ahd_interpolate(); // really don't need it here due to fallback op
      else if (quality == 4)
        dcb(iterations, dcb_enhance);
      //  LIBRAW_DEMOSAIC_PACK_GPL2
      else if (quality == 5)
        ahd_interpolate_mod();
      else if (quality == 6)
        afd_interpolate_pl(2, 1);
      else if (quality == 7)
        vcd_interpolate(0);
      else if (quality == 8)
        vcd_interpolate(12);
      else if (quality == 9)
        lmmse_interpolate(1);

      // LIBRAW_DEMOSAIC_PACK_GPL3
      else if (quality == 10)
        amaze_demosaic_RT();
      // LGPL2
      else if (quality == 11)
        dht_interpolate();
      else if (quality == 12)
        aahd_interpolate();
      // fallback to AHD
      else
      {
        ahd_interpolate();
        imgdata.process_warnings |= LIBRAW_WARN_FALLBACK_TO_AHD;
      }

      SET_PROC_FLAG(LIBRAW_PROGRESS_INTERPOLATE);
    }
    if (IO.mix_green)
    {
      for (P1.colors = 3, i = 0; i < S.height * S.width; i++)
        imgdata.image[i][1] = (imgdata.image[i][1] + imgdata.image[i][3]) >> 1;
      SET_PROC_FLAG(LIBRAW_PROGRESS_MIX_GREEN);
    }

    if (!P1.is_foveon)
    {
      if (P1.colors == 3)
      {

        if (quality == 8)
        {
          if (eeci_refine_fl == 1)
            refinement();
          if (O.med_passes > 0)
            median_filter_new();
          if (es_med_passes_fl > 0)
            es_median_filter();
        }
        else
        {
          median_filter();
        }
        SET_PROC_FLAG(LIBRAW_PROGRESS_MEDIAN_FILTER);
      }
    }

    if (O.highlight == 2)
    {
      blend_highlights();
      SET_PROC_FLAG(LIBRAW_PROGRESS_HIGHLIGHTS);
    }

    if (O.highlight > 2)
    {
      recover_highlights();
      SET_PROC_FLAG(LIBRAW_PROGRESS_HIGHLIGHTS);
    }

    if (O.use_fuji_rotate)
    {
      fuji_rotate();
      SET_PROC_FLAG(LIBRAW_PROGRESS_FUJI_ROTATE);
    }

    if (!libraw_internal_data.output_data.histogram)
    {
      libraw_internal_data.output_data.histogram =
          (int(*)[LIBRAW_HISTOGRAM_SIZE])malloc(sizeof(*libraw_internal_data.output_data.histogram) * 4);
      merror(libraw_internal_data.output_data.histogram, "LibRaw::dcraw_process()");
    }
#ifndef NO_LCMS
    if (O.camera_profile)
    {
      apply_profile(O.camera_profile, O.output_profile);
      SET_PROC_FLAG(LIBRAW_PROGRESS_APPLY_PROFILE);
    }
#endif

    convert_to_rgb();
    SET_PROC_FLAG(LIBRAW_PROGRESS_CONVERT_RGB);

    if (O.use_fuji_rotate)
    {
      stretch();
      SET_PROC_FLAG(LIBRAW_PROGRESS_STRETCH);
    }
    O.four_color_rgb = save_4color; // also, restore

    return 0;
  }
  catch (LibRaw_exceptions err)
  {
    EXCEPTION_HANDLER(err);
  }
}

// clang-format off
// Supported cameras:
static const char *static_camera_list[] = {
	"Adobe Digital Negative (DNG)",
	"AgfaPhoto DC-833m",
	"Alcatel 5035D",
	"Apple iPad Pro",
	"Apple iPhone SE",
	"Apple iPhone 6s",
	"Apple iPhone 6 plus",
	"Apple iPhone 7",
	"Apple iPhone 7 plus",
	"Apple QuickTake 100",
	"Apple QuickTake 150",
	"Apple QuickTake 200",
	"ARRIRAW format",
	"AVT F-080C",
	"AVT F-145C",
	"AVT F-201C",
	"AVT F-510C",
	"AVT F-810C",
	"Baumer TXG14",
	"BlackMagic Cinema Camera",
	"BlackMagic Micro Cinema Camera",
	"BlackMagic Pocket Cinema Camera",
	"BlackMagic Production Camera 4k",
	"BlackMagic URSA",
	"BlackMagic URSA Mini",
	"Canon PowerShot 600",
	"Canon PowerShot A5",
	"Canon PowerShot A5 Zoom",
	"Canon PowerShot A50",
	"Canon PowerShot A460 (CHDK hack)",
	"Canon PowerShot A470 (CHDK hack)",
	"Canon PowerShot A530 (CHDK hack)",
	"Canon PowerShot A550 (CHDK hack)",
	"Canon PowerShot A570 (CHDK hack)",
	"Canon PowerShot A590 (CHDK hack)",
	"Canon PowerShot A610 (CHDK hack)",
	"Canon PowerShot A620 (CHDK hack)",
	"Canon PowerShot A630 (CHDK hack)",
	"Canon PowerShot A640 (CHDK hack)",
	"Canon PowerShot A650 (CHDK hack)",
	"Canon PowerShot A710 IS (CHDK hack)",
	"Canon PowerShot A720 IS (CHDK hack)",
	"Canon PowerShot A3300 IS (CHDK hack)",
	"Canon PowerShot Pro70",
	"Canon PowerShot Pro90 IS",
	"Canon PowerShot Pro1",
	"Canon PowerShot G1",
	"Canon PowerShot G1 X",
	"Canon PowerShot G1 X Mark II",
	"Canon PowerShot G2",
	"Canon PowerShot G3",
	"Canon PowerShot G3 X",
	"Canon PowerShot G5",
	"Canon PowerShot G5 X",
	"Canon PowerShot G6",
	"Canon PowerShot G7 (CHDK hack)",
	"Canon PowerShot G7 X",
	"Canon PowerShot G7 X Mark II",
	"Canon PowerShot G9",
	"Canon PowerShot G9 X",
	"Canon PowerShot G10",
	"Canon PowerShot G11",
	"Canon PowerShot G12",
	"Canon PowerShot G15",
	"Canon PowerShot G16",
	"Canon PowerShot S2 IS (CHDK hack)",
	"Canon PowerShot S3 IS (CHDK hack)",
	"Canon PowerShot S5 IS (CHDK hack)",
	"Canon PowerShot SD300 (CHDK hack)",
	"Canon PowerShot SD750 (CHDK hack)",
	"Canon PowerShot SD950 (CHDK hack)",
	"Canon PowerShot S30",
	"Canon PowerShot S40",
	"Canon PowerShot S45",
	"Canon PowerShot S50",
	"Canon PowerShot S60",
	"Canon PowerShot S70",
	"Canon PowerShot S90",
	"Canon PowerShot S95",
	"Canon PowerShot S100",
	"Canon PowerShot S110",
	"Canon PowerShot S120",
	"Canon PowerShot SX1 IS",
	"Canon PowerShot SX50 HS",
	"Canon PowerShot SX60 HS",
	"Canon PowerShot SX100 IS (CHDK hack)",
	"Canon PowerShot SX110 IS (CHDK hack)",
	"Canon PowerShot SX120 IS (CHDK hack)",
	"Canon PowerShot SX130 IS (CHDK hack)",
	"Canon PowerShot SX220 HS (CHDK hack)",
	"Canon PowerShot SX510 HS (CHDK hack)",
	"Canon PowerShot SX10 IS (CHDK hack)",
	"Canon PowerShot SX20 IS (CHDK hack)",
	"Canon PowerShot SX30 IS (CHDK hack)",
	"Canon PowerShot IXUS 160 (CHDK hack)",
	"Canon PowerShot IXUS 900Ti (CHDK hack)",
	"Canon EOS D30",
	"Canon EOS D60",
	"Canon EOS 5D",
	"Canon EOS 5DS",
	"Canon EOS 5DS R",
	"Canon EOS 5D Mark II",
	"Canon EOS 5D Mark III",
	"Canon EOS 5D Mark IV",
	"Canon EOS 6D",
	"Canon EOS 7D",
	"Canon EOS 7D Mark II",
	"Canon EOS 10D",
	"Canon EOS 20D",
	"Canon EOS 20Da",
	"Canon EOS 30D",
	"Canon EOS 40D",
	"Canon EOS 50D",
	"Canon EOS 60D",
	"Canon EOS 60Da",
	"Canon EOS 70D",
	"Canon EOS 80D",
	"Canon EOS 300D / Digital Rebel / Kiss Digital",
	"Canon EOS 350D / Digital Rebel XT / Kiss Digital N",
	"Canon EOS 400D / Digital Rebel XTi / Kiss Digital X",
	"Canon EOS 450D / Digital Rebel XSi / Kiss Digital X2",
	"Canon EOS 500D / Digital Rebel T1i / Kiss Digital X3",
	"Canon EOS 550D / Digital Rebel T2i / Kiss Digital X4",
	"Canon EOS 600D / Digital Rebel T3i / Kiss Digital X5",
	"Canon EOS 650D / Digital Rebel T4i / Kiss Digital X6i",
	"Canon EOS 700D / Digital Rebel T5i",
	"Canon EOS 750D / Digital Rebel T6i",
	"Canon EOS 760D / Digital Rebel T6S",
	"Canon EOS 100D / Digital Rebel SL1",
	"Canon EOS 1000D / Digital Rebel XS / Kiss Digital F",
	"Canon EOS 1100D / Digital Rebel T3 / Kiss Digital X50",
	"Canon EOS 1200D",
	"Canon EOS 1300D",
	"Canon EOS C500",
	"Canon EOS D2000C",
	"Canon EOS M",
	"Canon EOS M2",
	"Canon EOS M3",
	"Canon EOS M5",
	"Canon EOS M10",
	"Canon EOS-1D",
	"Canon EOS-1DS",
	"Canon EOS-1D C",
	"Canon EOS-1D X",
	"Canon EOS-1D Mark II",
	"Canon EOS-1D Mark II N",
	"Canon EOS-1D Mark III",
	"Canon EOS-1D Mark IV",
	"Canon EOS-1Ds Mark II",
	"Canon EOS-1Ds Mark III",
	"Canon EOS-1D X Mark II",
	"Casio QV-2000UX",
	"Casio QV-3000EX",
	"Casio QV-3500EX",
	"Casio QV-4000",
	"Casio QV-5700",
	"Casio QV-R41",
	"Casio QV-R51",
	"Casio QV-R61",
	"Casio EX-F1",
	"Casio EX-FC300S",
	"Casio EX-FC400S",
	"Casio EX-FH20",
	"Casio EX-FH25",
	"Casio EX-FH100",
	"Casio EX-S20",
	"Casio EX-S100",
	"Casio EX-Z4",
	"Casio EX-Z50",
	"Casio EX-Z500",
	"Casio EX-Z55",
	"Casio EX-Z60",
	"Casio EX-Z75",
	"Casio EX-Z750",
	"Casio EX-Z8",
	"Casio EX-Z850",
	"Casio EX-Z1050",
	"Casio EX-ZR100",
	"Casio EX-Z1080",
	"Casio EX-ZR700",
	"Casio EX-ZR710",
	"Casio EX-ZR750",
	"Casio EX-ZR800",
	"Casio EX-ZR850",
	"Casio EX-ZR1000",
	"Casio EX-ZR1100",
	"Casio EX-ZR1200",
	"Casio EX-ZR1300",
	"Casio EX-ZR1500",
	"Casio EX-ZR3000",
	"Casio EX-ZR4000/5000",
	"Casio EX-100",
	"Casio EX-100F",
	"Casio EX-10",
	"Casio Exlim Pro 505",
	"Casio Exlim Pro 600",
	"Casio Exlim Pro 700",
	"Contax N Digital",
	"Creative PC-CAM 600",
	"Digital Bolex D16",
	"Digital Bolex D16M",
	"DJI 4384x3288",
	"DXO One",
	"Epson R-D1",
	"Epson R-D1s",
	"Epson R-D1x",
	"Foculus 531C",
	"FujiFilm E505",
	"FujiFilm E550",
	"FujiFilm E900",
	"FujiFilm F700",
	"FujiFilm F710",
	"FujiFilm F800",
	"FujiFilm F810",
	"FujiFilm S2Pro",
	"FujiFilm S3Pro",
	"FujiFilm S5Pro",
	"FujiFilm S20Pro",
	"FujiFilm S1",
	"FujiFilm S100FS",
	"FujiFilm S5000",
	"FujiFilm S5100/S5500",
	"FujiFilm S5200/S5600",
	"FujiFilm S6000fd",
	"FujiFilm S6500fd",
	"FujiFilm S7000",
	"FujiFilm S9000/S9500",
	"FujiFilm S9100/S9600",
	"FujiFilm S200EXR",
	"FujiFilm S205EXR",
	"FujiFilm SL1000",
	"FujiFilm HS10/HS11",
	"FujiFilm HS20EXR",
	"FujiFilm HS22EXR",
	"FujiFilm HS30EXR",
	"FujiFilm HS33EXR",
	"FujiFilm HS35EXR",
	"FujiFilm HS50EXR",
	"FujiFilm F505EXR",
	"FujiFilm F550EXR",
	"FujiFilm F600EXR",
	"FujiFilm F605EXR",
	"FujiFilm F770EXR",
	"FujiFilm F775EXR",
	"FujiFilm F800EXR",
	"FujiFilm F900EXR",
	"FujiFilm GFX 50S",
	"FujiFilm X-Pro1",
	"FujiFilm X-Pro2",
	"FujiFilm X-S1",
	"FujiFilm XQ1",
	"FujiFilm XQ2",
	"FujiFilm X100",
	"FujiFilm X100f",
	"FujiFilm X100S",
	"FujiFilm X100T",
	"FujiFilm X10",
	"FujiFilm X20",
	"FujiFilm X30",
	"FujiFilm X70",
	"FujiFilm X-A1",
	"FujiFilm X-A2",
	"FujiFilm X-E1",
	"FujiFilm X-E2",
	"FujiFilm X-E2S",
	"FujiFilm X-M1",
	"FujiFilm XF1",
	"FujiFilm X-T1",
	"FujiFilm X-T1 Graphite Silver",
	"FujiFilm X-T2",
	"FujiFilm X-T10",
	"FujiFilm X-T20",
	"FujiFilm IS-1",
	"Gione E7",
	"GITUP GIT2",
	"Google Pixel",
	"Google Pixel XL",
	"Hasselblad H5D-60",
	"Hasselblad H5D-50",
	"Hasselblad H5D-50c",
	"Hasselblad H5D-40",
	"Hasselblad H4D-60",
	"Hasselblad H4D-50",
	"Hasselblad H4D-40",
	"Hasselblad H4D-31",
	"Hasselblad H3DII-22",
	"Hasselblad H3DII-31",
	"Hasselblad H3DII-39",
	"Hasselblad H3DII-50",
	"Hasselblad H3D-22",
	"Hasselblad H3D-31",
	"Hasselblad H3D-39",
	"Hasselblad H2D-22",
	"Hasselblad H2D-39",
	"Hasselblad CFV",
	"Hasselblad CFH",
	"Hasselblad CF-22",
	"Hasselblad CF-31",
	"Hasselblad CF-39",
	"Hasselblad V96C",
	"Hasselblad Lusso",
	"Hasselblad Lunar",
	"Hasselblad True Zoom",
	"Hasselblad Stellar",
	"Hasselblad Stellar II",
	"Hasselblad HV",
	"Hasselblad X1D",
	"HTC UltraPixel",
	"HTC MyTouch 4G",
	"HTC One (A9)",
	"HTC One (M9)",
	"HTC 10",
	"Huawei P9",
	"Imacon Ixpress 96, 96C",
	"Imacon Ixpress 384, 384C (single shot only)",
	"Imacon Ixpress 132C",
	"Imacon Ixpress 528C (single shot only)",
	"ISG 2020x1520",
	"Ikonoskop A-Cam dII Panchromatic",
	"Ikonoskop A-Cam dII",
	"Kinefinity KineMINI",
	"Kinefinity KineRAW Mini",
	"Kinefinity KineRAW S35",
	"Kodak DC20",
	"Kodak DC25",
	"Kodak DC40",
	"Kodak DC50",
	"Kodak DC120",
	"Kodak DCS200",
	"Kodak DCS315C",
	"Kodak DCS330C",
	"Kodak DCS420",
	"Kodak DCS460",
	"Kodak DCS460A",
	"Kodak DCS460D",
	"Kodak DCS520C",
	"Kodak DCS560C",
	"Kodak DCS620C",
	"Kodak DCS620X",
	"Kodak DCS660C",
	"Kodak DCS660M",
	"Kodak DCS720X",
	"Kodak DCS760C",
	"Kodak DCS760M",
	"Kodak EOSDCS1",
	"Kodak EOSDCS3B",
	"Kodak NC2000F",
	"Kodak ProBack",
	"Kodak PB645C",
	"Kodak PB645H",
	"Kodak PB645M",
	"Kodak DCS Pro 14n",
	"Kodak DCS Pro 14nx",
	"Kodak DCS Pro SLR/c",
	"Kodak DCS Pro SLR/n",
	"Kodak C330",
	"Kodak C603",
	"Kodak P850",
	"Kodak P880",
	"Kodak S-1",
	"Kodak Z980",
	"Kodak Z981",
	"Kodak Z990",
	"Kodak Z1015",
	"Kodak KAI-0340",
	"Konica KD-400Z",
	"Konica KD-510Z",
	"Leaf AFi 5",
	"Leaf AFi 6",
	"Leaf AFi 7",
	"Leaf AFi-II 6",
	"Leaf AFi-II 7",
	"Leaf AFi-II 10",
	"Leaf AFi-II 10R",
	"Leaf Aptus-II 5",
	"Leaf Aptus-II 6",
	"Leaf Aptus-II 7",
	"Leaf Aptus-II 8",
	"Leaf Aptus-II 10",
	"Leaf Aptus-II 12",
	"Leaf Aptus-II 12R",
	"Leaf Aptus 17",
	"Leaf Aptus 22",
	"Leaf Aptus 54S",
	"Leaf Aptus 65",
	"Leaf Aptus 65S",
	"Leaf Aptus 75",
	"Leaf Aptus 75S",
	"Leaf Cantare",
	"Leaf Cantare XY",
	"Leaf CatchLight",
	"Leaf CMost",
	"Leaf Credo 40",
	"Leaf Credo 50",
	"Leaf Credo 60",
	"Leaf Credo 80 (low compression mode only)",
	"Leaf DCB-II",
	"Leaf Valeo 6",
	"Leaf Valeo 11",
	"Leaf Valeo 17",
	"Leaf Valeo 17wi",
	"Leaf Valeo 22",
	"Leaf Valeo 22wi",
	"Leaf Volare",
	"Lenovo a820",
	"Leica C (Typ 112)",
	"Leica Digilux 2",
	"Leica Digilux 3",
	"Leica Digital-Modul-R",
	"Leica D-LUX2",
	"Leica D-LUX3",
	"Leica D-LUX4",
	"Leica D-LUX5",
	"Leica D-LUX6",
	"Leica D-Lux (Typ 109)",
	"Leica M8",
	"Leica M8.2",
	"Leica M9",
	"Leica M10",
	"Leica M (Typ 240)",
	"Leica M (Typ 262)",
	"Leica Monochrom (Typ 240)",
	"Leica Monochrom (Typ 246)",
	"Leica M-D (Typ 262)",
	"Leica M-E",
	"Leica M-P",
	"Leica R8",
	"Leica Q (Typ 116)",
	"Leica S",
	"Leica S2",
	"Leica S (Typ 007)",
	"Leica SL (Typ 601)",
	"Leica T (Typ 701)",
	"Leica TL",
	"Leica X1",
	"Leica X (Typ 113)",
	"Leica X2",
	"Leica X-E (Typ 102)",
	"Leica X-U (Typ 113)",
	"Leica V-LUX1",
	"Leica V-LUX2",
	"Leica V-LUX3",
	"Leica V-LUX4",
	"Leica V-Lux (Typ 114)",
	"Leica X VARIO (Typ 107)",
	"LG G3",
	"LG G4",
	"Logitech Fotoman Pixtura",
	"Mamiya ZD",
	"Matrix 4608x3288",
	"Meizy MX4",
	"Micron 2010",
	"Minolta RD175",
	"Minolta DiMAGE 5",
	"Minolta DiMAGE 7",
	"Minolta DiMAGE 7i",
	"Minolta DiMAGE 7Hi",
	"Minolta DiMAGE A1",
	"Minolta DiMAGE A2",
	"Minolta DiMAGE A200",
	"Minolta DiMAGE G400",
	"Minolta DiMAGE G500",
	"Minolta DiMAGE G530",
	"Minolta DiMAGE G600",
	"Minolta DiMAGE Z2",
	"Minolta Alpha/Dynax/Maxxum 5D",
	"Minolta Alpha/Dynax/Maxxum 7D",
	"Motorola PIXL",
	"Nikon D1",
	"Nikon D1H",
	"Nikon D1X",
	"Nikon D2H",
	"Nikon D2Hs",
	"Nikon D2X",
	"Nikon D2Xs",
	"Nikon D3",
	"Nikon D3s",
	"Nikon D3X",
	"Nikon D4",
	"Nikon D4s",
	"Nikon D40",
	"Nikon D40X",
	"Nikon D5",
	"Nikon D50",
	"Nikon D60",
	"Nikon D70",
	"Nikon D70s",
	"Nikon D80",
	"Nikon D90",
	"Nikon D100",
	"Nikon D200",
	"Nikon D300",
	"Nikon D300s",
	"Nikon D500",
	"Nikon D600",
	"Nikon D610",
	"Nikon D700",
	"Nikon D750",
	"Nikon D800",
	"Nikon D800E",
	"Nikon D810",
	"Nikon D810A",
	"Nikon D3000",
	"Nikon D3100",
	"Nikon D3200",
	"Nikon D3300",
	"Nikon D3400",
	"Nikon D5000",
	"Nikon D5100",
	"Nikon D5200",
	"Nikon D5300",
	"Nikon D5500",
	"Nikon D5600",
	"Nikon D7000",
	"Nikon D7100",
	"Nikon D7200",
	"Nikon Df",
	"Nikon 1 AW1",
	"Nikon 1 J1",
	"Nikon 1 J2",
	"Nikon 1 J3",
	"Nikon 1 J4",
	"Nikon 1 J5",
	"Nikon 1 S1",
	"Nikon 1 S2",
	"Nikon 1 V1",
	"Nikon 1 V2",
	"Nikon 1 V3",
	"Nikon E700 (\"DIAG RAW\" hack)",
	"Nikon E800 (\"DIAG RAW\" hack)",
	"Nikon E880 (\"DIAG RAW\" hack)",
	"Nikon E900 (\"DIAG RAW\" hack)",
	"Nikon E950 (\"DIAG RAW\" hack)",
	"Nikon E990 (\"DIAG RAW\" hack)",
	"Nikon E995 (\"DIAG RAW\" hack)",
	"Nikon E2100 (\"DIAG RAW\" hack)",
	"Nikon E2500 (\"DIAG RAW\" hack)",
	"Nikon E3200 (\"DIAG RAW\" hack)",
	"Nikon E3700 (\"DIAG RAW\" hack)",
	"Nikon E4300 (\"DIAG RAW\" hack)",
	"Nikon E4500 (\"DIAG RAW\" hack)",
	"Nikon E5000",
	"Nikon E5400",
	"Nikon E5700",
	"Nikon E8400",
	"Nikon E8700",
	"Nikon E8800",
	"Nikon Coolpix A",
	"Nikon Coolpix B700",
	"Nikon Coolpix P330",
	"Nikon Coolpix P340",
	"Nikon Coolpix P6000",
	"Nikon Coolpix P7000",
	"Nikon Coolpix P7100",
	"Nikon Coolpix P7700",
	"Nikon Coolpix P7800",
	"Nikon Coolpix S6 (\"DIAG RAW\" hack)",
	"Nikon Coolscan NEF",
	"Nokia N95",
	"Nokia X2",
	"Nokia 1200x1600",
	"Nokia Lumia 950 XL",
	"Nokia Lumia 1020",
	"Nokia Lumia 1520",
	"Olympus AIR A01",
	"Olympus C3030Z",
	"Olympus C5050Z",
	"Olympus C5060Z",
	"Olympus C7070WZ",
	"Olympus C70Z,C7000Z",
	"Olympus C740UZ",
	"Olympus C770UZ",
	"Olympus C8080WZ",
	"Olympus X200,D560Z,C350Z",
	"Olympus E-1",
	"Olympus E-3",
	"Olympus E-5",
	"Olympus E-10",
	"Olympus E-20",
	"Olympus E-30",
	"Olympus E-300",
	"Olympus E-330",
	"Olympus E-400",
	"Olympus E-410",
	"Olympus E-420",
	"Olympus E-450",
	"Olympus E-500",
	"Olympus E-510",
	"Olympus E-520",
	"Olympus E-600",
	"Olympus E-620",
	"Olympus E-P1",
	"Olympus E-P2",
	"Olympus E-P3",
	"Olympus E-P5",
	"Olympus E-PL1",
	"Olympus E-PL1s",
	"Olympus E-PL2",
	"Olympus E-PL3",
	"Olympus E-PL5",
	"Olympus E-PL6",
	"Olympus E-PL7",
	"Olympus E-PL8",
	"Olympus E-PM1",
	"Olympus E-PM2",
	"Olympus E-M1",
	"Olympus E-M1 Mark II",
	"Olympus E-M10",
	"Olympus E-M10 Mark II",
	"Olympus E-M5",
	"Olympus E-M5 Mark II",
	"Olympus Pen F",
	"Olympus SP310",
	"Olympus SP320",
	"Olympus SP350",
	"Olympus SP500UZ",
	"Olympus SP510UZ",
	"Olympus SP550UZ",
	"Olympus SP560UZ",
	"Olympus SP565UZ",
	"Olympus SP570UZ",
	"Olympus STYLUS1",
	"Olympus STYLUS1s",
	"Olympus SH-2",
	"Olympus SH-3",
	"Olympus TG-4",
	"Olympus XZ-1",
	"Olympus XZ-2",
	"Olympus XZ-10",
	"OmniVision 4688",
	"OmniVision OV5647",
	"OmniVision OV5648",
	"OmniVision OV8850",
	"OmniVision 13860",
        "OnePlus One",
	"Panasonic DMC-CM1",
	"Panasonic DMC-FZ8",
	"Panasonic DMC-FZ18",
	"Panasonic DMC-FZ28",
	"Panasonic DMC-FZ30",
	"Panasonic DMC-FZ35/FZ38",
	"Panasonic DMC-FZ40",
	"Panasonic DMC-FZ45",
	"Panasonic DMC-FZ50",
	"Panasonic DMC-FZ7",
	"Panasonic DMC-FZ70",
	"Panasonic DMC-FZ72",
	"Panasonic DM-FZ80/82",
	"Panasonic DMC-FZ100",
	"Panasonic DMC-FZ150",
	"Panasonic DMC-FZ200",
	"Panasonic DMC-FZ300/330",
	"Panasonic DMC-FZ1000",
	"Panasonic DMC-FZ2000/2500/FZH1",
	"Panasonic DMC-FX150",
	"Panasonic DMC-G1",
	"Panasonic DMC-G10",
	"Panasonic DMC-G2",
	"Panasonic DMC-G3",
	"Panasonic DMC-G5",
	"Panasonic DMC-G6",
	"Panasonic DMC-G7/G70",
	"Panasonic DMC-G8/80/81/85",
	"Panasonic DMC-GF1",
	"Panasonic DMC-GF2",
	"Panasonic DMC-GF3",
	"Panasonic DMC-GF5",
	"Panasonic DMC-GF6",
	"Panasonic DMC-GF7",
	"Panasonic DMC-GH1",
	"Panasonic DMC-GH2",
	"Panasonic DMC-GH3",
	"Panasonic DMC-GH4",
	"Panasonic AG-GH4",
	"Panasonic DMC-GM1",
	"Panasonic DMC-GM1s",
	"Panasonic DMC-GM5",
	"Panasonic DMC-GX1",
	"Panasonic DMC-GX7",
	"Panasonic DMC-GX8",
	"Panasonic DMC-GX80/85",
	"Panasonic DC-GX800/850/GF9",
	"Panasonic DMC-L1",
	"Panasonic DMC-L10",
	"Panasonic DMC-LC1",
	"Panasonic DMC-LF1",
	"Panasonic DMC-LX1",
	"Panasonic DMC-LX2",
	"Panasonic DMC-LX3",
	"Panasonic DMC-LX5",
	"Panasonic DMC-LX7",
	"Panasonic DMC-LX9/10/15",
	"Panasonic DMC-LX100",
	"Panasonic DMC-TZ60/61/SZ40",
	"Panasonic DMC-TZ70/71/ZS50",
	"Panasonic DMC-TZ80/81/85/ZS60",
	"Panasonic DMC-TZ100/101/ZS100",
	"Pentax *ist D",
	"Pentax *ist DL",
	"Pentax *ist DL2",
	"Pentax *ist DS",
	"Pentax *ist DS2",
	"Pentax GR",
	"Pentax K10D",
	"Pentax K20D",
	"Pentax K100D",
	"Pentax K100D Super",
	"Pentax K110D",
	"Pentax K200D",
	"Pentax K2000/K-m",
	"Pentax K-x",
	"Pentax K-r",
	"Pentax K-01",
	"Pentax K-1",
	"Pentax K-3",
	"Pentax K-3 II",
	"Pentax K-30",
	"Pentax K-5",
	"Pentax K-5 II",
	"Pentax K-5 IIs",
	"Pentax K-50",
	"Pentax K-500",
	"Pentax K-7",
	"Pentax K-70",
	"Pentax K-S1",
	"Pentax K-S2",
	"Pentax MX-1",
	"Pentax Q",
	"Pentax Q7",
	"Pentax Q10",
	"Pentax QS-1",
	"Pentax Optio S",
	"Pentax Optio S4",
	"Pentax Optio 33WR",
	"Pentax Optio 750Z",
	"Pentax 645D",
	"Pentax 645Z",
	"PhaseOne IQ140",
	"PhaseOne IQ150",
	"PhaseOne IQ160",
	"PhaseOne IQ180",
	"PhaseOne IQ180 IR",
	"PhaseOne IQ250",
	"PhaseOne IQ260",
	"PhaseOne IQ260 Achromatic",
	"PhaseOne IQ280",
	"PhaseOne IQ3 50MP",
	"PhaseOne IQ3 60MP",
	"PhaseOne IQ3 80MP",
	"PhaseOne IQ3 100MP",
	"PhaseOne LightPhase",
	"PhaseOne Achromatic+",
	"PhaseOne H 10",
	"PhaseOne H 20",
	"PhaseOne H 25",
	"PhaseOne P 20",
	"PhaseOne P 20+",
	"PhaseOne P 21",
	"PhaseOne P 25",
	"PhaseOne P 25+",
	"PhaseOne P 30",
	"PhaseOne P 30+",
	"PhaseOne P 40+",
	"PhaseOne P 45",
	"PhaseOne P 45+",
	"PhaseOne P 65",
	"PhaseOne P 65+",
	"Photron BC2-HD",
	"Pixelink A782",
	"Polaroid x530",
	"RaspberryPi Camera",
	"RaspberryPi Camera V2",
	"Ricoh GR",
	"Ricoh GR Digital",
	"Ricoh GR Digital II",
	"Ricoh GR Digital III",
	"Ricoh GR Digital IV",
	"Ricoh GR II",
	"Ricoh GX100",
	"Ricoh GX200",
	"Ricoh GXR MOUNT A12",
	"Ricoh GXR MOUNT A16 24-85mm F3.5-5.5",
	"Ricoh GXR, S10 24-72mm F2.5-4.4 VC",
	"Ricoh GXR, GR A12 50mm F2.5 MACRO",
	"Ricoh GXR, GR LENS A12 28mm F2.5",
	"Ricoh GXR, GXR P10",
#ifndef NO_JASPER
	"Redcode R3D format",
#endif
	"Rollei d530flex",
	"RoverShot 3320af",
	"Samsung EX1",
	"Samsung EX2F",
	"Samsung GX-1L",
	"Samsung GX-1S",
	"Samsung GX10",
	"Samsung GX20",
	"Samsung Galaxy NX (EK-GN120)",
	"Samsung Galaxy S7 (SM-G935F)",
	"Samsung NX1",
	"Samsung NX5",
	"Samsung NX10",
	"Samsung NX11",
	"Samsung NX100",
	"Samsung NX1000",
	"Samsung NX1100",
	"Samsung NX20",
	"Samsung NX200",
	"Samsung NX210",
	"Samsung NX2000",
	"Samsung NX30",
	"Samsung NX300",
	"Samsung NX300M",
	"Samsung NX3000",
	"Samsung NX500",
	"Samsung NX mini",
	"Samsung Pro815",
	"Samsung WB550",
	"Samsung WB2000",
	"Samsung S85 (hacked)",
	"Samsung S850 (hacked)",
	"Samsung Galaxy S3",
	"Samsung Galaxy S7",
	"Samsung Galaxy S7 Edge",
	"Samsung Galaxy Nexus",
	"Sarnoff 4096x5440",
	"Seitz 6x17",
	"Seitz Roundshot D3",
	"Seitz Roundshot D2X",
	"Seitz Roundshot D2Xs",
	"Sigma SD9",
	"Sigma SD10",
	"Sigma SD14",
	"Sigma SD15",
	"Sigma SD1",
	"Sigma SD1 Merill",
	"Sigma DP1",
	"Sigma DP1 Merill",
	"Sigma DP1S",
	"Sigma DP1X",
	"Sigma DP2",
	"Sigma DP2 Merill",
	"Sigma DP2S",
	"Sigma DP2X",
	"Sigma DP3 Merill",
	"Sigma dp0 Quattro",
	"Sigma dp1 Quattro",
	"Sigma dp2 Quattro",
	"Sigma dp3 Quattro",
	"Sigma sd Quattro",
	"Sigma sd Quattro H",
	"Sinar eMotion 22",
	"Sinar eMotion 54",
	"Sinar eSpirit 65",
	"Sinar eMotion 75",
	"Sinar eVolution 75",
	"Sinar 3072x2048",
	"Sinar 4080x4080",
	"Sinar 4080x5440",
	"Sinar STI format",
	"Sinar Sinarback 54",
	"SMaL Ultra-Pocket 3",
	"SMaL Ultra-Pocket 4",
	"SMaL Ultra-Pocket 5",
	"Sony A7",
	"Sony A7 II",
	"Sony A7R",
	"Sony A7R II",
	"Sony A7S",
	"Sony A7S II",
	"Sony ILCA-68 (A68)",
	"Sony ILCA-77M2 (A77-II)",
	"Sony ILCA-99M2 (A99-II)",
	"Sony ILCE-3000",
	"Sony ILCE-5000",
	"Sony ILCE-5100",
	"Sony ILCE-6000",
	"Sony ILCE-6300",
	"Sony ILCE-6500",
	"Sony ILCE-QX1",
	"Sony DSC-F828",
	"Sony DSC-R1",
	"Sony DSC-RX1",
	"Sony DSC-RX1R",
	"Sony DSC-RX1R II",
	"Sony DSC-RX10",
	"Sony DSC-RX10II",
	"Sony DSC-RX10III",
	"Sony DSC-RX100",
	"Sony DSC-RX100II",
	"Sony DSC-RX100III",
	"Sony DSC-RX100IV",
	"Sony DSC-RX100V",
	"Sony DSC-V3",
	"Sony DSLR-A100",
	"Sony DSLR-A200",
	"Sony DSLR-A230",
	"Sony DSLR-A290",
	"Sony DSLR-A300",
	"Sony DSLR-A330",
	"Sony DSLR-A350",
	"Sony DSLR-A380",
	"Sony DSLR-A390",
	"Sony DSLR-A450",
	"Sony DSLR-A500",
	"Sony DSLR-A550",
	"Sony DSLR-A560",
	"Sony DSLR-A580",
	"Sony DSLR-A700",
	"Sony DSLR-A850",
	"Sony DSLR-A900",
	"Sony NEX-3",
	"Sony NEX-3N",
	"Sony NEX-5",
	"Sony NEX-5N",
	"Sony NEX-5R",
	"Sony NEX-5T",
	"Sony NEX-6",
	"Sony NEX-7",
	"Sony NEX-C3",
	"Sony NEX-F3",
	"Sony NEX-VG20",
	"Sony NEX-VG30",
	"Sony NEX-VG900",
	"Sony SLT-A33",
	"Sony SLT-A35",
	"Sony SLT-A37",
	"Sony SLT-A55V",
	"Sony SLT-A57",
	"Sony SLT-A58",
	"Sony SLT-A65V",
	"Sony SLT-A77V",
	"Sony SLT-A99V",
	"Sony XCD-SX910CR",
	"Sony IMX135-mipi 13mp",
	"Sony IMX135-QCOM",
	"Sony IMX072-mipi",
	"Sony IMX214",
	"Sony IMX219",
	"Sony IMX230",
	"Sony IMX298-mipi 16mp",
	"Sony IMX219-mipi 8mp",
	"Sony Xperia L",
	"STV680 VGA",
	"PtGrey GRAS-50S5C",
	"JaiPulnix BB-500CL",
	"JaiPulnix BB-500GE",
	"SVS SVS625CL",
	"YUNEEC CGO4",
	"Xiaomi MI3",
	"Xiaomi RedMi Note3 Pro",
	NULL
};
// clang-format on

const char **LibRaw::cameraList() { return static_camera_list; }
int LibRaw::cameraCount() { return (sizeof(static_camera_list) / sizeof(static_camera_list[0])) - 1; }

const char *LibRaw::strprogress(enum LibRaw_progress p)
{
  switch (p)
  {
  case LIBRAW_PROGRESS_START:
    return "Starting";
  case LIBRAW_PROGRESS_OPEN:
    return "Opening file";
  case LIBRAW_PROGRESS_IDENTIFY:
    return "Reading metadata";
  case LIBRAW_PROGRESS_SIZE_ADJUST:
    return "Adjusting size";
  case LIBRAW_PROGRESS_LOAD_RAW:
    return "Reading RAW data";
  case LIBRAW_PROGRESS_REMOVE_ZEROES:
    return "Clearing zero values";
  case LIBRAW_PROGRESS_BAD_PIXELS:
    return "Removing dead pixels";
  case LIBRAW_PROGRESS_DARK_FRAME:
    return "Subtracting dark frame data";
  case LIBRAW_PROGRESS_FOVEON_INTERPOLATE:
    return "Interpolating Foveon sensor data";
  case LIBRAW_PROGRESS_SCALE_COLORS:
    return "Scaling colors";
  case LIBRAW_PROGRESS_PRE_INTERPOLATE:
    return "Pre-interpolating";
  case LIBRAW_PROGRESS_INTERPOLATE:
    return "Interpolating";
  case LIBRAW_PROGRESS_MIX_GREEN:
    return "Mixing green channels";
  case LIBRAW_PROGRESS_MEDIAN_FILTER:
    return "Median filter";
  case LIBRAW_PROGRESS_HIGHLIGHTS:
    return "Highlight recovery";
  case LIBRAW_PROGRESS_FUJI_ROTATE:
    return "Rotating Fuji diagonal data";
  case LIBRAW_PROGRESS_FLIP:
    return "Flipping image";
  case LIBRAW_PROGRESS_APPLY_PROFILE:
    return "ICC conversion";
  case LIBRAW_PROGRESS_CONVERT_RGB:
    return "Converting to RGB";
  case LIBRAW_PROGRESS_STRETCH:
    return "Stretching image";
  case LIBRAW_PROGRESS_THUMB_LOAD:
    return "Loading thumbnail";
  default:
    return "Some strange things";
  }
}

#undef ID

#include "../internal/libraw_x3f.cpp"

void x3f_clear(void *p) { x3f_delete((x3f_t *)p); }

static char *utf2char(utf16_t *str, char *buffer)
{
  char *b = buffer;

  while (*str != 0x00)
  {
    char *chr = (char *)str;
    *b++ = *chr;
    str++;
  }
  *b = 0;
  return buffer;
}

static void *lr_memmem(const void *l, size_t l_len, const void *s, size_t s_len)
{
  register char *cur, *last;
  const char *cl = (const char *)l;
  const char *cs = (const char *)s;

  /* we need something to compare */
  if (l_len == 0 || s_len == 0)
    return NULL;

  /* "s" must be smaller or equal to "l" */
  if (l_len < s_len)
    return NULL;

  /* special case where s_len == 1 */
  if (s_len == 1)
    return (void *)memchr(l, (int)*cs, l_len);

  /* the last position where its possible to find "s" in "l" */
  last = (char *)cl + l_len - s_len;

  for (cur = (char *)cl; cur <= last; cur++)
    if (cur[0] == cs[0] && memcmp(cur, cs, s_len) == 0)
      return cur;
  return NULL;
}

void LibRaw::parse_x3f()
{
  x3f_t *x3f = x3f_new_from_file(libraw_internal_data.internal_data.input);
  if (!x3f)
    return;
  _x3f_data = x3f;

  x3f_header_t *H = NULL;
  x3f_directory_section_t *DS = NULL;

  H = &x3f->header;
  // Parse RAW size from RAW section
  x3f_directory_entry_t *DE = x3f_get_raw(x3f);
  if (!DE)
    return;
  imgdata.sizes.flip = H->rotation;
  x3f_directory_entry_header_t *DEH = &DE->header;
  x3f_image_data_t *ID = &DEH->data_subsection.image_data;
  imgdata.sizes.raw_width = ID->columns;
  imgdata.sizes.raw_height = ID->rows;
  // Parse other params from property section
  DE = x3f_get_prop(x3f);
  if ((x3f_load_data(x3f, DE) == X3F_OK))
  {
    // Parse property list
    DEH = &DE->header;
    x3f_property_list_t *PL = &DEH->data_subsection.property_list;
    if (PL->property_table.size != 0)
    {
      int i;
      x3f_property_t *P = PL->property_table.element;
      for (i = 0; i < PL->num_properties; i++)
      {
        char name[100], value[100];
        utf2char(P[i].name, name);
        utf2char(P[i].value, value);
        if (!strcmp(name, "ISO"))
          imgdata.other.iso_speed = atoi(value);
        if (!strcmp(name, "CAMMANUF"))
          strcpy(imgdata.idata.make, value);
        if (!strcmp(name, "CAMMODEL"))
          strcpy(imgdata.idata.model, value);
        if (!strcmp(name, "CAMSERIAL"))
          strcpy(imgdata.shootinginfo.BodySerial, value);
        if (!strcmp(name, "WB_DESC"))
          strcpy(imgdata.color.model2, value);
        if (!strcmp(name, "TIME"))
          imgdata.other.timestamp = atoi(value);
        if (!strcmp(name, "SHUTTER"))
          imgdata.other.shutter = atof(value);
        if (!strcmp(name, "APERTURE"))
          imgdata.other.aperture = atof(value);
        if (!strcmp(name, "FLENGTH"))
          imgdata.other.focal_len = atof(value);
        if (!strcmp(name, "FLEQ35MM"))
          imgdata.lens.makernotes.FocalLengthIn35mmFormat = atof(value);
        if (!strcmp(name, "LENSARANGE"))
        {
          char *sp;
          imgdata.lens.makernotes.MaxAp4CurFocal = imgdata.lens.makernotes.MinAp4CurFocal = atof(value);
          sp = strrchr(value, ' ');
          if (sp)
          {
            imgdata.lens.makernotes.MinAp4CurFocal = atof(sp);
            if (imgdata.lens.makernotes.MaxAp4CurFocal > imgdata.lens.makernotes.MinAp4CurFocal)
              my_swap(float, imgdata.lens.makernotes.MaxAp4CurFocal, imgdata.lens.makernotes.MinAp4CurFocal);
          }
        }
        if (!strcmp(name, "LENSFRANGE"))
        {
          char *sp;
          imgdata.lens.makernotes.MinFocal = imgdata.lens.makernotes.MaxFocal = atof(value);
          sp = strrchr(value, ' ');
          if (sp)
          {
            imgdata.lens.makernotes.MaxFocal = atof(sp);
            if ((imgdata.lens.makernotes.MaxFocal + 0.17f) < imgdata.lens.makernotes.MinFocal)
              my_swap(float, imgdata.lens.makernotes.MaxFocal, imgdata.lens.makernotes.MinFocal);
          }
        }
        if (!strcmp(name, "LENSMODEL"))
        {
          char *sp;
          imgdata.lens.makernotes.LensID = strtol(value, &sp, 16); // atoi(value);
          if (imgdata.lens.makernotes.LensID)
            imgdata.lens.makernotes.LensMount = Sigma_X3F;
        }
      }
      imgdata.idata.raw_count = 1;
      load_raw = &LibRaw::x3f_load_raw;
      imgdata.sizes.raw_pitch = imgdata.sizes.raw_width * 6;
      imgdata.idata.is_foveon = 1;
      libraw_internal_data.internal_output_params.raw_color = 1; // Force adobe coeff
      imgdata.color.maximum = 0x3fff;                            // To be reset by color table
      libraw_internal_data.unpacker_data.order = 0x4949;
    }
  }
  else
  {
    // No property list
    if (imgdata.sizes.raw_width == 5888 || imgdata.sizes.raw_width == 2944 || imgdata.sizes.raw_width == 6656 ||
        imgdata.sizes.raw_width == 3328 || imgdata.sizes.raw_width == 5504 ||
        imgdata.sizes.raw_width == 2752) // Quattro
    {
      imgdata.idata.raw_count = 1;
      load_raw = &LibRaw::x3f_load_raw;
      imgdata.sizes.raw_pitch = imgdata.sizes.raw_width * 6;
      imgdata.idata.is_foveon = 1;
      libraw_internal_data.internal_output_params.raw_color = 1; // Force adobe coeff
      libraw_internal_data.unpacker_data.order = 0x4949;
      strcpy(imgdata.idata.make, "SIGMA");
#if 1
      // Try to find model number in first 2048 bytes;
      int pos = libraw_internal_data.internal_data.input->tell();
      libraw_internal_data.internal_data.input->seek(0, SEEK_SET);
      unsigned char buf[2048];
      libraw_internal_data.internal_data.input->read(buf, 2048, 1);
      libraw_internal_data.internal_data.input->seek(pos, SEEK_SET);
      unsigned char *fnd = (unsigned char *)lr_memmem(buf, 2048, "SIGMA dp", 8);
      unsigned char *fndsd = (unsigned char *)lr_memmem(buf, 2048, "sd Quatt", 8);
      if (fnd)
      {
        unsigned char *nm = fnd + 8;
        snprintf(imgdata.idata.model, 64, "dp%c Quattro", *nm <= '9' && *nm >= '0' ? *nm : '2');
      }
      else if (fndsd)
      {
        snprintf(imgdata.idata.model, 64, "%s", fndsd);
      }
      else
#endif
          if (imgdata.sizes.raw_width == 6656 || imgdata.sizes.raw_width == 3328)
        strcpy(imgdata.idata.model, "sd Quattro H");
      else
        strcpy(imgdata.idata.model, "dp2 Quattro");
    }
    // else
  }
  // Try to get thumbnail data
  LibRaw_thumbnail_formats format = LIBRAW_THUMBNAIL_UNKNOWN;
  if ((DE = x3f_get_thumb_jpeg(x3f)))
  {
    format = LIBRAW_THUMBNAIL_JPEG;
  }
  else if ((DE = x3f_get_thumb_plain(x3f)))
  {
    format = LIBRAW_THUMBNAIL_BITMAP;
  }
  if (DE)
  {
    x3f_directory_entry_header_t *DEH = &DE->header;
    x3f_image_data_t *ID = &DEH->data_subsection.image_data;
    imgdata.thumbnail.twidth = ID->columns;
    imgdata.thumbnail.theight = ID->rows;
    imgdata.thumbnail.tcolors = 3;
    imgdata.thumbnail.tformat = format;
    libraw_internal_data.internal_data.toffset = DE->input.offset;
    write_thumb = &LibRaw::x3f_thumb_loader;
  }
}

INT64 LibRaw::x3f_thumb_size()
{
  try
  {
    x3f_t *x3f = (x3f_t *)_x3f_data;
    if (!x3f)
      return -1; // No data pointer set
    x3f_directory_entry_t *DE = x3f_get_thumb_jpeg(x3f);
    if (!DE)
      DE = x3f_get_thumb_plain(x3f);
    if (!DE)
      return -1;
    int64_t p = x3f_load_data_size(x3f, DE);
    if (p < 0 || p > 0xffffffff)
      return -1;
    return p;
  }
  catch (...)
  {
    return -1;
  }
}

void LibRaw::x3f_thumb_loader()
{
  try
  {
    x3f_t *x3f = (x3f_t *)_x3f_data;
    if (!x3f)
      return; // No data pointer set
    x3f_directory_entry_t *DE = x3f_get_thumb_jpeg(x3f);
    if (!DE)
      DE = x3f_get_thumb_plain(x3f);
    if (!DE)
      return;
    if (X3F_OK != x3f_load_data(x3f, DE))
      throw LIBRAW_EXCEPTION_IO_CORRUPT;
    x3f_directory_entry_header_t *DEH = &DE->header;
    x3f_image_data_t *ID = &DEH->data_subsection.image_data;
    imgdata.thumbnail.twidth = ID->columns;
    imgdata.thumbnail.theight = ID->rows;
    imgdata.thumbnail.tcolors = 3;
    if (imgdata.thumbnail.tformat == LIBRAW_THUMBNAIL_JPEG)
    {
      imgdata.thumbnail.thumb = (char *)malloc(ID->data_size);
      merror(imgdata.thumbnail.thumb, "LibRaw::x3f_thumb_loader()");
      memmove(imgdata.thumbnail.thumb, ID->data, ID->data_size);
      imgdata.thumbnail.tlength = ID->data_size;
    }
    else if (imgdata.thumbnail.tformat == LIBRAW_THUMBNAIL_BITMAP)
    {
      imgdata.thumbnail.tlength = ID->columns * ID->rows * 3;
      imgdata.thumbnail.thumb = (char *)malloc(ID->columns * ID->rows * 3);
      merror(imgdata.thumbnail.thumb, "LibRaw::x3f_thumb_loader()");
      char *src0 = (char *)ID->data;
      for (int row = 0; row < ID->rows; row++)
      {
        int offset = row * ID->row_stride;
        if (offset + ID->columns * 3 > ID->data_size)
          break;
        char *dest = &imgdata.thumbnail.thumb[row * ID->columns * 3];
        char *src = &src0[offset];
        memmove(dest, src, ID->columns * 3);
      }
    }
  }
  catch (...)
  {
    // do nothing
  }
}

static inline uint32_t _clampbits(int x, uint32_t n)
{
  uint32_t _y_temp;
  if ((_y_temp = x >> n))
    x = ~_y_temp >> (32 - n);
  return x;
}

void LibRaw::x3f_dpq_interpolate_rg()
{
  int w = imgdata.sizes.raw_width / 2;
  int h = imgdata.sizes.raw_height / 2;
  unsigned short *image = (ushort *)imgdata.rawdata.color3_image;

  for (int color = 0; color < 2; color++)
  {
    for (int y = 2; y < (h - 2); y++)
    {
      uint16_t *row0 = &image[imgdata.sizes.raw_width * 3 * (y * 2) + color]; // dst[1]
      uint16_t row0_3 = row0[3];
      uint16_t *row1 = &image[imgdata.sizes.raw_width * 3 * (y * 2 + 1) + color]; // dst1[1]
      uint16_t row1_3 = row1[3];
      for (int x = 2; x < (w - 2); x++)
      {
        row1[0] = row1[3] = row0[3] = row0[0];
        row0 += 6;
        row1 += 6;
      }
    }
  }
}

#define _ABS(a) ((a) < 0 ? -(a) : (a))

#undef CLIP
#define CLIP(value, high) ((value) > (high) ? (high) : (value))

void LibRaw::x3f_dpq_interpolate_af(int xstep, int ystep, int scale)
{
  unsigned short *image = (ushort *)imgdata.rawdata.color3_image;
  unsigned int rowpitch = imgdata.rawdata.sizes.raw_pitch / 2; // in 16-bit words
                                                               // Interpolate single pixel
  for (int y = 0; y < imgdata.rawdata.sizes.height + imgdata.rawdata.sizes.top_margin; y += ystep)
  {
    if (y < imgdata.rawdata.sizes.top_margin)
      continue;
    if (y < scale)
      continue;
    if (y > imgdata.rawdata.sizes.raw_height - scale)
      break;
    uint16_t *row0 = &image[imgdata.sizes.raw_width * 3 * y];                // Наша строка
    uint16_t *row_minus = &image[imgdata.sizes.raw_width * 3 * (y - scale)]; // Строка выше
    uint16_t *row_plus = &image[imgdata.sizes.raw_width * 3 * (y + scale)];  // Строка ниже
    for (int x = 0; x < imgdata.rawdata.sizes.width + imgdata.rawdata.sizes.left_margin; x += xstep)
    {
      if (x < imgdata.rawdata.sizes.left_margin)
        continue;
      if (x < scale)
        continue;
      if (x > imgdata.rawdata.sizes.raw_width - scale)
        break;
      uint16_t *pixel0 = &row0[x * 3];
      uint16_t *pixel_top = &row_minus[x * 3];
      uint16_t *pixel_bottom = &row_plus[x * 3];
      uint16_t *pixel_left = &row0[(x - scale) * 3];
      uint16_t *pixel_right = &row0[(x + scale) * 3];
      uint16_t *pixf = pixel_top;
      if (_ABS(pixf[2] - pixel0[2]) > _ABS(pixel_bottom[2] - pixel0[2]))
        pixf = pixel_bottom;
      if (_ABS(pixf[2] - pixel0[2]) > _ABS(pixel_left[2] - pixel0[2]))
        pixf = pixel_left;
      if (_ABS(pixf[2] - pixel0[2]) > _ABS(pixel_right[2] - pixel0[2]))
        pixf = pixel_right;
      int blocal = pixel0[2], bnear = pixf[2];
      if (blocal < imgdata.color.black + 16 || bnear < imgdata.color.black + 16)
      {
        if (pixel0[0] < imgdata.color.black)
          pixel0[0] = imgdata.color.black;
        if (pixel0[1] < imgdata.color.black)
          pixel0[1] = imgdata.color.black;
        pixel0[0] = CLIP((pixel0[0] - imgdata.color.black) * 4 + imgdata.color.black, 16383);
        pixel0[1] = CLIP((pixel0[1] - imgdata.color.black) * 4 + imgdata.color.black, 16383);
      }
      else
      {
        float multip = float(bnear - imgdata.color.black) / float(blocal - imgdata.color.black);
        if (pixel0[0] < imgdata.color.black)
          pixel0[0] = imgdata.color.black;
        if (pixel0[1] < imgdata.color.black)
          pixel0[1] = imgdata.color.black;
        float pixf0 = pixf[0];
        if (pixf0 < imgdata.color.black)
          pixf0 = imgdata.color.black;
        float pixf1 = pixf[1];
        if (pixf1 < imgdata.color.black)
          pixf1 = imgdata.color.black;

        pixel0[0] = CLIP(((float(pixf0 - imgdata.color.black) * multip + imgdata.color.black) +
                          ((pixel0[0] - imgdata.color.black) * 3.75 + imgdata.color.black)) /
                             2,
                         16383);
        pixel0[1] = CLIP(((float(pixf1 - imgdata.color.black) * multip + imgdata.color.black) +
                          ((pixel0[1] - imgdata.color.black) * 3.75 + imgdata.color.black)) /
                             2,
                         16383);
        // pixel0[1] = float(pixf[1]-imgdata.color.black)*multip + imgdata.color.black;
      }
    }
  }
}

void LibRaw::x3f_dpq_interpolate_af_sd(int xstart, int ystart, int xend, int yend, int xstep, int ystep, int scale)
{
  unsigned short *image = (ushort *)imgdata.rawdata.color3_image;
  unsigned int rowpitch = imgdata.rawdata.sizes.raw_pitch / 2; // in 16-bit words
  // Interpolate single pixel
  for (int y = ystart; y < yend && y < imgdata.rawdata.sizes.height + imgdata.rawdata.sizes.top_margin; y += ystep)
  {
    uint16_t *row0 = &image[imgdata.sizes.raw_width * 3 * y];                // Наша строка
    uint16_t *row1 = &image[imgdata.sizes.raw_width * 3 * (y + 1)];          // Следующая строка
    uint16_t *row_minus = &image[imgdata.sizes.raw_width * 3 * (y - scale)]; // Строка выше
    uint16_t *row_plus =
        &image[imgdata.sizes.raw_width * 3 * (y + scale)]; // Строка ниже AF-point (scale=2 -> ниже row1
    uint16_t *row_minus1 = &image[imgdata.sizes.raw_width * 3 * (y - 1)];
    for (int x = xstart; x < xend && x < imgdata.rawdata.sizes.width + imgdata.rawdata.sizes.left_margin; x += xstep)
    {
      uint16_t *pixel00 = &row0[x * 3]; // Current pixel
      float sumR = 0.f, sumG = 0.f;
      float cnt = 0.f;
      for (int xx = -scale; xx <= scale; xx += scale)
      {
        sumR += row_minus[(x + xx) * 3];
        sumR += row_plus[(x + xx) * 3];
        sumG += row_minus[(x + xx) * 3 + 1];
        sumG += row_plus[(x + xx) * 3 + 1];
        cnt += 1.f;
        if (xx)
        {
          cnt += 1.f;
          sumR += row0[(x + xx) * 3];
          sumG += row0[(x + xx) * 3 + 1];
        }
      }
      pixel00[0] = sumR / 8.f;
      pixel00[1] = sumG / 8.f;

      if (scale == 2)
      {
        uint16_t *pixel0B = &row0[x * 3 + 3]; // right pixel
        uint16_t *pixel1B = &row1[x * 3 + 3]; // right pixel
        float sumG0 = 0, sumG1 = 0.f;
        float cnt = 0.f;
        for (int xx = -scale; xx <= scale; xx += scale)
        {
          sumG0 += row_minus1[(x + xx) * 3 + 2];
          sumG1 += row_plus[(x + xx) * 3 + 2];
          cnt += 1.f;
          if (xx)
          {
            sumG0 += row0[(x + xx) * 3 + 2];
            sumG1 += row1[(x + xx) * 3 + 2];
            cnt += 1.f;
          }
        }
        pixel0B[2] = sumG0 / cnt;
        pixel1B[2] = sumG1 / cnt;
      }

      //			uint16_t* pixel10 = &row1[x*3]; // Pixel below current
      //			uint16_t* pixel_bottom = &row_plus[x*3];
    }
  }
}

void LibRaw::x3f_load_raw()
{
  // already in try/catch
  int raise_error = 0;
  x3f_t *x3f = (x3f_t *)_x3f_data;
  if (!x3f)
    return; // No data pointer set
  if (X3F_OK == x3f_load_data(x3f, x3f_get_raw(x3f)))
  {
    x3f_directory_entry_t *DE = x3f_get_raw(x3f);
    x3f_directory_entry_header_t *DEH = &DE->header;
    x3f_image_data_t *ID = &DEH->data_subsection.image_data;
    if (!ID)
      throw LIBRAW_EXCEPTION_IO_CORRUPT;
    x3f_quattro_t *Q = ID->quattro;
    x3f_huffman_t *HUF = ID->huffman;
    x3f_true_t *TRU = ID->tru;
    uint16_t *data = NULL;
    if (ID->rows != S.raw_height || ID->columns != S.raw_width)
    {
      raise_error = 1;
      goto end;
    }
    if (HUF != NULL)
      data = HUF->x3rgb16.data;
    if (TRU != NULL)
      data = TRU->x3rgb16.data;
    if (data == NULL)
    {
      raise_error = 1;
      goto end;
    }

    size_t datasize = S.raw_height * S.raw_width * 3 * sizeof(unsigned short);
    S.raw_pitch = S.raw_width * 3 * sizeof(unsigned short);
    if (!(imgdata.rawdata.raw_alloc = malloc(datasize)))
      throw LIBRAW_EXCEPTION_ALLOC;

    imgdata.rawdata.color3_image = (ushort(*)[3])imgdata.rawdata.raw_alloc;
    if (HUF)
      memmove(imgdata.rawdata.raw_alloc, data, datasize);
    else if (TRU && (!Q || !Q->quattro_layout))
      memmove(imgdata.rawdata.raw_alloc, data, datasize);
    else if (TRU && Q)
    {
      // Move quattro data in place
      // R/B plane
      for (int prow = 0; prow < TRU->x3rgb16.rows && prow < S.raw_height / 2; prow++)
      {
        ushort(*destrow)[3] =
            (unsigned short(*)[3]) & imgdata.rawdata.color3_image[prow * 2 * S.raw_pitch / 3 / sizeof(ushort)][0];
        ushort(*srcrow)[3] = (unsigned short(*)[3]) & data[prow * TRU->x3rgb16.row_stride];
        for (int pcol = 0; pcol < TRU->x3rgb16.columns && pcol < S.raw_width / 2; pcol++)
        {
          destrow[pcol * 2][0] = srcrow[pcol][0];
          destrow[pcol * 2][1] = srcrow[pcol][1];
        }
      }
      for (int row = 0; row < Q->top16.rows && row < S.raw_height; row++)
      {
        ushort(*destrow)[3] =
            (unsigned short(*)[3]) & imgdata.rawdata.color3_image[row * S.raw_pitch / 3 / sizeof(ushort)][0];
        ushort(*srcrow) = (unsigned short *)&Q->top16.data[row * Q->top16.columns];
        for (int col = 0; col < Q->top16.columns && col < S.raw_width; col++)
          destrow[col][2] = srcrow[col];
      }
    }

#if 1
    if (TRU && Q && (imgdata.params.raw_processing_options & LIBRAW_PROCESSING_DP2Q_INTERPOLATEAF))
    {
      if (imgdata.sizes.raw_width == 5888 && imgdata.sizes.raw_height == 3672) // dpN Quattro normal
      {
        x3f_dpq_interpolate_af(32, 8, 2);
      }
      else if (imgdata.sizes.raw_width == 5888 && imgdata.sizes.raw_height == 3776) // sd Quattro normal raw
      {
        x3f_dpq_interpolate_af_sd(216, 464, imgdata.sizes.raw_width - 1, 3312, 16, 32, 2);
      }
      else if (imgdata.sizes.raw_width == 6656 && imgdata.sizes.raw_height == 4480) // sd Quattro H normal raw
      {
        x3f_dpq_interpolate_af_sd(232, 592, imgdata.sizes.raw_width - 1, 3888, 16, 32, 2);
      }
      else if (imgdata.sizes.raw_width == 3328 && imgdata.sizes.raw_height == 2240) // sd Quattro H half size
      {
        x3f_dpq_interpolate_af_sd(116, 296, imgdata.sizes.raw_width - 1, 2200, 8, 16, 1);
      }
      else if (imgdata.sizes.raw_width == 5504 && imgdata.sizes.raw_height == 3680) // sd Quattro H APS-C raw
      {
        x3f_dpq_interpolate_af_sd(8, 192, imgdata.sizes.raw_width - 1, 3185, 16, 32, 2);
      }
      else if (imgdata.sizes.raw_width == 2752 && imgdata.sizes.raw_height == 1840) // sd Quattro H APS-C half size
      {
        x3f_dpq_interpolate_af_sd(4, 96, imgdata.sizes.raw_width - 1, 1800, 8, 16, 1);
      }
      else if (imgdata.sizes.raw_width == 2944 && imgdata.sizes.raw_height == 1836) // dpN Quattro small raw
      {
        x3f_dpq_interpolate_af(16, 4, 1);
      }
      else if (imgdata.sizes.raw_width == 2944 && imgdata.sizes.raw_height == 1888) // sd Quattro small
      {
        x3f_dpq_interpolate_af_sd(108, 232, imgdata.sizes.raw_width - 1, 1656, 8, 16, 1);
      }
    }
#endif
    if (TRU && Q && Q->quattro_layout && (imgdata.params.raw_processing_options & LIBRAW_PROCESSING_DP2Q_INTERPOLATERG))
      x3f_dpq_interpolate_rg();
  }
  else
    raise_error = 1;
end:
  if (raise_error)
    throw LIBRAW_EXCEPTION_IO_CORRUPT;
}
