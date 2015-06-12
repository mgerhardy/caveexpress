#include "CaveExpressAnimation.h"

namespace caveexpress {

namespace Animations {
// walking to the left
Animation ANIMATION_WALK_LEFT("walk-left");
// walking to the right
Animation ANIMATION_WALK_RIGHT("walk-right");
// standing and waiting for a taxi
Animation ANIMATION_IDLE("idle");
// falling into the water
Animation ANIMATION_FALLING("falling");
// swimming towards the player to get rescued
Animation ANIMATION_SWIMMING_LEFT("swimming-left");
Animation ANIMATION_SWIMMING_RIGHT("swimming-right");
// struggle until getting rescued
Animation ANIMATION_SWIMMING_IDLE("swimming-idle");
// flying taxi animation
Animation ANIMATION_FLYING("flying");
Animation ANIMATION_CRASHED("crashed");
// flying npc animations
Animation ANIMATION_FLYING_LEFT("flying-left");
Animation ANIMATION_FLYING_RIGHT("flying-right");
// aggressive npc attacks
Animation ANIMATION_ATTACK_INIT_LEFT("attack-init-left", false);
Animation ANIMATION_ATTACK_INIT_RIGHT("attack-init-right", false);
Animation ANIMATION_ATTACK_LEFT("attack-left");
Animation ANIMATION_ATTACK_RIGHT("attack-right");
Animation ANIMATION_FALLING_LEFT("falling-left");
Animation ANIMATION_FALLING_RIGHT("falling-right");
Animation ANIMATION_FLYING_FALLING_LEFT("falling-left", false);
Animation ANIMATION_FLYING_FALLING_RIGHT("falling-right", false);
Animation ANIMATION_IDLE_LEFT("idle-left");
Animation ANIMATION_IDLE_RIGHT("idle-right");
Animation ANIMATION_TURN_LEFT("turn-left", false);
Animation ANIMATION_TURN_RIGHT("turn-right", false);
Animation ANIMATION_KNOCKOUT_LEFT("knockout-left", false);
Animation ANIMATION_KNOCKOUT_RIGHT("knockout-right", false);
Animation ANIMATION_DAZED("dazed", false);
Animation ANIMATION_DAZED_LEFT("dazed-left");
Animation ANIMATION_DAZED_RIGHT("dazed-right");
Animation ANIMATION_WAKEUP_LEFT("wakeup-left", false);
Animation ANIMATION_WAKEUP_RIGHT("wakeup-right", false);
Animation ANIMATION_ACTIVE("active");
Animation ANIMATION_EXPLODE("explode");
Animation ANIMATION_ROTATE("rotate");
}

}
