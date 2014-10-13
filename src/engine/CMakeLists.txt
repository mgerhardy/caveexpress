project(engine)
FILE(GLOB_RECURSE ${PROJECT_NAME}_SRCS *.cpp)
add_library(${PROJECT_NAME} ${${PROJECT_NAME}_SRCS})
