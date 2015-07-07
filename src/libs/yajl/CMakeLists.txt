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
cp_add_library(LIB yajl SRCS ${SRCS} CFLAGS "-Wno-cast-qual -Wno-unused-parameter -Wno-implicit-function-declaration -Wno-format")
