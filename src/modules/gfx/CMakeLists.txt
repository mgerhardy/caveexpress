set(gfx_SRCS
	AbstractGLFrontend.cpp
	SDLFrontend.cpp
	GL3Frontend.cpp
	GLFunc.cpp
	FrameBuffer.cpp
	Shader.cpp
)
add_library(gfx ${gfx_SRCS})
set_target_properties(gfx PROPERTIES FOLDER gfx)
set(DEPS common texture ui)
if (ANDROID)
	list(APPEND DEPS GLESv2 GLESv3)
endif()
cp_target_link_libraries(TARGET gfx LIBS ${DEPS})
