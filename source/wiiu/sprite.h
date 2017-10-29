#ifndef SPRITE_H
#define SPRITE_H

#include <gctypes.h>
#include <stdlib.h>

#define SPR_NONE       -1
#define SPR_BOX         1
#define SPR_ROBOT_HOR   0
#define SPR_ROBOT_DOWN  2
#define SPR_ROBOT_UP    3

#define SPRITE_COUNT    (SPR_ROBOT_UP + 1)

typedef struct sSprite {
    unsigned int frames;
    unsigned int width;
    unsigned int height;
    unsigned int bytes_per_pixel;
    unsigned char pixel_data[446 * 64 * 4 + 1];
} sprite_t;

extern const sprite_t logo;
extern const sprite_t tileset;

extern const sprite_t spr_box;
extern const sprite_t spr_robot_h;
extern const sprite_t spr_robot_up;
extern const sprite_t spr_robot_down;

const sprite_t* getSprite(size_t id);

#endif // SPRITE_H