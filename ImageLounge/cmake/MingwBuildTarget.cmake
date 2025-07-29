# mingw/mxe Windows cross compilation target
# almost the same as unix target so use it.

# note: only way I could get resources to link
qt_add_resources(NOMACS_QRC ${NOMACS_RESOURCES})

# mxe is used so generally the same as unix target
include(cmake/UnixBuildTarget.cmake)

target_sources(${BINARY_NAME} PRIVATE ${NOMACS_QRC})

target_link_libraries(${BINARY_NAME} shlwapi)
target_link_libraries(${DLL_CORE_NAME} shlwapi)

set_target_properties(${BINARY_NAME} PROPERTIES LINK_FLAGS_DEBUG "-mconsole")
set_target_properties(${DLL_CORE_NAME} PROPERTIES LINK_FLAGS_DEBUG "-mconsole")

set(MINGW_PACKAGER
    ${CMAKE_CURRENT_LIST_DIR}/MingwPackager.sh
    ${NOMACS_FULL_VERSION}
    ${QT_VERSION_MAJOR}
    ${CMAKE_BUILD_TYPE}
)

add_custom_target(collect COMMAND ${MINGW_PACKAGER})
