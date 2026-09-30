cmake_minimum_required(VERSION 3.25)

project(CatBoard LANGUAGES C CXX)

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

FetchContent_Declare(
    miniaudio
    GIT_REPOSITORY https://github.com/mackron/miniaudio.git
    GIT_TAG master
)
FetchContent_MakeAvailable(miniaudio)

find_package(Vorbis CONFIG REQUIRED)

add_executable(${PROJECT_NAME} WIN32
    src/main.cpp

    src/System/Tray.cpp
    src/System/Paths.cpp
    src/System/Helpers.cpp

    src/Config/Config.cpp
    src/Config/ConfigJson.cpp

    src/Input/Hotkeys.cpp
    src/Input/Mouse.cpp
    src/Input/Keyboard.cpp

    src/Media/Sound.cpp
    src/Media/Fonts.cpp
    src/Media/Images.cpp

    src/UI/SettingsWindow.cpp
    src/UI/MainWindow.cpp
    src/UI/Layouts.cpp
    src/UI/Render.cpp

    src/Assets/Resource/app.rc

    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp

    ${miniaudio_SOURCE_DIR}/miniaudio.c
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  UNICODE
  _UNICODE
)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    src/Assets/Resource
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${miniaudio_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    nlohmann_json::nlohmann_json
    d3d11 dxgi d3dcompiler dwmapi dcomp
    Vorbis::vorbisfile
    miniaudio_libvorbis
    winmm
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/src/Assets/Layouts"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Layouts"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/src/Assets/Sounds"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Sounds"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_SOURCE_DIR}/src/Assets/Fonts"
      "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Fonts"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_SOURCE_DIR}/src/Assets/Images"
      "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Images"
)

if(MINGW)
  target_link_options(${PROJECT_NAME} PRIVATE
        -static-libgcc
        -static-libstdc++
    )
endif()
