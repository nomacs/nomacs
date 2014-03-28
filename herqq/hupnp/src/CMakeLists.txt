set(HUpnp_SRCS
dataelements/hactioninfo.cpp
dataelements/hdeviceinfo.cpp
dataelements/hdiscoverytype.cpp
dataelements/hproduct_tokens.cpp
dataelements/hresourcetype.cpp
dataelements/hserviceid.cpp
dataelements/hserviceinfo.cpp
dataelements/hstatevariableinfo.cpp
dataelements/hudn.cpp
dataelements/hvaluerange_p.cpp
devicehosting/controlpoint/hclientmodel_creator_p.cpp
devicehosting/controlpoint/hcontrolpoint.cpp
devicehosting/controlpoint/hcontrolpoint_configuration.cpp
devicehosting/controlpoint/hcontrolpoint_dataretriever_p.cpp
devicehosting/controlpoint/hdevicebuild_p.cpp
devicehosting/controlpoint/hevent_subscriptionmanager_p.cpp
devicehosting/controlpoint/hevent_subscription_p.cpp
devicehosting/devicehost/hdevicehost.cpp
devicehosting/devicehost/hdevicehost_configuration.cpp
devicehosting/devicehost/hdevicehost_dataretriever_p.cpp
devicehosting/devicehost/hdevicehost_http_server_p.cpp
devicehosting/devicehost/hdevicehost_ssdp_handler_p.cpp
devicehosting/devicehost/hevent_notifier_p.cpp
devicehosting/devicehost/hevent_subscriber_p.cpp
devicehosting/devicehost/hservermodel_creator_p.cpp
devicehosting/hddoc_parser_p.cpp
devicehosting/hdevicestorage_p.cpp
devicehosting/hmodelcreation_p.cpp
devicehosting/messages/hcontrol_messages_p.cpp
devicehosting/messages/hevent_messages_p.cpp
devicehosting/messages/hnt_p.cpp
devicehosting/messages/hsid_p.cpp
devicehosting/messages/htimeout_p.cpp
devicemodel/client/hclientaction.cpp
devicemodel/client/hclientactionop.cpp
devicemodel/client/hclientadapterop.cpp
devicemodel/client/hclientdevice.cpp
devicemodel/client/hclientdevice_adapter.cpp
devicemodel/client/hclientservice.cpp
devicemodel/client/hclientservice_adapter.cpp
devicemodel/client/hclientstatevariable.cpp
devicemodel/hactionarguments.cpp
devicemodel/hactions_setupdata.cpp
devicemodel/hasyncop.cpp
devicemodel/hdevicemodel_infoprovider.cpp
devicemodel/hdevicemodel_validator.cpp
devicemodel/hdevices_setupdata.cpp
devicemodel/hexecargs.cpp
devicemodel/hservices_setupdata.cpp
devicemodel/hstatevariables_setupdata.cpp
devicemodel/hstatevariable_event.cpp
devicemodel/server/hdevicemodelcreator.cpp
devicemodel/server/hserveraction.cpp
devicemodel/server/hserverdevice.cpp
devicemodel/server/hserverservice.cpp
devicemodel/server/hserverstatevariable.cpp
general/hclonable.cpp
general/hlogger_p.cpp
general/hupnpinfo.cpp
general/hupnp_datatypes.cpp
general/hupnp_global.cpp
http/hhttp_asynchandler_p.cpp
http/hhttp_header_p.cpp
http/hhttp_messagecreator_p.cpp
http/hhttp_messaginginfo_p.cpp
http/hhttp_server_p.cpp
http/hhttp_utils_p.cpp
socket/hendpoint.cpp
socket/hmulticast_socket.cpp
ssdp/hdiscovery_messages.cpp
ssdp/hssdp.cpp
ssdp/hssdp_messagecreator_p.cpp
utils/hmisc_utils_p.cpp
utils/hsysutils_p.cpp
utils/hthreadpool_p.cpp
)
 
