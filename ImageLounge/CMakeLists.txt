#
# An initial version of the cmake build for nomacs.
# Currently tested on unix only.
#
# usage:
# cd nomacs/root/dir
# mkdir build
# cd build
# cmake ../ImageLounge -DCMAKE_INSTALL_PREFIX=../release -DCMAKE_BUILD_TYPE=debug (or release, by default)
#       on windows there should be -G "generator name" required as well...
# make
# make install
#
#
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0)

project(nomacs)


OPTION (ENABLE_RAW "Compile with raw images support (OpenCV, libraw)" ON)


set(NOMACS_VERSION 0.3.0)
add_definitions(-DNOMACS_VERSION="${NOMACS_VERSION}")

if (CMAKE_BUILD_TYPE STREQUAL "debug" OR CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    message(STATUS "A debug build. -DDEBUG is defined")
    add_definitions(-DDEBUG)
elseif (NOT MSVC) # debug and release need qt debug outputs on windows
    message(STATUS "A release build (non-debug). Debugging outputs are silently ignored.")
    add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif ()


#Set the custom CMake module directory where our include/lib finders are
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

IF(MSVC)
	OPTION (CREATE_DLL_PROJECT "CREATE PROJECT FOR DLLs" OFF)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libs)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Debug)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Release)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)	
ENDIF(MSVC)

#set(BUILD_EXAMPLES OFF CACHE BOOL "Build all examples")

# find the Qt
set(QT_MIN_VERSION "4.7.0")
set(QT_USE_GUI 1)
set(QT_USE_QTNETWORK 1)
find_package(Qt4 REQUIRED QtCore QtGui QtNetwork)
include(${QT_USE_FILE})


find_package(PkgConfig)
#IF (NOT PkgConfig_FOUND AND NOT MSVC)
#	message(FATAL_ERROR "PkgConfig not found")
#ENDIF()


