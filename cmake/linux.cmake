include(CheckFunctionExists)
include(CheckLibraryExists)

set(CMAKE_CXX_FLAGS "-std=c++11 -fno-rtti -fno-exceptions")

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
