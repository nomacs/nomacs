
# check cmake requirements
cmake_minimum_required (VERSION 3.10)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)

endif(COMMAND cmake_policy)

project(nomacs)

# needed for soname
set(NOMACS_VERSION_MAJOR 3)
set(NOMACS_VERSION_MINOR 21)
set(NOMACS_VERSION_PATCH 0)
set(NOMACS_VERSION ${NOMACS_VERSION_MAJOR}.${NOMACS_VERSION_MINOR})
set(NOMACS_FULL_VERSION ${NOMACS_VERSION}.${NOMACS_VERSION_PATCH})

if(CMAKE_CL_64)
	SET(NMC_ARCHITECTURE "x64")
else()
	SET(NMC_ARCHITECTURE "x86")
endif()

add_definitions(-DNOMACS_VERSION="${NOMACS_VERSION}")

set(BINARY_NAME ${CMAKE_PROJECT_NAME})
set(NOMACS_BUILD_DIRECTORY ${CMAKE_BINARY_DIR})

# if python is available, we can run additional build scripts (i.e. versioning)
find_package(Python QUIET)

# include macros needed
include("cmake/Utils.cmake")

set(QT5_MIN_VERSION 5.15.0)

# different compile options
option(ENABLE_OPENCV "Compile with Opencv (needed for RAW and TIFF)" ON)
option(ENABLE_RAW "Compile with raw images support (libraw)" ON)
option(ENABLE_TIFF "Compile with multi-layer tiff" ON)
option(ENABLE_QT_DEBUG "Disable Qt Debug Messages" ON)
option(ENABLE_QUAZIP "Compile with QuaZip (allows opening .zip files)" OFF)
option(ENABLE_INCREMENTER "Run Build Incrementer" OFF)
option(ENABLE_TRANSLATIONS "Compile Translations" ON)
option(ENABLE_PLUGINS "Compile nomacs with plugin support" ON)
option(ENABLE_HEIF "Download HEIF plug-in (Windows only)" OFF)
option(ENABLE_AVIF "Download AVIF plug-in (Windows only)" OFF)
option(ENABLE_JXL "Download JPEG XL plug-in (Windows only)" OFF)
option(ENABLE_JXR "Download JPEG XR plug-in (Windows only)" OFF)
option(ENABLE_CODE_COV "Run Code Coverage tests" OFF)
option(ENABLE_TESTING "Run tests" ON)
option(ENABLE_BENCHMARK "Run benchmark" OFF)
option(USE_SYSTEM_QUAZIP "QuaZip will not be compiled from source" ON) # ignored by MSVC

# Codecov
if (ENABLE_CODE_COV AND CMAKE_COMPILER_IS_GNUCXX)
    include("cmake/CodeCoverage.cmake")
    setup_target_for_coverage(${PROJECT_NAME}_coverage tests coverage)
endif()

# load paths from the user file if exists
if (EXISTS ${CMAKE_SOURCE_DIR}/CMakeUserPaths.cmake)
    include(${CMAKE_SOURCE_DIR}/CMakeUserPaths.cmake)

elseif (MSVC)

    set(DEPENDENCY_PATH "" CACHE STRING "Select the path where all dependencies are built")

    # windows needs some hints here...
    if (NOT "${DEPENDENCY_PATH}" STREQUAL "")
        set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${DEPENDENCY_PATH}/exiv2/")
        set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${DEPENDENCY_PATH}/opencv/")
        set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${DEPENDENCY_PATH}/quazip/")
        set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${DEPENDENCY_PATH}/libraw/")
    endif()
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (ENABLE_CODE_COV AND CMAKE_COMPILER_IS_GNUCXX)
	# Codecov
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -pthread -g -O0 -fprofile-arcs -ftest-coverage")
endif()

