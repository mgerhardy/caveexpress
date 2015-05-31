set(texture_SRCS Texture.cpp TextureCache.cpp)
add_library(texture ${texture_SRCS})
set_target_properties(texture PROPERTIES FOLDER texture)
target_link_libraries(texture libs)
