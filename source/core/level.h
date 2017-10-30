#ifndef LEVEL_H
#define LEVEL_H

#include "list.h"

#define TL_NONE     0
#define TL_WALL     1
#define TL_BOXPOST  2
#define TL_HOLE     3
#define TL_PORTAL   4

#define TILE_COUNT  4

#define TILE_ROW  100

#define TILE_GET(x,y) (((x)>=0&&(y)>=0) ? level.tilemap[((y)>>5)*TILE_ROW + ((x)>>5)] : TL_WALL)
#define TILE_SET(x,y,t) (level.tilemap[((y)>>5)*TILE_ROW + ((x)>>5)] = t)
#define IS_SNAPPED(x,y) (((x & 31)==0) && ((y & 31)==0))

typedef struct sCamera {
    int scx;
    int scy;
} camera_t;

typedef struct sLevel {
    int num;
    unsigned tilemap[TILE_ROW * TILE_ROW];
    unsigned hash;
    int boxes;
    camera_t cam;
} level_t;

typedef struct sLevelMeta {
    char filename[260];
    size_t filesize;
    char title[32];
    char author[32];
    unsigned ctime;
    unsigned hrand;
    int valid;
} levelmeta_t;

extern level_t level;

void newLevel();

int loadLevel(int num);
int loadLevelFile(const char* fname);

levelmeta_t* getLevelMeta(const char* fname);

int saveLevel(levelmeta_t* meta);

int downloadLevel(const char* fname);
void uploadLevel(const char* fname);

int deleteLevel(const char* fname);

#endif // LEVEL_H