set (CMAKE_CXX_STANDARD 14)

include_directories(../../include/evc/)
if(WIN32)
	include_directories(
        ../../../include/
        "C:/Libraries/boost_1_67_0/"
        )
endif(WIN32)


if(WIN32)
    link_directories(
        "C:/projects/VS2010/Win32/Release/"
        "C:/projects/build/Release/"
        "C:/Libraries/boost_1_67_0/lib32-msvc-14.0/"
    )
endif(WIN32)

add_executable(sanity_check_audio audio/Entry.cpp)
target_link_libraries(sanity_check_audio client_essential)