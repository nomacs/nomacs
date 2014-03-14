# ===========================================================
#
# This file is a part of Libraw project
# <a href="http://www.libraw.org">http://www.libraw.org</a>
#
# @date   2013-09-07
# @brief  Library for reading and processing of RAW images
#
# @author Copyright (C) 2013 by Gilles Caulier
#         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
#
# This program is free software; you can redistribute it
# and/or modify it under the terms of the GNU General
# Public License as published by the Free Software Foundation;
# either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# ============================================================

CMAKE_MINIMUM_REQUIRED(VERSION 2.6.3)

PROJECT(libraw)

# ==================================================================================================
# Library version info extraction

FILE(READ ${CMAKE_CURRENT_SOURCE_DIR}/libraw/libraw_version.h _libraw_version_content)

# API version strings
STRING(REGEX MATCH "#define LIBRAW_MAJOR_VERSION[ \t]*([0-9]*)\n" _version_major_match ${_libraw_version_content})
SET(RAW_LIB_MAJOR_VERSION "${CMAKE_MATCH_1}")

STRING(REGEX MATCH "#define LIBRAW_MINOR_VERSION[ \t]*([0-9]*)\n" _version_minor_match ${_libraw_version_content})
SET(RAW_LIB_MINOR_VERSION "${CMAKE_MATCH_1}")

STRING(REGEX MATCH "#define LIBRAW_PATCH_VERSION[ \t]*([0-9]*)\n" _version_patch_match ${_libraw_version_content})
SET(RAW_LIB_PATCH_VERSION "${CMAKE_MATCH_1}")

# ABI version strings

STRING(REGEX MATCH "#define LIBRAW_SHLIB_CURRENT[ \t]*([0-9]*)\n" _version_socur_match ${_libraw_version_content})
SET(RAW_LIB_SO_CUR_VERSION "${CMAKE_MATCH_1}")

STRING(REGEX MATCH "#define LIBRAW_SHLIB_REVISION[ \t]*([0-9]*)\n" _version_sorev_match ${_libraw_version_content})
SET(RAW_LIB_SO_REV_VERSION "${CMAKE_MATCH_1}")

STRING(REGEX MATCH "#define LIBRAW_SHLIB_AGE[ \t]*([0-9]*)\n" _version_soage_match ${_libraw_version_content})
SET(RAW_LIB_SO_AGE_VERSION "${CMAKE_MATCH_1}")

# Set env. variables accordinly.
SET(RAW_LIB_VERSION_STRING    "${RAW_LIB_MAJOR_VERSION}.${RAW_LIB_MINOR_VERSION}.${RAW_LIB_PATCH_VERSION}")
SET(RAW_LIB_VERSION_ID        "0x${RAW_LIB_MAJOR_VERSION}${RAW_LIB_MINOR_VERSION}${RAW_LIB_PATCH_VERSION}")
SET(RAW_LIB_SO_VERSION_STRING "${RAW_LIB_SO_CUR_VERSION}.${RAW_LIB_SO_REV_VERSION}.${RAW_LIB_SO_AGE_VERSION}")

MESSAGE(STATUS "LibRaw string version: ${RAW_LIB_VERSION_STRING}")
MESSAGE(STATUS "LibRaw ID version:     ${RAW_LIB_VERSION_ID}")
MESSAGE(STATUS "LibRaw SO version:     ${RAW_LIB_SO_VERSION_STRING}")

# ==================================================================================================
# Project Options

OPTION(ENABLE_OPENMP               "Build library with OpenMP support               (default=ON)"                 ON)
OPTION(ENABLE_LCMS                 "Build library with LCMS support                 (default=ON)"                 ON)
OPTION(ENABLE_EXAMPLES             "Build library with sample command-line programs (default=ON)"                 ON)
OPTION(ENABLE_RAWSPEED             "Build library with extra RawSpeed codec support (default=OFF)"                OFF)
OPTION(ENABLE_DEMOSAIC_PACK_GPL2   "Build library with extra Demosaic pack GPL2     (default=OFF)"                OFF)
OPTION(ENABLE_DEMOSAIC_PACK_GPL3   "Build library with extra Demosaic pack GPL3     (default=OFF)"                OFF)
OPTION(ENABLE_DCRAW_DEBUG          "Build library with debug message from dcraw     (default=OFF)"                OFF)

