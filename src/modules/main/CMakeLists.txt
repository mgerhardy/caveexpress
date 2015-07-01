set(main_SRCS Main.cpp)
add_library(main ${main_SRCS})
set_target_properties(main PROPERTIES FOLDER main)
cp_target_link_libraries(TARGET main LIBS server game common service)
