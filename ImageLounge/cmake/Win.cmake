# nomacs cmake file for a windows build

# load pathes from the user file if exists
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/CMakeUser.txt)
	include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeUser.txt)
endif()

if(ENABLE_PLUGINS)
	add_definitions(-DWITH_PLUGINS)
endif()

# prepare build directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/libs)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Debug)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release)

if (MSVC11)
	# use precompiled code if msvc 11 is found
	if(CMAKE_CL_64)
		SET(EXIV_SEARCH_PATH "../exiv2-0.25/msvc2012-precompiled/x64/" )
	else()
		SET(EXIV_SEARCH_PATH "../exiv2-0.25/msvc2012-precompiled/Win32/" )
	endif()

elseif (MSVC14)
	# use precompiled code if msvc 14 is found
	if(CMAKE_CL_64)
		SET(EXIV_SEARCH_PATH "../exiv2-0.25/msvc2015-precompiled/x64/" )
	else()
		SET(EXIV_SEARCH_PATH "../exiv2-0.25/msvc2015-precompiled/Win32/" )
	endif()

else ()
	# search for exiv2
	if(CMAKE_CL_64)
		SET(EXIV_SEARCH_PATH "../exiv2-0.24/msvc2012-nomacs/exiv2lib/x64/" )
	else()
		SET(EXIV_SEARCH_PATH "../exiv2-0.24/msvc2012-nomacs/exiv2lib/Win32/" )
	endif()
endif ()


find_path(EXIV2_BUILD_PATH NAMES "ReleaseDLL/libexiv2.lib"
									"ReleaseDLL/libexiv2.dll"
									"DebugDLL/libexiv2d.lib"
									"DebugDLL/libexiv2d.dll"
				PATHS ${EXIV_SEARCH_PATH}
				DOC "Path to the exiv2 build directory" NO_DEFAULT_PATH)

find_path(EXIV2_INCLUDE_DIRS "exiv2/exiv2.hpp"
				PATHS "../exiv2-0.25/include"
				DOC "Path to exiv2/exiv2.hpp" NO_DEFAULT_PATH)
MARK_AS_ADVANCED(EXIV2_INCLUDE_DIRS)

if( EXISTS ${EXIV2_BUILD_PATH}/ReleaseDLL/libexiv2.lib AND
	EXISTS ${EXIV2_BUILD_PATH}/DebugDLL/libexiv2.lib)

	set(EXIV2_LIBRARIES optimized ${EXIV2_BUILD_PATH}/ReleaseDLL/libexiv2.lib debug ${EXIV2_BUILD_PATH}/DebugDLL/libexiv2.lib)
	set(EXIV2_LIBRARY_DIRS "")
	set(EXIV2_FOUND true)
	add_definitions(-DEXV_UNICODE_PATH)
else()
	message(WARNING "exiv build directory not found. Needs EXIV2_BUILD_PATH which contains ReleaseDLL/libexiv2.dll, ReleaseDLL/libexiv2.lib, DebugDLL/libexiv2d.dll and DebugDLL/libexiv2d.lib")
endif()

# search for opencv
unset(OpenCV_LIB_DIR_DBG CACHE)
unset(OpenCV_3RDPARTY_LIB_DIR_DBG CACHE)
unset(OpenCV_3RDPARTY_LIB_DIR_OPT CACHE)
unset(OpenCV_CONFIG_PATH CACHE)
unset(OpenCV_LIB_DIR_DBG CACHE)
unset(OpenCV_LIB_DIR_OPT CACHE)
unset(OpenCV_LIBRARY_DIRS CACHE)
unset(OpenCV_DIR)
# unset(OpenCV_DIR CACHE) # maa that always set it to default!

