
PROJECT(ocrPlugin)

IF(EXISTS ${CMAKE_SOURCE_DIR}/CMakeUser.txt)
	include(${CMAKE_SOURCE_DIR}/CMakeUser.txt)
ENDIF()

# include macros needed
include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Utils.cmake")

NMC_POLICY()

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
file(GLOB PLUGIN_JSON "src/*.json")

NMC_PLUGIN_ID_AND_VERSION()

set (PLUGIN_RESOURCES
	src/nomacsPlugin.qrc
)
ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_DLL)

QT5_ADD_RESOURCES(PLUGIN_RCC ${PLUGIN_RESOURCES})

link_directories(${OpenCV_LIBRARY_DIRS} ${NOMACS_BUILD_DIRECTORY}/$(CONFIGURATION) ${NOMACS_BUILD_DIRECTORY}/libs ${NOMACS_BUILD_DIRECTORY})
ADD_LIBRARY(${PROJECT_NAME} SHARED ${PLUGIN_SOURCES} ${PLUGIN_MOC_SRC} ${PLUGIN_RCC} ${PLUGIN_HEADERS})	
target_link_libraries(${PROJECT_NAME} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTMAIN_LIBRARY} ${OpenCV_LIBS} ${NOMACS_LIBS})

## tesseract
find_package(Tesseract REQUIRED)
include_directories(${Tesseract_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${Tesseract_LIBRARIES})

## leptonica
#find_package(Leptonica REQUIRED CONFIG)
#target_link_libraries(${PROJECT_NAME} ${Leptonica_LIBRARIES})

message(${Tesseract_LIBRARIES})

message(${NOMACS_BUILD_DIRECTORY}/$<CONFIGURATION>/plugins/)
message(${DLL_DIR}/eng.traineddata)

set(DLL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# make plugins/tessdata directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${NOMACS_BUILD_DIRECTORY}/$<CONFIGURATION>/plugins/tessdata)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${DLL_DIR}/eng.traineddata ${NOMACS_BUILD_DIRECTORY}/$<CONFIGURATION>/plugins/tessdata)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${DLL_DIR}/deu.traineddata ${NOMACS_BUILD_DIRECTORY}/$<CONFIGURATION>/plugins/tessdata)

set(DLL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)
NMC_CREATE_TARGETS(${DLL_DIR}/lept174d.dll ${DLL_DIR}/tesseract305d.dll ${DLL_DIR}/eng.traineddata)
NMC_GENERATE_USER_FILE()
NMC_GENERATE_PACKAGE_XML(${PLUGIN_JSON})

qt5_use_modules(${PROJECT_NAME} Widgets Gui Network LinguistTools PrintSupport Concurrent)
