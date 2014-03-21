
HEADERS += \
    $$SRC_LOC/devicemodel/hactions_setupdata.h \
    $$SRC_LOC/devicemodel/hasyncop.h \
    $$SRC_LOC/devicemodel/hexecargs.h \
    $$SRC_LOC/devicemodel/hactioninvoke.h \
    $$SRC_LOC/devicemodel/hactioninvoke_callback.h \
    $$SRC_LOC/devicemodel/hactionarguments.h \
    $$SRC_LOC/devicemodel/hactionarguments_p.h \
    $$SRC_LOC/devicemodel/hdevicestatus.h \
    $$SRC_LOC/devicemodel/hservice_p.h \
    $$SRC_LOC/devicemodel/hdevice_p.h \
    $$SRC_LOC/devicemodel/hdevices_setupdata.h \
    $$SRC_LOC/devicemodel/hservices_setupdata.h \
    $$SRC_LOC/devicemodel/hstatevariable_event.h \
    $$SRC_LOC/devicemodel/hstatevariable_p.h \
    $$SRC_LOC/devicemodel/hdevicemodel_validator.h \
    $$SRC_LOC/devicemodel/hdevicemodel_validator_p.h \
    $$SRC_LOC/devicemodel/hdevicemodel_infoprovider.h \
    $$SRC_LOC/devicemodel/hstatevariables_setupdata.h \
    $$SRC_LOC/devicemodel/client/hclientaction.h \
    $$SRC_LOC/devicemodel/client/hclientactionop.h \
    $$SRC_LOC/devicemodel/client/hclientadapterop.h \
    $$SRC_LOC/devicemodel/client/hclientadapter_p.h \
    $$SRC_LOC/devicemodel/client/hclientaction_p.h \
    $$SRC_LOC/devicemodel/client/hclientdevice.h \
    $$SRC_LOC/devicemodel/client/hclientdevice_p.h \
    $$SRC_LOC/devicemodel/client/hclientdevice_adapter.h \
    $$SRC_LOC/devicemodel/client/hclientdevice_adapter_p.h \
    $$SRC_LOC/devicemodel/client/hclientservice.h \
    $$SRC_LOC/devicemodel/client/hclientservice_p.h \
    $$SRC_LOC/devicemodel/client/hclientservice_adapter.h \
    $$SRC_LOC/devicemodel/client/hclientservice_adapter_p.h \
    $$SRC_LOC/devicemodel/client/hclientstatevariable.h \
    $$SRC_LOC/devicemodel/client/hdefault_clientaction_p.h \
    $$SRC_LOC/devicemodel/client/hdefault_clientdevice_p.h \
    $$SRC_LOC/devicemodel/client/hdefault_clientservice_p.h \
    $$SRC_LOC/devicemodel/client/hdefault_clientstatevariable_p.h \
    $$SRC_LOC/devicemodel/server/hserveraction.h \
    $$SRC_LOC/devicemodel/server/hserverdevice.h \
    $$SRC_LOC/devicemodel/server/hserverdevice_p.h \
    $$SRC_LOC/devicemodel/server/hserverservice.h \
    $$SRC_LOC/devicemodel/server/hserverservice_p.h \
    $$SRC_LOC/devicemodel/server/hserverstatevariable.h \
    $$SRC_LOC/devicemodel/server/hdevicemodelcreator.h \
    $$SRC_LOC/devicemodel/server/hdefault_serverdevice_p.h \
    $$SRC_LOC/devicemodel/server/hdefault_serveraction_p.h \
    $$SRC_LOC/devicemodel/server/hdefault_serverstatevariable_p.h

EXPORTED_PRIVATE_HEADERS += \
    $$SRC_LOC/devicemodel/hasyncop_p.h \
    $$SRC_LOC/devicemodel/hservice_p.h \
    $$SRC_LOC/devicemodel/hdevice_p.h \
    $$SRC_LOC/devicemodel/client/hclientadapter_p.h \
    $$SRC_LOC/devicemodel/client/hclientdevice_adapter_p.h \
    $$SRC_LOC/devicemodel/client/hclientservice_adapter_p.h \
    $$SRC_LOC/devicemodel/server/hserverdevice_p.h \
    $$SRC_LOC/devicemodel/server/hserverservice_p.h

SOURCES += \
    $$SRC_LOC/devicemodel/hactions_setupdata.cpp \
    $$SRC_LOC/devicemodel/hasyncop.cpp \
    $$SRC_LOC/devicemodel/hexecargs.cpp \
    $$SRC_LOC/devicemodel/hactionarguments.cpp \
    $$SRC_LOC/devicemodel/hdevices_setupdata.cpp \
    $$SRC_LOC/devicemodel/hservices_setupdata.cpp \
    $$SRC_LOC/devicemodel/hstatevariable_event.cpp \
    $$SRC_LOC/devicemodel/hstatevariables_setupdata.cpp \
    $$SRC_LOC/devicemodel/hdevicemodel_validator.cpp \
    $$SRC_LOC/devicemodel/hdevicemodel_infoprovider.cpp \
    $$SRC_LOC/devicemodel/client/hclientdevice.cpp \
    $$SRC_LOC/devicemodel/client/hclientadapterop.cpp \
    $$SRC_LOC/devicemodel/client/hclientdevice_adapter.cpp \
    $$SRC_LOC/devicemodel/client/hclientaction.cpp \
    $$SRC_LOC/devicemodel/client/hclientactionop.cpp \
    $$SRC_LOC/devicemodel/client/hclientservice.cpp \
    $$SRC_LOC/devicemodel/client/hclientservice_adapter.cpp \
    $$SRC_LOC/devicemodel/client/hclientstatevariable.cpp \
    $$SRC_LOC/devicemodel/server/hserveraction.cpp \
    $$SRC_LOC/devicemodel/server/hserverdevice.cpp \
    $$SRC_LOC/devicemodel/server/hserverservice.cpp \
    $$SRC_LOC/devicemodel/server/hserverstatevariable.cpp \
    $$SRC_LOC/devicemodel/server/hdevicemodelcreator.cpp
