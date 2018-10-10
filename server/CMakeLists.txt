project(server)

set (CMAKE_CXX_STANDARD 11)

include_directories(../include/)

file(GLOB_RECURSE FILES_SOURCES *.cpp *.c)
#file(GLOB_RECURSE FILES_INCLUDE *.hpp *.h ../include/*.hpp ../include/*.h)

add_executable(${PROJECT_NAME} ${FILES_INCLUDE} ${FILES_SOURCES} )
target_link_libraries(${PROJECT_NAME} 
boost_system 
boost_log
pthread)
