#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <jansson.h>

#include "network.h"
#include "level.h"
#include "update.h"
#include "draw.h"
#include "util.h"

static list_t levels;

static unsigned cursor = 0;
static int online = 0;
static int listmode = 0;

void openUserLevelList(int mode) {
    cursor = 0;
    online = 0;
    listmode = mode;
    state = ST_LEVELS;

    clearList(&levels);

    struct dirent* ent;
    DIR* dir = opendir(getLevelPath());
    while((ent = readdir(dir)) != NULL) {
        levelmeta_t* meta = getLevelMeta(ent->d_name);
        if ((meta == NULL) || (mode == LISTMODE_UPLOAD && !meta->valid)) continue;
        insertAt(&levels, -1, meta);
    }
    closedir(dir);
}

void openOnlineLevelList(int mode) {
    cursor = 0;
    online = 1;
    listmode = mode;
    state = ST_LEVELS;

    clearList(&levels);

    json_t* json = json_loads(httpGet(mode == LISTMODE_MYLEVELS ? URL_ROOT"levels/?mylevels" : URL_ROOT"levels/"), JSON_DECODE_ANY, NULL);
    int size = json_array_size(json);
    int i;
    for (i = 0; i < size; i++) {
        levelmeta_t* meta = (levelmeta_t*)calloc(1, sizeof(levelmeta_t));
        if (!meta) continue;

        json_t* jsonobject = json_array_get(json, i);

        strcpy(meta->filename, json_string_value(json_object_get(jsonobject, "filename")));
        strcpy(meta->title, json_string_value(json_object_get(jsonobject, "title")));
        strcpy(meta->author, json_string_value(json_object_get(jsonobject, "author")));
        meta->filesize = atoi(json_string_value(json_object_get(jsonobject, "filesize")));

        insertAt(&levels, -1, meta);
    }
    json_decref(json);
}

static int sortLevelByTitle(const void* data1, const void* data2) {
    levelmeta_t* m1 = (levelmeta_t*)data1;
    levelmeta_t* m2 = (levelmeta_t*)data2;
    return (strcasecmp(m1->title, m2->title)<=0);
}

static int sortLevelByAuthor(const void* data1, const void* data2) {
    levelmeta_t* m1 = (levelmeta_t*)data1;
    levelmeta_t* m2 = (levelmeta_t*)data2;
    return (strcasecmp(m1->author, m2->author)<=0);
}

static void deleteLevelFromList(int option) {
    if (option) {
        levelmeta_t* meta = (levelmeta_t*)getNodeData(&levels, cursor);
        if (meta) {
            deleteLevel(meta->filename);
            if (online) {
                openOnlineLevelList(listmode);
            } else {
                openUserLevelList(listmode);
            }
        }
    }
}

void updateLevelList() {
    if (isKeyHeld(KEY_EXTRA)) {
        if (isKeyDown(KEY_DOWN)) {
            sortList(&levels, sortLevelByAuthor);
        } else if (isKeyDown(KEY_UP)) {
            sortList(&levels, sortLevelByTitle);
        }
    } else if (isKeyDown(KEY_DOWN) && cursor < levels.size-1) {
        cursor++;
    } else if (isKeyDown(KEY_UP) && cursor > 0) {
        cursor--;
    }
    if (isKeyDown(KEY_CANCEL))
        state = ST_TITLE;
    if (isKeyDown(KEY_CONFIRM)) {
        levelmeta_t* meta = (levelmeta_t*)getNodeData(&levels, cursor);
        if (meta) {
            if (online) {
                if (listmode != LISTMODE_MYLEVELS) {
                    error(getMessage(MSG_DOWNLOADNOK - downloadLevel(meta->filename)), NULL);
                }
            } else {
                if (listmode == LISTMODE_LOAD) {
                    editorStart(meta);
                } else {
                    uploadLevel(meta->filename);
                }
            }
        }
    }
    if (isKeyDown(KEY_DELETE) && (!online || listmode == LISTMODE_MYLEVELS)) {
        question(getMessage(QST_DELLEVEL), deleteLevelFromList);
    }
}

#define ENTRY_X1    (DISPLAY_WIDTH / 8)
#define ENTRY_X2    (DISPLAY_WIDTH - ENTRY_X1)
#define ENTRY_Y1    (DISPLAY_HEIGHT / 3)
#define ENTRY_Y2    (DISPLAY_HEIGHT - ENTRY_Y1)

static int i = 0;
static void drawLevelEntry(void* data) {
    levelmeta_t* m = (levelmeta_t*)data;
    int yoffs = (i++)*(ENTRY_Y2 - ENTRY_Y1 + 32) - cursor*(ENTRY_Y2 - ENTRY_Y1 + 32);
    drawRectangle(ENTRY_X1 + 8, ENTRY_Y1 + yoffs, ENTRY_X2 - 8, ENTRY_Y2 + yoffs, RGBA8(220,220,220,240), 1);
    drawText(      C_BLACK, ENTRY_X1 + 16, ENTRY_Y1 + 20 + yoffs, m->title);
    drawText(      C_BLACK, ENTRY_X1 + 16, ENTRY_Y1 + 40 + yoffs, m->author);
    drawTextFormat(C_BLACK, ENTRY_X1 + 16, ENTRY_Y1 + 60 + yoffs, "%.2f KB", m->filesize/1024.0);
    // TODO: level preview image
}

void drawLevelList() {
    drawClearColor(C_WHITE);
    drawRectangle(ENTRY_X1, ENTRY_Y1, ENTRY_X2, ENTRY_Y2, RGBA8(255,200,16,220), 1);

    if (!levels.size) {
        drawTextCenter(C_BLACK, DISPLAY_WIDTH>>1, DISPLAY_HEIGHT>>1, "¯\\_(\"-.-)_/¯");
    } else {
        i = 0;
        foreach(&levels, drawLevelEntry);
    }
}
