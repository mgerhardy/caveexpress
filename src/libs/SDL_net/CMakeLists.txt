set(SRCS
	SDLnet.c
	SDLnetselect.c
	SDLnetTCP.c
	SDLnetUDP.c
	)
add_library(SDL_net ${SRCS})
set_target_properties(SDL_net PROPERTIES FOLDER SDL_net)
target_link_libraries(SDL_net SDL)