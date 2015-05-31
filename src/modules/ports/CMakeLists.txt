if(WIN32)
	set(ports_SRCS Windows.cpp project.rc)
	configure_file(project.rc.in project.rc)
elseif(APPLE)
	set(ports_SRCS Darwin.cpp Cocoa.mm)
elseif(UNIX)
	set(ports_SRCS Unix.cpp)
	add_definitions(-D_GNU_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE)
endif()

#ports/HTML5.cpp
#ports/IOS.cpp
#ports/Android.cpp
#ports/NaCl.cpp

add_library(ports ${ports_SRCS})
set_target_properties(ports PROPERTIES FOLDER ports)
target_link_libraries(ports libs)
