#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "draw.h"
#include "robot.h"
#include "util.h"

level_t level;

static list_t userlevels = { .first = NULL, .last = NULL, .size = 0 };

#pragma pack(push, 1)

struct sLevelHeader {
    char magic[4];
    uint8_t version;
    uint8_t bg;
    uint16_t width;
    uint16_t height;
    uint16_t robotx;
    uint16_t roboty;
    uint16_t objcount;
    char title[16];
    char author[16];
};

struct sTile {
    uint32_t count;
    uint16_t tile;
};

struct sObj {
    uint16_t id;
    uint16_t x;
    uint16_t y;
    uint16_t xtarg;
    uint16_t ytarg;
};

#pragma pack(pop)

static int loadLevelAux(FILE* lvl, int num) {
    // Carregar cabeçalho do nível
    struct sLevelHeader levelheader;
    if (!lvl || !fread(&levelheader, 1, sizeof(levelheader), lvl)) {
        fclose(lvl);
        return 0;
    }

    // Limpar o nível anterior
    newLevel();

    int i, j;

    // Carregar tiles
    struct sTile tiledata;
    for (i = 0; i < 1000 * 1000; i += tiledata.count) {
        if (!fread(&tiledata, sizeof(tiledata), 1, lvl))
            break;
        for (j = i; j < i+tiledata.count; j++)
            level.tilemap[j] = tiledata.tile;
    }

    // Carregar objetos (neste caso, as caixas)
    struct sObj objdata;
    for (i = 0; i < levelheader.objcount; i++) {
        if (
            !fread(&objdata, sizeof(objdata), 1, lvl) ||
            !entityAdd(objdata.x, objdata.y, objdata.id)
        ) break;
    }
    fclose(lvl);

    // Carregar robô
    initRobot(levelheader.robotx, levelheader.roboty);

    level.num = num;
    level.boxes = levelheader.objcount;

    return 1;
}

int loadLevel(int num) {
    char fname[260];
    sprintf(fname, "levels/%03d.lvl", num);
    FILE* lvl = fopen(fname, "rb");
    return loadLevelAux(lvl, num);
}

int loadLevelFile(const char* path) {
    char fname[260];
    sprintf(fname, "%s/%s", getLevelPath(), path);
    FILE* lvl = fopen(fname, "rb");
    return loadLevelAux(lvl, -1);
}

void newLevel() {
    entityDestroyAll();
    memset(&level, 0, sizeof(level));
}

int saveLevel(levelmeta_t* meta) {
    if (!meta) return 0;

    // Gerar o arquivo de saída
    char fname[260];
    sprintf(fname, "%s/%s", getLevelPath(), meta->filename);
    FILE* lvl = fopen(fname, "wb");
    if (!lvl) return 0;

    // Salvar o cabeçalho
    struct sLevelHeader levelheader;
    memcpy(levelheader.magic, "RBLV", 4);
    levelheader.version = 1;
    levelheader.bg = 1;
    levelheader.width = level.width;
    levelheader.height = level.height;
    levelheader.robotx = getRobotX();
    levelheader.roboty = getRobotY();
    levelheader.objcount = getEntityCount(ENT_ANY) - 1;
    strcpy(levelheader.title, meta->title);
    strcpy(levelheader.author, meta->author);
    fwrite(&levelheader, 1, sizeof(levelheader), lvl);

    int i;

    // Salvar tiles
    struct sTile tiledata = { .tile = 0, .count = 0 };
    for (i = 0; i < 1000 * 1000; i++) {
        if (level.tilemap[i] == tiledata.tile) {
            tiledata.count++;
        } else {
            fwrite(&tiledata, 1, sizeof(tiledata), lvl);
            tiledata.tile = level.tilemap[i];
            tiledata.count = 1;
        }
    }
    fwrite(&tiledata, 1, sizeof(tiledata), lvl);

    // Salvar objetos (caixas)
    // TODO: encontrar uma solução mais elegante que não
    // envolva função dentro de função
    struct sObj auxobj;
    void saveLevelObject(void* data) {
        entity_t* ent = (entity_t*)data;
        auxobj.id = ent->id;
        auxobj.x = ent->x;
        auxobj.y = ent->y;
        fwrite(&auxobj, 1, sizeof(auxobj), lvl);
    }
    foreach(getEntityList(ENT_BOX), saveLevelObject);
    fclose(lvl);

    return 1;
}

// Função que gera a lista de níveis feitos pelo usuário
const list_t* getUserLevelList(int rebuild) {
    if (userlevels.size > 0 && !rebuild) {
        return &userlevels;
    } else {
        clearList(&userlevels);
    }

    // Escanear o diretório de níveis
    struct dirent* ent;
    DIR* dir = opendir(getLevelPath());
    int i = 0;
    char fname[260];

    while(ent = readdir(dir)) {
        struct sLevelHeader header;
        levelmeta_t* meta = (levelmeta_t*)calloc(1, sizeof(levelmeta_t));
        if (!meta) break;

        // Checa se o arquivo existe e se de fato é um nível
        // (todo nível começa com a sequência "RBLV")
        strcpy(meta->filename, ent->d_name);
        sprintf(fname, "%s/%s", getLevelPath(), meta->filename);
        FILE* file = fopen(fname, "rb");
        if (
            !file || fread(&header, 1, sizeof(header), file)!=sizeof(header) ||
            strncmp(header.magic, "RBLV", 4)!=0
        ) {
            fclose(file);
            continue;
        }

        // Título e autor
        header.title[15] = header.author[15] = '\0';
        strcpy(meta->title, header.title);
        strcpy(meta->author, header.author);

        // Tamanho do arquivo
        fseek(file, 0, SEEK_END);
        meta->filesize = ftell(file);
        fclose(file);

        insertAt(&userlevels, -1, meta);
    }
    closedir(dir);

    return &userlevels;
}

void sortUserLevelList(cmp comparator) {
    sortList(&userlevels, comparator);
}
