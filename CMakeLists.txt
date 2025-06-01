cmake_minimum_required(VERSION 3.16)

option(BUILD_SHARED_LIBS "FALSE = build static, TRUE = build shared" FALSE)
option(AUI_LUA_BUILD_EXAMPLES "Whether or not to build examples" FALSE)

project(aui.bindings.lua)

set(CMAKE_CXX_STANDARD 20)

set(AUI_VERSION v8.0.0-rc.2)
file(
        DOWNLOAD
        https://raw.githubusercontent.com/aui-framework/aui/${AUI_VERSION}/aui.boot.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
include(${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
auib_import(aui https://github.com/aui-framework/aui
            COMPONENTS core views uitests
            VERSION ${AUI_VERSION})

auib_import(clg https://github.com/Alex2772/cpp_lua_glue
            VERSION "08f09ea35b3694ffee3196c8a085f4ca06731eb4"
            ADD_SUBDIRECTORY)

target_compile_definitions(clg INTERFACE CLG_MANUAL_CLEANUP=1)

if (MSVC)
    add_definitions("/MP")
endif()


file(GLOB_RECURSE UIENGINE_SOURCE_FILES src/*.cpp)

aui_module(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC include)

find_package(Python REQUIRED COMPONENTS Interpreter)

function(lua_reflection _input)
    get_filename_component(_filename ${_input} NAME)

    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lua_reflection)

    set(_output ${CMAKE_CURRENT_BINARY_DIR}/lua_reflection/${_filename})

    add_custom_command(OUTPUT ${_output}
            COMMAND Python::Interpreter ARGS ${CMAKE_CURRENT_SOURCE_DIR}/lua_reflection.py ${_input} ${_output})

    target_sources(${PROJECT_NAME} PRIVATE ${_output})

endfunction()

file(GLOB_RECURSE items lua_reflection/*.h)
foreach(_item ${items})
    lua_reflection("${_item}")
endforeach()

aui_link(${PROJECT_NAME} PUBLIC aui::views clg)

if (NOT ANDROID AND NOT IOS)
    aui_link(${PROJECT_NAME} PUBLIC aui::uitests)
    aui_enable_tests(${PROJECT_NAME})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/lua_reflection)

if (AUI_LUA_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

if (TARGET aui.spine)
    target_compile_definitions(${PROJECT_NAME} PRIVATE AUI_BINDINGS_LUA_SPINE=1)
else ()
    target_compile_definitions(${PROJECT_NAME} PRIVATE AUI_BINDINGS_LUA_SPINE=0)
endif ()