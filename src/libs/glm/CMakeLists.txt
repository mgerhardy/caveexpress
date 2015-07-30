set(SRCS
	glm/detail/glm.cpp
	)
cp_add_library(LIB glm SRCS ${SRCS} CFLAGS "-Wno-shadow")
