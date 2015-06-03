set(main_SRCS Main.cpp)
add_library(main ${main_SRCS})
set_target_properties(main PROPERTIES FOLDER main)
target_link_libraries(main server game common)
