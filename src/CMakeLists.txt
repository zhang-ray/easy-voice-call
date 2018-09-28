
set (CMAKE_CXX_STANDARD 11)

include_directories(../include/)

file(GLOB_RECURSE FILES_SOURCES *.cpp *.c)
file(GLOB_RECURSE FILES_INCLUDE *.hpp *.h ../include/*.hpp ../include/*.h)

if(WIN32)
    link_directories(../VS2010/Win32/Debug)
endif(WIN32)

add_executable(${PROJECT_NAME} ${FILES_INCLUDE} ${FILES_SOURCES} )
#target_link_libraries(${PROJECT_NAME} asound pthread)
target_link_libraries(${PROJECT_NAME} 
pthread
portaudio
#fdk-aac
opus
)
