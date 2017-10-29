#include <string.h>

#include "sprite.h"

static const sprite_t* sprites[SPRITE_COUNT] = { &spr_robot_h, &spr_box, &spr_robot_down, &spr_robot_up };

const sprite_t* getSprite(size_t id) {
    return (id >= SPRITE_COUNT) ? NULL : sprites[id];
}
