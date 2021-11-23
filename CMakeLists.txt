cmake_minimum_required(VERSION 3.8)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds are not allowed.")
endif()

project(Asteroids VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(SDL2 REQUIRED sdl2)
    pkg_search_module(SDL2IMAGE REQUIRED SDL2_image)
    #pkg_search_module(SDL2TTF REQUIRED SDL2_ttf)
    set(SDL2_INCLUDE_PATH ${SDL2_INCLUDE_DIRS})
    set(SDL2_LIBRARY_PATH ${SDL2_LIBRARIES})
    set(SDL2_IMAGE_INCLUDE_PATH ${SDL2IMAGE_INCLUDE_DIRS})
    set(SDL2_IMAGE_LIBRARY_PATH ${SDL2IMAGE_LIBRARIES})
    #set(SDL2_TTF_INCLUDE_PATH ${SDL2TTF_INCLUDE_DIRS})
    #set(SDL2_TTF_LIBRARY_PATH ${SDL2TTF_LIBRARIES})
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

    #message("Searching for SDL2_ttf")
    #find_path(SDL2_TTF_FIND_PATH NAMES "include/SDL_ttf.h")
    #if(SDL2_TTF_FIND_PATH STREQUAL "")
        #message(FATAL_ERROR "Couldn't find SDL2_ttf library.")
    #endif()

    #set(SDL2_TTF_INCLUDE_PATH "${SDL2_TTF_FIND_PATH}/include")
    #if(${CMAKE_SIZEOF_VOID_P} MATCHES 8)
        #set(SDL2_TTF_LIBRARY_PATH "${SDL2_TTF_FIND_PATH}/lib/x64/SDL2_ttf.lib")
        #set(SDL2_TTF_OUTPUT_DLL "${SDL2_TTF_FIND_PATH}/lib/x64/SDL2_ttf.dll;${SDL2_TTF_FIND_PATH}/lib/x64/libfreetype-6.dll")
    #else()
        #set(SDL2_TTF_LIBRARY_PATH "${SDL2_TTF_FIND_PATH}/lib/x86/SDL2_ttf.lib")
        #set(SDL2_TTF_OUTPUT_DLL "${SDL2_TTF_FIND_PATH}/lib/x86/SDL2_ttf.dll;${SDL2_TTF_FIND_PATH}/lib/x86/libfreetype-6.dll")
    #endif()
else()
    message(FATAL_ERROR "Unsupported operating system.")
endif()

include_directories(${SDL2_INCLUDE_PATH})
include_directories(${SDL2_IMAGE_INCLUDE_PATH})
#include_directories(${SDL2_TTF_INCLUDE_PATH})
add_executable(${PROJECT_NAME} main.cpp entities.hpp entities.cpp scene.hpp scene.cpp utility.hpp utility.cpp)
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY_PATH})
target_link_libraries(${PROJECT_NAME} ${SDL2_IMAGE_LIBRARY_PATH})
#target_link_libraries(${PROJECT_NAME} ${SDL2_TTF_LIBRARY_PATH})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/Assets
)
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/Assets"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/Assets
)

if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_STARTUP_PROJECT ${CMAKE_CURRENT_BINARY_DIR})
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:Debug>,Debug,Release>)
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SDL2_OUTPUT_DLL}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SDL2_IMAGE_OUTPUT_DLL}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
    #add_custom_command(
        #TARGET ${PROJECT_NAME} POST_BUILD
        #COMMAND ${CMAKE_COMMAND} -E copy_if_different
        #${SDL2_TTF_OUTPUT_DLL}
        #$<TARGET_FILE_DIR:${PROJECT_NAME}>
    #)
endif()