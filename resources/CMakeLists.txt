add_subdirectory(icons)
add_subdirectory(html)

install(FILES kdingui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kding)
install(FILES de-en.txt DESTINATION ${DATA_INSTALL_DIR}/kding)
install(FILES kding.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

#include(GNUInstallDirs)
#install(FILES Readme.de-en.txt DESTINATION ${CMAKE_INSTALL_DOCDIR})
