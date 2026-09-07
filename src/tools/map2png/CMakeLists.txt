# One source file, two binaries. caveexpress-shared and cavepacker-shared both
# register SpriteType/EntityType names into a process-wide Enum map, so they
# cannot be linked into the same executable.
set(MAP2PNG_SOURCES Map2Png.cpp)

if (CAVEEXPRESS)
	add_executable(caveexpress-map2png ${MAP2PNG_SOURCES})
	set_target_properties(caveexpress-map2png PROPERTIES FOLDER map2png)
	target_compile_definitions(caveexpress-map2png PRIVATE MAP2PNG_CAVEEXPRESS)
	cp_target_link_libraries(TARGET caveexpress-map2png LIBS common client caveexpress-shared caveexpress-client caveexpress-server)
endif()

if (CAVEPACKER)
	add_executable(cavepacker-map2png ${MAP2PNG_SOURCES})
	set_target_properties(cavepacker-map2png PROPERTIES FOLDER map2png)
	target_compile_definitions(cavepacker-map2png PRIVATE MAP2PNG_CAVEPACKER)
	cp_target_link_libraries(TARGET cavepacker-map2png LIBS common client cavepacker-shared cavepacker-client cavepacker-server)
endif()
