# CMake build system for exiv2 library and executables
# Copyright 2012-     Robin Mills      <robin@clanmills.com>
# Copyright 2010-2012 Gilles Caulier   <caulier dot gilles at gmail dot com>
# Copyright 2008      Patrick Spendrin <ps_ml@gmx.de>

# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if ( NOT MSVC )
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

# Private headers which are only needed for the library itself
SET( LIBEXIV2_PRIVATE_HDR canonmn_int.hpp
                          cr2image_int.hpp
                          crwimage_int.hpp
                          fujimn_int.hpp
                          makernote_int.hpp
                          minoltamn_int.hpp
                          nikonmn_int.hpp
                          olympusmn_int.hpp
                          orfimage_int.hpp
                          panasonicmn_int.hpp
                          pentaxmn_int.hpp
                          pngchunk_int.hpp
                          rcsid_int.hpp
                          rw2image_int.hpp
                          samsungmn_int.hpp
                          sigmamn_int.hpp
                          sonymn_int.hpp
                          tags_int.hpp
                          tiffcomposite_int.hpp
                          tifffwd_int.hpp
                          tiffimage_int.hpp
                          tiffvisitor_int.hpp
   )

# Add standalone C++ header files to this list
SET( LIBEXIV2_HDR         asfvideo.hpp
                          basicio.hpp
                          bmpimage.hpp
                          convert.hpp
                          cr2image.hpp
                          crwimage.hpp
                          datasets.hpp
                          easyaccess.hpp
                          epsimage.hpp
                          error.hpp
                          exif.hpp
                          exiv2.hpp
                          futils.hpp
                          gifimage.hpp
                          image.hpp
                          iptc.hpp
                          jp2image.hpp
                          jpgimage.hpp
                          matroskavideo.hpp
                          metadatum.hpp
                          mrwimage.hpp
                          orfimage.hpp
                          pgfimage.hpp
                          preview.hpp
                          properties.hpp
                          psdimage.hpp
                          quicktimevideo.hpp
                          rafimage.hpp
                          riffvideo.hpp
                          rw2image.hpp
                          tags.hpp
                          tgaimage.hpp
                          tiffimage.hpp
                          types.hpp
                          value.hpp
                          version.hpp
                          xmp.hpp
                          xmpsidecar.hpp
   )

# Add library C++ source files to this list
SET( LIBEXIV2_SRC         asfvideo.cpp
                          basicio.cpp
                          bmpimage.cpp
                          canonmn.cpp
                          convert.cpp
                          cr2image.cpp
                          crwedit.cpp
                          crwimage.cpp
                          datasets.cpp
                          easyaccess.cpp
                          epsimage.cpp
                          error.cpp
                          exif.cpp
                          futils.cpp
                          fujimn.cpp
                          gifimage.cpp
                          image.cpp
                          iptc.cpp
                          jp2image.cpp
                          jpgimage.cpp
                          makernote.cpp
                          matroskavideo.cpp
                          metadatum.cpp
                          minoltamn.cpp
                          mrwimage.cpp
                          nikonmn.cpp
                          olympusmn.cpp
                          orfimage.cpp
                          panasonicmn.cpp
                          pentaxmn.cpp
                          pgfimage.cpp
                          preview.cpp
                          properties.cpp
                          psdimage.cpp
                          quicktimevideo.cpp
                          rafimage.cpp
                          riffvideo.cpp
                          rw2image.cpp
                          samsungmn.cpp
                          sigmamn.cpp
                          sonymn.cpp
                          tags.cpp
                          tgaimage.cpp
                          tiffcomposite.cpp
                          tiffimage.cpp
                          tiffvisitor.cpp
                          types.cpp
                          value.cpp
                          version.cpp
                          xmp.cpp
                          xmpsidecar.cpp
   )

##
# add png*.cpp files if PNG support requested
IF( EXIV2_ENABLE_PNG )
    IF( ZLIB_FOUND )
        SET( LIBEXIV2_SRC ${LIBEXIV2_SRC}
                          pngchunk.cpp
                          pngimage.cpp
           )
        set( LIBEXIV2_HDR ${LIBEXIV2_HDR} pngimage.hpp )
    ENDIF( ZLIB_FOUND )
ENDIF( EXIV2_ENABLE_PNG )

##
# add xmp source if xmp is requested and xmp library not built
IF( EXIV2_ENABLE_XMP )
    IF( NOT EXIV2_ENABLE_LIBXMP )
        SET( LIBEXIV2_SRC ${XMPSRC} ${LIBEXIV2_SRC} )
    ENDIF( NOT EXIV2_ENABLE_LIBXMP )
ENDIF( EXIV2_ENABLE_XMP )

##
# Create source file lists for applications
# exiv2 application
SET( EXIV2_SRC            exiv2.cpp
                          actions.cpp
                          utils.cpp
)

##
# modify source lists to suit environment
IF(NOT HAVE_TIMEGM )
    SET( LIBEXIV2_SRC     ${LIBEXIV2_SRC} localtime.c    )
    SET( EXIV2_SRC        ${EXIV2_SRC}    localtime.c    )
    SET( PATHTEST_SRC     ${PATHTEST_SRC} localtime.c    )
ENDIF( NOT HAVE_TIMEGM )

IF( MSVC )
    SET( MC_SRC           ${MC_SRC}       getopt_win32.c )
    SET( EXIV2_SRC        ${EXIV2_SRC}    getopt_win32.c )
    SET( LIBEXIV2_SRC     ${LIBEXIV2_SRC} getopt_win32.c )
    SET( PATHTEST_SRC     ${PATHTEST_SRC} getopt_win32.c )
