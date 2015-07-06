set(SRCS
	SDLnet.c
	SDLnetselect.c
	SDLnetTCP.c
	SDLnetUDP.c
	)
cp_add_library(
	LIB SDL2_net
	SRCS ${SRCS}
	CFLAGS "-Wpointer-sign")
if(NOT SDL2_NET_FOUND)
	include_directories(${SDL2_INCLUDE_DIRS})
	target_link_libraries(SDL2_net ${SDL2_LIBRARIES})
endif()
