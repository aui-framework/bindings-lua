project(aui.bindings.lua.example)

# note: to use in an external project, import aui.bindings.lua.example as follows:
# set(AUI_VERSION master)
# file(
#         DOWNLOAD
#         https://raw.githubusercontent.com/aui-framework/aui/${AUI_VERSION}/aui.boot.cmake
#         ${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
# include(${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
# auib_import(aui.bindings.lua https://github.com/aui-framework/bindings-lua
#             ADD_SUBDIRECTORY
# )

aui_executable(${PROJECT_NAME})
aui_link(${PROJECT_NAME} PRIVATE aui.bindings.lua)
aui_compile_assets(${PROJECT_NAME})
