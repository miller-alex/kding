project(kding)

cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS
    Core
    Gui
    Widgets
)
find_package(KF5 REQUIRED COMPONENTS
    Completion
    Config
    ConfigWidgets
    CoreAddons
    DBusAddons
    GlobalAccel
    I18n
    IconThemes
    KHtml
    Notifications
    WidgetsAddons
    WindowSystem
    XmlGui
)

include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(FeatureSummary)

add_subdirectory(src)
add_subdirectory(resources)
add_subdirectory(doc)
add_subdirectory(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
