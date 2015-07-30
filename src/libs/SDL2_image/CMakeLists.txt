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
if (DARWIN)
	list(APPEND SRCS
		IMG_UIImage.m
		IMG_ImageIO.m
	)
endif()
cp_add_library(
	LIB SDL2_image
	SRCS ${SRCS}
	CFLAGS "-Wno-cast-qual -Wno-attributes -Wno-unused-parameter -Wno-sign-compare"
	DEFINES "-DLOAD_PNG -DPNG_NO_CONFIG_H")
# TODO
#-O3 -fstrict-aliasing -fprefetch-loop-arrays
if (NOT SDL2_IMAGE_FOUND)
	include_directories("libpng-1.6.2")
	target_link_libraries(SDL2_image ${SDL2_LIBRARIES} ${ZLIB_LIBRARIES})
	include_directories(${SDL2_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
endif()
