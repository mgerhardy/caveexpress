set(mainloop_SRCS SDLMainLoop.cpp)
add_library(mainloop ${mainloop_SRCS})
set_target_properties(mainloop PROPERTIES FOLDER mainloop)
cp_target_link_libraries(TARGET mainloop LIBS common game network client service)
