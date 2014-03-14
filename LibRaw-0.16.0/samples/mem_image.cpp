/* -*- C++ -*-
 * File: mem_image.cpp
 * Copyright 2008-2013 LibRaw LLC (info@libraw.org)
 *
 * LibRaw mem_image/mem_thumb API test. Results should be same (bitwise) to dcraw [-4] [-6] [-e]
 * Testing note: for ppm-thumbnails you should use dcraw -w -e for thumbnail extraction

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

#include "libraw/libraw.h"

#ifdef WIN32
#define snprintf _snprintf
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <netinet/in.h>
#endif


// no error reporting, only params check
void write_ppm(libraw_processed_image_t *img, const char *basename)
{
    if(!img) return;
    // type SHOULD be LIBRAW_IMAGE_BITMAP, but we'll check
    if(img->type != LIBRAW_IMAGE_BITMAP) return;
    // only 3-color images supported...
    if(img->colors != 3) return;

    char fn[1024];
    snprintf(fn,1024,"%s.ppm",basename);
    FILE *f = fopen(fn,"wb");
    if(!f) return;
    fprintf (f, "P6\n%d %d\n%d\n", img->width, img->height, (1 << img->bits)-1);
/*
  NOTE:
  data in img->data is not converted to network byte order.
  So, we should swap values on some architectures for dcraw compatibility
  (unfortunately, xv cannot display 16-bit PPMs with network byte order data
*/
#define SWAP(a,b) { a ^= b; a ^= (b ^= a); }
    if (img->bits == 16 && htons(0x55aa) != 0x55aa)
        for(unsigned i=0; i< img->data_size; i+=2)
            SWAP(img->data[i],img->data[i+1]);
#undef SWAP

    fwrite(img->data,img->data_size,1,f);
    fclose(f);
}

void write_thumb(libraw_processed_image_t *img, const char *basename)
{
    if(!img) return;

    if(img->type == LIBRAW_IMAGE_BITMAP)
        {
            char fnt[1024];
            snprintf(fnt,1024,"%s.thumb",basename);
            write_ppm(img,fnt);
        }
    else if (img->type == LIBRAW_IMAGE_JPEG)
        {
            char fn[1024];
            snprintf(fn,1024,"%s.thumb.jpg",basename);
            FILE *f = fopen(fn,"wb");
            if(!f) return;
            fwrite(img->data,img->data_size,1,f);
            fclose(f);
        }
}



int main(int ac, char *av[])
{
    int  i, ret, output_thumbs=0;

    // don't use fixed size buffers in real apps!

    LibRaw RawProcessor;
    
    if(ac<2) 
        {
            printf(
                "mem_image - LibRaw sample, to illustrate work for memory buffers. Emulates dcraw [-4] [-1] [-e] [-h]\n"
                "Usage: %s [-D] [-T] [-v] [-e] raw-files....\n"
                "\t-6 - output 16-bit PPM\n"
                "\t-4 - linear 16-bit data\n"
                "\t-e - extract thumbnails (same as dcraw -e in separate run)\n",
                "\t-h - use half_size\n");
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
                    if(av[i][1]=='6' && av[i][2]==0)
                        OUT.output_bps = 16;
                    if(av[i][1]=='4' && av[i][2]==0)
                        {
                            OUT.output_bps = 16;
                            OUT.gamm[0] = OUT.gamm[1] =  OUT.no_auto_bright    = 1;
                        }
                    if(av[i][1]=='e' && av[i][2]==0)
                        output_thumbs++;
                    if(av[i][1]=='h' && av[i][2]==0)
                        OUT.half_size=1;
                    continue;
                }
            printf("Processing %s\n",av[i]);
            if( (ret = RawProcessor.open_file(av[i])) != LIBRAW_SUCCESS)
                {
                    fprintf(stderr,"Cannot open %s: %s\n",av[i],libraw_strerror(ret));
                    continue; // no recycle b/c open file will recycle itself
                }
            
            if( (ret = RawProcessor.unpack() ) != LIBRAW_SUCCESS)
                {
                    fprintf(stderr,"Cannot unpack %s: %s\n",av[i],libraw_strerror(ret));
                    continue;
                }

            // we should call dcraw_process before thumbnail extraction because for
            // some cameras (i.e. Kodak ones) white balance for thumbnal should be set
            // from main image settings


            ret = RawProcessor.dcraw_process();
                
            if(LIBRAW_SUCCESS !=ret)
                {
                    fprintf(stderr,"Cannot do postpocessing on %s: %s\n",
                            av[i],libraw_strerror(ret));
                    if(LIBRAW_FATAL_ERROR(ret))
                        continue; 
                }
            libraw_processed_image_t *image = RawProcessor.dcraw_make_mem_image(&ret);
            if(image)
                {
                    write_ppm(image,av[i]);
                    LibRaw::dcraw_clear_mem(image);
                }
            else
                fprintf(stderr,"Cannot unpack %s to memory buffer: %s\n" , av[i],libraw_strerror(ret));

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
                            libraw_processed_image_t *thumb = RawProcessor.dcraw_make_mem_thumb(&ret);
                            if(thumb)
                                {
                                    write_thumb(thumb,av[i]);
                                    LibRaw::dcraw_clear_mem(thumb);
                                }
                            else
                                fprintf(stderr,"Cannot unpack thumbnail of %s to memory buffer: %s\n" , av[i],libraw_strerror(ret));
                        }

                }
                    
            RawProcessor.recycle(); // just for show this call
        }
    return 0;
}
