#include "update.h"
#include "draw.h"

static u32 kdown = 0;
static u32 kheld = 0;
static u32 kup = 0;
static touchPosition touch = { 0 };

static char msgcaption[260] = "";

static clb errorclb = NULL;
static qcb questionclb = NULL;

void updateInput() {
    hidScanInput();
    kdown = hidKeysDown();
    kheld = hidKeysHeld();
    kup = hidKeysUp();
    hidTouchRead(&touch);
}

int isKeyDown(int key) {
    if (key == KEY_ANY)
        return kdown;
    return kdown & key;
}

int isKeyHeld(int key) {
    if (key == KEY_ANY)
        return kheld;
    return kheld & key;
}

int isKeyUp(int key) {
    if (key == KEY_ANY)
        return kup;
    return kup & key;
}

int getMouseButton() {
    return touch.px || touch.py;
}

int getMouseX() {
    return touch.px;
}

int getMouseY() {
    return touch.py;
}

void getUserInput(int size, const char* caption, icb callback) {
    char buffer[size+1];
    SwkbdState kb;

    swkbdInit(&kb, SWKBD_TYPE_QWERTY, 2, size);
    swkbdSetHintText(&kb, caption);
    swkbdSetButton(&kb, SWKBD_BUTTON_LEFT, getMessage(MSG_CANCEL), 0);
    swkbdSetButton(&kb, SWKBD_BUTTON_RIGHT, getMessage(MSG_CONFIRM), 1);
    swkbdSetValidation(&kb, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);

    swkbdInputText(&kb, buffer, size+1);

    SwkbdResult result = swkbdGetResult(&kb);
    if (result==SWKBD_D1_CLICK1 && callback) {
        callback(buffer);
    }
}

void error(const char* caption, clb callback) {
    if (!caption) return;
    hud = H_MESSAGE;
    errorclb = callback;
    strncpy(msgcaption, caption, sizeof(msgcaption)-1);
}

void updateUserInput() {
    
}

void updateError() {
    if (isKeyDown(KEY_ANY)) {
        if (errorclb) errorclb();
        hud = 0;
    }
}

void drawUserInput() {
    
}

#define ENTRY_X1    (DISPLAY_WIDTH>>2)
#define ENTRY_X2    (DISPLAY_WIDTH - ENTRY_X1)
#define ENTRY_Y1    (DISPLAY_HEIGHT/3)
#define ENTRY_Y2    (DISPLAY_HEIGHT - ENTRY_Y1)

void drawError() {
    drawRectangle(ENTRY_X1, ENTRY_Y1, ENTRY_X2, ENTRY_Y2, RGBA8(255,255,255,220), 1);
    drawTextMultiline(RGBA8(16, 16, 16, 255), (DISPLAY_WIDTH>>1), ENTRY_Y1, (ENTRY_X2 - ENTRY_X1 - 32), 1, msgcaption);
}
