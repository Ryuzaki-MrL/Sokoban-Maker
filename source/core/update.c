#include "update.h"

void update() {
    if (hud) {
        switch(hud) {
            case H_QUESTION: updateQuestion(); break;
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
