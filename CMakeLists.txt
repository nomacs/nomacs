# works for windows
# untested (and should NOT work :( ) for linux/unix

PROJECT(nomacs-plugins)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

MARK_AS_ADVANCED(CMAKE_INSTALL_PREFIX)
SET(NOMACS_BUILD_DIRECTORY "NOT_SET" CACHE PATH "Path to the nomacs build directory")
IF (${NOMACS_BUILD_DIRECTORY} STREQUAL "NOT_SET")
	MESSAGE(FATAL_ERROR "You have to set the nomacs build directory")
ENDIF()


OPTION (ENABLE_JP2K "Compile JPEG2000 plugin (jasper and qtjp2)" ON)
IF (ENABLE_JP2K)
	add_subdirectory(jasper-1.900.1)
	add_subdirectory(qtjp2imageformat)
	add_dependencies(qtjp22 libjasper)
ENDIF(ENABLE_JP2K)
	
OPTION (ENABLE_FAKEMINIATURES "Compile FakeMiniatures plugin" ON)
IF (ENABLE_FAKEMINIATURES)
	add_subdirectory(FakeMiniaturesPlugin)
ENDIF(ENABLE_FAKEMINIATURES)

OPTION (ENABLE_FLIP "Compile Flip plugin" ON)
IF (ENABLE_FLIP)
	add_subdirectory(FlipPlugin)
ENDIF(ENABLE_FLIP)

OPTION (ENABLE_TEST "Compile Test plugin" ON)
IF (ENABLE_TEST)
	add_subdirectory(TestPlugin)
ENDIF(ENABLE_TEST)

OPTION (ENABLE_TEST "Compile Paint plugin" ON)
IF (ENABLE_TEST)
	add_subdirectory(PaintPlugin)
ENDIF(ENABLE_TEST)
