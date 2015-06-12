set(sound_SRCS Sound.cpp SDLSoundEngine.cpp)
add_library(sound ${sound_SRCS})
set_target_properties(sound PROPERTIES FOLDER sound)
target_link_libraries(sound common SDL SDL_mixer)
