include(CMakeParseArguments)

if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
	set(CP_GCC 1)
	message(STATUS "C++ Compiler: GCC")
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
	set(CP_CLANG 1)
	message(STATUS "C++ Compiler: Clang")
elseif (MSVC)
	set(CP_MSVC 1)
	message(STATUS "C++ Compiler: MSVC")
else()
	message(WARNING "C++ Compiler: Unknown")
endif()
message(STATUS "Target processor: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Host processor: ${CMAKE_HOST_SYSTEM_PROCESSOR}")

#-------------------------------------------------------------------------------
# macros
#-------------------------------------------------------------------------------

if (NOT WIN32)
	string(ASCII 27 Esc)
	set(ColorReset  "${Esc}[m")
	set(ColorRed    "${Esc}[31m")
	set(ColorGreen  "${Esc}[32m")
	set(ColorYellow "${Esc}[33m")
	set(ColorBlue   "${Esc}[34m")
endif()

#function(message)
#	list(GET ARGV 0 TYPE)
#	if (TYPE STREQUAL FATAL_ERROR)
#		list(REMOVE_AT ARGV 0)
#		_message(${TYPE} "${ColorRed}${ARGV}${ColorReset}")
#	elseif(TYPE STREQUAL WARNING)
#		list(REMOVE_AT ARGV 0)
#		_message(${TYPE} "${ColorYellow}${ARGV}${ColorReset}")
#	elseif(TYPE STREQUAL STATUS)
#		list(REMOVE_AT ARGV 0)
#		_message(${TYPE} "${ColorGreen}${ARGV}${ColorReset}")
#	elseif (ARGV)
#		_message("${ARGV}")
#	endif()
#endfunction()

macro(cp_message MSG)
	if (VERBOSE)
		message("${ColorBlue}${MSG}${ColorReset}")
	endif()
endmacro()

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
	if (DIRLIST)
		set(${RESULT} ${DIRLIST})
		list(SORT ${RESULT})
	endif()
endmacro()

