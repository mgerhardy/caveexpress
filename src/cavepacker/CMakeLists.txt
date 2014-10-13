project(cavepacker)

set(${PROJECT_NAME}_MAJOR_VERSION 0)
set(${PROJECT_NAME}_MINOR_VERSION 1)
set(${PROJECT_NAME}_PATCH_VERSION 0)
set(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_VERSION})

file(GLOB ${PROJECT_NAME}_SRC
	*.cpp
	client/ui/windows/*.cpp
	client/ui/windows/intro/*.cpp
	client/ui/nodes/*.cpp
	client/*.cpp
	server/*.cpp
	server/entities/*.cpp
	server/map/*.cpp
	shared/*.cpp
	shared/network/*.cpp)
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRC})

target_link_libraries(${PROJECT_NAME} engine)
