#include "entity.h"
#include "sprite.h"
#include "util.h"

static list_t entities[ENT_COUNT] = { { 0 } };
static list_t entitiestmp[ENT_COUNT] = { { 0 } };

static int uidFinder(const void* ent, const void* args) {
    return (((const entity_t*)ent)->uid == *((int*)args));
}

static int entityFinderPos(const void* data, const void* coords) {
    entity_t* ent = (entity_t*)data;
    int* args = (int*)coords;
    const sprite_t* spr = getSprite(ent->sprite);
    if (!ent || !spr) return 0;
    return rectangleCollision(args[0], args[1], args[2], args[3], ent->x, ent->y, spr->width, spr->height);
}

entity_t* entityAdd(int x, int y, int id) {
    static int uid_ref = 0;
    entity_t* ent = (entity_t*)calloc(1, sizeof(entity_t));
    if (!ent) return NULL;
    ent->id = id;
    ent->uid = ++uid_ref;
    ent->x = ent->xstart = x;
    ent->y = ent->ystart = y;
    ent->sprite = id;
    ent->visible = 1;
    insertAt(entities+id, -1, ent);
    return ent;
}

int entityDestroy(int uid) {
    int i;
    for (i = 0; i < ENT_COUNT; i++) {
        if (removeAt(entities+i, getNodePos(entities+i, uidFinder, &uid), NULL)) {
            return 1;
        }
    }
    return 0;
}

int entityDestroyPos(int x, int y, int w, int h, int id) {
    int params[] = { x, y, w, h };
    return removeAt(entities+id, getNodePos(entities+id, entityFinderPos, params), NULL);
}

void entityDestroyAll() {
    int i;
    for (i = 0; i < ENT_COUNT; i++)
        clearList(entities + i);
}

entity_t* entityCollision(int x, int y, int w, int h, int id) {
    int params[] = { x, y, w, h };
    if (id == ENT_ANY) {
        int i;
        for (i = 0; i < ENT_COUNT; i++) {
            entity_t* found = (entity_t*)search(entities+i, entityFinderPos, params);
            if (found) return found;
        }
        return NULL;
    }
    return (entity_t*)search(entities+id, entityFinderPos, params);
}

const list_t* getEntityList(int id) {
    if (id < 0) return entities;
    return &entities[id];
}

int getEntityCount(int id) {
    int t = 0;
    if (id < 0) {
        int i;
        for (i = 0; i < ENT_COUNT; i++) {
            t += entities[i].size;
        }
    } else {
        t = entities[id].size;
    }
    return t;
}

void entitySaveState() {
    int i;
    for (i = 1; i < ENT_COUNT; i++) {
        copyList(entities+i, entitiestmp+i, sizeof(entity_t));
    }
}

void entityLoadState() {
    int i;
    for (i = 1; i < ENT_COUNT; i++) {
        copyList(entitiestmp+i, entities+i, sizeof(entity_t));
    }
}
