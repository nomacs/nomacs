# nomacs cmake file for a Unix/Linux build

if(CMAKE_BUILD_TYPE STREQUAL "debug" OR CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    message(STATUS "A debug build. -DDEBUG is defined")
    add_definitions(-DDEBUG)
else()
    message(STATUS "A release build (non-debug). Debugging outputs are silently ignored.")
    add_definitions(-DQT_NO_DEBUG_OUTPUT)
    add_definitions(-DNDEBUG)
endif()

# try to use system libraries or not
# currently disabled due to packaging problems
option(USE_SYSTEM_QUAZIP "Use quazip libary provided by system" OFF)
option(USE_SYSTEM_LIBQPSD "Use qpsd libary provided by system" OFF)


# search for pkgConfig, needed for exvi2, libraw, and older OpenCV versions
find_package(PkgConfig)

# search for exiv2
pkg_check_modules(EXIV2 REQUIRED exiv2>=0.28.1)
message(STATUS "exiv2 version by pkg-config: ${EXIV2_VERSION}")

find_file(HAVE_EXIV2_HPP exiv2/exiv2.hpp ${EXIV2_INCLUDE_DIRS})
MARK_AS_ADVANCED(HAVE_EXIV2_HPP)
if (HAVE_EXIV2_HPP)
    message(STATUS "exiv2 contains exiv2.hpp, using it")
    add_definitions(-DHAVE_EXIV2_HPP)
else (HAVE_EXIV2_HPP)
    message(STATUS "exiv2 does not contain exiv2.hpp. Using older include set")
endif (HAVE_EXIV2_HPP)

if(NOT EXIV2_FOUND)
    message(FATAL_ERROR "exiv2 library was not found")
endif(NOT EXIV2_FOUND)

# search for opencv
unset(OpenCV_FOUND CACHE)
if(ENABLE_OPENCV)
    find_package(OpenCV REQUIRED core imgproc)
    if (NOT OpenCV_LIBRARIES) # OpenCV_FOUND can not be used since it is set in Ubuntu 12.04 (without finding opencv)
        # Older OpenCV versions only supplied pkg-config files
        if(PKG_CONFIG_FOUND)
            pkg_check_modules(OpenCV opencv>=2.1.0)
        endif()
    endif()

    if(NOT OpenCV_FOUND)
        message(FATAL_ERROR "OpenCV not found, but requested.")
    endif()

    add_definitions(-DWITH_OPENCV)
    if(${OpenCV_VERSION} EQUAL "2.1.0")
        add_definitions(-DDISABLE_LANCZOS)
    endif()
endif(ENABLE_OPENCV)

# search for libraw
unset(LIBRAW_FOUND CACHE)
if(ENABLE_RAW)
    if(NOT OpenCV_FOUND)
        message(FATAL_ERROR "OpenCV is mandotory when enabling RAW. You have to enable ENABLE_OPENCV")
    endif()

    pkg_check_modules(LIBRAW  libraw>=0.12.0)
    if(NOT LIBRAW_FOUND)
        message(FATAL_ERROR "libraw not found. It's mandatory when used with ENABLE_RAW enabled")
    else()
        add_definitions(-DWITH_LIBRAW)
    endif()
endif(ENABLE_RAW)

#search for multi-layer tiff
unset(TIFF_INCLUDE_DIR CACHE)
unset(TIFF_LIBRARY CACHE)
if(ENABLE_TIFF)
    if(NOT OpenCV_FOUND)
        message(FATAL_ERROR "OpenCV is mandotory when enabling TIFF. You have to enable ENABLE_OPENCV")
    endif()
    find_package(TIFF)
    if(TIFF_FOUND)
        SET(TIFF_CONFIG_DIR "")
        add_definitions(-DWITH_LIBTIFF)
    else()
        message(FATAL_ERROR "libtiff was not found. It's mandatory when used with ENABLE_TIFF enabled.")
    endif()
endif(ENABLE_TIFF)

#search for quazip
unset(QUAZIP_SOURCE_DIRECTORY CACHE)
unset(QUAZIP_INCLUDE_DIRECTORY CACHE)
unset(QUAZIP_LIBS CACHE)
unset(QUAZIP_BUILD_DIRECTORY CACHE)
unset(QUAZIP_DEPENDENCY CACHE)
unset(QUAZIP_FOUND CACHE)
unset(QUAZIP_LIBRARIES CACHE)

unset(QUAZIP_HEADERS CACHE)
unset(QUAZIP_SOURCES CACHE)
unset(QT_ROOT CACHE)

if(ENABLE_QUAZIP)
    if(USE_SYSTEM_QUAZIP)
        # mxe quazip package uses -lquazip, has no cmake,
        # and its quazip.pc file is broken
        if (DEFINED ENV{MXE_TARGET})
            message(STATUS "QUAZIP: using mxe version")
            set(QUAZIP_INCLUDE_DIR "")
            set(QUAZIP_LIBRARIES quazip)
            add_definitions(-DWITH_QUAZIP)
            add_definitions(-DWITH_QUAZIP1)
        else()
            find_package(QuaZip-Qt5 QUIET)
            if (QuaZip-Qt5_FOUND)
                message(STATUS "QUAZIP: QuaZip-1 for Qt5 found")
                set(QUAZIP_LIBRARIES QuaZip::QuaZip)
                add_definitions(-DWITH_QUAZIP)
                add_definitions(-DWITH_QUAZIP1)
            else ()
                message(WARNING "QUAZIP: 1.x not found. Trying to find 0.x")
                # QuaZip < 1.0. This leg can be removed (including cmake/FindQuaZip5.cmake)
                # ater the final migration to 1.x in Linux distros
                SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

                find_package(QuaZip5)
                if(NOT QUAZIP_FOUND)
                    message(FATAL_ERROR "QUAZIP: even 0.x not found. It's mandatory when used with ENABLE_QUAZIP enabled, you can also disable USE_SYSTEM_QUAZIP")
                endif()

                add_definitions(-DWITH_QUAZIP)
            endif (QuaZip-Qt5_FOUND)
        endif()
    else()
        message(STATUS "QUAZIP: using 3rdparty/quazip-0.7")
        find_package(ZLIB REQUIRED)
        set(QUAZIP_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/quazip-0.7/quazip ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/quazip-0.7/)

        file(GLOB QUAZIP_SOURCES "3rdparty/quazip-0.7/quazip/*.c" "3rdparty/quazip-0.7/quazip/*.cpp")
        file(GLOB QUAZIP_HEADERS "3rdparty/quazip-0.7/quazip/*.h")

        add_definitions(-DWITH_QUAZIP)
    endif(USE_SYSTEM_QUAZIP)
endif(ENABLE_QUAZIP)

# add libqpsd
IF(USE_SYSTEM_LIBQPSD)
find_package(qpsd QUIET)
if(NOT QPSD_FOUND)
    message(FATAL_ERROR "QPSD not found. It's mandatory when used with USE_SYSTEM_LIBQPSD enabled, you can also disable USE_SYSTEM_LIBQPSD")
endif()
ELSE()
file(GLOB LIBQPSD_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libqpsd/*.cpp")
file(GLOB LIBQPSD_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libqpsd/*.h")
ENDIF(USE_SYSTEM_LIBQPSD)
