set(server_SRCS SDLBackend.cpp box2d/DebugRenderer.cpp)
add_library(server ${server_SRCS})
set_target_properties(server PROPERTIES FOLDER server)
# TODO: remove box2d here
cp_target_link_libraries(TARGET server LIBS common game network client ${BOX2D_LIBRARIES} service)
