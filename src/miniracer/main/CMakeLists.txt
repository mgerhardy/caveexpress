project(miniracer)
cp_add_executable(TARGET ${PROJECT_NAME} SRCS MiniRacer.cpp ../../Main.cpp WINDOWED APPNAME "MiniRacer" VERSION 1.4 VERSION_CODE 9)
cp_target_link_libraries(TARGET ${PROJECT_NAME} LIBS server game common service)
set(TPS
	miniracer
)
#texturepacker(${PROJECT_NAME} "${TPS}")
#textures(${PROJECT_NAME})
create_dir_header(${PROJECT_NAME})
