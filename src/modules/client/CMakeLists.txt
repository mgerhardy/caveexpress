set(client_SRCS
	ClientConsole.cpp
	SDLFrontend.cpp
	Camera.cpp
	ClientMap.cpp
	FrameBuffer.cpp
	shaders/Shader.cpp
	entities/ClientEntity.cpp
	entities/ClientPlayer.cpp
	entities/ClientMapTile.cpp
	GL3Frontend.cpp
	GL1Frontend.cpp
	GLFunc.cpp
	particles/Snow.cpp
	particles/Sparkle.cpp
	particles/Particle.cpp
	particles/Bubble.cpp
	particles/ParticleSystem.cpp
	BGFXFrontend.cpp
)
add_library(client ${client_SRCS})
set_target_properties(client PROPERTIES FOLDER client)
target_link_libraries(client libs game common ui texture sound sprite GL)
