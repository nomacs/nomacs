# set all include directories for in and out of source builds
file(GLOB SRCS "*.c" "*.cpp")
file(GLOB PUBLIC_HEADERS "*.h")

# Must be added to enable export macro
ADD_DEFINITIONS(-DQUAZIP_BUILD)

add_library(quazip SHARED ${SRCS})
set_target_properties(quazip PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_include_directories(quazip PRIVATE 	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${ZLIB_INCLUDE_DIRS})
# Link against ZLIB_LIBRARIES if needed (on Windows this variable is empty)
target_link_libraries(quazip ${QT_QTMAIN_LIBRARY} ${QT_QTCORE_LIBRARY} ${ZLIB_LIBRARIES})
qt5_use_modules(quazip Core)

# added by stefan for nomacs
# message(STATUS "binarydir: ${NOMACS_BUILD_DIRECTORY}")
# add_custom_command(TARGET quazip POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:quazip> ${NOMACS_BUILD_DIRECTORY}/$<CONFIGURATION>/)

