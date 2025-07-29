# nomacs cmake file for a windows build

# load pathes from the user file if exists
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/CMakeUser.txt)
    include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeUser.txt)
endif()

if(ENABLE_PLUGINS)
    add_definitions(-DWITH_PLUGINS)
endif()

# prepare build directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/libs)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Debug)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release)

# add exiv2
find_package(exiv2 REQUIRED)

# search for opencv
unset(OpenCV_LIB_DIR_DBG CACHE)
unset(OpenCV_3RDPARTY_LIB_DIR_DBG CACHE)
unset(OpenCV_3RDPARTY_LIB_DIR_OPT CACHE)
unset(OpenCV_CONFIG_PATH CACHE)
unset(OpenCV_LIB_DIR_DBG CACHE)
unset(OpenCV_LIB_DIR_OPT CACHE)
unset(OpenCV_LIBRARY_DIRS CACHE)
unset(OpenCV_DIR)
# unset(OpenCV_DIR CACHE) # maa that always set it to default!

if(ENABLE_OPENCV)
    find_package(OpenCV REQUIRED core imgproc)

    set(OpenCV_LIBRARY_DIRS
        ${OpenCV_LIBRARY_DIRS}
        ${OpenCV_LIB_DIR_DBG}
        ${OpenCV_LIB_DIR_OPT}
        ${OpenCV_DIR}/lib/${OpenCV_LIB_DIR_DBG}
        ${OpenCV_DIR}/lib/${OpenCV_LIB_DIR_OPT}
    )
    if(NOT OpenCV_FOUND)
        message(FATAL_ERROR "OpenCV not found.")
    else()
        add_definitions(-DWITH_OPENCV)
    endif()

    if(${OpenCV_VERSION} EQUAL "2.1.0")
        add_definitions(-DDISABLE_LANCZOS)
    endif()

    # unset include directories since OpenCV sets them global
    get_property(the_include_dirs DIRECTORY . PROPERTY INCLUDE_DIRECTORIES)
    list(REMOVE_ITEM the_include_dirs ${OpenCV_INCLUDE_DIRS})
    set_property(DIRECTORY . PROPERTY INCLUDE_DIRECTORIES ${the_include_dirs})
endif(ENABLE_OPENCV)

# search for libraw
if(ENABLE_RAW)
    # we need our CMAKE (https://github.com/nomacs/LibRaw) to add LibRaw like this
    find_package(libraw)
    if(LIBRAW_FOUND)
        add_definitions(-DWITH_LIBRAW)
    else()
        message(WARNING "LibRaw build directory not found...")
    endif()
endif(ENABLE_RAW)

# search for multi-layer tiff
# we try to grab the OpenCV's libtiff
unset(TIFF_BUILD_PATH CACHE)
unset(TIFF_CONFIG_DIR CACHE)
unset(TIFF_INCLUDE_DIR CACHE)

if(ENABLE_TIFF)
    if(NOT OpenCV_FOUND)
        message(FATAL_ERROR "OpenCV is mandotory when enabling TIFF. You have to enable ENABLE_OPENCV")
    endif(NOT OpenCV_FOUND)

    unset(TIFF_BUILD_PATH CACHE)
    unset(TIFF_CONFIG_DIR CACHE)

    find_path(
        TIFF_BUILD_PATH
        NAMES "Release/libtiff.lib" "Debug/libtiffd.lib"
        PATHS "${OpenCV_3RDPARTY_LIB_DIR_OPT}/../" "${OpenCV_DIR}/3rdparty/lib"
        DOC "Path to the libtiff build directory"
        NO_DEFAULT_PATH
    )
    find_path(TIFF_CONFIG_DIR NAMES "tif_config.h" HINTS "${OpenCV_DIR}/3rdparty/libtiff")

    # @stefan we need here the path to opencv/3rdparty/libtiff ... update 10.07.2013 stefan: currently not possible with the cmake of opencv
    find_path(
        TIFF_INCLUDE_DIR
        NAMES "tiffio.h"
        HINTS
            "${OpenCV_DIR}/../3rdparty/libtiff"
            "${OpenCV_DIR}/../sources/3rdparty/libtiff"
            "${OpenCV_DIR}/../../opencv/3rdparty/libtiff"
            "${OpenCV_DIR}/../opencv/3rdparty/libtiff"
    )
    set(TIFF_LIBRARIES
        optimized
        "${TIFF_BUILD_PATH}/Release/libtiff.lib;"
        optimized
        "${OpenCV_DIR}/3rdparty/lib/Release/zlib.lib"
        debug
        "${TIFF_BUILD_PATH}/Debug/libtiffd.lib"
        debug
        "${OpenCV_DIR}/3rdparty/lib/Debug/zlibd.lib"
    )

    if(NOT ${OpenCV_VERSION} LESS "3.4.0")
        set(TIFF_LIBRARIES
            optimized
            "${TIFF_BUILD_PATH}/Release/libtiff.lib;"
            optimized
            "${OpenCV_DIR}/3rdparty/lib/Release/zlib.lib"
            optimized
            "${TIFF_BUILD_PATH}/Release/libjpeg-turbo.lib;"
            debug
            "${TIFF_BUILD_PATH}/Debug/libtiffd.lib"
            debug
            "${OpenCV_DIR}/3rdparty/lib/Debug/zlibd.lib"
            debug
            "${TIFF_BUILD_PATH}/Debug/libjpeg-turbod.lib"
        )
    else()
        # TODO: remove - just for flo
        message("OpenCV Version is 3.3 or older: ${OpenCV_VERSION}")
    endif()

    if(TIFF_LIBRARIES AND EXISTS ${TIFF_CONFIG_DIR} AND EXISTS ${TIFF_INCLUDE_DIR})
        add_definitions(-DWITH_LIBTIFF)
    elseif(NOT EXISTS ${TIFF_CONFIG_DIR})
        message(
            FATAL_ERROR
            "TIFF_CONFIG_DIR dir not found. Needs path which contains Release/libtiff.lib and debug/libtiffd.lib (usually in the OpenCV_Build_Directory/libs). Note: tif_config.h is only available if you have compiled OpenCV on yourself. If you want to use the precompiled version you have to disable TIFF"
        )
    elseif(NOT EXISTS ${TIFF_INCLUDE_DIR})
        message(STATUS "looking for opencv in: ${OPENCV_DIR}")
        message(
            FATAL_ERROR
            "UPDATE TIFF_INCLUDE_DIR not found. Needs path which contains tif_config.h. Usually located in OpenCV Source directory ./3rdparty/libtiff"
        )
    endif()
endif(ENABLE_TIFF)

# these variables need to be set before adding subdirectory with projects
set(NOMACS_BUILD_DIRECTORY ${CMAKE_BINARY_DIR})

if(ENABLE_QUAZIP)
    add_definitions(-DWITH_QUAZIP)
    include_directories(
        "${PROJECT_SOURCE_DIR}/../3rd-party/quazip/"
        "${PROJECT_SOURCE_DIR}/../3rd-party/opencv/3rdparty/zlib"
    )
    set(QUAZIP_LIBRARIES "${DEPENDENCY_PATH}/quazip/quazip/Release/quazip1-qt${QT_VERSION_MAJOR}.lib")
endif(ENABLE_QUAZIP)