# find Qt
NMC_FINDQT()
if(${QT_VERSION_MAJOR} VERSION_GREATER_EQUAL "6")
    # Still we a using a tons of deprecated features (silence them for a while)
    if(NOT MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
    endif()

    # We need these to biuld with Qt6
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if (NOT ENABLE_QT_DEBUG)
	message (STATUS "disabling qt debug messages")
	add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()

if(MSVC)
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Win.cmake)
elseif(APPLE)
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Mac.cmake)
elseif(UNIX)
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Unix.cmake)
elseif(MINGW)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Mingw.cmake)
else()
	message(STATUS "build system unkown ... fallback to unix")
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Unix.cmake)
endif()

if (ENABLE_TESTING AND NOT UNIX) 
	set(ENABLE_TESTING OFF)
	message(WARNING "testing is only supported on unix")
elseif (ENABLE_TESTING)
	find_package(GTest)
	if(GTEST_FOUND)
		enable_testing()
		add_subdirectory(tests EXCLUDE_FROM_ALL)
	else()
		set(ENABLE_TESTING OFF)
		message(WARNING "GTest not found, disabling testing.")
	endif()
endif()

if (ENABLE_BENCHMARK AND NOT UNIX) 
	set(ENABLE_BENCHMARK OFF)
	message(WARNING "benchmark is only supported on unix")
elseif (ENABLE_BENCHMARK)
	find_package(benchmark)
	if(benchmark_FOUND)
		add_subdirectory(benchmarks EXCLUDE_FROM_ALL)
	else()
		set(ENABLE_BENCHMARK OFF)
		message(WARNING "benchmark not found, disabling benchmark.")
	endif()
endif()

file(GLOB NOMACS_EXE_SOURCES "src/*.cpp")
file(GLOB NOMACS_EXE_HEADERS "src/*.h")

# gui
file(GLOB GUI_SOURCES "src/DkGui/*.cpp")
file(GLOB GUI_HEADERS "src/DkGui/*.h")

# core
file(GLOB CORE_SOURCES "src/DkCore/*.cpp")
file(GLOB CORE_HEADERS "src/DkCore/*.h")

# themes
file(GLOB NOMACS_THEMES "src/themes/*.css")

# gather information for building
include_directories (
	${EXIV2_INCLUDE_DIRS}
	${LIBRAW_INCLUDE_DIRECTORY}
	${CMAKE_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/src/DkCore
	${CMAKE_CURRENT_SOURCE_DIR}/src/DkGui
	${TIFF_INCLUDE_DIR}
	${TIFF_CONFIG_DIR}
	${QUAZIP_INCLUDE_DIR}
	${QUAZIP_ZLIB_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libqpsd	# needed for linux psd hack
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/drif
)

if (APPLE) # todo: somehow add this to Mac.cmake or MacBuildTarget.cmake
	execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_HEADERS OUTPUT_VARIABLE qt_install_headers)
	include_directories (BEFORE ${qt_install_headers})
	SET (NOMACS_SOURCES ${NOMACS_SOURCES} macosx/nomacs.icns)
endif (APPLE)

IF(NOT ENABLE_PLUGINS)
	LIST(REMOVE_ITEM GUI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/DkPluginManager.cpp)
	LIST(REMOVE_ITEM GUI_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/DkPluginManager.h)
ENDIF(NOT ENABLE_PLUGINS)

# Try to get revision from Git head
set(NOMACS_REVISION "")

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../.git)
	execute_process(COMMAND git rev-parse --short HEAD
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		OUTPUT_VARIABLE NOMACS_REVISION_HASH
		OUTPUT_STRIP_TRAILING_WHITESPACE)

	execute_process(COMMAND git log -n 1 --pretty=%cd --date=short
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		OUTPUT_VARIABLE NOMACS_REVISION_DATE
		OUTPUT_STRIP_TRAILING_WHITESPACE)

	set(NOMACS_REVISION "${NOMACS_REVISION_HASH}, ${NOMACS_REVISION_DATE}")

	execute_process(COMMAND git diff-index --quiet HEAD ${CMAKE_CURRENT_SOURCE_DIR}/src
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		RESULT_VARIABLE NOMACS_SRC_MODIFIED
		ERROR_QUIET)

	if(NOT NOMACS_SRC_MODIFIED EQUAL 0) 
		set(NOMACS_REVISION "${NOMACS_REVISION}, custom")
	endif()
