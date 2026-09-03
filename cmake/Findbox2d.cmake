# Find Box2D 2.4.x or 3.x without a hard version pin.
# Prefer CMake package config (box2d::box2d), then pkg-config / manual search.
# Sets: box2d_FOUND, box2d_VERSION, CP_BOX2D_MAJOR/MINOR/MICRO, and target box2d.

include(${CMAKE_CURRENT_LIST_DIR}/box2d_version.cmake)

function(_cp_box2d_alias)
	if (TARGET box2d::box2d AND NOT TARGET box2d)
		add_library(box2d INTERFACE)
		target_link_libraries(box2d INTERFACE box2d::box2d)
	endif()
endfunction()

if (TARGET box2d::box2d OR TARGET box2d)
	_cp_box2d_alias()
	cp_box2d_collect_include_dirs(_cp_b2_inc)
	set(_cp_b2_hint "")
	if (DEFINED box2d_VERSION AND box2d_VERSION)
		set(_cp_b2_hint "${box2d_VERSION}")
	elseif (DEFINED BOX2D_VERSION AND BOX2D_VERSION)
		set(_cp_b2_hint "${BOX2D_VERSION}")
	endif()
	cp_box2d_detect_version(INCLUDE_DIRS ${_cp_b2_inc} HINT "${_cp_b2_hint}")
	return()
endif()

set(_CP_BOX2D_VERSION "")

# Config-mode first (Debian 2.4 and upstream 3.x both export box2d::box2d).
find_package(box2d QUIET CONFIG)
if (box2d_FOUND AND TARGET box2d::box2d)
	if (DEFINED box2d_VERSION AND box2d_VERSION)
		set(_CP_BOX2D_VERSION "${box2d_VERSION}")
	elseif (DEFINED PACKAGE_VERSION AND PACKAGE_VERSION)
		set(_CP_BOX2D_VERSION "${PACKAGE_VERSION}")
	endif()
else()
	include(${ROOT_DIR}/cmake/macros.cmake)
	cp_find(box2d box2d.h box2d "")
	if (BOX2D_FOUND)
		find_package(PkgConfig QUIET)
		if (PKG_CONFIG_FOUND)
			pkg_check_modules(_CP_BOX2D_PC QUIET box2d)
			if (_CP_BOX2D_PC_VERSION)
				set(_CP_BOX2D_VERSION "${_CP_BOX2D_PC_VERSION}")
			endif()
		endif()
	endif()
endif()

if (NOT box2d_FOUND AND NOT BOX2D_FOUND AND NOT TARGET box2d AND NOT TARGET box2d::box2d)
	set(box2d_FOUND FALSE)
	return()
endif()

set(box2d_FOUND TRUE)
_cp_box2d_alias()
cp_box2d_collect_include_dirs(_cp_b2_inc)
cp_box2d_detect_version(INCLUDE_DIRS ${_cp_b2_inc} HINT "${_CP_BOX2D_VERSION}")
