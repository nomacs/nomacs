set(HUpnpAV_SRCS
cds_model/cds_objects/halbum.cpp
cds_model/cds_objects/haudiobook.cpp
cds_model/cds_objects/haudiobroadcast.cpp
cds_model/cds_objects/haudiochannelgroup.cpp
cds_model/cds_objects/haudioitem.cpp
cds_model/cds_objects/haudioprogram.cpp
cds_model/cds_objects/hbookmarkfolder.cpp
cds_model/cds_objects/hbookmarkitem.cpp
cds_model/cds_objects/hchannelgroup.cpp
cds_model/cds_objects/hcontainer.cpp
cds_model/cds_objects/hepgcontainer.cpp
cds_model/cds_objects/hepgitem.cpp
cds_model/cds_objects/hgenrecontainer.cpp
cds_model/cds_objects/himageitem.cpp
cds_model/cds_objects/hitem.cpp
cds_model/cds_objects/hmovie.cpp
cds_model/cds_objects/hmoviegenre.cpp
cds_model/cds_objects/hmusicalbum.cpp
cds_model/cds_objects/hmusicartist.cpp
cds_model/cds_objects/hmusicgenre.cpp
cds_model/cds_objects/hmusictrack.cpp
cds_model/cds_objects/hmusicvideoclip.cpp
cds_model/cds_objects/hobject.cpp
cds_model/cds_objects/hperson.cpp
cds_model/cds_objects/hphoto.cpp
cds_model/cds_objects/hphotoalbum.cpp
cds_model/cds_objects/hplaylistcontainer.cpp
cds_model/cds_objects/hplaylistitem.cpp
cds_model/cds_objects/hstoragefolder.cpp
cds_model/cds_objects/hstoragesystem.cpp
cds_model/cds_objects/hstoragevolume.cpp
cds_model/cds_objects/htextitem.cpp
cds_model/cds_objects/hvideobroadcast.cpp
cds_model/cds_objects/hvideochannelgroup.cpp
cds_model/cds_objects/hvideoitem.cpp
cds_model/cds_objects/hvideoprogram.cpp
cds_model/datasource/habstract_cds_datasource.cpp
cds_model/datasource/hcds_datasource.cpp
cds_model/datasource/hcds_datasource_configuration.cpp
cds_model/datasource/hfsys_datasource.cpp
cds_model/datasource/hfsys_datasource_configuration.cpp
cds_model/datasource/hrootdir.cpp
cds_model/hcdsclassinfo.cpp
cds_model/hchannelgroupname.cpp
cds_model/hchannel_id.cpp
cds_model/hcontentduration.cpp
cds_model/hdatetimerange.cpp
cds_model/hdeviceudn.cpp
cds_model/hforeignmetadata.cpp
cds_model/hgenre.cpp
cds_model/hmatching_id.cpp
cds_model/hpersonwithrole.cpp
cds_model/hprice.cpp
cds_model/hprogramcode.cpp
cds_model/hscheduledtime.cpp
cds_model/hsortinfo.cpp
cds_model/hstatevariablecollection.cpp
cds_model/model_mgmt/hcdsobjectdata_p.cpp
cds_model/model_mgmt/hcdsproperties.cpp
cds_model/model_mgmt/hcdsproperty.cpp
cds_model/model_mgmt/hcdspropertyinfo.cpp
cds_model/model_mgmt/hcdsproperty_db.cpp
cds_model/model_mgmt/hcds_dlite_serializer.cpp
cds_model/model_mgmt/hcds_fsys_reader_p.cpp
common/hprotocolinfo.cpp
common/hradioband.cpp
common/hrating.cpp
common/hresource.cpp
common/hstoragemedium.cpp
connectionmanager/habstractconnectionmanager_service.cpp
connectionmanager/hconnectioninfo.cpp
connectionmanager/hconnectionmanager_adapter.cpp
connectionmanager/hconnectionmanager_id.cpp
connectionmanager/hconnectionmanager_info.cpp
connectionmanager/hconnectionmanager_serviceconfiguration.cpp
connectionmanager/hconnectionmanager_service_p.cpp
connectionmanager/hprepareforconnection_result.cpp
connectionmanager/hprotocolinforesult.cpp
contentdirectory/habstractcontentdirectory_service.cpp
contentdirectory/hcontentdirectory_adapter.cpp
contentdirectory/hcontentdirectory_info.cpp
contentdirectory/hcontentdirectory_service.cpp
contentdirectory/hcontentdirectory_serviceconfiguration.cpp
contentdirectory/hcreateobjectresult.cpp
contentdirectory/hfreeformqueryresult.cpp
contentdirectory/hsearchresult.cpp
contentdirectory/htransferprogressinfo.cpp
controlpoint/hav_controlpoint.cpp
controlpoint/hav_controlpoint_configuration.cpp
hav_devicemodel_creator.cpp
hav_devicemodel_infoprovider.cpp
hav_global.cpp
hupnpavinfo.cpp
mediabrowser/hcds_lastchange_info.cpp
mediabrowser/hmediabrowser.cpp
mediarenderer/habstractmediarenderer_device.cpp
mediarenderer/havt_lastchange_info.cpp
mediarenderer/hconnection.cpp
mediarenderer/hconnectionmanager_sinkservice_p.cpp
mediarenderer/hmediarenderer_adapter.cpp
mediarenderer/hmediarenderer_deviceconfiguration.cpp
mediarenderer/hmediarenderer_device_p.cpp
mediarenderer/hmediarenderer_info.cpp
mediarenderer/hrcs_lastchange_info.cpp
mediarenderer/hrendererconnection.cpp
mediarenderer/hrendererconnection_info.cpp
mediarenderer/hrendererconnection_manager.cpp
mediarenderer/htransport_sinkservice_p.cpp
mediaserver/habstractmediaserver_device.cpp
mediaserver/hconnectionmanager_sourceservice_p.cpp
mediaserver/hmediaserver_adapter.cpp
mediaserver/hmediaserver_deviceconfiguration.cpp
mediaserver/hmediaserver_device_p.cpp
mediaserver/hmediaserver_info.cpp
renderingcontrol/habstractrenderingcontrol_service.cpp
renderingcontrol/hchannel.cpp
renderingcontrol/hrenderingcontrol_adapter.cpp
renderingcontrol/hrenderingcontrol_info.cpp
renderingcontrol/hrenderingcontrol_service_p.cpp
renderingcontrol/hvolumedbrange_result.cpp
transport/habstract_avtransport_service.cpp
transport/havtransport_adapter.cpp
transport/havtransport_info.cpp
transport/hdevicecapabilities.cpp
transport/hduration.cpp
transport/hmediainfo.cpp
transport/hplaymode.cpp
transport/hpositioninfo.cpp
transport/hrecordmediumwritestatus.cpp
transport/hrecordqualitymode.cpp
transport/hseekinfo.cpp
transport/htransportaction.cpp
transport/htransportinfo.cpp
transport/htransportsettings.cpp
transport/htransportstate.cpp
)
 
