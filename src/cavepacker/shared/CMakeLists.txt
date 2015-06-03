set(cavepacker-shared_SRCS
	CavePackerAnimation.cpp
	CavePackerSQLitePersister.cpp
	CavePackerSpriteType.cpp
	CavePackerAchievement.cpp
	CavePackerEntityType.cpp
	network/ProtocolMessageFactory.cpp
)
add_library(cavepacker-shared ${cavepacker-shared_SRCS})
set_target_properties(cavepacker-shared PROPERTIES FOLDER cavepacker-shared)
target_link_libraries(cavepacker-shared common network)
