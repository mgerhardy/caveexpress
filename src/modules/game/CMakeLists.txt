set(game_SRCS GameRegistry.cpp IGame.cpp Main.cpp)
add_library(game ${game_SRCS})
set_target_properties(game PROPERTIES FOLDER game)
