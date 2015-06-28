package_global(yajl)

if (NOT YAJL_FOUND)
set(SRCS
	yajl_alloc.c
	yajl_buf.c
	yajl.c
	yajl_encode.c
	yajl_gen.c
	yajl_lex.c
	yajl_parser.c
	yajl_tree.c
	)
add_library(yajl ${SRCS})
set_target_properties(yajl PROPERTIES FOLDER yajl)
set_target_properties(yajl PROPERTIES COMPILE_FLAGS "-Wno-cast-qual -Wno-unused-parameter")

set(YAJL_LIBRARY "yajl")
set(YAJL_INCLUDE_DIR "${ROOT_DIR}/src/libs/yajl")
var_global(YAJL_LIBRARY)
var_global(YAJL_INCLUDE_DIR)

endif()
