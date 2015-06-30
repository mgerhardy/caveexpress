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
	Log.cpp
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
cp_target_link_libraries(TARGET common LIBS network ports lua5.2 sdl2 sqlite3)