macro(create_dir_header PROJECTNAME)
	set(TARGET_DIR ${CMAKE_BINARY_DIR})
	file(MAKE_DIRECTORY ${TARGET_DIR})
	set(TARGET_FILE ${TARGET_DIR}/${PROJECTNAME}-files.h.in)
	set(BASEDIR ${ROOT_DIR}/base/${PROJECTNAME})
	set(FINAL_TARGET_FILE ${TARGET_DIR}/${PROJECTNAME}-files.h)

	set(SUBDIRS)
	get_subdirs(SUBDIRS ${BASEDIR})
	file(WRITE ${TARGET_FILE} "")
	foreach(SUBDIR ${SUBDIRS})
		file(GLOB FILESINDIR RELATIVE ${BASEDIR}/${SUBDIR} ${BASEDIR}/${SUBDIR}/*)
		list(LENGTH FILESINDIR LISTENTRIES)
		file(APPEND ${TARGET_FILE} "if (basedir == \"${SUBDIR}/\") {\n")
		file(APPEND ${TARGET_FILE} "\tentriesAll.reserve(${LISTENTRIES});\n")
		set(RESULT ${FILESINDIR})
		list(SORT RESULT)
		foreach(FILEINDIR ${RESULT})
			string(COMPARE EQUAL "${FILEINDIR}" ".gitignore" IGNOREFILE)
			if (NOT IGNOREFILE)
				file(APPEND ${TARGET_FILE} "\tentriesAll.push_back(\"${FILEINDIR}\");\n")
			endif()
		endforeach()
		file(APPEND ${TARGET_FILE} "\treturn entriesAll;\n}\n")
	endforeach()
	configure_file(${TARGET_FILE} ${FINAL_TARGET_FILE})
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

macro(texturepacker)
	set(_OPTIONS_ARGS)
	set(_ONE_VALUE_ARGS PROJECTNAME)
	set(_MULTI_VALUE_ARGS FILELIST)

	cmake_parse_arguments(_TP "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN} )

	if (NOT _TP_PROJECTNAME)
		message(FATAL_ERROR "texturepacker requires the PROJECTNAME argument")
	endif()
	if (NOT _TP_FILELIST)
		message(FATAL_ERROR "texturepacker requires the FILELIST argument")
	endif()

	if (TEXTUREPACKER_BIN)
		set(DEPENDENCIES)
		foreach(FILEENTRY ${_TP_FILELIST})
			string(REPLACE "{n}" "0" FILEENTRY_N "${FILEENTRY}")
			set(PNGBIG ${ROOT_DIR}/base/${_TP_PROJECTNAME}/pics/${FILEENTRY_N}-big.png)
			set(PNGSMALL ${ROOT_DIR}/base/${_TP_PROJECTNAME}/pics/${FILEENTRY_N}-small.png)
			string(REPLACE "-{n}" "" FILEENTRY_CLEAN "${FILEENTRY}")
			set(TPS ${ROOT_DIR}/contrib/assets/png-packed/${FILEENTRY_CLEAN}.tps)
			add_custom_command(OUTPUT ${PNGBIG} ${PNGSMALL} COMMAND ${TEXTUREPACKER_BIN} ARGS ${TPS} DEPENDS ${TPS} VERBATIM)
			list(APPEND DEPENDENCIES ${PNGBIG} ${PNGSMALL})
		endforeach()
		if (DEPENDENCIES)
			add_custom_target(${_TP_PROJECTNAME}_texturepacker DEPENDS ${DEPENDENCIES})
			add_dependencies(${_TP_PROJECTNAME} ${_TP_PROJECTNAME}_texturepacker)
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

#
# Install android packages
#
# parameters:
# PACKAGE The package id that you need to install
#
macro(cp_android_package PACKAGE)
	message("install android sdk package ${PACKAGE}")
	file(WRITE ${CMAKE_BINARY_DIR}/yes.txt "y")
	execute_process(
		COMMAND ${ANDROID_SDK_TOOL} list target -c
		OUTPUT_VARIABLE TARGETS_LIST
	)
	if (${TARGETS_LIST} MATCHES ${PACKAGE})
		message("${PACKAGE} is already installed")
	else()
		execute_process(
			COMMAND ${ANDROID_SDK_TOOL} update sdk -a -u -s -t ${PACKAGE}
			INPUT_FILE ${CMAKE_BINARY_DIR}/yes.txt
		)
	endif()
endmacro()

#
# Prepare android workspace with assets and sdk/ndk commands.
#
# Also adds some helper targets:
# * android-PROJECTNAME-uninstall uninstalls the application
# * android-PROJECTNAME-install installs the application
# * android-PROJECTNAME-start starts the application
# * android-PROJECTNAME-backtrace creates a backtrace from a crash
#
# parameters:
# PROJECTNAME: the project name in lower case letters - used for e.g. resolving the java classes and icons
#              if this is a test project for a specific game, it should start with 'tests_' - as we then just
#              reuse some of the game settings and assets for the tests.
# APPNAME: the normal app name, must not contain whitespaces, but can contain upper case letters.
# VERSION: the version code, e.g. 1.0
# VERSION_CODE: the android version code needed for google play store
#
macro(cp_android_prepare PROJECTNAME APPNAME VERSION VERSION_CODE)
	if (ANDROID_INSTALL_PACKAGES)
		cp_android_package("android-13")
		cp_android_package("android-16")
		cp_android_package("extra-google-google_play_services")
		cp_android_package("platform-tools")
		cp_android_package("build-tools-${ANDROID_SDK_BUILD_TOOLS_VERSION}")
		set(ANDROID_INSTALL_PACKAGES OFF)
	endif()
	# TODO: add java app and activity classes to dependencies to recompile target on java class changes
	message("prepare java code for ${PROJECTNAME}")
	file(COPY ${ANDROID_ROOT} DESTINATION ${CMAKE_BINARY_DIR}/android-${PROJECTNAME})
	if (HD_VERSION)
		set(PACKAGENAME ${PROJECTNAME}hd)
	else()
		set(PACKAGENAME ${PROJECTNAME})
	endif()
	set(APPCLASS ${APPNAME})
	if (${PACKAGENAME} MATCHES "tests")
		set(PACKAGENAME "tests")
		set(APPCLASS "TestsApp")
	endif()
	set(WHITELIST base libsdl ${PACKAGENAME})
	set(ANDROID_BIN_ROOT ${CMAKE_BINARY_DIR}/android-${PROJECTNAME})
	get_subdirs(SUBDIRS ${ANDROID_BIN_ROOT}/src/org)
	list(REMOVE_ITEM SUBDIRS ${WHITELIST})
	foreach(DIR ${SUBDIRS})
		file(REMOVE_RECURSE ${ANDROID_BIN_ROOT}/src/org/${DIR})
	endforeach()
	configure_file(${ANDROID_BIN_ROOT}/AndroidManifest.xml.in ${ANDROID_BIN_ROOT}/AndroidManifest.xml @ONLY)
	configure_file(${ANDROID_BIN_ROOT}/strings.xml.in ${ANDROID_BIN_ROOT}/res/values/strings.xml @ONLY)
	configure_file(${ANDROID_BIN_ROOT}/default.properties.in ${ANDROID_BIN_ROOT}/default.properties @ONLY)
	configure_file(${ANDROID_BIN_ROOT}/project.properties.in ${ANDROID_BIN_ROOT}/project.properties @ONLY)
	add_custom_target(android-${PROJECTNAME}-backtrace ${ANDROID_ADB} logcat | ndk-stack -sym ${ANDROID_BIN_ROOT}/obj/local/${ANDROID_NDK_SYMDIR} WORKING_DIRECTORY ${ANDROID_BIN_ROOT})
	add_custom_target(android-${PROJECTNAME}-install ${ANDROID_ANT} ${ANT_INSTALL_TARGET} WORKING_DIRECTORY ${ANDROID_BIN_ROOT})
	add_custom_target(android-${PROJECTNAME}-uninstall ${ANDROID_ANT} uninstall WORKING_DIRECTORY ${ANDROID_BIN_ROOT})
	set(APP_PACKAGENAME "org.${PACKAGENAME}")
	add_custom_target(android-${PROJECTNAME}-start ${ANDROID_ADB} shell am start -n ${APP_PACKAGENAME}/${APP_PACKAGENAME}.${APPCLASS} WORKING_DIRECTORY ${ANDROID_BIN_ROOT})
	string(REPLACE "tests_" "" CLEAN_PROJECTNAME "${PROJECTNAME}")
	if (EXISTS ${ROOT_DIR}/contrib/installer/android/${CLEAN_PROJECTNAME}/)
		file(COPY ${ROOT_DIR}/contrib/installer/android/${CLEAN_PROJECTNAME}/ DESTINATION ${ANDROID_BIN_ROOT})
	endif()
	message(STATUS "copy files from ${ROOT_DIR}/base/${CLEAN_PROJECTNAME} to ${ANDROID_BIN_ROOT}/assets/base/${CLEAN_PROJECTNAME}")
	file(COPY ${ROOT_DIR}/base/${CLEAN_PROJECTNAME} DESTINATION ${ANDROID_BIN_ROOT}/assets/base)
	#install(DIRECTORY ${ROOT_DIR}/base/${CLEAN_PROJECTNAME} DESTINATION ${ANDROID_BIN_ROOT}/assets/base)
	set(RESOLUTIONS hdpi ldpi mdpi xhdpi)
	set(ICON "${PACKAGENAME}-icon.png")
	if (NOT EXISTS ${ROOT_DIR}/contrib/${ICON})
		set(ICON "${CLEAN_PROJECTNAME}-icon.png")
	endif()
	if (EXISTS ${ROOT_DIR}/contrib/${ICON})
		foreach(RES ${RESOLUTIONS})
			file(MAKE_DIRECTORY ${ANDROID_BIN_ROOT}/res/drawable-${RES})
			configure_file(${ROOT_DIR}/contrib/${ICON} ${ANDROID_BIN_ROOT}/res/drawable-${RES}/icon.png COPYONLY)
		endforeach()
	endif()
	set(ANDROID_SO_OUTDIR ${ANDROID_BIN_ROOT}/libs/${ANDROID_NDK_SYMDIR})
	set_target_properties(${PROJECTNAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${ANDROID_SO_OUTDIR})
	set_target_properties(${PROJECTNAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE ${ANDROID_SO_OUTDIR})
	set_target_properties(${PROJECTNAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG ${ANDROID_SO_OUTDIR})
	if (NOT EXISTS ${ANDROID_BIN_ROOT}/local.properties)
		message("=> create Android SDK project: ${PROJECTNAME}")
		execute_process(COMMAND ${ANDROID_SDK_TOOL} --silent update project
				--path .
				--name ${APPNAME}
				--target ${ANDROID_API}
				WORKING_DIRECTORY ${ANDROID_BIN_ROOT})
		execute_process(COMMAND ${ANDROID_SDK_TOOL} --silent update lib-project
				--path google-play-services_lib
				--target ${ANDROID_API}
				WORKING_DIRECTORY ${ANDROID_BIN_ROOT})
	endif()
	set(APK_NAME ${APPNAME}-${ANT_TARGET}-unsigned.apk)
	set(FINAL_APK_NAME ${APPNAME}-${ANT_TARGET}.apk)

	add_custom_command(TARGET ${PROJECTNAME} POST_BUILD COMMAND ${ANDROID_ANT} ${ANT_FLAGS} ${ANT_TARGET} WORKING_DIRECTORY ${ANDROID_BIN_ROOT})
	add_custom_command(TARGET ${PROJECTNAME} POST_BUILD COMMAND ${JARSIGNER} -sigalg SHA1withRSA -digestalg SHA1 -keypass ${ANDROID_KEYPASS} -storepass ${ANDROID_STOREPASS} -keystore ${ANDROID_KEYSTORE} ${APK_NAME} ${ANDROID_ALIAS} WORKING_DIRECTORY ${ANDROID_BIN_ROOT}/bin/)
	add_custom_command(TARGET ${PROJECTNAME} POST_BUILD COMMAND ${JARSIGNER} -verify -certs ${APK_NAME} WORKING_DIRECTORY ${ANDROID_BIN_ROOT}/bin/)
	add_custom_command(TARGET ${PROJECTNAME} POST_BUILD COMMAND ${ANDROID_ZIPALIGN} -f -v 4 ${APK_NAME} ${FINAL_APK_NAME} WORKING_DIRECTORY ${ANDROID_BIN_ROOT}/bin/)
	#add_custom_command(TARGET ${PROJECTNAME} POST_BUILD COMMAND ${ANDROID_ANT} uninstall ${ANT_INSTALL_TARGET} WORKING_DIRECTORY ${ANDROID_BIN_ROOT})
	#add_custom_command(TARGET ${PROJECTNAME} POST_BUILD COMMAND ${ANDROID_ADB} shell am start -n org.${PROJECTNAME}/org.${PROJECTNAME}.${APPNAME} WORKING_DIRECTORY ${ANDROID_BIN_ROOT})
endmacro()

#
# put a variable into the global namespace
#
macro(var_global VARIABLES)
	foreach(VAR ${VARIABLES})
		cp_message("${VAR} => ${${VAR}}")
		set(${VAR} ${${VAR}} CACHE STRING "" FORCE)
		mark_as_advanced(${VAR})
	endforeach()
endmacro()

macro(package_global LIB)
	find_package(${LIB})
endmacro()

#
# Add external dependency. It will trigger a find_package and use the system wide install if found, otherwise the bundled version
# If you set USE_BUILTIN the system wide is ignored.
#
# parameters:
# LIB:
# CFLAGS:
# LINKERFLAGS:
# SRCS: the list of source files for the bundled lib
# DEFINES: a list of defines (without -D or /D)
#
macro(cp_add_library)
	set(_OPTIONS_ARGS)
	set(_ONE_VALUE_ARGS LIB CFLAGS LINKERFLAGS)
	set(_MULTI_VALUE_ARGS SRCS DEFINES)

	cmake_parse_arguments(_ADDLIB "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN} )

	if (NOT _ADDLIB_LIB)
		message(FATAL_ERROR "cp_add_library requires the LIB argument")
	endif()
	if (NOT _ADDLIB_SRCS)
		message(FATAL_ERROR "cp_add_library requires the SRCS argument")
	endif()

	find_package(${_ADDLIB_LIB})
	string(TOUPPER ${_ADDLIB_LIB} PREFIX)
	if (${PREFIX}_FOUND)
		message(STATUS "System wide installation found for ${_ADDLIB_LIB}")
		var_global(${PREFIX}_FOUND)
	else()
		message(STATUS "No system wide installation found, use built-in version of ${_ADDLIB_LIB}")
		set(${PREFIX}_LIBRARIES ${_ADDLIB_LIB})
		set(${PREFIX}_LINKERFLAGS ${_ADDLIB_LINKERFLAGS})
		#set(${PREFIX}_COMPILERFLAGS "${_ADDLIB_DEFINES} ${_ADDLIB_CFLAGS}")
		set(${PREFIX}_DEFINITIONS ${_ADDLIB_DEFINES})
		set(${PREFIX}_INCLUDE_DIRS ${ROOT_DIR}/src/libs/${_ADDLIB_LIB})
		add_library(${_ADDLIB_LIB} STATIC ${_ADDLIB_SRCS})
		include_directories(${${PREFIX}_INCLUDE_DIRS})
		set_target_properties(${_ADDLIB_LIB} PROPERTIES COMPILE_DEFINITIONS "${${PREFIX}_DEFINITIONS}")
		if (NOT CP_MSVC)
			set_target_properties(${_ADDLIB_LIB} PROPERTIES COMPILE_FLAGS "${_ADDLIB_CFLAGS}")
		endif()
		set_target_properties(${_ADDLIB_LIB} PROPERTIES FOLDER ${_ADDLIB_LIB})
		#var_global(${PREFIX}_COMPILERFLAGS)
		var_global(${PREFIX}_LIBRARIES)
		var_global(${PREFIX}_LINKERFLAGS)
		var_global(${PREFIX}_INCLUDE_DIRS)
		var_global(${PREFIX}_DEFINITIONS)
	endif()
	set(${PREFIX}_EXTERNAL ON)
	var_global(${PREFIX}_EXTERNAL)
endmacro()

#
# macro for the FindLibName.cmake files. If USE_BUILTIN is set we don't search for system wide installs at all.
#
# parameters:
# LIB: the library we are trying to find
# HEADER: the header we are trying to find
# SUFFIX: suffix for the include dir
#
# Example: cp_find(SDL2_image SDL_image.h include/SDL2)
#
macro(cp_find LIB HEADER SUFFIX)
	if (NOT USE_BUILTIN)
		string(TOUPPER ${LIB} PREFIX)
		if(CMAKE_SIZEOF_VOID_P EQUAL 8)
			set(_PROCESSOR_ARCH "x64")
		else()
			set(_PROCESSOR_ARCH "x86")
		endif()
		set(_SEARCH_PATHS
			~/Library/Frameworks
			/Library/Frameworks
			/usr/local
			/usr
			/sw # Fink
			/opt/local # DarwinPorts
			/opt/csw # Blastwave
			/opt
		)
		find_package(PkgConfig)
		if (LINK_STATIC_LIBS)
			set(PKG_PREFIX _${PREFIX}_STATIC)
		else()
			set(PKG_PREFIX _${PREFIX})
		endif()
		if (PKG_CONFIG_FOUND)
			message(STATUS "Checking for ${LIB}")
			pkg_check_modules(_${PREFIX} ${LIB})
			if (_${PREFIX}_FOUND)
				cp_message("Found ${LIB} via pkg-config")
				cp_message("CFLAGS: ${${PKG_PREFIX}_CFLAGS_OTHER}")
				cp_message("LDFLAGS: ${${PKG_PREFIX}_LDFLAGS_OTHER}")
				cp_message("LIBS: ${${PKG_PREFIX}_LIBRARIES}")
				cp_message("INCLUDE: ${${PKG_PREFIX}_INCLUDE_DIRS}")
				set(${PREFIX}_COMPILERFLAGS ${${PKG_PREFIX}_CFLAGS_OTHER})
				var_global(${PREFIX}_COMPILERFLAGS)
				set(${PREFIX}_LINKERFLAGS ${${PKG_PREFIX}_LDFLAGS_OTHER})
				var_global(${PREFIX}_LINKERFLAGS)
				set(${PREFIX}_FOUND ${${PKG_PREFIX}_FOUND})
				var_global(${PREFIX}_FOUND)
				set(${PREFIX}_INCLUDE_DIRS ${${PKG_PREFIX}_INCLUDE_DIRS})
				var_global(${PREFIX}_INCLUDE_DIRS)
				set(${PREFIX}_LIBRARY_DIRS ${${PKG_PREFIX}_LIBRARY_DIRS})
				var_global(${PREFIX}_LIBRARY_DIRS)
				set(${PREFIX}_LIBRARIES ${${PKG_PREFIX}_LIBRARIES})
				var_global(${PREFIX}_LIBRARIES)
			else()
				cp_message("Could not find ${LIB} via pkg-config")
			endif()
		endif()
		if (NOT ${PKG_PREFIX}_FOUND)
			find_path(${PREFIX}_INCLUDE_DIRS ${HEADER}
				HINTS ENV ${PREFIX}DIR
				PATH_SUFFIXES include ${SUFFIX}
				PATHS
					${${PKG_PREFIX}_INCLUDE_DIRS}}
					${_SEARCH_PATHS}
			)
			find_library(${PREFIX}_LIBRARIES
				${LIB}
				HINTS ENV ${PREFIX}DIR
				PATH_SUFFIXES lib64 lib lib/${_PROCESSOR_ARCH}
				PATHS
					${${PKG_PREFIX}_LIBRARY_DIRS}}
					${_SEARCH_PATHS}
			)
			include(FindPackageHandleStandardArgs)
			find_package_handle_standard_args(${LIB} DEFAULT_MSG ${PREFIX}_LIBRARIES ${PREFIX}_INCLUDE_DIRS)
			var_global(${PREFIX}_INCLUDE_DIRS)
			var_global(${PREFIX}_LIBRARIES)
			var_global(${PREFIX}_FOUND)
		endif()
	endif()
endmacro()

macro(cp_recurse_resolve_dependencies LIB DEPS)
	list(APPEND ${DEPS} ${LIB})
	get_property(_DEPS GLOBAL PROPERTY ${LIB}_DEPS)
	foreach(DEP ${_DEPS})
		#cp_message("=> resolved dependency ${DEP} for ${LIB}")
		cp_recurse_resolve_dependencies(${DEP} ${DEPS})
	endforeach()
endmacro()

macro(cp_resolve_dependencies LIB DEPS)
	get_property(_DEPS GLOBAL PROPERTY ${LIB}_DEPS)
	list(APPEND ${DEPS} ${LIB})
	foreach(DEP ${_DEPS})
		#cp_message("=> resolved dependency ${DEP} for ${LIB}")
		cp_recurse_resolve_dependencies(${DEP} ${DEPS})
	endforeach()
endmacro()

macro(cp_unique INPUTSTR OUT)
	string(REPLACE " " ";" TMP "${INPUTSTR}")
	list(REVERSE TMP)
	list(REMOVE_DUPLICATES TMP)
	list(REVERSE TMP)
	string(REPLACE ";" " " ${OUT} "${TMP}")
endmacro()

macro(cp_target_link_libraries)
	set(_OPTIONS_ARGS)
	set(_ONE_VALUE_ARGS TARGET)
	set(_MULTI_VALUE_ARGS LIBS)

	cmake_parse_arguments(_LINKLIBS "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN} )

	if (NOT _LINKLIBS_TARGET)
		message(FATAL_ERROR "cp_target_link_libraries requires the TARGET argument")
	endif()
	if (NOT _LINKLIBS_LIBS)
		message(FATAL_ERROR "cp_target_link_libraries requires the LIBS argument")
	endif()
	set(LINK_LIBS)
	cp_message("Resolve dependencies for ${ColorGreen}${_LINKLIBS_TARGET}${ColorReset}")
	foreach(LIB ${_LINKLIBS_LIBS})
		cp_resolve_dependencies(${LIB} LINK_LIBS)
	endforeach()
	if (LINK_LIBS)
		list(REVERSE LINK_LIBS)
		list(REMOVE_DUPLICATES LINK_LIBS)
		list(REVERSE LINK_LIBS)
	endif()
	cp_message("Dependencies for ${ColorGreen}${_LINKLIBS_TARGET}${ColorReset}: ${LINK_LIBS}")

	string(TOUPPER ${_LINKLIBS_TARGET} TARGET)

	set(LINKERFLAGS)
	set(COMPILERFLAGS)
	set(LIBRARIES)
	set(DEFINITIONS)
	set(INCLUDE_DIRS)
	if (${_LINKLIBS_TARGET}_INCLUDE_DIRS)
		list(APPEND INCLUDE_DIRS ${${_LINKLIBS_TARGET}_INCLUDE_DIRS})
	endif()

	foreach(LIB ${LINK_LIBS})
		string(TOUPPER ${LIB} PREFIX)
		if (${PREFIX}_LINKERFLAGS)
			list(APPEND LINKERFLAGS ${${PREFIX}_LINKERFLAGS})
			cp_message("Add linker flags from ${LIB} to ${_LINKLIBS_TARGET}")
		endif()
		if (${PREFIX}_COMPILERFLAGS)
			list(APPEND COMPILERFLAGS ${${PREFIX}_COMPILERFLAGS})
			cp_message("Add compiler flags from ${LIB} to ${_LINKLIBS_TARGET}")
		endif()
		if (${PREFIX}_DEFINITIONS)
			list(APPEND DEFINITIONS ${${PREFIX}_DEFINITIONS})
			cp_message("Add definitions from ${LIB} to ${_LINKLIBS_TARGET}")
		endif()
		if (NOT ${PREFIX}_EXTERNAL)
			list(APPEND LIBRARIES ${LIB})
		endif()
		if (${PREFIX}_LIBRARIES)
			list(APPEND LIBRARIES ${${PREFIX}_LIBRARIES})
		endif()
		if (${PREFIX}_INCLUDE_DIRS)
			list(APPEND INCLUDE_DIRS ${${PREFIX}_INCLUDE_DIRS})
			cp_message("Add include dirs for ${LIB} to ${_LINKLIBS_TARGET}: ${${PREFIX}_INCLUDE_DIRS}")
		endif()
	endforeach()
	if (LINKERFLAGS)
		cp_unique("${LINKERFLAGS}" ${TARGET}_LINKERFLAGS)
		cp_message("LDFLAGS for ${_LINKLIBS_TARGET}: ${${TARGET}_LINKERFLAGS}")
		var_global(${TARGET}_LINKERFLAGS)
		set_target_properties(${_LINKLIBS_TARGET} PROPERTIES LINK_FLAGS "${${TARGET}_LINKERFLAGS}")
	endif()
	if (COMPILERFLAGS)
		cp_unique("${COMPILERFLAGS}" ${TARGET}_COMPILERFLAGS)
		cp_message("CFLAGS for ${_LINKLIBS_TARGET}: ${${TARGET}_COMPILERFLAGS}")
		var_global(${TARGET}_COMPILERFLAGS)
		set_target_properties(${_LINKLIBS_TARGET} PROPERTIES COMPILE_FLAGS "${${TARGET}_COMPILERFLAGS}")
	endif()
	if (DEFINITIONS)
		set(${TARGET}_DEFINITIONS ${DEFINITIONS})
		list(REMOVE_DUPLICATES ${TARGET}_DEFINITIONS)
		cp_message("DEFINITIONS for ${_LINKLIBS_TARGET}: ${${TARGET}_DEFINITIONS}")
		var_global(${TARGET}_DEFINITIONS)
		set_target_properties(${_LINKLIBS_TARGET} PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}")
	endif()

	set_property(GLOBAL PROPERTY ${_LINKLIBS_TARGET}_DEPS ${_LINKLIBS_LIBS})
	if (LIBRARIES)
		list(REVERSE LIBRARIES)
		list(REMOVE_DUPLICATES LIBRARIES)
		list(REVERSE LIBRARIES)
		cp_message("LIBS for ${ColorRed}${_LINKLIBS_TARGET}${ColorReset} => ${LIBRARIES} => ${LINK_LIBS}")
		target_link_libraries(${_LINKLIBS_TARGET} ${LIBRARIES})
	endif()
	if (INCLUDE_DIRS)
		list(REMOVE_DUPLICATES INCLUDE_DIRS)
		cp_message("Set include dir for ${_LINKLIBS_TARGET} to ${INCLUDE_DIRS}")
		include_directories(${INCLUDE_DIRS})
	endif()
endmacro()

macro(cp_osx_generate_plist_file TARGET)
	set(PLIST_PATH ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
	set_target_properties(${TARGET} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PLIST_PATH})
	if (IOS)
		configure_file(${ROOT_DIR}/cmake/iOSBundleInfo.plist.template ${PLIST_PATH} @ONLY)
	else()
		configure_file(${ROOT_DIR}/cmake/MacOSXBundleInfo.plist.template ${PLIST_PATH} @ONLY)
	endif()
endmacro()

macro(cp_osx_add_target_properties TARGET)
	cp_osx_generate_plist_file(${TARGET})
	if (IOS)
		set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iOS Developer")
		set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2")
	endif()
	set_target_properties(${TARGET} PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.${TARGET}")
	set_target_properties(${TARGET} PROPERTIES MACOSX_BUNDLE_ICON_FILE "Icon.png")
	if (${CMAKE_GENERATOR} STREQUAL "Xcode")
		set_target_properties(${TARGET} PROPERTIES MACOSX_BUNDLE_EXECUTABLE_NAME \${EXECUTABLE_NAME})
		set_target_properties(${TARGET} PROPERTIES MACOSX_BUNDLE_PRODUCT_NAME \${PRODUCT_NAME})
		set_target_properties(${TARGET} PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME \${PRODUCT_NAME})
	else()
		set_target_properties(${TARGET} PROPERTIES MACOSX_BUNDLE_EXECUTABLE_NAME "${TARGET}")
		set_target_properties(${TARGET} PROPERTIES MACOSX_BUNDLE_PRODUCT_NAME "${TARGET}")
		set_target_properties(${TARGET} PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "${TARGET}")
	endif()
endmacro()

#
# set up the binary for the application. This will also set up platform specific stuff for you
#
# Example: cp_add_executable(TARGET SomeTargetName SRCS Source.cpp Main.cpp WINDOWED APPNAME "Some App Name" VERSION 1.0 VERSION_CODE 1)
#
macro(cp_add_executable)
	set(_OPTIONS_ARGS WINDOWED)
	set(_ONE_VALUE_ARGS TARGET VERSION VERSION_CODE APPNAME)
	set(_MULTI_VALUE_ARGS SRCS)

	cmake_parse_arguments(_EXE "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN} )

	set(GUI_IDENTIFIER org.${_EXE_TARGET})
	set(APPNAME ${_EXE_APPNAME})
	set(VERSION ${_EXE_VERSION})
	set(VERSION_CODE ${_EXE_VERSION_CODE})
	if (VERBOSE)
		message(STATUS "Prepare android build with settings:")
		message(STATUS "- APPNAME:        '${APPNAME}'")
		message(STATUS "- VERSION:        '${VERSION}'")
		message(STATUS "- VERSION_CODE:   '${VERSION_CODE}'")
		message(STATUS "- GUI_IDENTIFIER: '${GUI_IDENTIFIER}'")
	endif()

	set(CPACK_COMPONENT_${_EXE_TARGET}_DISPLAY_NAME ${APPNAME})

	if (ANDROID)
		add_library(${_EXE_TARGET} SHARED ${_EXE_SRCS})
		cp_android_prepare(${_EXE_TARGET} ${APPNAME} ${VERSION} ${VERSION_CODE})
	else()
		if (WINDOWED)
			if (WINDOWS)
				configure_file(${ROOT_DIR}/src/project.rc.in ${PROJECT_BINARY_DIR}/project.rc)
				list(APPEND _EXE_SRCS ${PROJECT_BINARY_DIR}/project.rc)
				add_executable(${_EXE_TARGET} WIN32 ${_EXE_SRCS})
			elseif (DARWIN OR IOS)
				add_executable(${_EXE_TARGET} MACOSX_BUNDLE ${_EXE_SRCS})
				cp_osx_add_target_properties(${_EXE_TARGET})
			else()
				add_executable(${_EXE_TARGET} ${_EXE_SRCS})
			endif()
		else()
			add_executable(${_EXE_TARGET} ${_EXE_SRCS})
		endif()
	endif()

	if (NACL)
		set_target_properties(${_EXE_TARGET} PROPERTIES PROFILING_POSTFIX .pexe)
		set_target_properties(${_EXE_TARGET} PROPERTIES RELEASE_POSTFIX .pexe)
		set_target_properties(${_EXE_TARGET} PROPERTIES DEBUG_POSTFIX .pexe)
	elseif (EMSCRIPTEN)
		set_target_properties(${_EXE_TARGET} PROPERTIES PROFILING_POSTFIX .html)
		set_target_properties(${_EXE_TARGET} PROPERTIES RELEASE_POSTFIX .html)
		set_target_properties(${_EXE_TARGET} PROPERTIES DEBUG_POSTFIX .html)
		configure_file(${ROOT_DIR}/contrib/installer/html5/shell.html.in ${CMAKE_CURRENT_BINARY_DIR}/shell.html @ONLY)
		set_target_properties(${_EXE_TARGET} PROPERTIES LINK_FLAGS "--preload-file ${ROOT_DIR}/base/${_EXE_TARGET}@/ --shell-file ${CMAKE_CURRENT_BINARY_DIR}/shell.html")
	endif()

	set_target_properties(${_EXE_TARGET} PROPERTIES FOLDER ${_EXE_TARGET})
	string(REGEX REPLACE "tests_" "" BASEDIR ${_EXE_TARGET})
	# install relative to /usr/<APPNAME>
	install(DIRECTORY ${ROOT_DIR}/base/${BASEDIR} DESTINATION ${_EXE_TARGET}/base COMPONENT ${_EXE_TARGET})
	install(TARGETS ${_EXE_TARGET} DESTINATION ${_EXE_TARGET} COMPONENT ${_EXE_TARGET})
	configure_file(${ROOT_DIR}/src/game.h.in ${PROJECT_BINARY_DIR}/game.h)
	include_directories(${PROJECT_BINARY_DIR})
endmacro()
