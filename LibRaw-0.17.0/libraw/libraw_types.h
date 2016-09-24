/* -*- C++ -*-
 * File: libraw_types.h
 * Copyright 2008-2015 LibRaw LLC (info@libraw.org)
 * Created: Sat Mar  8 , 2008
 *
 * LibRaw C data structures
 *

LibRaw is free software; you can redistribute it and/or modify
it under the terms of the one of three licenses as you choose:

1. GNU LESSER GENERAL PUBLIC LICENSE version 2.1
   (See file LICENSE.LGPL provided in LibRaw distribution archive for details).

2. COMMON DEVELOPMENT AND DISTRIBUTION LICENSE (CDDL) Version 1.0
   (See file LICENSE.CDDL provided in LibRaw distribution archive for details).

3. LibRaw Software License 27032010
   (See file LICENSE.LibRaw.pdf provided in LibRaw distribution archive for details).

 */

#ifndef _LIBRAW_TYPES_H
#define _LIBRAW_TYPES_H

#include <sys/types.h>
#ifndef WIN32
#include <sys/time.h>
#endif
#include <stdio.h>

#if defined (_OPENMP)

#if defined(WIN32)
# if defined (_MSC_VER) && (_MSC_VER >= 1600 || (_MSC_VER == 1500 && _MSC_FULL_VER >= 150030729) )
/* VS2010+ : OpenMP works OK, VS2008: have tested by cgilles */
#   define LIBRAW_USE_OPENMP
#elif defined (__INTEL_COMPILER) && (__INTEL_COMPILER >=910)
/*  Have not tested on 9.x and 10.x, but Intel documentation claims OpenMP 2.5 support in 9.1 */
#   define LIBRAW_USE_OPENMP
#else
#  undef LIBRAW_USE_OPENMP
#endif
/* Not Win32 */
# elif (defined(__APPLE__) || defined(__MACOSX__)) && defined(_REENTRANT)
#   undef LIBRAW_USE_OPENMP
# else
#   define LIBRAW_USE_OPENMP
# endif
#endif

#ifdef LIBRAW_USE_OPENMP
#include <omp.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

#if defined(USE_LCMS)
#include <lcms.h>
#elif defined(USE_LCMS2)
#include <lcms2.h>
#else
#define NO_LCMS
#endif

#include "libraw_const.h"
#include "libraw_version.h"

#ifdef WIN32
typedef __int64 INT64;
typedef unsigned __int64 UINT64;
#else
typedef long long INT64;
typedef unsigned long long UINT64;
#endif

typedef unsigned char uchar;
typedef unsigned short ushort;

#ifdef WIN32
#ifdef LIBRAW_NODLL
# define DllDef
#else
# ifdef LIBRAW_BUILDLIB
#    define DllDef   __declspec( dllexport )
# else
#    define DllDef   __declspec( dllimport )
# endif
#endif
#else
#  define DllDef
#endif

typedef struct
{
    const char          *decoder_name;
    unsigned             decoder_flags;
}libraw_decoder_info_t;

typedef struct
{
    unsigned    mix_green;
    unsigned    raw_color;
    unsigned    zero_is_bad;
    ushort      shrink;
    ushort      fuji_width;
} libraw_internal_output_params_t;


typedef void (* memory_callback)(void * data, const char *file, const char *where);
typedef void (*exif_parser_callback) (void *context, int tag, int type, int len,unsigned int ord, void *ifp);

DllDef void default_memory_callback(void *data,const char *file, const char *where);

typedef void (*data_callback)(void *data,const char *file, const int offset);

DllDef void default_data_callback(void *data,const char *file, const int offset);

typedef int (* progress_callback) (void *data,enum LibRaw_progress stage, int iteration,int expected);

typedef struct
{
    memory_callback mem_cb;
    void*  memcb_data;

    data_callback data_cb;
    void*       datacb_data;

    progress_callback progress_cb;
    void *progresscb_data;

	exif_parser_callback exif_cb;
	void *exifparser_data;
} libraw_callbacks_t;


typedef struct
{
    enum LibRaw_image_formats type;
    ushort      height,
                width,
                colors,
                bits;
    unsigned int  data_size;
    unsigned char data[1];
}libraw_processed_image_t;


