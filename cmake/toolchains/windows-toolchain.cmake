set(CMAKE_SYSTEM_NAME Windows)

if (MSYS)
set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_C_FLAGS "-Wcast-qual -Wcast-align -Wpointer-arith -Wshadow -Wall -Wextra -Wreturn-type -Wwrite-strings -Wno-unused-parameter")
set(CMAKE_C_FLAGS_RELEASE "-O3 -ftree-vectorize -msse3 -ffast-math -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "-O0 -DDEBUG=1 -ggdb")

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fno-rtti -fno-exceptions -std=c++11 -Wnon-virtual-dtor")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")

set(CMAKE_CXX_STANDARD_LIBRARIES "kernel user gdi winspool shell ole oleaut uuid comdlg advapi dbghelp wsock ws2_ rpcrt4 wininet")
set(CMAKE_C_STANDARD_LIBRARIES "kernel user gdi winspool shell ole oleaut uuid comdlg advapi dbghelp wsock ws2_ rpcrt4 wininet")
else()
# define the standard link libraries
set(CMAKE_CXX_STANDARD_LIBRARIES "kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib dbghelp.lib wsock32.lib ws2_32.lib rpcrt4.lib wininet.lib")
set(CMAKE_C_STANDARD_LIBRARIES "kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib dbghelp.lib wsock32.lib ws2_32.lib rpcrt4.lib wininet.lib")
endif()
