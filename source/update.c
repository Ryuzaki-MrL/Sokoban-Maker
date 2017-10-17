#include "update.h"
#include "draw.h"
#include "robot.h"
#include "save.h"
#include "state.h"
#include "message.h"
#include "editor.h"

static void updateUserInput();
static void updateTitleScreen();
static void updateRankScreen();
static void updateMainGame();
static void updateEntities();
static void updatePauseMenu();
static void updateLevelList();

static int inputpos = 0;
static int inputmax = 0;

static clb errorclb = NULL;

void gotoState(int newstate) {
    state = newstate;
    cursor = 0;
}

void gotoScreen(int newscreen) {
    screen = newscreen;
    cursor = 0;
}

static void gotoNextLevel() {
    if (level.num == L_EDITOR) {
        editorStopLevel();
        return;
    }
    setBestTime(level.num - 1, level.timer);
    if (level.num == LEVELCOUNT) {
        setLevel(0xFF);
        gotoState(ST_TITLE);
    } else if (loadLevel(level.num + 1)) {
        gotoState(ST_MAIN);
    } else {
        gotoState(ST_TITLE);
    }
}

static void gotoGameOver() {
    if (level.num == L_EDITOR) {
        editorStopLevel();
        return;
    }
    loadLevel(level.num);
}

void getUserInput(char* dst, int size, const char* caption, clb callback) {
    userinput = dst;
    inputpos = strlen(userinput);
    inputmax = size;
    userinput[inputpos] = '\0';
    unichar = 0;
    drawSetMessageCaption(caption);
    hud = H_INPUT;
    errorclb = callback;
}

void error(const char* caption, clb callback) {
    hud = H_MESSAGE;
    errorclb = callback;
    drawSetMessageCaption(caption);
}

void update() {
    switch(hud) {
        case H_MESSAGE: {
            if (keyany) {
                if (errorclb) errorclb();
                hud = 0;
            } return;
        }
        case H_INPUT: {
            updateUserInput();
            return;
        }
    }
    switch(state) {
        case ST_MAIN: updateMainGame(); break;
        case ST_PAUSE: updatePauseMenu(); break;
        case ST_RANK: updateRankScreen(); break;
        case ST_LEVELS: updateLevelList(); break;
        case ST_EDITOR: updateLevelEditor(); break;
        default: updateTitleScreen(); break;
    }
}

// Função auxiliar que atualiza o estado dos objetos
static void updateEntityAux(void* data) {
    entity_t* ent = (entity_t*)data;
    const sprite_t* spr = getSprite(ent->sprite);
    if (!ent || !spr) return;

    // Se o objeto está se movendo...
    if (ent->moving) {
        // Desloca-se "speed" pixels na direção "xdir" e "ydir"
        ent->x += ent->xdir * ent->speed;
        ent->y += ent->ydir * ent->speed;

        // Se o objeto estiver alinhado com a grade...
        if (IS_SNAPPED(ent->x, ent->y)) {
            // Para de se mover
            ent->moving = 0;

            // Verifica em qual tile está pisando
            switch(TILE_GET(ent->x, ent->y)) {
                // Se for um buraco, game over
                case TL_HOLE: {
                    ent->visible = 0;
                    error(getMessage(MSG_GAMEOVER), gotoGameOver);
                    return;
                }
                // Se for um portal, retorna para a posição
                // inicial APENAS se estiver livre
                case TL_PORTAL: {
                    if ((TILE_GET(ent->xstart, ent->ystart)!=TL_WALL) && !entityCollision(ent->xstart, ent->ystart, 32, 32, ENT_ANY)) {
                        ent->x = ent->xstart;
                        ent->y = ent->ystart;
                    }
                    break;
                }
            }

            // Se o objeto atualizado for uma caixa...
            if (ent->id == ENT_BOX) {
                // Verifica se está em cima do tile de destino
                if (TILE_GET(ent->x, ent->y)==TL_BOXPOST) {
                    // Se sim, diminui o número de caixas restantes
                    if (ent->frame==0) {
                        level.boxes--;
                        // Ao chegar a zero, o nível é concluído
                        if (level.boxes == 0) {
                            error(getMessage(MSG_COMPLETE), gotoNextLevel);
                        }
                    }
                    ent->frame = 1;
                } else {
                    if (ent->frame==1)
                        level.boxes++;
                    ent->frame = 0;
                }
            }
        }
    }
    ent->frame += ent->anispd;
}

// Percorre a lista de objetos e atualiza seus
// estados utilizando a função acima
static void updateEntities() {
    int i;
    for (i = 0; i < ENT_COUNT; i++) {
        foreach(getEntityList(i), updateEntityAux);
    }
}

