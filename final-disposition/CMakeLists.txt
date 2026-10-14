cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# Ensure ninja is being used
if (NOT CMAKE_GENERATOR STREQUAL "Ninja Multi-Config")
    message(FATAL_ERROR "Please use Ninja Multi-Config generator")
endif ()

option(FINAL_DISPOSITION_EDITOR_MODE "Enables editor mode for final disposition" OFF)

project(final-disposition)

file(GLOB_RECURSE FINAL_DISPOSITION_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(final-disposition ${FINAL_DISPOSITION_SOURCES})

target_include_directories(final-disposition PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(final-disposition PRIVATE
        erebus
)

add_custom_target(COPY_ASSETS
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:final-disposition>/resources"
)

add_dependencies(final-disposition COPY_ASSETS)

if(FINAL_DISPOSITION_EDITOR_MODE)
        add_compile_definitions(final-disposition PUBLIC
                FINAL_DISPOSITION_EDITOR_MODE        
        )
endif()
