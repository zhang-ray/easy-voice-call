#cmake_minimum_required(VERSION 3.1.0)

project(evc_qt5)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)

# Populate a CMake variable with the sources
set(evc_qt5_SRCS
    mainwindow.ui
    mainwindow.cpp
    main.cpp
)
# Tell CMake to create the evc_qt5 executable
add_executable(evc_qt5 WIN32 ${evc_qt5_SRCS})
# Use the Widgets module from Qt 5
target_link_libraries(evc_qt5 Qt5::Widgets)