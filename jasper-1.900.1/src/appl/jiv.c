/*
 * Copyright (c) 2002-2003 Michael David Adams.
 * All rights reserved.
 */

/* __START_OF_JASPER_LICENSE__
 * 
 * JasPer License Version 2.0
 * 
 * Copyright (c) 2001-2006 Michael David Adams
 * Copyright (c) 1999-2000 Image Power, Inc.
 * Copyright (c) 1999-2000 The University of British Columbia
 * 
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person (the
 * "User") obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 * 
 * 1.  The above copyright notices and this permission notice (which
 * includes the disclaimer below) shall be included in all copies or
 * substantial portions of the Software.
 * 
 * 2.  The name of a copyright holder shall not be used to endorse or
 * promote products derived from the Software without specific prior
 * written permission.
 * 
 * THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL PART OF THIS
 * LICENSE.  NO USE OF THE SOFTWARE IS AUTHORIZED HEREUNDER EXCEPT UNDER
 * THIS DISCLAIMER.  THE SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.  IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
 * INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.  NO ASSURANCES ARE
 * PROVIDED BY THE COPYRIGHT HOLDERS THAT THE SOFTWARE DOES NOT INFRINGE
 * THE PATENT OR OTHER INTELLECTUAL PROPERTY RIGHTS OF ANY OTHER ENTITY.
 * EACH COPYRIGHT HOLDER DISCLAIMS ANY LIABILITY TO THE USER FOR CLAIMS
 * BROUGHT BY ANY OTHER ENTITY BASED ON INFRINGEMENT OF INTELLECTUAL
 * PROPERTY RIGHTS OR OTHERWISE.  AS A CONDITION TO EXERCISING THE RIGHTS
 * GRANTED HEREUNDER, EACH USER HEREBY ASSUMES SOLE RESPONSIBILITY TO SECURE
 * ANY OTHER INTELLECTUAL PROPERTY RIGHTS NEEDED, IF ANY.  THE SOFTWARE
 * IS NOT FAULT-TOLERANT AND IS NOT INTENDED FOR USE IN MISSION-CRITICAL
 * SYSTEMS, SUCH AS THOSE USED IN THE OPERATION OF NUCLEAR FACILITIES,
 * AIRCRAFT NAVIGATION OR COMMUNICATION SYSTEMS, AIR TRAFFIC CONTROL
 * SYSTEMS, DIRECT LIFE SUPPORT MACHINES, OR WEAPONS SYSTEMS, IN WHICH
 * THE FAILURE OF THE SOFTWARE OR SYSTEM COULD LEAD DIRECTLY TO DEATH,
 * PERSONAL INJURY, OR SEVERE PHYSICAL OR ENVIRONMENTAL DAMAGE ("HIGH
 * RISK ACTIVITIES").  THE COPYRIGHT HOLDERS SPECIFICALLY DISCLAIM ANY
 * EXPRESS OR IMPLIED WARRANTY OF FITNESS FOR HIGH RISK ACTIVITIES.
 * 
 * __END_OF_JASPER_LICENSE__
 */

/******************************************************************************\
* Includes
\******************************************************************************/

#include <jasper/jasper.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>

/******************************************************************************\
*
\******************************************************************************/

#define MAXCMPTS	256
#define BIGPANAMOUNT	0.90
#define SMALLPANAMOUNT	0.05
#define	BIGZOOMAMOUNT	2.0
#define	SMALLZOOMAMOUNT	1.41421356237310

#define	min(x, y)	(((x) < (y)) ? (x) : (y))
#define	max(x, y)	(((x) > (y)) ? (x) : (y))

typedef struct {

	/* The number of image files to view. */
	int numfiles;

	/* The names of the image files. */
	char **filenames;

	/* The title for the window. */
	char *title;

	/* The time to wait before advancing to the next image (in ms). */
	int tmout;

	/* Loop indefinitely over all images. */
	int loop;

	int verbose;

} cmdopts_t;

typedef struct {

	int width;

	int height;

	GLshort *data;

} pixmap_t;

