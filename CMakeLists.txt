cmake_minimum_required(VERSION 3.25)

project(CatBoard LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(MINIAUDIO_DIR ${CMAKE_SOURCE_DIR}/ThirdParty/miniaudio)
set(MINIAUDIO_VORBIS_DIR ${MINIAUDIO_DIR}/extras/decoders/libvorbis)
set(imgui_SOURCE_DIR ${CMAKE_SOURCE_DIR}/ThirdParty/imgui)

find_package(Vorbis CONFIG REQUIRED)

add_executable(${PROJECT_NAME} WIN32
    src/main.cpp

    src/System/Tray.cpp
    src/System/Paths.cpp
    src/System/Helpers.cpp

    src/Configs/Config.cpp
    src/Configs/JsonParser.cpp

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

    Resource/app.rc

    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp

    ${MINIAUDIO_DIR}/miniaudio.c
    ${MINIAUDIO_VORBIS_DIR}/miniaudio_libvorbis.c
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  UNICODE
  _UNICODE
)


target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/ThirdParty
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Configs
    ${CMAKE_SOURCE_DIR}/src/Input
    ${CMAKE_SOURCE_DIR}/src/Media
    ${CMAKE_SOURCE_DIR}/src/System
    ${CMAKE_SOURCE_DIR}/src/UI
    ${CMAKE_SOURCE_DIR}/Assets

    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${miniaudio_SOURCE_DIR}

    ${MINIAUDIO_DIR}
    ${MINIAUDIO_VORBIS_DIR}
)


target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d11 dxgi d3dcompiler dwmapi dcomp
    Vorbis::vorbisfile
    winmm
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/Assets/Layouts"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Layouts"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Assets/Sounds"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Sounds"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_SOURCE_DIR}/Assets/Fonts"
      "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Fonts"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_SOURCE_DIR}/Assets/Images"
      "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Images"
)

if(MINGW)
  target_link_options(${PROJECT_NAME} PRIVATE
        -static-libgcc
        -static-libstdc++
    )
endif()
