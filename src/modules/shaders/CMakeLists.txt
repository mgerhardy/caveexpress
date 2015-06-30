set(shader_SRCS Shader.cpp)
add_library(shader ${shader_SRCS})
set_target_properties(shader PROPERTIES FOLDER shader)
cp_target_link_libraries(TARGET shader LIBS common texture gfx)