typedef struct {

	/* The index of the current image file. */
	int filenum;

	/* The image. */
	jas_image_t *image;
	jas_image_t *altimage;

	float botleftx;
	float botlefty;
	float toprightx;
	float toprighty;

	int viewportwidth;
	int viewportheight;

	/* The image for display. */
	pixmap_t vp;

	/* The active timer ID. */
	int activetmid;

	/* The next available timer ID. */
	int nexttmid;

	int monomode;

	int cmptno;

} gs_t;

/******************************************************************************\
*
\******************************************************************************/

static void displayfunc(void);
static void reshapefunc(int w, int h);
static void keyboardfunc(unsigned char key, int x, int y);
static void specialfunc(int key, int x, int y);
static void timerfunc(int value);

static void usage(void);
static void nextimage(void);
static void previmage(void);
static void nextcmpt(void);
static void prevcmpt(void);
static int loadimage(void);
static void unloadimage(void);
static int jas_image_render2(jas_image_t *image, int cmptno, float vtlx, float vtly,
  float vsx, float vsy, int vw, int vh, GLshort *vdata);
static int jas_image_render(jas_image_t *image, float vtlx, float vtly,
  float vsx, float vsy, int vw, int vh, GLshort *vdata);

static void dumpstate(void);
static int pixmap_resize(pixmap_t *p, int w, int h);
static void pixmap_clear(pixmap_t *p);
static void cmdinfo(void);

static void cleanupandexit(int);
static void init(void);

static void zoom(float sx, float sy);
static void pan(float dx, float dy);
static void panzoom(float dx, float dy, float sx, float sy);
static void render(void);

/******************************************************************************\
*
\******************************************************************************/

jas_opt_t opts[] = {
	{'V', "version", 0},
	{'v', "v", 0},
	{'h', "help", 0},
	{'w', "wait", JAS_OPT_HASARG},
	{'l', "loop", 0},
	{'t', "title", JAS_OPT_HASARG},
	{-1, 0, 0}
};

char *cmdname = 0;
cmdopts_t cmdopts;
gs_t gs;
jas_stream_t *streamin = 0;

/******************************************************************************\
*
\******************************************************************************/

