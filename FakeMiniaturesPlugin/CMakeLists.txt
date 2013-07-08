PROJECT(fakeMiniaturesNomacsPlugin)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(PLUGIN_VERSION 1.0.0)
add_definitions(-DPLUGIN_VERSION="${PLUGIN_VERSION}")

if (CMAKE_BUILD_TYPE STREQUAL "debug" OR CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    message(STATUS "A debug build. -DDEBUG is defined")
    add_definitions(-DDEBUG)
    ADD_DEFINITIONS(-DQT_NO_DEBUG)
elseif (NOT MSVC) # debug and release need qt debug outputs on windows
    message(STATUS "A release build (non-debug). Debugging outputs are silently ignored.")
    add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif ()

#Set the custom CMake module directory where our include/lib finders are
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

IF(MSVC)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libs)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Debug)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Release)
ENDIF(MSVC)

# find the Qt
set(QT_MIN_VERSION "4.7.0")
set(QT_USE_GUI 1)
set(QT_USE_QTNETWORK 1)
find_package(Qt4 REQUIRED QtCore QtGui QtNetwork)
include(${QT_USE_FILE})
	          
# OpenCV
find_package(OpenCV 2.1.0 REQUIRED core imgproc)

IF (OpenCV_VERSION VERSION_LESS 2.4.0 AND OpenCV_FOUND AND MSVC) # OpenCV didn't allow to define packages before version 2.4.0 ... plugin was linking against all libs even if they were not compiled -> error
	string(REGEX REPLACE "\\." "" OpenCV_SHORT_VERSION ${OpenCV_VERSION})
	SET(OpenCV_LIBS "debug;opencv_imgproc${OpenCV_SHORT_VERSION}d;optimized;opencv_imgproc${OpenCV_SHORT_VERSION};debug;opencv_core${OpenCV_SHORT_VERSION}d;optimized;opencv_core${OpenCV_SHORT_VERSION};")
ENDIF()
IF (NOT OpenCV_FOUND)
	message(FATAL_ERROR "OpenCV not found. It's mandatory for this plugin!") 
ELSE()
	add_definitions(-DWITH_OPENCV)
ENDIF()
	
include_directories (
	${QT_INCLUDES}
	${OpenCV_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_SOURCE_DIR}/src
)

file(GLOB PLUGIN_SOURCES "src/*.cpp")
file(GLOB PLUGIN_HEADERS "src*.h")
file(GLOB PLUGIN_MOCS "src/*.h")

set (PLUGIN_RESOURCES
        src/nomacsPlugin.qrc
)

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_DLL)

QT4_WRAP_CPP(PLUGIN_MOC_SRC ${PLUGIN_MOCS})
QT4_ADD_RESOURCES(PLUGIN_RCC ${PLUGIN_RESOURCES})

set(EXE_NAME ${CMAKE_PROJECT_NAME})
link_directories(${OpenCV_LIBRARY_DIRS})
ADD_LIBRARY(${EXE_NAME} SHARED ${PLUGIN_SOURCES} ${PLUGIN_MOC_SRC} ${PLUGIN_RCC} ${PLUGIN_HEADERS})	
target_link_libraries(${EXE_NAME} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTMAIN_LIBRARY} ${OpenCV_LIBS})

IF (MSVC) # copy qt dlls and change settings for different projects

	SET(OpenCV_REQUIRED_MODULES core imgproc)
	FOREACH(opencvlib ${OpenCV_REQUIRED_MODULES})
		FILE(GLOB dllpath ${OpenCV_DIR}/bin/Release/opencv_${opencvlib}*.dll)
		FILE(COPY ${dllpath} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)

		FILE(GLOB dllpath ${OpenCV_DIR}/bin/Debug/opencv_${opencvlib}*d.dll)
		FILE(COPY ${dllpath} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
	ENDFOREACH(opencvlib)

	SET (QTLIBLIST QtCore QtGui QtNetwork)
	FOREACH(qtlib ${QTLIBLIST})
		GET_FILENAME_COMPONENT(QT_DLL_PATH_tmp ${QT_QMAKE_EXECUTABLE} PATH)
		FILE(COPY ${QT_DLL_PATH_tmp}/${qtlib}4.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
		FILE(COPY ${QT_DLL_PATH_tmp}/${qtlib}d4.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
	ENDFOREACH(qtlib)


	SET(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "limited configs" FORCE)
	add_definitions(/Zc:wchar_t-)
	set_target_properties(${EXE_NAME} PROPERTIES LINK_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:CONSOLE /LARGEADDRESSAWARE")
	set_target_properties(${EXE_NAME} PROPERTIES LINK_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /SUBSYSTEM:CONSOLE /LARGEADDRESSAWARE")

	SET(_moc ${CMAKE_CURRENT_BINARY_DIR}/GeneratedFiles)
	SOURCE_GROUP("Generated Files" FILES ${PLUGIN_MOC_SRC} ${PLUGIN_RCC})


ELSE(MSVC)
	message(WARNING "VISUAL STUDIO NOT FOUND!")
ENDIF()