set(HUpnpAV_HEADERS
cds_model/cds_objects/halbum.h
cds_model/cds_objects/halbum_p.h
cds_model/cds_objects/haudiobook.h
cds_model/cds_objects/haudiobook_p.h
cds_model/cds_objects/haudiobroadcast.h
cds_model/cds_objects/haudiobroadcast_p.h
cds_model/cds_objects/haudiochannelgroup.h
cds_model/cds_objects/haudiochannelgroup_p.h
cds_model/cds_objects/haudioitem.h
cds_model/cds_objects/haudioitem_p.h
cds_model/cds_objects/haudioprogram.h
cds_model/cds_objects/haudioprogram_p.h
cds_model/cds_objects/hbookmarkfolder.h
cds_model/cds_objects/hbookmarkfolder_p.h
cds_model/cds_objects/hbookmarkitem.h
cds_model/cds_objects/hbookmarkitem_p.h
cds_model/cds_objects/hchannelgroup.h
cds_model/cds_objects/hchannelgroup_p.h
cds_model/cds_objects/hcontainer.h
cds_model/cds_objects/hcontainer_p.h
cds_model/cds_objects/hepgcontainer.h
cds_model/cds_objects/hepgcontainer_p.h
cds_model/cds_objects/hepgitem.h
cds_model/cds_objects/hepgitem_p.h
cds_model/cds_objects/hgenrecontainer.h
cds_model/cds_objects/hgenrecontainer_p.h
cds_model/cds_objects/himageitem.h
cds_model/cds_objects/himageitem_p.h
cds_model/cds_objects/hitem.h
cds_model/cds_objects/hitem_p.h
cds_model/cds_objects/hmovie.h
cds_model/cds_objects/hmoviegenre.h
cds_model/cds_objects/hmoviegenre_p.h
cds_model/cds_objects/hmovie_p.h
cds_model/cds_objects/hmusicalbum.h
cds_model/cds_objects/hmusicalbum_p.h
cds_model/cds_objects/hmusicartist.h
cds_model/cds_objects/hmusicartist_p.h
cds_model/cds_objects/hmusicgenre.h
cds_model/cds_objects/hmusicgenre_p.h
cds_model/cds_objects/hmusictrack.h
cds_model/cds_objects/hmusictrack_p.h
cds_model/cds_objects/hmusicvideoclip.h
cds_model/cds_objects/hmusicvideoclip_p.h
cds_model/cds_objects/hobject.h
cds_model/cds_objects/hobject_p.h
cds_model/cds_objects/hperson.h
cds_model/cds_objects/hperson_p.h
cds_model/cds_objects/hphoto.h
cds_model/cds_objects/hphotoalbum.h
cds_model/cds_objects/hphotoalbum_p.h
cds_model/cds_objects/hphoto_p.h
cds_model/cds_objects/hplaylistcontainer.h
cds_model/cds_objects/hplaylistcontainer_p.h
cds_model/cds_objects/hplaylistitem.h
cds_model/cds_objects/hplaylistitem_p.h
cds_model/cds_objects/hstoragefolder.h
cds_model/cds_objects/hstoragefolder_p.h
cds_model/cds_objects/hstoragesystem.h
cds_model/cds_objects/hstoragesystem_p.h
cds_model/cds_objects/hstoragevolume.h
cds_model/cds_objects/hstoragevolume_p.h
cds_model/cds_objects/htextitem.h
cds_model/cds_objects/htextitem_p.h
cds_model/cds_objects/hvideobroadcast.h
cds_model/cds_objects/hvideobroadcast_p.h
cds_model/cds_objects/hvideochannelgroup.h
cds_model/cds_objects/hvideochannelgroup_p.h
cds_model/cds_objects/hvideoitem.h
cds_model/cds_objects/hvideoitem_p.h
cds_model/cds_objects/hvideoprogram.h
cds_model/cds_objects/hvideoprogram_p.h
cds_model/datasource/habstract_cds_datasource.h
cds_model/datasource/habstract_cds_datasource_p.h
cds_model/datasource/hcds_datasource.h
cds_model/datasource/hcds_datasource_configuration.h
cds_model/datasource/hcds_datasource_configuration_p.h
cds_model/datasource/hcds_datasource_p.h
cds_model/datasource/hfsys_datasource.h
cds_model/datasource/hfsys_datasource_configuration.h
cds_model/datasource/hfsys_datasource_configuration_p.h
cds_model/datasource/hfsys_datasource_p.h
cds_model/datasource/hrootdir.h
cds_model/hcdsclassinfo.h
cds_model/hchannelgroupname.h
cds_model/hchannel_id.h
cds_model/hcontentduration.h
cds_model/hdatetimerange.h
cds_model/hdeviceudn.h
cds_model/hforeignmetadata.h
cds_model/hgenre.h
cds_model/hmatching_id.h
cds_model/hpersonwithrole.h
cds_model/hprice.h
cds_model/hprogramcode.h
cds_model/hscheduledtime.h
cds_model/hsortinfo.h
cds_model/hstatevariablecollection.h
cds_model/model_mgmt/hcdsobjectdata_p.h
cds_model/model_mgmt/hcdsproperties.h
cds_model/model_mgmt/hcdsproperty.h
cds_model/model_mgmt/hcdspropertyinfo.h
cds_model/model_mgmt/hcdsproperty_db.h
cds_model/model_mgmt/hcdsproperty_db_p.h
cds_model/model_mgmt/hcds_dlite_serializer.h
cds_model/model_mgmt/hcds_dlite_serializer_p.h
cds_model/model_mgmt/hcds_fsys_reader_p.h
common/hprotocolinfo.h
common/hradioband.h
common/hrating.h
common/hresource.h
common/hstoragemedium.h
connectionmanager/habstractconnectionmanager_service.h
connectionmanager/habstractconnectionmanager_service_p.h
connectionmanager/hconnectioninfo.h
connectionmanager/hconnectionmanager_adapter.h
connectionmanager/hconnectionmanager_adapter_p.h
connectionmanager/hconnectionmanager_id.h
connectionmanager/hconnectionmanager_info.h
connectionmanager/hconnectionmanager_serviceconfiguration.h
connectionmanager/hconnectionmanager_serviceconfiguration_p.h
connectionmanager/hconnectionmanager_service_p.h
connectionmanager/hprepareforconnection_result.h
connectionmanager/hprotocolinforesult.h
contentdirectory/habstractcontentdirectory_service.h
contentdirectory/habstractcontentdirectory_service_p.h
contentdirectory/hcontentdirectory_adapter.h
contentdirectory/hcontentdirectory_adapter_p.h
contentdirectory/hcontentdirectory_info.h
contentdirectory/hcontentdirectory_service.h
contentdirectory/hcontentdirectory_serviceconfiguration.h
contentdirectory/hcontentdirectory_serviceconfiguration_p.h
contentdirectory/hcontentdirectory_service_p.h
contentdirectory/hcreateobjectresult.h
contentdirectory/hfreeformqueryresult.h
contentdirectory/hsearchresult.h
contentdirectory/htransferprogressinfo.h
controlpoint/hav_controlpoint.h
controlpoint/hav_controlpoint_configuration.h
controlpoint/hav_controlpoint_configuration_p.h
controlpoint/hav_controlpoint_p.h
hav_defs.h
hav_devicemodel_creator.h
hav_devicemodel_infoprovider.h
hav_fwd.h
hav_global.h
hav_global_p.h
hupnpavinfo.h
mediabrowser/hcds_lastchange_info.h
mediabrowser/hmediabrowser.h
mediabrowser/hmediabrowser_p.h
mediarenderer/habstractmediarenderer_device.h
mediarenderer/havt_lastchange_info.h
mediarenderer/hconnection.h
mediarenderer/hconnectionmanager_sinkservice_p.h
mediarenderer/hmediarenderer_adapter.h
mediarenderer/hmediarenderer_adapter_p.h
mediarenderer/hmediarenderer_deviceconfiguration.h
mediarenderer/hmediarenderer_deviceconfiguration_p.h
mediarenderer/hmediarenderer_device_p.h
mediarenderer/hmediarenderer_info.h
mediarenderer/hrcs_lastchange_info.h
mediarenderer/hrendererconnection.h
mediarenderer/hrendererconnection_info.h
mediarenderer/hrendererconnection_info_p.h
mediarenderer/hrendererconnection_manager.h
mediarenderer/hrendererconnection_manager_p.h
mediarenderer/hrendererconnection_p.h
mediarenderer/htransport_sinkservice_p.h
mediaserver/habstractmediaserver_device.h
mediaserver/hconnectionmanager_sourceservice_p.h
mediaserver/hmediaserver_adapter.h
mediaserver/hmediaserver_adapter_p.h
mediaserver/hmediaserver_deviceconfiguration.h
mediaserver/hmediaserver_deviceconfiguration_p.h
mediaserver/hmediaserver_device_p.h
mediaserver/hmediaserver_info.h
renderingcontrol/habstractrenderingcontrol_service.h
renderingcontrol/habstractrenderingcontrol_service_p.h
renderingcontrol/hchannel.h
renderingcontrol/hrenderingcontrol_adapter.h
renderingcontrol/hrenderingcontrol_adapter_p.h
renderingcontrol/hrenderingcontrol_info.h
renderingcontrol/hrenderingcontrol_service_p.h
renderingcontrol/hvolumedbrange_result.h
transport/habstract_avtransport_service.h
transport/habstract_avtransport_service_p.h
transport/havtransport_adapter.h
transport/havtransport_adapter_p.h
transport/havtransport_info.h
transport/hdevicecapabilities.h
transport/hduration.h
transport/hmediainfo.h
transport/hplaymode.h
transport/hpositioninfo.h
transport/hrecordmediumwritestatus.h
transport/hrecordqualitymode.h
transport/hseekinfo.h
transport/htransportaction.h
transport/htransportinfo.h
transport/htransportsettings.h
transport/htransportstate.h
)
 
