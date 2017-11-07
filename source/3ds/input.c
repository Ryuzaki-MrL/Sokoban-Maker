#include "update.h"
#include "draw.h"

static u32 kdown = 0;
static u32 kheld = 0;
static u32 kup = 0;
static touchPosition touch = { 0 };

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

void getUserInput(int size, const char* caption, int password, icb callback) {
    char buffer[size+1];
    SwkbdState kb;

    swkbdInit(&kb, SWKBD_TYPE_QWERTY, 2, size);
    swkbdSetHintText(&kb, caption);
    swkbdSetPasswordMode(&kb, password);
    swkbdSetButton(&kb, SWKBD_BUTTON_LEFT, getMessage(MSG_CANCEL), 0);
    swkbdSetButton(&kb, SWKBD_BUTTON_RIGHT, getMessage(MSG_CONFIRM), 1);
    swkbdSetValidation(&kb, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);

    swkbdInputText(&kb, buffer, size+1);

    SwkbdResult result = swkbdGetResult(&kb);
    if (result==SWKBD_D1_CLICK1 && callback) {
        callback(buffer);
    }
}

void updateUserInput() {
    // STUB
}

void drawUserInput() {
    // STUB
}
