set(gfx_SRCS
	SDLFrontend.cpp
	FrameBuffer.cpp
	GL3Frontend.cpp
	GL1Frontend.cpp
	GLFunc.cpp
	BGFXFrontend.cpp
)
add_library(gfx ${gfx_SRCS})
set_target_properties(gfx PROPERTIES FOLDER client)
target_link_libraries(gfx libs common texture ui client shader game sound GL)
