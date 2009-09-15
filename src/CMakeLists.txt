set(kding_SRCS
    main.cpp
    mainwindow.cpp
    systemtrayicon.cpp
    translationwidget.cpp
    searchengine.cpp
    searchbackendfactory.cpp
    htmlgenerator.cpp
    resultlist.cpp
    application.cpp
    generalsettings.cpp
    backendsettings.cpp
)

kde4_add_ui_files( kding_SRCS
    translationwidgetbase.ui
    generalsettingsbase.ui
    backendsettingsbase.ui
)

kde4_add_kcfg_files( kding_SRCS settings.kcfgc )

kde4_add_executable( kding ${kding_SRCS} )

target_link_libraries( kding ${KDE4_KDEUI_LIBS} ${KDE4_KHTML_LIBS} )

install( FILES kding.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
install( TARGETS kding ${INSTALL_TARGETS_DEFAULT_ARGS} )
