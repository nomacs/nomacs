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

add_nomacs_test(
    test_batch_paste
    integration
    DkBatchPaste_test.cpp
)
target_link_libraries(test_batch_paste PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
