set(NOMACS_RC src/nomacs.rc) #add resource file when compiling with MSVC 
set(VERSION_LIB Version.lib)
set(EXIV2_LIBRARY_DIRS ${CMAKE_CURRENT_BINARY_DIR}/libs) #add libs directory to library dirs 


# create the targets
set(BINARY_NAME ${CMAKE_PROJECT_NAME})
set(DLL_NAME lib${CMAKE_PROJECT_NAME})
set(LIB_NAME optimized ${DLL_NAME}.lib debug ${DLL_NAME}d.lib)
LIST(REMOVE_ITEM NOMACS_SOURCES ${CMAKE_SOURCE_DIR}/src/main.cpp)
link_directories(${LIBRAW_LIBRARY_DIRS} ${OpenCV_LIBRARY_DIRS} ${EXIV2_LIBRARY_DIRS})
add_executable(${BINARY_NAME} WIN32  MACOSX_BUNDLE src/main.cpp ${NOMACS_QM} ${NOMACS_TRANSLATIONS} ${NOMACS_RC})

target_link_libraries(${BINARY_NAME} ${QT_QTCORE_LIBRARY} ${QT_QTSVG_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTMAIN_LIBRARY} ${VERSION_LIB} ${LIB_NAME})

set_target_properties(${BINARY_NAME} PROPERTIES COMPILE_FLAGS "-DDK_DLL_IMPORT -DNOMINMAX")
set_target_properties(${BINARY_NAME} PROPERTIES LINK_FLAGS_REALLYRELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS /LARGEADDRESSAWARE")
set_target_properties(${BINARY_NAME} PROPERTIES LINK_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:CONSOLE /LARGEADDRESSAWARE")
set_target_properties(${BINARY_NAME} PROPERTIES LINK_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /SUBSYSTEM:CONSOLE /LARGEADDRESSAWARE")
set_target_properties(${BINARY_NAME} PROPERTIES IMPORTED_IMPLIB "")
		
add_library(${DLL_NAME} SHARED ${NOMACS_SOURCES} ${NOMACS_UI} ${NOMACS_RESOURCES} ${NOMACS_HEADERS} ${NOMACS_RC})
target_include_directories(${DLL_NAME} PRIVATE  ${OpenCV_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
target_include_directories(${BINARY_NAME} PRIVATE  ${OpenCV_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
target_link_libraries(${DLL_NAME} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTSVG_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTMAIN_LIBRARY} ${EXIV2_LIBRARIES} ${LIBRAW_LIBRARIES} ${OpenCV_LIBS} ${VERSION_LIB} ${TIFF_LIBRARIES} ${HUPNP_LIBS} ${HUPNPAV_LIBS} ${QUAZIP_DEPENDENCY} ${WEBP_LIBRARY}) 
add_dependencies(${BINARY_NAME} ${DLL_NAME} ${QUAZIP_DEPENDENCY} ${LIBQPSD_LIBRARY} ${WEBP_LIBRARY}) #QUAZIP_DEPENDENCY, LIBQPSD_LIBRARY, and WEBP_LIBRARY are empty when disabled

qt5_use_modules(${BINARY_NAME} 	Widgets Gui Network LinguistTools PrintSupport Concurrent Svg)
qt5_use_modules(${DLL_NAME} 	Widgets Gui Network LinguistTools PrintSupport Concurrent Svg)

SET(CMAKE_SHARED_LINKER_FLAGS_REALLYRELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS /LARGEADDRESSAWARE") # /subsystem:windows does not work due to a bug in cmake (see http://public.kitware.com/Bug/view.php?id=12566)

set_target_properties(${DLL_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/libs)
set_target_properties(${DLL_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/libs)
set_target_properties(${DLL_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_REALLYRELEASE ${CMAKE_CURRENT_BINARY_DIR}/libs)
				
set_target_properties(${DLL_NAME} PROPERTIES COMPILE_FLAGS "-DDK_DLL_EXPORT -DNOMINMAX")
set_target_properties(${DLL_NAME} PROPERTIES LINK_FLAGS_REALLYRELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS /LARGEADDRESSAWARE")
set_target_properties(${DLL_NAME} PROPERTIES LINK_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:CONSOLE /LARGEADDRESSAWARE")
set_target_properties(${DLL_NAME} PROPERTIES LINK_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /SUBSYSTEM:CONSOLE /LARGEADDRESSAWARE")
set_target_properties(${DLL_NAME} PROPERTIES DEBUG_OUTPUT_NAME ${DLL_NAME}d)
set_target_properties(${DLL_NAME} PROPERTIES RELEASE_OUTPUT_NAME ${DLL_NAME})

# copy required dlls to the directories
set(OpenCV_REQUIRED_MODULES core imgproc FORCE)
foreach(opencvlib ${OpenCV_REQUIRED_MODULES})
	file(GLOB dllpath ${OpenCV_DIR}/bin/Release/opencv_${opencvlib}*.dll)
	file(COPY ${dllpath} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
	file(COPY ${dllpath} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)
	
	file(GLOB dllpath ${OpenCV_DIR}/bin/Debug/opencv_${opencvlib}*d.dll)
	file(COPY ${dllpath} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
endforeach(opencvlib)

set(QTLIBLIST Qt5Core Qt5Gui Qt5Network Qt5Widgets Qt5PrintSupport Qt5Concurrent Qt5Svg)
foreach(qtlib ${QTLIBLIST})
	get_filename_component(QT_DLL_PATH_tmp ${QT_QMAKE_EXECUTABLE} PATH)
	file(COPY ${QT_DLL_PATH_tmp}/${qtlib}.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
	file(COPY ${QT_DLL_PATH_tmp}/${qtlib}.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)
	file(COPY ${QT_DLL_PATH_tmp}/${qtlib}d.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
endforeach(qtlib)

# create settings file for portable version while working
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/Release/settings.nfo)
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/Release/settings.nfo "")
endif()
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/Debug/settings.nfo)
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/Debug/settings.nfo "")
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

# copy msvc dlls
if (MSVC14)

	# diem: TODO not working yet!
	find_file(MSVCP NAMES msvcp140.dll PATHS ${VC_RUNTIME_DIR} NO_DEFAULT_PATH)
	find_file(MSVCR NAMES msvcr140.dll PATHS ${VC_RUNTIME_DIR} NO_DEFAULT_PATH)
	if(MSVCP)
		file(COPY ${MSVCP} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)
		file(COPY ${MSVCR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)
	else()
		message(STATUS "Could not copy msvcp140.dll from ${VC_RUNTIME_DIR}")
		message(STATUS  "\nInfo: Could not find the correct msvcp libraries. You have to copy them manually to ReallyRelease if you want to distribute nomacs")
	endif()

else ()
	find_file(MSVCP NAMES msvcp${VS_VERSION}0.dll PATHS ${VC_RUNTIME_DIR} NO_DEFAULT_PATH)
	find_file(MSVCR NAMES msvcr${VS_VERSION}0.dll PATHS ${VC_RUNTIME_DIR} NO_DEFAULT_PATH)
	if(MSVCP)
		file(COPY ${MSVCP} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)
		file(COPY ${MSVCR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)
	else()
		message(STATUS "Could not copy msvcp${VS_VERSION}0.dll")
		message(STATUS  "\nInfo: Could not find the correct msvcp libraries. You have to copy them manually to ReallyRelease if you want to distribute nomacs")
	endif()
endif() # if (MSVC14)

# copy translation files after each build
add_custom_command(TARGET ${BINARY_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory \"${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/translations/\")
foreach(QM ${NOMACS_QM})
	add_custom_command(TARGET ${BINARY_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy \"${QM}\" \"${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/translations/\")
endforeach(QM)


# set properties for Visual Studio Projects
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;ReallyRelease" CACHE STRING "limited configs" FORCE)
add_definitions(/Zc:wchar_t-)
set(CMAKE_CXX_FLAGS_DEBUG "/W4 ${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "/W4 /O2 ${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_REALLYRELEASE "${CMAKE_CXX_FLAGS_RELEASE}  /DQT_NO_DEBUG_OUTPUT")
set(CMAKE_EXE_LINKER_FLAGS_REALLYRELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS /LARGEADDRESSAWARE") # /subsystem:windows does not work due to a bug in cmake (see http://public.kitware.com/Bug/view.php?id=12566)

set_target_properties(${EXE_NAME} PROPERTIES LINK_FLAGS_REALLYRELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS /LARGEADDRESSAWARE")
set_target_properties(${EXE_NAME} PROPERTIES LINK_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:CONSOLE /LARGEADDRESSAWARE")
set_target_properties(${EXE_NAME} PROPERTIES LINK_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /SUBSYSTEM:CONSOLE /LARGEADDRESSAWARE")

file(GLOB NOMACS_AUTOMOC "${CMAKE_BINARY_DIR}/*_automoc.cpp")
source_group("Generated Files" FILES ${NOMACS_UI} ${NOMACS_RC} ${NOMACS_QM} ${NOMACS_AUTOMOC})
set_source_files_properties(${NOMACS_TRANSLATIONS} PROPERTIES HEADER_FILE_ONLY TRUE)
source_group("Translations" FILES ${NOMACS_TRANSLATIONS})

# generate configuration file
if(DLL_NAME)
	get_property(NOMACS_DEBUG_NAME TARGET ${DLL_NAME} PROPERTY DEBUG_OUTPUT_NAME)
	get_property(NOMACS_RELEASE_NAME TARGET ${DLL_NAME} PROPERTY RELEASE_OUTPUT_NAME)
	set(NOMACS_LIBS optimized ${CMAKE_BINARY_DIR}/libs/${NOMACS_RELEASE_NAME}.lib debug  ${CMAKE_BINARY_DIR}/libs/${NOMACS_DEBUG_NAME}.lib)
endif()
set(NOMACS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(NOMACS_BUILD_DIRECTORY ${CMAKE_BINARY_DIR})
configure_file(${NOMACS_SOURCE_DIR}/nomacs.cmake.in ${CMAKE_BINARY_DIR}/nomacsConfig.cmake)