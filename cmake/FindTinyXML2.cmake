#.rst:
# FindTinyXML2
# ------------
#
# Locate tinyxml2 library
#
# This module defines
#
# ::
#
#   TINYXML2_LIBRARIES, the library to link against
#   TINYXML2_FOUND, if false, do not try to link to tinyxml2
#   TINYXML2_INCLUDE_DIRS, where to find headers.
#

find_path(TinyXML2_INCLUDE_DIR tinyxml2.h
  HINTS ENV TinyXML2_DIR
  PATH_SUFFIXES include/tinyxml2 include
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
)

find_library(TinyXML2_LIBRARY 
  NAMES tinyxml2 libtinyxml2
  HINTS ENV TinyXML2_DIR
  PATH_SUFFIXES lib
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)

set(TINYXML2_INCLUDE_DIRS "${TinyXML2_INCLUDE_DIR}")
set(TINYXML2_LIBRARIES "${TinyXML2_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TINYXML2 DEFAULT_MSG TINYXML2_LIBRARIES TINYXML2_INCLUDE_DIRS)

mark_as_advanced(TINYXML2_INCLUDE_DIRS TINYXML2_LIBRARIES TinyXML2_LIBRARY)
