#include <string.h>
#include <time.h>

#include "editor.h"
#include "state.h"
#include "update.h"
#include "robot.h"
#include "message.h"
#include "sha1.h"

#define CAMSPEED 16

static levelmeta_t currentlevel = { 0 };
static int hasfname = 0;

static int rx = 0;
static int ry = 0;

static int tilemode = 1;
static int seltile = TL_WALL;
static int selent = ENT_BOX;

static void inputAuthorCallback() {
    hasfname = 0;
    initRobot(0, 0);
    gotoState(ST_EDITOR);
}

static void inputTitleCallback() {
    getUserInput(currentlevel.author, 15, getMessage(MSG_AUTHOR), inputAuthorCallback);
}

// Gera um nome de arquivo a partir do SHA1 do nível
static void makeFilename() {
    currentlevel.ctime = time(NULL);
    currentlevel.hrand = rand();
    SHA1(currentlevel.filename, (const char*)&currentlevel, sizeof(levelmeta_t));
    strhex(currentlevel.filename, currentlevel.filename, 20);
    hasfname = 1;
}

void editorStart(levelmeta_t* meta) {
    if (meta && loadLevelFile(meta->filename)) {
        currentlevel = *meta;
        hasfname = 1;
        gotoState(ST_EDITOR);
    } else {
        newLevel();
        getUserInput(currentlevel.title, 15, getMessage(MSG_TITLE), inputTitleCallback);
    }
    level.num = L_EDITOR;
}

void editorExit(int commit) {
    if (commit)
        editorCommitLevel();
    gotoState(ST_TITLE);
}

void updateLevelEditor() {
    if (kheld[ALLEGRO_KEY_LCTRL]) { // Atalhos
        // CTRL + G: ativa/desativa a grade
        if (kdown[ALLEGRO_KEY_G]) {
            drawToggleGrid();
        }
        // CTRL + S: salva o nível
        if (kdown[ALLEGRO_KEY_S]) {
            editorCommitLevel();
        }
        // CTRL + O: abre a lista de níveis
        if (kdown[ALLEGRO_KEY_O]) {
            gotoState(ST_LEVELS);
        }
    } else { // Movimento da câmera
        if (kheld[KB_DOWN] && (level.cam.scy+CAMSPEED <= 3200-DISPLAY_HEIGHT)) level.cam.scy += CAMSPEED;
        if (kheld[KB_UP] && (level.cam.scy-CAMSPEED >= 0)) level.cam.scy -= CAMSPEED;
        if (kheld[KB_RIGHT] && (level.cam.scx+CAMSPEED <= 3200-DISPLAY_WIDTH)) level.cam.scx += CAMSPEED;
        if (kheld[KB_LEFT] && (level.cam.scx-CAMSPEED >= 0)) level.cam.scx -= CAMSPEED;
    }

    // Abre o menu
    if (kdown[KB_PAUSE] || kdown[KB_CANCEL]) {
        gotoState(ST_PAUSE);
        gotoScreen(1);
    }

    // Mouse input
    int mouse_scx = mouse_x + level.cam.scx;
    int mouse_scy = mouse_y + level.cam.scy;
    if (mouse_b == MB_LEFT) {
        // Se o mouse estiver na barra lateral, seleciona um tile/objeto
        if (mouse_x >= DISPLAY_WIDTH-80) {
            int i;
            for (i = 0; i < ENT_COUNT+TILE_COUNT; i++) {
                if (rectangleCollision(
                    mouse_x, mouse_y, 1, 1,
                    DISPLAY_WIDTH-56, 40+i*48, 32, 32)
                ) {
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
                if (selent == ENT_BOX) level.boxes++;
                entityAdd((mouse_scx>>5)<<5, (mouse_scy>>5)<<5, selent);
            }
        }
    } else if (mouse_b == MB_RIGHT) { // O botão direito remove tiles/objetos
        entity_t* ent = entityCollision(mouse_scx, mouse_scy, 1, 1, ENT_ANY);
        if (ent && entityDestroyPos(mouse_scx, mouse_scy, 1, 1, ent->id)) {
            if (ent->id == ENT_BOX) level.boxes--;
        } else {
            TILE_SET(mouse_scx, mouse_scy, 0);
        }
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
    tilemode = 1;
    seltile = TL_WALL;
    gotoState(ST_MAIN);
}

void editorStopLevel() {
    entityLoadState();
    resetRobot(rx, ry);
    level.cam.scx = (getRobotX() < 320) ? 0 : getRobotX() - (DISPLAY_WIDTH>>1);
    level.cam.scy = (getRobotY() < 240) ? 0 : getRobotY() - (DISPLAY_HEIGHT>>1);
    level.boxes = getEntityCount(ENT_BOX);
    gotoState(ST_EDITOR);
}
