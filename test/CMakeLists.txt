project(hdf5thumbnailer)
cmake_minimum_required(VERSION 3.3.1)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
find_package(Qt5Gui)

# Tell CMake to create the helloworld executable
add_executable(helloworld test.cpp)

# Use the Widgets module from Qt 5.
target_link_libraries(helloworld Qt5::Gui hdf5 hdf5_cpp hdf5_hl)