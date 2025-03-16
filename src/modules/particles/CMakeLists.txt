set(particle_SRCS
	Particle.cpp
	ParticleSystem.cpp

	Bubble.cpp
	Dirt.cpp
	Leaf.cpp
	Rain.cpp
	Sand.cpp 
	Snow.cpp
	Sparkle.cpp
)
add_library(particle ${particle_SRCS})
set_target_properties(particle PROPERTIES FOLDER particle)
cp_target_link_libraries(TARGET particle LIBS common texture)
