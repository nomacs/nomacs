set(NOMACS_RC src/nomacs.rc) #add resource file when compiling with MSVC
set(VERSION_LIB Version.lib)

# create the targets
set(BINARY_NAME ${PROJECT_NAME})
set(DLL_CORE_NAME ${PROJECT_NAME}Core)
set(DLL_LOADER_NAME ${PROJECT_NAME}Loader)
set(DLL_GUI_NAME ${PROJECT_NAME}Gui)

set(LIB_CORE_NAME optimized ${DLL_CORE_NAME}.lib debug ${DLL_CORE_NAME}d.lib)
set(LIB_LOADER_NAME optimized ${DLL_LOADER_NAME}.lib debug ${DLL_LOADER_NAME}d.lib)
set(LIB_NAME optimized ${DLL_GUI_NAME}.lib debug ${DLL_GUI_NAME}d.lib)


#binary
link_directories(${OpenCV_LIBRARY_DIRS} ${CMAKE_BINARY_DIR}/libs/)
set(CHANGLOG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/changelog.txt)
add_executable(${BINARY_NAME} WIN32  MACOSX_BUNDLE ${NOMACS_EXE_SOURCES} ${NOMACS_EXE_HEADERS} ${NOMACS_QM} ${NOMACS_RC} ${CHANGLOG_FILE}) #changelog is added here, so that it appears in visual studio
set_source_files_properties(${CHANGLOG_FILE} PROPERTIES HEADER_FILE_ONLY TRUE) # define that changelog should not be compiled
target_link_libraries(${BINARY_NAME} ${LIB_NAME} ${LIB_CORE_NAME} ${LIB_LOADER_NAME} ${EXIV2_LIBRARIES} ${LIBRAW_LIBRARIES} ${OpenCV_LIBS} ${VERSION_LIB} ${TIFF_LIBRARIES} ${HUPNP_LIBS} ${HUPNPAV_LIBS} ${QUAZIP_DEPENDENCY})

set_target_properties(${BINARY_NAME} PROPERTIES COMPILE_FLAGS "-DDK_DLL_IMPORT -DNOMINMAX")

# add core
add_library(${DLL_CORE_NAME} SHARED ${CORE_SOURCES} ${NOMACS_UI} ${CORE_HEADERS} ${NOMACS_RCC} ${NOMACS_RC})
target_link_libraries(${DLL_CORE_NAME} ${VERSION_LIB} ${OpenCV_LIBS})

# add loader
add_library(${DLL_LOADER_NAME} SHARED ${LOADER_SOURCES} ${NOMACS_UI} ${NOMACS_RCC} ${LOADER_HEADERS} ${AUTOFLOW_RC})
target_link_libraries(${DLL_LOADER_NAME} ${LIB_CORE_NAME} ${EXIV2_LIBRARIES} ${LIBRAW_LIBRARIES} ${OpenCV_LIBS} ${VERSION_LIB} ${TIFF_LIBRARIES} ${HUPNP_LIBS} ${HUPNPAV_LIBS} ${QUAZIP_DEPENDENCY})

# add GUI
add_library(${DLL_GUI_NAME} SHARED ${GUI_SOURCES} ${NOMACS_UI} ${NOMACS_RCC} ${GUI_HEADERS} ${NOMACS_RC})
target_link_libraries(${DLL_GUI_NAME} ${LIB_CORE_NAME} ${LIB_LOADER_NAME} ${EXIV2_LIBRARIES} ${LIBRAW_LIBRARIES} ${OpenCV_LIBS} ${VERSION_LIB} ${TIFF_LIBRARIES} ${HUPNP_LIBS} ${HUPNPAV_LIBS} ${QUAZIP_DEPENDENCY})


add_dependencies(${DLL_LOADER_NAME} ${DLL_CORE_NAME})
add_dependencies(${DLL_GUI_NAME} ${DLL_LOADER_NAME} ${DLL_CORE_NAME})
add_dependencies(${BINARY_NAME} ${DLL_GUI_NAME} ${DLL_LOADER_NAME} ${DLL_CORE_NAME} ${QUAZIP_DEPENDENCY} ${LIBQPSD_LIBRARY})