static void updateUserInput() {
    if (!userinput || kdown[KB_PAUSE] || kdown[KB_CANCEL]) {
        hud = 0;
        if (kdown[KB_PAUSE] && errorclb) {
            errorclb();
        }
        return;
    }

    if (unichar==-1) {
        if (inputpos > 0) {
            userinput[--inputpos] = '\0';
        }
    } else if ((char)unichar > 31) {
        if (inputpos < inputmax) {
            userinput[inputpos++] = (char)unichar;
            userinput[inputpos] = '\0';
        }
    } else if (kdown[KB_LEFT]) {
        if (inputpos > 0) inputpos--;
    } else if (kdown[KB_RIGHT]) {
        if (inputpos < inputmax-1) inputpos++;
    }

    unichar = 0;
}

static void updateTitleScreen() {
    static const int curmax[] = { 5, 2 };
    if (kdown[KB_DOWN] && cursor < curmax[screen])
        cursor++;
    if (kdown[KB_UP] && cursor > 0)
        cursor--;
    if (kdown[KB_PAUSE]) {
        switch(screen*10 + cursor) {
            case 0: { // New Game
                newGame();
                if (loadLevel(1)) gotoState(ST_MAIN);
                break;
            }
            case 1: { // Continue
                if (loadLevel(getLevel())) gotoState(ST_MAIN);
                break;
            }
            case 2: { // Level Editor
                gotoScreen(1);
                break;
            }
            case 3: { // Ranking
                gotoState(ST_RANK);
                break;
            }
            case 4: { // Language
                setLanguage((getLanguage() + 1) % getLangCount());
                setMessageLanguage(getLanguage());
                break;
            }
            case 5: { // Save & Quit
                running = 0;
                break;
            }

            case 10: { // New Level
                editorStart(NULL);
                break;
            }
            case 11: { // Load Level
                gotoState(ST_LEVELS);
                getUserLevelList(1);
                break;
            }
            case 12: { // Go Back
                gotoScreen(0);
                break;
            }
        }
    }
}

static void updateRankScreen() {
    if (kdown[KB_RIGHT] && cursor<LEVELCOUNT-1)
        cursor++;
    if (kdown[KB_LEFT] && cursor>0)
        cursor--;
    if (kdown[KB_PAUSE] || kdown[KB_CANCEL])
        gotoState(ST_TITLE);
}

static void updateMainGame() {
    static unsigned framecount = 0;
    if (framecount++ > 59) {
        level.timer++;
        framecount = 0;
    }

    if (level.num == L_EDITOR) {
        if (kdown[KB_CANCEL]) {
            editorStopLevel();
        }
    } else {
        if (kdown[KB_PAUSE] || kdown[KB_CANCEL]) {
            gotoState(ST_PAUSE);
            gotoScreen(0);
        }
    }

    updateRobot();
    updateEntities();
}

static void updatePauseMenu() {
    static const int curmax[] = { 2, 3 };
    if (kdown[KB_DOWN] && cursor<curmax[screen])
        cursor++;
    if (kdown[KB_UP] && cursor>0)
        cursor--;
    if (kdown[KB_PAUSE]) {
        switch(screen*10 + cursor) {
            case 1: loadLevel(level.num); // Restart
            case 0: gotoState(ST_MAIN); break; // Resume
            case 2: setLevel(level.num); saveGame(); gotoState(ST_TITLE); break; // Save & Quit

            case 10: gotoState(ST_EDITOR); break;
            case 11: editorCommitLevel(); break; // Save Level
            case 12: editorPlayLevel(); break; // Play Level
            case 13: gotoState(ST_TITLE); break; //question(getMessage(QST_SAVELEVEL), editorExit); break; // Save & Quit
        }
    }
}

static int sortLevelByTitle(const void* data1, const void* data2) {
    levelmeta_t* m1 = (levelmeta_t*)data1;
    levelmeta_t* m2 = (levelmeta_t*)data2;
    return (strcmp(m1->title, m2->title)<=0);
}

static int sortLevelByAuthor(const void* data1, const void* data2) {
    levelmeta_t* m1 = (levelmeta_t*)data1;
    levelmeta_t* m2 = (levelmeta_t*)data2;
    return (strcmp(m1->author, m2->author)<=0);
}

static void updateLevelList() {
    const list_t* l = getUserLevelList(0);
    if (kdown[KB_DOWN] && cursor < l->size-1)
        cursor++;
    if (kdown[KB_UP] && cursor > 0)
        cursor--;
    if (kdown[KB_CANCEL])
        gotoState(ST_TITLE);
    if (kdown[KB_PAUSE]) {
        levelmeta_t* meta = (levelmeta_t*)getNodeData(l, cursor);
        if (meta) {
            editorStart(meta);
        }
    }
    if (kdown[ALLEGRO_KEY_TAB]) { // DEBUG ONLY
        sortUserLevelList(sortLevelByTitle);
    }
}
