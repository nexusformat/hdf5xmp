project(hdf5thumbnail)
cmake_minimum_required(VERSION 3.3.1)
set(QT_MIN_VERSION "5.2.0")

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option(gnome "If you have gnome installed" ON)
option(kde "If you have kde installed" ON)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Gui)

include(FeatureSummary)
include(WriteBasicConfigVersionFile)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

if(kde)

  message("KDE enabled")
  
  add_subdirectory(kde)

  find_package(KF5 REQUIRED COMPONENTS KIO Archive)

  set(hdf5thumbnail_kde_SRCS kde/hdf5thumbnail.cpp)
  

  add_library(hdf5thumbnail MODULE ${hdf5thumbnail_kde_SRCS})

  
  target_link_libraries(hdf5thumbnail Qt5::Gui KF5::KIOWidgets KF5::Archive hdf5 hdf5_cpp hdf5_hl)

  set(LIBRARY_INSTALL_DIR /usr/lib/qt5/plugins/)

  install(TARGETS hdf5thumbnail DESTINATION ${LIBRARY_INSTALL_DIR})
  install(FILES kde/hdf5thumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})

endif()

if(gnome)

  message("GNOME enabled")


  add_subdirectory(gnome)

  set(hdf5thumbnail_gnome_SRCS gnome/hdf5thumbnail.cpp)

  add_executable(hdf5thumbnailer ${hdf5thumbnail_gnome_SRCS})
  
  target_link_libraries(hdf5thumbnailer hdf5 hdf5_cpp hdf5_hl)

  install(TARGETS hdf5thumbnailer DESTINATION /usr/bin)
  install(FILES gnome/hdf5.thumbnailer DESTINATION /usr/share/thumbnailers/)
  
endif()
