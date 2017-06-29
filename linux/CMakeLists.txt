project(hdf5thumbnail)
cmake_minimum_required(VERSION 3.3.1)

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option(gnome "If you have gnome installed" ON)
option(kde "If you have kde installed" ON)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

add_library(base64 STATIC base64.cpp)
add_library(tinyXML2 STATIC tinyxml2.cpp)
include_directories(.)

if(kde)

  message("KDE enabled")
  
  add_subdirectory(kde)

  find_package(KF5 REQUIRED COMPONENTS KIO)

  set(hdf5thumbnail_kde_SRCS kde/hdf5thumbnail.cpp)
  
  add_library(hdf5thumbnail MODULE ${hdf5thumbnail_kde_SRCS})
  
  target_link_libraries(hdf5thumbnail KF5::KIOWidgets tinyXML2 base64)

  set(LIBRARY_INSTALL_DIR /usr/lib/qt5/plugins/)

  install(TARGETS hdf5thumbnail DESTINATION ${LIBRARY_INSTALL_DIR})
  install(FILES kde/hdf5thumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})

endif()

if(gnome)

  message("GNOME enabled")


  add_subdirectory(gnome)

  set(hdf5thumbnail_gnome_SRCS gnome/hdf5thumbnail.cpp)
  
  add_executable(hdf5thumbnailer ${hdf5thumbnail_gnome_SRCS})

  target_link_libraries(hdf5thumbnailer tinyXML2 base64)
  
  install(TARGETS hdf5thumbnailer DESTINATION /usr/bin)
  install(FILES gnome/hdf5.thumbnailer DESTINATION /usr/share/thumbnailers/)
  
endif()
