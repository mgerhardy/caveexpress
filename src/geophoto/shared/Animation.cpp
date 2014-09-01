#include "Animation.h"

Animation::AnimationMap Animation::_animations;
uint8_t Animation::_cnt;

Animation Animation::ANIMATION_NONE("");
// walking to the left
Animation Animation::ANIMATION_WALK_LEFT("walk-left");
// walking to the right
Animation Animation::ANIMATION_WALK_RIGHT("walk-right");
// standing and waiting for a taxi
Animation Animation::ANIMATION_IDLE("idle");
// falling into the water
Animation Animation::ANIMATION_FALLING("falling");
// swimming towards the player to get rescued
Animation Animation::ANIMATION_SWIMMING_LEFT("swimming-left");
Animation Animation::ANIMATION_SWIMMING_RIGHT("swimming-right");
// struggle until getting rescued
Animation Animation::ANIMATION_SWIMMING_IDLE("swimming-idle");
// flying taxi animation
Animation Animation::ANIMATION_FLYING("flying");
// flying npc animations
Animation Animation::ANIMATION_FLYING_LEFT("flying-left");
Animation Animation::ANIMATION_FLYING_RIGHT("flying-right");
// aggressive npc attacks
Animation Animation::ANIMATION_ATTACK_INIT_LEFT("attack-init-left", false);
Animation Animation::ANIMATION_ATTACK_INIT_RIGHT("attack-init-right", false);
Animation Animation::ANIMATION_ATTACK_LEFT("attack-left");
Animation Animation::ANIMATION_ATTACK_RIGHT("attack-right");
Animation Animation::ANIMATION_FALLING_LEFT("falling-left");
Animation Animation::ANIMATION_FALLING_RIGHT("falling-right");
Animation Animation::ANIMATION_IDLE_LEFT("idle-left");
Animation Animation::ANIMATION_IDLE_RIGHT("idle-right");
Animation Animation::ANIMATION_TURN_LEFT("turn-left", false);
Animation Animation::ANIMATION_TURN_RIGHT("turn-right", false);
Animation Animation::ANIMATION_KNOCKOUT_LEFT("knockout-left", false);
Animation Animation::ANIMATION_KNOCKOUT_RIGHT("knockout-right", false);
Animation Animation::ANIMATION_DAZED("dazed", false);
Animation Animation::ANIMATION_DAZED_LEFT("dazed-left");
Animation Animation::ANIMATION_DAZED_RIGHT("dazed-right");
Animation Animation::ANIMATION_WAKEUP_LEFT("wakeup-left", false);
Animation Animation::ANIMATION_WAKEUP_RIGHT("wakeup-right", false);
Animation Animation::ANIMATION_ACTIVE("active");
Animation Animation::ANIMATION_EXPLODE("explode");
Animation Animation::ANIMATION_ROTATE("rotate");
