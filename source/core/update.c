#include "update.h"
#include "maingame.h"
#include "pause.h"
#include "editor.h"
#include "levellist.h"
#include "titlescreen.h"

void update() {
    if (hud) {
        switch(hud) {
            case H_MESSAGE: updateError(); break;
            case H_INPUT: updateUserInput(); break;
        }
    } else {
        switch(state) {
            case ST_MAIN: updateMainGame(); break;
            case ST_PAUSE: updatePauseMenu(); break;
            case ST_LEVELS: updateLevelList(); break;
            case ST_EDITOR: updateLevelEditor(); break;
            default: updateTitleScreen(); break;
        }
    }
}
