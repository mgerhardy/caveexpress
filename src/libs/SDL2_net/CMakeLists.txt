set(SRCS
	SDLnet.c
	SDLnetselect.c
	SDLnetTCP.c
	SDLnetUDP.c
	)
cp_add_library(
	LIB SDL2_net
	SRCS ${SRCS}
	CFLAGS "-Wno-pointer-sign")
if(NOT SDL2_NET_FOUND)
	target_link_libraries(SDL2_net PUBLIC sdl2)
endif()
