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
FetchContent_MakeAvailable(SDL2_mixer)

FetchContent_Declare(
	SDL2_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
    GIT_TAG release-2.8.5
	GIT_SUBMODULES_RECURSE ON
)
set(SDL2IMAGE_AVIF OFF)
FetchContent_MakeAvailable(SDL2_image)

FetchContent_Declare(
    SDL2_net
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_net.git
    GIT_TAG release-2.2.0
)
FetchContent_MakeAvailable(SDL2_net)

FetchContent_Declare(
    Box2D
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG v2.4.2
)
FetchContent_MakeAvailable(Box2D)
