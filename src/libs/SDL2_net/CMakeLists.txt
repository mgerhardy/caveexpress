set(SRCS
	SDLnet.c
	SDLnetselect.c
	SDLnetTCP.c
	SDLnetUDP.c
	)
cp_add_library(LIB SDL2_net SRCS ${SRCS})
