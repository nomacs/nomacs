/* -*- C++ -*-
 * File: unprocessed_raw.cpp
 * Copyright 2008-2010 LibRaw LLC (info@libraw.org)
 * Created: Fri Jan 02, 2009
 *
 * LibRaw sample
 * Generates unprocessed raw image: with masked pixels and without black subtraction
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
#ifndef WIN32
#include <netinet/in.h>
#else
#include <winsock2.h>
#endif

#include "libraw/libraw.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

int main(int ac, char *av[])
{
    int  i, ret;
    int verbose=1,autoscale=0,use_gamma=0,add_borders=0,subtract_black=0;
    char outfn[1024]; 

    LibRaw RawProcessor;
    if(ac<2) 
        {
          usage:
            printf(
                "unprocessed_raw - LibRaw %s sample. %d cameras supported\n"
                "Usage: %s [-q] [-A] [-g] [-s N] [-N] raw-files....\n"
                "\t-q - be quiet\n"
                "\t-s N - select Nth image in file (default=0)\n"
                "\t-M - add black (masked) pixels data to bitmap\n"
                "\t-B - subtract black level from pixel data\n"
                "\t-g - use gamma correction with gamma 2.2 (not precise,use for visual inspection only)\n"
                "\t-A - autoscaling (by integer factor)\n"
                "\t-N - no raw curve/zeroes filtering\n"
                ,LibRaw::version(),
                LibRaw::cameraCount(),
                av[0]);
            return 0;
        }
    
#define P1 RawProcessor.imgdata.idata
#define S RawProcessor.imgdata.sizes
#define C RawProcessor.imgdata.color
#define T RawProcessor.imgdata.thumbnail
#define P2 RawProcessor.imgdata.other
#define OUT RawProcessor.imgdata.params

    OUT.document_mode=2;
    OUT.output_bps=16;
    OUT.output_tiff=1;
    OUT.user_flip=0;
    OUT.no_auto_bright = 1;
    OUT.filtering_mode=(LibRaw_filtering)(LIBRAW_FILTERING_NOZEROES);
    for (i=1;i<ac;i++)
        {
            if(av[i][0]=='-')
                {
                    if(av[i][1]=='q' && av[i][2]==0)
                        verbose=0;
                    else if(av[i][1]=='A' && av[i][2]==0)
                        autoscale=1;
                    else if(av[i][1]=='g' && av[i][2]==0)
                        use_gamma = 1;
                    else if(av[i][1]=='M' && av[i][2]==0)
                        add_borders = 1;
                    else if(av[i][1]=='B' && av[i][2]==0)
                        subtract_black = 1;
                    else if(av[i][1]=='N' && av[i][2]==0)
                        OUT.filtering_mode=LIBRAW_FILTERING_NONE;
                    else if(av[i][1]=='s' && av[i][2]==0)
                        {
                            i++;
                            OUT.shot_select=atoi(av[i]);
                        }
                    else
                        goto usage;
                    continue;
                }
            if(add_borders && subtract_black)
                {
                    fprintf(stderr,"Cannot add masked pixels data and subtract black simultaneously\n");
                    exit(1);
                }
            int c;
            if(!use_gamma)
                OUT.gamm[0] = OUT.gamm[1] = 1;
            if(verbose) printf("Processing file %s\n",av[i]);
            if( (ret = RawProcessor.open_file(av[i])) != LIBRAW_SUCCESS)
                {
                    fprintf(stderr,"Cannot open %s: %s\n",av[i],libraw_strerror(ret));
                    continue; // no recycle b/c open file will recycle itself
                }
            if(verbose)
                {
                    printf("Image size: %dx%d\nRaw size: %dx%d\n",S.width,S.height,S.raw_width,S.raw_height);
                    printf("Margins: top=%d, left=%d, right=%d, bottom=%d\n",
                           S.top_margin,S.left_margin,S.right_margin,S.bottom_margin);
                }

            if( (ret = RawProcessor.unpack() ) != LIBRAW_SUCCESS)
                {
                    fprintf(stderr,"Cannot unpack %s: %s\n",av[i],libraw_strerror(ret));
                    continue;
                }

            if(verbose)
                printf("Unpacked....\n");

            if(add_borders)
                if( (ret = RawProcessor.add_masked_borders_to_bitmap() ) != LIBRAW_SUCCESS)
                    {
                        fprintf(stderr,"Cannot add mask data to bitmap %s\n",av[i]);
                    }
            if(subtract_black)
                RawProcessor.subtract_black();
                
            // move all pixel data to component 0
            for(int r=0;r<S.iheight;r++)
                for(c=0;c<S.iwidth;c++)
                    RawProcessor.imgdata.image[r*S.iwidth+c][0] 
		      = RawProcessor.imgdata.image[r*S.iwidth+c][RawProcessor.COLOR(r,c)];
            P1.colors=1;
            if(autoscale)
                {
                    unsigned max=0,scale;
                    for(int j=0; j<S.iheight*S.iwidth; j++)
                        if(max < RawProcessor.imgdata.image[j][0])
                            max = RawProcessor.imgdata.image[j][0]; 
                    if (max >0 && max< 1<<15)
                        {
                            scale = (1<<16)/max;
                            if(verbose)
                                printf("Scaling with multiplier=%d (max=%d)\n",scale,max);
                            
                            for(int j=0; j<S.iheight*S.iwidth; j++)
                                RawProcessor.imgdata.image[j][0] *= scale;
                        }
                }

            
            if(OUT.shot_select)
                snprintf(outfn,sizeof(outfn),"%s-%d.tiff",av[i],OUT.shot_select);
            else
                snprintf(outfn,sizeof(outfn),"%s.tiff",av[i]);

            if(verbose) printf("Writing file %s\n",outfn);
            if( LIBRAW_SUCCESS != (ret = RawProcessor.dcraw_ppm_tiff_writer(outfn)))
                fprintf(stderr,"Cannot write %s: %s\n",outfn,libraw_strerror(ret));
        }
    return 0;
}
