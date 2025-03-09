set(SOURCES
	stb_image_write.h
	stb_image.h
	stb_rect_pack.h
	stb_image_resize2.h
	TextureAtlas.cpp
	pugiconfig.hpp
	pugixml.cpp
	pugixml.hpp
)

add_executable(textureatlas ${SOURCES})
target_compile_definitions(textureatlas PRIVATE -DPUGIXML_NO_EXCEPTIONS)