int main(int argc, char **argv)
{
	int c;

	init();

	/* Determine the base name of this command. */
	if ((cmdname = strrchr(argv[0], '/'))) {
		++cmdname;
	} else {
		cmdname = argv[0];
	}

	/* Initialize the JasPer library. */
	if (jas_init()) {
		abort();
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutCreateWindow(cmdname);
	glutReshapeFunc(reshapefunc);
	glutDisplayFunc(displayfunc);
	glutSpecialFunc(specialfunc);
	glutKeyboardFunc(keyboardfunc);

	cmdopts.numfiles = 0;
	cmdopts.filenames = 0;
	cmdopts.title = 0;
	cmdopts.tmout = 0;
	cmdopts.loop = 0;
	cmdopts.verbose = 0;

	while ((c = jas_getopt(argc, argv, opts)) != EOF) {
		switch (c) {
		case 'w':
			cmdopts.tmout = atof(jas_optarg) * 1000;
			break;
		case 'l':
			cmdopts.loop = 1;
			break;
		case 't':
			cmdopts.title = jas_optarg;
			break;
		case 'v':
			cmdopts.verbose = 1;
			break;
		case 'V':
			printf("%s\n", JAS_VERSION);
			fprintf(stderr, "libjasper %s\n", jas_getversion());
			cleanupandexit(EXIT_SUCCESS);
			break;
		default:
		case 'h':
			usage();
			break;
		}
	}

	if (jas_optind < argc) {
		/* The images are to be read from one or more explicitly named
		  files. */
		cmdopts.numfiles = argc - jas_optind;
		cmdopts.filenames = &argv[jas_optind];
	} else {
		/* The images are to be read from standard input. */
		static char *null = 0;
		cmdopts.filenames = &null;
		cmdopts.numfiles = 1;
	}

	streamin = jas_stream_fdopen(0, "rb");

	/* Load the next image. */
	nextimage();

	/* Start the GLUT main event handler loop. */
	glutMainLoop();

	return EXIT_SUCCESS;
}

/******************************************************************************\
*
\******************************************************************************/

static void cmdinfo()
{
	fprintf(stderr, "JasPer Image Viewer (Version %s).\n",
	  JAS_VERSION);
	fprintf(stderr, "Copyright (c) 2002-2003 Michael David Adams.\n"
	  "All rights reserved.\n");
	fprintf(stderr, "%s\n", JAS_NOTES);
}

static char *helpinfo[] = {
"The following options are supported:\n",
"    --help                  Print this help information and exit.\n",
"    --version               Print version information and exit.\n",
"    --loop                  Loop indefinitely through images.\n",
"    --wait N                Advance to next image after N seconds.\n",
0
};

static void usage()
{
	char *s;
	int i;
	cmdinfo();
	fprintf(stderr, "usage: %s [options] [file1 file2 ...]\n", cmdname);
	for (i = 0, s = helpinfo[i]; s; ++i, s = helpinfo[i]) {
		fprintf(stderr, "%s", s);
	}
	cleanupandexit(EXIT_FAILURE);
}

/******************************************************************************\
* GLUT Callback Functions
\******************************************************************************/

/* Display callback function. */

static void displayfunc()
{

	float w;
	float h;
	int regbotleftx;
	int regbotlefty;
	int regtoprightx;
	int regtoprighty;
	int regtoprightwidth;
	int regtoprightheight;
	int regwidth;
	int regheight;
	float x;
	float y;
	float xx;
	float yy;

	if (cmdopts.verbose) {
		fprintf(stderr, "displayfunc()\n");
	}

	regbotleftx = max(ceil(gs.botleftx), 0);
	regbotlefty = max(ceil(gs.botlefty), 0);
	regtoprightx = min(gs.vp.width, floor(gs.toprightx));
	regtoprighty = min(gs.vp.height, floor(gs.toprighty));
	regwidth = regtoprightx - regbotleftx;
	regheight = regtoprighty - regbotlefty;
	w = gs.toprightx - gs.botleftx;
	h = gs.toprighty - gs.botlefty;
	x = (regbotleftx - gs.botleftx) / w;
	y = (regbotlefty - gs.botlefty) / h;
	xx = (regtoprightx - gs.botleftx) / w;
	yy = (regtoprighty - gs.botlefty) / h;

	assert(regwidth > 0);
	assert(regheight > 0);
	assert(abs(((double) regheight / regwidth) - ((double) gs.viewportheight / gs.viewportwidth)) < 1e-5);

	glClear(GL_COLOR_BUFFER_BIT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, sizeof(GLshort));
	glPixelStorei(GL_UNPACK_ROW_LENGTH, gs.vp.width);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, regbotleftx);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, regbotlefty);
	glRasterPos2f(x * gs.viewportwidth, y * gs.viewportheight);
	glPixelZoom((xx - x) * ((double) gs.viewportwidth) / regwidth, (yy - y) * ((double) gs.viewportheight) / regheight);
	glDrawPixels(regwidth, regheight, GL_RGBA, GL_UNSIGNED_SHORT,
	  gs.vp.data);
	glFlush();
	glutSwapBuffers();

}

/* Reshape callback function. */

static void reshapefunc(int w, int h)
{
	if (cmdopts.verbose) {
		fprintf(stderr, "reshapefunc(%d, %d)\n", w, h);
		dumpstate();
	}

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, 0);
	glRasterPos2i(0, 0);

	zoom((double) gs.viewportwidth / w, (double) gs.viewportheight / h);
	gs.viewportwidth = w;
	gs.viewportheight = h;

}

/* Keyboard callback function. */

