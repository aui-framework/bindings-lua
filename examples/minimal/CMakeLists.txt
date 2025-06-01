project(aui.bindings.lua.example)

# note: to use in an external project, import aui.bindings.lua.example as follows:
# auib_import(aui.bindings.lua https://github.com/aui-framework/bindings-lua
#             ADD_SUBDIRECTORY
# )

aui_executable(${PROJECT_NAME})
aui_link(${PROJECT_NAME} PRIVATE aui.bindings.lua)
aui_compile_assets(${PROJECT_NAME})
