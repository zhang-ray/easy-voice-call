#cmake_minimum_required(VERSION 3.1.0)

project(client_qt5)

set (CMAKE_CXX_STANDARD 11)

if(WIN32)
    set(CMAKE_PREFIX_PATH "C:/Qt/5.11.1/msvc2015")
else(WIN32)
    if(APPLE AND EXISTS /usr/local/opt/qt5)
        # ensure it can be found by CMake since it is not in the default /usr/local prefix.
        list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/qt5")
    endif()
endif(WIN32)


# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)


include_directories(../include)
if(WIN32)
	include_directories(../../include/)
endif(WIN32)



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
