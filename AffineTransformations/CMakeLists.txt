
PROJECT(affineTransformPlugin)

######################################################### CHANGE THIS FOR NEW PLUGINS ###################################################
set(PLUGIN_VERSION 1.0.0)
set(PLUGIN_ID "7a4a50a45ddb408ba46a16a40e6518fd") #GUID without hyphens generated at http://www.guidgenerator.com/
######################################################### CHANGE THIS FOR NEW PLUGINS ###################################################

add_definitions(-DPLUGIN_VERSION="${PLUGIN_VERSION}")
add_definitions(-DPLUGIN_ID="${PLUGIN_ID}")



CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

#find nomacs build directory
MARK_AS_ADVANCED(CMAKE_INSTALL_PREFIX)
SET(NOMACS_BUILD_DIRECTORY "NOT_SET" CACHE PATH "Path to the nomacs build directory")
SET(NOMACS_INCLUDE_DIRECTORY "NOT_SET" CACHE PATH "Path to the nomacs source directory")

IF(${NOMACS_BUILD_DIRECTORY} STREQUAL "NOT_SET")
	MESSAGE(FATAL_ERROR "You have to set the nomacs build directory")
ENDIF()

if (CMAKE_BUILD_TYPE STREQUAL "debug" OR CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    message(STATUS "A debug build. -DDEBUG is defined")
    add_definitions(-DDEBUG)
    ADD_DEFINITIONS(-DQT_NO_DEBUG)
elseif (NOT MSVC) # debug and release need qt debug outputs on windows
    message(STATUS "A release build (non-debug). Debugging outputs are silently ignored.")
    add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif ()

#Set the custom CMake module directory where our include/lib finders are
# SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# IF(MSVC)
	# file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libs)
	# file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Debug)
	# file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Release)
# ENDIF(MSVC)

# find the Qt
set(QT_MIN_VERSION "4.7.0")
set(QT_USE_GUI 1)
set(QT_USE_QTNETWORK 1)
find_package(Qt4 REQUIRED QtCore QtGui QtNetwork)
include(${QT_USE_FILE})
	          
# OpenCV
SET(OpenCV_LIBS "")
if (PKG_CONFIG_FOUND) # not sure: pkgconfig is needed for old linux  with old old opencv systems
	pkg_check_modules(OpenCV  opencv>=2.1.0)
	SET(OpenCV_LIBS ${OpenCV_LIBRARIES})
endif(PKG_CONFIG_FOUND)
IF (OpenCV_LIBS STREQUAL "") 
	find_package(OpenCV 2.1.0 REQUIRED core imgproc)
ENDIF()
IF (OpenCV_VERSION VERSION_LESS 2.4.0 AND OpenCV_FOUND AND MSVC) # OpenCV didn't allow to define packages before version 2.4.0 ... nomacs was linking against all libs even if they were not compiled -> error
	string(REGEX REPLACE "\\." "" OpenCV_SHORT_VERSION ${OpenCV_VERSION})
	SET(OpenCV_LIBS "debug;opencv_imgproc${OpenCV_SHORT_VERSION}d;optimized;opencv_imgproc${OpenCV_SHORT_VERSION};debug;opencv_core${OpenCV_SHORT_VERSION}d;optimized;opencv_core${OpenCV_SHORT_VERSION};")
ENDIF()
IF (NOT OpenCV_FOUND)
	message(FATAL_ERROR "OpenCV not found. It's mandatory when used with ENABLE_RAW enabled") 
ELSE()
	add_definitions(-DWITH_OPENCV)
ENDIF()
	
include_directories (
	${QT_INCLUDES}
	${OpenCV_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_SOURCE_DIR}/src
	${NOMACS_INCLUDE_DIRECTORY}
)

file(GLOB PLUGIN_SOURCES "src/*.cpp")
file(GLOB PLUGIN_HEADERS "src/*.h")
file(GLOB PLUGIN_MOCS "src/*.h", ${NOMACS_INCLUDE_DIRECTORY}/DkPluginInterface.h)

set (PLUGIN_RESOURCES
        src/nomacsPlugin.qrc
)

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_DLL)

set(NOMACS_LIB optimized libnomacs.lib debug libnomacsd.lib)

QT4_WRAP_CPP(PLUGIN_MOC_SRC ${PLUGIN_MOCS})
QT4_ADD_RESOURCES(PLUGIN_RCC ${PLUGIN_RESOURCES})

link_directories(${OpenCV_LIBRARY_DIRS} ${NOMACS_BUILD_DIRECTORY}/$(CONFIGURATION) ${NOMACS_BUILD_DIRECTORY}/libs)
ADD_LIBRARY(${PROJECT_NAME} SHARED ${PLUGIN_SOURCES} ${PLUGIN_MOC_SRC} ${PLUGIN_RCC} ${PLUGIN_HEADERS})	
target_link_libraries(${PROJECT_NAME} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTMAIN_LIBRARY} ${OpenCV_LIBS} ${NOMACS_LIB})

IF (MSVC)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${NOMACS_BUILD_DIRECTORY}/$<CONFIGURATION>/plugins/)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${NOMACS_BUILD_DIRECTORY}/$<CONFIGURATION>/plugins/)
	
	# write dll to d.txt (used for automated plugin download)
	if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/d.txt)
		file(READ ${CMAKE_CURRENT_SOURCE_DIR}/d.txt fileContent)	
		if(CMAKE_CL_64)
			string (REGEX MATCHALL "x64" matches ${fileContent})
			if(NOT matches)
				file(APPEND ${CMAKE_CURRENT_SOURCE_DIR}/d.txt "x64 ${PROJECT_NAME}.dll\n")
			endif()
		else()
			string (REGEX MATCHALL "x86" matches ${fileContent})		
			if(NOT matches)
				file(APPEND ${CMAKE_CURRENT_SOURCE_DIR}/d.txt "x86 ${PROJECT_NAME}.dll\n")
			endif()	
		endif(CMAKE_CL_64)
	else()
		if(CMAKE_CL_64)
			file(APPEND ${CMAKE_CURRENT_SOURCE_DIR}/d.txt "x64 ${PROJECT_NAME}.dll\n")
		else()
			file(APPEND ${CMAKE_CURRENT_SOURCE_DIR}/d.txt "x86 ${PROJECT_NAME}.dll\n")
		endif()	
	endif()
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/${PROJECT_NAME}.dll DESTINATION ${NOMACS_PLUGIN_INSTALL_DIRECTORY}/${PLUGIN_ID}/${PLUGIN_VERSION}/${PLUGIN_ARCHITECTURE}/ CONFIGURATIONS Release)
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/d.txt DESTINATION ${NOMACS_PLUGIN_INSTALL_DIRECTORY}/${PLUGIN_ID}/${PLUGIN_VERSION}/ CONFIGURATIONS Release)
	
ENDIF(MSVC)