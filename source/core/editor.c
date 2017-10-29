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
#define STACK_SIZE 21

static stack_t* undostack = NULL;
static stack_t* redostack = NULL;

static levelmeta_t currentlevel;
static int hasfname = 0;

static int rx = 0;
static int ry = 0;

static int showgrid = 1;
static int tilemode = 1;
static int cursor = 0;
static int seltile = TL_WALL;
static int selent = ENT_BOX;

typedef void(*act)(void*);

typedef struct sAction {
    act action;
    int args[3];
} action_t;
/*
static void actionEntityAdd(int args[]) {
    entityAdd(args[0], args[1], args[2]);
}

static void actionEntityDestroy(int args[]) {
    entityDestroy(args[0]);
}

static void actionTileSet(int args[]) {
    TILE_SET(args[0], args[1], args[2]);
}

static void actionResetRobot(int args[]) {
    resetRobot(args[0], args[1]);
}
*/
static void inputAuthorCallback(const char* author) {
    strcpy(currentlevel.author, author);
    hasfname = 0;
    initRobot(0, 0);
    state = ST_EDITOR;
}

static void inputTitleCallback(const char* title) {
    strcpy(currentlevel.title, title);
    getUserInput(sizeof(currentlevel.author)-1, getMessage(MSG_AUTHOR), inputAuthorCallback);
}

// Gera um nome de arquivo a partir do SHA1 do nível
static void makeFilename() {
    currentlevel.ctime = time(NULL);
    currentlevel.hrand = rand();
    SHA1(currentlevel.filename, (const char*)&currentlevel, sizeof(levelmeta_t));
    strhex(currentlevel.filename, currentlevel.filename, 20);
    hasfname = 1;
}
/*
static void undoAdd(act action, int args[]) {
    if (!undostack || !redostack) return;
}

static void redoAdd(act action, int args[]) {
    if (!redostack || !undostack) return;
}

static void undo() {
    
}

static void redo() {
    
}
*/
static void editorDeleteAtPos(int x, int y) {
    entity_t* ent = entityCollision(x, y, 1, 1, ENT_BOX);
    if (ent && entityDestroyPos(x, y, 1, 1, ent->id)) {
        level.boxes--;
    } else {
        TILE_SET(x, y, 0);
    }
}

void editorStart(levelmeta_t* meta) {
    undostack = initStack(STACK_SIZE, sizeof(action_t));
    redostack = initStack(STACK_SIZE, sizeof(action_t));
    if (meta && loadLevelFile(meta->filename)) {
        currentlevel = *meta;
        hasfname = 1;
        state = ST_EDITOR;
    } else {
        newLevel();
        memset(&currentlevel, 0, sizeof(currentlevel));
        getUserInput(sizeof(currentlevel.title)-1, getMessage(MSG_TITLE), inputTitleCallback);
    }
}

void editorExit(int commit) {
    destroyStack(undostack);
    destroyStack(redostack);
    if (commit)
        editorCommitLevel();
    state = (ST_TITLE);
}

void updateLevelEditor() {
#if !defined(_3DS) && !defined(__wiiu__)
    if (isKeyHeld(KEY_EXTRA)) { // Atalhos
        // CTRL + G: ativa/desativa a grade
        if (isKeyDown(ALLEGRO_KEY_G)) {
            showgrid ^= 1;
        }
        // CTRL + S: salva o nível
        if (isKeyDown(ALLEGRO_KEY_S)) {
            editorCommitLevel();
        }
        // CTRL + O: abre a lista de níveis
        if (isKeyDown(ALLEGRO_KEY_O)) {
            openUserLevelList(LISTMODE_LOAD);
        }
        // CTRL + Z: desfazer
        if (isKeyDown(ALLEGRO_KEY_Z)) {
            //undo();
        }
        // CTRL + Y: refazer
        if (isKeyDown(ALLEGRO_KEY_Y)) {
            //redo();
        }
    } else
#else
    if (isKeyDown(KEY_X)) {
        showgrid ^= 1;
    }
#endif
    { // Movimento da câmera
        if (isKeyHeld(KEY_DOWN) && (level.cam.scy+CAMSPEED < (TILE_ROW<<5)-DISPLAY_HEIGHT)) level.cam.scy += CAMSPEED;
        else if (isKeyHeld(KEY_UP) && (level.cam.scy-CAMSPEED >= 0)) level.cam.scy -= CAMSPEED;
        if (isKeyHeld(KEY_RIGHT) && (level.cam.scx+CAMSPEED < (TILE_ROW<<5)-DISPLAY_WIDTH)) level.cam.scx += CAMSPEED;
        else if (isKeyHeld(KEY_LEFT) && (level.cam.scx-CAMSPEED >= 0)) level.cam.scx -= CAMSPEED;
    }

    // Abre o menu
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
        } else if (getMouseX() >= DISPLAY_WIDTH-80) { // Tile/object bar
            int i;
            for (i = 0; i < ENT_COUNT+TILE_COUNT; i++) {
                if (rectangleCollision(getMouseX(), getMouseY(), 1, 1, DISPLAY_WIDTH-56, 40+i*48, 32, 32)) {
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
        } else if (tilemode) { // Senão, cria o tile/objeto selecionado
            TILE_SET(mouse_scx, mouse_scy, seltile);
        } else {
            if (selent == ENT_ROBOT) {
                resetRobot((mouse_scx>>5)<<5, (mouse_scy>>5)<<5);
            } else {
                if (selent == ENT_BOX)
                    level.boxes++;
                entityAdd((mouse_scx>>5)<<5, (mouse_scy>>5)<<5, selent);
            }
        }
    } else if (mouse_btn == MBT_RIGHT) { // O botão direito remove tiles/objetos
        editorDeleteAtPos(mouse_scx, mouse_scy);
    }
}

void editorCommitLevel() {
    if (!hasfname) makeFilename();
    saveLevel(&currentlevel);
    error(getMessage(MSG_LEVELSAVED), NULL);
}

void editorPlayLevel() {
    entitySaveState();
    rx = getRobotX();
    ry = getRobotY();
    state = (ST_MAIN);
}

void editorStopLevel() {
    entityLoadState();
    resetRobot(rx, ry);
    level.cam.scx = (getRobotX() <  (DISPLAY_WIDTH>>1)) ? 0 : getRobotX() - (DISPLAY_WIDTH>>1);
    level.cam.scy = (getRobotY() < (DISPLAY_HEIGHT>>1)) ? 0 : getRobotY() - (DISPLAY_HEIGHT>>1);
    level.boxes = getEntityCount(ENT_BOX);
    state = ST_EDITOR;
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

void drawLevelEditor() {
    drawLevel();
    if (showgrid) drawGrid();

    drawRectangle(DISPLAY_WIDTH-80, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, C_WHITE, 1);
    drawRectangle(DISPLAY_WIDTH-64, (DISPLAY_HEIGHT/12 - 8) + cursor*48, DISPLAY_WIDTH-16, (DISPLAY_HEIGHT/12 + 40) + cursor*48, RGBA8(255,0,0,255), 1);

    int i;
    for (i = 0; i < TILE_COUNT; i++) {
        drawTile(i, DISPLAY_WIDTH-56, DISPLAY_HEIGHT/12 + i*48);
    }
    for (i = TILE_COUNT; i < ENT_COUNT+TILE_COUNT; i++) {
        drawSprite(i-TILE_COUNT, 0, DISPLAY_WIDTH-56, DISPLAY_HEIGHT/12 + i*48);
    }
}
