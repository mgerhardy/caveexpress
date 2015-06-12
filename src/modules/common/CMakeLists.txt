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
	ConsoleFrontend.cpp
	String.cpp
	ConfigVar.cpp
	CommandSystem.cpp
	ServiceProvider.cpp
	SpriteDefinition.cpp
	URI.cpp
	ByteStream.cpp
)

add_library(common ${common_SRCS})
set_target_properties(common PROPERTIES FOLDER common)
target_link_libraries(common network ports lua SDL sqlite)
