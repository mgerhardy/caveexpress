set(game_SRCS GameRegistry.cpp IGame.cpp)
add_library(game ${game_SRCS})
set_target_properties(game PROPERTIES FOLDER game)
cp_target_link_libraries(TARGET game LIBS common)
