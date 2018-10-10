project(client_cli)

set (CMAKE_CXX_STANDARD 11)

include_directories(../include/)

if(WIN32)
	include_directories(../../include/)
endif(WIN32)

file(GLOB_RECURSE FILES_SOURCES *.cpp *.c)
file(GLOB_RECURSE FILES_INCLUDE *.hpp *.h ../include/*.hpp ../include/*.h)


if(WIN32)
    link_directories("C:/projects/VS2010/Win32/Release/"  "C:/projects/build/Release/")
endif(WIN32)





add_executable(${PROJECT_NAME} ${FILES_INCLUDE} ${FILES_SOURCES} )
#target_link_libraries(${PROJECT_NAME} asound pthread)


target_link_libraries(${PROJECT_NAME} client_essential)
