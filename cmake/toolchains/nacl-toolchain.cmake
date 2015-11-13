set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_CROSSCOMPILING TRUE)

set(CMAKE_C_FLAGS "-Wcast-qual -Wcast-align -Wpointer-arith -Wno-long-long -Wno-multichar -Wshadow -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Wreturn-type -Wwrite-strings -Wno-variadic-macros -Wno-unknown-pragmas")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "-O0 -DDEBUG -g")

set(CMAKE_CXX_STANDARD_LIBRARIES "-lppapi_gles2 -lppapi_cpp -lppapi -lpthread")
set(CMAKE_C_STANDARD_LIBRARIES "lppapi_gles2 -lppapi -lpthread")

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wnon-virtual-dtor")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")

set(CMAKE_EXE_LINKER_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")
