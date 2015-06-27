package_global(lua)

if (NOT LUA_FOUND)
set(SRCS
	lapi.cpp
	lauxlib.cpp
	lbaselib.cpp
	lbitlib.cpp
	lcode.cpp
	lcorolib.cpp
	lctype.cpp
	ldblib.cpp
	ldebug.cpp
	ldo.cpp
	ldump.cpp
	lfunc.cpp
	lgc.cpp
	linit.cpp
	liolib.cpp
	llex.cpp
	lmathlib.cpp
	lmem.cpp
	loadlib.cpp
	lobject.cpp
	lopcodes.cpp
	loslib.cpp
	lparser.cpp
	lstate.cpp
	lstring.cpp
	lstrlib.cpp
	ltable.cpp
	ltablib.cpp
	ltm.cpp
	lundump.cpp
	lvm.cpp
	lzio.cpp
	)

add_library(lua ${SRCS})
target_link_libraries(lua ${CMAKE_DL_LIBS})
set_target_properties(lua PROPERTIES FOLDER lua)
add_definitions(-DLUA_USE_LONGJMP -DLUA_COMPAT_MODULE)
if (LINUX)
	add_definitions(-DLUA_USE_LINUX)
elseif(DARWIN)
	add_definitions(-DLUA_USE_MACOSX)
endif()
set_target_properties(lua PROPERTIES COMPILE_FLAGS "-Wno-cast-qual")

set(LUA_LIBRARIES "lua")
set(LUA_INCLUDE_DIR "${ROOT_DIR}/src/libs/lua")
var_global(LUA_LIBRARIES)
var_global(LUA_INCLUDE_DIR)

endif()
