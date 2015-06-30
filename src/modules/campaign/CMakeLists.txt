set(campaign_SRCS
	CampaignMap.cpp
	CampaignManager.cpp
	persister/SQLitePersister.cpp
	persister/GooglePlayPersister.cpp
	Campaign.cpp
)

add_library(campaign ${campaign_SRCS})
set_target_properties(campaign PROPERTIES FOLDER campaign)
cp_target_link_libraries(TARGET campaign LIBS common)
