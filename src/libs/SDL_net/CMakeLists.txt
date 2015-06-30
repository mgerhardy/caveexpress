package_global(SDL2_net)

if (NOT SDL2_NET_FOUND)
set(SRCS
	SDLnet.c
	SDLnetselect.c
	SDLnetTCP.c
	SDLnetUDP.c
	)
add_library(SDL2_net ${SRCS})
set_target_properties(SDL2_net PROPERTIES FOLDER SDL2_net)
target_link_libraries(SDL2_net ${SDL2_LIBRARY})
include_directories(${SDL2_INCLUDE_DIR})

set(SDL2_NET_LIBRARIES "SDL2_net")
set(SDL2_NET_INCLUDE_DIR "${ROOT_DIR}/src/libs/SDL_net")
var_global(SDL2_NET_LIBRARIES)
var_global(SDL2_NET_INCLUDE_DIR)

endif()