set(HUpnp_MOC_HDRS
 dataelements/hactioninfo.h
 dataelements/hactioninfo_p.h
 dataelements/hdeviceinfo.h
 dataelements/hdeviceinfo_p.h
 dataelements/hdiscoverytype.h
 dataelements/hproduct_tokens.h
 dataelements/hresourcetype.h
 dataelements/hserviceid.h
 dataelements/hserviceinfo.h
 dataelements/hserviceinfo_p.h
 dataelements/hstatevariableinfo.h
 dataelements/hstatevariableinfo_p.h
 dataelements/hudn.h
 dataelements/hvaluerange_p.h
 devicehosting/controlpoint/hclientmodel_creator_p.h
 devicehosting/controlpoint/hcontrolpoint.h
 devicehosting/controlpoint/hcontrolpoint_configuration.h
 devicehosting/controlpoint/hcontrolpoint_configuration_p.h
 devicehosting/controlpoint/hcontrolpoint_dataretriever_p.h
 devicehosting/controlpoint/hcontrolpoint_p.h
 devicehosting/controlpoint/hdevicebuild_p.h
 devicehosting/controlpoint/hevent_subscriptionmanager_p.h
 devicehosting/controlpoint/hevent_subscription_p.h
 devicehosting/devicehost/hdevicehost.h
 devicehosting/devicehost/hdevicehost_configuration.h
 devicehosting/devicehost/hdevicehost_configuration_p.h
 devicehosting/devicehost/hdevicehost_dataretriever_p.h
 devicehosting/devicehost/hdevicehost_http_server_p.h
 devicehosting/devicehost/hdevicehost_p.h
 devicehosting/devicehost/hdevicehost_runtimestatus_p.h
 devicehosting/devicehost/hdevicehost_ssdp_handler_p.h
 devicehosting/devicehost/hevent_notifier_p.h
 devicehosting/devicehost/hevent_subscriber_p.h
 devicehosting/devicehost/hpresence_announcer_p.h
 devicehosting/devicehost/hserverdevicecontroller_p.h
 devicehosting/devicehost/hservermodel_creator_p.h
 devicehosting/hddoc_parser_p.h
 devicehosting/hdevicestorage_p.h
 devicehosting/hmodelcreation_p.h
 devicehosting/messages/hcontrol_messages_p.h
 devicehosting/messages/hevent_messages_p.h
 devicehosting/messages/hnt_p.h
 devicehosting/messages/hsid_p.h
 devicehosting/messages/htimeout_p.h
 devicemodel/client/hclientaction.h
 devicemodel/client/hclientactionop.h
 devicemodel/client/hclientactionop_p.h
 devicemodel/client/hclientaction_p.h
 devicemodel/client/hclientadapterop.h
 devicemodel/client/hclientadapter_p.h
 devicemodel/client/hclientdevice.h
 devicemodel/client/hclientdevice_adapter.h
 devicemodel/client/hclientdevice_adapter_p.h
 devicemodel/client/hclientdevice_p.h
 devicemodel/client/hclientservice.h
 devicemodel/client/hclientservice_adapter.h
 devicemodel/client/hclientservice_adapter_p.h
 devicemodel/client/hclientservice_p.h
 devicemodel/client/hclientstatevariable.h
 devicemodel/client/hdefault_clientaction_p.h
 devicemodel/client/hdefault_clientdevice_p.h
 devicemodel/client/hdefault_clientservice_p.h
 devicemodel/client/hdefault_clientstatevariable_p.h
 devicemodel/hactionarguments.h
 devicemodel/hactionarguments_p.h
 devicemodel/hactioninvoke.h
 devicemodel/hactioninvoke_callback.h
 devicemodel/hactions_setupdata.h
 devicemodel/hasyncop.h
 devicemodel/hasyncop_p.h
 devicemodel/hdevicemodel_infoprovider.h
 devicemodel/hdevicemodel_validator.h
 devicemodel/hdevicemodel_validator_p.h
 devicemodel/hdevicestatus.h
 devicemodel/hdevices_setupdata.h
 devicemodel/hdevice_p.h
 devicemodel/hexecargs.h
 devicemodel/hservices_setupdata.h
 devicemodel/hservice_p.h
 devicemodel/hstatevariables_setupdata.h
 devicemodel/hstatevariable_event.h
 devicemodel/hstatevariable_p.h
 devicemodel/server/hdefault_serveraction_p.h
 devicemodel/server/hdefault_serverdevice_p.h
 devicemodel/server/hdefault_serverstatevariable_p.h
 devicemodel/server/hdevicemodelcreator.h
 devicemodel/server/hserveraction.h
 devicemodel/server/hserveraction_p.h
 devicemodel/server/hserverdevice.h
 devicemodel/server/hserverdevice_p.h
 devicemodel/server/hserverservice.h
 devicemodel/server/hserverservice_p.h
 devicemodel/server/hserverstatevariable.h
 general/hclonable.h
 general/hlogger_p.h
 general/hupnpinfo.h
 general/hupnp_datatypes.h
 general/hupnp_datatypes_p.h
 general/hupnp_defs.h
 general/hupnp_fwd.h
 general/hupnp_global.h
 general/hupnp_global_p.h
 http/hhttp_asynchandler_p.h
 http/hhttp_header_p.h
 http/hhttp_messagecreator_p.h
 http/hhttp_messaginginfo_p.h
 http/hhttp_p.h
 http/hhttp_server_p.h
 http/hhttp_utils_p.h
 socket/hendpoint.h
 socket/hmulticast_socket.h
 ssdp/hdiscovery_messages.h
 ssdp/hssdp.h
 ssdp/hssdp_messagecreator_p.h
 ssdp/hssdp_p.h
 utils/hfunctor.h
 utils/hglobal.h
 utils/hmisc_utils_p.h
 utils/hsysutils_p.h
 utils/hthreadpool_p.h
)
 
