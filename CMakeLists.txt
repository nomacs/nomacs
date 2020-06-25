if(CMAKE_HOST_WIN32) # ubuntu 14.04 has cmake 2.8.12
    CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
else()
    CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
endif()

PROJECT(nomacs-plugins)

# include macros needed
include("cmake/Utils.cmake")

# load paths from the user file if exists
if (EXISTS ${CMAKE_SOURCE_DIR}/CMakeUserPaths.cmake)
	include(${CMAKE_SOURCE_DIR}/CMakeUserPaths.cmake)
endif()

NMC_POLICY()

OPTION (ENABLE_FAKE_MINIATURES "Compile FakeMiniatures plugin" ON)
OPTION (ENABLE_TRANSFORM "Compile Transform plugin" ON)
OPTION (ENABLE_PAINT "Compile Paint plugin" ON)
OPTION (ENABLE_PAGE "Compile Document Page Extraction plugin" ON)
OPTION (ENABLE_COMPOSITE "Compile Composite Image plugin" ON)
OPTION (ENABLE_SIMPLE "Compile SIMPLE PLUGIN" OFF)

if(NOT CMAKE_CL_64 AND MSVC)
    OPTION (ENABLE_NIKON "Compile Nikon plugin" ON)
else()
    message(STATUS "Info: The Nikon Plugin cannot be built with x64 compiler configuration")
endif()

NMC_PREPARE_PLUGIN()
NMC_FINDQT()
NMC_FIND_OPENCV("core" "imgproc")

set(BUILDING_MULTIPLE_PLUGINS true)

IF (ENABLE_FAKE_MINIATURES)
    add_subdirectory(FakeMiniaturesPlugin)
ENDIF(ENABLE_FAKE_MINIATURES)

IF (ENABLE_TRANSFORM)
    add_subdirectory(AffineTransformations)
ENDIF(ENABLE_TRANSFORM)

IF (ENABLE_PAINT)
    add_subdirectory(PaintPlugin)
ENDIF(ENABLE_PAINT)

IF (ENABLE_PAGE)
    add_subdirectory(PageExtractionPlugin)
ENDIF(ENABLE_PAGE)

IF (ENABLE_COMPOSITE)
    add_subdirectory(CompositePlugin)
ENDIF(ENABLE_COMPOSITE)

IF (ENABLE_SIMPLE)
    add_subdirectory(SIMPLE_PLUGIN)
ENDIF(ENABLE_SIMPLE)