if(ENABLE_OPENCV)
	find_package(OpenCV REQUIRED core imgproc)

	SET(OpenCV_LIBRARY_DIRS ${OpenCV_LIBRARY_DIRS} ${OpenCV_LIB_DIR_DBG} ${OpenCV_LIB_DIR_OPT} ${OpenCV_DIR}/lib/${OpenCV_LIB_DIR_DBG} ${OpenCV_DIR}/lib/${OpenCV_LIB_DIR_OPT})
	if(NOT OpenCV_FOUND)
		message(FATAL_ERROR "OpenCV not found.")
	else()
		add_definitions(-DWITH_OPENCV)
	endif()

	if(${OpenCV_VERSION} EQUAL "2.1.0")
		add_definitions(-DDISABLE_LANCZOS)
	endif()

	# unset include directories since OpenCV sets them global
	get_property(the_include_dirs  DIRECTORY . PROPERTY INCLUDE_DIRECTORIES)
	list(REMOVE_ITEM the_include_dirs ${OpenCV_INCLUDE_DIRS})
	set_property(DIRECTORY . PROPERTY INCLUDE_DIRECTORIES ${the_include_dirs})

endif(ENABLE_OPENCV)

# search for libraw
if(ENABLE_RAW)
	if(NOT OpenCV_FOUND)
		message(FATAL_ERROR "OpenCV is mandotory when enabling RAW. You have to enable ENABLE_OPENCV")
	endif()

	find_path(LIBRAW_INCLUDE_DIRS "libraw/libraw.h" PATHS "../LibRaw-0.17.0/" DOC "Path to libraw/libraw.h" NO_DEFAULT_PATH)

	if (MSVC14)
		if(CMAKE_CL_64)
			set(LIBRAW_SEARCH_PATH "../LibRaw-0.17.0/build2015/bin/x64")
		else()
			set(LIBRAW_SEARCH_PATH "../LibRaw-0.17.0/build2015/bin/Win32")
		endif()
	else()
		if(CMAKE_CL_64)
			set(LIBRAW_SEARCH_PATH "../LibRaw-0.17.0/build2012/bin/x64")
		else()
			set(LIBRAW_SEARCH_PATH "../LibRaw-0.17.0/build2012/bin/Win32")
		endif()
	endif()

	find_path(LIBRAW_BUILD_PATH NAMES "release/libraw.lib" "release/libraw.dll" "debug/libraw.lib" "debug/libraw.dll"
								PATHS ${LIBRAW_SEARCH_PATH} DOC "Path to the libraw build directory" NO_DEFAULT_PATH)
	if(EXISTS ${LIBRAW_BUILD_PATH}/release/libraw.lib AND
		EXISTS ${LIBRAW_BUILD_PATH}/debug/libraw.lib)
			set(LIBRAW_LIBRARIES optimized ${LIBRAW_BUILD_PATH}/release/libraw.lib debug ${LIBRAW_BUILD_PATH}/debug/libraw.lib)

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

	find_path(TIFF_BUILD_PATH NAMES "Release/libtiff.lib" "Debug/libtiffd.lib" PATHS "${OpenCV_3RDPARTY_LIB_DIR_OPT}/../" "${OpenCV_DIR}/3rdparty/lib" DOC "Path to the libtiff build directory" NO_DEFAULT_PATH)
	find_path(TIFF_CONFIG_DIR NAMES "tif_config.h" HINTS "${OpenCV_DIR}/3rdparty/libtiff" )

	# @stefan we need here the path to opencv/3rdparty/libtiff ... update 10.07.2013 stefan: currently not possible with the cmake of opencv
	find_path(TIFF_INCLUDE_DIR NAMES "tiffio.h" HINTS "${OpenCV_DIR}/../3rdparty/libtiff" "${OpenCV_DIR}/../sources/3rdparty/libtiff" "${OpenCV_DIR}/../opencv/3rdparty/libtiff")
    set(TIFF_LIBRARIES optimized "${TIFF_BUILD_PATH}/Release/libtiff.lib;" optimized "${OpenCV_DIR}/3rdparty/lib/Release/zlib.lib" debug "${TIFF_BUILD_PATH}/Debug/libtiffd.lib"  debug "${OpenCV_DIR}/3rdparty/lib/Debug/zlibd.lib")

	if(TIFF_LIBRARIES AND EXISTS ${TIFF_CONFIG_DIR} AND EXISTS ${TIFF_INCLUDE_DIR})
		add_definitions(-DWITH_LIBTIFF)
	else(NOT EXISTS ${TIFF_CONFIG_DIR})
		message(FATAL_ERROR "TIFF_CONFIG_DIR dir not found. Needs path which contains Release/libtiff.lib and debug/libtiffd.lib (usually in the OpenCV_Build_Directory/libs). Note: tif_config.h is only available if you have compiled OpenCV on yourself. If you want to use the precompiled version you have to disable TIFF")
	else(NOT EXISTS ${TIFF_INCLUDE_DIR})
		message(FATAL_ERROR "TIFF_INCLUDE_DIR not found. Needs path which contains tif_config.h. Usually located in OpenCV Source directory ./3rdparty/libtiff")
	endif()