qt4_wrap_cpp(HUpnpAV_MOCS ${HUpnpAV_HEADERS})


include_directories( ${CMAKE_SOURCE_DIR}/hupnp/include )      
include_directories( ${CMAKE_SOURCE_DIR}/hupnp/include/private )      
include_directories( ${CMAKE_SOURCE_DIR}/hupnp_av/include )   
add_library(HUpnpAV SHARED ${HUpnpAV_SRCS} ${HUpnpAV_MOCS})
set_target_properties(HUpnpAV PROPERTIES
VERSION "${HUpnp_MAJOR_VERSION}.${HUpnp_MINOR_VERSION}.${HUpnp_PATCH_VERSION}"
SOVERSION "${HUpnp_MAJOR_VERSION}.${HUpnp_MINOR_VERSION}.${HUpnp_PATCH_VERSION}"
)
set_target_properties(HUpnpAV PROPERTIES COMPILE_FLAGS "-DH_BUILD_UPNP_AV_LIB") 
set_target_properties(HUpnpAV PROPERTIES DEBUG_OUTPUT_NAME HUpnpAVd)
set_target_properties(HUpnpAV PROPERTIES RELEASE_OUTPUT_NAME HUpnpAV)
 

 
# include_directories( ${CMAKE_SOURCE_DIR}/hupnp/lib/qtsoap-2.7-opensource/src )
# include_directories( ${CMAKE_SOURCE_DIR}/hupnp/include )
include(${QT_USE_FILE})
 
 
if (MSVC) # add ws2_32, needed by hupnp, as library
	SET(ADDITIONAL_WIN_LIB ws2_32)
else(MSVC)
	SET(ADDITIONAL_WIN_LIB "")
ENDIF(MSVC)
	
 target_link_libraries(HUpnpAV
 ${HUPNP_LIBS}
 ${QT_LIBRARIES}
 ${ADDITIONAL_WIN_LIB}
 ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTMAIN_LIBRARY}
)
 
 
# install(TARGETS HUpnp
# RUNTIME DESTINATION bin
# LIBRARY DESTINATION lib
# ARCHIVE DESTINATION lib COMPONENT Devel)
 
# install(FILES ${HUpnp_INDIRECT_HEADERS}
# DESTINATION include/HUpnpCore
# COMPONENT Devel
# )
 
# install(FILES ${HUpnp_PUBLIC_HEADERS}
# DESTINATION include/HUpnpCore/public
# COMPONENT Devel
# )
