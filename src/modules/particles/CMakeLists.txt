set(particle_SRCS Bubble.cpp Particle.cpp ParticleSystem.cpp Snow.cpp Sparkle.cpp)
add_library(particle ${particle_SRCS})
set_target_properties(particle PROPERTIES FOLDER particle)
cp_target_link_libraries(TARGET particle LIBS common texture)