SET(DEMOSAIC_PACK_GPL2_RPATH "LibRaw-demosaic-pack-GPL2-${RAW_LIB_VERSION_STRING}" CACHE STRING
    "Relavive path to extra Demosaic pack GPL2 (default=LibRaw-demosaic-pack-GPL2-M.m.p)")

SET(DEMOSAIC_PACK_GPL3_RPATH "LibRaw-demosaic-pack-GPL3-${RAW_LIB_VERSION_STRING}" CACHE STRING 
    "Relavive path to extra Demosaic pack GPL3 (default=LibRaw-demosaic-pack-GPL3-M.m.p)")

SET(RAWSPEED_RPATH           "RawSpeed"           CACHE STRING 
    "Relavive path to extra RawSpeed codec     (default=RawSpeed)")

SET(DEMOSAIC_PACK_GPL2_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${DEMOSAIC_PACK_GPL2_RPATH}")
SET(DEMOSAIC_PACK_GPL3_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${DEMOSAIC_PACK_GPL3_RPATH}")
SET(RAWSPEED_PATH           "${CMAKE_CURRENT_SOURCE_DIR}/${RAWSPEED_RPATH}")

# ==================================================================================================
# General definitions rules

SET(LIB_SUFFIX "" CACHE STRING "Define suffix of lib directory name (32/64)" )

# To prevent warnings from M$ compiler
IF(WIN32 AND MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
    ADD_DEFINITIONS(-D_ATL_SECURE_NO_WARNINGS)
    ADD_DEFINITIONS(-D_AFX_SECURE_NO_WARNINGS)
ENDIF()

# Under Windows, use specific flag to compile.
IF(WIN32)
    ADD_DEFINITIONS(-DDJGPP)
ENDIF()

# -- Check dependencies --------------------------------------------------------------------------------

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH} )

INCLUDE(MacroBoolTo01)
INCLUDE(MacroLogFeature)
INCLUDE(MacroOptionalFindPackage)

# Math library check

IF(NOT WIN32)
    FIND_LIBRARY(MATH_LIBRARY m)
ENDIF()

# LCMS version 1 and 2 library check

SET(LCMS_SUPPORT_CAN_BE_COMPILED false)

IF (ENABLE_LCMS)
    MESSAGE(STATUS "Check for LCMS1 availability...")
    FIND_PACKAGE(LCMS)
    IF (LCMS_FOUND)
        MESSAGE(STATUS "Found LCMS1: ${LCMS_LIBRARIES} ${LCMS_INCLUDE_DIR}")
        INCLUDE_DIRECTORIES(${LCMS_INCLUDE_DIR})
        MACRO_LOG_FEATURE(LCMS_FOUND "LCMS1" "A small-footprint color management engine" "http://www.littlecms.com" TRUE "" "Needed by libkdcraw")
        # Flag to compile Little CMS version 1 with LibRaw
        ADD_DEFINITIONS(-DUSE_LCMS)
        # For compatibility
        SET(LCMS2_LIBRARIES ${LCMS_LIBRARIES})
        SET(LCMS_SUPPORT_CAN_BE_COMPILED true)
    ELSE ()
        MESSAGE(STATUS "Check for LCMS2 availability instead LCMS1...")
        FIND_PACKAGE(LCMS2)
        IF (LCMS2_FOUND AND (LCMS2_VERSION VERSION_EQUAL 2.1 OR LCMS2_VERSION VERSION_GREATER 2.1))
            MESSAGE(STATUS "Found LCMS2 instead LCMS1: ${LCMS2_LIBRARIES} ${LCMS2_INCLUDE_DIR}")
            INCLUDE_DIRECTORIES(${LCMS2_INCLUDE_DIR})
            MACRO_LOG_FEATURE(LCMS2_FOUND "LCMS2" "A small-footprint color management engine" "http://www.littlecms.com" FALSE "" "Needed by libkdcraw")
            # Flag to compile Little CMS version 2 with LibRaw
            ADD_DEFINITIONS(-DUSE_LCMS2)
            SET(LCMS_SUPPORT_CAN_BE_COMPILED true)
        ENDIF ()
    ENDIF ()
ENDIF()

# For registration to libraw_config.h
MACRO_BOOL_TO_01(LCMS_SUPPORT_CAN_BE_COMPILED LIBRAW_USE_LCMS)

# JPEG library check

FIND_PACKAGE(JPEG)
IF(JPEG_FOUND)
    # check version of libjpeg
    # inspired from digikam/CMakeLists.txt
    SET(_jpeglib_version_source "#include <stddef.h>\n#include<stdio.h>\n#include <jpeglib.h>\n int main()\n {\n #if (JPEG_LIB_VERSION >= 80) \n #error JPEG_LIB_VERSION >= 80 found \n #endif \n }\n")
    SET(_jpeglib_version_source_file ${CMAKE_BINARY_DIR}/CMakeTmp/cmake_jpeglib_version_check.cpp)
    FILE(WRITE "${_jpeglib_version_source_file}" "${_jpeglib_version_source}")
    SET(_jpeglib_version_include_dirs "-DINCLUDE_DIRECTORIES:STRING=${JPEG_INCLUDE_DIR}")

    TRY_COMPILE(_jpeglib_version_compile_result ${CMAKE_BINARY_DIR} ${_jpeglib_version_source_file}
                CMAKE_FLAGS "${_jpeglib_version_include_dirs}"
               )
    IF(_jpeglib_version_compile_result)
        MESSAGE(STATUS "Identified libjpeg version less than 80 (too low)")
        SET(JPEG_FOUND FALSE)
    ENDIF()
ENDIF()

MACRO_LOG_FEATURE(JPEG_FOUND "libjpeg" "JPEG image format support" "http://www.ijg.org" FALSE "80" "needed for the LibRaw DNG lossy codec")

# Flag to use libjpeg with LibRaw DNG lossy codec
IF(JPEG_FOUND)
    ADD_DEFINITIONS(-DUSE_JPEG)
    ADD_DEFINITIONS(-DUSE_JPEG8)
    INCLUDE_DIRECTORIES(${JPEG_INCLUDE_DIR})
ENDIF()

# For registration to libraw_config.h
MACRO_BOOL_TO_01(JPEG_FOUND LIBRAW_USE_DNGLOSSYCODEC)

# OpenMP library check

SET(OPENMP_FOUND false)

IF(ENABLE_OPENMP)
    # OpenMP library detection (requires CMake >= 2.6.3)
    # NOTE: OpenMP under MacOSX do not support multithreading.

    IF(NOT APPLE)
        MACRO_OPTIONAL_FIND_PACKAGE(OpenMP)

        IF(OPENMP_FOUND)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
            IF("${OpenMP_CXX_FLAGS}" STREQUAL "-fopenmp")
                SET(OPENMP_LDFLAGS "-lgomp")
            ENDIF()
            IF("${OpenMP_CXX_FLAGS}" STREQUAL "-xopenmp")
                SET(OPENMP_LDFLAGS "-xopenmp")
            ENDIF()
        ENDIF()
    ENDIF()
ENDIF()

# For registration to libraw_config.h
MACRO_BOOL_TO_01(OPENMP_FOUND LIBRAW_USE_OPENMP)

# Jasper library check

FIND_PACKAGE(Jasper)

# Flag to use libjasper with LibRaw RedCine codec
IF(JASPER_FOUND)
    ADD_DEFINITIONS(-DUSE_JASPER)
    INCLUDE_DIRECTORIES(${JASPER_INCLUDE_DIR})
ENDIF()

# For registration to libraw_config.h
MACRO_BOOL_TO_01(JASPER_FOUND LIBRAW_USE_REDCINECODEC)

# For RawSpeed Codec Support

SET(RAWSPEED_FOUND false)
SET(RAWSPEED_SUPPORT_CAN_BE_COMPILED false)

IF(ENABLE_RAWSPEED)

    FIND_PACKAGE(LibXml2)
    FIND_PACKAGE(Pthreads)

    MESSAGE(STATUS "RawSpeed codec path: ${RAWSPEED_PATH}")

    IF(EXISTS "${RAWSPEED_PATH}/Common.cpp")
        SET(RAWSPEED_FOUND true)
    ELSE()
        MESSAGE(STATUS "RawSpeed source code not found. Please checkout source code from RawStudio project website.")
    ENDIF()

    IF(ENABLE_RAWSPEED AND RAWSPEED_FOUND AND JPEG_FOUND AND LIBXML2_FOUND AND PTHREADS_FOUND)

        SET(RAWSPEED_SUPPORT_CAN_BE_COMPILED true)

    ELSE()
        IF (NOT JPEG_FOUND)
            MESSAGE(STATUS "LibJPEG dependency not resolved. LibRaw cannot be compiled with RawSpeed codec")
        ENDIF()

        IF (NOT LIBXML2_FOUND)
            MESSAGE(STATUS "LibXML2 dependency not resolved. LibRaw cannot be compiled with RawSpeed codec")
        ENDIF()

        IF (NOT PTHREADS_FOUND)
            MESSAGE(STATUS "Pthreads dependency not resolved. LibRaw cannot be compiled with RawSpeed codec")
        ENDIF()

    ENDIF()
ENDIF()

# For registration to libraw_config.h
MACRO_BOOL_TO_01(RAWSPEED_SUPPORT_CAN_BE_COMPILED LIBRAW_USE_RAWSPEED)

# -- Compilation rules for RawSpeed library -------------------------------------------------------------

IF(RAWSPEED_SUPPORT_CAN_BE_COMPILED)

    INCLUDE_DIRECTORIES(${RAWSPEED_PATH})

    INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${PTHREADS_INCLUDE_DIR})

    # Flag to include RawSpeed codec with Libraw
    ADD_DEFINITIONS(-DUSE_RAWSPEED)

    ADD_DEFINITIONS(${LIBXML2_DEFINITIONS} ${PTHREADS_DEFINITIONS})

    SET(librawspeed_LIB_SRCS ${RAWSPEED_PATH}/ArwDecoder.cpp
                             ${RAWSPEED_PATH}/BitPumpJPEG.cpp
                             ${RAWSPEED_PATH}/BitPumpMSB.cpp
                             ${RAWSPEED_PATH}/BitPumpMSB32.cpp
                             ${RAWSPEED_PATH}/BitPumpPlain.cpp
                             ${RAWSPEED_PATH}/BlackArea.cpp
                             ${RAWSPEED_PATH}/ByteStream.cpp
                             ${RAWSPEED_PATH}/ByteStreamSwap.cpp
                             ${RAWSPEED_PATH}/Camera.cpp
                             ${RAWSPEED_PATH}/CameraMetaData.cpp
                             ${RAWSPEED_PATH}/CameraMetadataException.cpp
                             ${RAWSPEED_PATH}/CameraSensorInfo.cpp
                             ${RAWSPEED_PATH}/ColorFilterArray.cpp
                             ${RAWSPEED_PATH}/Common.cpp
                             ${RAWSPEED_PATH}/Cr2Decoder.cpp
                             ${RAWSPEED_PATH}/DngDecoder.cpp
                             ${RAWSPEED_PATH}/DngDecoderSlices.cpp
                             ${RAWSPEED_PATH}/DngOpcodes.cpp
                             ${RAWSPEED_PATH}/FileIOException.cpp
                             ${RAWSPEED_PATH}/FileMap.cpp
                             ${RAWSPEED_PATH}/IOException.cpp
                             ${RAWSPEED_PATH}/LJpegDecompressor.cpp
                             ${RAWSPEED_PATH}/LJpegPlain.cpp
                             ${RAWSPEED_PATH}/NefDecoder.cpp
                             ${RAWSPEED_PATH}/NikonDecompressor.cpp
                             ${RAWSPEED_PATH}/OrfDecoder.cpp
                             ${RAWSPEED_PATH}/PefDecoder.cpp
                             ${RAWSPEED_PATH}/PentaxDecompressor.cpp
                             ${RAWSPEED_PATH}/RawDecoder.cpp
                             ${RAWSPEED_PATH}/RawDecoderException.cpp
                             ${RAWSPEED_PATH}/RawImage.cpp
                             ${RAWSPEED_PATH}/RawImageDataFloat.cpp
                             ${RAWSPEED_PATH}/RawImageDataU16.cpp
                             ${RAWSPEED_PATH}/RawParser.cpp
                             ${RAWSPEED_PATH}/Rw2Decoder.cpp
                             ${RAWSPEED_PATH}/SrwDecoder.cpp
                             ${RAWSPEED_PATH}/TiffEntry.cpp
                             ${RAWSPEED_PATH}/TiffEntryBE.cpp
                             ${RAWSPEED_PATH}/TiffIFD.cpp
                             ${RAWSPEED_PATH}/TiffIFDBE.cpp
                             ${RAWSPEED_PATH}/TiffParser.cpp
                             ${RAWSPEED_PATH}/TiffParserException.cpp
                             ${RAWSPEED_PATH}/TiffParserHeaderless.cpp
                             ${RAWSPEED_PATH}/TiffParserOlympus.cpp
    )