ENDIF( MSVC )

##
# msvn tuning
include(../CMake_msvc.txt)
msvc_runtime_configure(${EXIV2_ENABLE_SHARED})

# ******************************************************************************
# exiv2lib library

ADD_LIBRARY( exiv2lib ${STATIC_FLAG} ${LIBEXIV2_SRC} ${LIBEXIV2_HDR} )

SET_TARGET_PROPERTIES( exiv2lib PROPERTIES
    VERSION       ${GENERIC_LIB_VERSION}
    SOVERSION     ${GENERIC_LIB_SOVERSION}
    DEFINE_SYMBOL EXV_BUILDING_LIB
    OUTPUT_NAME   exiv2
)

if ( MSVC ) 
    source_group("Header Files" FILES ${LIBEXIV2_HDR} )
    TARGET_LINK_LIBRARIES( exiv2lib ${EXPAT_LIBRARIES} )
else()
    if ( NOT MINGW )
        TARGET_LINK_LIBRARIES( exiv2lib dl ${EXPAT_LIBRARIES} )
    else()
        TARGET_LINK_LIBRARIES( exiv2lib    ${EXPAT_LIBRARIES} )
    endif()
endif()

if( EXIV2_ENABLE_LIBXMP )
    ADD_DEPENDENCIES( exiv2lib xmp )
    LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH}/$(ConfigurationName))
    TARGET_LINK_LIBRARIES( exiv2lib xmp )
ENDIF()

IF( EXIV2_ENABLE_PNG )
    IF( ZLIB_FOUND )
        IF( MSVC )
            if ( EXIV2_ENABLE_SHARED )      
                TARGET_LINK_LIBRARIES( exiv2lib optimized ${ZLIB_LIBRARIES} debug ${ZLIB_LIBRARIES}d )
            else()
                TARGET_LINK_LIBRARIES( exiv2lib optimized zlibstatic.lib debug zlibstaticd.lib )
            endif()
        ELSE()
            TARGET_LINK_LIBRARIES( exiv2lib ${ZLIB_LIBRARIES} )
        ENDIF()
    ENDIF()
ENDIF()

IF( EXIV2_ENABLE_NLS )
    TARGET_LINK_LIBRARIES( exiv2lib ${LIBINTL_LIBRARIES} )
ENDIF( EXIV2_ENABLE_NLS )

IF( ICONV_FOUND )
    TARGET_LINK_LIBRARIES( exiv2lib ${ICONV_LIBRARIES} )
ENDIF( ICONV_FOUND )

IF (CYGWIN OR MINGW)
    TARGET_LINK_LIBRARIES( exiv2lib psapi )
ENDIF(CYGWIN OR MINGW)


##
# copy header files (TODO: rewrite for NMake)
IF (MSVC )
    if (OUT_OF_SOURCE) 
        ADD_CUSTOM_COMMAND(
          TARGET exiv2lib
          PRE_BUILD
          COMMAND set S=$(SolutionDir)..\\src\nset I=$(SolutionDir)include\nset E=%I%\\exiv2\nif NOT exist %I% mkdir %I%\nif NOT exist %E% mkdir %E%\ncopy /y %S%\\*.h %E%\ncopy /y %S%\\*.hpp %E%
        )
    else()
        ADD_CUSTOM_COMMAND(
          TARGET exiv2lib
          PRE_BUILD
          COMMAND set S=$(SolutionDir)src\nset I=$(SolutionDir)include\nset E=%I%\\exiv2\nif NOT exist %I% mkdir %I%\nif NOT exist %E% mkdir %E%\ncopy /y %S%\\*.h %E%\ncopy /y %S%\\*.hpp %E%
        )
    endif()
ENDIF()

##
# copy zlib and expat dlls to output directory (TODO: rewrite for NMake)
IF ( MSVC AND EXIV2_ENABLE_SHARED )
    get_filename_component(EXPAT_LIBRARY_DIR ${EXPAT_LIBRARY} PATH)
    string(REGEX REPLACE "/" "\\\\" EXPAT_LIBRARY_DIR ${EXPAT_LIBRARY_DIR})
    get_filename_component(ZLIB_LIBRARY_DIR ${ZLIB_LIBRARY} PATH)
    string(REGEX REPLACE "/" "\\\\" ZLIB_LIBRARY_DIR ${ZLIB_LIBRARY_DIR})
    ADD_CUSTOM_COMMAND(
      TARGET exiv2lib
      POST_BUILD
      COMMAND copy /y ${EXPAT_LIBRARY_DIR}\\*.dll $(SolutionDir)\\bin\\${P_DIR}\\${T_DIR}\\$(ConfigurationName)
    )
    ADD_CUSTOM_COMMAND(
      TARGET exiv2lib
      POST_BUILD
      COMMAND copy /y ${ZLIB_LIBRARY_DIR}\\*.dll $(SolutionDir)\\bin\\${P_DIR}\\${T_DIR}\\$(ConfigurationName)
    )
ENDIF()
INSTALL( TARGETS exiv2lib ${INSTALL_TARGET_STANDARD_ARGS} )

# ******************************************************************************
# exiv2 application
ADD_EXECUTABLE( exiv2 ${EXIV2_SRC} )
TARGET_LINK_LIBRARIES( exiv2 exiv2lib )
INSTALL( TARGETS exiv2 ${INSTALL_TARGET_STANDARD_ARGS} )

# ******************************************************************************
# Headers
INSTALL( FILES ${LIBEXIV2_HDR} DESTINATION include/exiv2 )

# ******************************************************************************
# Man page
INSTALL( FILES exiv2.1 DESTINATION man/man1 )

# That's all Folks!
##
