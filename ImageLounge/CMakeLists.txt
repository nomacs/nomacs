
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

# needed for soname
set(NOMACS_VERSION_MAJOR 3)
set(NOMACS_VERSION_MINOR 0)
set(NOMACS_VERSION_PATCH 0)
set(NOMACS_VERSION ${NOMACS_VERSION_MAJOR}.${NOMACS_VERSION_MINOR})

add_definitions(-DNOMACS_VERSION="${NOMACS_VERSION}")

set(BINARY_NAME ${CMAKE_PROJECT_NAME})
set(NOMACS_BUILD_DIRECTORY ${CMAKE_BINARY_DIR})

if(DEFINED GLOBAL_READ_BUILD)
	if(${GLOBAL_READ_BUILD}) # cannot be incooperated into if one line above
		# set output path when building for READ
		if(MSVC)
			SET(CMAKE_LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
			SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>)
			SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>)
		else()
			set(NOMACS_BUILD_DIRECTORY ${CMAKE_BINARY_DIR}/READ)
			SET(CMAKE_LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/READ)
			SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/READ)
			SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/READ)	
		endif()
	endif()
endif()

# include macros needed
include("cmake/Utils.cmake")

# different compile options
option(ENABLE_OPENCV "Compile with Opencv (needed for RAW and TIFF)" ON)
option(ENABLE_RAW "Compile with raw images support (libraw)" ON)
option(ENABLE_WEBP "Compile with webP support (webP)" OFF)
option(ENABLE_TIFF "Compile with multi-layer tiff" ON)
option(DISABLE_QT_DEBUG "Disable Qt Debug Messages" OFF)
option(ENABLE_QUAZIP "Compile with QuaZip (allows opening .zip files)" ON)
option(ENABLE_INCREMENTER "Run Build Incrementer" OFF)

if(MSVC)
	option(ENABLE_QUAZIP "Compile with QuaZip (allows opening .zip files)" ON)
	option(ENABLE_UPNP "Compile with UPNP" OFF)
	option(ENABLE_PLUGINS "Compile with plugin system" ON)
elseif(APPLE)
	option(ENABLE_QUAZIP "Compile with QuaZip (allows opening .zip files)" OFF)
	option(ENABLE_UPNP "Compile with UPNP" OFF)
	option(ENABLE_PLUGINS "Compile with plugin system" OFF)
	message(STATUS "QuaZip disabled cause^ ur using a mac")
	
elseif(UNIX)
	option(ENABLE_UPNP "Compile with UPNP" OFF)
	option(ENABLE_PLUGINS "Compile with plugin system" ON)
	option(ENABLE_QUAZIP "Compile with QuaZip (allows opening .zip files)" ON)
else()
	option(ENABLE_UPNP "Compile with UPNP" OFF)
	option(ENABLE_PLUGINS "Compile with plugin system" OFF)
	option(ENABLE_QUAZIP "Compile with QuaZip (allows opening .zip files)" ON)
endif()

if (ENABLE_QUAZIP)
	message(STATUS "QuaZip enabled")	
endif()

# load paths from the user file if exists 
if(EXISTS ${CMAKE_SOURCE_DIR}/CMakeUser.cmake)
	include(${CMAKE_SOURCE_DIR}/CMakeUser.cmake)
endif()

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
elseif(NOT MSVC)
	message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# find Qt
NMC_FINDQT()

if (DISABLE_QT_DEBUG)
	message (STATUS "disabling qt debug messages")
	add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()

if(MSVC)
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Win.cmake)
elseif(APPLE)
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Mac.cmake)
elseif(UNIX)
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Unix.cmake)
else()
	message(STATUS "build system unkown ... fallback to unix")
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Unix.cmake)
endif()

file(GLOB NOMACS_EXE_SOURCES "src/*.cpp")
file(GLOB NOMACS_EXE_HEADERS "src/*.h")

# gui
file(GLOB GUI_SOURCES "src/DkGui/*.cpp")
file(GLOB GUI_HEADERS "src/DkGui/*.h")

# loader
file(GLOB LOADER_SOURCES "src/DkLoader/*.cpp")
file(GLOB LOADER_HEADERS "src/DkLoader/*.h")

# core
file(GLOB CORE_SOURCES "src/DkCore/*.cpp")
file(GLOB CORE_HEADERS "src/DkCore/*.h")

# gather information for building
include_directories (
	${EXIV2_INCLUDE_DIRS} 
	${LIBRAW_INCLUDE_DIRS}
	${CMAKE_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/src/DkCore
	${CMAKE_CURRENT_SOURCE_DIR}/src/DkLoader
	${CMAKE_CURRENT_SOURCE_DIR}/src/DkGui
	${WEBP_INCLUDEDIR}
	${TIFF_INCLUDE_DIR}
	${TIFF_CONFIG_DIR}
	${HUPNP_INCLUDE_DIR}
	${QUAZIP_INCLUDE_DIRECTORY}
	# ${ZLIB_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libqpsd
)

if (APPLE) # todo: somehow add this to Mac.cmake or MacBuildTarget.cmake
	SET (NOMACS_SOURCES ${NOMACS_SOURCES} macosx/nomacs.icns)
endif (APPLE)

IF(NOT ENABLE_UPNP)
	LIST(REMOVE_ITEM GUI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/DkUpnp.cpp)
	LIST(REMOVE_ITEM GUI_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/DkUpnp.h)
ENDIF(NOT ENABLE_UPNP)

IF(NOT ENABLE_PLUGINS)
	LIST(REMOVE_ITEM GUI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/DkPluginManager.cpp)
	LIST(REMOVE_ITEM GUI_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/DkPluginManager.h)
ENDIF(NOT ENABLE_PLUGINS)


set (NOMACS_FORMS
src/nomacs.ui
 )

set (NOMACS_RESOURCES
src/nomacs.qrc
 )

file(GLOB NOMACS_TRANSLATIONS "translations/*.ts")

QT5_ADD_RESOURCES(NOMACS_RCC ${NOMACS_RESOURCES})
QT5_ADD_TRANSLATION(NOMACS_QM ${NOMACS_TRANSLATIONS})

if(MSVC)
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/WinBuildTarget.cmake)
elseif(APPLE)
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/MacBuildTarget.cmake)
elseif(UNIX)
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/UnixBuildTarget.cmake)
else()
	message(STATUS "build system unkown ... fallback to unix")
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/UnixBuildTarget.cmake)
endif()


#debug for printing out all variables 
# get_cmake_property(_variableNames VARIABLES)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

if(UNIX AND ENABLE_PLUGINS)
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/plugins")
		SET(NOMACS_FOUND true)
		SET(NOMACS_VARS_ALREADY_SET true)
		add_subdirectory(plugins)
	else()
		message(WARNING "plugins directory not found, not building plugins. You have to check out the nomacs-plugins git to the 'plugins' folder if you want to build them")
	endif()
endif()
