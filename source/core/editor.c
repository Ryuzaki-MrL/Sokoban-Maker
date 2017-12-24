#include <string.h>
#include <time.h>

#include "draw.h"
#include "editor.h"
#include "state.h"
#include "update.h"
#include "robot.h"
#include "sha1.h"
#include "stack.h"
#include "util.h"

#define CAMSPEED 16
#define STACK_SIZE 100

static stack_t* undostack = NULL;
static stack_t* redostack = NULL;

static levelmeta_t currentlevel;
static int hasfname = 0;
static int modified = 0;

static int rx = 0;
static int ry = 0;

static int showgrid = 1;
static int tilemode = 1;
static int cursor = 0;
static int seltile = TL_WALL;
static int selent = ENT_BOX;

typedef void(*act)(int, int, int);

typedef struct sAction {
    act action;
    int arg0;
    int arg1;
    int arg2;
    int wasmod;
    int wasvalid;
} action_t;

static void actionEntityAdd(int x, int y, int id);
static void revertEntityAdd(int x, int y, int id);
static void actionEntityDestroy(int x, int y, int id);
static void revertEntityDestroy(int x, int y, int id);
static void actionTilePut(int x, int y, int tile);
static void revertTilePut(int x, int y, int tile);
static void actionResetRobot(int x, int y, int check);
static void revertResetRobot(int x, int y, int check);

static void undoAdd(act action, int arg0, int arg1, int arg2) {
    if (!undostack || !redostack) return;
    action_t a = {action, arg0, arg1, arg2, modified, currentlevel.valid};
    push(undostack, &a);
}

static void redoAdd(act action, int arg0, int arg1, int arg2) {
    if (!redostack || !undostack) return;
    action_t a = {action, arg0, arg1, arg2, modified, currentlevel.valid};
    push(redostack, &a);
}

static void actionEntityAdd(int x, int y, int id) {
    if (entityAdd(x, y, id)) {
        undoAdd(revertEntityAdd, x, y, id);
    }
}

static void revertEntityAdd(int x, int y, int id) {
    entityDestroyPos(x, y, 1, 1, id);
    redoAdd(actionEntityAdd, x, y, id);
}

static void actionEntityDestroy(int x, int y, int id) {
    if (entityDestroyPos(x, y, 1, 1, id)) {
        undoAdd(revertEntityDestroy, x, y, id);
    }
}

static void revertEntityDestroy(int x, int y, int id) {
    entityAdd(x, y, id);
    redoAdd(actionEntityDestroy, x, y, id);
}

static void actionTilePut(int x, int y, int tile) {
    undoAdd(revertTilePut, x, y, TILE_GET(x, y));
    TILE_SET(x, y, tile);
}

static void revertTilePut(int x, int y, int tile) {
    redoAdd(actionTilePut, x, y, TILE_GET(x, y));
    TILE_SET(x, y, tile);
}

static void actionResetRobot(int x, int y, int check) {
    undoAdd(revertResetRobot, getRobotX(), getRobotY(), 0);
    resetRobot(x, y);
}

static void revertResetRobot(int x, int y, int check) {
    redoAdd(actionResetRobot, getRobotX(), getRobotY(), 0);
    resetRobot(x, y);
}

static void inputAuthorCallback(const char* author) {
    strcpy(currentlevel.author, author);
    hasfname = 0;
    initRobot(0, 0);
    state = ST_EDITOR;
}

static void inputTitleCallback(const char* title) {
    strcpy(currentlevel.title, title);
    getUserInput(sizeof(currentlevel.author)-1, getMessage(MSG_AUTHOR), 0, inputAuthorCallback);
}

static void makeFilename() {
    currentlevel.ctime = time(NULL);
    currentlevel.hrand = rand();
    SHA1(currentlevel.filename, (const char*)&currentlevel, sizeof(levelmeta_t));
    strhex(currentlevel.filename, currentlevel.filename, 20);
    hasfname = 1;
}

static void undo() {
    if (!undostack) return;
    action_t a;
    if (pop(undostack, &a)) {
        a.action(a.arg0, a.arg1, a.arg2);
        modified = a.wasmod;
        currentlevel.valid = a.wasvalid;
    }
}

static void redo() {
    if (!redostack) return;
    action_t a;
    if (pop(redostack, &a)) {
        a.action(a.arg0, a.arg1, a.arg2);
        modified = a.wasmod;
        currentlevel.valid = a.wasvalid;
    }
}