typedef struct
{
  char        make[64];
  char        model[64];
  char		  	software[64];
  unsigned    raw_count;
  unsigned    dng_version;
  unsigned    is_foveon;
  int         colors;
  unsigned    filters;
  char        xtrans[6][6];
  char        xtrans_abs[6][6];
  char        cdesc[5];
  unsigned    xmplen;
  char	      *xmpdata;

}libraw_iparams_t;

typedef struct
{
    ushort      raw_height,
                raw_width,
                height,
                width,
                top_margin,
                left_margin;
    ushort      iheight,
                iwidth;
    unsigned    raw_pitch;
    double      pixel_aspect;
    int         flip;
    int         mask[8][4];

} libraw_image_sizes_t;

struct ph1_t
{
  int format, key_off, tag_21a;
  int t_black, split_col, black_col, split_row, black_row;
  float tag_210;
};

typedef struct
{
  unsigned short illuminant;
  float calibration[4][4];
  float colormatrix[4][3];
} libraw_dng_color_t;

typedef struct
{
	int CanonColorDataVer;
	int CanonColorDataSubVer;
	int SpecularWhiteLevel;
	int AverageBlackLevel;
} libraw_canon_makernotes_t;

typedef struct
{
  ushort      curve[0x10000];
  unsigned    cblack[4102];
  unsigned    black;
  unsigned    data_maximum;
  unsigned    maximum;
  ushort      white[8][8];
  float       cam_mul[4];
  float       pre_mul[4];
  float       cmatrix[3][4];
  float       rgb_cam[3][4];
  float       cam_xyz[4][3];
  struct ph1_t       phase_one_data;
  float       flash_used;
  float       canon_ev;
  char        model2[64];
  void        *profile;
  unsigned    profile_length;
  unsigned    black_stat[8];
  libraw_dng_color_t  dng_color[2];
  libraw_canon_makernotes_t canon_makernotes;
  float	      baseline_exposure;
  int		  OlympusSensorCalibration[2];
  float       FujiExpoMidPointShift;
  int		digitalBack_color;
}libraw_colordata_t;

typedef struct
{
    enum LibRaw_thumbnail_formats tformat;
    ushort      twidth,
                theight;
    unsigned    tlength;
    int         tcolors;

    char       *thumb;
}libraw_thumbnail_t;

typedef struct
{
	float latitude[3]; /* Deg,min,sec */
	float longtitude[3]; /* Deg,min,sec */
	float gpstimestamp[3]; /* Deg,min,sec */
	float altitude;
	char  altref, latref, longref, gpsstatus;
	char  gpsparsed;

} libraw_gps_info_t;

typedef struct
{
    float       iso_speed;
    float       shutter;
    float       aperture;
    float       focal_len;
    time_t      timestamp;
    unsigned    shot_order;
    unsigned    gpsdata[32];
	libraw_gps_info_t parsed_gps;
    char        desc[512],
                artist[64];
} libraw_imgother_t;

typedef struct
{
    unsigned    greybox[4];     /* -A  x1 y1 x2 y2 */
    unsigned    cropbox[4];     /* -B x1 y1 x2 y2 */
    double      aber[4];        /* -C */
    double      gamm[6];        /* -g */
    float       user_mul[4];    /* -r mul0 mul1 mul2 mul3 */
    unsigned    shot_select;    /* -s */
    float       bright;         /* -b */
    float       threshold;      /*  -n */
    int         half_size;      /* -h */
    int         four_color_rgb; /* -f */
    int         highlight;      /* -H */
    int         use_auto_wb;    /* -a */
    int         use_camera_wb;  /* -w */
    int         use_camera_matrix; /* +M/-M */
    int         output_color;   /* -o */
    char        *output_profile; /* -o */
    char        *camera_profile; /* -p */
    char        *bad_pixels;    /* -P */
    char        *dark_frame;    /* -K */
    int         output_bps;     /* -4 */
    int         output_tiff;    /* -T */
    int         user_flip;      /* -t */
    int         user_qual;      /* -q */
    int         user_black;     /* -k */
    int		user_cblack[4];
    int         user_sat;       /* -S */

    int         med_passes;     /* -m */
    float       auto_bright_thr;
    float       adjust_maximum_thr;
    int         no_auto_bright; /* -W */
    int         use_fuji_rotate;/* -j */
    int         green_matching;
    /* DCB parameters */
    int         dcb_iterations;
    int         dcb_enhance_fl;
    int         fbdd_noiserd;
    /* VCD parameters */
    int         eeci_refine;
    int         es_med_passes;
    /* AMaZE*/
    int         ca_correc;
    float       cared;
    float	cablue;
    int cfaline;
    float linenoise;
    int cfa_clean;
    float lclean;
    float cclean;
    int cfa_green;
    float green_thresh;
    int exp_correc;
    float exp_shift;
    float exp_preser;
   /* WF debanding */
    int   wf_debanding;
    float wf_deband_treshold[4];
	/* Raw speed */
    int use_rawspeed;
  /* Disable Auto-scale */
    int no_auto_scale;
  /* Disable intepolation */
    int no_interpolation;
  /* Disable sRAW YCC to RGB conversion */
  int sraw_ycc;
  /* Force use x3f data decoding either if demosaic pack GPL2 enabled */
  int force_foveon_x3f;
  int x3f_flags;
  /* Sony ARW2 digging mode */
  int sony_arw2_options;
  int sony_arw2_posterization_thr;
  /* Nikon Coolscan */
  float coolscan_nef_gamma;
}libraw_output_params_t;

