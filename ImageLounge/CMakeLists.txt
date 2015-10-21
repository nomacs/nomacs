
# check cmake requirements
cmake_minimum_required(VERSION 2.8)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
	  
	IF(CMAKE_VERSION VERSION_GREATER 2.8.10)
		cmake_policy(SET CMP0020 NEW)	# qt warnings
	ENDIF()
  IF(CMAKE_VERSION VERSION_GREATER 2.8.12.9)
    cmake_policy(SET CMP0043 NEW)	# COMPILE_DEFINITIONS_<CONFIG> warnings
  ENDIF()
endif(COMMAND cmake_policy)

######################## begin of project
project(nomacs)

set(NOMACS_VERSION 3.0)
add_definitions(-DNOMACS_VERSION="${NOMACS_VERSION}")

set(BINARY_NAME ${CMAKE_PROJECT_NAME})


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# include macros needed
include("cmake/Utils.cmake")

# different compile options
option(ENABLE_OPENCV "Compile with Opencv (needed for RAW and TIFF)" ON)
option(ENABLE_RAW "Compile with raw images support (libraw)" ON)
option(ENABLE_WEBP "Compile with webP support (webP)" ON)
option(ENABLE_TIFF "Compile with multi-layer tiff" ON)
option(DISABLE_QT_DEBUG "Disable Qt Debug Messages" OFF)
option(ENABLE_QUAZIP "Compile with QuaZip (allows opening .zip files)" ON)

if(MSVC)
  option(ENABLE_UPNP "Compile with UPNP" OFF)
  option(ENABLE_PLUGINS "Compile with plugin system" ON)
elseif(APPLE)
  option(ENABLE_UPNP "Compile with UPNP" OFF)
  option(ENABLE_PLUGINS "Compile with plugin system" OFF)
elseif(UNIX)
  option(ENABLE_UPNP "Compile with UPNP" OFF)
  option(ENABLE_PLUGINS "Compile with plugin system" ON)
else()
  option(ENABLE_UPNP "Compile with UPNP" OFF)
  option(ENABLE_PLUGINS "Compile with plugin system" OFF)
endif()

# load paths from the user file if exists 
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/CMakeUser.cmake)
	include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeUser.cmake)
endif()

# find Qt
NMC_FINDQT()

if (DISABLE_QT_DEBUG)
	message (STATUS "disabling qt debug messages")
	add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()

if(MSVC)
	include(${CMAKE_SOURCE_DIR}/cmake/Win.cmake)
elseif(APPLE)
	include(${CMAKE_SOURCE_DIR}/cmake/Mac.cmake)
else(UNIX)
	include(${CMAKE_SOURCE_DIR}/cmake/Unix.cmake)
else()
	message(STATUS "build system unkown ... fallback to unix")
	include(${CMAKE_SOURCE_DIR}/cmake/Unix.cmake)
endif()

# gather information for building
include_directories (
	${EXIV2_INCLUDE_DIRS} 
	${LIBRAW_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_SOURCE_DIR}/src
	${WEBP_INCLUDEDIR}
	${TIFF_INCLUDE_DIR}
	${TIFF_CONFIG_DIR}
	${HUPNP_INCLUDE_DIR}
	${QUAZIP_INCLUDE_DIRECTORY}
	# ${ZLIB_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/3rdparty/libqpsd
)

file(GLOB NOMACS_SOURCES "src/*.cpp")
file(GLOB NOMACS_HEADERS "src/*.h")

if (APPLE) # todo: somehow add this to Mac.cmake or MacBuildTarget.cmake
	SET (NOMACS_SOURCES ${NOMACS_SOURCES} macosx/nomacs.icns)
endif (APPLE)

file(GLOB NOMACS_MOCS "src/*.h")

 IF(NOT ENABLE_UPNP)
	 LIST(REMOVE_ITEM NOMACS_SOURCES ${CMAKE_SOURCE_DIR}/src/DkUpnp.cpp)
	 LIST(REMOVE_ITEM NOMACS_MOCS ${CMAKE_SOURCE_DIR}/src/DkUpnp.h)
	 LIST(REMOVE_ITEM NOMACS_HEADERS ${CMAKE_SOURCE_DIR}/src/DkUpnp.h)
 ENDIF(NOT ENABLE_UPNP)

 IF(NOT ENABLE_PLUGINS)
	 LIST(REMOVE_ITEM NOMACS_SOURCES ${CMAKE_SOURCE_DIR}/src/DkPluginManager.cpp)
	 LIST(REMOVE_ITEM NOMACS_MOCS ${CMAKE_SOURCE_DIR}/src/DkPluginManager.h)
	 LIST(REMOVE_ITEM NOMACS_HEADERS ${CMAKE_SOURCE_DIR}/src/DkPluginManager.h)
 ENDIF(NOT ENABLE_PLUGINS)


set (NOMACS_FORMS
  src/nomacs.ui
)

set (NOMACS_RESOURCES
  src/nomacs.qrc
)

file(GLOB NOMACS_TRANSLATIONS "translations/*.ts")

QT5_ADD_TRANSLATION(NOMACS_QM ${NOMACS_TRANSLATIONS})

if(MSVC)
	include(${CMAKE_SOURCE_DIR}/cmake/WinBuildTarget.cmake)
elseif(APPLE)
	include(${CMAKE_SOURCE_DIR}/cmake/MacBuildTarget.cmake)
else(UNIX)
	include(${CMAKE_SOURCE_DIR}/cmake/UnixBuildTarget.cmake)
else()
	message(STATUS "build system unkown ... fallback to unix")
	include(${CMAKE_SOURCE_DIR}/cmake/UnixBuildTarget.cmake)
endif()


#debug for printing out all variables 
# get_cmake_property(_variableNames VARIABLES)
# foreach (_variableName ${_variableNames})
    # message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

if(UNIX AND ENABLE_PLUGINS)
  if(EXISTS "${CMAKE_SOURCE_DIR}/plugins")
    SET(NOMACS_FOUND true)
    SET(NOMACS_VARS_ALREADY_SET true)
    add_subdirectory(plugins)
  else()
    message(FATAL_ERROR "plugins directory not found. You have to check out the nomacs-plugins git to 'plugins' folder or disable plugins")
  endif()
endif()

