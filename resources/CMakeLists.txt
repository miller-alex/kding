add_subdirectory( icons )

install( FILES kdingui.rc kding.css welcome.html DESTINATION ${DATA_INSTALL_DIR}/kding )
install( FILES de-en.txt Readme.de-en.txt DESTINATION ${DATA_INSTALL_DIR}/kding )
install( FILES kding.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