endif()

message(STATUS "revision is " ${NOMACS_REVISION})


# create version file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/DkCore/DkVersion.h.in ${CMAKE_CURRENT_BINARY_DIR}/DkVersion.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/DkCore/DkVersion.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/src/DkCore/DkVersion.cpp)
list(APPEND CORE_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/src/DkCore/DkVersion.cpp)

set(NOMACS_FORMS src/nomacs.ui)
set(NOMACS_RESOURCES src/nomacs.qrc)

file(GLOB NOMACS_TRANSLATIONS "translations/*.ts")

qt_add_resources(NOMACS_RCC ${NOMACS_RESOURCES})

if (${ENABLE_TRANSLATIONS})
	qt_add_translation(NOMACS_QM ${NOMACS_TRANSLATIONS})
endif()

if(MSVC)
    if (QT_VERSION_MAJOR STREQUAL "5")
        include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/WinBuildTarget.cmake)
    else()
        include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/WinBuildTarget-qt6.cmake)
    endif()
elseif(APPLE)
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/MacBuildTarget.cmake)
elseif(MINGW)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/MingwBuildTarget.cmake)
elseif(UNIX)
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/UnixBuildTarget.cmake)
else()
	message(STATUS "build system unkown ... fallback to unix")
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/UnixBuildTarget.cmake)
endif()

# get the version stored in version.cache
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/utils/version.cache NOMACS_CACHED_VERSION)
string(STRIP "${NOMACS_CACHED_VERSION}" NOMACS_CACHED_VERSION)
string(REGEX MATCH "^version:(.*)$" HAS_MATCH ${NOMACS_CACHED_VERSION})
if (HAS_MATCH)
  set(NOMACS_CACHED_VERSION ${CMAKE_MATCH_1})
else()
  message(FATAL_ERROR "I could not parse version.cache")
endif()

