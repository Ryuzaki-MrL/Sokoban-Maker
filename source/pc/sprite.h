#ifndef SPRITE_H
#define SPRITE_H

#include <allegro5/allegro.h>

#define SPR_NONE       -1
#define SPR_BOX         1
#define SPR_ROBOT_HOR   0
#define SPR_ROBOT_DOWN  2
#define SPR_ROBOT_UP    3

#define SPRITE_COUNT    (SPR_ROBOT_UP + 1)

typedef struct sSprite {
    const char* fname;
    ALLEGRO_BITMAP* bitmap;
    size_t frames;
    size_t width;
    size_t height;
} sprite_t;

void loadSprites();
void freeSprites();

const sprite_t* getSprite(size_t id);

#endif // SPRITE_H