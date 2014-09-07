TEMPLATE = lib
TARGET   = HUpnpAv
QT      += network
QT      -= gui
CONFIG  += warn_on dll
DEFINES += H_BUILD_UPNP_AV_LIB
VERSION = 0.0.2

exists(options.pri) {
    include(options.pri)
}

INCLUDEPATH += ./include/

isEmpty(PREFIX) {
    INCLUDEPATH += ../hupnp/include
    LIBS += -L"./../hupnp/bin/" -lHUpnp
}
else {
    INCLUDEPATH += $$PREFIX/include/HUpnpCore/
    LIBS += -L"$$PREFIX/lib" -lHUpnp
}

OBJECTS_DIR = obj
DESTDIR     = ./bin
MOC_DIR     = obj

win32 {
    TARGET_EXT = .dll
}

SRC_LOC = src
include (hupnp_av.pri)
include (src/common/common.pri)
include (src/cds_model/cds_model.pri)
include (src/transport/transport.pri)
include (src/mediaserver/mediaserver.pri)
include (src/mediabrowser/mediabrowser.pri)
include (src/controlpoint/controlpoint.pri)
include (src/mediarenderer/mediarenderer.pri)
include (src/renderingcontrol/renderingcontrol.pri)
include (src/contentdirectory/contentdirectory.pri)
include (src/connectionmanager/connectionmanager.pri)

CONFIG(USE_QT_INSTALL_LOC) {
    INSTLOC_INC = $$[QT_INSTALL_HEADERS]/HUpnpAv
    INSTLOC_LIB = $$[QT_INSTALL_LIBS]
}
else {
    isEmpty(PREFIX) {
        PREFIX = ./deploy
    }

    INSTLOC_INC = $$PREFIX/include/HUpnpAv
    INSTLOC_LIB = $$PREFIX/lib
}

includes.files += ./include/HUpnpAv/H*
includes.path = $$INSTLOC_INC

public_headers.files = $$find(HEADERS, ^((?!_p).)*$)*
public_headers.path = $$INSTLOC_INC/public

private_headers.files = $$EXPORTED_PRIVATE_HEADERS
private_headers.path = $$INSTLOC_INC/private

target.path = $$INSTLOC_LIB

INSTALLS += includes public_headers private_headers target
