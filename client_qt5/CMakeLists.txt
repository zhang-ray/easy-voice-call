#cmake_minimum_required(VERSION 3.1.0)

project(client_qt5)

set (CMAKE_CXX_STANDARD 14)


include_directories(../include)


if(WIN32)
    set(CMAKE_PREFIX_PATH "C:/Qt/5.11.1/msvc2015")
    include_directories(
        ../../include/
        "C:/Libraries/boost_1_67_0/"
    )
    link_directories(
        "C:/projects/VS2010/Win32/Release/"
        "C:/projects/build/Release/"
        "C:/Libraries/boost_1_67_0/lib32-msvc-14.0/"
        )
else(WIN32)
	add_definitions(-DBOOST_LOG_DYN_LINK)
    if(APPLE AND EXISTS /usr/local/opt/qt5)
        # ensure it can be found by CMake since it is not in the default /usr/local prefix.
        list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/qt5")
    endif()
	link_directories(/usr/local/lib/)
endif(WIN32)



# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)



# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)

# Populate a CMake variable with the sources
set(SRCS
    mainwindow.ui
    mainwindow.hpp
    mainwindow.cpp
    main.cpp
)

# QRC files
set(QRC_SOURCE_FILES resources/Resource.qrc)
qt5_add_resources(QRC_FILES ${QRC_SOURCE_FILES})



if(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${SRCS} ${QRC_FILES} resources/Resource.rc)
else(WIN32)
    add_executable(${PROJECT_NAME} ${SRCS} ${QRC_FILES})
endif(WIN32)


if(WIN32)
    target_link_libraries(${PROJECT_NAME} Qt5::Widgets client_essential Qt5::WinMain)
else(WIN32)
	if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	    target_link_libraries(${PROJECT_NAME}
			Qt5::Widgets
			client_essential
			boost_system boost_system-mt 
			boost_log_setup boost_log_setup-mt 
			boost_log boost_log-mt 
			boost_filesystem boost_filesystem-mt 
			boost_thread-mt
			pthread
		)
    endif()
endif(WIN32)