qt4_wrap_cpp(HUpnp_MOC_SRCS ${HUpnp_MOC_HDRS})
 
file(GLOB HUpnp_INDIRECT_HEADERS ${CMAKE_SOURCE_DIR}/hupnp/include/HUpnpCore/H*)
file(GLOB HUpnp_PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/hupnp/include/HUpnpCore/H*)
 
set(HUpnp_PUBLIC_HEADERS
devicemodel/server/hserverservice.h
devicemodel/server/hdevicemodelcreator.h
devicemodel/server/hserveraction.h
devicemodel/server/hserverdevice.h
devicemodel/server/hserverstatevariable.h
devicemodel/hactioninvoke.h
devicemodel/hstatevariable_event.h
devicemodel/hservices_setupdata.h
devicemodel/client/hclientservice.h
devicemodel/client/hclientstatevariable.h
devicemodel/client/hclientaction.h
devicemodel/client/hclientdevice.h
devicemodel/client/hclientactionop.h
devicemodel/hactions_setupdata.h
devicemodel/hasyncop.h
devicemodel/hdevicemodel_infoprovider.h
devicemodel/hexecargs.h
devicemodel/hdevices_setupdata.h
devicemodel/hactionarguments.h
devicemodel/hactioninvoke_callback.h
devicemodel/hdevicestatus.h
devicemodel/hstatevariables_setupdata.h
general/hupnpinfo.h
general/hupnp_defs.h
general/hclonable.h
general/hupnp_fwd.h
general/hupnp_datatypes.h
general/hupnp_global.h
dataelements/hserviceinfo.h
dataelements/hserviceid.h
dataelements/hproduct_tokens.h
dataelements/hdeviceinfo.h
dataelements/hudn.h
dataelements/hstatevariableinfo.h
dataelements/hdiscoverytype.h
dataelements/hresourcetype.h
dataelements/hactioninfo.h
socket/hendpoint.h
socket/hmulticast_socket.h
ssdp/hssdp.h
ssdp/hdiscovery_messages.h
devicehosting/devicehost/hdevicehost_configuration.h
devicehosting/devicehost/hdevicehost.h
devicehosting/controlpoint/hcontrolpoint.h
devicehosting/controlpoint/hcontrolpoint_configuration.h
utils/hfunctor.h
utils/hglobal.h
)
 
add_library(HUpnp SHARED ${HUpnp_SRCS} ${HUpnp_MOC_SRCS})
set_target_properties(HUpnp PROPERTIES
VERSION "${HUpnp_MAJOR_VERSION}.${HUpnp_MINOR_VERSION}.${HUpnp_PATCH_VERSION}"
SOVERSION "${HUpnp_MAJOR_VERSION}.${HUpnp_MINOR_VERSION}.${HUpnp_PATCH_VERSION}"
)
set_target_properties(HUpnp PROPERTIES COMPILE_FLAGS "-DH_BUILD_UPNP_CORE_LIB") 
set_target_properties(HUpnp PROPERTIES DEBUG_OUTPUT_NAME HUpnpd)
set_target_properties(HUpnp PROPERTIES RELEASE_OUTPUT_NAME HUpnp)
 

 
include_directories( ${CMAKE_SOURCE_DIR}/hupnp/lib/qtsoap-2.7-opensource/src )
include_directories( ${CMAKE_SOURCE_DIR}/hupnp/include )
include(${QT_USE_FILE})
 
 
if (MSVC) # add ws2_32, needed by hupnp, as library
	SET(ADDITIONAL_WIN_LIB ws2_32)
else(MSVC)
	SET(ADDITIONAL_WIN_LIB "")
ENDIF(MSVC)
	
# find_library(QT_SOAP_LIBRARIES QtSolutions_SOAP-2.7)
target_link_libraries(HUpnp
# ${QT_SOAP_LIBRARIES}
# ${QT_LIBRARIES}
QtSolutions_SOAP-2.7
${ADDITIONAL_WIN_LIB}
 ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTMAIN_LIBRARY}
)
 
 
install(TARGETS HUpnp
RUNTIME DESTINATION bin
LIBRARY DESTINATION lib
ARCHIVE DESTINATION lib COMPONENT Devel)
 
install(FILES ${HUpnp_INDIRECT_HEADERS}
DESTINATION include/HUpnpCore
COMPONENT Devel
)
 
install(FILES ${HUpnp_PUBLIC_HEADERS}
DESTINATION include/HUpnpCore/public
COMPONENT Devel
)
