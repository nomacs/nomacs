if(CMAKE_HOST_WIN32) # ubuntu 14.04 has cmake 2.8.12
	CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
else()
	CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
endif()

PROJECT(nomacs-plugins)

# include macros needed
include("cmake/Utils.cmake")

IF(EXISTS ${CMAKE_SOURCE_DIR}/CMakeUser.txt)
	include(${CMAKE_SOURCE_DIR}/CMakeUser.txt)
ENDIF()

NMC_POLICY()

OPTION (ENABLE_FAKE_MINIATURES "Compile FakeMiniatures plugin" ON)
if(NOT CMAKE_CL_64 AND MSVC)
	OPTION (ENABLE_NIKON "Compile Nikon plugin" ON)
else()
	message(STATUS "Info: The Nikon Plugin cannot be built with x64 compiler configuration")
endif()
OPTION (ENABLE_TRANSFORM "Compile Transform plugin" ON)
OPTION (ENABLE_THRESHOLD "Compile Threshold plugin" ON)
OPTION (ENABLE_PAINT "Compile Paint plugin" ON)
OPTION (ENABLE_DOC "Compile DocAnalysis plugin" OFF)
OPTION (ENABLE_PAGE "Compile Document Page Extraction plugin" OFF)
OPTION (ENABLE_OCR "Compile Ocr plugin" OFF)
OPTION (ENABLE_SIMPLE "Compile SIMPLE PLUGIN" OFF)
OPTION (ENABLE_INSTAGRAM "Compile INSTAGRAM PLUGIN" ON)
OPTION (ENABLE_FILTER "Compile Instagram like filters" ON)
OPTION (ENABLE_INSTAGRAM_FILTER "Compile INSTAGRAM LIKE PLUGIN" ON)
OPTION (ENABLE_INSTA_LIKE_FILTER "Compile INSTAGRAM LIKE FILTER PLUGIN" ON)

NMC_PREPARE_PLUGIN()

NMC_FINDQT()

# OpenCV
# TODO ein if wenn zumindest 1 projekt ausgewählt ist das opencv braucht
NMC_FIND_OPENCV()

NMC_EXIV_INCLUDES()

if(CMAKE_CL_64)
	SET(PLUGIN_ARCHITECTURE "x64")
else()
	SET(PLUGIN_ARCHITECTURE "x86")
endif()


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


IF (ENABLE_THRESHOLD)
	add_subdirectory(ThresholdPlugin)
ENDIF(ENABLE_THRESHOLD)


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

