#ifdef   _MSC_VER
#include <winsock2.h>
#define strdup _strdup
#define close  _close
#include <stdio.h> // for snprintf (C99)
#define snprintf _snprintf
#endif

/* Name of package */
#define PACKAGE "libssh"

/* Version number of package */
#define VERSION "0.5.5"

/* #undef LOCALEDIR */
#define DATADIR "/usr/local/share/libssh"
#define LIBDIR "/usr/local/lib"
#define PLUGINDIR "/usr/local/lib/libssh-4"
#define SYSCONFDIR "/usr/local/etc"

// TODO: fix and document this in msvc2005/ReadMe.txt
#define BINARYDIR "/c/Users/rmills/gnu/libssh/build"
#define SOURCEDIR "/c/Users/rmills/gnu/libssh/libssh-0.5.5"

// Visual Studio 2012 supports ntohll
#if    _MSC_VER >= 1700
#define HAVE_NTOHLL 1
#endif


/************************** HEADER FILES *************************/

/* Define to 1 if you have the <argp.h> header file. */
#define HAVE_ARGP_H 1

/* Define to 1 if you have the <pty.h> header file. */
#define HAVE_PTY_H 1

/* Define to 1 if you have the <termios.h> header file. */
#define HAVE_TERMIOS_H 1

/* Define to 1 if you have the <openssl/aes.h> header file. */
#define HAVE_OPENSSL_AES_H 1

/* Define to 1 if you have the <wspiapi.h> header file. */
/* #undef HAVE_WSPIAPI_H */

/* Define to 1 if you have the <openssl/blowfish.h> header file. */
#define HAVE_OPENSSL_BLOWFISH_H 1

/* Define to 1 if you have the <openssl/des.h> header file. */
#define HAVE_OPENSSL_DES_H 1

/* Define to 1 if you have the <pthread.h> header file. */
//#define HAVE_PTHREAD_H 1


/*************************** FUNCTIONS ***************************/

/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF 1

/* Define to 1 if you have the `_snprintf' function. */
/* #undef HAVE__SNPRINTF */

/* Define to 1 if you have the `_snprintf_s' function. */
/* #undef HAVE__SNPRINTF_S */

/* Define to 1 if you have the `vsnprintf' function. */
#define HAVE_VSNPRINTF 1

/* Define to 1 if you have the `_vsnprintf' function. */
/* #undef HAVE__VSNPRINTF */

/* Define to 1 if you have the `_vsnprintf_s' function. */
/* #undef HAVE__VSNPRINTF_S */

/* Define to 1 if you have the `strncpy' function. */
#define HAVE_STRNCPY 1

/* Define to 1 if you have the `cfmakeraw' function. */
#define HAVE_CFMAKERAW 1

/* Define to 1 if you have the `getaddrinfo' function. */
#define HAVE_GETADDRINFO 1

/* Define to 1 if you have the `poll' function. */
//#define HAVE_POLL 1

/* Define to 1 if you have the `select' function. */
#define HAVE_SELECT 1

/* Define to 1 if you have the `regcomp' function. */
#define HAVE_REGCOMP 1

/* Define to 1 if you have the `clock_gettime' function. */
//#define HAVE_CLOCK_GETTIME 1

/* Define to 1 if you have the `ntohll' function. */
/* #undef HAVE_NTOHLL */

/*************************** LIBRARIES ***************************/

/* Define to 1 if you have the `crypto' library (-lcrypto). */
#define HAVE_LIBCRYPTO 1

/* Define to 1 if you have the `gcrypt' library (-lgcrypt). */
/* #undef HAVE_LIBGCRYPT */

/* Define to 1 if you have the `z' library (-lz). */
#define HAVE_LIBZ 1

/* Define to 1 if you have the `pthread' library (-lpthread). */
//#define HAVE_PTHREAD 1


/**************************** OPTIONS ****************************/

/* Define to 1 if you want to enable ZLIB */
#define WITH_LIBZ 1

/* Define to 1 if you want to enable SFTP */
#define WITH_SFTP 1

/* Define to 1 if you want to enable SSH1 */
/* #undef WITH_SSH1 */

/* Define to 1 if you want to enable server support */
#define WITH_SERVER 1

/* Define to 1 if you want to enable debug output for crypto functions */
/* #undef DEBUG_CRYPTO */

/* Define to 1 if you want to enable pcap output support (experimental) */
#define WITH_PCAP 1

/* Define to 1 if you want to enable calltrace debug output */
#define DEBUG_CALLTRACE 1

/*************************** ENDIAN *****************************/

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
/* #undef WORDS_BIGENDIAN */
