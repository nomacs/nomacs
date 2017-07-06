set(NOMACS_RC src/nomacs.rc) #add resource file when compiling with MSVC
set(VERSION_LIB Version.lib)

# create the targets
set(BINARY_NAME ${PROJECT_NAME})
set(DLL_CORE_NAME ${PROJECT_NAME}Core)
set(LIB_CORE_NAME optimized ${DLL_CORE_NAME}.lib debug ${DLL_CORE_NAME}d.lib)

#binary
link_directories(${OpenCV_LIBRARY_DIRS} ${CMAKE_BINARY_DIR}/libs/)
set(CHANGLOG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/changelog.txt)

add_executable(
	${BINARY_NAME} WIN32  MACOSX_BUNDLE 
	${NOMACS_EXE_SOURCES} ${NOMACS_EXE_HEADERS} 
	${NOMACS_QM} ${NOMACS_RC} ${CHANGLOG_FILE}
	)
# define that changelog should not be compiled
set_source_files_properties(${CHANGLOG_FILE} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(
	${BINARY_NAME} 
	${LIB_CORE_NAME} 
	${EXIV2_LIBRARIES} 
	${LIBRAW_LIBRARIES} 
	${OpenCV_LIBS} 
	${TIFF_LIBRARIES} 
	${QUAZIP_DEPENDENCY}
	)
	
set_target_properties(${BINARY_NAME} PROPERTIES COMPILE_FLAGS "-DDK_DLL_IMPORT -DNOMINMAX")

if (ENABLE_READ_BUILD)
	set_target_properties(${BINARY_NAME} PROPERTIES COMPILE_FLAGS "-DREAD_TUWIEN")
endif()

# add DLL
add_library(
	${DLL_CORE_NAME} SHARED 
	${GUI_HEADERS} ${GUI_SOURCES} 
	${CORE_HEADERS} ${CORE_SOURCES} 
	${NOMACS_RCC} ${NOMACS_RC}
	)
target_link_libraries(
	${DLL_CORE_NAME}
	${EXIV2_LIBRARIES} 		# metadata support
	${VERSION_LIB} 			# needed for registering the curren version
	${LIBRAW_LIBRARIES} 	# RAW support (optional)
	${OpenCV_LIBS} 			# image manipulation support (optional)
	${TIFF_LIBRARIES} 			# multip page tiff support (optional)
	${QUAZIP_DEPENDENCY}	# ZIP support (optional)
	)
# ${VERSION_LIB} 

add_dependencies(
	${BINARY_NAME} 
	${DLL_CORE_NAME} 
	${QUAZIP_DEPENDENCY} 
	${LIBQPSD_LIBRARY}
	)

target_include_directories(${BINARY_NAME} 		PRIVATE ${OpenCV_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
target_include_directories(${DLL_CORE_NAME} 	PRIVATE ${OpenCV_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})

qt5_use_modules(${BINARY_NAME} 		Widgets Gui Network LinguistTools PrintSupport Concurrent Svg WinExtras)
qt5_use_modules(${DLL_CORE_NAME} 	Widgets Gui Network LinguistTools PrintSupport Concurrent Svg WinExtras)

# set(_moc ${CMAKE_CURRENT_BINARY_DIR}/GeneratedFiles)
file(GLOB NOMACS_AUTOMOC "${CMAKE_BINARY_DIR}/*_automoc.cpp ${CMAKE_BINARY_DIR}/moc_.cpp")

# core flags
set_target_properties(${DLL_CORE_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)
set_target_properties(${DLL_CORE_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)
set_target_properties(${DLL_CORE_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)
set_target_properties(${DLL_CORE_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)

set_target_properties(${DLL_CORE_NAME} PROPERTIES COMPILE_FLAGS "-DDK_CORE_DLL_EXPORT -DNOMINMAX")
set_target_properties(${DLL_CORE_NAME} PROPERTIES DEBUG_OUTPUT_NAME ${DLL_CORE_NAME}d)
set_target_properties(${DLL_CORE_NAME} PROPERTIES RELEASE_OUTPUT_NAME ${DLL_CORE_NAME})

# make RelWithDebInfo link against release instead of debug opencv dlls
set_target_properties(${OpenCV_LIBS} PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)
set_target_properties(${OpenCV_LIBS} PROPERTIES MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE)

# copy additional Qt files

# add image plugins
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release/imageformats)
file(GLOB QT_IMAGE_FORMATS "${QT_QMAKE_PATH}/../plugins/imageformats/*.dll")
file(COPY ${QT_IMAGE_FORMATS} DESTINATION ${CMAKE_BINARY_DIR}/Release/imageformats PATTERN *d.dll EXCLUDE)
file(COPY ${QT_IMAGE_FORMATS} DESTINATION ${CMAKE_BINARY_DIR}/RelWithDebInfo/imageformats PATTERN *d.dll EXCLUDE)
file(COPY ${QT_IMAGE_FORMATS} DESTINATION ${CMAKE_BINARY_DIR}/Debug/imageformats)

file(GLOB QT_EXTRA_IMAGE_FORMATS "${QT_QMAKE_PATH}/../../qtimageformats/plugins/imageformats/*.dll")
file(COPY ${QT_EXTRA_IMAGE_FORMATS} DESTINATION ${CMAKE_BINARY_DIR}/Release/imageformats PATTERN *d.dll EXCLUDE)
file(COPY ${QT_EXTRA_IMAGE_FORMATS} DESTINATION ${CMAKE_BINARY_DIR}/RelWithDebInfo/imageformats PATTERN *d.dll EXCLUDE)
file(COPY ${QT_EXTRA_IMAGE_FORMATS} DESTINATION ${CMAKE_BINARY_DIR}/Debug/imageformats)

# Platforms
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release/platforms)
file(COPY ${QT_QMAKE_PATH}/../plugins/platforms/qwindows.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/platforms/)
file(COPY ${QT_QMAKE_PATH}/../plugins/platforms/qwindows.dll DESTINATION ${CMAKE_BINARY_DIR}/RelWithDebInfo/platforms/)
file(COPY ${QT_QMAKE_PATH}/../plugins/platforms/qwindowsd.dll DESTINATION ${CMAKE_BINARY_DIR}/Debug/platforms/)

# PrintSupport
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release/printsupport)
file(COPY ${QT_QMAKE_PATH}/../plugins/printsupport/windowsprintersupport.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/printsupport)
file(COPY ${QT_QMAKE_PATH}/../plugins/printsupport/windowsprintersupport.dll DESTINATION ${CMAKE_BINARY_DIR}/RelWithDebInfo/printsupport)
file(COPY ${QT_QMAKE_PATH}/../plugins/printsupport/windowsprintersupportd.dll DESTINATION ${CMAKE_BINARY_DIR}/Debug/printsupport)

if (NOT Qt5Widgets_VERSION VERSION_LESS 5.9.0)
	# WinExtras

	file(COPY ${QT_QMAKE_PATH}/../../qtwinextras/bin/Qt5WinExtras.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
	file(COPY ${QT_QMAKE_PATH}/../../qtwinextras/bin/Qt5WinExtras.dll DESTINATION ${CMAKE_BINARY_DIR}/RelWithDebInfo/)
	file(COPY ${QT_QMAKE_PATH}/../../qtwinextras/bin/Qt5WinExtrasd.dll DESTINATION ${CMAKE_BINARY_DIR}/Debug/)

	# SVG support
	file(COPY ${QT_QMAKE_PATH}/../../qtsvg/bin/Qt5Svg.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
	file(COPY ${QT_QMAKE_PATH}/../../qtsvg/bin/Qt5Svg.dll DESTINATION ${CMAKE_BINARY_DIR}/RelWithDebInfo/)
	file(COPY ${QT_QMAKE_PATH}/../../qtsvg/bin/Qt5Svgd.dll DESTINATION ${CMAKE_BINARY_DIR}/Debug/)

	file(COPY ${QT_QMAKE_PATH}/../../qtsvg/plugins/imageformats/qsvg.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/imageformats)
	file(COPY ${QT_QMAKE_PATH}/../../qtsvg/plugins/imageformats/qsvg.dll DESTINATION ${CMAKE_BINARY_DIR}/RelWithDebInfo/imageformats)
	file(COPY ${QT_QMAKE_PATH}/../../qtsvg/plugins/imageformats/qsvgd.dll DESTINATION ${CMAKE_BINARY_DIR}/Debug/imageformats)
endif()

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

# copy translation files after each build
add_custom_command(TARGET ${BINARY_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory \"${CMAKE_BINARY_DIR}/$<CONFIGURATION>/translations/\")
foreach(QM ${NOMACS_QM})
	add_custom_command(TARGET ${BINARY_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy \"${QM}\" \"${CMAKE_BINARY_DIR}/$<CONFIGURATION>/translations/\")
endforeach(QM)

# add build incrementer command if requested
if (ENABLE_INCREMENTER)
	add_custom_command(TARGET ${DLL_CORE_NAME} POST_BUILD COMMAND cscript /nologo ${CMAKE_CURRENT_SOURCE_DIR}/src/incrementer.vbs ${CMAKE_CURRENT_SOURCE_DIR}/src/nomacs.rc)
	message(STATUS "build incrementer enabled...")
endif()

# set properties for Visual Studio Projects
add_definitions(/Zc:wchar_t-)
set(CMAKE_CXX_FLAGS_DEBUG "/W4 /EHsc ${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "/W4 /O2 /EHsc -DDK_INSTALL -DQT_NO_DEBUG_OUTPUT ${CMAKE_CXX_FLAGS_RELEASE}")

source_group("Generated Files" FILES ${NOMACS_RCC} ${NOMACS_RC} ${NOMACS_QM} ${NOMACS_AUTOMOC})
source_group("Translations" FILES ${NOMACS_TRANSLATIONS})
source_group("Changelog" FILES ${CHANGLOG_FILE})

# generate configuration file
if(DLL_CORE_NAME)
	get_property(CORE_DEBUG_NAME TARGET ${DLL_CORE_NAME} PROPERTY DEBUG_OUTPUT_NAME)
	get_property(CORE_RELEASE_NAME TARGET ${DLL_CORE_NAME} PROPERTY RELEASE_OUTPUT_NAME)
	set(NOMACS_CORE_LIB optimized ${CMAKE_BINARY_DIR}/libs/Release/${CORE_RELEASE_NAME}.lib debug  ${CMAKE_BINARY_DIR}/libs/Debug/${CORE_DEBUG_NAME}.lib)
endif()

set(NOMACS_LIBS ${NOMACS_CORE_LIB})
set(NOMACS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(NOMACS_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/DkGui ${CMAKE_CURRENT_SOURCE_DIR}/src/DkCore ${CMAKE_CURRENT_SOURCE_DIR}/src/DkLoader ${CMAKE_BINARY_DIR})
configure_file(${NOMACS_SOURCE_DIR}/nomacs.cmake.in ${CMAKE_BINARY_DIR}/nomacsConfig.cmake)

### DependencyCollector
set(DC_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/cmake/DependencyCollector.py)
set(DC_CONFIG ${CMAKE_BINARY_DIR}/DependencyCollector.ini)

GET_FILENAME_COMPONENT(VS_PATH ${CMAKE_LINKER} PATH)
if(CMAKE_CL_64)
	SET(VS_PATH "${VS_PATH}/../../../Common7/IDE/Remote Debugger/x64")
else()
	SET(VS_PATH "${VS_PATH}/../../Common7/IDE/Remote Debugger/x86")
endif()
SET(DC_PATHS_RELEASE ${EXIV2_BUILD_PATH}/ReleaseDLL ${LIBRAW_BUILD_PATH}/Release ${OpenCV_DIR}/bin/Release ${QT_QMAKE_PATH} ${VS_PATH})
SET(DC_PATHS_DEBUG ${EXIV2_BUILD_PATH}/DebugDLL ${LIBRAW_BUILD_PATH}/Debug ${OpenCV_DIR}/bin/Debug ${QT_QMAKE_PATH} ${VS_PATH})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/DependencyCollector.config.cmake.in ${DC_CONFIG})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND python ${DC_SCRIPT} --infile $<TARGET_FILE:${PROJECT_NAME}> --configfile ${DC_CONFIG} --configuration $<CONFIGURATION>)



