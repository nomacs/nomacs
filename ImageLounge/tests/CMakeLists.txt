include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/DkCore)

add_executable(core_tests DkUtils_test.cpp)

target_link_libraries(
    core_tests
    ${DLL_CORE_NAME}
    ${OpenCV_LIBS}
    GTest::gtest_main
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
)

add_custom_target(
    check
    COMMAND
        LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/:$ENV{LD_LIBRARY_PATH}
        DYLD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/:$ENV{DYLD_LIBRARY_PATH}
        ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS core_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

include(GoogleTest)
gtest_discover_tests(core_tests)
