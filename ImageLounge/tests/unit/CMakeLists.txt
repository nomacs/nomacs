# Tests with no side effects can share a binary.
# Append new test files to the list.
add_nomacs_test(
    tests_core
    unit
    DkUtils_test.cpp
    DkBaseViewPort_test.cpp
    DkNativeImage_test.cpp
    DkMetaData_test.cpp
)
