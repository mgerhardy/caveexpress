set(SRCS
	SDLnet.c
	SDLnetselect.c
	SDLnetTCP.c
	SDLnetUDP.c
	)
add_library(SDL2_net ${SRCS})
set_target_properties(SDL2_net PROPERTIES FOLDER SDL2_net)
target_link_libraries(SDL2_net ${SDL2_LIBRARY})
include_directories("${SDL2_INCLUDE_DIR}")
