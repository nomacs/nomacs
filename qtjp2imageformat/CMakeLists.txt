# works for windows
# untested (and should NOT work :( ) for linux/unix

PROJECT(QTJP22)

# don't know if we need this
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

MARK_AS_ADVANCED(CMAKE_INSTALL_PREFIX)
SET(NOMACS_BUILD_DIRECTORY "NOT_SET" CACHE PATH "Path to the nomacs build directory")
IF (${NOMACS_BUILD_DIRECTORY} STREQUAL "NOT_SET")
	MESSAGE(FATAL_ERROR "You have to set the nomacs build directory")
ENDIF()

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

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/src/
	${QT_INCLUDES}
	${LIBJASPER_INCLUDE_DIRECTORY}
)

file(GLOB_RECURSE QTJP2_SOURCES "plugin/*.cpp" "src/*.cpp")
file(GLOB_RECURSE QTJP2_HEADERS "plugin/*.h" "src/*.h")

add_library(qtjp22 SHARED ${QTJP2_SOURCES} ${QTJP2_HEADERS})
target_link_libraries(qtjp22 ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTMAIN_LIBRARY})
target_link_libraries(qtjp22 debug ${LIBJASPER_DEBUG_LIB})
target_link_libraries(qtjp22 optimized ${LIBJASPER_RELEASE_LIB})

if(MSVC)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/qtjp22.dll ${CMAKE_CURRENT_BINARY_DIR}/Release/qtjp22.lib  DESTINATION ${NOMACS_BUILD_DIRECTORY}/Release/imageformats CONFIGURATIONS Release)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/qtjp22.dll ${CMAKE_CURRENT_BINARY_DIR}/Release/qtjp22.lib  DESTINATION ${NOMACS_BUILD_DIRECTORY}/ReallyRelease/imageformats CONFIGURATIONS Release)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/qtjp22.dll ${CMAKE_CURRENT_BINARY_DIR}/Debug/qtjp22.lib  DESTINATION ${NOMACS_BUILD_DIRECTORY}/Debug/imageformats CONFIGURATIONS Debug)
endif(MSVC)	


