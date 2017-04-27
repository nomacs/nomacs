if(CMAKE_HOST_WIN32) # ubuntu 14.04 has cmake 2.8.12
    CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
else()
    CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
endif()

PROJECT(nomacs-plugins)

# include macros needed
include("cmake/Utils.cmake")

IF(EXISTS ${CMAKE_SOURCE_DIR}/CMakeUser.cmake)
    include(${CMAKE_SOURCE_DIR}/CMakeUser.cmake)
ENDIF()

NMC_POLICY()

OPTION (ENABLE_FAKE_MINIATURES "Compile FakeMiniatures plugin" ON)
if(NOT CMAKE_CL_64 AND MSVC)
    OPTION (ENABLE_NIKON "Compile Nikon plugin" ON)
else()
    message(STATUS "Info: The Nikon Plugin cannot be built with x64 compiler configuration")
endif()
OPTION (ENABLE_TRANSFORM "Compile Transform plugin" ON)
OPTION (ENABLE_PAINT "Compile Paint plugin" ON)
OPTION (ENABLE_DOC "Compile DocAnalysis plugin" OFF)
OPTION (ENABLE_PAGE "Compile Document Page Extraction plugin" ON)
OPTION (ENABLE_OCR "Compile Ocr plugin" OFF)
OPTION (ENABLE_SIMPLE "Compile SIMPLE PLUGIN" OFF)
OPTION (ENABLE_PATCHMATCHING "Compile Patch Matching Plugin" OFF)
OPTION (ENABLE_IMAGESTITCHING "Compile Image Stitching Plugin" OFF)
OPTION (ENABLE_RULERDETECTION "Compile Ruler Detection Plugin" ON)

NMC_PREPARE_PLUGIN()
NMC_FINDQT()
NMC_FIND_OPENCV()

set(BUILDING_MULTIPLE_PLUGINS true)

IF (ENABLE_FAKE_MINIATURES)
    add_subdirectory(FakeMiniaturesPlugin)
ENDIF(ENABLE_FAKE_MINIATURES)

IF (ENABLE_NIKON)
  add_subdirectory(NikonPlugin)
ENDIF(ENABLE_NIKON)

IF (ENABLE_TRANSFORM)
    add_subdirectory(AffineTransformations)
ENDIF(ENABLE_TRANSFORM)

IF (ENABLE_PAINT)
    add_subdirectory(PaintPlugin)
ENDIF(ENABLE_PAINT)

IF (ENABLE_DOC)
    add_subdirectory(DocAnalysisPlugin)
ENDIF(ENABLE_DOC)

IF (ENABLE_PAGE)
    add_subdirectory(PageExtractionPlugin)
ENDIF(ENABLE_PAGE)

IF (ENABLE_OCR)
    add_subdirectory(OcrPlugin)
ENDIF(ENABLE_OCR)

IF (ENABLE_SIMPLE)
    add_subdirectory(SIMPLE_PLUGIN)
ENDIF(ENABLE_SIMPLE)

IF (ENABLE_INSTAGRAM)
    add_subdirectory(InstagramFilterPlugin)
ENDIF(ENABLE_INSTAGRAM)

IF (ENABLE_FILTER)
    add_subdirectory(FilterPlugin)
ENDIF(ENABLE_FILTER)

IF (ENABLE_INSTAGRAM_FILTER)
    add_subdirectory(InstagramLikeFilterPlugin)
ENDIF(ENABLE_INSTAGRAM_FILTER)

IF (ENABLE_INSTA_LIKE_FILTER)
    add_subdirectory(InstaLikeFilterPlugin)
ENDIF(ENABLE_INSTA_LIKE_FILTER)

IF (ENABLE_MARS)
    add_subdirectory(MarsPlugin)
ENDIF (ENABLE_MARS)

IF (ENABLE_PATCHMATCHING)
    add_subdirectory(PatchMatchingPlugin)
ENDIF (ENABLE_PATCHMATCHING)

IF (ENABLE_IMAGESTITCHING)
    add_subdirectory(ImageStitchingPlugin)
ENDIF (ENABLE_IMAGESTITCHING)

IF (ENABLE_RULERDETECTION)
    add_subdirectory(RulerDetectionPlugin)
ENDIF (ENABLE_RULERDETECTION)
