#include "network.h"
#include "update.h"
#include "draw.h"

static int cursor[] = { 0, 0 };
static int screen = 0;

void inputPasswordCallback(const char* password) {
    httpAddPostFieldText("pass", password[0] ? password : "guest");
    char v[] = {'0'+VERSION_MAJOR, '.', '0'+VERSION_MINOR, '.', '0'+VERSION_MICRO, '\0'};
    httpAddPostFieldText("version", v);
    const char* res = httpPost();
    if (httpGetResponseCode() != 200) {
        error(res[0] ? res : getMessage(MSG_CONNFAIL), NULL);
    } else {
        screen = 1;
    }
    httpEndConnection();
}

void inputUsernameCallback(const char* username) {
    httpAddPostFieldText("user", username[0] ? username : "guest");
    getUserInput(20, getMessage(MSG_PASSWORD), 1, inputPasswordCallback);
}

void updateTitleScreen() {
    static int cursormax[] = { 4, 4 };
    if (isKeyDown(KEY_DOWN) && cursor[screen] < cursormax[screen]) {
        cursor[screen]++;
    }
    if (isKeyDown(KEY_UP) && cursor[screen] > 0) {
        cursor[screen]--;
    }
    if (isKeyDown(KEY_CONFIRM)) {
        switch(screen*10 + cursor[screen]) {
            case 0: { // New Level
                editorStart(NULL);
                break;
            }
            case 1: { // Load Level
                openUserLevelList(LISTMODE_LOAD);
                break;
            }
            case 2: { // Online Levels
                httpStartConnection(URL_ROOT"users/login.php");
                getUserInput(20, getMessage(MSG_USERNAME), 0, inputUsernameCallback);
                break;
            }
            case 3: { // Language
                setLanguage(getLanguage() + 1);
                break;
            }
            case 4: { // Exit
                running = 0;
                break;
            }

            case 10: { // Download Level
                openOnlineLevelList(LISTMODE_DOWNLOAD);
                break;
            }
            case 11: { // Upload Level
                openUserLevelList(LISTMODE_UPLOAD);
                break;
            }
            case 12: { // My Levels
                openOnlineLevelList(LISTMODE_MYLEVELS);
                break;
            }
            case 13: { // My Profile
                break;
            }
            case 14: { // Logout
                httpGet(URL_ROOT"users/logout.php");
                screen = 0;
            }
        }
    }
}

void drawTitleScreen() {
    drawClearColor(RGBA8(0x7F, 0x7F, 0x7F, 0xFF));
    drawLogo();
    if (screen == 0) {
        drawText(      C_BLACK, DISPLAY_WIDTH/3, DISPLAY_HEIGHT - 120, getMessage(MSG_NEWLEVEL));
        drawText(      C_BLACK, DISPLAY_WIDTH/3, DISPLAY_HEIGHT - 100, getMessage(MSG_LOADLEVEL));
        drawText(      C_BLACK, DISPLAY_WIDTH/3, DISPLAY_HEIGHT -  80, getMessage(MSG_ONLINEHUB));
        drawTextFormat(C_BLACK, DISPLAY_WIDTH/3, DISPLAY_HEIGHT -  60, getMessage(MSG_LANGUAGE), getLangString());
        drawText(      C_BLACK, DISPLAY_WIDTH/3, DISPLAY_HEIGHT -  40, getMessage(MSG_EXIT));
    } else {
        drawText(      C_BLACK, DISPLAY_WIDTH/3, DISPLAY_HEIGHT - 120, getMessage(MSG_DOWNLEVEL));
        drawText(      C_BLACK, DISPLAY_WIDTH/3, DISPLAY_HEIGHT - 100, getMessage(MSG_UPLEVEL));
        drawText(      C_BLACK, DISPLAY_WIDTH/3, DISPLAY_HEIGHT -  80, getMessage(MSG_MYLEVELS));
        drawText(      C_BLACK, DISPLAY_WIDTH/3, DISPLAY_HEIGHT -  60, getMessage(MSG_MYPROFILE));
        drawText(      C_BLACK, DISPLAY_WIDTH/3, DISPLAY_HEIGHT -  40, getMessage(MSG_LOGOUT));
    }
    drawRectangle(
        DISPLAY_WIDTH/3 - 12, DISPLAY_HEIGHT - 112 + cursor[screen]*20,
        DISPLAY_WIDTH/3 -  4, DISPLAY_HEIGHT - 104 + cursor[screen]*20, C_BLACK, 1
    );
}