static void editorAddAtPos(int x, int y) {
    if (tilemode) {
        actionTilePut(x, y, seltile);
    } else {
        if (selent == ENT_ROBOT) {
            actionResetRobot((x>>5)<<5, (y>>5)<<5, 0);
        } else {
            actionEntityAdd((x>>5)<<5, (y>>5)<<5, selent);
        }
    }
    clearStack(redostack);
    modified = 1;
    currentlevel.valid = 0;
}

static void editorDeleteAtPos(int x, int y) {
    entity_t* ent = entityCollision(x, y, 1, 1, ENT_BOX);
    if (ent) {
        actionEntityDestroy((x>>5)<<5, (y>>5)<<5, ent->id);
    } else {
        actionTilePut(x, y, 0);
    }
    clearStack(redostack);
    modified = 1;
    currentlevel.valid = 0;
}

void editorStart(levelmeta_t* meta) {
    undostack = initStack(STACK_SIZE, sizeof(action_t));
    redostack = initStack(STACK_SIZE, sizeof(action_t));
    modified = 0;
    if (meta && loadLevelFile(meta->filename)) {
        currentlevel = *meta;
        hasfname = 1;
        state = ST_EDITOR;
    } else {
        newLevel();
        memset(&currentlevel, 0, sizeof(currentlevel));
        getUserInput(sizeof(currentlevel.title)-1, getMessage(MSG_TITLE), 0, inputTitleCallback);
    }
}

void editorExit(int commit) {
    destroyStack(undostack);
    destroyStack(redostack);
    undostack = redostack = NULL;
    if (commit)
        editorCommitLevel();
    state = ST_TITLE;
}

void editorSaveQuit() {
    if (modified) {
        question(getMessage(QST_SAVELEVEL), editorExit);
    } else {
        editorExit(0);
    }
}

#define CURSOR_HEIGHT   (DISPLAY_HEIGHT>>3)

void updateLevelEditor() {
    if (isKeyHeld(KEY_EXTRA)) {
        // CTRL + G: toggle grid
        if (isKeyDown(KEY_GRID)) {
            showgrid ^= 1;
        }
        // CTRL + S: save level
        if (isKeyDown(KEY_SAVE)) {
            editorCommitLevel();
        }
        // CTRL + O: open level
        if (isKeyDown(KEY_OPEN)) {
            editorSaveQuit();
            openUserLevelList(LISTMODE_LOAD);
        }
        // CTRL + Z: undo
        if (isKeyDown(KEY_UNDO)) {
            undo();
        }
        // CTRL + Y: redo
        if (isKeyDown(KEY_REDO)) {
            redo();
        }
    }
#if defined(_3DS) || defined(__wiiu__)
    else if (isKeyDown(KEY_X)) {
        showgrid ^= 1;
    }
#endif
    if (isKeyHeld(KEY_DOWN) && (level.cam.scy+CAMSPEED < (TILE_ROW<<5)-DISPLAY_HEIGHT)) {
        level.cam.scy += CAMSPEED;
    } else if (isKeyHeld(KEY_UP) && (level.cam.scy-CAMSPEED >= 0)) {
        level.cam.scy -= CAMSPEED;
    }
    if (isKeyHeld(KEY_RIGHT) && (level.cam.scx+CAMSPEED < (TILE_ROW<<5)-DISPLAY_WIDTH)) {
        level.cam.scx += CAMSPEED;
    } else if (isKeyHeld(KEY_LEFT) && (level.cam.scx-CAMSPEED >= 0)) {
        level.cam.scx -= CAMSPEED;
    }

    // Menu
    if (isKeyDown(KEY_CONFIRM) || isKeyDown(KEY_CANCEL)) {
        state = ST_PAUSE;
    }

    // Mouse input
    int mouse_scx = getMouseX() + level.cam.scx;
    int mouse_scy = getMouseY() + level.cam.scy;
    int mouse_btn = getMouseButton();
    if (mouse_btn == MBT_LEFT) {
        if (isKeyHeld(KEY_EXTRA)) {
            editorDeleteAtPos(mouse_scx, mouse_scy);
        } else if (getMouseX() >= DISPLAY_WIDTH-(DISPLAY_WIDTH>>3)) { // Tile/object bar
            int i;
            for (i = 0; i < ENT_COUNT+TILE_COUNT; i++) {
                if (rectangleCollision(getMouseX(), getMouseY(), 1, 1, DISPLAY_WIDTH-(DISPLAY_WIDTH>>3), CURSOR_HEIGHT - 8 + i*CURSOR_HEIGHT, (DISPLAY_WIDTH>>3), CURSOR_HEIGHT)) {
                    if (i < TILE_COUNT) {
                        seltile = i+1;
                        tilemode = 1;
                    } else {
                        selent = i-TILE_COUNT;
                        tilemode = 0;
                    }
                    cursor = i;
                }
            }
        } else {
            editorAddAtPos(mouse_scx, mouse_scy);
        }
    } else if (mouse_btn == MBT_RIGHT) {
        editorDeleteAtPos(mouse_scx, mouse_scy);
    }
}

