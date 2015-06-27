# - Find Box2D
# Find the native Box2D includes and libraries
#
#  BOX2D_INCLUDE_DIR - where to find TmxParser/Config.hpp, etc.
#  BOX2D_LIBRARIES   - List of libraries when using libTmxParser.
#  BOX2D_FOUND       - True if libTmxParser found.

if(BOX2D_INCLUDE_DIR)
  # Already in cache, be silent
  set(BOX2D_FIND_QUIETLY TRUE)
endif(BOX2D_INCLUDE_DIR)

find_path(BOX2D_INCLUDE_DIR Box2D.h
  PATH_SUFFIXES include include/Box2D
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw          # Fink
  /opt/local   # DarwinPorts
  /opt/csw     # Blastwave
  /opt
  ${BOX2DDIR}
  $ENV{BOX2DDIR})

find_library(BOX2D_LIBRARY_DEBUG
  Box2D-d
  PATH_SUFFIXES lib64 lib
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw          # Fink
  /opt/local   # DarwinPorts
  /opt/csw     # Blastwave
  /opt
  ${BOX2DDIR}
  $ENV{BOX2DDIR})

find_library(BOX2D_LIBRARY_RELEASE
  Box2D
  PATH_SUFFIXES lib64 lib
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw          # Fink
  /opt/local   # DarwinPorts
  /opt/csw     # Blastwave
  /opt
  ${BOX2DDIR}
  $ENV{BOX2DDIR})

if(BOX2D_LIBRARY_DEBUG OR BOX2D_LIBRARY_RELEASE)
  # Library found
  set(BOX2D_FOUND TRUE)

  # If both were found, set BOX2D_LIBRARY to the release version
  if(BOX2D_LIBRARY_DEBUG AND BOX2D_LIBRARY_RELEASE)
    set(BOX2D_LIBRARY ${BOX2D_LIBRARY_RELEASE})
  endif()

  if(BOX2D_LIBRARY_DEBUG AND NOT BOX2D_LIBRARY_RELEASE)
    set(BOX2D_LIBRARY ${BOX2D_LIBRARY_DEBUG})
  endif()

  if(NOT BOX2D_LIBRARY_DEBUG AND BOX2D_LIBRARY_RELEASE)
    set(BOX2D_LIBRARY ${BOX2D_LIBRARY_RELEASE})
  endif()
else()
  set(BOX2D_FOUND FALSE)
endif()

# Handle the QUIETLY and REQUIRED arguments and set SNDFILE_FOUND to TRUE if
# all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BOX2D DEFAULT_MSG BOX2D_LIBRARY BOX2D_INCLUDE_DIR)

if(BOX2D_FOUND)
  set(BOX2D_LIBRARIES ${BOX2D_LIBRARY})
else(BOX2D_FOUND)
  set(BOX2D_LIBRARIES)
endif(BOX2D_FOUND)

mark_as_advanced(BOX2D_INCLUDE_DIR
  BOX2D_LIBRARY
  BOX2D_LIBRARY_RELEASE
  BOX2D_LIBRARY_DEBUG)
