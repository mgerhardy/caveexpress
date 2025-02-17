set(common_SRCS
	ConfigPersisterSQL.cpp
	ThemeType.cpp
	KeyValueParser.cpp
	TextureDefinition.cpp
	IConsole.cpp
	String.cpp
	SQLite.cpp
	Thread.cpp
	ConfigManager.cpp
	EventHandler.cpp
	MemoryAllocator.cpp
	TextConsole.cpp
	MapManager.cpp
	FileSystem.cpp
	LUALibrary.cpp
	LUAMapContext.cpp
	Log.cpp
	TimeManager.cpp
	ExecutionTime.cpp
	File.cpp
	ConsoleFrontend.cpp
	ConfigVar.cpp
	CommandSystem.cpp
	SpriteDefinition.cpp
	URI.cpp
	ByteStream.cpp
)

set(LIBS lua::lua SDL2::SDL2 SQLite::SQLite3 box2d glm)
if(WINDOWS)
	list(APPEND LIBS SDL2::SDL2main)
	set(ports_SRCS ports/Windows.cpp)
elseif(ANDROID)
	set(ports_SRCS ports/Unix.cpp ports/Android.cpp)
elseif(IOS)
	set(ports_SRCS ports/Unix.cpp ports/IOS.cpp ports/IOSObjc.mm)
elseif(DARWIN)
	set(ports_SRCS ports/Unix.cpp ports/Darwin.cpp ports/DarwinObjc.mm)
	list(APPEND LIBS "-framework CoreServices" "-framework Foundation" "-framework CoreFoundation")
elseif(EMSCRIPTEN)
	set(ports_SRCS ports/Unix.cpp ports/HTML5.cpp)
elseif(UNIX)
	set(ports_SRCS ports/Unix.cpp)
	add_definitions(-D_GNU_SOURCE -D_DEFAULT_SOURCE -D_XOPEN_SOURCE)
endif()

if (CURSES_FOUND)
	list(APPEND LIBS ${CURSES_LIBRARIES})
endif()

add_library(common ${common_SRCS} ${ports_SRCS})
set_target_properties(common PROPERTIES FOLDER common)
cp_target_link_libraries(TARGET common LIBS ${LIBS})
