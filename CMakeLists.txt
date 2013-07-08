# works for windows
# untested (and should NOT work :( ) for linux/unix

PROJECT(nomacs-plugins)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

MARK_AS_ADVANCED(CMAKE_INSTALL_PREFIX)
SET(NOMACS_BUILD_DIRECTORY "NOT_SET" CACHE PATH "Path to the nomacs build directory")
IF (${NOMACS_BUILD_DIRECTORY} STREQUAL "NOT_SET")
	MESSAGE(FATAL_ERROR "You have to set the nomacs build directory")
ENDIF()


add_subdirectory(jasper-1.900.1)
add_subdirectory(qtjp2imageformat)
add_dependencies(qtjp22 libjasper)
add_subdirectory(FakeMiniaturesPlugin)

