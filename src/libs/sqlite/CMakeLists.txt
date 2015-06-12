set(SRCS sqlite3.c)
add_library(sqlite ${SRCS})
set_target_properties(sqlite PROPERTIES FOLDER sqlite)
add_definitions(-DSQLITE_HAVE_ISNAN -DSQLITE_THREADSAFE=0)
