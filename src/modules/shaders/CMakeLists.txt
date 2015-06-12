set(shader_SRCS Shader.cpp)
add_library(shader ${shader_SRCS})
set_target_properties(shader PROPERTIES FOLDER shader)
target_link_libraries(shader common texture gfx)
