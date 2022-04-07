# nomacs cmake file for a mac build

if(CMAKE_BUILD_TYPE STREQUAL "debug" OR CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "DEBUG")
	message(STATUS "A debug build. -DDEBUG is defined")
	add_definitions(-DDEBUG)
else()
	message(STATUS "A release build (non-debug). Debugging outputs are silently ignored.")
	add_definitions(-DQT_NO_DEBUG_OUTPUT)
	add_definitions(-DNDEBUG)
endif()

# search for pkgConfig, needed for exvi2, libraw, and older OpenCV versions
find_package(PkgConfig)

# search for exiv2
pkg_check_modules(EXIV2 REQUIRED exiv2>=0.27)
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
	set(OpenCV_LIBS "")
	set(OpenCV_FOUND false)
	# if(PKG_CONFIG_FOUND) # not sure: pkgconfig is needed for old linux  with old old opencv systems
	# 	pkg_check_modules(OpenCV  opencv>=2.1.0)
	# 	set(OpenCV_LIBS ${OpenCV_LIBRARIES})
	# endif(PKG_CONFIG_FOUND)

	if(OpenCV_LIBS STREQUAL "")
		find_package(OpenCV REQUIRED core imgproc)
	endif(OpenCV_LIBS STREQUAL "")

	if(NOT OpenCV_FOUND)
		message(FATAL_ERROR "OpenCV not found.")
	else()
		add_definitions(-DWITH_OPENCV)
	endif()

	# OpenCV has really draconian dependencies on mac (generating bundle greater tha 105MB)
	# so I play bad games here - expecting all will work with manually updated library names
	message(STATUS "")
	message(STATUS "APPLE: some hacks with OpenCV libraries linking will be performed (only limited set of libs will be used)")
	message(STATUS "APPLE:    originals: ${OpenCV_LIBS}")
	set(OpenCV_LIBS "opencv_core;opencv_imgproc")
	message(STATUS "APPLE:          new: ${OpenCV_LIBS}")
	message(STATUS "")

	if(${OpenCV_VERSION} EQUAL "2.1.0")
		add_definitions(-DDISABLE_LANCZOS)
	endif()

endif(ENABLE_OPENCV)


# search for libraw
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
		SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

		find_package(QuaZip5 REQUIRED)
        if(NOT QUAZIP_FOUND)
	    	message(FATAL_ERROR "QUAZIP not found. It's mandatory when used with ENABLE_QUAZIP enabled, you can also disable USE_SYSTEM_QUAZIP")
	    else()
    		add_definitions(-DWITH_QUAZIP)
	    endif()
    else()
		find_package(ZLIB REQUIRED)
		set(QUAZIP_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/quazip-0.7/quazip ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/quazip-0.7/)

		file(GLOB QUAZIP_SOURCES "3rdparty/quazip-0.7/quazip/*.c" "3rdparty/quazip-0.7/quazip/*.cpp")
		file(GLOB QUAZIP_HEADERS "3rdparty/quazip-0.7/quazip/*.h")

		add_definitions(-DWITH_QUAZIP)
	endif(USE_SYSTEM_QUAZIP)
endif(ENABLE_QUAZIP)


# add libqpsd
file(GLOB LIBQPSD_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libqpsd/*.cpp")
file(GLOB LIBQPSD_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libqpsd/*.h")
