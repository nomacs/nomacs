# CMake build system for exiv2 library and executables
# Copyright 2012-     Robin Mills      <robin@clanmills.com>
# Copyright 2010-2012 Gilles Caulier   <caulier dot gilles at gmail dot com>
# Copyright 2008      Patrick Spendrin <ps_ml at gmx dot de>

# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

##
# Running cmake in a tree in which we have run ./configure ; make
# creates include/exiv2/exv_conf.h which conflicts with cmake's own exv_conf.h
# This causes incorrect compilation and linking errors.
#
# It's OK to delete this as it will be recreated on demand by <exiv2-dir>/Makefile
if( EXISTS "${CMAKE_SOURCE_DIR}/include/exiv2/exv_conf.h"  )
    file(REMOVE "${CMAKE_SOURCE_DIR}/include/exiv2/exv_conf.h")
endif()
##

CMAKE_MINIMUM_REQUIRED( VERSION 2.8.12 )
PROJECT( exiv2 )

if( POLICY CMP0042 )
    cmake_policy(SET CMP0042 NEW) # enable MACOSX_RPATH support
else()
    SET(CMAKE_MACOSX_RPATH 1)
endif()
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")


SET( PACKAGE_COPYRIGHT      "Andreas Huggel" )
SET( PACKAGE_BUGREPORT      "ahuggel@gmx.net" )
SET( PACKAGE                "exiv2" )
SET( PACKAGE_NAME           "exiv2" )
SET( PACKAGE_STRING         "exiv2 0.25" )
SET( PACKAGE_TARNAME        "exiv2" )
SET( PACKAGE_VERSION        "0.25" )
SET( PACKAGE_URL            "http://www.exiv2.org" )
SET( GENERIC_LIB_VERSION    "14.0.0" )
SET( GENERIC_LIB_SOVERSION  "14" )

# options and their default values
OPTION( EXIV2_ENABLE_SHARED        "Build exiv2 as a shared library (dll)"                 ON  )
OPTION( EXIV2_ENABLE_XMP           "Build with XMP metadata support"                       ON  )
OPTION( EXIV2_ENABLE_LIBXMP        "Build a static convenience Library for XMP"            ON  )
OPTION( EXIV2_ENABLE_VIDEO         "Build with video support"                              OFF )
OPTION( EXIV2_ENABLE_PNG           "Build with png support (requires libz)"                ON  )
OPTION( EXIV2_ENABLE_NLS           "Build native language support (requires gettext)"      ON  )
OPTION( EXIV2_ENABLE_PRINTUCS2     "Build with Printucs2"                                  ON  )
OPTION( EXIV2_ENABLE_LENSDATA      "Build including lens data"                             ON  )
OPTION( EXIV2_ENABLE_COMMERCIAL    "Build with the EXV_COMMERCIAL_VERSION symbol set"      OFF )
OPTION( EXIV2_ENABLE_BUILD_SAMPLES "Build the unit tests"                                  ON  )
OPTION( EXIV2_ENABLE_BUILD_PO      "Build translations files"                              OFF )
OPTION( EXIV2_ENABLE_VIDEO         "Build video support into library"                      OFF )
OPTION( EXIV2_ENABLE_WEBREADY      "Build webready support into library"                   OFF )
OPTION( EXIV2_ENABLE_CURL          "USE Libcurl for HttpIo"                                ON  )
OPTION( EXIV2_ENABLE_SSH           "USE Libssh for SshIo"                                  ON  )

IF( MINGW OR UNIX )
    IF ( CMAKE_CXX_COMPILER STREQUAL "g++" OR CMAKE_C_COMPILER STREQUAL "gcc" )
        ADD_DEFINITIONS(-Wall
                    -Wcast-align
                    -Wpointer-arith
                    -Wformat-security
                    -Wmissing-format-attribute
                    -Woverloaded-virtual
                    -W
                   )
    ENDIF()
ENDIF( MINGW OR UNIX )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/src/ ${CMAKE_SOURCE_DIR}/include/  ${CMAKE_SOURCE_DIR}/include/exiv2/)

