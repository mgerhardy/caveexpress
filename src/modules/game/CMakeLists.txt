set(game_SRCS GameRegistry.cpp IGame.cpp)
add_library(game ${game_SRCS})
set_target_properties(game PROPERTIES FOLDER game)
target_link_libraries(game libs)
