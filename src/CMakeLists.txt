
set (CMAKE_CXX_STANDARD 11)

include_directories(../include/)

file(GLOB_RECURSE FILES_SOURCES *.cpp *.c)
file(GLOB_RECURSE FILES_INCLUDE *.hpp *.h ../include/*.hpp ../include/*.h)

if(WIN32)
    link_directories("C:/projects/easy-voice-call/")
endif(WIN32)

add_executable(${PROJECT_NAME} ${FILES_INCLUDE} ${FILES_SOURCES} )
#target_link_libraries(${PROJECT_NAME} asound pthread)


if(WIN32)
    target_link_libraries(${PROJECT_NAME} opus portaudio_static_x86)
else(WIN32)
    target_link_libraries(${PROJECT_NAME} portaudio opus pthread
    #fdk-aac
    )
endif(WIN32)