package_global(TinyXML2)

if (NOT TINYXML2_FOUND)
set(SRCS tinyxml2.cpp)
add_library(tinyxml2 ${SRCS})
set_target_properties(tinyxml2 PROPERTIES FOLDER tinyxml2)

set(TINYXML2_LIBRARIES "tinyxml2")
set(TINYXML2_INCLUDE_DIRS "${ROOT_DIR}/src/libs/tinyxml2")
var_global(TINYXML2_LIBRARIES)
var_global(TINYXML2_INCLUDE_DIRS)

endif()