typedef struct
{
    /* really allocated bitmap */
  void          *raw_alloc;
  /* alias to single_channel variant */
  ushort        *raw_image;
  /* alias to 4-channel variant */
  ushort        (*color4_image)[4] ;
  /* alias to 3-color variand decoded by RawSpeed */
  ushort        (*color3_image)[3];

  /* Phase One black level data; */
  short  (*ph1_cblack)[2];
  short  (*ph1_rblack)[2];
  /* save color and sizes here, too.... */
  libraw_iparams_t  iparams;
  libraw_image_sizes_t sizes;
  libraw_internal_output_params_t ioparams;
  libraw_colordata_t color;
} libraw_rawdata_t;

typedef struct
{
	unsigned long long LensID;
	char	Lens[128];
	ushort	LensFormat;    /* to characterize the image circle the lens covers */
	ushort	LensMount;     /* 'male', lens itself */
	unsigned long long  CamID;
	ushort	CameraFormat;  /* some of the sensor formats */
	ushort	CameraMount;   /* 'female', body throat */
	char	body[64];
	short	FocalType;       /* -1/0 is unknown; 1 is fixed focal; 2 is zoom */
	char	LensFeatures_pre[16], LensFeatures_suf[16];
	float	MinFocal, MaxFocal;
	float	MaxAp4MinFocal, MaxAp4MaxFocal, MinAp4MinFocal, MinAp4MaxFocal;
	float	MaxAp, MinAp;
	float	CurFocal, CurAp;
	float	MaxAp4CurFocal, MinAp4CurFocal;
	float	LensFStops;
	unsigned long long TeleconverterID;
	char	Teleconverter[128];
	unsigned long long AdapterID;
	char	Adapter[128];
	unsigned long long AttachmentID;
	char	Attachment[128];
	short CanonFocalUnits;
	float	FocalLengthIn35mmFormat;
} libraw_makernotes_lens_t;

typedef struct
{
	float NikonEffectiveMaxAp;
	uchar NikonLensIDNumber, NikonLensFStops, NikonMCUVersion, NikonLensType;
} libraw_nikonlens_t;

typedef struct
{
	float MinFocal, MaxFocal, MaxAp4MinFocal, MaxAp4MaxFocal;
} libraw_dnglens_t;

typedef struct
{
	float MinFocal, MaxFocal, MaxAp4MinFocal, MaxAp4MaxFocal, EXIF_MaxAp;
	char LensMake[128], Lens[128];
	ushort FocalLengthIn35mmFormat;
	libraw_nikonlens_t nikon;
	libraw_dnglens_t dng;
	libraw_makernotes_lens_t makernotes;
} libraw_lensinfo_t;


typedef struct
{
  ushort                      (*image)[4] ;
  libraw_image_sizes_t        sizes;
  libraw_iparams_t            idata;
  libraw_lensinfo_t			  lens;
  libraw_output_params_t		params;
  unsigned int                progress_flags;
  unsigned int                process_warnings;
  libraw_colordata_t          color;
  libraw_imgother_t           other;
  libraw_thumbnail_t          thumbnail;
  libraw_rawdata_t            rawdata;
  void                *parent_class;
} libraw_data_t;


#ifdef __cplusplus
}
#endif

#endif