endif(ENABLE_TIFF)

#search for UPnP
if(ENABLE_UPNP)
	unset(HUpnp_DIR CACHE)
	find_package(HUpnp)
	if(HUpnp_FOUND)
		file(COPY ${HUPNP_LIB_DIR}/Release/HUpnp.dll DESTINATION ${CMAKE_BINARY_DIR}/Release)
		file(COPY ${HUPNP_LIB_DIR}/Release/HUpnp.dll DESTINATION ${CMAKE_BINARY_DIR}/RelWithDebInfo)
		file(COPY ${HUPNP_LIB_DIR}/Release/HUpnp.dll DESTINATION ${CMAKE_BINARY_DIR}/MinSizeRel)
		file(COPY ${HUPNP_LIB_DIR}/Release/HUpnpAV.dll DESTINATION ${CMAKE_BINARY_DIR}/Release)
		file(COPY ${HUPNP_LIB_DIR}/Release/HUpnpAV.dll DESTINATION ${CMAKE_BINARY_DIR}/RelWithDebInfo)
		file(COPY ${HUPNP_LIB_DIR}/Release/HUpnpAV.dll DESTINATION ${CMAKE_BINARY_DIR}/MinSizeRel)
		file(COPY ${HUPNP_LIB_DIR}/Release/QtSoap27.dll DESTINATION ${CMAKE_BINARY_DIR}/Release)
		file(COPY ${HUPNP_LIB_DIR}/Release/QtSoap27.dll DESTINATION ${CMAKE_BINARY_DIR}/RelWithDebInfo)
		file(COPY ${HUPNP_LIB_DIR}/Release/QtSoap27.dll DESTINATION ${CMAKE_BINARY_DIR}/MinSizeRel)

		file(COPY ${HUPNP_LIB_DIR}/Debug/HUpnpd.dll DESTINATION ${CMAKE_BINARY_DIR}/Debug)
		file(COPY ${HUPNP_LIB_DIR}/Debug/HUpnpAVd.dll DESTINATION ${CMAKE_BINARY_DIR}/Debug)
		file(COPY ${HUPNP_LIB_DIR}/Debug/QtSoap27d.dll DESTINATION ${CMAKE_BINARY_DIR}/Debug)
		add_definitions(-DWITH_UPNP)
	else()
		message(FATAL_ERROR "HUpnp not found. Mandatory when UPNP is enabled. You have to compile herqq")
	endif(HUpnp_FOUND)
endif(ENABLE_UPNP)


# these variables need to be set before adding subdirectory with projects
set(NOMACS_BUILD_DIRECTORY ${CMAKE_BINARY_DIR})

#search for quazip
unset(QUAZIP_SOURCE_DIRECTORY CACHE)
unset(QUAZIP_INCLUDE_DIRECTORY CACHE)
unset(QUAZIP_LIBS CACHE)
unset(QUAZIP_BUILD_DIRECTORY CACHE)
unset(QUAZIP_DEPENDENCY CACHE)
unset(QUAZIP_FOUND CACHE)

if(ENABLE_QUAZIP)
	# QT_ROOT needed by QuaZip cmake
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/quazip-0.7)
	# add build directory to cmake path - otherwise the QuaZipConfig.cmake is no longer found
	SET (CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_BINARY_DIR})

	set_target_properties(quazip PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)
	set_target_properties(quazip PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)
	set_target_properties(quazip PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)
	set_target_properties(quazip PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)

	find_package(QuaZip)
	if(QUAZIP_FOUND)
		add_definitions(-DWITH_QUAZIP)
		set(QUAZIP_DEPENDENCY "quazip")
	else()
		message(FATAL_ERROR "CMake was unable to find quazip")
	endif(QUAZIP_FOUND)
endif(ENABLE_QUAZIP)

#add libqpsd
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libqpsd)
set(LIBQPSD_LIBRARY "qpsd")
