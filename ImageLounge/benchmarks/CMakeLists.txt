include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/DkCore)

add_executable(core_benchmarks DkImageStorage_bench.cpp)

target_link_libraries(
    core_benchmarks
    nomacsCore
    ${OpenCV_LIBS}
    benchmark::benchmark
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
)

add_custom_target(
    bench
    COMMAND core_benchmarks
    DEPENDS core_benchmarks
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
