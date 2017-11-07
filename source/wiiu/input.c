#include <string.h>

#include "draw.h"
#include "update.h"
#include "util.h"

static int vpadError = -1;

static u32 kdown = 0;
static u32 kheld = 0;
static u32 kup = 0;
static int mouse_b = 0;
static int mouse_v = 0;
static int mouse_x = 0;
static int mouse_y = 0;
static int unichar = 0;

static char msgcaption[260] = "";
static char userinput[260] = "";
static int inputpos = 0;
static int inputmax = 0;
static int ispassword = 0;
static icb inputclb = NULL;

static int redraw = 2;

static const int kbalpha[] = {
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '/',
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '"', '_', 
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ';', ':', '!', '$',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@',
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '\'','.',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '?', '#'
};

static const int kbnumber[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '&',
    'á', 'à', 'ã', 'â', 'ª', 'é', 'ê', 'í', 'ó', 'õ', 'ô',
    'º', 'ú', 'ñ', '§', '¹', '²', '³', '£', '¢', '¬', 'ç',
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '&',
    'Á', 'À', 'Ã', 'Â', 'ª', 'É', 'Ê', 'Í', 'Ó', 'Õ', 'Ô',
    'º', 'Ú', 'Ñ', '§', '¹', '²', '³', '£', '¢', '¬', 'Ç'
};

static const int kbsymbol[] = {
    '~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
    '-', '+', '=', '_', '`', ':', ';', '[', ']', '{', '}',
    '\\','|', '"', '\'','<', '>', ',', '.', '?', '/', '£',
    '~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
    '-', '+', '=', '_', '`', ':', ';', '[', ']', '{', '}',
    '\\','|', '"', '\'','<', '>', ',', '.', '?', '/', '£'
};

static const char* kbbottom[] = {
    "SHIFT", " ", "ABC", "123", "#!?"
};

static const int* kbcurrent = kbalpha;
static int kbshift = 0;

void updateInput() {
    VPADData vpad;
    VPADRead(0, &vpad, 1, &vpadError);
    VPADTPData tpcalibrated;
    VPADGetTPCalibratedPointEx(0, VPAD_TP_854x480, &tpcalibrated, &vpad.tpdata);
    kdown = vpad.btns_d;
    kheld = vpad.btns_h;
    kup = vpad.btns_r;
    mouse_b = tpcalibrated.touched;
    mouse_v = tpcalibrated.invalid;
    mouse_x = tpcalibrated.x;
    mouse_y = tpcalibrated.y;
}

int isKeyDown(int key) {
    int k;
    if (key == KEY_ANY) {
        k = kdown;
    } else {
        k = kdown & key;
    }
    if (k)
        os_usleep(9000);
    return k;
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
    return mouse_b;
}

int getMouseX() {
    return mouse_x;
}

int getMouseY() {
    return mouse_y;
}

void getUserInput(int size, const char* caption, int password, icb callback) {
    if (!caption) return;
    memset(userinput, 0, sizeof(userinput));
    inputpos = 0;
    inputmax = size;
    unichar = 0;
    redraw = 2;
    strncpy(msgcaption, caption, sizeof(msgcaption)-1);
    hud = H_INPUT;
    ispassword = password;
    inputclb = callback;
}

#define ENTRY_X1    (DISPLAY_WIDTH>>2)
#define ENTRY_X2    (DISPLAY_WIDTH - ENTRY_X1)
#define ENTRY_Y1    (64)
#define ENTRY_Y2    (ENTRY_Y1 + 120)

