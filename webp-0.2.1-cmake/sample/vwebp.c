// Copyright 2011 Google Inc. All Rights Reserved.
//
// This code is licensed under the same terms as WebM:
//  Software License Agreement:  http://www.webmproject.org/license/software/
//  Additional IP Rights Grant:  http://www.webmproject.org/license/additional/
// -----------------------------------------------------------------------------
//
//  Simple WebP file viewer.
//
// Compiling on linux:
//   sudo apt-get install libglut3-dev mesa-common-dev
//   gcc -o vwebp vwebp.c -O3 -lwebp -lwebpmux -lglut -lGL -lpthread -lm
// Compiling on Mac + XCode:
//   gcc -o vwebp vwebp.c -lwebp -lwebpmux -framework GLUT -framework OpenGL
//
// Author: Skal (pascal.massimino@gmail.com)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "webp/decode.h"
#include "webp/mux.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#ifdef FREEGLUT
#include <GL/freeglut.h>
#endif
#endif

#include "./example_util.h"

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

static void Help(void);

// Unfortunate global variables. Gathered into a struct for comfort.
static struct {
  int has_animation;
  int done;
  int decoding_error;
  int print_info;

  uint32_t flags;
  int loop_count;
  int frame_num;
  int frame_max;

  const char* file_name;
  WebPData data;
  WebPMux* mux;
  WebPDecoderConfig* config;
  const WebPDecBuffer* pic;
} kParams = {
  0, 0, 0, 0,         // has_animation, ...
  0, 1, 1, 0,         // flags, ...
  NULL, { NULL, 0 },  // file_name, ...
  NULL, NULL, NULL    // mux, ...
};

static void ClearPreviousPic(void) {
  WebPFreeDecBuffer((WebPDecBuffer*)kParams.pic);
  kParams.pic = NULL;
}

static void ClearParams(void) {
  ClearPreviousPic();
  WebPDataClear(&kParams.data);
  WebPMuxDelete(kParams.mux);
  kParams.mux = NULL;
}

//------------------------------------------------------------------------------
// Callbacks

static void HandleKey(unsigned char key, int pos_x, int pos_y) {
  (void)pos_x;
  (void)pos_y;
  if (key == 'q' || key == 'Q' || key == 27 /* Esc */) {
#ifdef FREEGLUT
    glutLeaveMainLoop();
#else
    ClearParams();
    exit(0);
#endif
  } else if (key == 'i') {
    kParams.print_info = 1 - kParams.print_info;
    glutPostRedisplay();
  }
}

static void HandleReshape(int width, int height) {
  // TODO(skal): proper handling of resize, esp. for large pictures.
  // + key control of the zoom.
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

static void PrintString(const char* const text) {
  void* const font = GLUT_BITMAP_9_BY_15;
  int i;
  for (i = 0; text[i]; ++i) {
    glutBitmapCharacter(font, text[i]);
  }
}

static void DrawCheckerBoard(void) {
  const int square_size = 8;  // must be a power of 2
  int x, y;
  GLint viewport[4];  // x, y, width, height

  glPushMatrix();

  glGetIntegerv(GL_VIEWPORT, viewport);
  // shift to integer coordinates with (0,0) being top-left.
  glOrtho(0, viewport[2], viewport[3], 0, -1, 1);
  for (y = 0; y < viewport[3]; y += square_size) {
    for (x = 0; x < viewport[2]; x += square_size) {
      const GLubyte color = 128 + 64 * (!((x + y) & square_size));
      glColor3ub(color, color, color);
      glRecti(x, y, x + square_size, y + square_size);
    }
  }
  glPopMatrix();
}

static void HandleDisplay(void) {
  const WebPDecBuffer* pic = kParams.pic;
  if (pic == NULL) return;
  glClear(GL_COLOR_BUFFER_BIT);
  glPushMatrix();
  glPixelZoom(1, -1);
  glRasterPos2f(-1, 1);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, pic->u.RGBA.stride / 4);
  DrawCheckerBoard();
  glDrawPixels(pic->width, pic->height,
               GL_RGBA, GL_UNSIGNED_BYTE,
               (GLvoid*)pic->u.RGBA.rgba);
  if (kParams.print_info) {
    char tmp[32];

    glColor4f(0.0, 0.0, 0.0, 1.0);
    glRasterPos2f(-0.95f, 0.90f);
    PrintString(kParams.file_name);

    snprintf(tmp, sizeof(tmp), "Dimension:%d x %d", pic->width, pic->height);
    glColor4f(0.0, 0.0, 0.0, 1.0);
    glRasterPos2f(-0.95f, 0.80f);
    PrintString(tmp);
  }
  glPopMatrix();
  glFlush();
}

static void StartDisplay(const WebPDecBuffer* const pic) {
  glutInitDisplayMode(GLUT_RGBA);
  glutInitWindowSize(pic->width, pic->height);
  glutCreateWindow("WebP viewer");
  glutDisplayFunc(HandleDisplay);
  glutIdleFunc(NULL);
  glutKeyboardFunc(HandleKey);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  HandleReshape(pic->width, pic->height);
}

//------------------------------------------------------------------------------
// File decoding

static int Decode(const int frame_number, int* const duration) {
  WebPDecoderConfig* const config = kParams.config;
  WebPData *data, image_data;
  int x_off = 0, y_off = 0;
  WebPDecBuffer* const output_buffer = &config->output;
  int ok = 0;

  ClearPreviousPic();
  if (kParams.has_animation) {
    if (WebPMuxGetFrame(kParams.mux, frame_number, &image_data,
                        &x_off, &y_off, duration) != WEBP_MUX_OK) {
      goto end;
    }
    if (x_off != 0 || y_off != 0) {
      fprintf(stderr,
              "Frame offsets not yet supported! Forcing offset to 0,0\n");
      x_off = y_off = 0;
    }
    data = &image_data;
  } else {
    data = &kParams.data;
  }

  output_buffer->colorspace = MODE_RGBA;
  ok = (WebPDecode(data->bytes_, data->size_, config) == VP8_STATUS_OK);

 end:
  if (!ok) {
    fprintf(stderr, "Decoding of frame #%d failed!\n", frame_number);
  } else {
    kParams.pic = output_buffer;
  }
  return ok;
}

