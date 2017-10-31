#include "update.h"
#include "draw.h"

static int cursor = 0;

void updatePauseMenu() {
    if (isKeyDown(KEY_DOWN) && cursor<3) {
        cursor++;
    }
    if (isKeyDown(KEY_UP) && cursor>0) {
        cursor--;
    }
    if (isKeyDown(KEY_CONFIRM)) {
        switch(cursor) {
            case 0: state = ST_EDITOR; break; // Resume
            case 1: editorCommitLevel(); break; // Save Level
            case 2: editorPlayLevel(); break; // Play Level
            case 3: editorSaveQuit(); break; // Save & Quit
        }
    }
}

void drawPauseMenu() {
    drawBackground();
    drawRectangle(80, 80, DISPLAY_WIDTH - 80, DISPLAY_HEIGHT - 80, RGBA8(255,255,255,220), 1);
    drawText(C_BLACK, DISPLAY_WIDTH/3, DISPLAY_HEIGHT - 270, getMessage(MSG_RESUME));
    drawText(C_BLACK, DISPLAY_WIDTH/3, DISPLAY_HEIGHT - 250, getMessage(MSG_SAVELEVEL));
    drawText(C_BLACK, DISPLAY_WIDTH/3, DISPLAY_HEIGHT - 230, getMessage(MSG_PLAYLEVEL));
    drawText(C_BLACK, DISPLAY_WIDTH/3, DISPLAY_HEIGHT - 210, getMessage(MSG_EXIT));
    drawRectangle(
        DISPLAY_WIDTH/3 - 12, DISPLAY_HEIGHT - 262 + cursor*20,
        DISPLAY_WIDTH/3 -  4, DISPLAY_HEIGHT - 254 + cursor*20, C_BLACK, 1
    );
}