# add build incrementer command if requested
if (ENABLE_INCREMENTER AND Python_FOUND)

    add_custom_command(
        TARGET ${DLL_CORE_NAME} PRE_BUILD 
        COMMAND ${Python_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/versionincrement.py
        ${NOMACS_VERSION})

    add_custom_command(
        TARGET ${DLL_CORE_NAME} PRE_BUILD
        COMMAND ${Python_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/versionupdate.py
        ${CMAKE_BINARY_DIR}/DkVersion.h)

    # I am having issues with the PRE_BUILD above
    # if version increment is off, we just need to run the update once (for version won't change)
    # so let's do that:
    execute_process(
        COMMAND ${Python_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/versionupdate.py
        ${CMAKE_BINARY_DIR}/DkVersion.h)

    message(WARNING "build incrementer enabled, using version.cache: \"${NOMACS_CACHED_VERSION}\"")
else()
  # NOTE: the appveyor windows builds rely on version.cache for the installer and msi version numbers
  if (NOT NOMACS_CACHED_VERSION STREQUAL NOMACS_FULL_VERSION)
    message(FATAL_ERROR "version.cache mismatch \"${NOMACS_CACHED_VERSION}\", it should be ${NOMACS_FULL_VERSION}")
  endif()
  message(STATUS "nomacs version is ${NOMACS_FULL_VERSION}")
endif()


NMC_INSTALL()

NMC_COPY_FILES()

#debug for printing out all variables
# get_cmake_property(_variableNames VARIABLES)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

if(ENABLE_PLUGINS)

	if (NOT EXISTS ${PLUGINS_DIR})
		set(PLUGINS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/plugins")
	else ()
		message(STATUS "${PLUGINS_DIR} is defined")
	endif()

	if (NOT EXISTS ${PLUGINS_DIR} AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/plugins")
		set(PLUGINS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/plugins")
	elseif (NOT EXISTS ${PLUGINS_DIR} AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../nomacs-plugins/src")
		# set the snapcraft/flathub dir
		set(PLUGINS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../nomacs-plugins/src")
	endif ()

	if(EXISTS "${PLUGINS_DIR}/CMakeLists.txt")
		message(STATUS "using plugin directory: ${PLUGINS_DIR}")
		set(NOMACS_FOUND true)
		set(NOMACS_VARS_ALREADY_SET true)
		add_subdirectory(${PLUGINS_DIR} "${CMAKE_CURRENT_BINARY_DIR}/plugins")
	else()
		message(STATUS "I failed to locate plugins in: ${PLUGINS_DIR}")
		message(WARNING "plugins directory not found, not building plugins. You have to check out the nomacs-plugins git to the 'plugins' folder if you want to build them")
		SET(ENABLE_PLUGINS OFF)
	endif()
endif()

# get rid of VSCode warning (when using MSBuild)
# see: https://stackoverflow.com/questions/36451368/get-rid-of-cmake-warning-manually-specified-variables-were-not-used-by-the-proj/36452112
set(ignoreMe "${CMAKE_EXPORT_COMPILE_COMMANDS}")

# status
MESSAGE(STATUS "")
MESSAGE(STATUS "----------------------------------------------------------------------------------")
MESSAGE(STATUS " ${PROJECT_NAME} - Image Lounge ${NOMACS_VERSION}  <https://nomacs.org>")
message(STATUS " Qt version: ${Qt${QT_DEFAULT_MAJOR_VERSION}_VERSION}")


IF(OpenCV_FOUND)
    MESSAGE(STATUS " nomacs will be compiled with OPENCV support .................. YES")
ELSE()
    MESSAGE(STATUS " nomacs will be compiled with OPENCV support .................. NO")
ENDIF()

IF(LIBRAW_FOUND)
    MESSAGE(STATUS " nomacs will be compiled with LIBRAW support .................. YES")
ELSE()
    MESSAGE(STATUS " nomacs will be compiled with LIBRAW support .................. NO")
ENDIF()

IF(ENABLE_TIFF)
    MESSAGE(STATUS " nomacs will be compiled with extended TIFF support ........... YES")
ELSE()
    MESSAGE(STATUS " nomacs will be compiled with extended TIFF support ........... NO")
ENDIF()

IF(ENABLE_PLUGINS)
    MESSAGE(STATUS " nomacs will be compiled with plugin support .................. YES")
ELSE()
    MESSAGE(STATUS " nomacs will be compiled with plugin support .................. NO")
ENDIF()

IF(ENABLE_TRANSLATIONS)
    MESSAGE(STATUS " nomacs will be compiled with Translations .................... YES")
ELSE()
    MESSAGE(STATUS " nomacs will be compiled with Translations .................... NO")
ENDIF()

IF(ENABLE_QUAZIP)
    MESSAGE(STATUS " nomacs will be compiled with QuaZip support .................. YES")
ELSE()
    MESSAGE(STATUS " nomacs will be compiled with QuaZip support .................. NO")
ENDIF()

IF(ENABLE_TESTING)
    MESSAGE(STATUS " nomacs tests ................................................. YES")
ELSE()
    MESSAGE(STATUS " nomacs tests ................................................. NO")
ENDIF()

IF(ENABLE_BENCHMARK)
    MESSAGE(STATUS " nomacs benchmarks ............................................ YES")
ELSE()
    MESSAGE(STATUS " nomacs benchmarks ............................................ NO")
ENDIF()

MESSAGE(STATUS "----------------------------------------------------------------------------------")
