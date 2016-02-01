set(caveexpress-shared_SRCS
	CaveExpressAchievement.cpp
	CaveExpressSoundType.cpp
	CaveExpressCooldown.cpp
	CaveExpressAnimation.cpp
	CaveExpressSpriteType.cpp
	CaveExpressEntityType.cpp
	CaveExpressMapFailedReasons.cpp
)
add_library(caveexpress-shared ${caveexpress-shared_SRCS})
set_target_properties(caveexpress-shared PROPERTIES FOLDER caveexpress-shared)
cp_target_link_libraries(TARGET caveexpress-shared LIBS common network)
