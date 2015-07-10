set(miniracer-shared_SRCS
	MiniRacerAnimation.cpp
	MiniRacerSQLitePersister.cpp
	MiniRacerSpriteType.cpp
	MiniRacerAchievement.cpp
	MiniRacerEntityType.cpp
)
add_library(miniracer-shared ${miniracer-shared_SRCS})
set_target_properties(miniracer-shared PROPERTIES FOLDER miniracer-shared)
cp_target_link_libraries(TARGET miniracer-shared LIBS common network)
