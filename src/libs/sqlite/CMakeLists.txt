package_global(SQLite3)

if (NOT SQLITE3_FOUND)
set(SRCS sqlite3.c)
add_library(sqlite ${SRCS})
set_target_properties(sqlite PROPERTIES FOLDER sqlite)
add_definitions(${SQLITE3_DEFINITIONS} -DSQLITE_HAVE_ISNAN -DSQLITE_THREADSAFE=0)
set_target_properties(sqlite PROPERTIES COMPILE_FLAGS "-Wno-cast-qual")

set(SQLITE3_LIBRARIES "sqlite")
set(SQLITE3_INCLUDE_DIRS "${ROOT_DIR}/src/libs/sqlite")
var_global(SQLITE3_LIBRARIES)
var_global(SQLITE3_INCLUDE_DIRS)
endif()
