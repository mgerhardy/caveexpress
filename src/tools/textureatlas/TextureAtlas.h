#pragma once

// disable cast-qual warnings for this header
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
// disable -Wmissing-field-initializers
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#define STB_RECT_PACK_IMPLEMENTATION
#include "external/stb_rect_pack.h"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "external/stb_image_resize2.h"

#include "external/murmur.h"
#include "external/pugixml.hpp"

#pragma GCC diagnostic pop