static void keyboardfunc(unsigned char key, int x, int y)
{
	if (cmdopts.verbose) {
		fprintf(stderr, "keyboardfunc(%d, %d, %d)\n", key, x, y);
	}

	switch (key) {
	case ' ':
		nextimage();
		break;
	case '\b':
		previmage();
		break;
	case '>':
		zoom(BIGZOOMAMOUNT, BIGZOOMAMOUNT);
		glutPostRedisplay();
		break;
	case '.':
		zoom(SMALLZOOMAMOUNT, SMALLZOOMAMOUNT);
		glutPostRedisplay();
		break;
	case '<':
		zoom(1.0 / BIGZOOMAMOUNT, 1.0 / BIGZOOMAMOUNT);
		glutPostRedisplay();
		break;
	case ',':
		zoom(1.0 / SMALLZOOMAMOUNT, 1.0 / SMALLZOOMAMOUNT);
		glutPostRedisplay();
		break;
	case 'c':
		nextcmpt();
		break;
	case 'C':
		prevcmpt();
		break;
	case 'h':
		fprintf(stderr, "h             help\n");
		fprintf(stderr, ">             zoom in (large)\n");
		fprintf(stderr, ",             zoom in (small)\n");
		fprintf(stderr, "<             zoom out (large)\n");
		fprintf(stderr, ".             zoom out (small)\n");
		fprintf(stderr, "down arrow    pan down\n");
		fprintf(stderr, "up arrow      pan up\n");
		fprintf(stderr, "left arrow    pan left\n");
		fprintf(stderr, "right arrow   pan right\n");
		fprintf(stderr, "space         next image\n");
		fprintf(stderr, "backspace     previous image\n");
		fprintf(stderr, "q             quit\n");
		break;
	case 'q':
		cleanupandexit(EXIT_SUCCESS);
		break;
	}
}

/* Special keyboard callback function. */

static void specialfunc(int key, int x, int y)
{
	if (cmdopts.verbose) {
		fprintf(stderr, "specialfunc(%d, %d, %d)\n", key, x, y);
	}

	switch (key) {
	case GLUT_KEY_UP:
		{
			float panamount;
			panamount = (glutGetModifiers() & GLUT_ACTIVE_SHIFT) ?
			  BIGPANAMOUNT : SMALLPANAMOUNT;
			pan(0.0, panamount * (gs.toprighty - gs.botlefty));
			glutPostRedisplay();
		}
		break;
	case GLUT_KEY_DOWN:
		{
			float panamount;
			panamount = (glutGetModifiers() & GLUT_ACTIVE_SHIFT) ?
			  BIGPANAMOUNT : SMALLPANAMOUNT;
			pan(0.0, -panamount * (gs.toprighty - gs.botlefty));
			glutPostRedisplay();
		}
		break;
	case GLUT_KEY_LEFT:
		{
			float panamount;
			panamount = (glutGetModifiers() & GLUT_ACTIVE_SHIFT) ?
			  BIGPANAMOUNT : SMALLPANAMOUNT;
			pan(-panamount * (gs.toprightx - gs.botleftx), 0.0);
			glutPostRedisplay();
		}
		break;
	case GLUT_KEY_RIGHT:
		{
			float panamount;
			panamount = (glutGetModifiers() & GLUT_ACTIVE_SHIFT) ?
			  BIGPANAMOUNT : SMALLPANAMOUNT;
			pan(panamount * (gs.toprightx - gs.botleftx), 0.0);
			glutPostRedisplay();
		}
		break;
	default:
		break;
	}
}

/* Timer callback function. */

static void timerfunc(int value)
{
	if (cmdopts.verbose) {
		fprintf(stderr, "timerfunc(%d)\n", value);
	}
	if (value == gs.activetmid) {
		nextimage();
	}
}

/******************************************************************************\
*
\******************************************************************************/

static void zoom(float sx, float sy)
{
	panzoom(0, 0, sx, sy);
}

static void pan(float dx, float dy)
{
	panzoom(dx, dy, 1.0, 1.0);
}

