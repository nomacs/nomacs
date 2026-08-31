include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/DkCore)

include(GoogleTest)

# dependencies used for all tests
add_library(test_libs INTERFACE)
target_link_libraries(
    test_libs #
    INTERFACE #
        ${DLL_CORE_NAME}
        ${OpenCV_LIBS}
        GTest::gtest_main
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
)

add_custom_target(tests_unit)
add_custom_target(tests_integration)
add_custom_target(tests)
add_dependencies(tests tests_unit tests_integration)

function(add_nomacs_test name category)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE test_libs)
    gtest_discover_tests(${name} DISCOVERY_MODE PRE_TEST PROPERTIES LABELS ${category})
    add_dependencies(tests_${category} ${name})
    set_property(DIRECTORY PROPERTY TEST_INCLUDE_FILE "${NOMACS_TEST_ENV_FILE}")
endfunction()

# set environment variables so `ctest` is run against this build's products
set(NOMACS_TEST_ENV_FILE "${CMAKE_CURRENT_BINARY_DIR}/GlobalCTestEnv.cmake")
file(
    WRITE
    "${NOMACS_TEST_ENV_FILE}"
    "# environment overrides for ctest
set(ENV{QT_PLUGIN_PATH} \"${CMAKE_BINARY_DIR}/nomacs-plugins\")
set(ENV{LD_LIBRARY_PATH} \"${CMAKE_BINARY_DIR}:\$ENV{LD_LIBRARY_PATH}\")
set(ENV{DYLD_LIBRARY_PATH} \"${CMAKE_BINARY_DIR}:\$ENV{DYLD_LIBRARY_PATH}\")
"
)

# convenience script for running test binaries directly (gdb etc)
file(
    WRITE
    "${CMAKE_BINARY_DIR}/test.env"
    "# source this file when running nomacs test binaries directly (not required for ctest)
set -a
export QT_PLUGIN_PATH=\"${CMAKE_BINARY_DIR}/nomacs-plugins\"
export LD_LIBRARY_PATH=\"${CMAKE_BINARY_DIR}:\$LD_LIBRARY_PATH\"
export DYLD_LIBRARY_PATH=\"${CMAKE_BINARY_DIR}:\$DYLD_LIBRARY_PATH\"
"
)

add_subdirectory(unit)
add_subdirectory(integration)

# cmake's "make test" target does not have any dependencies, add "make check" for that
add_custom_target(
    check #
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(
    check-unit
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L unit
    DEPENDS tests_unit
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(
    check-integration
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L integration
    DEPENDS tests_integration
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
