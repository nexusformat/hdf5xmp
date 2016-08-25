project(hdf5thumbnailer)
cmake_minimum_required(VERSION 3.3.1)
set(QT_MIN_VERSION "5.2.0")

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(FeatureSummary)
include(WriteBasicConfigVersionFile)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Gui)
find_package(KF5 REQUIRED COMPONENTS KIO Archive)

set(hdf5thumbnail_SRCS hdf5thumbnail.cpp)

add_library(hdf5thumbnail MODULE ${hdf5thumbnail_SRCS})
target_link_libraries(hdf5thumbnail Qt5::Gui KF5::KIOWidgets KF5::Archive hdf5 hdf5_cpp hdf5_hl)

install(TARGETS hdf5thumbnail DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES hdf5thumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})
