set(SRCS
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
if (DARWIN OR IOS)
	list(APPEND SRCS
		IMG_ImageIO.m
	)
else()
	list(APPEND SRCS
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
	)
endif()
cp_add_library(
	LIB SDL2_image
	SRCS ${SRCS}
	CFLAGS "-Wno-cast-qual -Wno-attributes -Wno-unused-parameter -Wno-sign-compare"
	DEFINES LOAD_PNG PNG_NO_CONFIG_H)
# TODO
#-O3 -fstrict-aliasing -fprefetch-loop-arrays
if (NOT SDL2_IMAGE_FOUND)
	include_directories("libpng-1.6.2")
	set(LIBS "")
	list(APPEND LIBS ${SDL2_LIBRARIES} ${ZLIB_LIBRARIES})
	if (IOS OR DARWIN)
		if (IOS)
			set(FRAMEWORKS Foundation CoreGraphics MobileCoreServices ImageIO UIKit)
		else()
			set(FRAMEWORKS Foundation CoreServices ApplicationServices)
		endif()
		foreach (FW ${FRAMEWORKS})
			find_library(${FW}_FRAMEWORK ${FW})
			if (${FW}_FRAMEWORK-NOTFOUND)
				message(STATUS "Could not find ${FW} framework")
			else()
				message(STATUS "Found ${FW} => ${${FW}_FRAMEWORK}")
				list(APPEND LIBS ${${FW}_FRAMEWORK})
			endif()
		endforeach()
	endif()
	target_link_libraries(SDL2_image ${LIBS})
	include_directories(${SDL2_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
endif()