ENDIF()

# -- Common LibRaw library compilation rules ------------------------------------------------------------------

SET(DEMOSAIC_PACK_GPL2_SUPPORT_CAN_BE_COMPILED false)

IF(ENABLE_DEMOSAIC_PACK_GPL2)
    MESSAGE(STATUS "Demosaic pack GPL2 path: ${DEMOSAIC_PACK_GPL2_PATH}")

    IF(EXISTS "${DEMOSAIC_PACK_GPL2_PATH}/dcraw_foveon.c")
        INCLUDE_DIRECTORIES(${DEMOSAIC_PACK_GPL2_PATH})
        # Flag to include demosaic pack GPL2
        ADD_DEFINITIONS(-DLIBRAW_DEMOSAIC_PACK_GPL2)
        SET(DEMOSAIC_PACK_GPL2_SUPPORT_CAN_BE_COMPILED true)
    ELSE()
        MESSAGE(STATUS "Demosaic pack GPL2 source code not found. Please checkout source code from Libraw project website.")
    ENDIF()
ENDIF()

# For registration to libraw_config.h
MACRO_BOOL_TO_01(DEMOSAIC_PACK_GPL2_SUPPORT_CAN_BE_COMPILED LIBRAW_USE_DEMOSAIC_PACK_GPL2)

