set(caveexpress-shared_SRCS
	CaveExpressAchievement.cpp
	CaveExpressSoundType.cpp
	CaveExpressAnimation.cpp
	CaveExpressSpriteType.cpp
	CaveExpressEntityType.cpp
	CaveExpressMapFailedReasons.cpp
)
add_library(caveexpress-shared ${caveexpress-shared_SRCS})
set_target_properties(caveexpress-shared PROPERTIES FOLDER caveexpress-shared)
target_link_libraries(caveexpress-shared common network)
