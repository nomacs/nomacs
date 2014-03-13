cmake_minimum_required(VERSION 2.8)
if (CMAKE_BUILD_TYPE STREQUAL "debug" OR CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    message(STATUS "A debug build. -DDEBUG is defined")
    add_definitions(-DDEBUG)
endif()
# ---------- Setup output Directories -------------------------
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY
   ${CMAKE_BINARY_DIR}/libs )

# --------- Setup the Executable output Directory -------------
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
   ${CMAKE_BINARY_DIR}/libs )

# --------- Setup the Executable output Directory -------------
SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY
   ${CMAKE_BINARY_DIR}/libs ) 
   
project(HUpnp)
set(HUpnp_MAJOR_VERSION 0)
set(HUpnp_MINOR_VERSION 1)
set(HUpnp_PATCH_VERSION 0)
# find_package(Qt4 COMPONENTS QtCore QtXml QtNetwork REQUIRED)
find_package(Qt4 REQUIRED QtCore QtXml QtNetwork)
 
add_subdirectory(hupnp)

get_property(HUPNP_DEBUG_NAME TARGET HUpnp PROPERTY DEBUG_OUTPUT_NAME)
get_property(HUPNP_RELEASE_NAME TARGET HUpnp PROPERTY RELEASE_OUTPUT_NAME)

get_property(QTSOAP_DEBUG_NAME TARGET QtSolutions_SOAP-2.7 PROPERTY DEBUG_OUTPUT_NAME)
get_property(QTSOAP_RELEASE_NAME TARGET QtSolutions_SOAP-2.7 PROPERTY RELEASE_OUTPUT_NAME)

set(HUPNP_LIBS optimized ${CMAKE_BINARY_DIR}/libs/${HUPNP_RELEASE_NAME}.lib optimized ${CMAKE_BINARY_DIR}/libs/${QTSOAP_RELEASE_NAME}.lib debug  ${CMAKE_BINARY_DIR}/libs/${HUPNP_DEBUG_NAME}.lib debug ${CMAKE_BINARY_DIR}/libs/${QTSOAP_DEBUG_NAME}.lib)
MESSAGE(STATUS "libs: ${HUPNP_LIBS}")
configure_file(${HUpnp_SOURCE_DIR}/HUpnpConfig.cmake.in ${CMAKE_BINARY_DIR}/HUpnpConfig.cmake)
 