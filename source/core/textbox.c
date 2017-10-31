#include <string.h>

#include "draw.h"
#include "update.h"
#include "util.h"

static char msgcaption[260] = "";
static int seloption = 0;

static clb errorclb = NULL;
static qcb questionclb = NULL;

void error(const char* caption, clb callback) {
    if (!caption) return;
    hud = H_MESSAGE;
    errorclb = callback;
    strncpy(msgcaption, caption, sizeof(msgcaption)-1);
}

void question(const char* question, qcb callback) {
    if (!question) return;
    hud = H_QUESTION;
    questionclb = callback;
    seloption = 0;
    strncpy(msgcaption, question, sizeof(msgcaption)-1);
}

void updateError() {
    if (isKeyDown(KEY_ANY)) {
        hud = 0;
        if (errorclb) errorclb();
    }
}

#define ENTRY_X1    (DISPLAY_WIDTH>>2)
#define ENTRY_X2    (DISPLAY_WIDTH - ENTRY_X1)
#define ENTRY_Y1    (64)
#define ENTRY_Y2    (ENTRY_Y1 + 120)
#define ENTRY_W     (ENTRY_X2 - ENTRY_X1)

#define BTN_OFFSET  (ENTRY_W/10)
#define BTN_WIDTH   (BTN_OFFSET + (ENTRY_W/3))
#define BTN_HEIGHT  ((BTN_OFFSET>>1) + ((ENTRY_W/3)/3))

void updateQuestion() {
    if (isKeyDown(KEY_LEFT) || rectangleCollision(getMouseX(), getMouseY(), 1, 1, ENTRY_X1 + BTN_OFFSET, ENTRY_Y2 - BTN_HEIGHT, BTN_WIDTH, BTN_HEIGHT - (BTN_OFFSET>>1))) {
        seloption = 0;
    } else if (isKeyDown(KEY_RIGHT) || rectangleCollision(getMouseX(), getMouseY(), 1, 1, ENTRY_X2 - BTN_WIDTH, ENTRY_Y2 - BTN_HEIGHT, BTN_WIDTH, BTN_HEIGHT - (BTN_OFFSET>>1))) {
        seloption = 1;
    }
    if (isKeyDown(KEY_CONFIRM) || getMouseButton()==MBT_LEFT) {
        hud = 0;
        if (questionclb) questionclb(!seloption);
    } else if (isKeyDown(KEY_CANCEL)) {
        hud = 0;
    }
}

void drawError() {
    drawRectangle(ENTRY_X1, ENTRY_Y1, ENTRY_X2, ENTRY_Y2, RGBA8(255,255,255,220), 1);
    drawTextMultiline(RGBA8(16, 16, 16, 255), (DISPLAY_WIDTH>>1), ENTRY_Y1 + 16, (ENTRY_W - 32), 1, msgcaption);
}

#define BTN_COLOR   RGBA8(240,240,240,255)
#define SEL_COLOR   RGBA8(240,16,16,255)

void drawQuestion() {
    drawError();
    drawRectangle(ENTRY_X1 + BTN_OFFSET, ENTRY_Y2 - BTN_HEIGHT, ENTRY_X1 + BTN_WIDTH, ENTRY_Y2 - (BTN_OFFSET>>1), (seloption==0) ? SEL_COLOR : BTN_COLOR, 1);
    drawRectangle(ENTRY_X2 - BTN_WIDTH, ENTRY_Y2 - BTN_HEIGHT, ENTRY_X2 - BTN_OFFSET, ENTRY_Y2 - (BTN_OFFSET>>1), (seloption==1) ? SEL_COLOR : BTN_COLOR, 1);
    drawTextCenter(RGBA8(16, 16, 16, 255), ENTRY_X1 + (BTN_WIDTH>>1) + (BTN_OFFSET>>1), ENTRY_Y2 - BTN_HEIGHT, getMessage(MSG_YES));
    drawTextCenter(RGBA8(16, 16, 16, 255), ENTRY_X2 - (BTN_WIDTH>>1) - (BTN_OFFSET>>1), ENTRY_Y2 - BTN_HEIGHT, getMessage(MSG_NO));
}
