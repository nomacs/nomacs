include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/DkCore)

add_executable(core_tests DkUtils_test.cpp)

target_link_libraries(
    core_tests
    nomacsCore
    ${OpenCV_LIBS}
    GTest::gtest_main
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
)

add_custom_target(
    check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS core_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

include(GoogleTest)
gtest_discover_tests(core_tests)
