# nomacs cmake file for a windows build

# load pathes from the user file if exists
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/CMakeUser.txt)
    include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeUser.txt)
endif()

if(ENABLE_PLUGINS)
    add_definitions(-DWITH_PLUGINS)
endif()

# add exiv2
find_package(PkgConfig)
pkg_check_modules(EXIV2 REQUIRED exiv2>=0.27)

if(ENABLE_OPENCV)
    find_package(OpenCV REQUIRED core imgproc)

    if(NOT OpenCV_FOUND)
        message(FATAL_ERROR "OpenCV not found.")
    else()
        add_definitions(-DWITH_OPENCV)
    endif()
endif(ENABLE_OPENCV)

# search for libraw
if(ENABLE_RAW)
    pkg_check_modules(LIBRAW libraw>=0.22.1)
    if(NOT LIBRAW_FOUND)
        message(FATAL_ERROR "libraw not found. It's mandatory when used with ENABLE_RAW enabled")
    else()
        add_definitions(-DWITH_LIBRAW)
    endif()
endif(ENABLE_RAW)

# search for multi-layer tiff

if(ENABLE_TIFF)
    pkg_check_modules(TIFF libtiff-4)
    if(NOT TIFF_FOUND)
        message(FATAL_ERROR "libtiff not found.")
    else()
        add_definitions(-DWITH_LIBTIFF)
    endif()
endif(ENABLE_TIFF)

# these variables need to be set before adding subdirectory with projects
set(NOMACS_BUILD_DIRECTORY ${CMAKE_BINARY_DIR})

if(ENABLE_QUAZIP)
    find_package(QuaZip-Qt6)
    if(QuaZip-Qt6_FOUND)
        message(STATUS "QuaZip-Qt6 found")
        set(QUAZIP_LIBRARIES QuaZip::QuaZip)
        add_definitions(-DWITH_QUAZIP)
    else()
        message(FATAL_ERROR "QuaZip-Qt6 was not found")
    endif()
endif(ENABLE_QUAZIP)

find_package(KDSingleApplication-qt6 REQUIRED)
add_definitions(-DWITH_KDSINGLEAPPLICATION)
