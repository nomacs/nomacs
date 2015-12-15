/* -*- C++ -*-
 * File: identify.cpp
 * Copyright 2008-2015 LibRaw LLC (info@libraw.org)
 * Created: Sat Mar  8, 2008
 *
 * LibRaw C++ demo: emulates dcraw -i [-v]
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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "libraw/libraw.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

#define P1 MyCoolRawProcessor.imgdata.idata
#define P2 MyCoolRawProcessor.imgdata.other

#define mnLens MyCoolRawProcessor.imgdata.lens.makernotes
#define exifLens MyCoolRawProcessor.imgdata.lens

#define S MyCoolRawProcessor.imgdata.sizes
#define O MyCoolRawProcessor.imgdata.params
#define C MyCoolRawProcessor.imgdata.color
#define T MyCoolRawProcessor.imgdata.thumbnail


int main(int ac, char *av[])
{
    int verbose = 0, ret,print_unpack=0,print_frame=0;
    LibRaw MyCoolRawProcessor;

    for (int i=1;i<ac;i++) {
        if(av[i][0]=='-')
            {
                if(av[i][1]=='v' && av[i][2]==0) verbose++;
                if(av[i][1]=='u' && av[i][2]==0) print_unpack++;
                if(av[i][1]=='f' && av[i][2]==0) print_frame++;
                if(av[i][1]=='x' && av[i][2]==0) O.force_foveon_x3f=1;
                continue;
            }
        if( (ret = MyCoolRawProcessor.open_file(av[i])) != LIBRAW_SUCCESS)
          {
            printf("Cannot decode %s: %s\n",av[i],libraw_strerror(ret));
            continue; // no recycle, open_file will recycle
          }
        if(verbose) {
            if( (ret =  MyCoolRawProcessor.adjust_sizes_info_only()))
            {
                printf("Cannot decode %s: %s\n",av[i],libraw_strerror(ret));
                continue; // no recycle, open_file will recycle
            }

            printf ("\nFilename: %s\n", av[i]);
            printf ("Timestamp: %s", ctime(&(P2.timestamp)));
            printf ("Camera: %s %s\n", P1.make, P1.model);
            if (P2.artist[0])
                printf ("Owner: %s\n", P2.artist);
            if (P1.dng_version) {
                printf ("DNG Version: ");
                for (int i=24; i >= 0; i -= 8)
                    printf ("%d%c", P1.dng_version >> i & 255, i ? '.':'\n');
            }
            printf ("\nEXIF:\n");
            printf ("\tMinFocal: %0.1f mm\n", exifLens.MinFocal);
            printf ("\tMaxFocal: %0.1f mm\n", exifLens.MaxFocal);
            printf ("\tMaxAp @MinFocal: f/%0.1f\n", exifLens.MaxAp4MinFocal);
            printf ("\tMaxAp @MaxFocal: f/%0.1f\n", exifLens.MaxAp4MaxFocal);
            printf ("\tMaxAperture @CurFocal: f/%0.1f\n", exifLens.EXIF_MaxAp);
            printf ("\tFocalLengthIn35mmFormat: %d mm\n", exifLens.FocalLengthIn35mmFormat);
            printf ("\tLensMake: %s\n", exifLens.LensMake);
            printf ("\tLens: %s\n", exifLens.Lens);
            printf ("\n");

			printf ("\nMakernotes:\n");
			if (mnLens.body[0])
			  {
			  	printf("\tMF Camera Body: %s\n", mnLens.body);
			  }
			printf("\tCameraFormat: %d, ", mnLens.CameraFormat);
			switch (mnLens.CameraFormat)
				{
					case 0:  printf("Undefined\n"); break;
					case 1:  printf("APS-C\n"); break;
					case 2:  printf("FF\n"); break;
					case 3:  printf("MF\n"); break;
					case 4:  printf("APS-H\n"); break;
					case 8:  printf("4/3\n"); break;
					default: printf("Unknown\n"); break;
				}
			printf("\tCameraMount: %d, ", mnLens.CameraMount);
			switch (mnLens.CameraMount)
				{
					case 0:  printf("Undefined or Fixed Lens\n"); break;
					case 1:  printf("Sony/Minolta A\n"); break;
					case 2:  printf("Sony E\n"); break;
					case 3:  printf("Canon EF\n"); break;
					case 4:  printf("Canon EF-S\n"); break;
					case 5:  printf("Canon EF-M\n"); break;
					case 6:  printf("Nikon F\n"); break;
					case 7:  printf("Nikon CX\n"); break;
					case 8:  printf("4/3\n"); break;
					case 9:  printf("m4/3\n"); break;
					case 10: printf("Pentax K\n"); break;
					case 11: printf("Pentax Q\n"); break;
					case 12: printf("Pentax 645\n"); break;
					case 13: printf("Fuji X\n"); break;
					case 14: printf("Leica M\n"); break;
					case 15: printf("Leica R\n"); break;
					case 16: printf("Leica S\n"); break;
					case 17: printf("Samsung NX\n"); break;
					case 19: printf("Samsung NX-M\n"); break;
					case 99: printf("Fixed Lens\n"); break;
					default: printf("Unknown\n"); break;
				}

			if (mnLens.LensID == -1)
				{
					printf("\tLensID: n/a\n");
				}
			else
				{
					printf("\tLensID: %llu 0x%0llx\n", mnLens.LensID, mnLens.LensID);
				}
			printf("\tLens: %s\n", mnLens.Lens);
			printf("\tLensFormat: %d, ", mnLens.LensFormat);
			switch (mnLens.LensFormat)
				{
					case 0:  printf("Undefined\n"); break;
					case 1:  printf("APS-C\n"); break;
					case 2:  printf("FF\n"); break;
					case 3:  printf("MF\n"); break;
					case 8:  printf("4/3\n"); break;
					default: printf("Unknown\n"); break;
				}
			printf("\tLensMount: %d, ", mnLens.LensMount);
			switch (mnLens.LensMount)
				{
					case 0:  printf("Undefined or Fixed Lens\n"); break;
					case 1:  printf("Sony/Minolta A\n"); break;
					case 2:  printf("Sony E\n"); break;
					case 3:  printf("Canon EF\n"); break;
					case 4:  printf("Canon EF-S\n"); break;
					case 5:  printf("Canon EF-M\n"); break;
					case 6:  printf("Nikon F\n"); break;
					case 7:  printf("Nikon CX\n"); break;
					case 8:  printf("4/3\n"); break;
					case 9:  printf("m4/3\n"); break;
					case 10: printf("Pentax K\n"); break;
					case 11: printf("Pentax Q\n"); break;
					case 12: printf("Pentax 645\n"); break;
					case 13: printf("Fuji X\n"); break;
					case 14: printf("Leica M\n"); break;
					case 15: printf("Leica R\n"); break;
					case 16: printf("Leica S\n"); break;
					case 17: printf("Samsung NX\n"); break;
					case 18: printf("Ricoh module\n"); break;
					case 99: printf("Fixed Lens\n"); break;
					default: printf("Unknown\n"); break;
				}
			printf("\tFocalType: %d, ", mnLens.FocalType);
			switch (mnLens.FocalType)
				{
					case 0:  printf("Undefined\n"); break;
					case 1:  printf("Fixed Focal\n"); break;
					case 2:  printf("Zoom\n"); break;
					default: printf("Unknown\n"); break;
				}
			printf("\tLensFeatures_pre: %s\n", mnLens.LensFeatures_pre);
			printf("\tLensFeatures_suf: %s\n", mnLens.LensFeatures_suf);
			printf("\tMinFocal: %0.1f mm\n", mnLens.MinFocal);
			printf("\tMaxFocal: %0.1f mm\n", mnLens.MaxFocal);
			printf("\tMaxAp @MinFocal: f/%0.1f\n", mnLens.MaxAp4MinFocal);
			printf("\tMaxAp @MaxFocal: f/%0.1f\n", mnLens.MaxAp4MaxFocal);
			printf("\tMinAp @MinFocal: f/%0.1f\n", mnLens.MinAp4MinFocal);
			printf("\tMinAp @MaxFocal: f/%0.1f\n", mnLens.MinAp4MaxFocal);
			printf("\tMaxAp: f/%0.1f\n", mnLens.MaxAp);
			printf("\tMinAp: f/%0.1f\n", mnLens.MinAp);
			printf("\tCurFocal: %0.1f mm\n", mnLens.CurFocal);
			printf("\tCurAp: f/%0.1f\n", mnLens.CurAp);
			printf("\tMaxAp @CurFocal: f/%0.1f\n", mnLens.MaxAp4CurFocal);
			printf("\tMinAp @CurFocal: f/%0.1f\n", mnLens.MinAp4CurFocal);

			if (exifLens.makernotes.FocalLengthIn35mmFormat > 1.0f)
				printf("\tFocalLengthIn35mmFormat: %0.1f mm\n", exifLens.makernotes.FocalLengthIn35mmFormat);

			if (exifLens.nikon.NikonEffectiveMaxAp > 0.1f)
				printf("\tNikonEffectiveMaxAp: f/%0.1f\n", exifLens.nikon.NikonEffectiveMaxAp);

			if (exifLens.makernotes.LensFStops > 0.1f)
				printf("\tLensFStops @CurFocal: %0.2f\n", exifLens.makernotes.LensFStops);

			printf("\tTeleconverterID: %lld\n", mnLens.TeleconverterID);
			printf("\tTeleconverter: %s\n", mnLens.Teleconverter);
			printf("\tAdapterID: %lld\n", mnLens.AdapterID);
			printf("\tAdapter: %s\n", mnLens.Adapter);
			printf("\tAttachmentID: %lld\n", mnLens.AttachmentID);
			printf("\tAttachment: %s\n", mnLens.Attachment);
			printf ("\n");

            printf ("ISO speed: %d\n", (int) P2.iso_speed);
            printf ("Shutter: ");
            if (P2.shutter > 0 && P2.shutter < 1)
                P2.shutter = (printf ("1/"), 1 / P2.shutter);
            printf ("%0.1f sec\n", P2.shutter);
            printf ("Aperture: f/%0.1f\n", P2.aperture);
            printf ("Focal length: %0.1f mm\n", P2.focal_len);
            if(C.profile)
                printf ("Embedded ICC profile: yes, %d bytes\n", C.profile_length);
            else
                printf ("Embedded ICC profile: no\n");

	   if (C.baseline_exposure > -999.f) printf ("Baseline exposure: %04.3f\n", C.baseline_exposure);

            printf ("Number of raw images: %d\n", P1.raw_count);
            if (C.FujiExpoMidPointShift) printf ("Fuji Exposure shift: %04.3f\n", C.FujiExpoMidPointShift);
            if (S.pixel_aspect != 1)
                printf ("Pixel Aspect Ratio: %0.6f\n", S.pixel_aspect);
            if (T.tlength)
                printf ("Thumb size:  %4d x %d\n", T.twidth, T.theight);
            printf ("Full size:   %4d x %d\n", S.raw_width, S.raw_height);

            printf ("Image size:  %4d x %d\n", S.width, S.height);
            printf ("Output size: %4d x %d\n", S.iwidth, S.iheight);
            printf ("Raw colors: %d", P1.colors);
            if (P1.filters)
                {
                    printf ("\nFilter pattern: ");
                    if (!P1.cdesc[3]) P1.cdesc[3] = 'G';
                    for (int i=0; i < 16; i++)
                        putchar (P1.cdesc[MyCoolRawProcessor.fcol(i >> 1,i & 1)]);
                }
            printf ("\nDaylight multipliers:");
            for(int c=0;c<P1.colors;c++) printf (" %f", C.pre_mul[c]);
            if (C.cam_mul[0] > 0)
                {
                    printf ("\nCamera multipliers:");
                    for(int c=0;c<4;c++) printf (" %f", C.cam_mul[c]);
                }
            printf("\nCam->XYZ matrix:\n");
            for(int i=0; i< 4; i++)
                printf("%6.4f\t%6.4f\t%6.4f\n",C.cam_xyz[i][0],C.cam_xyz[i][1],C.cam_xyz[i][2]);
        }
        else
            {
                if(print_unpack)
                    {
                        char frame[48]="";
                        if(print_frame)
                            {
                                ushort right_margin = S.raw_width - S.width - S.left_margin;
                                ushort bottom_margin = S.raw_height - S.height - S.top_margin;
                                snprintf(frame,48,"F=%dx%dx%dx%d RS=%dx%d",
                                         S.left_margin,S.top_margin,right_margin,bottom_margin,
                                         S.raw_width,S.raw_height);
                                printf ("%s\t%s\t%s\t%s/%s\n",
                                av[i],
                                MyCoolRawProcessor.unpack_function_name(),
                                frame,
                                P1.make, P1.model);
                            }
                    }
                else
                    printf ("%s is a %s %s image.\n", av[i],P1.make, P1.model);
//					printf ("%s=%s=%d=%04.3f=%04.3f\n", P1.make, P1.model, (int)P2.iso_speed, C.baseline_exposure, C.FujiExpoMidPointShift);
            }
        MyCoolRawProcessor.recycle();
    }// endfor
    return 0;
}
