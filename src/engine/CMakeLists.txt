FILE(GLOB engine_SRCS
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
	*.cpp
	../*.cpp
	common/*.cpp
	common/campaign/*.cpp
	common/campaign/persister/*.cpp
	common/network/*.cpp
	server/box2d/*.cpp
	server/*.cpp)

if(WIN32)
FILE(GLOB engine_PLATFORM_SRCS
	common/ports/Windows.cpp
	common/ports/project.rc)
elseif(APPLE)
FILE(GLOB engine_PLATFORM_SRCS
	common/ports/Darwin.cpp
	common/ports/Cocoa.mm)
elseif(UNIX)
FILE(GLOB engine_PLATFORM_SRCS
	common/ports/Unix.cpp)
	add_definitions(-D_GNU_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE)
endif()

add_library(engine ${engine_SRCS} ${engine_PLATFORM_SRCS})
set_target_properties(engine PROPERTIES FOLDER engine)
target_link_libraries(engine libs GL)
