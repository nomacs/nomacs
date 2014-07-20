# works for windows
# untested (and should NOT work :( ) for linux/unix

PROJECT(nomacs-plugins)

IF(EXISTS ${CMAKE_SOURCE_DIR}/CMakeUser.txt)
	include(${CMAKE_SOURCE_DIR}/CMakeUser.txt)
ENDIF()

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# MARK_AS_ADVANCED(CMAKE_INSTALL_PREFIX)

find_package(nomacs)
if(NOT NOMACS_FOUND)
	message(STATUS "find_package was unable to find nomacs")
	SET(NOMACS_BUILD_DIRECTORY "NOT_SET" CACHE PATH "Path to the nomacs build directory")
	SET(NOMACS_PLUGIN_INSTALL_DIRECTORY "NOT_SET-ONLY_NEEDED_FOR_DEPLOYING" CACHE PATH "Path to the plugin install directory for deploying")
	IF (${NOMACS_BUILD_DIRECTORY} STREQUAL "NOT_SET")
		MESSAGE(FATAL_ERROR "You have to set the nomacs build directory")
	ENDIF()
endif()



if(CMAKE_CL_64)
	SET(PLUGIN_ARCHITECTURE "x64")
else()
	SET(PLUGIN_ARCHITECTURE "x86")
endif()


OPTION (ENABLE_JP2K "Compile JPEG2000 plugin (jasper and qtjp2)" OFF)
IF (ENABLE_JP2K)
	add_subdirectory(jasper-1.900.1)
	add_subdirectory(qtjp2imageformat)
	add_dependencies(qtjp22 libjasper)
ENDIF(ENABLE_JP2K)
	
OPTION (ENABLE_FAKE_MINIATURES "Compile FakeMiniatures plugin" ON)
IF (ENABLE_FAKE_MINIATURES)
	add_subdirectory(FakeMiniaturesPlugin)
ENDIF(ENABLE_FAKE_MINIATURES)

OPTION (ENABLE_FLIP "Compile Flip plugin" ON)
IF (ENABLE_FLIP)
	add_subdirectory(FlipPlugin)
ENDIF(ENABLE_FLIP)

if(NOT CMAKE_CL_64 AND MSVC)
	OPTION (ENABLE_NIKON "Compile Nikon plugin" ON)
	IF (ENABLE_NIKON)
		add_subdirectory(NikonPlugin)
	ENDIF(ENABLE_NIKON)
else()
	message(STATUS "Info: The Nikon Plugin cannot be built with x64 compiler configuration")
endif()

OPTION (ENABLE_TRANSFORM "Compile Transform plugin" ON)
IF (ENABLE_TRANSFORM)
	add_subdirectory(AffineTransformations)
ENDIF(ENABLE_TRANSFORM)

OPTION (ENABLE_THRESHOLD "Compile Threshold plugin" ON)
IF (ENABLE_THRESHOLD)
	add_subdirectory(ThresholdPlugin)
ENDIF(ENABLE_THRESHOLD)

OPTION (ENABLE_PAINT "Compile Paint plugin" ON)
IF (ENABLE_PAINT)
	add_subdirectory(PaintPlugin)
ENDIF(ENABLE_PAINT)

OPTION (ENABLE_TEST "Compile Test plugin" OFF)
IF (ENABLE_TEST)
	add_subdirectory(TestPlugin)
ENDIF(ENABLE_TEST)
