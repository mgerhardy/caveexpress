#pragma once

namespace Constant {
const int FPS_SERVER = 10;
const int FPS_PHYSICS = 60;
const int FPS_CLIENT = FPS_PHYSICS;
const float DELTA_PHYSICS_MILLIS = 1000.0f / (float)FPS_PHYSICS;
const float DELTA_PHYSICS_SECONDS = 1.0f / (float)FPS_PHYSICS;
}