#define KBSIZE      (sizeof(kbalpha)/sizeof(kbalpha[0])/2)

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

    if (mouse_b) {
        unsigned i, x, y, p = 0;
        for (i = 0; i < KBSIZE; i++) {
            x = 48 + (i%(KBSIZE/3))*64;
            y = ENTRY_Y2 + 32 + (i/(KBSIZE/3))*64;
            if (rectangleCollision(mouse_x, mouse_y, 1, 1, x, y, 56, 56)) {
                unichar = kbcurrent[i + kbshift*KBSIZE];
                return;
            }
        }
        if (rectangleCollision(mouse_x, mouse_y, 1, 1, 752, ENTRY_Y2+32, 56, 56)) {
            unichar = -1;
            return;
        }
        if (rectangleCollision(mouse_x, mouse_y, 1, 1, 752, ENTRY_Y2+160, 56, 56)) {
            hud = 0;
            if (inputclb) inputclb(userinput);
            return;
        }
        x = 48;
        for (i = 0; i < 5; i++) {
            x += p;
            p = (1 + (kbbottom[i][0]==' '))*128;
            if (rectangleCollision(mouse_x, mouse_y, 1, 1, x, ENTRY_Y2+224, p-8, 56)) {
                redraw = 2;
                switch(kbbottom[i][0]) {
                    case 'S': kbshift ^= 1; break;
                    case 'A': kbcurrent = kbalpha; break;
                    case '1': kbcurrent = kbnumber; break;
                    case '#': kbcurrent = kbsymbol; break;
                    default:  unichar = ' '; redraw = 0; break;
                }
                return;
            }
        }
    }

    if (unichar==-1) {
        if (inputpos > 0) {
            userinput[--inputpos] = '\0';
        }
    } else if ((char)unichar > 31) {
        if (inputpos < inputmax) {
            userinput[inputpos++] = (char)unichar;
            userinput[inputpos] = '\0';
            os_usleep(9000);
        }
    }

    unichar = 0;
}

#define KEYCOLOR    RGBA8(248,248,248,255)

void drawUserInput() {
    drawRectangle(ENTRY_X1, ENTRY_Y1, ENTRY_X2, ENTRY_Y2, RGBA8(220,220,220,255), 1);
    drawRectangle(ENTRY_X1 + 24, ENTRY_Y1 + 64, ENTRY_X2 - 24, ENTRY_Y1 + 104, C_WHITE, 1);
    drawTextCenter(RGBA8(16, 16, 16, 255), (DISPLAY_WIDTH>>1), ENTRY_Y1 + 16, msgcaption);
    if (ispassword) {
        char tmp[260] = "";
        memset(tmp, '*', inputpos);
        drawTextCenter(RGBA8(16, 16, 16, 255), (DISPLAY_WIDTH>>1), ENTRY_Y1 + 64 + 12, tmp);
    } else {
        drawTextCenter(RGBA8(16, 16, 16, 255), (DISPLAY_WIDTH>>1), ENTRY_Y1 + 64 + 12, userinput);
    }

    if (!redraw) return;

    // On-screen keyboard
    unsigned i, x, y, p = 0;
    drawRectangle(0, ENTRY_Y2 + 16, DISPLAY_WIDTH, DISPLAY_HEIGHT, RGBA8(220,220,220,255), 1);

    char k[2] = "";
    for (i = 0; i < KBSIZE; i++) {
        x = 48 + (i%(KBSIZE/3))*64;
        y = ENTRY_Y2 + 32 + (i/(KBSIZE/3))*64;
        drawRectangle(x, y, x+56, y+56, KEYCOLOR, 1);
        k[0] = (char)kbcurrent[i + kbshift*KBSIZE];
        drawText(C_BLACK, x+24, y+20, k);
    }

    drawRectangle(752, ENTRY_Y2+32, 752+56, ENTRY_Y2+32+56, KEYCOLOR, 1);
    drawText(C_BLACK, 752+16, ENTRY_Y2+32+20, "<-");
    drawRectangle(752, ENTRY_Y2+160, 752+56, ENTRY_Y2+160+56, KEYCOLOR, 1);
    drawText(C_BLACK, 752+16, ENTRY_Y2+160+20, "OK");

    x = 48;
    for (i = 0; i < 5; i++) {
        x += p;
        p = (1 + (kbbottom[i][0]==' '))*128;
        drawRectangle(x, ENTRY_Y2+224, x+p-8, ENTRY_Y2+224+56, KEYCOLOR, 1);
        drawTextCenter(C_BLACK, x+((p-8)>>1), ENTRY_Y2+224+20, kbbottom[i]);
    }

    redraw--;
}
