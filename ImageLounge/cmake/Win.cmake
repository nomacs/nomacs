# nomacs cmake file for a windows build

# load pathes from the user file if exists 
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/CMakeUser.txt)
	include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeUser.txt)
endif()

if(ENABLE_PLUGINS)
	add_definitions(-DWITH_PLUGINS)
endif()


# prepare build directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libs)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Debug)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Release)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)

# search for exiv2
if(CMAKE_CL_64)
	SET(EXIV_SEARCH_PATH "../exiv2-0.23/msvc64/exiv2lib/x64/" )
else()
	SET(EXIV_SEARCH_PATH "../exiv2-0.23/msvc64/exiv2lib/Win32/" )
endif()

find_path(EXIV2_BUILD_PATH NAMES "ReleaseDLL/exiv2.lib"
									"ReleaseDLL/exiv2.dll"
									"DebugDLL/exiv2d.lib"
									"DebugDLL/exiv2d.dll"
				PATHS ${EXIV_SEARCH_PATH}
				DOC "Path to the exiv2 build directory" NO_DEFAULT_PATH)
				
find_path(EXIV2_INCLUDE_DIRS "exiv2/exiv2.hpp" 
				PATHS "../exiv2-0.23/msvc64/include" 
				DOC "Path to exiv2/exiv2.hpp" NO_DEFAULT_PATH)
MARK_AS_ADVANCED(EXIV2_INCLUDE_DIRS)

