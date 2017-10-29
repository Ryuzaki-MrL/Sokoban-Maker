#include <stdio.h>

#include "sprite.h"

static sprite_t nullspr = { .fname = "", .frames = 0, .width = 0, .height = 0 };
static sprite_t sprites[SPRITE_COUNT] = {
    { "robot_h.png", 4, 32, 32 },
    { "box.png",     2, 32, 32 },
    { "robot_d.png", 4, 32, 32 },
    { "robot_u.png", 4, 32, 32 }
};

void loadSprites() {
    int i;
    char path[260] = "romfs:/";
    for (i = 0; i < SPRITE_COUNT; i++) {
        strcpy(path+7, sprites[i].fname);
        pp2d_load_texture_png(i, path);
    }
}

const sprite_t* getSprite(size_t id) {
    return (id >= SPRITE_COUNT) ? &nullspr : &sprites[id];
}
