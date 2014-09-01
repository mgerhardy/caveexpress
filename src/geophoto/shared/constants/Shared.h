#pragma once

namespace Constant {
const int FPS_SERVER = 10;
const int FPS_PHYSICS = 60;
const int FPS_CLIENT = FPS_PHYSICS;
const int SCALE_FACTOR = 100;
const float DELTA_PHYSICS_MILLIS = 1000.0f / (float)FPS_PHYSICS;
const float DELTA_PHYSICS_SECONDS = 1.0f / (float)FPS_PHYSICS;
const int AMOUNT_OF_FRUITS_TO_GET_A_NEW_LIFE = 4;
}
