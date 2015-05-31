add_subdirectory(game)
add_subdirectory(common)

FILE(GLOB modules_SRCS
	client/shaders/*.cpp
	client/sprites/*.cpp
	client/particles/*.cpp
	client/textures/*.cpp
	client/entities/*.cpp
	client/sound/sdl/*.cpp
	client/sound/*.cpp
	client/*.cpp
	client/ui/*.cpp
	client/ui/nodes/*.cpp
	client/ui/windows/*.cpp
	client/ui/windows/intro/*.cpp
	client/ui/layouts/*.cpp
	server/box2d/*.cpp
	server/*.cpp)

add_library(modules ${modules_SRCS} ${modules_PLATFORM_SRCS})
set_target_properties(modules PROPERTIES FOLDER modules)
target_link_libraries(modules libs game common GL)