SET(DEMOSAIC_PACK_GPL3_SUPPORT_CAN_BE_COMPILED false)

IF(ENABLE_DEMOSAIC_PACK_GPL3)
    MESSAGE(STATUS "Demosaic pack GPL3 path: ${DEMOSAIC_PACK_GPL3_PATH}")

    IF(EXISTS "${DEMOSAIC_PACK_GPL3_PATH}/green_equi.c")
        INCLUDE_DIRECTORIES(${DEMOSAIC_PACK_GPL3_PATH})
        # Flag to include demosaic pack GPL3
        ADD_DEFINITIONS(-DLIBRAW_DEMOSAIC_PACK_GPL3)
        SET(DEMOSAIC_PACK_GPL3_SUPPORT_CAN_BE_COMPILED true)
    ELSE()
        MESSAGE(STATUS "Demosaic pack GPL3 source code not found. Please checkout source code from Libraw project website.")
    ENDIF()
ENDIF()

# For registration to libraw_config.h
MACRO_BOOL_TO_01(DEMOSAIC_PACK_GPL3_SUPPORT_CAN_BE_COMPILED LIBRAW_USE_DEMOSAIC_PACK_GPL3)

# Flag to add debug print on the console 
IF(ENABLE_DCRAW_DEBUG)
    ADD_DEFINITIONS(-DDCRAW_VERBOSE)
