
PROJECT(fakeMiniaturesPlugin)

IF(EXISTS ${CMAKE_SOURCE_DIR}/CMakeUser.txt)
	include(${CMAKE_SOURCE_DIR}/CMakeUser.txt)
ENDIF()

# include macros needed
include("../cmake/Utils.cmake")

######################################################### CHANGE THIS FOR NEW PLUGINS ###################################################
set(PLUGIN_VERSION 2.0.0)
set(PLUGIN_ID a2ac7b68866b4ab29fb1df3e170b8f0d) #GUID without hyphens generated at http://www.guidgenerator.com/
######################################################### CHANGE THIS FOR NEW PLUGINS ###################################################

add_definitions(-DPLUGIN_VERSION="${PLUGIN_VERSION}")
add_definitions(-DPLUGIN_ID="${PLUGIN_ID}")


CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
	  cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)

#find nomacs build directory
MARK_AS_ADVANCED(CMAKE_INSTALL_PREFIX)
find_package(nomacs)
if(NOT NOMACS_FOUND)
	SET(NOMACS_BUILD_DIRECTORY "NOT_SET" CACHE PATH "Path to the nomacs build directory")
	SET(NOMACS_PLUGIN_INSTALL_DIRECTORY "NOT_SET" CACHE PATH "Path to the plugin install directory for deploying")
	IF (${NOMACS_BUILD_DIRECTORY} STREQUAL "NOT_SET")
		MESSAGE(FATAL_ERROR "You have to set the nomacs build directory")
	ENDIF()
endif()


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
NMC_FINDQT()
	          
# OpenCV
NMC_FIND_OPENCV()
	
include_directories (
	${QT_INCLUDES}
	${OpenCV_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${NOMACS_INCLUDE_DIRECTORY}
)

file(GLOB PLUGIN_SOURCES "src/*.cpp")
file(GLOB PLUGIN_HEADERS "src/*.h")
file(GLOB PLUGIN_MOCS "src/*.h" "${NOMACS_INCLUDE_DIRECTORY}/DkPluginInterface.h")

set (PLUGIN_RESOURCES
        src/nomacsPlugin.qrc
)

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_DLL)

QT5_WRAP_CPP(PLUGIN_MOC_SRC ${PLUGIN_MOCS})
QT5_ADD_RESOURCES(PLUGIN_RCC ${PLUGIN_RESOURCES})

link_directories(${OpenCV_LIBRARY_DIRS} ${NOMACS_BUILD_DIRECTORY}/$(CONFIGURATION) ${NOMACS_BUILD_DIRECTORY}/libs ${NOMACS_BUILD_DIRECTORY})
ADD_LIBRARY(${PROJECT_NAME} SHARED ${PLUGIN_SOURCES} ${PLUGIN_MOC_SRC} ${PLUGIN_RCC} ${PLUGIN_HEADERS})	
target_link_libraries(${PROJECT_NAME} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTMAIN_LIBRARY} ${OpenCV_LIBS} ${NOMACS_LIBS})

IF (MSVC)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${NOMACS_BUILD_DIRECTORY}/$<CONFIGURATION>/plugins/)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${NOMACS_BUILD_DIRECTORY}/$<CONFIGURATION>/plugins/)
	
	# write dll to d.txt (used for automated plugin download)
	if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/d.txt)
		file(READ ${CMAKE_CURRENT_SOURCE_DIR}/d.txt fileContent)	
		if(CMAKE_CL_64)
			string (REGEX MATCHALL "x64" matches ${fileContent})
			if(NOT matches)
				file(APPEND ${CMAKE_CURRENT_SOURCE_DIR}/d.txt "x64 x64/${PROJECT_NAME}.dll\n")
			endif()
		else()
			string (REGEX MATCHALL "x86" matches ${fileContent})		
			if(NOT matches)
				file(APPEND ${CMAKE_CURRENT_SOURCE_DIR}/d.txt "x86 x86/${PROJECT_NAME}.dll\n")
			endif()	
		endif(CMAKE_CL_64)
	else()
		if(CMAKE_CL_64)
			file(APPEND ${CMAKE_CURRENT_SOURCE_DIR}/d.txt "x64 x64/${PROJECT_NAME}.dll\n")
		else()
			file(APPEND ${CMAKE_CURRENT_SOURCE_DIR}/d.txt "x86 x86/${PROJECT_NAME}.dll\n")
		endif()	
	endif()
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/${PROJECT_NAME}.dll DESTINATION ${NOMACS_PLUGIN_INSTALL_DIRECTORY}/${PLUGIN_ID}/${PLUGIN_VERSION}/${PLUGIN_ARCHITECTURE}/ CONFIGURATIONS Release)
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/d.txt DESTINATION ${NOMACS_PLUGIN_INSTALL_DIRECTORY}/${PLUGIN_ID}/${PLUGIN_VERSION}/ CONFIGURATIONS Release)
	
ENDIF(MSVC)

qt5_use_modules(${PROJECT_NAME} Widgets Gui Network LinguistTools PrintSupport Concurrent)
