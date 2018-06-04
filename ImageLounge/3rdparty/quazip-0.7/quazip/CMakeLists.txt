project(quazip)

# set all include directories for in and out of source builds
file(GLOB SRCS "*.c" "*.cpp")
file(GLOB PUBLIC_HEADERS "*.h")

# Must be added to enable export macro
add_definitions(-DQUAZIP_BUILD)

add_library(${PROJECT_NAME} SHARED ${SRCS})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${ZLIB_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui)
