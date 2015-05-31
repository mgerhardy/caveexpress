set(server_SRCS Main.cpp SDLBackend.cpp box2d/DebugRenderer.cpp)
add_library(server ${server_SRCS})
set_target_properties(server PROPERTIES FOLDER server)
target_link_libraries(server libs client)