if( MSVC )

    # cmake_policy(SET CMP0008)

    include(CMake_msvc.txt)
    msvc_runtime_report()
    msvc_runtime_configure(${EXIV2_ENABLE_SHARED})
    msvc_runtime_report()

    ADD_DEFINITIONS(-DPSAPI_VERSION=1) # to be compatible with <= WinVista (#905)

    set(E expat-2.1.0)
    set(Z zlib-1.2.7)
    set(C curl-7.39.0)
    set(C libssh-0.5.5)

    if( NOT IS_DIRECTORY ./../${E} )
        set(OUT_OF_SOURCE 1)
    endif()

    # link dependent libraries
    FIND_PACKAGE(EXPAT QUIET)
    if ( NOT EXPAT_LIBRARY  )
        set(EXPAT_LIBRARY      ${CMAKE_SOURCE_DIR}/../${E}/$(ConfigurationName)/expat)
        set(EXPAT_INCLUDE_DIR  ${CMAKE_SOURCE_DIR}/../${E}/lib)
        include_directories( ${EXPAT_INCLUDE_DIR} msvc64\\include src\\include)
    endif()

    FIND_PACKAGE(CURL)
    if ( NOT CURL_FOUND )
        set(CURL_LIBRARY       ${CMAKE_SOURCE_DIR}/../${C}/$(ConfigurationName)/curl)
        set(CURL_INCLUDE_DIR   ${CMAKE_SOURCE_DIR}/../${C})
        include_directories( ${CURL_INCLUDE_DIR})
    endif()

    # FIND_PACKAGE(SSH)
    if ( NOT SSH_FOUND )
        set(SSH_LIBRARY       ${CMAKE_SOURCE_DIR}/../${C}/$(ConfigurationName)/ssh)
        set(SSH_INCLUDE_DIR   ${CMAKE_SOURCE_DIR}/../${C})
        include_directories( ${SSH_INCLUDE_DIR})
    endif()

    FIND_PACKAGE(ZLIB QUIET)
    if ( NOT ZLIB_FOUND )
        set(ZLIB_LIBRARY       ${CMAKE_SOURCE_DIR}/../${Z}/$(ConfigurationName)/zlib)
        set(ZLIB_INCLUDE_DIR   ${CMAKE_SOURCE_DIR}/../${Z})
    endif()
    include_directories(${ZLIB_INCLUDE_DIR})

    if ( EXIV2_ENABLE_SHARED )
        add_library(${E} SHARED IMPORTED)
        add_library(${Z} SHARED IMPORTED)
    endif()

    ##
    # Set the output directory
    # x64/Win32
    if( CMAKE_SIZEOF_VOID_P MATCHES 8 )
        SET( P_DIR x64)
    else()
        set( P_DIR Win32 )
    endif()

    # DLL/static build
    if( EXIV2_ENABLE_SHARED )
        set( T_DIR Dynamic )
    else()
        set( T_DIR Static)
    endif()

    set(B_DIR ${PROJECT_BINARY_DIR}/bin/${P_DIR}/${T_DIR})
    set(CMAKE_CURRENT_BINARY_DIR ${B_DIR})
    set(EXECUTABLE_OUTPUT_PATH   ${B_DIR})
    set(LIBRARY_OUTPUT_PATH      ${B_DIR})
endif( MSVC )

IF( EXIV2_ENABLE_XMP )
    SET( HAVE_XMP_TOOLKIT ON )
    SET( XMPLIB "do" )
ENDIF( EXIV2_ENABLE_XMP )

INCLUDE( config/CMakeChecks.txt )

SET( INSTALL_TARGET_STANDARD_ARGS
     RUNTIME DESTINATION bin
     LIBRARY DESTINATION lib
     ARCHIVE DESTINATION lib
)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

##
# add sources from XMPSDK directory
# this needs to be done here to be visible to xmpsdk, src and sample sub directories
SET( XMPSRC ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/ExpatAdapter.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/MD5.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/ParseRDF.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/UnicodeConversions.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/WXMPIterator.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/WXMPMeta.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/WXMPUtils.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/XML_Node.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/XMPCore_Impl.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/XMPIterator.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/XMPMeta-GetSet.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/XMPMeta-Parse.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/XMPMeta-Serialize.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/XMPMeta.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/XMPUtils-FileInfo.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/XMPUtils.cpp
)

