# works for windows
# untested (and should NOT work :( ) for linux/unix
PROJECT(libjasper)

INCLUDE (CheckIncludeFiles)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
# don't know if we need this
if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

file(GLOB_RECURSE LIBJASPER_SOURCES "src/libjasper/*.c")
file(GLOB_RECURSE LIBJASPER_HEADERS "src/libjasper/*.h")

SET(LIBJASPER_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/libjasper/include/)
SET(LIBJASPER_RELEASE_LIB ${CMAKE_CURRENT_BINARY_DIR}/Release/libjasper.lib)
SET(LIBJASPER_DEBUG_LIB ${CMAKE_CURRENT_BINARY_DIR}/Debug/libjasper.lib)
include_directories (
	${LIBJASPER_INCLUDE_DIRECTORY}
)

if(MSVC)
	add_definitions(/DJAS_WIN_MSVC_BUILD)
endif(MSVC)	

add_definitions(/D_LIB)
CHECK_INCLUDE_FILES(stdbool.h HAVE_STDBOOL_H)
CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILES(sys/time.h HAVE_SYSTIME_H)

add_library(libjasper SHARED ${LIBJASPER_SOURCES} ${LIBJASPER_HEADERS})

# if(MSVC)
	# add_custom_command(TARGET libjasper POST_BUILD COMMAND xcopy \"${CMAKE_CURRENT_BINARY_DIR}/Release/libjasper.dll\" \"${CMAKE_CURRENT_SOURCE_DIR}/../bin/Release\" /C)
	# add_custom_command(TARGET libjasper POST_BUILD COMMAND xcopy \"${CMAKE_CURRENT_BINARY_DIR}/Release/libjasper.lib\" \"${CMAKE_CURRENT_SOURCE_DIR}/../bin/Release\" /C)
	# add_custom_command(TARGET libjasper POST_BUILD COMMAND xcopy \"${CMAKE_CURRENT_BINARY_DIR}/Debug/libjasper.dll\" \"${CMAKE_CURRENT_SOURCE_DIR}/../bin/Debug\" /C)
	# add_custom_command(TARGET libjasper POST_BUILD COMMAND xcopy \"${CMAKE_CURRENT_BINARY_DIR}/Debug/libjasper.lib\" \"${CMAKE_CURRENT_SOURCE_DIR}/../bin/Debug\" /C)
# endif(MSVC)	

if(MSVC)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/libjasper.dll ${CMAKE_CURRENT_BINARY_DIR}/Release/libjasper.lib  DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../bin/Release CONFIGURATIONS Release)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/libjasper.dll ${CMAKE_CURRENT_BINARY_DIR}/Debug/libjasper.lib  DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../bin/Debug CONFIGURATIONS Debug)
endif(MSVC)	

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libjasper.cmake.skeleton ${CMAKE_CURRENT_SOURCE_DIR}/libjasper.cmake @ONLY)

# ADD_CUSTOM_TARGET(install_jaspar ALL
    # COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/cmake_install.cmake)
# ADD_DEPENDENCIES(install_jaspar libjaspar)

MESSAGE(STATUS "configuration file generated")