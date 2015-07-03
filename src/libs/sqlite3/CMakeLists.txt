cp_add_library(
	LIB sqlite3
	SRCS sqlite3.c
	CFLAGS "-Wno-cast-qual"
	DEFINES "-DSQLITE_HAVE_ISNAN -DSQLITE_THREADSAFE=0")