static void panzoom(float dx, float dy, float sx, float sy)
{
	float w;
	float h;
	float cx;
	float cy;
	int reginh;
	int reginv;

	reginh = (gs.botleftx >= 0 && gs.toprightx <= gs.vp.width);
	reginv = (gs.botlefty >= 0 && gs.toprighty <= gs.vp.height);

	if (cmdopts.verbose) {
		fprintf(stderr, "start of panzoom\n");
		dumpstate();
		fprintf(stderr, "reginh=%d reginv=%d\n", reginh, reginv);
	}

	if (dx || dy) {
		gs.botleftx += dx;
		gs.botlefty += dy;
		gs.toprightx += dx;
		gs.toprighty += dy;
	}

	if (sx != 1.0 || sy != 1.0) {
		cx = (gs.botleftx + gs.toprightx) / 2.0;
		cy = (gs.botlefty + gs.toprighty) / 2.0;
		w = gs.toprightx - gs.botleftx;
		h = gs.toprighty - gs.botlefty;
		gs.botleftx = cx - 0.5 * w / sx;
		gs.botlefty = cy - 0.5 * h / sy;
		gs.toprightx = cx + 0.5 * w / sx;
		gs.toprighty = cy + 0.5 * h / sy;
	}

	if (reginh) {
		if (gs.botleftx < 0) {
			dx = -gs.botleftx;
			gs.botleftx += dx;
			gs.toprightx += dx;
		} else if (gs.toprightx > gs.vp.width) {
			dx = gs.vp.width - gs.toprightx;
			gs.botleftx += dx;
			gs.toprightx += dx;
		}
	}
	if (gs.botleftx < 0 || gs.toprightx > gs.vp.width) {
		float w;
		w = gs.toprightx - gs.botleftx;
		gs.botleftx = 0.5 * gs.vp.width - 0.5 * w;
		gs.toprightx = 0.5 * gs.vp.width + 0.5 * w;
	}

	if (reginv) {
		if (gs.botlefty < 0) {
			dy = -gs.botlefty;
			gs.botlefty += dy;
			gs.toprighty += dy;
		} else if (gs.toprighty > gs.vp.height) {
			dy = gs.vp.height - gs.toprighty;
			gs.botlefty += dy;
			gs.toprighty += dy;
		}
	}
	if (gs.botlefty < 0 || gs.toprighty > gs.vp.height) {
		float h;
		h = gs.toprighty - gs.botlefty;
		gs.botlefty = 0.5 * gs.vp.height - 0.5 * h;
		gs.toprighty = 0.5 * gs.vp.height + 0.5 * h;
	}

	if (cmdopts.verbose) {
		fprintf(stderr, "end of panzoom\n");
		dumpstate();
	}
}

static void nextcmpt()
{
	if (gs.monomode) {
		if (gs.cmptno == jas_image_numcmpts(gs.image) - 1) {
			if (gs.altimage) {
				gs.monomode = 0;
			} else {
				gs.cmptno = 0;
			}
		} else {
			++gs.cmptno;
		}
	} else {
		gs.monomode = 1;
		gs.cmptno = 0;
	}
	render();
	glutPostRedisplay();
}

static void prevcmpt()
{
	if (gs.monomode) {
		if (!gs.cmptno) {
			gs.monomode = 0;
		} else {
			--gs.cmptno;
		}
	} else {
		gs.monomode = 1;
		gs.cmptno = jas_image_numcmpts(gs.image) - 1;
	}
	render();
	glutPostRedisplay();
}

static void nextimage()
{
	int n;
	unloadimage();
	for (n = cmdopts.numfiles; n > 0; --n) {
		++gs.filenum;
		if (gs.filenum >= cmdopts.numfiles) {
			if (cmdopts.loop) {
				gs.filenum = 0;
			} else {
				cleanupandexit(EXIT_SUCCESS);
			}
		}
		if (!loadimage()) {
			return;
		}
		fprintf(stderr, "cannot load image\n");
	}
	cleanupandexit(EXIT_SUCCESS);
}

static void previmage()
{
	int n;
	unloadimage();
	for (n = cmdopts.numfiles; n > 0; --n) {
		--gs.filenum;
		if (gs.filenum < 0) {
			if (cmdopts.loop) {
				gs.filenum = cmdopts.numfiles - 1;
			} else {
				cleanupandexit(EXIT_SUCCESS);
			}
		}
		if (!loadimage()) {
			return;
		}
	}
	cleanupandexit(EXIT_SUCCESS);
}

