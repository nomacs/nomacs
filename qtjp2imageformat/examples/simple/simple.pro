TEMPLATE = app
INCLUDEPATH += .

SOURCES += simple.cpp 

RESOURCES = simple.qrc

CONFIG += qt warn_on thread create_prl link_prl console

# If the plugin is built statically, uncomment the following line.
# (If the jasper libs are not in a system path, also add -L<jasper-libdir> )
#LIBS += -L$$[QT_INSTALL_PLUGINS]/imageformats -lqtjp2 -ljasper
