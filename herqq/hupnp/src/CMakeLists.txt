set(HUpnp_SRCS
dataelements/hactioninfo.cpp
dataelements/hdeviceinfo.cpp
dataelements/hserviceinfo.cpp
dataelements/hserviceid.cpp
dataelements/hudn.cpp
dataelements/hresourcetype.cpp
dataelements/hproduct_tokens.cpp
dataelements/hdiscoverytype.cpp
dataelements/hstatevariableinfo.cpp
dataelements/hvaluerange_p.cpp
 
devicehosting/hdevicestorage_p.cpp
devicehosting/hddoc_parser_p.cpp
devicehosting/hmodelcreation_p.cpp
devicehosting/messages/hcontrol_messages_p.cpp
devicehosting/messages/hevent_messages_p.cpp
devicehosting/messages/hnt_p.cpp
devicehosting/messages/hsid_p.cpp
devicehosting/messages/htimeout_p.cpp
devicehosting/controlpoint/hcontrolpoint.cpp
devicehosting/controlpoint/hclientmodel_creator_p.cpp
devicehosting/controlpoint/hdevicebuild_p.cpp
devicehosting/controlpoint/hcontrolpoint_configuration.cpp
devicehosting/controlpoint/hcontrolpoint_dataretriever_p.cpp
devicehosting/controlpoint/hevent_subscription_p.cpp
devicehosting/controlpoint/hevent_subscriptionmanager_p.cpp
devicehosting/devicehost/hdevicehost.cpp
devicehosting/devicehost/hservermodel_creator_p.cpp
devicehosting/devicehost/hdevicehost_dataretriever_p.cpp
devicehosting/devicehost/hevent_notifier_p.cpp
devicehosting/devicehost/hdevicehost_configuration.cpp
devicehosting/devicehost/hdevicehost_ssdp_handler_p.cpp
devicehosting/devicehost/hdevicehost_http_server_p.cpp
devicehosting/devicehost/hevent_subscriber_p.cpp
 
devicemodel/hactions_setupdata.cpp
devicemodel/hasyncop.cpp
devicemodel/hexecargs.cpp
devicemodel/hactionarguments.cpp
devicemodel/hdevices_setupdata.cpp
devicemodel/hservices_setupdata.cpp
devicemodel/hstatevariable_event.cpp
devicemodel/hstatevariables_setupdata.cpp
devicemodel/hdevicemodel_infoprovider.cpp
devicemodel/client/hclientdevice.cpp
devicemodel/client/hclientaction.cpp
devicemodel/client/hclientactionop.cpp
devicemodel/client/hclientservice.cpp
devicemodel/client/hclientstatevariable.cpp
devicemodel/server/hserveraction.cpp
devicemodel/server/hserverdevice.cpp
devicemodel/server/hserverservice.cpp
devicemodel/server/hserverstatevariable.cpp
devicemodel/server/hdevicemodelcreator.cpp
 
general/hupnp_global.cpp
general/hclonable.cpp
general/hlogger_p.cpp
general/hupnpinfo.cpp
general/hupnp_datatypes.cpp
 
http/hhttp_utils_p.cpp
http/hhttp_header_p.cpp
http/hhttp_server_p.cpp
http/hhttp_asynchandler_p.cpp
http/hhttp_messaginginfo_p.cpp
http/hhttp_messagecreator_p.cpp
 
socket/hendpoint.cpp
socket/hmulticast_socket.cpp
 
ssdp/hssdp.cpp
ssdp/hdiscovery_messages.cpp
ssdp/hssdp_messagecreator_p.cpp
utils/hmisc_utils_p.cpp
utils/hsysutils_p.cpp
utils/hthreadpool_p.cpp
 
 
http/hhttp_utils_p.cpp
http/hhttp_header_p.cpp
http/hhttp_server_p.cpp
http/hhttp_asynchandler_p.cpp
http/hhttp_messaginginfo_p.cpp
http/hhttp_messagecreator_p.cpp
)
 
set(HUpnp_MOC_HDRS
utils/hthreadpool_p.h
 
devicemodel/server/hserverservice.h
devicemodel/server/hserveraction.h
devicemodel/server/hserverdevice.h
devicemodel/server/hserverstatevariable.h
devicemodel/client/hclientservice.h
devicemodel/client/hdefault_clientdevice_p.h
devicemodel/client/hclientaction_p.h
devicemodel/client/hclientstatevariable.h
devicemodel/client/hclientaction.h
devicemodel/client/hclientdevice.h
http/hhttp_asynchandler_p.h
http/hhttp_server_p.h
ssdp/hssdp.h
devicehosting/devicehost/hdevicehost_p.h
devicehosting/devicehost/hserverdevicecontroller_p.h
devicehosting/devicehost/hevent_subscriber_p.h
devicehosting/devicehost/hevent_notifier_p.h
devicehosting/devicehost/hdevicehost_ssdp_handler_p.h
devicehosting/devicehost/hdevicehost_http_server_p.h
devicehosting/devicehost/hdevicehost.h
devicehosting/controlpoint/hdevicebuild_p.h
devicehosting/controlpoint/hcontrolpoint.h
devicehosting/controlpoint/hcontrolpoint_dataretriever_p.h
devicehosting/controlpoint/hcontrolpoint_p.h
devicehosting/controlpoint/hevent_subscriptionmanager_p.h
devicehosting/controlpoint/hevent_subscription_p.h
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
