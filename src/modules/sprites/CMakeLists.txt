set(sprite_SRCS Sprite.cpp SpriteCache.cpp)
add_library(sprite ${sprite_SRCS})
set_target_properties(sprite PROPERTIES FOLDER sprite)
cp_target_link_libraries(TARGET sprite LIBS common texture ui)
