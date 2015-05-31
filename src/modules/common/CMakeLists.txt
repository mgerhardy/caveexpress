set(common_SRCS
	ConfigPersisterSQL.cpp
	ThemeType.cpp
	KeyValueParser.cpp
	TextureDefinition.cpp
	IConsole.cpp
	SQLite.cpp
	Thread.cpp
	ConfigManager.cpp
	EventHandler.cpp
	MemoryAllocator.cpp
	TextConsole.cpp
	MapManager.cpp
	FileSystem.cpp
	LUA.cpp
	Logger.cpp
	TimeManager.cpp
	ExecutionTime.cpp
	File.cpp
	network/NoNetwork.cpp
	network/Network.cpp
	network/INetwork.cpp
	ConsoleFrontend.cpp
	String.cpp
	ConfigVar.cpp
	CommandSystem.cpp
	ServiceProvider.cpp
	SpriteDefinition.cpp
	URI.cpp
	campaign/CampaignMap.cpp
	campaign/CampaignManager.cpp
	campaign/persister/SQLitePersister.cpp
	campaign/persister/GooglePlayPersister.cpp
	campaign/Campaign.cpp
	ByteStream.cpp
)

if(WIN32)
set(common_PLATFORM_SRCS
	ports/Windows.cpp
	ports/project.rc)
elseif(APPLE)
set(common_PLATFORM_SRCS
	ports/Darwin.cpp
	ports/Cocoa.mm)
elseif(UNIX)
set(common_PLATFORM_SRCS
	ports/Unix.cpp)
	add_definitions(-D_GNU_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE)
endif()

#ports/HTML5.cpp
#ports/IOS.cpp
#ports/Android.cpp
#ports/NaCl.cpp

add_library(common ${common_SRCS} ${common_PLATFORM_SRCS})
set_target_properties(common PROPERTIES FOLDER common)
target_link_libraries(common libs)
