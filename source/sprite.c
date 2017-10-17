#include <stdio.h>

#include "sprite.h"

static sprite_t nullspr = { .fname = "", .bitmap = NULL, .frames = 0, .width = 0, .height = 0 };
static sprite_t sprites[SPRITE_COUNT] = {
    { "robot_h.png", NULL, 4, 32, 32 },
    { "box.png",     NULL, 2, 32, 32 },
    { "robot_d.png", NULL, 4, 32, 32 },
    { "robot_u.png", NULL, 4, 32, 32 }
};

void loadSprites() {
    int i;
    char fname[260] = "sprites/";
    for (i = 0; i < SPRITE_COUNT; i++) {
        strcpy(fname+8, sprites[i].fname);
        sprites[i].bitmap = al_load_bitmap(fname);
    }
}

void freeSprites() {
    int i;
    for (i = 0; i < SPRITE_COUNT; i++) {
        al_destroy_bitmap(sprites[i].bitmap);
    }
}

const sprite_t* getSprite(size_t id) {
    return (id >= SPRITE_COUNT) ? &nullspr : &sprites[id];
}
