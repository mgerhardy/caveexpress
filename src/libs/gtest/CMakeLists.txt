set(SRCS src/gtest-all.cc)
cp_add_library(LIB gtest SRCS ${SRCS} CFLAGS "-Wno-sign-compare")
set_target_properties(gtest PROPERTIES FOLDER gtest)
include_directories("${ROOT_DIR}/src/libs/gtest" "${ROOT_DIR}/src/libs/gtest/include")
