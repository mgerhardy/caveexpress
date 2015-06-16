#pragma once

#include <string>

// map setting name
namespace msn {
const std::string WIDTH = "width";
const std::string HEIGHT = "height";
const std::string POINTS = "points";
const std::string TUTORIAL = "tutorial";
const std::string NPCS = "npcs";
const std::string INTROWINDOW = "introwindow";
const std::string REFERENCETIME = "referencetime";
// can be used for those wind levels where you lose if not all packages are
// delivered when you are touching the side borders
const std::string SIDEBORDERFAIL = "sideborderfail";
const std::string PACKAGE_TRANSFER_COUNT = "packagetransfercount";
const std::string NPC_TRANSFER_COUNT = "npctransfercount";
const std::string NPC_INITIAL_SPAWN_TIME = "initialspawntime";
const std::string GEYSER_INITIAL_DELAY_TIME = "geyserinitialdelay";
const std::string FLYING_NPC = "flyingnpc";
const std::string FISH_NPC = "fishnpc";
const std::string WIND = "wind";
const std::string GRAVITY = "gravity";
const std::string WATER_HEIGHT = "waterheight";
const std::string WATER_CHANGE = "waterchangespeed";
const std::string WATER_RISING_DELAY = "waterrisingdelay";
const std::string WATER_FALLING_DELAY = "waterfallingdelay";
const std::string THEME = "theme";
}

// map setting default
namespace msd {
const std::string PACKAGE_TRANSFER_COUNT = "3";
const std::string NPC_TRANSFER_COUNT = "0";
const std::string TUTORIAL = "0";
const std::string WIND = "0.0";
const std::string NPCS = "3";
const std::string FLYING_NPC = "false";
const std::string FISH_NPC = "false";
const std::string WATER_HEIGHT = "1.0";
const std::string SIDEBORDERFAIL = "false";
const std::string WATER_CHANGE = "0.0";
const std::string PLAYER_X = "2";
const std::string PLAYER_Y = "1";
const std::string WATER_RISING_DELAY = "0";
const std::string WATER_FALLING_DELAY = "0";
const std::string INTROWINDOW = "";
}

// map setting default values
namespace msdv {
const float GRAVITY = 9.81;
const int POINTS = 100;
const int REFERENCETIME = 30;
}
