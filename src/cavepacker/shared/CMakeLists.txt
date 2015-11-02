set(cavepacker-shared_SRCS
	CavePackerAnimation.cpp
	CavePackerSQLitePersister.cpp
	CavePackerSpriteType.cpp
	CavePackerAchievement.cpp
	CavePackerEntityType.cpp
	SolutionUtil.cpp
)
add_library(cavepacker-shared ${cavepacker-shared_SRCS})
set_target_properties(cavepacker-shared PROPERTIES FOLDER cavepacker-shared)
cp_target_link_libraries(TARGET cavepacker-shared LIBS common network)
