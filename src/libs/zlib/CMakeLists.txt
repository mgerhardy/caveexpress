package_global(ZLIB)

if (NOT ZLIB_FOUND)
set(SRCS
	adler32.c
	compress.c
	crc32.c
	deflate.c
	gzclose.c
	gzlib.c
	gzread.c
	gzwrite.c
	infback.c
	inffast.c
	inflate.c
	inftrees.c
	trees.c
	uncompr.c
	zutil.c)
add_library(zlib ${SRCS})
set_target_properties(zlib PROPERTIES FOLDER zlib)
set_target_properties(zlib PROPERTIES COMPILE_FLAGS "-Wno-cast-qual -Wno-implicit-function-declaration")

set(ZLIB_LIBRARIES "zlib")
set(ZLIB_INCLUDE_DIRS "${ROOT_DIR}/src/libs/zlib")
var_global(ZLIB_LIBRARIES)
var_global(ZLIB_INCLUDE_DIRS)

endif()
