cmake_minimum_required(VERSION 3.8)

function(custom_find_package_required PACKAGE_NAME)
    message("Finding package: ${PACKAGE_NAME}")

    if(NOT MSVC)
        if(PACKAGE_NAME STREQUAL "SDL2")
            find_package(SDL2 REQUIRED)
            set(SDL2_LIB_INCLUDE_DIRS "${SDL2_INCLUDE_DIRS}" PARENT_SCOPE)
            set(SDL2_LIB_LIBRARIES "${SDL2_LIBRARIES}" PARENT_SCOPE)
        elseif(PACKAGE_NAME STREQUAL "SDL2_image")
            find_package(SDL2_image REQUIRED)
            set(SDL2_IMAGE_LIB_INCLUDE_DIRS "${SDL2_IMAGE_INCLUDE_DIRS}" PARENT_SCOPE)
            set(SDL2_IMAGE_LIB_LIBRARIES "${SDL2_IMAGE_LIBRARIES}" PARENT_SCOPE)
        endif()
        return()
    endif()

    if(PACKAGE_NAME STREQUAL "SDL2")

        find_path(LIB_PATH NAMES "include/SDL.h" NO_CACHE)
        if(LIB_PATH STREQUAL "")
            message(FATAL_ERROR "Couldn't find $(PACKAGE_NAME) library.")
        endif()

        set(SDL2_LIB_INCLUDE_DIRS "${LIB_PATH}/include" PARENT_SCOPE)
        if(${CMAKE_SIZEOF_VOID_P} MATCHES 8)
            set(SDL2_LIB_LIBRARIES "${LIB_PATH}/lib/x64/SDL2.lib" PARENT_SCOPE)
            set(SDL2_LIB_RESULT_DLL "${LIB_PATH}/lib/x64/SDL2.dll" PARENT_SCOPE)
        else()
            set(SDL2_LIB_LIBRARIES "${LIB_PATH}/lib/x86/SDL2.lib" PARENT_SCOPE)
            set(SDL2_LIB_RESULT_DLL "${LIB_PATH}/lib/x86/SDL2.dll" PARENT_SCOPE)
        endif()

    elseif(PACKAGE_NAME STREQUAL "SDL2_image")

        find_path(LIB_PATH NAMES "include/SDL_image.h" NO_CACHE)
        if(LIB_PATH STREQUAL "")
            message(FATAL_ERROR "Couldn't find $(PACKAGE_NAME) library.")
        endif()

        set(SDL2_IMAGE_LIB_INCLUDE_DIRS "${LIB_PATH}/include" PARENT_SCOPE)
        if(${CMAKE_SIZEOF_VOID_P} MATCHES 8)
            set(SDL2_IMAGE_LIB_LIBRARIES "${LIB_PATH}/lib/x64/SDL2_image.lib" PARENT_SCOPE)
            set(SDL2_IMAGE_LIB_RESULT_DLL "${LIB_PATH}/lib/x64/SDL2_image.dll;${LIB_PATH}/lib/x64/zlib1.dll;${LIB_PATH}/lib/x64/libpng16-16.dll" PARENT_SCOPE)
        else()
            set(SDL2_IMAGE_LIB_LIBRARIES "${LIB_PATH}/lib/x86/SDL2_image.lib" PARENT_SCOPE)
            set(SDL2_IMAGE_LIB_RESULT_DLL "${LIB_PATH}/lib/x86/SDL2_image.dll;${LIB_PATH}/lib/x86/zlib1.dll;${LIB_PATH}/lib/x86/libpng16-16.dll" PARENT_SCOPE)
        endif()

    endif()
endfunction()

project(Asteroids45 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED False)

custom_find_package_required("SDL2")
message("Found SDL2 in ${SDL2_LIB_INCLUDE_DIRS}")

custom_find_package_required("SDL2_image")
message("Found SDL2_image in ${SDL2_IMAGE_LIB_INCLUDE_DIRS}")

include_directories(${SDL2_LIB_INCLUDE_DIRS})
include_directories(${SDL2_IMAGE_LIB_INCLUDE_DIRS})
add_executable(Asteroids45 main.cpp)
target_link_libraries(Asteroids45 ${SDL2_LIB_LIBRARIES})
target_link_libraries(Asteroids45 ${SDL2_IMAGE_LIB_LIBRARIES})

if(MSVC)
    set_target_properties(Asteroids45 PROPERTIES VS_STARTUP_PROJECT ${CMAKE_CURRENT_BINARY_DIR})
    set_target_properties(Asteroids45 PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:Debug>,Debug,Release>)
endif()

set_target_properties(Asteroids45 PROPERTIES LINKER_LANGUAGE C)
if(MSVC)
    add_custom_command(
        TARGET Asteroids45 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SDL2_LIB_RESULT_DLL}
        $<TARGET_FILE_DIR:Asteroids45>
    )
    add_custom_command(
        TARGET Asteroids45 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SDL2_IMAGE_LIB_RESULT_DLL}
        $<TARGET_FILE_DIR:Asteroids45>
    )
endif()