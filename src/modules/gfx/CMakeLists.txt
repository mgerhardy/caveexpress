set(gfx_SRCS
	SDLFrontend.cpp
	FrameBuffer.cpp
)
if (NOT ANDROID)
list(APPEND gfx_SRCS
	GL3Frontend.cpp
	GL1Frontend.cpp
	GLFunc.cpp
	BGFXFrontend.cpp
)
endif()
add_library(gfx ${gfx_SRCS})
set_target_properties(gfx PROPERTIES FOLDER client)
set(DEPS common texture ui client shader game sound)
if (ANDROID)
list(APPEND DEPS GLESv1_CM GLESv2)
else()
package_global(OpenGL)
list(APPEND DEPS ${OPENGL_LIBRARIES})
endif()
target_link_libraries(gfx ${DEPS})
