
PROJECT(affineTransformPlugin)

IF(EXISTS ${CMAKE_SOURCE_DIR}/CMakeUser.txt)
	include(${CMAKE_SOURCE_DIR}/CMakeUser.txt)
ENDIF()

# include macros needed
include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Utils.cmake")

######################################################### CHANGE THIS FOR NEW PLUGINS ###################################################
set(PLUGIN_VERSION 1.0.3)
set(PLUGIN_ID 7a4a50a45ddb408ba46a16a40e6518fd) #GUID without hyphens generated at http://www.guidgenerator.com/
######################################################### CHANGE THIS FOR NEW PLUGINS ###################################################

add_definitions(-DPLUGIN_VERSION="${PLUGIN_VERSION}")
add_definitions(-DPLUGIN_ID="${PLUGIN_ID}")



if (NOT BUILDING_MULTIPLE_PLUGINS)
  # prepare plugin
  NMC_PREPARE_PLUGIN()
  
  # find the Qt
  NMC_FINDQT()

  # OpenCV
  NMC_FIND_OPENCV()
endif()
	
include_directories (
	${QT_INCLUDES}
	${OpenCV_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${NOMACS_INCLUDE_DIRECTORY}
)

file(GLOB PLUGIN_SOURCES "src/*.cpp")
file(GLOB PLUGIN_HEADERS "src/*.h" "${NOMACS_INCLUDE_DIRECTORY}/DkPluginInterface.h")
# file(GLOB PLUGIN_MOCS "src/*.h" "${NOMACS_INCLUDE_DIRECTORY}/DkPluginInterface.h")

set (PLUGIN_RESOURCES
        src/nomacsPlugin.qrc
)

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_DLL)

# QT5_WRAP_CPP(PLUGIN_MOC_SRC ${PLUGIN_MOCS})
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
	
elseif(UNIX)
	install(TARGETS ${PROJECT_NAME} RUNTIME LIBRARY DESTINATION lib/nomacs-plugins)
endif(MSVC)


qt5_use_modules(${PROJECT_NAME} Widgets Gui Network LinguistTools PrintSupport Concurrent)
