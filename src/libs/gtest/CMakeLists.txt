set(SRCS src/gtest-all.cc)
add_library(gtest ${SRCS})
set_target_properties(gtest PROPERTIES FOLDER gtest)
set_target_properties(gtest PROPERTIES COMPILE_FLAGS "-Wno-sign-compare")
