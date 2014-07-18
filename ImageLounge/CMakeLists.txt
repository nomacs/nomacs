
# check cmake requirements
cmake_minimum_required(VERSION 2.6)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
	  
	IF(CMAKE_VERSION VERSION_GREATER 2.8.10)
		cmake_policy(SET CMP0020 NEW)	# qt warnings
	ENDIF()
endif(COMMAND cmake_policy)

######################## begin of project
project(nomacs)

set(NOMACS_VERSION 2.0.1)
add_definitions(-DNOMACS_VERSION="${NOMACS_VERSION}")

set(BINARY_NAME ${CMAKE_PROJECT_NAME})

# include macros needed
include("cmake/Utils.cmake")

# different compile options
option(ENABLE_OPENCV "Compile with Opencv (needed for RAW and TIFF)" ON)
option(ENABLE_RAW "Compile with raw images support (libraw)" ON)
option(ENABLE_WEBP "Compile with webP support (webP)" ON)
option(ENABLE_TIFF "Compile with multi-layer tiff" ON)
option(DISABLE_QT_DEBUG "Disable Qt Debug Messages" OFF)
option(ENABLE_QT5 "Compile with Qt5 (Qt5)" OFF)

if(MSVC)
  option(ENABLE_UPNP "Compile with UPNP" ON)
  option(ENABLE_PLUGINS "Compile with plugin system (currently only windows is supported)" ON)
else()
  option(ENABLE_UPNP "Compile with UPNP" OFF)
  option(ENABLE_PLUGINS "Compile with plugin system (currently only windows is supported)" OFF)
endif()

# load paths from the user file if exists 
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/CMakeUser.cmake)
	include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeUser.cmake)
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

# find Qt
NMC_FINDQT()

# webp for all platforms
NMC_ENABLE_WEBP()


# gather information for building
include_directories (
	${QT_INCLUDES}
	${EXIV2_INCLUDE_DIRS}
	${LIBRAW_INCLUDE_DIRS}
	${OpenCV_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_SOURCE_DIR}/src
	${WEBP_INCLUDE_DIR}
	${TIFF_INCLUDE_DIR}
	${TIFF_CONFIG_DIR}
	${HUPNP_INCLUDE_DIR}
)

file(GLOB NOMACS_SOURCES "src/*.cpp")
file(GLOB NOMACS_HEADERS "src/*.h")

if (APPLE) # todo: somehow add this to Mac.cmake or MacBuildTarget.cmake
	SET (NOMACS_SOURCES ${NOMACS_SOURCES} macosx/nomacs.icns)
endif (APPLE)

file(GLOB NOMACS_MOCS "src/*.h")

file(GLOB LIBQPSD_SOURCES "libqpsd/*.cpp")
file(GLOB LIBQPSD_HEADERS "libqpsd/*.h")
file(GLOB LIBQPSD_MOCS "libqpsd/*.h")

IF(NOT ENABLE_UPNP)
	LIST(REMOVE_ITEM NOMACS_SOURCES ${CMAKE_SOURCE_DIR}/src/DkUpnp.cpp)
	LIST(REMOVE_ITEM NOMACS_MOCS ${CMAKE_SOURCE_DIR}/src/DkUpnp.h)
	LIST(REMOVE_ITEM NOMACS_HEADERS ${CMAKE_SOURCE_DIR}/src/DkUpnp.h)
ENDIF(NOT ENABLE_UPNP)

set (NOMACS_FORMS
        src/nomacs.ui
)

set (NOMACS_RESOURCES
		src/nomacs.qrc
)

set (NOMACS_RESOURCES
		src/nomacs.qrc
)

set(NOMACS_TRANSLATIONS
	translations/nomacs_als.ts
	translations/nomacs_az.ts
	translations/nomacs_cs.ts		
	translations/nomacs_de.ts
	translations/nomacs_en.ts
	translations/nomacs_es.ts		
	translations/nomacs_fr.ts
	translations/nomacs_it.ts
	translations/nomacs_pl.ts
	translations/nomacs_pt.ts
	translations/nomacs_ru.ts
	translations/nomacs_sk.ts
	translations/nomacs_sl.ts
	translations/nomacs_sr.ts		
	translations/nomacs_zh.ts		
)

IF (NOT ENABLE_QT5)
 QT4_ADD_TRANSLATION(NOMACS_QM ${NOMACS_TRANSLATIONS})
 QT4_ADD_RESOURCES(NOMACS_RCC ${NOMACS_RESOURCES})
 QT4_WRAP_CPP(NOMACS_MOC_SRC ${NOMACS_MOCS})
 QT4_WRAP_CPP(LIBQPSD_MOC_SRC ${LIBQPSD_MOCS})
ELSE()
 QT5_ADD_TRANSLATION(NOMACS_QM ${NOMACS_TRANSLATIONS})
 QT5_ADD_RESOURCES(NOMACS_RCC ${NOMACS_RESOURCES})
 QT5_WRAP_CPP(NOMACS_MOC_SRC ${NOMACS_MOCS})
 QT5_WRAP_CPP(LIBQPSD_MOC_SRC ${LIBQPSD_MOCS})
ENDIF()

if (DISABLE_QT_DEBUG)
	message (STATUS "disabling qt debug messages")
	add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()

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