# copy files to the build directory
if( EXISTS ${EXIV2_BUILD_PATH}/ReleaseDLL/exiv2.dll AND
	EXISTS ${EXIV2_BUILD_PATH}/DebugDLL/exiv2d.dll AND 
	EXISTS ${EXIV2_BUILD_PATH}/ReleaseDLL/exiv2.lib AND 
	EXISTS ${EXIV2_BUILD_PATH}/DebugDLL/exiv2d.lib)
	if(EXISTS ${EXIV2_BUILD_PATH}/ReleaseDLL/libexpat.dll AND 
		EXISTS ${EXIV2_BUILD_PATH}/DebugDLL/libexpat.dll)
		if(EXISTS ${EXIV2_BUILD_PATH}/ReleaseDLL/zlib1.dll AND
			EXISTS ${EXIV2_BUILD_PATH}/DebugDLL/zlib1d.dll )
	
			file(COPY ${EXIV2_BUILD_PATH}/ReleaseDLL/exiv2.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
			file(COPY ${EXIV2_BUILD_PATH}/ReleaseDLL/exiv2.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)
			file(COPY ${EXIV2_BUILD_PATH}/ReleaseDLL/exiv2.lib DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/libs)
			file(COPY ${EXIV2_BUILD_PATH}/DebugDLL/exiv2d.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
			file(COPY ${EXIV2_BUILD_PATH}/DebugDLL/exiv2d.lib DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/libs)

			file(COPY ${EXIV2_BUILD_PATH}/ReleaseDLL/libexpat.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
			file(COPY ${EXIV2_BUILD_PATH}/ReleaseDLL/libexpat.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)
			file(COPY ${EXIV2_BUILD_PATH}/DebugDLL/libexpat.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)

			file(COPY ${EXIV2_BUILD_PATH}/ReleaseDLL/zlib1.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
			file(COPY ${EXIV2_BUILD_PATH}/ReleaseDLL/zlib1.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)
			file(COPY ${EXIV2_BUILD_PATH}/DebugDLL/zlib1d.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
			
			set(EXIV2_LIBRARIES optimized exiv2.lib debug exiv2d.lib)
			set(EXIV2_LIBRARY_DIRS "")
			set(EXIV2_FOUND true)
		else()
			message(WARNING "zlib build directory not found. Needs EXIV2_BUILD_PATH which contains ReleaseDLL/zlib1.dll and DebugDLL/zlib1d.dll")
		endif()
	else()
		message(WARNING "expat build directory not found. Needs EXIV2_BUILD_PATH which contains ReleaseDLL/libexpat.dll and DebugDLL/libexpat.dll")
	endif()
else()
	message(WARNING "exiv build directory not found. Needs EXIV2_BUILD_PATH which contains ReleaseDLL/exiv2.dll, ReleaseDLL/exiv2.lib, DebugDLL/exiv2d.dll and DebugDLL/exiv2d.lib")
endif()	

# search for opencv
unset(OpenCV_LIB_DIR_DBG CACHE)
unset(OpenCV_3RDPARTY_LIB_DIR_DBG CACHE)
unset(OpenCV_3RDPARTY_LIB_DIR_OPT CACHE)
unset(OpenCV_CONFIG_PATH CACHE)
unset(OpenCV_LIB_DIR_DBG CACHE)
unset(OpenCV_LIB_DIR_OPT CACHE)

if(ENABLE_OPENCV)
	find_package(OpenCV 2.1.0 REQUIRED core imgproc)
	if(OpenCV_VERSION VERSION_LESS 2.4.0 AND OpenCV_FOUND) # OpenCV didn't allow to define packages before version 2.4.0 ... nomacs was linking against all libs even if they were not compiled -> error
		string(REGEX REPLACE "\\." "" OpenCV_SHORT_VERSION ${OpenCV_VERSION})
		set(OpenCV_LIBS "debug;opencv_imgproc${OpenCV_SHORT_VERSION}d;optimized;opencv_imgproc${OpenCV_SHORT_VERSION};debug;opencv_core${OpenCV_SHORT_VERSION}d;optimized;opencv_core${OpenCV_SHORT_VERSION};")
	endif()

	if(NOT OpenCV_FOUND)
		message(FATAL_ERROR "OpenCV not found.") 
	else()
		add_definitions(-DWITH_OPENCV)
	endif()

	if(${OpenCV_VERSION} EQUAL "2.1.0")
		add_definitions(-DDISABLE_LANCZOS)
	endif()
endif(ENABLE_OPENCV)

# search for libraw
if(ENABLE_RAW)
	if(NOT OpenCV_FOUND)
		message(FATAL_ERROR "OpenCV is mandotory when enabling RAW. You have to enable ENABLE_OPENCV")
	endif()

	find_path(LIBRAW_INCLUDE_DIRS "libraw/libraw.h" PATHS "../LibRaw-0.16.0/" DOC "Path to libraw/libraw.h" NO_DEFAULT_PATH)

	if(CMAKE_CL_64)
		set(LIBRAW_SEARCH_PATH "../LibRaw-0.16.0/buildfiles/x64")
	else()
		set(LIBRAW_SEARCH_PATH "../LibRaw-0.16.0/buildfiles/Win32")
	endif()

	find_path(LIBRAW_BUILD_PATH NAMES "release/libraw.lib" "release/libraw.dll" "debug/libraw.lib" "debug/libraw.dll"
								PATHS ${LIBRAW_SEARCH_PATH} DOC "Path to the libraw build directory" NO_DEFAULT_PATH)

	if(EXISTS ${LIBRAW_BUILD_PATH}/release/libraw.dll AND
		EXISTS ${LIBRAW_BUILD_PATH}/debug/libraw.dll AND 
		EXISTS ${LIBRAW_BUILD_PATH}/release/libraw.lib AND 
		EXISTS ${LIBRAW_BUILD_PATH}/debug/libraw.lib)

			file(COPY ${LIBRAW_BUILD_PATH}/release/libraw.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
			file(COPY ${LIBRAW_BUILD_PATH}/release/libraw.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)
			file(COPY ${LIBRAW_BUILD_PATH}/release/libraw.lib DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/libs)
			file(COPY ${LIBRAW_BUILD_PATH}/debug/libraw.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
			file(COPY ${LIBRAW_BUILD_PATH}/debug/libraw.lib DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/libs)

			set(LIBRAW_LIBRARY_DIRS "")
			set(LIBRAW_LIBRARIES optimized libraw.lib debug libraw.lib)

			set(LIBRAW_FOUND true)
			add_definitions(-DWITH_LIBRAW)

	else()
			message(WARNING "libraw build directory not found. Needs path which contains release/libraw.dll, release/libraw.lib, debug/libraw.dll and debug/libraw.lib")
	endif()
endif(ENABLE_RAW)

# search for multi-layer tiff
# we try to grab the OpenCV's libtiff
	unset(TIFF_BUILD_PATH CACHE)
	unset(TIFF_CONFIG_DIR CACHE)
	unset(TIFF_INCLUDE_DIR CACHE)
if(ENABLE_TIFF)
	if(NOT OpenCV_FOUND)
		message(FATAL_ERROR "OpenCV is mandotory when enabling TIFF. You have to enable ENABLE_OPENCV")
	endif(NOT OpenCV_FOUND)

	unset(TIFF_BUILD_PATH CACHE)
	unset(TIFF_CONFIG_DIR CACHE)

	message(STATUS "OpenCV_3RDPARTY_LIB_DIR_OPT: ${OpenCV_3RDPARTY_LIB_DIR_OPT}")
	message(STATUS "OpenCV_DIR: ${OpenCV_DIR}")
	find_path(TIFF_BUILD_PATH NAMES "Release/libtiff.lib" "Debug/libtiffd.lib" PATHS "${OpenCV_3RDPARTY_LIB_DIR_OPT}/../" "${OpenCV_DIR}/3rdparty/lib" DOC "Path to the libtiff build directory" NO_DEFAULT_PATH)
	if(EXISTS "${TIFF_BUILD_PATH}/Release/libtiff.lib" AND EXISTS "${TIFF_BUILD_PATH}/Debug/libtiffd.lib")
		FILE(COPY ${TIFF_BUILD_PATH}/Debug/libtiffd.lib DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/libs)
		FILE(COPY ${TIFF_BUILD_PATH}/Release/libtiff.lib DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/libs)
	else()
		message(FATAL_ERROR, "could not locate libtiff liberaries. Needs TIFF_BUILD_PATH which contains /Release/libtiff.lib, /Debug/libtiffd.lib")
	endif()

	set(TIFF_LIBRARIES optimized "libtiff" debug "libtiffd")		
	find_path(TIFF_CONFIG_DIR NAMES "tif_config.h" HINTS "${OpenCV_DIR}/3rdparty/libtiff" )

	# @stefan we need here the path to opencv/3rdparty/libtiff ... update 10.07.2013 stefan: currently not possible with the cmake of opencv
	find_path(TIFF_INCLUDE_DIR NAMES "tiffio.h" HINTS "${OpenCV_DIR}/../3rdparty/libtiff" "${OpenCV_DIR}/../sources/3rdparty/libtiff" "${OpenCV_DIR}/../opencv/3rdparty/libtiff")

	if(TIFF_LIBRARIES AND EXISTS ${TIFF_CONFIG_DIR} AND EXISTS ${TIFF_INCLUDE_DIR})
		add_definitions(-DWITH_LIBTIFF)
	else(NOT EXISTS ${TIFF_CONFIG_DIR})
		message(FATAL_ERROR "TIFF_CONFIG_DIR dir not found. Needs path which contains Release/libtiff.lib and debug/libtiffd.lib (usually in the OpenCV_Build_Directory/libs)")
		message(FATAL_ERROR "opencv dir: ${OpenCV_DIR}")
	else(NOT EXISTS ${TIFF_INCLUDE_DIR})
		message(FATAL_ERROR "TIFF_INCLUDE_DIR not found. Needs path which contains tif_config.h. Usually located in OpenCV Source directory ./3rdparty/libtiff")
	endif()
endif(ENABLE_TIFF)

#search for UPnP
if(ENABLE_UPNP)
	unset(HUpnp_DIR CACHE)
	find_package(HUpnp)
	if(HUpnp_FOUND)
		file(COPY ${HUPNP_LIB_DIR}/Release/HUpnp.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
		file(COPY ${HUPNP_LIB_DIR}/Release/HUpnp.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)
		file(COPY ${HUPNP_LIB_DIR}/Release/HUpnpAV.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
		file(COPY ${HUPNP_LIB_DIR}/Release/HUpnpAV.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)			
		file(COPY ${HUPNP_LIB_DIR}/Release/QtSoap27.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
		file(COPY ${HUPNP_LIB_DIR}/Release/QtSoap27.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)

		file(COPY ${HUPNP_LIB_DIR}/Debug/HUpnpd.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
		file(COPY ${HUPNP_LIB_DIR}/Debug/HUpnpAVd.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
		file(COPY ${HUPNP_LIB_DIR}/Debug/QtSoap27d.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
		add_definitions(-DWITH_UPNP)
	else()
		message(FATAL_ERROR "HUpnp not found. Mandatory when UPNP is enabled. You have to compile herqq")
	endif(HUpnp_FOUND)	
endif(ENABLE_UPNP)	

#search for quazip
unset(QUAZIP_SOURCE_DIRECTORY CACHE)
unset(QUAZIP_INCLUDE_DIRECTORY CACHE)
unset(QUAZIP_LIBS CACHE)
unset(QUAZIP_BUILD_DIRECTORY CACHE)
unset(QUAZIP_DEPENDENCY CACHE)
unset(QUAZIP_FOUND CACHE)

if(ENABLE_QUAZIP)
	# QT_ROOT needed by QuaZip cmake 
    if ("${QT_ROOT}" STREQUAL "")
        set(QT_ROOT ${QT_QTCORE_INCLUDE_DIR}/../../)
    endif()    
	# these variables need to be set before adding subdirectory
	SET(CMAKE_SHARED_LINKER_FLAGS_REALLYRELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS /LARGEADDRESSAWARE") # /subsystem:windows does not work due to a bug in cmake (see http://public.kitware.com/Bug/view.php?id=12566)
	set(CMAKE_CXX_FLAGS_REALLYRELEASE "-W3 -O2 -DQT_NO_DEBUG_OUTPUT")
	
	add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/quazip-0.7)
	
	set_target_properties(quazip PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/libs)
	set_target_properties(quazip PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/libs)
	set_target_properties(quazip PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_REALLYRELEASE ${CMAKE_CURRENT_BINARY_DIR}/libs)

	find_package(QuaZip)
	if(QUAZIP_FOUND)
		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:quazip> ${NOMACS_BUILD_DIRECTORY}/$<CONFIGURATION>/)
		set(QUAZIP_DEPENDENCY "quazip")
	else()
		message(FATAL_ERROR "CMake was unable to find quazip")
	endif(QUAZIP_FOUND)
endif(ENABLE_QUAZIP)

