# Dear ImGui from https://github.com/ocornut/imgui (docking branch)
set(SOURCES
    misc/cpp/imgui_stdlib.cpp

    backends/imgui_impl_opengl3.cpp
    backends/imgui_impl_sdl2.cpp
    backends/imgui_impl_sdlrenderer2.cpp

    imgui_widgets.cpp
	imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui.cpp
)

add_library(imgui ${SOURCES})
target_include_directories(imgui PUBLIC . backends misc/cpp)
if (NOT EMSCRIPTEN)
	target_link_libraries(imgui PUBLIC SDL2::SDL2)
endif()
if (ANDROID)
	target_link_libraries(imgui PUBLIC GLESv3)
endif()
