set(SOURCES
	external/stb_image_write.h
	external/stb_image.h
	external/stb_rect_pack.h
	external/stb_image_resize2.h
	external/pugiconfig.hpp external/pugixml.cpp external/pugixml.hpp

	TextureAtlas.cpp TextureAtlas.h
)

add_executable(textureatlas ${SOURCES})
target_compile_definitions(textureatlas PRIVATE -DPUGIXML_NO_EXCEPTIONS)