# exiv2
pkg_check_modules(EXIV2  exiv2>=0.20)
message(STATUS "exiv2 version by pkg-config: ${EXIV2_VERSION}")
IF (NOT EXIV2_FOUND AND MSVC)

	find_path(EXIV2_BUILD_PATH NAMES "ReleaseDLL/exiv2.lib"
										"ReleaseDLL/exib2.dll"
										"DebugDLL/exiv2d.lib"
										"DebugDLL/exiv2d.dll"
					PATHS "../exiv2-0.21.1/msvc/bin/" 
					DOC "Path to the exiv2 build directory" NO_DEFAULT_PATH)
					
	find_path(EXIV2_INCLUDE_DIRS "exiv2/exiv2.hpp" PATHS "exiv2-0.21.1/msvc/include" DOC "Path to exiv2/exiv2.hpp" NO_DEFAULT_PATH)

	find_path(EXPAT_BUILD_PATH NAMES "release/libexpat.dll" "debug/libexpatd.dll" PATHS "../expat-2.0.1/win32/bin/" DOC "Path to the expat build directory" NO_DEFAULT_PATH)
	
	find_path(ZLIB_BUILD_PATH NAMES "Win32_DLL_Release/zlib1.dll" "Win32_DLL_Debug/zlib1d.dll" PATHS "../zlib-1.2.3/projects/visualc6/" DOC "Path to the zlib build directory" NO_DEFAULT_PATH)
	
    IF(  EXISTS ${EXIV2_BUILD_PATH}/ReleaseDLL/exiv2.dll AND
		EXISTS ${EXIV2_BUILD_PATH}/DebugDLL/exiv2d.dll AND 
		EXISTS ${EXIV2_BUILD_PATH}/ReleaseDLL/exiv2.lib AND 
		EXISTS ${EXIV2_BUILD_PATH}/DebugDLL/exiv2d.lib)
		IF( EXISTS ${EXPAT_BUILD_PATH}/Release/libexpat.dll AND 
			EXISTS ${EXPAT_BUILD_PATH}/Debug/libexpatd.dll)
			IF( EXISTS ${ZLIB_BUILD_PATH}/Win32_DLL_Release/zlib1.dll AND
				EXISTS ${ZLIB_BUILD_PATH}/Win32_DLL_Debug/zlib1d.dll )
		
				FILE(COPY ${EXIV2_BUILD_PATH}/ReleaseDLL/exiv2.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
				FILE(COPY ${EXIV2_BUILD_PATH}/ReleaseDLL/exiv2.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)
				FILE(COPY ${EXIV2_BUILD_PATH}/ReleaseDLL/exiv2.lib DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/libs)
				FILE(COPY ${EXIV2_BUILD_PATH}/DebugDLL/exiv2d.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
				FILE(COPY ${EXIV2_BUILD_PATH}/DebugDLL/exiv2d.lib DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/libs)

				FILE(COPY ${EXPAT_BUILD_PATH}/Release/libexpat.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
				FILE(COPY ${EXPAT_BUILD_PATH}/Release/libexpat.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)
				FILE(COPY ${EXPAT_BUILD_PATH}/Debug/libexpatd.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)

				FILE(COPY ${ZLIB_BUILD_PATH}/Win32_DLL_Release/zlib1.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
				FILE(COPY ${ZLIB_BUILD_PATH}/Win32_DLL_Release/zlib1.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)
				FILE(COPY ${ZLIB_BUILD_PATH}/Win32_DLL_Debug/zlib1d.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
				
				SET(EXIV2_LIBRARIES optimized exiv2.lib debug exiv2d.lib)
				SET(EXIV2_LIBRARY_DIRS "")
				SET(EXIV2_FOUND true)
			ELSE ()
				message(WARNING "zlib build directory not found. Needs path which contains Win32_DLL_Release/zlib1.dll and Win32_DLL_Debug/zlib1d.dll")
			ENDIF()
		ELSE()
			message(WARNING "expat build directory not found. Needs path which contains Release/libexpat.dll and Debug/libexpatd.dll")
		ENDIF()
	ELSE()
		message(WARNING "exiv build directory not found. Needs path which contains ReleaseDLL/exiv2.dll, ReleaseDLL/exiv2.lib, DebugDLL/exiv2d.dll and DebugDLL/exiv2d.lib")
	ENDIF()	
	
ENDIF()

find_file(HAVE_EXIV2_HPP exiv2/exiv2.hpp ${EXIV2_INCLUDE_DIRS})
if (HAVE_EXIV2_HPP)
	message(STATUS "exiv2 contains exiv2.hpp, using it")
	add_definitions(-DHAVE_EXIV2_HPP)
else (HAVE_EXIV2_HPP)
	message(STATUS "exiv2 does not contain exiv2.hpp. Using older include set")
endif (HAVE_EXIV2_HPP)


IF (${ENABLE_RAW})

	# NOTE: libraw is used only when there is OpenCV found and used
	# LibRaw
	pkg_check_modules(LIBRAW  libraw>=0.12.0)
        IF (NOT LIBRAW_FOUND AND NOT MSVC)
                message(FATAL_ERROR "libraw not found. It's mandatory when used with ENABLE_RAW enabled") 
        ENDIF()
	IF (NOT LIBRAW_FOUND AND MSVC)
        	find_path(LIBRAW_INCLUDE_DIRS "libraw/libraw.h" PATHS "LibRaw-0.1.3.5" DOC "Path to libraw/libraw.h" NO_DEFAULT_PATH)

        	find_path(LIBRAW_BUILD_PATH NAMES "release/libraw.lib" "release/libraw.dll" "debug/librawd.lib" "debug/librawd.dll"
        	                            PATHS "../LibRaw-0.13.5/buildfiles/" DOC "Path to the libraw build directory" NO_DEFAULT_PATH)

	        IF( EXISTS ${LIBRAW_BUILD_PATH}/release/libraw.dll AND
                	EXISTS ${LIBRAW_BUILD_PATH}/debug/librawd.dll AND 
        	        EXISTS ${LIBRAW_BUILD_PATH}/release/libraw.lib AND 
	                EXISTS ${LIBRAW_BUILD_PATH}/debug/librawd.lib)

                	FILE(COPY ${LIBRAW_BUILD_PATH}/release/libraw.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
        	        FILE(COPY ${LIBRAW_BUILD_PATH}/release/libraw.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)
	                FILE(COPY ${LIBRAW_BUILD_PATH}/release/libraw.lib DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/libs)
                	FILE(COPY ${LIBRAW_BUILD_PATH}/debug/librawd.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
        	        FILE(COPY ${LIBRAW_BUILD_PATH}/debug/librawd.lib DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/libs)

	                SET(LIBRAW_LIBRARY_DIRS "")
                	SET(LIBRAW_LIBRARIES optimized libraw.lib debug librawd.lib)

                	SET(LIBRAW_FOUND true)
        	ELSE()
                	message(WARNING "libraw build directory not found. Needs path which contains release/libraw.dll, release/libraw.lib, debug/librawd.dll and debug/librawd.lib")
        	ENDIF()
	ENDIF()

	# OpenCV
	pkg_check_modules(OpenCV  opencv>=2.1.0)
	IF (NOT OpenCV_FOUND AND MSVC)
		find_path(OpenCV_BUILD_PATH NAMES "bin/Release/opencv_core231.dll" "bin/Release/opencv_imgproc231.dll" 
										"bin/Debug/opencv_core231d.dll" "bin/Debug/opencv_imgproc231d.dll" 
										"lib/Release/opencv_core231.lib" "bin/Release/opencv_imgproc231.lib"
										"lib/Debug/opencv_core231.lib" "bin/Debug/opencv_imgproc231d.lib"
				DOC "Path to the OpenCV Build Directory")
		find_path(OpenCV_INCLUDE_DIRS "opencv/cv.h" PATHS "OpenCV/include" DOC "Path to cv/cv.h")
		
		IF( EXISTS ${OpenCV_BUILD_PATH}/bin/Release/opencv_core231.dll AND
			EXISTS ${OpenCV_BUILD_PATH}/bin/Release/opencv_imgproc231.dll AND
			EXISTS ${OpenCV_BUILD_PATH}/bin/Debug/opencv_core231d.dll AND
			EXISTS ${OpenCV_BUILD_PATH}/bin/Debug/opencv_imgproc231d.dll AND
			EXISTS ${OpenCV_BUILD_PATH}/lib/Release/opencv_core231.lib AND
			EXISTS ${OpenCV_BUILD_PATH}/lib/Release/opencv_imgproc231.lib AND
			EXISTS ${OpenCV_BUILD_PATH}/lib/Debug/opencv_core231d.lib AND
			EXISTS ${OpenCV_BUILD_PATH}/lib/Debug/opencv_imgproc231d.lib
		)
			if (EXISTS ${OpenCV_INCLUDE_DIRS})
				FILE(COPY ${OpenCV_BUILD_PATH}/bin/Release/opencv_core231.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
				FILE(COPY ${OpenCV_BUILD_PATH}/bin/Release/opencv_core231.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)

				FILE(COPY ${OpenCV_BUILD_PATH}/bin/Release/opencv_imgproc231.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
				FILE(COPY ${OpenCV_BUILD_PATH}/bin/Release/opencv_imgproc231.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)

				FILE(COPY ${OpenCV_BUILD_PATH}/bin/Debug/opencv_core231d.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
				FILE(COPY ${OpenCV_BUILD_PATH}/bin/Debug/opencv_imgproc231d.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)

				FILE(COPY ${OpenCV_BUILD_PATH}/lib/Release/opencv_core231.lib DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/libs)
				FILE(COPY ${OpenCV_BUILD_PATH}/lib/Release/opencv_imgproc231.lib DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/libs)

				FILE(COPY ${OpenCV_BUILD_PATH}/lib/Debug/opencv_core231d.lib DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/libs)
				FILE(COPY ${OpenCV_BUILD_PATH}/lib/Debug/opencv_imgproc231d.lib DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/libs)

				SET(OpenCV_LIBRARY_DIRS "")
				SET(OpenCV_LIBRARIES optimized opencv_core231.lib opencv_imgproc231.lib debug opencv_core231d.lib opencv_imgproc231d.lib)
				
				SET(OpenCV_FOUND true)
				SET(OpenCV_VERSION "2.3.1")
			ELSE()
					message(WARNING "OpenCV include dir  not found")		
			ENDIF()
		ELSE()
			message(WARNING "OpenCV build directory not found. Needs path which contains 
			/bin/Release/opencv_core231.dll, 
			/bin/Debug/opencv_core231d.dll,
			/bin/Release/opencv_imgproc231.dll,
			/bin/Debug/opencv_imgproc231d.dll,
			/lib/Release/opencv_core231.lib 
			/lib/Debug/opencv_core231d.lib, 
			/lib/Release/opencv_imgproc231.lib,
			/lib/Debug/opencv_imgproc231d.lib")
		ENDIF()	
	ENDIF()
	IF (NOT OpenCV_FOUND)
		message(FATAL_ERROR "OpenCV not found. It's mandatory when used with ENABLE_RAW enabled") 
	ENDIF()
	add_definitions(-DWITH_OPENCV)
	
	if (APPLE)
		# OpenCV has really draconian dependencies on mac (generating bundle greater tha 105MB)
		# so I play bad games here - expecting all will work with manually updated library names
		message(STATUS "")
		message(STATUS "APPLE: some hacks with OpenCV libraries linking will be performed (only limited set of libs will be used)")
		message(STATUS "APPLE:    originals: ${OpenCV_LIBRARIES}")
		set(OpenCV_LIBRARIES "opencv_core;opencv_imgproc")
        	message(STATUS "APPLE:          new: ${OpenCV_LIBRARIES}")
		message(STATUS "")
	endif (APPLE)

	# lanczos interpolation is not integrated in opencv 2.1 - all other things work
	IF(${OpenCV_VERSION} EQUAL "2.1.0")
		add_definitions(-DDISABLE_LANCZOS)
	ENDIF()

ENDIF()
	
include_directories (
	${QT_INCLUDES}
	${EXIV2_INCLUDE_DIRS}
	${LIBRAW_INCLUDE_DIRS}
	${OpenCV_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_SOURCE_DIR}/src
)


set (NOMACS_SOURCES
        src/DkError.cpp
        src/DkImage.cpp
        src/DkMath.cpp
        src/DkConnection.cpp
        src/DkNetwork.cpp
        src/DkNoMacs.cpp
        src/DkTimer.cpp
        src/DkUtils.cpp
        src/DkViewPort.cpp
        src/DkWidgets.cpp
        src/DkSettings.cpp
        src/BorderLayout.cpp
		src/DkTransferToolBar.cpp
        src/DkMenu.cpp
        src/main.cpp
)
if (APPLE)
	SET (NOMACS_SOURCES ${NOMACS_SOURCES} macosx/nomacs.icns)
endif (APPLE)


set (NOMACS_MOCS
        #src/DkError.h
        src/DkImage.h
        #src/DkMath.h
        src/DkConnection.h
        src/DkNetwork.h
        src/DkNoMacs.h
        #src/DkTimer.h
        #src/DkUtils.h
        src/DkViewPort.h
        src/DkWidgets.h
        src/DkMenu.h
        src/DkSettings.h
		src/DkTransferToolBar.h
        #src/BorderLayout.h
        #src/resource.h
)

set (NOMACS_HEADERS
        src/DkError.h
        src/DkImage.h
        src/DkMath.h
        src/DkConnection.h
        src/DkNetwork.h
        src/DkNoMacs.h
        src/DkTimer.h
        src/DkUtils.h
        src/DkViewPort.h
        src/DkWidgets.h
        src/DkMenu.h
        src/DkSettings.h
		src/DkTransferToolBar.h
        src/BorderLayout.h
        #src/resource.h
)

set (NOMACS_FORMS
        src/nomacs.ui
)

set (NOMACS_RESOURCES
        src/nomacs.qrc
)

set(NOMACS_TRANSLATIONS
		translations/nomacs_de.ts
		translations/nomacs_ru.ts
)

if (MSVC) # some settings for visual studio
	SET(NOMACS_RC src/nomacs.rc) #add resource file when compiling with MSVC 
	SET(VERSION_LIB Version.lib)
	SET(EXIV2_LIBRARY_DIRS ${CMAKE_CURRENT_BINARY_DIR}/libs) #add libs directory to library dirs 
else(MSVC)
	SET(NOMACS_RC "")
	SET(VERSION_LIB "")
ENDIF(MSVC)

QT4_ADD_TRANSLATION(NOMACS_QM ${NOMACS_TRANSLATIONS})
QT4_WRAP_UI(NOMACS_UI ${NOMACS_FORMS})
QT4_WRAP_CPP(NOMACS_MOC_SRC ${NOMACS_MOCS})
QT4_ADD_RESOURCES(NOMACS_RCC ${NOMACS_RESOURCES})

#IF (EXIV2_FOUND AND LIBRAW_FOUND AND OpenCV_FOUND OR NOT COMPILEWITHOPENCV)
IF (EXIV2_FOUND)

	set(EXE_NAME ${CMAKE_PROJECT_NAME})
	link_directories(${LIBRAW_LIBRARY_DIRS} ${OpenCV_LIBRARY_DIRS} ${EXIV2_LIBRARY_DIRS})
	add_executable(${EXE_NAME} WIN32 MACOSX_BUNDLE ${NOMACS_SOURCES} ${NOMACS_UI} ${NOMACS_MOC_SRC} ${NOMACS_RCC} ${NOMACS_HEADERS} ${NOMACS_RC} ${NOMACS_QM} ${NOMACS_TRANSLATIONS})
	target_link_libraries(${EXE_NAME} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTMAIN_LIBRARY} ${EXIV2_LIBRARIES} ${LIBRAW_LIBRARIES} ${OpenCV_LIBRARIES} ${VERSION_LIB})


	IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
		SET_TARGET_PROPERTIES(${EXE_NAME} PROPERTIES LINK_FLAGS -fopenmp)
	ENDIF()

	IF (APPLE)
        	# mac's bundle install
	        SET_TARGET_PROPERTIES(${EXE_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/macosx/Info.plist.in")
        	SET(MACOSX_BUNDLE_ICON_FILE nomacs.icns)
        	SET(MACOSX_BUNDLE_INFO_STRING "${EXE_NAME} ${NOMACS_VERSION}")
        	SET(MACOSX_BUNDLE_GUI_IDENTIFIER "org.nomacs")
        	SET(MACOSX_BUNDLE_LONG_VERSION_STRING "${NOMACS_VERSION}")
        	SET(MACOSX_BUNDLE_BUNDLE_NAME "${EXE_NAME}")
        	SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "${NOMACS_VERSION}")
        	SET(MACOSX_BUNDLE_BUNDLE_VERSION "${NOMACS_VERSION}")
        	SET(MACOSX_BUNDLE_COPYRIGHT "(c) Nomacs team")
        	SET_SOURCE_FILES_PROPERTIES(${CMAKE_SOURCE_DIR}/macosx/nomacs.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

		INSTALL(TARGETS ${EXE_NAME} BUNDLE DESTINATION ${CMAKE_INSTALL_PREFIX})

		# create a "transportable" bundle - all libs into the bundle: "make bundle" after make install
		configure_file(macosx/bundle.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/bundle.cmake @ONLY)
		add_custom_target(bundle ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/bundle.cmake)

	ELSEIF (MSVC) # copy qt dlls and change settings for different projects
		SET (QTLIBLIST QtCore QtGui QtNetwork)
		
		FOREACH(qtlib ${QTLIBLIST})
			GET_FILENAME_COMPONENT(QT_DLL_PATH_tmp ${QT_QMAKE_EXECUTABLE} PATH)
			FILE(COPY ${QT_DLL_PATH_tmp}/${qtlib}4.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
			FILE(COPY ${QT_DLL_PATH_tmp}/${qtlib}4.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)
			FILE(COPY ${QT_DLL_PATH_tmp}/${qtlib}d4.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
		ENDFOREACH(qtlib)
		
		
		SET(CMAKE_CONFIGURATION_TYPES "Debug;Release;ReallyRelease" CACHE STRING "limited configs" FORCE)
		add_definitions(/Zc:wchar_t-)
		SET(CMAKE_CXX_FLAGS_REALLYRELEASE "-W3 -O2 -DQT_NO_DEBUG_OUTPUT")
		SET(CMAKE_EXE_LINKER_FLAGS_REALLYRELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS /LARGEADDRESSAWARE") # /subsystem:windows does not work due to a bug in cmake (see http://public.kitware.com/Bug/view.php?id=12566)
		set_target_properties(${EXE_NAME} PROPERTIES LINK_FLAGS_REALLYRELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS /LARGEADDRESSAWARE")
		set_target_properties(${EXE_NAME} PROPERTIES LINK_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:CONSOLE /LARGEADDRESSAWARE")
		set_target_properties(${EXE_NAME} PROPERTIES LINK_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /SUBSYSTEM:CONSOLE /LARGEADDRESSAWARE")
		
		SET(_moc ${CMAKE_CURRENT_BINARY_DIR}/GeneratedFiles)
		SOURCE_GROUP("Generated Files" FILES ${NOMACS_MOC_SRC} ${NOMACS_RCC} ${NOMACS_UI} ${NOMACS_RC}  ${NOMACS_QM})
		set_source_files_properties(${NOMACS_TRANSLATIONS} PROPERTIES HEADER_FILE_ONLY TRUE)
		SOURCE_GROUP("Translations" FILES ${NOMACS_TRANSLATIONS})
		
		IF(${CREATE_DLL_PROJECT})
			SET(LIB_TYPE SHARED)
			add_library(${EXE_NAME}dll ${LIB_TYPE} ${NOMACS_SOURCES} ${NOMACS_UI} ${NOMACS_MOC_SRC} ${NOMACS_RCC} ${NOMACS_HEADERS} ${NOMACS_RC})
			set_target_properties(${EXE_NAME}dll PROPERTIES COMPILE_FLAGS "-DDKDLL")
			SET(CMAKE_SHARED_LINKER_FLAGS_REALLYRELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}")
		ENDIF(${CREATE_DLL_PROJECT})
		
	ELSE(UNIX)	
	
		# installation
		#  binary
		install(TARGETS ${EXE_NAME} RUNTIME DESTINATION bin LIBRARY DESTINATION lib${LIB_SUFFIX})
		#  desktop file
		install(FILES nomacs.desktop DESTINATION share/applications)
		#  icon
		install(FILES src/img/nomacs.png DESTINATION share/pixmaps)

		# "make dist" target
		string(TOLOWER ${CMAKE_PROJECT_NAME} CPACK_PACKAGE_NAME)
		SET(CPACK_PACKAGE_VERSION "${NOMACS_VERSION}")
		SET(CPACK_SOURCE_GENERATOR "TBZ2")
		SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
		SET(CPACK_IGNORE_FILES "/CVS/;/\\\\.svn/;/\\\\.git/;\\\\.swp$;\\\\.#;/#;\\\\.tar.gz$;/CMakeFiles/;CMakeCache.txt;refresh-copyright-and-license.pl;build;release;")
		SET(CPACK_SOURCE_IGNORE_FILES ${CPACK_IGNORE_FILES})
		INCLUDE(CPack)
		# simulate autotools' "make dist"
		add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
	ENDIF()

	
ELSE()
	message(WARNING "At least one library has not been found")
ENDIF()
