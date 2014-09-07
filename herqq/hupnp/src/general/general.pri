
HEADERS += \
    $$SRC_LOC/general/hupnp_defs.h \
    $$SRC_LOC/general/hupnp_fwd.h \
    $$SRC_LOC/general/hlogger_p.h \
    $$SRC_LOC/general/hupnp_global_p.h \
    $$SRC_LOC/general/hupnp_global.h \
    $$SRC_LOC/general/hclonable.h \
    $$SRC_LOC/general/hupnpinfo.h \
    $$SRC_LOC/general/hupnp_datatypes.h \
    $$SRC_LOC/general/hupnp_datatypes_p.h

SOURCES += \
    $$SRC_LOC/general/hupnp_global.cpp \
    $$SRC_LOC/general/hclonable.cpp \
    $$SRC_LOC/general/hlogger_p.cpp \
    $$SRC_LOC/general/hupnpinfo.cpp \
    $$SRC_LOC/general/hupnp_datatypes.cpp

EXPORTED_PRIVATE_HEADERS += \
    $$SRC_LOC/general/hlogger_p.h