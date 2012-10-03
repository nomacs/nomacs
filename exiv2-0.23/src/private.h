/*!
  @file    private.h
  @brief   This file is from the tz distribution at ftp://elsie.nci.nih.gov/pub/
  @version $Rev: 1358 $
*/
#ifndef PRIVATE_H

#define PRIVATE_H

/*
** This file is in the public domain, so clarified as of
** 1996-06-05 by Arthur David Olson (arthur_david_olson@nih.gov).
*/

/*
** This header is for use ONLY with the time conversion code.
** There is no guarantee that it will remain unchanged,
** or that it will remain at all.
** Do NOT copy it to any system include directory.
** Thank you!
*/

/*
** ID
*/

#ifndef lint
#ifndef NOID
static char	privatehid[] = "@(#)private.h	7.53";
#endif /* !defined NOID */
#endif /* !defined lint */

/* ahu: moved required preprocessor symbols to config.h */

/* ahu: disable warnings */
#ifdef _MSC_VER
// disable warning 'uses old-style declarator' C4131
#pragma warning (disable: 4131)
#endif

/*
** Nested includes
*/

#include "sys/types.h"	/* for time_t */
#include "stdio.h"
#include "errno.h"
#include "string.h"
#include "limits.h"	/* for CHAR_BIT */
#include "time.h"
#include "stdlib.h"

/* ahu: added io.h for MSVC */
#ifdef _MSC_VER
# include "io.h"
#endif

/* ahu: deleted include libintl.h */

/* ahu: deleted include sys/wait.h and WIFEXITED, WEXITSTATUS macros */

#if EXV_HAVE_UNISTD_H - 0
#include "unistd.h"	/* for F_OK and R_OK */
#endif /* EXV_HAVE_UNISTD_H - 0 */

#if !(EXV_HAVE_UNISTD_H - 0)
#ifndef F_OK
#define F_OK	0
#endif /* !defined F_OK */
#ifndef R_OK
#define R_OK	4
#endif /* !defined R_OK */
#endif /* !(EXV_HAVE_UNISTD_H - 0) */

/* Unlike <ctype.h>'s isdigit, this also works if c < 0 | c > UCHAR_MAX.  */
#define is_digit(c) ((unsigned)(c) - '0' <= 9)

/*
** Workarounds for compilers/systems.
*/

/*
** SunOS 4.1.1 cc lacks prototypes.
*/

#ifndef P
#ifdef __STDC__
#define P(x)	x
#endif /* defined __STDC__ */
#ifndef __STDC__
#define P(x)	()
#endif /* !defined __STDC__ */
#endif /* !defined P */

/*
** SunOS 4.1.1 headers lack EXIT_SUCCESS.
*/

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS	0
#endif /* !defined EXIT_SUCCESS */

/*
** SunOS 4.1.1 headers lack EXIT_FAILURE.
*/

#ifndef EXIT_FAILURE
#define EXIT_FAILURE	1
#endif /* !defined EXIT_FAILURE */

/*
** SunOS 4.1.1 headers lack FILENAME_MAX.
*/

#ifndef FILENAME_MAX

#ifndef MAXPATHLEN
#ifdef unix
#include "sys/param.h"
#endif /* defined unix */
#endif /* !defined MAXPATHLEN */

#ifdef MAXPATHLEN
#define FILENAME_MAX	MAXPATHLEN
#endif /* defined MAXPATHLEN */
#ifndef MAXPATHLEN
#define FILENAME_MAX	1024		/* Pure guesswork */
#endif /* !defined MAXPATHLEN */

#endif /* !defined FILENAME_MAX */

/* ahu: deleted unlink declaration and remove define */

/* ahu: deleted errno declaration */

/* ahu: deleted private function declarations */

/*
** Finally, some convenience items.
*/

#ifndef TRUE
#define TRUE	1
#endif /* !defined TRUE */

#ifndef FALSE
#define FALSE	0
#endif /* !defined FALSE */

#ifndef TYPE_BIT
#define TYPE_BIT(type)	(sizeof (type) * CHAR_BIT)
#endif /* !defined TYPE_BIT */

#ifndef TYPE_SIGNED
#define TYPE_SIGNED(type) (((type) -1) < 0)
#endif /* !defined TYPE_SIGNED */

#ifndef INT_STRLEN_MAXIMUM
/*
** 302 / 1000 is log10(2.0) rounded up.
** Subtract one for the sign bit if the type is signed;
** add one for integer division truncation;
** add one more for a minus sign if the type is signed.
*/
#define INT_STRLEN_MAXIMUM(type) \
    ((TYPE_BIT(type) - TYPE_SIGNED(type)) * 302 / 1000 + 1 + TYPE_SIGNED(type))
#endif /* !defined INT_STRLEN_MAXIMUM */

/*
** INITIALIZE(x)
*/

#ifndef GNUC_or_lint
#ifdef lint
#define GNUC_or_lint
#endif /* defined lint */
#ifndef lint
#ifdef __GNUC__
#define GNUC_or_lint
#endif /* defined __GNUC__ */
#endif /* !defined lint */
#endif /* !defined GNUC_or_lint */

#ifndef INITIALIZE
#ifdef GNUC_or_lint
#define INITIALIZE(x)	((x) = 0)
#endif /* defined GNUC_or_lint */
#ifndef GNUC_or_lint
#define INITIALIZE(x)
#endif /* !defined GNUC_or_lint */
#endif /* !defined INITIALIZE */

/* ahu: deleted definition of _(msgid) macro */

#ifndef TZ_DOMAIN
#define TZ_DOMAIN "tz"
#endif /* !defined TZ_DOMAIN */

#if HAVE_INCOMPATIBLE_CTIME_R
#undef asctime_r
#undef ctime_r
char *asctime_r P((struct tm const *, char *));
char *ctime_r P((time_t const *, char *));
#endif /* HAVE_INCOMPATIBLE_CTIME_R */

/*
** UNIX was a registered trademark of The Open Group in 2003.
*/

#endif /* !defined PRIVATE_H */