target_include_directories(${BINARY_NAME} 		PRIVATE ${OpenCV_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
target_include_directories(${DLL_GUI_NAME} 		PRIVATE ${OpenCV_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
target_include_directories(${DLL_LOADER_NAME} 	PRIVATE ${OpenCV_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
target_include_directories(${DLL_CORE_NAME} 	PRIVATE ${OpenCV_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})

qt5_use_modules(${BINARY_NAME} 		Widgets Gui Network LinguistTools PrintSupport Concurrent Svg)
qt5_use_modules(${DLL_GUI_NAME} 	Widgets Gui Network LinguistTools PrintSupport Concurrent Svg)
qt5_use_modules(${DLL_LOADER_NAME} 	Widgets Gui Network LinguistTools PrintSupport Concurrent Svg)
qt5_use_modules(${DLL_CORE_NAME} 	Widgets Gui Network LinguistTools PrintSupport Concurrent Svg)

# qt_wrap_cpp(${DLL_GUI_NAME} ${GUI_SOURCES} ${LOADER_HEADERS});

# core flags
set_target_properties(${DLL_CORE_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)
set_target_properties(${DLL_CORE_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)
set_target_properties(${DLL_CORE_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)
set_target_properties(${DLL_CORE_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)

set_target_properties(${DLL_CORE_NAME} PROPERTIES COMPILE_FLAGS "-DDK_CORE_DLL_EXPORT -DNOMINMAX")
set_target_properties(${DLL_CORE_NAME} PROPERTIES DEBUG_OUTPUT_NAME ${DLL_CORE_NAME}d)
set_target_properties(${DLL_CORE_NAME} PROPERTIES RELEASE_OUTPUT_NAME ${DLL_CORE_NAME})

# loader flags
set_target_properties(${DLL_LOADER_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)
set_target_properties(${DLL_LOADER_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)
set_target_properties(${DLL_LOADER_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)
set_target_properties(${DLL_LOADER_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)

set_target_properties(${DLL_LOADER_NAME} PROPERTIES COMPILE_FLAGS "-DDK_LOADER_DLL_EXPORT -DNOMINMAX")
set_target_properties(${DLL_LOADER_NAME} PROPERTIES DEBUG_OUTPUT_NAME ${DLL_LOADER_NAME}d)
set_target_properties(${DLL_LOADER_NAME} PROPERTIES RELEASE_OUTPUT_NAME ${DLL_LOADER_NAME})

# gui flags
set_target_properties(${DLL_GUI_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)
set_target_properties(${DLL_GUI_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)
set_target_properties(${DLL_GUI_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)
set_target_properties(${DLL_GUI_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)

set_target_properties(${DLL_GUI_NAME} PROPERTIES COMPILE_FLAGS "-DDK_GUI_DLL_EXPORT -DNOMINMAX")
set_target_properties(${DLL_GUI_NAME} PROPERTIES DEBUG_OUTPUT_NAME ${DLL_GUI_NAME}d)
set_target_properties(${DLL_GUI_NAME} PROPERTIES RELEASE_OUTPUT_NAME ${DLL_GUI_NAME})

target_link_libraries(${DLL_GUI_NAME} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTSVG_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTMAIN_LIBRARY} ${EXIV2_LIBRARIES} ${LIBRAW_LIBRARIES} ${OpenCV_LIBS} ${VERSION_LIB} ${TIFF_LIBRARIES} ${HUPNP_LIBS} ${HUPNPAV_LIBS} ${QUAZIP_DEPENDENCY})

set_target_properties(${DLL_GUI_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)
set_target_properties(${DLL_GUI_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)
set_target_properties(${DLL_GUI_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)
set_target_properties(${DLL_GUI_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)

set_target_properties(${DLL_GUI_NAME} PROPERTIES DEBUG_OUTPUT_NAME ${DLL_GUI_NAME}d)
set_target_properties(${DLL_GUI_NAME} PROPERTIES RELEASE_OUTPUT_NAME ${DLL_GUI_NAME})

# make RelWithDebInfo link against release instead of debug opencv dlls
set_target_properties(${OpenCV_LIBS} PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)

# copy required dlls to the directories
set(OpenCV_REQUIRED_MODULES core imgproc FORCE)
foreach(opencvlib ${OpenCV_REQUIRED_MODULES})
	file(GLOB dllpath ${OpenCV_DIR}/bin/Release/opencv_${opencvlib}*.dll)
	file(COPY ${dllpath} DESTINATION ${CMAKE_BINARY_DIR}/Release)
	file(COPY ${dllpath} DESTINATION ${CMAKE_BINARY_DIR}/RelWithDebInfo)
	file(COPY ${dllpath} DESTINATION ${CMAKE_BINARY_DIR}/MinSizeRel)

	file(GLOB dllpath ${OpenCV_DIR}/bin/Debug/opencv_${opencvlib}*d.dll)
	file(COPY ${dllpath} DESTINATION ${CMAKE_BINARY_DIR}/Debug)

endforeach(opencvlib)

set(QTLIBLIST Qt5Core Qt5Gui Qt5Network Qt5Widgets Qt5PrintSupport Qt5Concurrent Qt5Svg)
foreach(qtlib ${QTLIBLIST})
	get_filename_component(QT_DLL_PATH_tmp ${QT_QMAKE_EXECUTABLE} PATH)
	file(COPY ${QT_DLL_PATH_tmp}/${qtlib}.dll DESTINATION ${CMAKE_BINARY_DIR}/Release)
	file(COPY ${QT_DLL_PATH_tmp}/${qtlib}.dll DESTINATION ${CMAKE_BINARY_DIR}/RelWithDebInfo)
	file(COPY ${QT_DLL_PATH_tmp}/${qtlib}.dll DESTINATION ${CMAKE_BINARY_DIR}/MinSizeRel)
	file(COPY ${QT_DLL_PATH_tmp}/${qtlib}d.dll DESTINATION ${CMAKE_BINARY_DIR}/Debug)
endforeach(qtlib)

# copy additional Qt files
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release/imageformats)
file(GLOB QT_IMAGE_FORMATS "${QT_DLL_PATH_tmp}/../plugins/imageformats/*.dll")
file(COPY ${QT_IMAGE_FORMATS} DESTINATION ${CMAKE_BINARY_DIR}/Release/imageformats PATTERN *d.dll EXCLUDE)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release/platforms)
file(COPY ${QT_DLL_PATH_tmp}/../plugins/platforms/qwindows.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/platforms/)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release/printsupport)
file(COPY ${QT_DLL_PATH_tmp}/../plugins/printsupport/windowsprintersupport.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/printsupport)

# create settings file for portable version while working
if(NOT EXISTS ${CMAKE_BINARY_DIR}/RelWithDebInfo/settings.nfo)
	file(WRITE ${CMAKE_BINARY_DIR}/RelWithDebInfo/settings.nfo "")
endif()
if(NOT EXISTS ${CMAKE_BINARY_DIR}/MinSizeRel/settings.nfo)
	file(WRITE ${CMAKE_BINARY_DIR}/MinSizeRel/settings.nfo "")
endif()
if(NOT EXISTS ${CMAKE_BINARY_DIR}/Debug/settings.nfo)
	file(WRITE ${CMAKE_BINARY_DIR}/Debug/settings.nfo "")
endif()


# copy msvcpXXX.dll to Really Release
# todo: visual studio 2013
if (CMAKE_GENERATOR STREQUAL "Visual Studio 11" OR  CMAKE_GENERATOR STREQUAL "Visual Studio 11 Win64")
	set(VS_VERSION 11)
else()
	set(VS_VERSION 10)
endif()

if (MSVC11)
	get_filename_component(VS_DIR [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\11.0\\Setup\\VS;ProductDir] REALPATH CACHE)
	set(VS_VERSION 11)
elseif (MSVC10)
	get_filename_component(VS_DIR [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\10.0\\Setup\\VS;ProductDir] REALPATH CACHE)
	set(VS_VERSION 10)
elseif (MSVC12)
	get_filename_component(VS_DIR [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\12.0\\Setup\\VS;ProductDir] REALPATH CACHE)
	set(VS_VERSION 12)
endif()

if(CMAKE_CL_64)
	set(VC_RUNTIME_DIR "${VS_DIR}/VC/redist/x64/Microsoft.VC${VS_VERSION}0.CRT")
else()
	set(VC_RUNTIME_DIR "${VS_DIR}/VC/redist/x86/Microsoft.VC${VS_VERSION}0.CRT")
endif()

# copy translation files after each build
add_custom_command(TARGET ${BINARY_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory \"${CMAKE_BINARY_DIR}/$<CONFIGURATION>/translations/\")
foreach(QM ${NOMACS_QM})
	add_custom_command(TARGET ${BINARY_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy \"${QM}\" \"${CMAKE_BINARY_DIR}/$<CONFIGURATION>/translations/\")
endforeach(QM)

# add build incrementer command if requested
if (ENABLE_INCREMENTER)
	add_custom_command(TARGET ${DLL_GUI_NAME} POST_BUILD COMMAND cscript /nologo ${CMAKE_CURRENT_SOURCE_DIR}/src/incrementer.vbs ${CMAKE_CURRENT_SOURCE_DIR}/src/nomacs.rc)
	message(STATUS "build incrementer enabled...")
endif()

# set properties for Visual Studio Projects
add_definitions(/Zc:wchar_t-)
set(CMAKE_CXX_FLAGS_DEBUG "/W4 /EHsc ${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "/W4 /O2 /EHsc -DDK_INSTALL -DQT_NO_DEBUG_OUTPUT ${CMAKE_CXX_FLAGS_RELEASE}")

file(GLOB NOMACS_AUTOMOC "${CMAKE_BINARY_DIR}/*_automoc.cpp")
source_group("Generated Files" FILES ${NOMACS_RCC} ${NOMACS_UI} ${NOMACS_RC} ${NOMACS_QM} ${NOMACS_AUTOMOC})
#set_source_files_properties(${NOMACS_TRANSLATIONS} PROPERTIES HEADER_FILE_ONLY TRUE)
source_group("Translations" FILES ${NOMACS_TRANSLATIONS})
source_group("Changelog" FILES ${CHANGLOG_FILE})

# generate configuration file
if(DLL_CORE_NAME)
	get_property(CORE_DEBUG_NAME TARGET ${DLL_CORE_NAME} PROPERTY DEBUG_OUTPUT_NAME)
	get_property(CORE_RELEASE_NAME TARGET ${DLL_CORE_NAME} PROPERTY RELEASE_OUTPUT_NAME)
	set(NOMACS_CORE_LIB optimized ${CMAKE_BINARY_DIR}/libs/Release/${CORE_RELEASE_NAME}.lib debug  ${CMAKE_BINARY_DIR}/libs/Debug/${CORE_DEBUG_NAME}.lib)
endif()
if(DLL_LOADER_NAME)
	get_property(LOADER_DEBUG_NAME TARGET ${DLL_LOADER_NAME} PROPERTY DEBUG_OUTPUT_NAME)
	get_property(LOADER_RELEASE_NAME TARGET ${DLL_LOADER_NAME} PROPERTY RELEASE_OUTPUT_NAME)
	set(NOMACS_LOADER_LIB optimized ${CMAKE_BINARY_DIR}/libs/Release/${LOADER_RELEASE_NAME}.lib debug  ${CMAKE_BINARY_DIR}/libs/Debug/${LOADER_DEBUG_NAME}.lib)
endif()
if(DLL_GUI_NAME)
	get_property(GUI_DEBUG_NAME TARGET ${DLL_GUI_NAME} PROPERTY DEBUG_OUTPUT_NAME)
	get_property(GUI_RELEASE_NAME TARGET ${DLL_GUI_NAME} PROPERTY RELEASE_OUTPUT_NAME)
	set(NOMACS_GUI_LIB optimized ${CMAKE_BINARY_DIR}/libs/Release/${GUI_RELEASE_NAME}.lib debug  ${CMAKE_BINARY_DIR}/libs/Debug/${GUI_DEBUG_NAME}.lib)
endif()

set(NOMACS_LIBS ${NOMACS_CORE_LIB} ${NOMACS_LOADER_LIB} ${NOMACS_GUI_LIB})
set(NOMACS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(NOMACS_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/DkGui ${CMAKE_CURRENT_SOURCE_DIR}/src/DkCore ${CMAKE_CURRENT_SOURCE_DIR}/src/DkLoader ${CMAKE_BINARY_DIR})
configure_file(${NOMACS_SOURCE_DIR}/nomacs.cmake.in ${CMAKE_BINARY_DIR}/nomacsConfig.cmake)
