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
	lutf8lib.cpp
	lvm.cpp
	lzio.cpp
	)

cp_add_library(
	LIB lua5.2
	SRCS ${SRCS}
	CFLAGS "-Wno-cast-qual -Wno-parentheses-equality"
	DEFINES "-DLUA_USE_LONGJMP -DLUA_COMPAT_MODULE")
if (LINUX)
	add_definitions(-DLUA_USE_LINUX)
elseif(DARWIN)
	add_definitions(-DLUA_USE_MACOSX)
endif()
add_definitions(-DLUA_USE_LONGJMP -DLUA_COMPAT_MODULE)
if (ANDROID)
add_definitions(-D\"lua_getlocaledecpoint\(\)='.'\")
add_definitions(-DLUA_ANSI -DLUA_USE_C89)
endif()
