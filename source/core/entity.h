#ifndef ENTITY_H
#define ENTITY_H

#include "level.h"

#define ENT_ANY        -2
#define ENT_NONE       -1
#define ENT_ROBOT       0
#define ENT_BOX         1

#define ENT_COUNT       2

// Kinda useless struct, since there are only boxes.
typedef struct sEntity {
    int id;
    int uid;
    int x;
    int y;
    int xdir;
    int ydir;
    int xstart;
    int ystart;
    int sprite;
    int moving;
    int speed;
    int visible;
    float anispd;
    float frame;
} entity_t;

entity_t* entityAdd(int x, int y, int id);

int entityDestroy(int uid);
int entityDestroyPos(int x, int y, int w, int h, int id);
void entityDestroyAll();

entity_t* entityCollision(int x, int y, int w, int h, int id);

const list_t* getEntityList(int id);
int getEntityCount(int id);

void entitySaveState();
void entityLoadState();

#endif // ENTITY_H
