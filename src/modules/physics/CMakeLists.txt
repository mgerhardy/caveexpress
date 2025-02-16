set(physics_SRCS DebugRenderer.cpp)
add_library(physics ${physics_SRCS})
set_target_properties(physics PROPERTIES FOLDER physics)
cp_target_link_libraries(TARGET physics LIBS common box2d SDL2::SDL2)