ENDIF()

# For registration to libraw_config.h
MACRO_BOOL_TO_01(ENABLE_DCRAW_DEBUG LIBRAW_USE_DCRAW_DEBUG)

# Flag to export library symbols
IF (WIN32)
    ADD_DEFINITIONS(-DLIBRAW_BUILDLIB)
ENDIF()

# Create a config header for client application.
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/data/libraw_config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/libraw_config.h)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/
                    ${CMAKE_CURRENT_SOURCE_DIR}/
                   )

# -- Log messages about configuration ------------------------------------------------------------------

MESSAGE(STATUS "")
MESSAGE(STATUS "----------------------------------------------------------------------------------")
MESSAGE(STATUS " Libraw ${RAW_LIB_VERSION_STRING} configuration            <http://www.libraw.org>")
MESSAGE(STATUS "")

IF(OPENMP_FOUND)
    MESSAGE(STATUS " Libraw will be compiled with OpenMP support .................. YES")
ELSE()
    MESSAGE(STATUS " Libraw will be compiled with OpenMP support .................. NO")
ENDIF()

IF(LCMS_SUPPORT_CAN_BE_COMPILED)
    MESSAGE(STATUS " Libraw will be compiled with LCMS support .................... YES")
ELSE()
    MESSAGE(STATUS " Libraw will be compiled with LCMS support .................... NO")
