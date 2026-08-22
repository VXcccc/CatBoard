cmake_minimum_required(VERSION 4.4.2)

project(CatBoard LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.8-docking
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

add_executable(${PROJECT_NAME}  WIN32
    src/main.cpp
    src/SystemTray.cpp
    src/SettingsWindow.cpp
    src/Paths.cpp
    src/Config.cpp
    src/ConfigJson.cpp
    src/Helpers.cpp
    src/InputHandle.cpp
    src/Sound.cpp
    src/Resource/app.o
    src/Resource/app.rc
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# find_package(imgui CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
  nlohmann_json::nlohmann_json
  d3d11 dxgi d3dcompiler dwmapi
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/src/Layouts"
    "$<TARGET_FILE_DIR:CatBoard>/Layouts"

    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/src/Sounds"
    "$<TARGET_FILE_DIR:CatBoard>/Sounds"
)


if(MINGW)
  target_link_options(${PROJECT_NAME} PRIVATE
        -static-libgcc
        -static-libstdc++
    )
endif()
