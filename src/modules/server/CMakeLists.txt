set(server_SRCS SDLBackend.cpp box2d/DebugRenderer.cpp)
add_library(server ${server_SRCS})
set_target_properties(server PROPERTIES FOLDER server)
target_link_libraries(server libs common game network gfx client)
