set(sound_SRCS Sound.cpp SDLSoundEngine.cpp)
add_library(sound ${sound_SRCS})
set_target_properties(sound PROPERTIES FOLDER sound)
cp_target_link_libraries(TARGET sound LIBS SDL2_mixer::SDL2_mixer common)
