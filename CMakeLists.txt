cmake_minimum_required(VERSION 3.27)
project(bluebell-v2 C)
set(CMAKE_C_STANDARD 99)

# Adding Raylib
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "5.5"
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)
FetchContent_Declare(
    nfd
    GIT_REPOSITORY "https://github.com/btzy/nativefiledialog-extended.git"
    GIT_TAG "v1.2.1"
)
set(NFD_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(NFD_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(raylib)
FetchContent_MakeAvailable(nfd)

add_executable(bluebell-v2 
    main.c
    includes/cJSON.c
)

target_compile_options(bluebell-v2 PUBLIC)
target_include_directories(bluebell-v2 PUBLIC .)

target_link_libraries(bluebell-v2 PUBLIC raylib nfd)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")

add_custom_command(
        TARGET bluebell-v2 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        ${CMAKE_CURRENT_BINARY_DIR}/resources)
