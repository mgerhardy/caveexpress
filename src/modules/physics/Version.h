#pragma once

#include "cp-config.h"

#define CP_BOX2D_VERSIONNUM(major, minor, patch) ((major) * 1000000 + (minor) * 1000 + (patch))

#define CP_BOX2D_VERSIONNUM_MAJOR(version) ((version) / 1000000)
#define CP_BOX2D_VERSIONNUM_MINOR(version) (((version) / 1000) % 1000)
#define CP_BOX2D_VERSIONNUM_MICRO(version) ((version) % 1000)

#define CP_BOX2D_VERSION CP_BOX2D_VERSIONNUM(CP_BOX2D_MAJOR_VERSION, CP_BOX2D_MINOR_VERSION, CP_BOX2D_MICRO_VERSION)

#define CP_BOX2D_VERSION_ATLEAST(X, Y, Z) (CP_BOX2D_VERSION >= CP_BOX2D_VERSIONNUM(X, Y, Z))

#ifndef CP_BOX2D_MAJOR
#define CP_BOX2D_MAJOR CP_BOX2D_MAJOR_VERSION
#endif