static int loadimage()
{
	int reshapeflag;
	jas_stream_t *in;
	int scrnwidth;
	int scrnheight;
	int vh;
	int vw;
	char *pathname;
	jas_cmprof_t *outprof;

	assert(!gs.image);
	assert(!gs.altimage);

	gs.image = 0;
	gs.altimage = 0;

	pathname = cmdopts.filenames[gs.filenum];

	if (pathname && pathname[0] != '\0') {
		if (cmdopts.verbose) {
			fprintf(stderr, "opening file %s\n", pathname);
		}
		/* The input image is to be read from a file. */
		if (!(in = jas_stream_fopen(pathname, "rb"))) {
			fprintf(stderr, "error: cannot open file %s\n", pathname);
			goto error;
		}
	} else {
		/* The input image is to be read from standard input. */
		in = streamin;
	}

	if (cmdopts.verbose) {
		fprintf(stderr, "decoding image\n");
	}

	/* Get the input image data. */
	if (!(gs.image = jas_image_decode(in, -1, 0))) {
		fprintf(stderr, "error: cannot load image data\n");
		goto error;
	}

	/* Close the input stream. */
	if (in != streamin) {
		jas_stream_close(in);
	}

	if (cmdopts.verbose) {
		fprintf(stderr, "creating color profile\n");
	}

	if (!(outprof = jas_cmprof_createfromclrspc(JAS_CLRSPC_SRGB)))
		goto error;
	if (!(gs.altimage = jas_image_chclrspc(gs.image, outprof, JAS_CMXFORM_INTENT_PER)))
		goto error;

	vw = jas_image_width(gs.image);
	vh = jas_image_height(gs.image);

	gs.botleftx = jas_image_tlx(gs.image);
	gs.botlefty = jas_image_tly(gs.image);
	gs.toprightx = jas_image_brx(gs.image);
	gs.toprighty = jas_image_bry(gs.image);
	if (gs.altimage) {
		gs.monomode = 0;
	} else {
		gs.monomode = 1;
		gs.cmptno = 0;
	}


	if (cmdopts.verbose) {
		fprintf(stderr, "num of components %d\n", jas_image_numcmpts(gs.image));
		fprintf(stderr, "dimensions %d %d\n", jas_image_width(gs.image), jas_image_height(gs.image));
	}

	gs.viewportwidth = vw;
	gs.viewportheight = vh;
	pixmap_resize(&gs.vp, vw, vh);
	if (cmdopts.verbose) {
		fprintf(stderr, "preparing image for viewing\n");
	}
	render();
	if (cmdopts.verbose) {
		fprintf(stderr, "done preparing image for viewing\n");
	}

	if (vw != glutGet(GLUT_WINDOW_WIDTH) ||
	  vh != glutGet(GLUT_WINDOW_HEIGHT)) {
		glutReshapeWindow(vw, vh);
	}
	if (cmdopts.title) {
		glutSetWindowTitle(cmdopts.title);
	} else {
		glutSetWindowTitle((pathname && pathname[0] != '\0') ? pathname :
		  "stdin");
	}
	/* If we reshaped the window, GLUT will automatically invoke both
	  the reshape and display callback (in this order).  Therefore, we
	  only need to explicitly force the display callback to be invoked
	  if the window was not reshaped. */
	glutPostRedisplay();

	if (cmdopts.tmout != 0) {
		glutTimerFunc(cmdopts.tmout, timerfunc, gs.nexttmid);
		gs.activetmid = gs.nexttmid;
		++gs.nexttmid;
	}

	return 0;

error:
	unloadimage();
	return -1;
}

static void unloadimage()
{
	if (gs.image) {
		jas_image_destroy(gs.image);
		gs.image = 0;
	}
	if (gs.altimage) {
		jas_image_destroy(gs.altimage);
		gs.altimage = 0;
	}
}

/******************************************************************************\
*
\******************************************************************************/

static void pixmap_clear(pixmap_t *p)
{
	memset(p->data, 0, 4 * p->width * p->height * sizeof(GLshort));
}

static int pixmap_resize(pixmap_t *p, int w, int h)
{
	p->width = w;
	p->height = h;
	if (!(p->data = realloc(p->data, w * h * 4 * sizeof(GLshort)))) {
		return -1;
	}
	return 0;
}

static void dumpstate()
{
	printf("blx=%f bly=%f trx=%f try=%f\n", gs.botleftx, gs.botlefty, gs.toprightx, gs.toprighty);
}

#define	vctocc(i, co, cs, vo, vs) \
  (((vo) + (i) * (vs) - (co)) / (cs))

