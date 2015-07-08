set(SRCS src/gtest-all.cc)
cp_add_library(LIB gtest SRCS ${SRCS} CFLAGS "-Wno-sign-compare" DEFINES "-DGTEST_HAS_EXCEPTIONS=0")
include_directories("${ROOT_DIR}/src/libs/gtest" "${ROOT_DIR}/src/libs/gtest/include")
