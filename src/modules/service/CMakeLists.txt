set(service_SRCS ServiceProvider.cpp)
add_library(service ${service_SRCS})
set_target_properties(service PROPERTIES FOLDER service)
cp_target_link_libraries(TARGET service LIBS network common game)