static int jas_image_render(jas_image_t *image, float vtlx, float vtly,
  float vsx, float vsy, int vw, int vh, GLshort *vdata)
{
	int i;
	int j;
	int k;
	int x;
	int y;
	int v[3];
	GLshort *vdatap;
	int cmptlut[3];
	int width;
	int height;
	int hs;
	int vs;
	int tlx;
	int tly;

	if ((cmptlut[0] = jas_image_getcmptbytype(image,
	  JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R))) < 0 ||
	  (cmptlut[1] = jas_image_getcmptbytype(image,
	  JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G))) < 0 ||
	  (cmptlut[2] = jas_image_getcmptbytype(image,
	  JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B))) < 0)
		goto error;
	width = jas_image_cmptwidth(image, cmptlut[0]);
	height = jas_image_cmptheight(image, cmptlut[0]);
	tlx = jas_image_cmpttlx(image, cmptlut[0]);
	tly = jas_image_cmpttly(image, cmptlut[0]);
	vs = jas_image_cmptvstep(image, cmptlut[0]);
	hs = jas_image_cmpthstep(image, cmptlut[0]);
	for (i = 1; i < 3; ++i) {
		if (jas_image_cmptwidth(image, cmptlut[i]) != width ||
		  jas_image_cmptheight(image, cmptlut[i]) != height)
			goto error;
	}
	for (i = 0; i < vh; ++i) {
		vdatap = &vdata[(vh - 1 - i) * (4 * vw)];
		for (j = 0; j < vw; ++j) {
			x = vctocc(j, tlx, hs, vtlx, vsx);
			y = vctocc(i, tly, vs, vtly, vsy);
			if (x >= 0 && x < width && y >= 0 && y < height) {
				for (k = 0; k < 3; ++k) {
					v[k] = jas_image_readcmptsample(image, cmptlut[k], x, y);
					v[k] <<= 16 - jas_image_cmptprec(image, cmptlut[k]);
					if (v[k] < 0) {
						v[k] = 0;
					} else if (v[k] > 65535) {
						v[k] = 65535;
					}
				}
			} else {
				v[0] = 0;
				v[1] = 0;
				v[2] = 0;
			}	
			*vdatap++ = v[0];
			*vdatap++ = v[1];
			*vdatap++ = v[2];
			*vdatap++ = 0;
		}
	}
	return 0;
error:
	return -1;
}

static int jas_image_render2(jas_image_t *image, int cmptno, float vtlx,
  float vtly, float vsx, float vsy, int vw, int vh, GLshort *vdata)
{
	int i;
	int j;
	int x;
	int y;
	int v;
	GLshort *vdatap;

	if (cmptno < 0 || cmptno >= image->numcmpts_) {
		fprintf(stderr, "bad parameter\n");
		goto error;
	}
	for (i = 0; i < vh; ++i) {
		vdatap = &vdata[(vh - 1 - i) * (4 * vw)];
		for (j = 0; j < vw; ++j) {
			x = vctocc(j, jas_image_cmpttlx(image, cmptno), jas_image_cmpthstep(image, cmptno), vtlx, vsx);
			y = vctocc(i, jas_image_cmpttly(image, cmptno), jas_image_cmptvstep(image, cmptno), vtly, vsy);
			v = (x >= 0 && x < jas_image_cmptwidth(image, cmptno) && y >=0 && y < jas_image_cmptheight(image, cmptno)) ? jas_image_readcmptsample(image, cmptno, x, y) : 0;
			v <<= 16 - jas_image_cmptprec(image, cmptno);
			if (v < 0) {
				v = 0;
			} else if (v > 65535) {
				v = 65535;
			}
			*vdatap++ = v;
			*vdatap++ = v;
			*vdatap++ = v;
			*vdatap++ = 0;
		}
	}
	return 0;
error:
	return -1;
}


static void render()
{
	float vtlx;
	float vtly;

	vtlx = gs.botleftx;
	vtly = gs.toprighty;
	if (cmdopts.verbose) {
//		fprintf(stderr, "vtlx=%f, vtly=%f, vsx=%f, vsy=%f\n",
//		  vtlx, vtly, gs.sx, gs.sy);
	}

	if (gs.monomode) {
		if (cmdopts.verbose) {
			fprintf(stderr, "component %d\n", gs.cmptno);
		}
		jas_image_render2(gs.image, gs.cmptno, 0.0, 0.0,
		  1.0, 1.0, gs.vp.width, gs.vp.height, gs.vp.data);
	} else {
		if (cmdopts.verbose) {
			fprintf(stderr, "color\n");
		}
		jas_image_render(gs.altimage, 0.0, 0.0, 1.0, 1.0,
		  gs.vp.width, gs.vp.height, gs.vp.data);
	}

}

