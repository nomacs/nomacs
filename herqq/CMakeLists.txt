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
 