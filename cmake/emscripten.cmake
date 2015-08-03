set(UNITTESTS OFF)
set(TOOLS OFF)
set(USE_BUILTIN ON)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --shell-file ${ROOT_DIR}/contrib/installer/html5/shell.html --js-library ${ROOT_DIR}/contrib/installer/html5/library.js")
