/*!
  @file    timegm.h
  @brief   Declaration of timegm(). The implementation is in localtime.c
  @version $Rev: 1800 $
*/
#ifndef TIMEGM_H_
#define TIMEGM_H_

#include <time.h>

/*
 The following comments are copied from the Makefile of the tz distribution,
 available at ftp://elsie.nci.nih.gov/pub/:

 NIST-PCTS:151-2, Version 1.4, (1993-12-03) is a test suite put
 out by the National Institute of Standards and Technology
 which claims to test C and Posix conformance.  If you want to pass PCTS, add
       -DPCTS
 to the end of the "CFLAGS=" line.

 If your system has a "zone abbreviation" field in its "struct tm"s
 (or if you decide to add such a field in your system's "time.h" file),
 add the name to a define such as
	-DTM_ZONE=tm_zone
 or
	-DTM_ZONE=_tm_zone
 to the end of the "CFLAGS=" line.

 If you want functions that were inspired by early versions of X3J11's work,
 add
       -DSTD_INSPIRED
 to the end of the "CFLAGS=" line.

 If you want to allocate state structures in localtime, add
       -DALL_STATE
 to the end of the "CFLAGS=" line.  Storage is obtained by calling malloc.

 If you want Source Code Control System ID's left out of object modules, add
	-DNOID

 Add the following to the end of the "CFLAGS=" line as needed.
  -DTZDEFRULESTRING=\",date/time,date/time\" to default to the specified
	DST transitions if the time zone files cannot be accessed

 If you want to use System V compatibility code, add
	-DUSG_COMPAT
 to the end of the "CFLAGS=" line.  This arrange for "timezone" and "daylight"
 variables to be kept up-to-date by the time conversion functions.  Neither
 "timezone" nor "daylight" is described in X3J11's work.

 If you want an "altzone" variable (a la System V Release 3.1), add
	-DALTZONE
 to the end of the "CFLAGS=" line.
 This variable is not described in X3J11's work.

 If your system has a "GMT offset" field in its "struct tm"s
 (or if you decide to add such a field in your system's "time.h" file),
 add the name to a define such as
	-DTM_GMTOFF=tm_gmtoff
 or
	-DTM_GMTOFF=_tm_gmtoff
 to the end of the "CFLAGS=" line.
 Neither tm_gmtoff nor _tm_gmtoff is described in X3J11's work;
 in its work, use of "tm_gmtoff" is described as non-conforming.
 Both Linux and BSD have done the equivalent of defining TM_GMTOFF in
 their recent releases.

 If you want a "gtime" function (a la MACH), add
	-DCMUCS
 to the end of the "CFLAGS=" line
 This function is not described in X3J11's work.
*/

#define STD_INSPIRED
#define NOID

#ifdef  __cplusplus
extern "C" {
#endif

// The UTC version of mktime
/* rmills - timegm is replaced with _mkgmtime on VC 2005 and up */
/*        - see localtime.c                                     */
#if !defined(_MSC_VER) || (_MSC_VER < 1400)
time_t timegm(struct tm * const tmp);
#else
#define timegm _mkgmtime
#endif

#ifdef  __cplusplus
}
#endif

#endif                                  // #ifndef TIMEGM_H_
