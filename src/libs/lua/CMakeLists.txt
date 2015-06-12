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
set_target_properties(lua PROPERTIES FOLDER lua)
add_definitions(-DLUA_USE_LONGJMP -DLUA_COMPAT_MODULE)