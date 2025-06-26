
PROJECT(CompositePlugin)

IF(EXISTS ${CMAKE_SOURCE_DIR}/CMakeUser.txt)
	include(${CMAKE_SOURCE_DIR}/CMakeUser.txt)
ENDIF()

# include macros needed
include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Utils.cmake")

NMC_POLICY()

if (NOT BUILDING_MULTIPLE_PLUGINS)
  # prepare plugin
  NMC_PREPARE_PLUGIN()
  
  # find the Qt
  NMC_FINDQT()

  # OpenCV
  NMC_FIND_OPENCV("core" "imgproc")
endif()

include_directories (
	${QT_INCLUDES}
	${OpenCV_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${NOMACS_INCLUDE_DIRECTORY}
)

file(GLOB PLUGIN_SOURCES "src/*.cpp")
file(GLOB PLUGIN_HEADERS "src/*.h" "${NOMACS_INCLUDE_DIRECTORY}/DkPluginInterface.h")
file(GLOB PLUGIN_JSON "src/*.json")

NMC_PLUGIN_ID_AND_VERSION()

# uncomment if you want to add the plugin version or id
# add_definitions(-DPLUGIN_VERSION="${PLUGIN_VERSION}")
# add_definitions(-DPLUGIN_ID="${PLUGIN_ID}")

set (PLUGIN_RESOURCES
	src/nomacsPlugin.qrc
)

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_DLL)

qt_add_resources(PLUGIN_RCC ${PLUGIN_RESOURCES})

link_directories(${OpenCV_LIBRARY_DIRS} ${NOMACS_BUILD_DIRECTORY}/libs ${NOMACS_BUILD_DIRECTORY})
ADD_LIBRARY(${PROJECT_NAME} SHARED ${PLUGIN_SOURCES} ${PLUGIN_MOC_SRC} ${PLUGIN_RCC} ${PLUGIN_HEADERS})	
target_link_libraries(${PROJECT_NAME} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTMAIN_LIBRARY} ${OpenCV_LIBS} ${NOMACS_LIBS})
target_link_libraries(${PROJECT_NAME} Qt::Widgets Qt::Gui Qt::Network)

NMC_CREATE_TARGETS()
NMC_GENERATE_USER_FILE()
NMC_GENERATE_PACKAGE_XML(${PLUGIN_JSON})

