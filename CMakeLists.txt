cmake_minimum_required(VERSION 3.8)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds are not allowed.")
endif()

project(AsteroidsGame VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

message("Project Asteroids.")

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(SDL2 REQUIRED sdl2)
    pkg_search_module(SDL2IMAGE REQUIRED SDL2_image)
    set(SDL2_INCLUDE_PATH ${SDL2_INCLUDE_DIRS})
    set(SDL2_LIBRARY_PATH ${SDL2_LIBRARIES})
    set(SDL2_IMAGE_INCLUDE_PATH ${SDL2IMAGE_INCLUDE_DIRS})
    set(SDL2_IMAGE_LIBRARY_PATH ${SDL2IMAGE_LIBRARIES})
elseif(MSVC)
    message("Searching for SDL2")
    find_path(SDL2_FIND_PATH NAMES "include/SDL.h")
    if(SDL2_FIND_PATH STREQUAL "")
            message(FATAL_ERROR "Couldn't find SDL2 library.")
    endif()
    set(SDL2_INCLUDE_PATH "${SDL2_FIND_PATH}/include")
    if(${CMAKE_SIZEOF_VOID_P} MATCHES 8)
        set(SDL2_LIBRARY_PATH "${SDL2_FIND_PATH}/lib/x64/SDL2.lib")
        set(SDL2_OUTPUT_DLL "${SDL2_FIND_PATH}/lib/x64/SDL2.dll")
    else()
        set(SDL2_LIBRARY_PATH "${SDL2_FIND_PATH}/lib/x86/SDL2.lib")
        set(SDL2_OUTPUT_DLL "${SDL2_FIND_PATH}/lib/x86/SDL2.dll")
    endif()

    message("Searching for SDL2_image")
    find_path(SDL2_IMAGE_FIND_PATH NAMES "include/SDL_image.h")
    if(SDL2_IMAGE_FIND_PATH STREQUAL "")
        message(FATAL_ERROR "Couldn't find SDL2_image library.")
    endif()

    set(SDL2_IMAGE_INCLUDE_PATH "${SDL2_IMAGE_FIND_PATH}/include")
    if(${CMAKE_SIZEOF_VOID_P} MATCHES 8)
        set(SDL2_IMAGE_LIBRARY_PATH "${SDL2_IMAGE_FIND_PATH}/lib/x64/SDL2_image.lib")
        set(SDL2_IMAGE_OUTPUT_DLL "${SDL2_IMAGE_FIND_PATH}/lib/x64/SDL2_image.dll;${SDL2_IMAGE_FIND_PATH}/lib/x64/zlib1.dll;${SDL2_IMAGE_FIND_PATH}/lib/x64/libpng16-16.dll")
    else()
        set(SDL2_IMAGE_LIBRARY_PATH "${SDL2_IMAGE_FIND_PATH}/lib/x86/SDL2_image.lib")
        set(SDL2_IMAGE_OUTPUT_DLL "${SDL2_IMAGE_FIND_PATH}/lib/x86/SDL2_image.dll;${SDL2_IMAGE_FIND_PATH}/lib/x86/zlib1.dll;${SDL2_IMAGE_FIND_PATH}/lib/x86/libpng16-16.dll")
    endif()
else()
    message(FATAL_ERROR "Unsupported operating system.")
endif()

include_directories(${SDL2_INCLUDE_PATH})
include_directories(${SDL2_IMAGE_INCLUDE_PATH})
add_executable(asteroids main.cpp entities.hpp entities.cpp scene.hpp scene.cpp utility.hpp utility.cpp)
target_link_libraries(asteroids ${SDL2_LIBRARY_PATH})
target_link_libraries(asteroids ${SDL2_IMAGE_LIBRARY_PATH})
set_target_properties(asteroids PROPERTIES LINKER_LANGUAGE CXX)

if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT asteroids)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:Debug>,Debug,Release>)
    add_custom_command(
        TARGET asteroids POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SDL2_OUTPUT_DLL}
        $<TARGET_FILE_DIR:asteroids>
    )
    add_custom_command(
        TARGET asteroids POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SDL2_IMAGE_OUTPUT_DLL}
        $<TARGET_FILE_DIR:asteroids>
    )
endif()