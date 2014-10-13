project(engine)
FILE(GLOB ${PROJECT_NAME}_SRCS
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
	common/*.cpp
	common/campaign/*.cpp
	common/campaign/persister/*.cpp
	common/network/*.cpp
	server/box2d/*.cpp
	server/*.cpp)

if(WIN32)
FILE(GLOB ${PROJECT_NAME}_PLATFORM_SRCS
	common/ports/Windows.cpp
	common/ports/project.rc)
elseif(UNIX)
FILE(GLOB ${PROJECT_NAME}_PLATFORM_SRCS 
	engine/common/ports/Unix.cpp)
endif()

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_SRCS} ${${PROJECT_NAME}_PLATFORM_SRCS})
