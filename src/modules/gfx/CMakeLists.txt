set(gfx_SRCS
	AbstractGLFrontend.cpp
	SDLFrontend.cpp
	GL3Frontend.cpp
	GLFunc.cpp
	FrameBuffer.cpp
	Shader.cpp
)
if (NOT ANDROID)
list(APPEND gfx_SRCS
	GL1Frontend.cpp
	BGFXFrontend.cpp
)
endif()
add_library(gfx ${gfx_SRCS})
set_target_properties(gfx PROPERTIES FOLDER gfx)
set(DEPS common texture ui)
if (ANDROID)
	list(APPEND DEPS GLESv1_CM GLESv2)
elseif(NOT IOS)
	package_global(OpenGL)
	list(APPEND DEPS ${OPENGL_LIBRARIES})
endif()
cp_target_link_libraries(TARGET gfx LIBS ${DEPS})
