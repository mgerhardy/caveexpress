set(texture_SRCS Texture.cpp TextureCache.cpp)
add_library(texture ${texture_SRCS})
set_target_properties(texture PROPERTIES FOLDER texture)
cp_target_link_libraries(TARGET texture LIBS common sdl2 SDL2_image)