ENDIF()

IF(ENABLE_EXAMPLES)
    MESSAGE(STATUS " Libraw will be compiled with example command-line programs ... YES")
ELSE()
    MESSAGE(STATUS " Libraw will be compiled with example command-line programs ... NO")
ENDIF()

IF(JASPER_FOUND)
    MESSAGE(STATUS " Libraw will be compiled with RedCine codec support ........... YES")
ELSE()
    MESSAGE(STATUS " Libraw will be compiled with RedCine codec support ........... NO")
ENDIF()

IF(JPEG_FOUND)
    MESSAGE(STATUS " Libraw will be compiled with DNG lossy codec support ......... YES")
ELSE()
    MESSAGE(STATUS " Libraw will be compiled with DNG lossy codec support ......... NO")
ENDIF()

IF(DEMOSAIC_PACK_GPL2_SUPPORT_CAN_BE_COMPILED)
    MESSAGE(STATUS " Libraw will be compiled with Demosaic Pack GPL2 support ...... YES")
ELSE()
    MESSAGE(STATUS " Libraw will be compiled with Demosaic Pack GPL2 support ...... NO")
ENDIF()

IF(DEMOSAIC_PACK_GPL3_SUPPORT_CAN_BE_COMPILED)
    MESSAGE(STATUS " Libraw will be compiled with Demosaic Pack GPL3 support ...... YES")
ELSE()
    MESSAGE(STATUS " Libraw will be compiled with Demosaic Pack GPL3 support ...... NO")
ENDIF()

IF(RAWSPEED_SUPPORT_CAN_BE_COMPILED)
    MESSAGE(STATUS " Libraw will be compiled with RawSpeed support ................ YES")
ELSE()
    MESSAGE(STATUS " Libraw will be compiled with RawSpeed support ................ NO")
ENDIF()

IF(ENABLE_DCRAW_DEBUG)
    MESSAGE(STATUS " Libraw will be compiled with debug message from dcraw ........ YES")
ELSE()
    MESSAGE(STATUS " Libraw will be compiled with debug message from dcraw ........ NO")
ENDIF()

MESSAGE(STATUS "----------------------------------------------------------------------------------")
MESSAGE(STATUS "")

# -- Dedicated libraw target which not support multi-threading ---------------------------------------

SET(libraw_LIB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/internal/dcraw_common.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/internal/dcraw_fileio.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/internal/demosaic_packs.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/libraw_cxx.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/libraw_c_api.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/libraw_datastream.cpp
   )

IF(RAWSPEED_SUPPORT_CAN_BE_COMPILED)
    SET(libraw_LIB_SRCS ${libraw_LIB_SRCS} ${librawspeed_LIB_SRCS})
ENDIF()

# Disable compilation warnings from LibRaw. Just to be clear on the console.
# Add flag to not support re-entrancy. Faster but cannot be used in multi-threading.
# Adjust flag for static lib and 64 bits computers using -fPIC for GCC compiler
# Use O4 GCC compilation option to prevent artifacts with OpenMP
FOREACH(_curentfile ${libraw_LIB_SRCS})
    IF(WIN32 AND MSVC)
        SET_SOURCE_FILES_PROPERTIES(${_curentfile} PROPERTIES COMPILE_FLAGS "-w -DLIBRAW_NOTHREADS")
    ELSE()
        SET_SOURCE_FILES_PROPERTIES(${_curentfile} PROPERTIES COMPILE_FLAGS "-w -DLIBRAW_NOTHREADS -fPIC -O4 ")
    ENDIF()
ENDFOREACH(_curentfile ${libraw_LIB_SRCS})

