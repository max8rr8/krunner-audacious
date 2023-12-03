cmake_minimum_required(VERSION 3.16)
project(audaciousrunner)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(QT_MIN_VERSION "6.4.0")
set(KF_MIN_VERSION "5.240.0")


find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui)
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS Runner I18n)

include(KDEInstallDirs)
include(KDEClangFormat)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS I18n Config Runner KIO Notifications)

# This takes care of building and installing the plugin
kcoreaddons_add_plugin(audaciousrunner SOURCES audaciousrunner.cpp INSTALL_NAMESPACE "kf6/krunner")
# We need to link the KRunner and other used libraries  to it
target_link_libraries(audaciousrunner
    KF6::I18n
    KF6::ConfigCore
    KF6::Runner
    KF6::KIOWidgets
    KF6::Notifications
)