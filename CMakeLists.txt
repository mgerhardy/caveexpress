cmake_minimum_required(VERSION 2.6)

set(ROOT_DIR ${PROJECT_SOURCE_DIR})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${ROOT_DIR})
SET(DEFAULT_LUAC_EXECUTABLE luac)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ROOT_DIR}/cmake)
SET(CMAKE_ECLIPSE_GENERATE_LINKED_RESOURCES OFF)

option(RELEASE "Build a release build" OFF)

#-------------------------------------------------------------------------------
# macros
#-------------------------------------------------------------------------------

macro(texture_file_write TARGET_FILE FILEENTRY)
	file(READ ${FILEENTRY} CONTENTS)
	string(REGEX REPLACE ";" "\\\\;" CONTENTS "${CONTENTS}")
	string(REGEX REPLACE "\n" ";" CONTENTS "${CONTENTS}")
	list(REMOVE_AT CONTENTS 0 -2)
	foreach(LINE ${CONTENTS})
		file(APPEND ${TARGET_FILE} "${LINE}\n")
	endforeach()
endmacro()

macro(get_subdirs RESULT DIR)
	file(GLOB SUBDIRS RELATIVE ${DIR} ${DIR}/*)
	set(DIRLIST "")
	foreach(CHILD ${SUBDIRS})
		if (IS_DIRECTORY ${DIR}/${CHILD})
			list(APPEND DIRLIST ${CHILD})
		endif()
	endforeach()
	set(${RESULT} ${DIRLIST})
	list(SORT ${RESULT})
endmacro()

macro(create_dir_header PROJECTNAME)
	set(TARGET_FILE ${ROOT_DIR}/src/${PROJECTNAME}/dir.h)
	set(BASEDIR ${ROOT_DIR}/base/${PROJECTNAME})

	set(SUBDIRS)
	get_subdirs(SUBDIRS ${BASEDIR})
	foreach(SUBDIR ${SUBDIRS})
		file(GLOB FILESINDIR RELATIVE ${BASEDIR}/${SUBDIR} ${BASEDIR}/${SUBDIR}/*)
		list(LENGTH FILESINDIR LISTENTRIES)
		file(APPEND ${TARGET_FILE} "if (basedir == \"${i}/\") {\n")
		file(APPEND ${TARGET_FILE} "entriesAll.reserve(${LISTENTRIES});\n")
		foreach(FILEINDIR ${FILESINDIR})
			file(APPEND ${TARGET_FILE} "entriesAll.push_back(\"${FILEINDIR}\");\n")
		endforeach()
	endforeach()
	file(APPEND ${TARGET_FILE} "return entriesAll;\n}\n")
	message("wrote ${TARGET_FILE}")
endmacro()

macro(texture_merge TARGET_FILE FILELIST_BIG FILELIST_SMALL)
	file(WRITE ${TARGET_FILE} "")

	file(APPEND ${TARGET_FILE} "texturesbig = {\n")
	foreach(FILEENTRY ${FILELIST_BIG})
		texture_file_write(${TARGET_FILE} ${FILEENTRY})
	endforeach()
	file(APPEND ${TARGET_FILE} "}\n")

	file(APPEND ${TARGET_FILE} "texturessmall = {\n")
	foreach(FILEENTRY ${FILELIST_SMALL})
		texture_file_write(${TARGET_FILE} ${FILEENTRY})
	endforeach()
	file(APPEND ${TARGET_FILE} "}\n")
endmacro()

macro(textures PROJECTNAME)
	file(GLOB FILELIST_BIG ${ROOT_DIR}/contrib/assets/png-packed/${PROJECTNAME}-*big.lua)
	file(GLOB FILELIST_SMALL ${ROOT_DIR}/contrib/assets/png-packed/${PROJECTNAME}-*small.lua)
	message("build complete.lua: ${ROOT_DIR}/contrib/assets/png-packed/${PROJECTNAME}")
	texture_merge(${PROJECT_BINARY_DIR}/complete.lua.in "${FILELIST_BIG}" "${FILELIST_SMALL}")
	configure_file(${PROJECT_BINARY_DIR}/complete.lua.in ${ROOT_DIR}/base/${PROJECTNAME}/textures/complete.lua COPYONLY)
endmacro()

macro(texturepacker PROJECTNAME FILELIST)
	if (TEXTUREPACKER_BIN)
		set(DEPENDENCIES)
		foreach(FILEENTRY ${FILELIST})
			set(PNGBIG ${ROOT_DIR}/base/${PROJECTNAME}/pics/${FILEENTRY}-big.png)
			set(PNGSMALL ${ROOT_DIR}/base/${PROJECTNAME}/pics/${FILEENTRY}-small.png)
			set(TPS ${ROOT_DIR}/contrib/assets/png-packed/${FILEENTRY}.tps)
			add_custom_command(OUTPUT ${PNGBIG} ${PNGSMALL} COMMAND ${TEXTUREPACKER_BIN} ARGS ${TPS} DEPENDS ${TPS} VERBATIM)
			list(APPEND DEPENDENCIES ${PNGBIG} ${PNGSMALL})
		endforeach()
		if (DEPENDENCIES)
			add_custom_target(${PROJECTNAME}_texturepacker DEPENDS ${DEPENDENCIES})
			add_dependencies(${PROJECTNAME} ${PROJECTNAME}_texturepacker)
		endif()
		set(DEPENDENCIES)
	endif()
endmacro()

macro(check_lua_files TARGET FILES)
	find_program(LUAC_EXECUTABLE NAMES ${DEFAULT_LUAC_EXECUTABLE})
	if (LUAC_EXECUTABLE)
		message("${LUAC_EXECUTABLE} found")
		foreach(_FILE ${FILES})
			string(REPLACE "/" "-" TARGETNAME ${_FILE})
			add_custom_target(
				${TARGETNAME}
				COMMENT "checking lua file ${_FILE}"
				COMMAND ${DEFAULT_LUAC_EXECUTABLE} -p ${_FILE}
				WORKING_DIRECTORY ${ROOT_DIR}/base/${TARGET}
			)
			add_dependencies(${TARGET} ${TARGETNAME})
		endforeach()
	else()
		message(WARNING "No lua compiler (${DEFAULT_LUAC_EXECUTABLE}) found")
	endif()
endmacro()

#-------------------------------------------------------------------------------
# macros end
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# variables
#-------------------------------------------------------------------------------

if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
	set(WINDOWS 1)
elseif (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Darwin")
	set(DARWIN 1)
elseif (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
	set(LINUX 1)
else()
	message(WARNING "Unknown host system - default to linux")
	set(LINUX 1)
endif()
find_program(TEXTUREPACKER_BIN NAMES TexturePacker)
if (TEXTUREPACKER_BIN-NOTFOUND)
	message("Could not find TexturePacker")
else()
	message("Found TexturePacker: ${TEXTUREPACKER_BIN}")
endif()

#-------------------------------------------------------------------------------
# end variables
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# options
#-------------------------------------------------------------------------------

option(NETWORKING "Enable networking" ON)

#-------------------------------------------------------------------------------
# end options
#-------------------------------------------------------------------------------

if (NOT CMAKE_TOOLCHAIN_FILE)
	if (WINDOWS)
		include(${ROOT_DIR}/cmake/windows.cmake)
	elseif (DARWIN)
		include(${ROOT_DIR}/cmake/darwin.cmake)
	elseif (ANDROID)
		include(${ROOT_DIR}/cmake/android.cmake)
	elseif (IOS)
		include(${ROOT_DIR}/cmake/ios.cmake)
	elseif (NACL)
		include(${ROOT_DIR}/cmake/nacl.cmake)
	elseif (EMSCRIPTEN)
		include(${ROOT_DIR}/cmake/emscripten.cmake)
	else()
		include(${ROOT_DIR}/cmake/linux.cmake)
	endif()
endif()

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/src/libs")

if(CMAKE_BUILD_TYPE STREQUAL "")
set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if (UNIX)
	find_program(CCACHE "ccache")
	if (CCACHE)
		message("Using ccache")
		set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE})
		set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE})
	else()
		message(WARNING "ccache not found")
	endif()
endif()

include_directories("${PROJECT_SOURCE_DIR}/src/libs/lua")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/Box2D")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/tinyxml2")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/sqlite")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/gtest/include")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/gtest")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/libogg-1.3.1/include")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/libpng-1.6.2")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/yajl")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/SDL_mixer")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/SDL_net")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/SDL_image")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/SDL/include")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/SDL/src")
include_directories("${PROJECT_SOURCE_DIR}/src/modules")
include_directories("${PROJECT_SOURCE_DIR}/src/modules/game")

if (UNIX AND NOT ANDROID)
include_directories("${PROJECT_SOURCE_DIR}/src/libs/SDL/linux")
message("Use preconfigured SDL for linux")
else()
include_directories("${PROJECT_SOURCE_DIR}/src/libs/SDL/general")
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/src/libs)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/modules)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/cavepacker)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/caveexpress)
if (NOT ANDROID)
	add_subdirectory(${PROJECT_SOURCE_DIR}/src/tests)
endif()
