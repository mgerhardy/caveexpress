cp_add_library(sqlite3 sqlite3.c "-Wno-cast-qual" "-DSQLITE_HAVE_ISNAN -DSQLITE_THREADSAFE=0")