ADD_LIBRARY(raw SHARED ${libraw_LIB_SRCS})

TARGET_LINK_LIBRARIES(raw ${MATH_LIBRARY})

IF(MINGW)
    TARGET_LINK_LIBRARIES(raw ws2_32)
ENDIF()

IF(OPENMP_FOUND)
    TARGET_LINK_LIBRARIES(raw ${OPENMP_LDFLAGS})
ENDIF()

IF(LCMS_SUPPORT_CAN_BE_COMPILED)
    TARGET_LINK_LIBRARIES(raw ${LCMS2_LIBRARIES})
ENDIF()

IF(JPEG_FOUND)
    TARGET_LINK_LIBRARIES(raw ${JPEG_LIBRARY})
ENDIF()

IF(JASPER_FOUND)
    TARGET_LINK_LIBRARIES(raw ${JASPER_LIBRARIES})
ENDIF()

IF(RAWSPEED_SUPPORT_CAN_BE_COMPILED)
    TARGET_LINK_LIBRARIES(raw ${LIBXML2_LIBRARIES})
ENDIF()

SET_TARGET_PROPERTIES(raw PROPERTIES VERSION     ${RAW_LIB_SO_VERSION_STRING})
SET_TARGET_PROPERTIES(raw PROPERTIES SOVERSION   ${RAW_LIB_SO_CUR_VERSION})
SET_TARGET_PROPERTIES(raw PROPERTIES OUTPUT_NAME "raw")

# -- Dedicated libraw target to support multi-threading ---------------------------------------------

SET(libraw_r_LIB_SRCS ${libraw_LIB_SRCS})

FOREACH(_curentfile ${libraw_r_LIB_SRCS})
    IF(WIN32 AND MSVC)
        SET_SOURCE_FILES_PROPERTIES(${_curentfile} PROPERTIES COMPILE_FLAGS "-w")
    ELSE()
        SET_SOURCE_FILES_PROPERTIES(${_curentfile} PROPERTIES COMPILE_FLAGS "-w -fPIC -O4")
    ENDIF()
ENDFOREACH(_curentfile ${libraw_r_LIB_SRCS})

ADD_LIBRARY(raw_r SHARED ${libraw_r_LIB_SRCS})

TARGET_LINK_LIBRARIES(raw_r ${MATH_LIBRARY})

IF(MINGW)
    TARGET_LINK_LIBRARIES(raw_r ws2_32)
ENDIF()

IF(OPENMP_FOUND)
    TARGET_LINK_LIBRARIES(raw_r ${OPENMP_LDFLAGS})
ENDIF()

IF(LCMS_SUPPORT_CAN_BE_COMPILED)
    TARGET_LINK_LIBRARIES(raw_r ${LCMS2_LIBRARIES})
ENDIF()

IF(JPEG_FOUND)
    TARGET_LINK_LIBRARIES(raw_r ${JPEG_LIBRARY})
ENDIF()

IF(JASPER_FOUND)
    TARGET_LINK_LIBRARIES(raw_r ${JASPER_LIBRARIES})
ENDIF()

IF(RAWSPEED_SUPPORT_CAN_BE_COMPILED)
    TARGET_LINK_LIBRARIES(raw_r ${LIBXML2_LIBRARIES} ${PTHREADS_LIBRARY})
ENDIF()

SET_TARGET_PROPERTIES(raw_r PROPERTIES VERSION     ${RAW_LIB_SO_VERSION_STRING})
SET_TARGET_PROPERTIES(raw_r PROPERTIES SOVERSION   ${RAW_LIB_SO_CUR_VERSION})
SET_TARGET_PROPERTIES(raw_r PROPERTIES OUTPUT_NAME "raw_r")

# -- Files to install -------------------------------------------------------------------------------------

# Configure and install data file for packaging.
IF(NOT WIN32)
    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/data/libraw.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libraw.pc @ONLY)
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/libraw.pc DESTINATION lib${LIB_SUFFIX}/pkgconfig)

    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/data/libraw_r.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libraw_r.pc @ONLY)
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/libraw_r.pc DESTINATION lib${LIB_SUFFIX}/pkgconfig)

    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/data/libraw.lsm.cmake ${CMAKE_CURRENT_BINARY_DIR}/libraw.lsm)
