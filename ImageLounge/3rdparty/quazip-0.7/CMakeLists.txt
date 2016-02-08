project (QuaZip)
cmake_minimum_required(VERSION 2.6)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
	  
	IF(CMAKE_VERSION VERSION_GREATER 2.8.10)
		cmake_policy(SET CMP0020 NEW)	# qt warnings
	ENDIF()
  IF(CMAKE_VERSION VERSION_GREATER 2.8.12.9)
    cmake_policy(SET CMP0043 NEW)	# COMPILE_DEFINITIONS_<CONFIG> warnings
  ENDIF()
endif(COMMAND cmake_policy)


#added by stefan for nomacs
SET(CMAKE_DEBUG_POSTFIX "d")
# Use system zlib on unix and Qt ZLIB on Windows
unset(ZLIB_INCLUDE_DIRS CACHE)
unset(ZLIB_LIBRARIES CACHE)
IF(UNIX)
	find_package(ZLIB REQUIRED)
ELSE(UNIX)
	find_path(ZLIB_INCLUDE_DIRS NAMES "zlib.h" HINTS "${QT_ROOT}/src/3rdparty/zlib" "${QT_ROOT}/../include/QtZlib" DOC  "Path to ZLIB headers of Qt")
	SET(ZLIB_LIBRARIES "")
	IF(NOT EXISTS "${ZLIB_INCLUDE_DIRS}/zlib.h")
		MESSAGE("Please specify a valid zlib include dir")
	ENDIF(NOT EXISTS "${ZLIB_INCLUDE_DIRS}/zlib.h")
ENDIF(UNIX)


# All build libraries are moved to this directory
SET(LIBRARY_OUTPUT_PATH ${NOMACS_BUILD_DIRECTORY})

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)")
set(LIB_DESTINATION "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE STRING "Library directory name" FORCE)

add_subdirectory(quazip)

# added by stefan for nomacs

# write configuration file
get_property(QUAZIP_DEBUG_NAME TARGET quazip PROPERTY DEBUG_OUTPUT_NAME)
get_property(QUAZIP_RELEASE_NAME TARGET quazip PROPERTY RELEASE_OUTPUT_NAME)
SET(QUAZIP_LIBS optimized ${QUAZIP_RELEASE_NAME}.lib debug ${QUAZIP_DEBUG_NAME}.lib)
SET(QUAZIP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(QUAZIP_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
SET(QUAZIP_BUILD_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
SET(ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIRS})
configure_file(${QUAZIP_SOURCE_DIR}/QuaZip.cmake.in ${QUAZIP_BUILD_DIRECTORY}/QuaZipConfig.cmake)

