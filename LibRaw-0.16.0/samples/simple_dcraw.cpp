/* -*- C++ -*-
 * File: simple_dcraw.cpp
 * Copyright 2008-2013 LibRaw LLC (info@libraw.org)
 * Created: Sat Mar  8, 2008
 *
 * LibRaw simple C++ API:  emulates call to "dcraw  [-D]  [-T] [-v] [-e] [-4]"

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
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#endif

#include "libraw/libraw.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

int my_progress_callback(void *unused_data,enum LibRaw_progress state,int iter, int expected)
{
    if(iter==0)
        printf("CB: state=%x, expected %d iterations\n",state,expected);
    return 0;
}


int main(int ac, char *av[])
{
    int  i, ret, verbose=0, output_thumbs=0;

    // don't use fixed size buffers in real apps!
    char outfn[1024],thumbfn[1024]; 

    LibRaw RawProcessor;
    if(ac<2) 
        {
            printf(
                "simple_dcraw - LibRaw %s sample. Emulates dcraw [-D] [-T] [-v] [-e] [-E]\n"
                " %d cameras supported\n"
                "Usage: %s [-D] [-T] [-v] [-e] raw-files....\n"
                "\t-4 - 16-bit mode\n"
                "\t-v - verbose output\n"
                "\t-T - output TIFF files instead of .pgm/ppm\n"
                "\t-e - extract thumbnails (same as dcraw -e in separate run)\n",LibRaw::version(),
                LibRaw::cameraCount(),
                av[0]);
            return 0;
        }
    
    putenv ((char*)"TZ=UTC"); // dcraw compatibility, affects TIFF datestamp field

#define P1 RawProcessor.imgdata.idata
#define S RawProcessor.imgdata.sizes
#define C RawProcessor.imgdata.color
#define T RawProcessor.imgdata.thumbnail
#define P2 RawProcessor.imgdata.other
#define OUT RawProcessor.imgdata.params


    for (i=1;i<ac;i++)
        {
            if(av[i][0]=='-')
                {
                    if(av[i][1]=='T' && av[i][2]==0)
                        OUT.output_tiff=1;
                    if(av[i][1]=='v' && av[i][2]==0)
                        verbose++;
                    if(av[i][1]=='e' && av[i][2]==0)
                        output_thumbs++;
                    if(av[i][1]=='4' && av[i][2]==0)
                        OUT.output_bps=16;
                    if(av[i][1]=='C' && av[i][2]==0)
                        RawProcessor.set_progress_handler(my_progress_callback,NULL);
                    continue;
                }

            if(verbose) printf("Processing file %s\n",av[i]);

            if( (ret = RawProcessor.open_file(av[i])) != LIBRAW_SUCCESS)
                {
                    fprintf(stderr,"Cannot open_file %s: %s\n",av[i],libraw_strerror(ret));
                    continue; // no recycle b/c open file will recycle itself
                }
            
            if(!output_thumbs) // No unpack for thumb extraction
              if( (ret = RawProcessor.unpack() ) != LIBRAW_SUCCESS)
                {
                    fprintf(stderr,"Cannot unpack %s: %s\n",av[i],libraw_strerror(ret));
                    continue;
                }

            // thumbnail unpacking and output in the middle of main
            // image processing - for test purposes!
            if(output_thumbs)
                {
                    if( (ret = RawProcessor.unpack_thumb() ) != LIBRAW_SUCCESS)
                        {
                            fprintf(stderr,"Cannot unpack_thumb %s: %s\n",av[i],libraw_strerror(ret));
                            if(LIBRAW_FATAL_ERROR(ret))
                                continue; // skip to next file
                        }
                    else
                        {
                            snprintf(thumbfn,sizeof(thumbfn),"%s.%s",
                                     av[i],T.tformat == LIBRAW_THUMBNAIL_JPEG ? "thumb.jpg" : "thumb.ppm");

                            if(verbose) printf("Writing thumbnail file %s\n",thumbfn);
                            if( LIBRAW_SUCCESS != (ret = RawProcessor.dcraw_thumb_writer(thumbfn)))
                                {
                                    fprintf(stderr,"Cannot write %s: %s\n",thumbfn,libraw_strerror(ret));
                                    if(LIBRAW_FATAL_ERROR(ret))
                                        continue; 
                                }
                        }
                    continue;
                }
            
            ret = RawProcessor.dcraw_process();
                
            if(LIBRAW_SUCCESS !=ret)
                {
                    fprintf(stderr,"Cannot do postpocessing on %s: %s\n",
                            av[i],libraw_strerror(ret));
                    if(LIBRAW_FATAL_ERROR(ret))
                        continue; 
                }
            snprintf(outfn,sizeof(outfn),
                     "%s.%s",
                     av[i], OUT.output_tiff ? "tiff" : (P1.colors>1?"ppm":"pgm"));

            if(verbose) printf("Writing file %s\n",outfn);

            if( LIBRAW_SUCCESS != (ret = RawProcessor.dcraw_ppm_tiff_writer(outfn)))
                fprintf(stderr,"Cannot write %s: %s\n",outfn,libraw_strerror(ret));

            RawProcessor.recycle(); // just for show this call
        }
    return 0;
}
