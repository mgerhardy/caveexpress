set(server_SRCS SDLBackend.cpp box2d/DebugRenderer.cpp)
add_library(server ${server_SRCS})
set_target_properties(server PROPERTIES FOLDER server)
cp_target_link_libraries(TARGET server LIBS common game network gfx client box2d service)
