# nomacs cmake file for a Unix/Linux build

#if (ENABLE_PLUGINS)
#	message(FATAL_ERROR "Plugin system is currently only supported on windows systems")
#endif()


if(CMAKE_BUILD_TYPE STREQUAL "debug" OR CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    message(STATUS "A debug build. -DDEBUG is defined")
    add_definitions(-DDEBUG)
else()
    message(STATUS "A release build (non-debug). Debugging outputs are silently ignored.")
    add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()

# try to use system libraries or not
# currently disabled due to packaging problems
option(USE_SYSTEM_WEBP "Use webp libary provided by system" OFF)
option(USE_SYSTEM_QUAZIP "Use webp libary provided by system" OFF)
option(USE_SYSTEM_LIBQPSD "Use qpsd libary provided by system" OFF)


# search for pkgConfig, needed for exvi2, libraw, and older OpenCV versions
find_package(PkgConfig)

# search for exiv2
pkg_check_modules(EXIV2  exiv2>=0.20)
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
	find_package(OpenCV 2.1.0 REQUIRED core imgproc imgcodecs photo)
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
unset(QUAZIP_MOCS CACHE)
unset(QT_ROOT CACHE)

if(ENABLE_QUAZIP)
  if(USE_SYSTEM_QUAZIP)
    SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
    
    find_package(QuaZIP REQUIRED)
    if(NOT QUAZIP_FOUND)
	    message(FATAL_ERROR "QUAZIP not found. It's mandatory when used with ENABLE_QUAZIP enabled, you can also disable USE_SYSTEM_QUAZIP") 
    else()
	    add_definitions(-DWITH_QUAZIP)
    endif()
  else()
    find_package(ZLIB REQUIRED)
    set(QUAZIP_INCLUDE_DIRECTORY ${CMAKE_SOURCE_DIR}/3rdparty/quazip-0.7/quazip ${CMAKE_SOURCE_DIR}/3rdparty/quazip-0.7/)
    
    file(GLOB QUAZIP_SOURCES "3rdparty/quazip-0.7/quazip/*.c" "3rdparty/quazip-0.7/quazip/*.cpp")
    file(GLOB QUAZIP_HEADERS "3rdparty/quazip-0.7/quazip/*.h")
    file(GLOB QUAZIP_MOCS "3rdparty/quazip-0.7/quazip/*.h")
    
    IF (NOT ENABLE_QT5)
      QT4_WRAP_CPP(QUAZIP_MOC_SRC ${QUAZIP_MOCS})
    ELSE()
      QT5_WRAP_CPP(QUAZIP_MOC_SRC ${QUAZIP_MOCS})
    ENDIF()
    add_definitions(-DWITH_QUAZIP)
  endif(USE_SYSTEM_QUAZIP)
endif(ENABLE_QUAZIP)

# add libqpsd
IF(USE_SYSTEM_LIBQPSD)
    find_package(qpsd REQUIRED)
    if(NOT QPSD_FOUND)
	    message(FATAL_ERROR "QUAZIP not found. It's mandatory when used with ENABLE_QUAZIP enabled, you can also disable USE_SYSTEM_QUAZIP") 
    endif()
ELSE()
  file(GLOB LIBQPSD_SOURCES "3rdparty/libqpsd/*.cpp")
  file(GLOB LIBQPSD_HEADERS "3rdparty/libqpsd/*.h")
  file(GLOB LIBQPSD_MOCS "3rdparty/libqpsd/*.h")
  IF (NOT ENABLE_QT5)
   QT4_WRAP_CPP(LIBQPSD_MOC_SRC ${LIBQPSD_MOCS})
  ELSE()
   QT5_WRAP_CPP(LIBQPSD_MOC_SRC ${LIBQPSD_MOCS})
  ENDIF()
ENDIF(USE_SYSTEM_LIBQPSD)

# add webp
SET(WEBP_INCLUDEDIR "")
SET(WEBP_SOURCE "")
SET(WEBP_LIBRARIES "")
if(ENABLE_WEBP)
  if(USE_SYSTEM_WEBP)
    pkg_check_modules(WEBP  libwebp>=0.3.1)
    if(NOT WEBP_FOUND)
	    message(FATAL_ERROR "libwebp not found. It's mandatory when used with ENABLE_WEBP enabled, you can also disable USE_SYSTEM_WEBP") 
    else()
	    add_definitions(-DWITH_WEBP)
    endif()
  else()
	add_definitions(-DNDEBUG -DWEBP_USE_THREAD)

	file(GLOB WEBP_DEC_SRCS
		RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libwebp/src/dec/*c
	)
	
	file(GLOB WEBP_DEMUX_SRCS
		RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libwebp/src/demux/*c
	)

	file(GLOB WEBP_DSP_SRCS
		RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libwebp/src/dsp/*c
	)

	file(GLOB WEBP_ENC_SRCS
		RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libwebp/src/enc/*c
	)

	file(GLOB WEBP_UTILS_SRCS
		RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libwebp/src/utils/*c
	)

	file(GLOB WEBP_MUX_SRCS
		RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libwebp/src/mux/*c
	)
	set(WEBP_SOURCE ${WEBP_DEC_SRCS} ${WEBP_DEMUX_SRCS} ${WEBP_DSP_SRCS} ${WEBP_ENC_SRCS} ${WEBP_UTILS_SRCS} ${WEBP_MUX_SRC})
	set(WEBP_INCLUDEDIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libwebp/src)
	add_definitions(-DWITH_WEBP)
  endif(USE_SYSTEM_WEBP)
endif(ENABLE_WEBP)