ENDIF()

# Install Shared header files.
INSTALL(FILES       ${CMAKE_CURRENT_SOURCE_DIR}/libraw/libraw.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/libraw/libraw_alloc.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/libraw/libraw_const.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/libraw/libraw_datastream.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/libraw/libraw_internal.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/libraw/libraw_types.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/libraw/libraw_version.h
                    ${CMAKE_CURRENT_BINARY_DIR}/libraw_config.h
        DESTINATION include/libraw
        COMPONENT   Devel
       )

# Install Shared binary files.
INSTALL(TARGETS raw
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib${LIB_SUFFIX}
        ARCHIVE DESTINATION lib${LIB_SUFFIX}
       )

INSTALL(TARGETS raw_r
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib${LIB_SUFFIX}
        ARCHIVE DESTINATION lib${LIB_SUFFIX}
       )

# Install find cmake script to the system for client applications.
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/FindLibRaw.cmake
        DESTINATION ${CMAKE_ROOT}/Modules)

# Install doc data files.
IF(NOT WIN32)
    INSTALL(FILES       ${CMAKE_CURRENT_SOURCE_DIR}/COPYRIGHT
                        ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.CDDL
                        ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.LGPL
                        ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.LibRaw.pdf
                        ${CMAKE_CURRENT_SOURCE_DIR}/Changelog.txt
            DESTINATION share/libraw
            COMPONENT main
           )
ENDIF()

# Uninstall rules
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/Uninstall.cmake ${CMAKE_BINARY_DIR}/cmake_uninstall.cmake COPYONLY)
ADD_CUSTOM_TARGET(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake")

# -- Compile LibRaw Examples --------------------------------------------------------------------------------

# add a small macro so that this is a bit cleaner
MACRO(LIBRAW_BUILD_SAMPLES)

    SET(_filename ${ARGV0})
    SET(_rawlib ${ARGV1})
    STRING(REPLACE "." ";" _temp ${_filename})
    LIST(GET _temp 0 _target)

    SET(${_target}_SRCS samples/${_filename})
    SET_SOURCE_FILES_PROPERTIES(${${_target}_SRCS} PROPERTIES COMPILE_FLAGS -w)

    ADD_EXECUTABLE(${_target} ${${_target}_SRCS})

    TARGET_LINK_LIBRARIES(${_target} ${_rawlib})

    IF(OPENMP_FOUND)
        TARGET_LINK_LIBRARIES(${_target} ${OPENMP_LDFLAGS})
    ENDIF()

    IF (${_rawlib} MATCHES "raw_r")
        TARGET_LINK_LIBRARIES(${_target} ${PTHREADS_LIBRARY})
    ENDIF()

    IF(WIN32)
        TARGET_LINK_LIBRARIES(${_target} ws2_32)
    ENDIF()

    INSTALL(TARGETS ${_target}
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib${LIB_SUFFIX}
            ARCHIVE DESTINATION lib${LIB_SUFFIX}
    )

ENDMACRO(LIBRAW_BUILD_SAMPLES)

IF(ENABLE_EXAMPLES)

    LIBRAW_BUILD_SAMPLES(simple_dcraw.cpp raw)
    LIBRAW_BUILD_SAMPLES(mem_image.cpp raw)
    LIBRAW_BUILD_SAMPLES(dcraw_emu.cpp raw)
    LIBRAW_BUILD_SAMPLES(4channels.cpp raw)
    LIBRAW_BUILD_SAMPLES(unprocessed_raw.cpp raw)
    LIBRAW_BUILD_SAMPLES(raw-identify.cpp raw)
    LIBRAW_BUILD_SAMPLES(multirender_test.cpp raw)
    LIBRAW_BUILD_SAMPLES(postprocessing_benchmark.cpp raw)

    IF (PTHREADS_FOUND)
        IF(WIN32)
            LIBRAW_BUILD_SAMPLES(half_mt_win32.c raw_r)
        ELSE()
            LIBRAW_BUILD_SAMPLES(dcraw_half.c raw_r)
            LIBRAW_BUILD_SAMPLES(half_mt.c raw_r)
        ENDIF()
    ENDIF()
ENDIF()
