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

ki18n_wrap_ui(kding_SRCS
    translationwidgetbase.ui
    generalsettingsbase.ui
    backendsettingsbase.ui
)

kconfig_add_kcfg_files(kding_SRCS settings.kcfgc)

add_executable(kding ${kding_SRCS})

target_link_libraries(kding
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    KF5::ConfigCore
    KF5::ConfigGui
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::DBusAddons
    KF5::GlobalAccel
    KF5::I18n
    KF5::IconThemes
    KF5::KHtml
    KF5::Notifications
    KF5::WidgetsAddons
    KF5::WindowSystem
    KF5::XmlGui
)

install(FILES kding.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(TARGETS kding ${INSTALL_TARGETS_DEFAULT_ARGS})
