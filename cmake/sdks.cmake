get_property(is_included GLOBAL PROPERTY cp_sdks_include_guard)
if (is_included)
return()
endif()
set_property(GLOBAL PROPERTY cp_sdks_include_guard true)

set(SDKS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../cp-sdks CACHE STRING "SDKs dir")

macro(cp_git_clone_sdk GITURL GITCLONETARGET)
	file(MAKE_DIRECTORY ${SDKS_DIR})
	if (NOT EXISTS ${SDKS_DIR}/${GITCLONETARGET})
		find_program(GIT_EXECUTABLE NAMES git)
		if (NOT GIT_EXECUTABLE)
			message(FATAL_ERROR "Could not setup the sdk for ${GITURL}")
		endif()
		execute_process(COMMAND ${GIT_EXECUTABLE} clone ${GITURL} ${GITCLONETARGET} WORKING_DIRECTORY ${SDKS_DIR})
	endif()
endmacro()

macro(cp_get_sdk URL FILENAME)
	file(DOWNLOAD ${URL} ${FILENAME} STATUS DOWNLOAD_STATUS)
	if (NOT DOWNLOAD_STATUS)
		message(FATAL_ERROR "Could not download ${URL}")
	endif()
endmacro()

if (STEAMLINK)
	cp_git_clone_sdk(https://github.com/ValveSoftware/steamlink-sdk.git steamlink-sdk)
endif()

if (EMSCRIPTEN)
	if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
		set(EMSDK_BIN emsdk.bat)
	else()
		set(EMSDK_BIN emsdk)
	endif()
	cp_git_clone_sdk(https://github.com/juj/emsdk emsdk)
	execute_process(COMMAND ${EMSDK_BIN} update WORKING_DIRECTORY ${SDKS_DIR}/emsdk)
	# TODO: support 32 bit
	execute_process(COMMAND ${EMSDK_BIN} install emscripten-incoming-64bit clang-incoming-64bit WORKING_DIRECTORY ${SDKS_DIR}/emsdk)
endif()

if (ANDROID)
# TODO: install android sdk and ndk
endif()

if ((MSYS OR MINGW) AND LINUX)
# TODO: install mxe
endif()