void editorCommitLevel() {
    if (!hasfname) makeFilename();
    saveLevel(&currentlevel);
    error(getMessage(MSG_LEVELSAVED), NULL);
    modified = 0;
}

void editorPlayLevel() {
    level.boxes = getEntityCount(ENT_BOX);
    entitySaveState();
    rx = getRobotX();
    ry = getRobotY();
    state = ST_MAIN;
}

void editorStopLevel() {
    entityLoadState();
    resetRobot(rx, ry);
    level.cam.scx = (getRobotX() <  (DISPLAY_WIDTH>>1)) ? 0 : getRobotX() - (DISPLAY_WIDTH>>1);
    level.cam.scy = (getRobotY() < (DISPLAY_HEIGHT>>1)) ? 0 : getRobotY() - (DISPLAY_HEIGHT>>1);
    state = ST_EDITOR;
}

void editorValidateLevel() {
    editorStopLevel();
    currentlevel.valid = 1;
}

static void drawGrid() {
    int ox = level.cam.scx >> 5;
    int oy = level.cam.scy >> 5;
    int xoff = (DISPLAY_WIDTH >> 5) << 1;
    int yoff = (DISPLAY_HEIGHT >> 5) << 1;
    int x, y;
    for (y = oy; y < oy+yoff; y++) {
        for (x = ox; x < ox+xoff; x++) {
            int dx = (x << 5) - (level.cam.scx);
            int dy = (y << 5) - (level.cam.scy);
            #if defined(_3DS)
                drawGridAux(dx, dy);
            #else
                drawRectangle(dx, dy, dx + 32, dy + 32, C_BLACK, 0);
            #endif
        }
    }
}

#define C_VALID         RGBA8(0, 0x7F, 0, 0xFF)
#define C_UNTESTED      RGBA8(0xFF, 0, 0, 0xFF)

void drawLevelEditor() {
    drawLevel();
    if (showgrid) drawGrid();

    drawRectangle(0, DISPLAY_HEIGHT - 24, 128, DISPLAY_HEIGHT, C_WHITE, 1);
#ifdef DEBUG
    drawTextFormat(C_BLACK, 0, 0, "Undo: %d/%d / Redo: %d/%d", getStackTop(undostack), getStackBottom(undostack), getStackTop(redostack), getStackBottom(redostack));
#endif
    drawTextCenter(currentlevel.valid ? C_VALID : C_UNTESTED, 64, DISPLAY_HEIGHT - 24, getMessage(modified ? (MSG_UNTESTEDM + currentlevel.valid) : (MSG_UNTESTED + currentlevel.valid)));
    drawRectangle(DISPLAY_WIDTH-(DISPLAY_WIDTH>>3), 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, C_WHITE, 1);
    drawRectangle(DISPLAY_WIDTH-(DISPLAY_WIDTH>>4)-(CURSOR_HEIGHT>>1), (DISPLAY_HEIGHT/12 - 4) + cursor*CURSOR_HEIGHT, DISPLAY_WIDTH-(DISPLAY_WIDTH>>4)+(CURSOR_HEIGHT>>1), ((DISPLAY_HEIGHT/12) + CURSOR_HEIGHT - 4) + cursor*CURSOR_HEIGHT, RGBA8(255,0,0,255), 1);

    int i;
    for (i = 0; i < TILE_COUNT; i++) {
        drawTile(i, DISPLAY_WIDTH-(DISPLAY_WIDTH>>4)-(CURSOR_HEIGHT>>1)+4, DISPLAY_HEIGHT/12 + i*CURSOR_HEIGHT);
    }
    for (i = TILE_COUNT; i < ENT_COUNT+TILE_COUNT; i++) {
        drawSprite(i-TILE_COUNT, 0, DISPLAY_WIDTH-(DISPLAY_WIDTH>>4)-(CURSOR_HEIGHT>>1)+4, DISPLAY_HEIGHT/12 + i*CURSOR_HEIGHT);
    }
}
