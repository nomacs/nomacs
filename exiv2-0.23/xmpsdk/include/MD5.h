#ifndef __MD5_h__
#define __MD5_h__

/*
 * This is the header file for the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 *
 * To compute the message digest of a chunk of bytes, declare an
 * MD5_CTX structure, pass it to MD5Init, call MD5Update as
 * needed on buffers full of bytes, and then call MD5Final, which
 * will fill a supplied 16-byte array with the digest.
 *
 * Changed so as no longer to depend on Colin Plumb's `usual.h'
 * header definitions; now uses stuff from dpkg's config.h
 *  - Ian Jackson <ian@chiark.greenend.org.uk>.
 * Still in the public domain.
 */

#include <sys/types.h>
#ifdef EXV_HAVE_STDINT_H
# include <stdint.h>
#endif

/* MSVC doesn't provide C99 types, but it has MS specific variants */
#ifdef _MSC_VER
typedef unsigned __int32 uint32_t;
#endif

typedef unsigned char md5byte;
typedef uint32_t UWORD32;

struct MD5_CTX {
	UWORD32 buf[4];
	UWORD32 bytes[2];
	UWORD32 in[16];
};

extern void MD5Init(struct MD5_CTX *context);
extern void MD5Update(struct MD5_CTX *context, md5byte const *buf, unsigned len);
extern void MD5Final(unsigned char digest[16], struct MD5_CTX *context);
extern void MD5Transform(UWORD32 buf[4], UWORD32 const in[16]);

#endif
