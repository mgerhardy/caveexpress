add_subdirectory(glm)
add_subdirectory(yajl)
if (UNITTESTS)
	add_subdirectory(gtest)
endif()

include(FetchContent)

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.32.0
)
FetchContent_MakeAvailable(SDL2)

FetchContent_Declare(
    SDL2_mixer
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
    GIT_TAG release-2.8.1
	GIT_SUBMODULES_RECURSE ON
)
set(SDL2MIXER_FLAC OFF)
set(SDL2MIXER_GME OFF)
set(SDL2MIXER_CMD OFF)
set(SDL2MIXER_MOD OFF)
set(SDL2MIXER_MIDI OFF)
set(SDL2MIXER_MP3 OFF)
FetchContent_MakeAvailable(SDL2_mixer)

FetchContent_Declare(
	SDL2_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
    GIT_TAG release-2.8.5
	GIT_SUBMODULES_RECURSE ON
)
set(SDL2IMAGE_AVIF OFF)
set(SDL2IMAGE_BMP OFF)
set(SDL2IMAGE_GIF OFF)
set(SDL2IMAGE_JPG OFF)
set(SDL2IMAGE_JXL OFF)
set(SDL2IMAGE_LBM OFF)
set(SDL2IMAGE_PCX OFF)
set(SDL2IMAGE_PNM OFF)
set(SDL2IMAGE_QOI OFF)
set(SDL2IMAGE_SVG OFF)
set(SDL2IMAGE_TGA OFF)
set(SDL2IMAGE_TIF OFF)
set(SDL2IMAGE_WEBP OFF)
set(SDL2IMAGE_XCF OFF)
set(SDL2IMAGE_XV OFF)
FetchContent_MakeAvailable(SDL2_image)

FetchContent_Declare(
    SDL2_net
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_net.git
    GIT_TAG release-2.2.0
)
FetchContent_MakeAvailable(SDL2_net)

FetchContent_Declare(
    box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG v2.4.2
)
set(BOX2D_BUILD_TESTBED OFF)
set(BOX2D_BUILD_UNIT_TESTS OFF)
set(BOX2D_BUILD_DOCS OFF)
FetchContent_MakeAvailable(box2d)
