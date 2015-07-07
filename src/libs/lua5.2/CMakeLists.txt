set(SRCS
	lapi.c
	lauxlib.c
	lbaselib.c
	lbitlib.c
	lcode.c
	lcorolib.c
	lctype.c
	ldblib.c
	ldebug.c
	ldo.c
	ldump.c
	lfunc.c
	lgc.c
	linit.c
	liolib.c
	llex.c
	lmathlib.c
	lmem.c
	loadlib.c
	lobject.c
	lopcodes.c
	loslib.c
	lparser.c
	lstate.c
	lstring.c
	lstrlib.c
	ltable.c
	ltablib.c
	ltm.c
	lundump.c
	lutf8lib.c
	lvm.c
	lzio.c
	)

cp_add_library(
	LIB lua5.2
	SRCS ${SRCS}
	CFLAGS "-Wno-cast-qual -Wno-parentheses-equality"
	DEFINES "-DLUA_USE_LONGJMP -DLUA_COMPAT_MODULE")
if (LINUX AND NOT ANDROID)
	add_definitions(-DLUA_USE_LINUX)
elseif(DARWIN)
	add_definitions(-DLUA_USE_MACOSX)
endif()
add_definitions(-DLUA_USE_LONGJMP -DLUA_COMPAT_MODULE)
if (ANDROID)
	add_definitions(-D\"lua_getlocaledecpoint\(\)='.'\")
	add_definitions(-DLUA_ANSI -DLUA_USE_C89)
endif()