static void decode_callback(int what) {
  if (what == 0 && !kParams.done) {
    int duration = 0;
    if (kParams.mux != NULL) {
      if (!Decode(kParams.frame_num, &duration)) {
        kParams.decoding_error = 1;
        kParams.done = 1;
      } else {
        ++kParams.frame_num;
        if (kParams.frame_num > kParams.frame_max) {
          kParams.frame_num = 1;
          --kParams.loop_count;
          kParams.done = (kParams.loop_count == 0);
        }
      }
    }
    glutPostRedisplay();
    glutTimerFunc(duration, decode_callback, what);
  }
}

//------------------------------------------------------------------------------
// Main

static void Help(void) {
  printf("Usage: vwebp in_file [options]\n\n"
         "Decodes the WebP image file and visualize it using OpenGL\n"
         "Options are:\n"
         "  -version  .... print version number and exit.\n"
         "  -nofancy ..... don't use the fancy YUV420 upscaler.\n"
         "  -nofilter .... disable in-loop filtering.\n"
         "  -mt .......... use multi-threading\n"
         "  -crop <x> <y> <w> <h> ... crop output with the given rectangle\n"
         "  -scale <w> <h> .......... scale the output (*after* any cropping)\n"
         "  -h     ....... this help message.\n"
        );
}

int main(int argc, char *argv[]) {
  WebPDecoderConfig config;
  WebPMuxError mux_err;
  int c;

  if (!WebPInitDecoderConfig(&config)) {
    fprintf(stderr, "Library version mismatch!\n");
    return -1;
  }
  kParams.config = &config;

  for (c = 1; c < argc; ++c) {
    if (!strcmp(argv[c], "-h") || !strcmp(argv[c], "-help")) {
      Help();
      return 0;
    } else if (!strcmp(argv[c], "-nofancy")) {
      config.options.no_fancy_upsampling = 1;
    } else if (!strcmp(argv[c], "-nofilter")) {
      config.options.bypass_filtering = 1;
    } else if (!strcmp(argv[c], "-version")) {
      const int version = WebPGetDecoderVersion();
      printf("%d.%d.%d\n",
        (version >> 16) & 0xff, (version >> 8) & 0xff, version & 0xff);
      return 0;
    } else if (!strcmp(argv[c], "-mt")) {
      config.options.use_threads = 1;
    } else if (!strcmp(argv[c], "-crop") && c < argc - 4) {
      config.options.use_cropping = 1;
      config.options.crop_left   = strtol(argv[++c], NULL, 0);
      config.options.crop_top    = strtol(argv[++c], NULL, 0);
      config.options.crop_width  = strtol(argv[++c], NULL, 0);
      config.options.crop_height = strtol(argv[++c], NULL, 0);
    } else if (!strcmp(argv[c], "-scale") && c < argc - 2) {
      config.options.use_scaling = 1;
      config.options.scaled_width  = strtol(argv[++c], NULL, 0);
      config.options.scaled_height = strtol(argv[++c], NULL, 0);
    } else if (argv[c][0] == '-') {
      printf("Unknown option '%s'\n", argv[c]);
      Help();
      return -1;
    } else {
      kParams.file_name = argv[c];
    }
  }

  if (kParams.file_name == NULL) {
    printf("missing input file!!\n");
    Help();
    return 0;
  }

  if (!ExUtilReadFile(kParams.file_name,
                      &kParams.data.bytes_, &kParams.data.size_)) {
    goto Error;
  }

  kParams.mux = WebPMuxCreate(&kParams.data, 0);
  if (kParams.mux == NULL) {
    fprintf(stderr, "Could not create demuxing object!\n");
    goto Error;
  }

  mux_err = WebPMuxGetFeatures(kParams.mux, &kParams.flags);
  if (mux_err != WEBP_MUX_OK) {
    goto Error;
  }
  if (kParams.flags & TILE_FLAG) {
    fprintf(stderr, "Tiling is not supported for now!\n");
    goto Error;
  }

  kParams.has_animation = !!(kParams.flags & ANIMATION_FLAG);

  if (kParams.has_animation) {
    mux_err = WebPMuxGetLoopCount(kParams.mux, &kParams.loop_count);
    if (mux_err != WEBP_MUX_OK && mux_err != WEBP_MUX_NOT_FOUND) {
      goto Error;
    }
    mux_err = WebPMuxNumChunks(kParams.mux, WEBP_CHUNK_IMAGE,
                                      &kParams.frame_max);
    if (mux_err != WEBP_MUX_OK) {
      goto Error;
    }
    printf("VP8X: Found %d images in file (loop count = %d)\n",
           kParams.frame_max, kParams.loop_count);
  }

  // Decode first frame
  {
    int duration;
    if (!Decode(1, &duration)) goto Error;
  }

  // Start display (and timer)
  glutInit(&argc, argv);
#ifdef FREEGLUT
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
#endif
  StartDisplay(kParams.pic);
  if (kParams.has_animation) glutTimerFunc(0, decode_callback, 0);
  glutMainLoop();

  // Should only be reached when using FREEGLUT:
  ClearParams();
  return 0;

 Error:
  ClearParams();
  return -1;
}

//------------------------------------------------------------------------------
