#ifndef LEVEL_H
#define LEVEL_H

#include "list.h"

#define LEVELCOUNT 20

#define L_EDITOR   -1

#define TL_NONE     0
#define TL_WALL     1
#define TL_BOXPOST  2
#define TL_HOLE     3
#define TL_PORTAL   4

#define TILE_COUNT  4

#define TILE_GET(x,y) (((x)>=0&&(y)>=0) ? level.tilemap[((y)>>5)*1000 + ((x)>>5)] : TL_WALL)
#define TILE_SET(x,y,t) (level.tilemap[((y)>>5)*1000 + ((x)>>5)] = t)
#define IS_SNAPPED(x,y) (((x & 31)==0) && ((y & 31)==0))

typedef struct sCamera {
    int scx;
    int scy;
} camera_t;

// Os níveis têm um tamanho fixo de 32000x32000 pixels (1000x1000 tiles)
// Acredito que ninguém nunca chegue a usar isso tudo.
// Obs.: Por causa disso os campos width e height acabaram se tornando inúteis.
typedef struct sLevel {
    int num;
    unsigned tilemap[1000 * 1000];
    unsigned width;
    unsigned height;
    unsigned timer;
    int boxes;
    camera_t cam;
} level_t;

// Metadados de um nível. Usados somente pelo editor de níveis.
typedef struct sLevelMeta {
    char filename[260];
    size_t filesize;
    char title[16];
    char author[16];
    unsigned ctime;
    unsigned hrand;
} levelmeta_t;

extern level_t level;

int loadLevel(int num);
int loadLevelFile(const char* path);

void newLevel();

int saveLevel(levelmeta_t* meta);

const list_t* getUserLevelList(int rebuild);
void sortUserLevelList(cmp comparator);

#endif // LEVEL_H