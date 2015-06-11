include(CheckFunctionExists)
include(CheckLibraryExists)

if (NOT NETWORKING)
	add_definitions(-DNONETWORK=1)
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
    check_function_exists(__atomic_fetch_add_4 HAVE___ATOMIC_FETCH_ADD_4)
    if (NOT HAVE___ATOMIC_FETCH_ADD_4)
        check_library_exists(atomic __atomic_fetch_add_4 "" HAVE_LIBATOMIC)
        if (HAVE_LIBATOMIC)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -latomic")
        endif()
    endif()
endif()

set(CMAKE_CXX_FLAGS "-fno-rtti -fno-exceptions -std=c++11 -pthread -Wall -Wextra -Wnon-virtual-dtor -Wno-reorder -pthread -Wcast-qual -Wcast-align -Wpointer-arith -Wno-long-long -Wno-multichar -Wshadow -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Wreturn-type -Wwrite-strings -Wno-variadic-macros -Wno-unknown-pragmas")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ftree-vectorize -msse3 -ffast-math -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -DDEBUG=1 -ggdb")

set(CMAKE_C_FLAGS "-pthread -Wcast-qual -Wcast-align -Wpointer-arith -Wno-long-long -Wno-multichar -Wshadow -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Wreturn-type -Wwrite-strings -Wno-variadic-macros -Wno-unknown-pragmas")
set(CMAKE_C_FLAGS_RELEASE "-O3 -ftree-vectorize -msse3 -ffast-math -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "-O0 -DDEBUG=1 -ggdb")
