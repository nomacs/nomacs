configure_file(org.nomacs.ImageLounge.appdata.xml.in org.nomacs.ImageLounge.appdata.xml)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/org.nomacs.ImageLounge.appdata.xml
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo
)

install(FILES org.nomacs.ImageLounge.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)

install(
    FILES ../nomacs.svg
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps
    RENAME org.nomacs.ImageLounge.svg
)