# the following lines need to be added to circumvent the error in xmpsdk\src\MD5.h
IF( ${EXV_HAVE_STDINT_H} )
    ADD_DEFINITIONS( -DEXV_HAVE_STDINT_H )
ENDIF( ${EXV_HAVE_STDINT_H} )

FIND_PACKAGE(Subversion QUIET)
SET(SVN_REVISION "0")
IF(SUBVERSION_FOUND)
    IF(EXISTS "${CMAKE_SOURCE_DIR}/.svn")
        Subversion_WC_INFO(${PROJECT_SOURCE_DIR} svnExiv2)
        SET(SVN_REVISION "${svnExiv2_WC_REVISION}")
    ENDIF()
ENDIF()
CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/src/svn_version.h.in
                ${CMAKE_CURRENT_SOURCE_DIR}/src/svn_version.h @ONLY)
MESSAGE(STATUS  "SVN version:  ${SVN_REVISION}")

IF( NOT MSVC )
  # Issue #722: out of source builds compiled against standard include files such as /usr/local/lib/include/exiv2
  # do not use CREATE_SYMLINK or CMAKE_CAN_SYMLINK as they don't work on CYGWIN
  EXECUTE_PROCESS( WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}           COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/src exiv2)

  IF( EXIV2_ENABLE_BUILD_SAMPLES )
    EXECUTE_PROCESS( WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/samples COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/src exiv2)
  ENDIF( EXIV2_ENABLE_BUILD_SAMPLES )

  IF( EXIV2_ENABLE_BUILD_PO )
    EXECUTE_PROCESS( WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/po      COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/src exiv2)
  ENDIF( EXIV2_ENABLE_BUILD_PO )
ENDIF()

ADD_SUBDIRECTORY( xmpsdk )
ADD_SUBDIRECTORY( src )

IF( EXIV2_ENABLE_BUILD_SAMPLES )
    ADD_SUBDIRECTORY( samples )
ENDIF( EXIV2_ENABLE_BUILD_SAMPLES )

IF( EXIV2_ENABLE_BUILD_PO )
    ADD_SUBDIRECTORY( po )
ENDIF( EXIV2_ENABLE_BUILD_PO )

##
# tests
ADD_CUSTOM_TARGET(tests  COMMAND make test   "EXIV2_BINDIR=${CMAKE_BINARY_DIR}/bin"  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test/" )
ADD_CUSTOM_TARGET(teste  COMMAND make teste  "EXIV2_BINDIR=${CMAKE_BINARY_DIR}/bin"  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test/" )
ADD_CUSTOM_TARGET(testv  COMMAND make testv  "EXIV2_BINDIR=${CMAKE_BINARY_DIR}/bin"  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test/" )
ADD_CUSTOM_TARGET(testvw COMMAND make testvw "EXIV2_BINDIR=${CMAKE_BINARY_DIR}/bin"  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test/" )
ADD_CUSTOM_TARGET(testx  COMMAND make testx  "EXIV2_BINDIR=${CMAKE_BINARY_DIR}/bin"  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test/" )

##
# http://dev.exiv2.org/boards/3/topics/1364
# effectively does a make doc on the root directory
# has to run 'make config' and './configure'
# and copy bin/taglist to <exiv2dir>/bin/taglist for use by 'make doc'
IF( MINGW OR UNIX OR APPLE)
  ADD_CUSTOM_TARGET(doc
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/doc"
    COMMAND chmod +x ./cmake_doc.sh
    COMMAND ./cmake_doc.sh "${CMAKE_BINARY_DIR}"
  )
ENDIF()

##
# support for test suite TODO: fixup for MSVC and think a bit more before deploying this
# ENABLE_TESTING()
# INCLUDE(Dart)
# ADD_TEST(tests bash -c "cd ${CMAKE_SOURCE_DIR}/test/ ; make EXIV2_BINDIR=${CMAKE_BINARY_DIR}/bin" )

# That's all Folks!
##
