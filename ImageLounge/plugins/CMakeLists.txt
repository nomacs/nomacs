# check cmake requirements
cmake_minimum_required(VERSION 3.10)

project(nomacs-plugins)

# include macros needed
include("cmake/Utils.cmake")

# load paths from the user file if exists
if(EXISTS ${CMAKE_SOURCE_DIR}/CMakeUserPaths.cmake)
    include(${CMAKE_SOURCE_DIR}/CMakeUserPaths.cmake)
endif()

NMC_POLICY()

option(ENABLE_FAKE_MINIATURES "Compile FakeMiniatures plugin" ON)
option(ENABLE_TRANSFORM "Compile Transform plugin" ON)
option(ENABLE_PAINT "Compile Paint plugin" ON)
option(ENABLE_PAGE "Compile Document Page Extraction plugin" ON)
option(ENABLE_COMPOSITE "Compile Composite Image plugin" ON)
option(ENABLE_SIMPLE "Compile SIMPLE PLUGIN" OFF)

NMC_PREPARE_PLUGIN()
NMC_FINDQT()
NMC_FIND_OPENCV("core" "imgproc")

set(BUILDING_MULTIPLE_PLUGINS true)

if(ENABLE_FAKE_MINIATURES)
    add_subdirectory(FakeMiniaturesPlugin)
endif(ENABLE_FAKE_MINIATURES)

if(ENABLE_TRANSFORM)
    add_subdirectory(AffineTransformations)
endif(ENABLE_TRANSFORM)

if(ENABLE_PAINT)
    add_subdirectory(PaintPlugin)
endif(ENABLE_PAINT)

if(ENABLE_PAGE)
    add_subdirectory(PageExtractionPlugin)
endif(ENABLE_PAGE)

if(ENABLE_COMPOSITE)
    add_subdirectory(CompositePlugin)
endif(ENABLE_COMPOSITE)

if(ENABLE_SIMPLE)
    add_subdirectory(SIMPLE_PLUGIN)
endif(ENABLE_SIMPLE)
