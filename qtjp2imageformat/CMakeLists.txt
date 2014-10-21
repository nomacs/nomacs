# works for windows
# untested (and should NOT work :( ) for linux/unix

PROJECT(QTJP22)

IF(EXISTS ${CMAKE_SOURCE_DIR}/CMakeUser.txt)
	include(${CMAKE_SOURCE_DIR}/CMakeUser.txt)
ENDIF()


# don't know if we need this
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

MARK_AS_ADVANCED(CMAKE_INSTALL_PREFIX)
find_package(nomacs)
if(NOT NOMACS_FOUND)
	SET(NOMACS_BUILD_DIRECTORY "NOT_SET" CACHE PATH "Path to the nomacs build directory")
	SET(NOMACS_PLUGIN_INSTALL_DIRECTORY "NOT_SET" CACHE PATH "Path to the plugin install directory for deploying")
	IF (${NOMACS_BUILD_DIRECTORY} STREQUAL "NOT_SET")
		MESSAGE(FATAL_ERROR "You have to set the nomacs build directory")
	ENDIF()
endif()

# find the Qt
set(QT_MIN_VERSION "4.7.0")
set(QT_USE_GUI 1)
set(QT_USE_QTNETWORK 1)
find_package(Qt4 REQUIRED QtCore QtGui)

find_file(LIBJASPER_CMAKE libjasper.cmake PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../jasper-1.900.1/ DOC "config file for jasper cmake, generated automatically when cmake is executed for jasper") 
if(${LIBJASPER_CMAKE} STREQUAL "LIBJASPER_CMAKE-NOTFOUND")
	message(FATAL_ERROR "you have to first execute cmake for jasper and then compile jasper")
endif()
include(${LIBJASPER_CMAKE})
include(${QT_USE_FILE})

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/src/
	${QT_INCLUDES}
	${LIBJASPER_INCLUDE_DIRECTORY}
)

file(GLOB_RECURSE QTJP2_SOURCES "plugin/*.cpp" "src/*.cpp")
file(GLOB_RECURSE QTJP2_HEADERS "plugin/*.h" "src/*.h")

add_library(qtjp22 SHARED ${QTJP2_SOURCES} ${QTJP2_HEADERS})
target_link_libraries(qtjp22 ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTMAIN_LIBRARY})
target_link_libraries(qtjp22 debug ${LIBJASPER_DEBUG_LIB})
target_link_libraries(qtjp22 optimized ${LIBJASPER_RELEASE_LIB})



IF (MSVC)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${NOMACS_BUILD_DIRECTORY}/$<CONFIGURATION>/plugins/)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${NOMACS_BUILD_DIRECTORY}/$<CONFIGURATION>/plugins/)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/${PROJECT_NAME}.dll DESTINATION ${NOMACS_PLUGIN_INSTALL_DIRECTORY}/${PLUGIN_ID}/${PLUGIN_VERSION}/${PLUGIN_ARCHITECTURE}/ CONFIGURATIONS Release)
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/d.txt DESTINATION ${NOMACS_PLUGIN_INSTALL_DIRECTORY}/${PLUGIN_ID}/${PLUGIN_VERSION}/ CONFIGURATIONS Release)

ENDIF ()

