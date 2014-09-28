project(engine)
FILE(GLOB ${PROJECT_NAME}_SRCS *.cpp)
add_library(${PROJECT_NAME} ${${PROJECT_NAME}_SRCS})
