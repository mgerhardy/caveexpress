package_global(SDL2_image)

if (NOT SDL2_IMAGE_FOUND)
set(SRCS
	libpng-1.6.2/png.c
	libpng-1.6.2/pngerror.c
	libpng-1.6.2/pngget.c
	libpng-1.6.2/pngmem.c
	libpng-1.6.2/pngpread.c
	libpng-1.6.2/pngread.c
	libpng-1.6.2/pngrio.c
	libpng-1.6.2/pngrtran.c
	libpng-1.6.2/pngrutil.c
	libpng-1.6.2/pngset.c
	libpng-1.6.2/pngtrans.c
	libpng-1.6.2/pngwio.c
	libpng-1.6.2/pngwrite.c
	libpng-1.6.2/pngwtran.c
	libpng-1.6.2/pngwutil.c
	IMG_bmp.c
	IMG.c
	IMG_gif.c
	IMG_jpg.c
	IMG_lbm.c
	IMG_pcx.c
	IMG_png.c
	IMG_pnm.c
	IMG_tga.c
	IMG_tif.c
	IMG_webp.c
	IMG_xcf.c
	IMG_xpm.c
	IMG_xv.c
	)
add_library(SDL2_image ${SRCS})
set_target_properties(SDL2_image PROPERTIES FOLDER SDL2_image)
add_definitions(-DLOAD_PNG -DPNG_NO_CONFIG_H)
#IMG_UIImage.m
#IMG_ImageIO.m
#-O3 -fstrict-aliasing -fprefetch-loop-arrays
#-lz
include_directories("libpng-1.6.2")
target_link_libraries(SDL2_image ${SDL2_LIBRARY} ${ZLIB_LIBRARIES})
include_directories(${SDL2_INCLUDE_DIR})
set_target_properties(SDL2_image PROPERTIES COMPILE_FLAGS "-Wno-cast-qual -Wno-attributes -Wno-unused-parameter -Wno-sign-compare")

set(SDL2_IMAGE_LIBRARY "SDL2_image")
set(SDL2_IMAGE_INCLUDE_DIR "${ROOT_DIR}/src/libs/SDL_image")
var_global(SDL2_IMAGE_LIBRARY)
var_global(SDL2_IMAGE_INCLUDE_DIR)

endif()
