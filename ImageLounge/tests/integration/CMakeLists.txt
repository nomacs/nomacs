# Tests that wish to have its own QApplication needs its own binary.
# Add `add_nomacs_test` for each of them.

add_nomacs_test(
    test_shortcutsmodel
    integration
    DkShortcutsModel_test.cpp
)
if(TARGET nomacs_plugins)
    add_dependencies(test_shortcutsmodel nomacs_plugins)
endif()
