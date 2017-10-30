#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "draw.h"
#include "robot.h"
#include "textbox.h"
#include "network.h"
#include "util.h"

level_t level;
static FILE* lvl = NULL;

#pragma pack(push, 1)

struct sLevelHeader {
    char magic[4];
    uint16_t bom;
    uint32_t hash;
    uint16_t robotx;
    uint16_t roboty;
    uint16_t objcount;
    char title[32];
    char author[32];
};

struct sTile {
    uint32_t count;
    uint16_t tile;
};

struct sObj {
    uint16_t id;
    uint16_t x;
    uint16_t y;
};

#pragma pack(pop)

static inline unsigned short __builtin_bswap16(unsigned short a) {
    return (a<<8)|(a>>8);
}

void newLevel() {
    entityDestroyAll();
    memset(&level, 0, sizeof(level));
}

static int loadLevelAux() {
    struct sLevelHeader levelheader;
    if (!lvl || !fread(&levelheader, 1, sizeof(levelheader), lvl)) {
        fclose(lvl);
        return 0;
    }
    if (levelheader.bom == 0xFFFE) {
        levelheader.hash = __builtin_bswap32(levelheader.hash);
        levelheader.robotx = __builtin_bswap16(levelheader.robotx);
        levelheader.roboty = __builtin_bswap16(levelheader.roboty);
        levelheader.objcount = __builtin_bswap16(levelheader.objcount);
    }

    newLevel();

    unsigned i, j;
    struct sTile tiledata;
    for (i = 0; i < TILE_ROW * TILE_ROW; i += tiledata.count) {
        if (!fread(&tiledata, sizeof(tiledata), 1, lvl))
            break;
        if (levelheader.bom == 0xFFFE) {
            tiledata.count = __builtin_bswap32(tiledata.count);
            tiledata.tile = __builtin_bswap16(tiledata.tile);
        }
        if ((i + tiledata.count) > (TILE_ROW * TILE_ROW))
            break;
        for (j = i; j < i+tiledata.count; j++)
            level.tilemap[j] = tiledata.tile;
    }

    struct sObj objdata;
    for (i = 0; i < levelheader.objcount; i++) {
        if (!fread(&objdata, sizeof(objdata), 1, lvl))
            break;
        if (levelheader.bom == 0xFFFE) {
            objdata.id = __builtin_bswap16(objdata.id);
            objdata.x = __builtin_bswap16(objdata.x);
            objdata.y = __builtin_bswap16(objdata.y);
        }
        if (!entityAdd(objdata.x, objdata.y, objdata.id))
            break;
    }
    fclose(lvl);

    initRobot(levelheader.robotx, levelheader.roboty);
    level.boxes = levelheader.objcount;

    return 1;
}

int loadLevel(int num) {
    char fname[260];
    sprintf(fname, "levels/%03d.lvl", num);
    lvl = fopen(fname, "rb");
    return loadLevelAux();
}

int loadLevelFile(const char* fname) {
    char path[260];
    sprintf(path, "%s/%s", getLevelPath(), fname);
    lvl = fopen(path, "rb");
    return loadLevelAux();
}

levelmeta_t* getLevelMeta(const char* fname) {
    struct sLevelHeader header;
    levelmeta_t* meta = (levelmeta_t*)calloc(1, sizeof(levelmeta_t));
    if (!meta) return NULL;

    char path[260];
    strcpy(meta->filename, fname);
    sprintf(path, "%s/%s", getLevelPath(), fname);
    FILE* file = fopen(path, "rb");
    if (
        !file || fread(&header, 1, sizeof(header), file)!=sizeof(header) ||
        strncmp(header.magic, "RBLV", 4)!=0
    ) {
        fclose(file);
        return NULL;
    }

    header.title[15] = header.author[15] = '\0';
    strcpy(meta->title, header.title);
    strcpy(meta->author, header.author);

    fseek(file, 0, SEEK_END);
    meta->filesize = ftell(file);
    fclose(file);

    return meta;
}

static struct sObj auxobj;
static void saveLevelObject(void* data) {
    entity_t* ent = (entity_t*)data;
    auxobj.id = ent->id;
    auxobj.x = ent->x;
    auxobj.y = ent->y;
    fwrite(&auxobj, 1, sizeof(auxobj), lvl);
}

int saveLevel(levelmeta_t* meta) {
    if (!meta) return 0;

    char fname[260];
    sprintf(fname, "%s/%s", getLevelPath(), meta->filename);
    lvl = fopen(fname, "wb");
    if (!lvl) return 0;

    struct sLevelHeader levelheader;
    memcpy(levelheader.magic, "RBLV", 4);
    levelheader.bom = 0xFEFF;
    levelheader.hash = 0;
    levelheader.robotx = getRobotX();
    levelheader.roboty = getRobotY();
    levelheader.objcount = getEntityCount(ENT_ANY) - 1;
    strcpy(levelheader.title, meta->title);
    strcpy(levelheader.author, meta->author);
    fwrite(&levelheader, 1, sizeof(levelheader), lvl);

    int i;
    struct sTile tiledata = { .tile = 0, .count = 0 };
    for (i = 0; i < TILE_ROW * TILE_ROW; i++) {
        if (level.tilemap[i] == tiledata.tile) {
            tiledata.count++;
        } else {
            fwrite(&tiledata, 1, sizeof(tiledata), lvl);
            tiledata.tile = level.tilemap[i];
            tiledata.count = 1;
        }
    }
    fwrite(&tiledata, 1, sizeof(tiledata), lvl);

    foreach(getEntityList(ENT_BOX), saveLevelObject);
    fclose(lvl);

    return 1;
}

int downloadLevel(const char* fname) {
    char url[260];
    char path[260];
    sprintf(path, "%s/%s", getLevelPath(), fname);
    sprintf(url, URL_ROOT"levels/download.php?file=%s", fname);
    return downloadFile(url, path);
}

void uploadLevel(const char* fname) {
    char path[260];
    sprintf(path, "%s/%s", getLevelPath(), fname);
    error(uploadFile(URL_ROOT"levels/upload.php", path), NULL);
}

int deleteLevel(const char* fname) {
    char path[260];
    sprintf(path, "%s/%s", getLevelPath(), fname);
    return remove(path);
}
