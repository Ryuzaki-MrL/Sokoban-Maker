#include "update.h"
#include "draw.h"

static int kdown[ALLEGRO_KEY_MAX] = { 0 };
static int kheld[ALLEGRO_KEY_MAX] = { 0 };
static int keyany = 0;
static int unichar = 0;
static int mouse_b = 0;
static int mouse_x = 0;
static int mouse_y = 0;

static char userinput[260] = "";
static int inputpos = 0;
static int inputmax = 0;

static char msgcaption[260] = "";

static clb errorclb = NULL;
static icb inputclb = NULL;
static qcb questionclb = NULL;

void updateInput(ALLEGRO_EVENT* event) {
    switch(event->type) {
        case ALLEGRO_EVENT_MOUSE_AXES: {
            mouse_x = event->mouse.x;
            mouse_y = event->mouse.y;
            break;
        }
        case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN: {
            mouse_b = event->mouse.button;
            break;
        }
        case ALLEGRO_EVENT_MOUSE_BUTTON_UP: {
            mouse_b = MBT_NONE;
            break;
        }
        case ALLEGRO_EVENT_KEY_CHAR: {
            unichar = (event->keyboard.keycode == ALLEGRO_KEY_BACKSPACE) ? -1 : event->keyboard.unichar;
            break;
        }
        case ALLEGRO_EVENT_KEY_DOWN: {
            keyany = 1;
            kdown[event->keyboard.keycode] = 1;
            kheld[event->keyboard.keycode] = 1;
            break;
        }
        case ALLEGRO_EVENT_KEY_UP: {
            keyany = 0;
            kdown[event->keyboard.keycode] = 0;
            kheld[event->keyboard.keycode] = 0;
            break;
        }
    }
}

int isKeyDown(int key) {
    int k;
    if (key == KEY_ANY) {
        k = keyany;
    } else {
        k = kdown[key];
        kdown[key] = 0;
    }
    keyany = 0;
    return k;
}

int isKeyHeld(int key) {
    if (key == KEY_ANY)
        return keyany;
    return kheld[key];
}

int isKeyUp(int key) {
    return 0;
}

int getMouseButton() {
    int mb = mouse_b;
    mouse_b = 0;
    return mb;
}

int getMouseX() {
    return mouse_x;
}

int getMouseY() {
    return mouse_y;
}

void getUserInput(int size, const char* caption, icb callback) {
    if (!caption) return;
    memset(userinput, 0, sizeof(userinput));
    inputpos = 0;
    inputmax = size;
    unichar = 0;
    strncpy(msgcaption, caption, sizeof(msgcaption)-1);
    hud = H_INPUT;
    inputclb = callback;
}

void error(const char* caption, clb callback) {
    if (!caption) return;
    hud = H_MESSAGE;
    errorclb = callback;
    strncpy(msgcaption, caption, sizeof(msgcaption)-1);
}

void updateUserInput() {
    if (isKeyDown(KEY_CONFIRM)) {
        hud = 0;
        if (inputclb) inputclb(userinput);
        return;
    }
    if (isKeyDown(KEY_CANCEL)) {
        hud = 0;
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
    } else if (isKeyDown(KEY_LEFT)) {
        if (inputpos > 0) inputpos--;
    } else if (isKeyDown(KEY_RIGHT)) {
        if (inputpos < inputmax-1) inputpos++;
    }

    unichar = 0;
}

void updateError() {
    if (isKeyDown(KEY_ANY)) {
        if (errorclb) errorclb();
        hud = 0;
    }
}

#define ENTRY_X1    (DISPLAY_WIDTH>>2)
#define ENTRY_X2    (DISPLAY_WIDTH - ENTRY_X1)
#define ENTRY_Y1    (DISPLAY_HEIGHT/3)
#define ENTRY_Y2    (DISPLAY_HEIGHT - ENTRY_Y1)

void drawUserInput() {
    drawRectangle(ENTRY_X1, ENTRY_Y1, ENTRY_X2, ENTRY_Y2, RGBA8(220,220,220,255), 1);
    drawRectangle(ENTRY_X1 + 24, ENTRY_Y1 + 64, ENTRY_X2 - 24, ENTRY_Y1 + 104, C_WHITE, 1);
    drawTextCenter(RGBA8(16, 16, 16, 255), (DISPLAY_WIDTH>>1), 196, msgcaption);
    drawTextCenter(RGBA8(16, 16, 16, 255), (DISPLAY_WIDTH>>1), 232, userinput);
}

void drawError() {
    drawRectangle(ENTRY_X1, ENTRY_Y1, ENTRY_X2, ENTRY_Y2, RGBA8(255,255,255,220), 1);
    drawTextMultiline(RGBA8(16, 16, 16, 255), (DISPLAY_WIDTH>>1), ENTRY_Y1, (ENTRY_X2 - ENTRY_X1 - 32), 1, msgcaption);
}
