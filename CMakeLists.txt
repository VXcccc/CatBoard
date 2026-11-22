cmake_minimum_required(VERSION 3.25)

project(CatBoard VERSION 0.1.0 LANGUAGES C CXX)

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
				src/Input/AnalogKeyboard.cpp

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
				${imgui_SOURCE_DIR}/imsearch.cpp
				${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
				${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
				${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp

				${MINIAUDIO_DIR}/miniaudio.c
				${MINIAUDIO_VORBIS_DIR}/miniaudio_libvorbis.c
)


target_compile_definitions(${PROJECT_NAME} PRIVATE
		UNICODE
		_UNICODE
		APP_VERSION_STR="${PROJECT_VERSION}"
		APP_VERSION_INT=${PROJECT_VERSION_MAJOR}${PROJECT_VERSION_MINOR}${PROJECT_VERSION_PATCH}
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

				${MINIAUDIO_DIR}
				${MINIAUDIO_VORBIS_DIR}
)

set(WOOTING_DIR  ${CMAKE_SOURCE_DIR}/ThirdParty/wooting-sdk)

add_library(WootingAnalog SHARED IMPORTED)
set_target_properties(WootingAnalog PROPERTIES
				IMPORTED_LOCATION "${WOOTING_DIR}/release/wooting_analog_sdk_dist.dll"
				IMPORTED_IMPLIB   "${WOOTING_DIR}/release/wooting_analog_sdk_dist.dll"
				INTERFACE_INCLUDE_DIRECTORIES "${WOOTING_DIR}/includes"
)
target_link_libraries(${PROJECT_NAME} PRIVATE
				d3d11 dxgi d3dcompiler dwmapi dcomp
				Vorbis::vorbisfile
				winmm
				WootingAnalog
)

set(ASSETS_DEST Assets)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_directory
				"${CMAKE_SOURCE_DIR}/Assets/Layouts"
				"$<TARGET_FILE_DIR:${PROJECT_NAME}>/${ASSETS_DEST}/Layouts"
				COMMAND ${CMAKE_COMMAND} -E copy_directory
								"${CMAKE_SOURCE_DIR}/Assets/Sounds"
								"$<TARGET_FILE_DIR:${PROJECT_NAME}>/${ASSETS_DEST}/Sounds"
				COMMAND ${CMAKE_COMMAND} -E copy_directory
						"${CMAKE_SOURCE_DIR}/Assets/Images"
						"$<TARGET_FILE_DIR:${PROJECT_NAME}>/${ASSETS_DEST}/Images"
				COMMAND ${CMAKE_COMMAND} -E copy_directory
				"${CMAKE_SOURCE_DIR}/Assets/Configs"
				"$<TARGET_FILE_DIR:${PROJECT_NAME}>/${ASSETS_DEST}/Configs"

				COMMAND ${CMAKE_COMMAND} -E copy_if_different
				"${WOOTING_DIR}/release/wooting_analog_sdk_dist.dll"
				"$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)

if(MINGW)
	target_link_options(${PROJECT_NAME} PRIVATE
								-static-libgcc
								-static-libstdc++
				)
endif()


# CPACK
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
install(FILES ${WOOTING_DIR}/release/wooting_analog_sdk_dist.dll DESTINATION .)
install(DIRECTORY
				${CMAKE_SOURCE_DIR}/Assets/Layouts
				${CMAKE_SOURCE_DIR}/Assets/Sounds
				${CMAKE_SOURCE_DIR}/Assets/Images
				${CMAKE_SOURCE_DIR}/Assets/Configs
				DESTINATION ${ASSETS_DEST}
)

set(CPACK_PACKAGE_NAME "CatBoard")
set(CPACK_PACKAGE_VENDOR "VXcccc")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME "CatBoard-${PROJECT_VERSION}-windows-x64")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "CatBoard")
set(CPACK_STRIP_FILES ON)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

# NSIS SETUP
set(CPACK_NSIS_INSTALL_ROOT "$USERLOCALAPPDATA\\\\Programs")
set(CPACK_NSIS_DISPLAY_NAME "CatBoard")
set(CPACK_NSIS_PACKAGE_NAME "CatBoard")
set(CPACK_NSIS_INSTALLED_ICON_NAME "CatBoard.exe")
set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
set(CPACK_PACKAGE_EXECUTABLES "CatBoard" "CatBoard")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_MODIFY_PATH OFF)
set(CPACK_NSIS_MANIFEST_DPI_AWARE ON)

file(TO_NATIVE_PATH "${CMAKE_SOURCE_DIR}/Resource/icon.ico" _ico)
string(REPLACE "\\" "\\\\" _ico "${_ico}")
set(CPACK_NSIS_MUI_ICON "${_ico}")
set(CPACK_NSIS_MUI_UNIICON "${_ico}")

set(CPACK_NSIS_MUI_FINISHPAGE_RUN "CatBoard.exe")

set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
		MessageBox MB_YESNO|MB_ICONQUESTION 'Create a desktop shortcut?' /SD IDNO IDNO +2
		CreateShortCut '$DESKTOP\\\\CatBoard.lnk' '$INSTDIR\\\\CatBoard.exe'
")
set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
		Delete '$DESKTOP\\\\CatBoard.lnk'
		Delete '$INSTDIR\\\\ActiveConfig.json'
		Delete '$INSTDIR\\\\UI.json'
")

include(CPack)


