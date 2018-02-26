#include <string.h>

#include "entity.h"
#include "sprite.h"
#include "util.h"

static entity_t entities[ENT_COUNT * ENT_MAX] = { { 0 } };
static entity_t entitiestmp[ENT_COUNT * ENT_MAX] = { { 0 } };
static int entsize[ENT_COUNT] = { 0 };

static int entityFinderPos(entity_t* ent, int* args) {
    const sprite_t* spr = getSprite(ent->sprite);
    if (!ent || !spr) return 0;
    return rectangleCollision(args[0], args[1], args[2], args[3], ent->x, ent->y, spr->width, spr->height);
}

entity_t* entityAdd(int x, int y, int id) {
    static int uid_ref = 0;
    if (entsize[id] >= ENT_MAX) return NULL;
    entity_t* ent = entities + id * ENT_MAX + entsize[id];
    ent->id = id;
    ent->uid = ++uid_ref;
    ent->x = ent->xstart = x;
    ent->y = ent->ystart = y;
    ent->sprite = id;
    ent->visible = 1;
    entsize[id]++;
    return ent;
}

int entityDestroy(int uid) {
    int i, j;
    for (i = 0; i < ENT_COUNT; i++) {
        for (j = 0; j < entsize[i]; j++) {
            if (entities[i * ENT_MAX + j].uid == uid) {
                memcpy(&entities[i * ENT_MAX + j], &entities[i * ENT_MAX + entsize[i] - 1], sizeof(entity_t));
                entsize[i]--;
                return 1;
            }
        }
    }
    return 0;
}

int entityDestroyPos(int x, int y, int w, int h, int id) {
    int params[] = { x, y, w, h };
    int i;
    for (i = 0; i < entsize[id]; i++) {
        if (entityFinderPos(&entities[id * ENT_MAX + i], params)) {
            memcpy(&entities[id * ENT_MAX + i], &entities[id * ENT_MAX + entsize[id] - 1], sizeof(entity_t));
            entsize[id]--;
            return 1;
        }
    }
    return 0;
}

void entityDestroyAll() {
    int i;
    for (i = 0; i < ENT_COUNT; i++) {
        entsize[i] = 0;
    }
}

entity_t* entityCollision(int x, int y, int w, int h, int id) {
    int params[] = { x, y, w, h };
    int i, j;
    for (i = (id==ENT_ANY) ? 0 : id; i < ENT_COUNT; i++) {
        for (j = 0; j < entsize[i]; j++) {
            if (entityFinderPos(&entities[id * ENT_MAX + j], params)) {
                return &entities[id * ENT_MAX + j];
            }
        }
        if (id != ENT_ANY) break;
    }
    return NULL;
}

void entityForeach(bnd function, int id) {
    int i, j;
    for (i = (id==ENT_ANY) ? 0 : id; i < ENT_COUNT; i++) {
        for (j = 0; j < entsize[i]; j++) {
            function(&entities[i * ENT_MAX + j]);
        }
        if (id != ENT_ANY) break;
    }
}

int getEntityCount(int id) {
    int t = 0;
    if (id < 0) {
        int i;
        for (i = 0; i < ENT_COUNT; i++) {
            t += entsize[i];
        }
    } else {
        t = entsize[id];
    }
    return t;
}

void entitySaveState() {
    memcpy(entitiestmp, entities, sizeof(entities));
}

void entityLoadState() {
    memcpy(entities, entitiestmp, sizeof(entities));
}
