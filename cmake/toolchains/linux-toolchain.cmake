include(CheckFunctionExists)
include(CheckLibraryExists)

if (CMAKE_COMPILER_IS_GNUCXX)
    check_function_exists(__atomic_fetch_add_4 HAVE___ATOMIC_FETCH_ADD_4)
    if (NOT HAVE___ATOMIC_FETCH_ADD_4)
        check_library_exists(atomic __atomic_fetch_add_4 "" HAVE_LIBATOMIC)
        if (HAVE_LIBATOMIC)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -latomic")
        endif()
    endif()
endif()

set(CMAKE_C_STANDARD_LIBRARIES ${CMAKE_DL_LIBS})

set(CMAKE_C_FLAGS "-pthread -Wcast-qual -Wcast-align -Wpointer-arith -Wshadow -Wall -Wextra -Wreturn-type -Wwrite-strings -Wno-unused-parameter")
set(CMAKE_C_FLAGS_RELEASE "-O3 -ftree-vectorize -msse3 -ffast-math -DNDEBUG -D_FORTIFY_SOURCE=2")
set(CMAKE_C_FLAGS_DEBUG "-O0 -DDEBUG=1 -ggdb")

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -Wnon-virtual-dtor -fno-rtti -fno-exceptions")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
