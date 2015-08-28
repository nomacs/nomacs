PROJECT(nomacs-plugins)

# include macros needed
include("cmake/Utils.cmake")

IF(EXISTS ${CMAKE_SOURCE_DIR}/CMakeUser.txt)
	include(${CMAKE_SOURCE_DIR}/CMakeUser.txt)
ENDIF()

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(ENABLE_QT5 ON)

OPTION (ENABLE_JP2K "Compile JPEG2000 plugin (jasper and qtjp2)" OFF)
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

NMC_PREPARE_PLUGIN()

NMC_FINDQT()

# OpenCV
NMC_FIND_OPENCV()

if(CMAKE_CL_64)
	SET(PLUGIN_ARCHITECTURE "x64")
else()
	SET(PLUGIN_ARCHITECTURE "x86")
endif()


set(BUILDING_MULTIPLE_PLUGINS true)

IF (ENABLE_JP2K)
	add_subdirectory(jasper-1.900.1)
	add_subdirectory(qtjp2imageformat)
	add_dependencies(qtjp22 libjasper)
ENDIF(ENABLE_JP2K)
	

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

OPTION (ENABLE_PAGE "Compile Document Page Extraction plugin" OFF)
IF (ENABLE_PAGE)
	add_subdirectory(PageExtractionPlugin)
ENDIF(ENABLE_PAGE)