#if 0

#define	vctocc(i, co, cs, vo, vs) \
  (((vo) + (i) * (vs) - (co)) / (cs))

static void drawview(jas_image_t *image, float vtlx, float vtly,
  float sx, float sy, pixmap_t *p)
{
	int i;
	int j;
	int k;
	int red;
	int grn;
	int blu;
	int lum;
	GLshort *datap;
	int x;
	int y;
	int *cmptlut;
	int numcmpts;
	int v[4];
	int u[4];
	int color;

	cmptlut = gs.cmptlut;
	switch (jas_image_colorspace(gs.image)) {
	case JAS_IMAGE_CS_RGB:
	case JAS_IMAGE_CS_YCBCR:
		color = 1;
		numcmpts = 3;
		break;
	case JAS_IMAGE_CS_GRAY:
	default:
		numcmpts = 1;
		color = 0;
		break;
	}

	for (i = 0; i < p->height; ++i) {
		datap = &p->data[(p->height - 1 - i) * (4 * p->width)];
		for (j = 0; j < p->width; ++j) {
			if (!gs.monomode && color) {
				for (k = 0; k < numcmpts; ++k) {
					x = vctocc(j, jas_image_cmpttlx(gs.image, cmptlut[k]), jas_image_cmpthstep(gs.image, cmptlut[k]), vtlx, sx);
					y = vctocc(i, jas_image_cmpttly(gs.image, cmptlut[k]), jas_image_cmptvstep(gs.image, cmptlut[k]), vtly, sy);
					v[k] = (x >= 0 && x < jas_image_cmptwidth(gs.image, cmptlut[k]) && y >=0 && y < jas_image_cmptheight(gs.image, cmptlut[k])) ? jas_matrix_get(gs.cmpts[cmptlut[k]], y, x) : 0;
					v[k] <<= 16 - jas_image_cmptprec(gs.image, cmptlut[k]);
				}
				switch (jas_image_colorspace(gs.image)) {
				case JAS_IMAGE_CS_RGB:
					break;
				case JAS_IMAGE_CS_YCBCR:
					u[0] = (1/1.772) * (v[0] + 1.402 * v[2]);
					u[1] = (1/1.772) * (v[0] - 0.34413 * v[1] - 0.71414 * v[2]);
					u[2] = (1/1.772) * (v[0] + 1.772 * v[1]);
					v[0] = u[0];
					v[1] = u[1];
					v[2] = u[2];
					break;
				}
			} else {
				x = vctocc(j, jas_image_cmpttlx(gs.image, gs.cmptno), jas_image_cmpthstep(gs.image, gs.cmptno), vtlx, sx);
				y = vctocc(i, jas_image_cmpttly(gs.image, gs.cmptno), jas_image_cmptvstep(gs.image, gs.cmptno), vtly, sy);
				v[0] = (x >= 0 && x < jas_image_cmptwidth(gs.image, gs.cmptno) && y >=0 && y < jas_image_cmptheight(gs.image, gs.cmptno)) ? jas_matrix_get(gs.cmpts[gs.cmptno], y, x) : 0;
				v[0] <<= 16 - jas_image_cmptprec(gs.image, gs.cmptno);
				v[1] = v[0];
				v[2] = v[0];
				v[3] = 0;
			}

for (k = 0; k < 3; ++k) {
	if (v[k] < 0) {
		v[k] = 0;
	} else if (v[k] > 65535) {
		v[k] = 65535;
	}
}

			*datap++ = v[0];
			*datap++ = v[1];
			*datap++ = v[2];
			*datap++ = 0;
		}
	}
}

#endif

static void cleanupandexit(int status)
{
	unloadimage();
	exit(status);
}

static void init()
{
	gs.filenum = -1;
	gs.image = 0;
	gs.altimage = 0;
	gs.nexttmid = 0;
	gs.vp.width = 0;
	gs.vp.height = 0;
	gs.vp.data = 0;
	gs.viewportwidth = -1;
	gs.viewportheight = -1;
}
