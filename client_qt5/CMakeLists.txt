#cmake_minimum_required(VERSION 3.1.0)

project(client_qt5)

set (CMAKE_CXX_STANDARD 11)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)


include_directories(../include)


# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)

# Populate a CMake variable with the sources
set(SRCS
    mainwindow.ui
    mainwindow.cpp
    main.cpp
)



add_executable(${PROJECT_NAME}  WIN32 ${SRCS})
target_link_libraries(
    ${PROJECT_NAME}
    Qt5::Widgets
    client_essential
)
