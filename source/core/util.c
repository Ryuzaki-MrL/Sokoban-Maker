#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#if !defined(_3DS) && !defined(__wiiu__)
    #include <shlobj.h>
#endif

#include "util.h"

static char userbasepath[260] = "";
static char userlevelpath[260] = "";

void initUserFolder() {
#ifdef _3DS
    strcpy(userbasepath, "/3ds");
#elif defined(__wiiu__)
    strcpy(userbasepath, "sd:/wiiu/apps");
#else
    SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, userbasepath);
#endif
    sprintf(userbasepath, "%s/Sokoban_Maker", userbasepath);
    sprintf(userlevelpath, "%s/levels", userbasepath);
#if defined(_3DS) || defined(__wiiu__)
    mkdir(userbasepath, 0777);
    mkdir(userlevelpath, 0777);
#else
    mkdir(userbasepath);
    mkdir(userlevelpath);
#endif
}

const char* getUserPath() {
    return userbasepath;
}

const char* getLevelPath() {
    return userlevelpath;
}

char* strhex(char* out, const char* str, int len) {
    char hexresult[(len<<1) + 1];
    int i;
    for (i = 0; i < len; i++) {
        sprintf(hexresult + (i<<1), "%02x", str[i]&0xff);
    }
    strcpy(out, hexresult);
    return out;
}

int rectangleCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && h1 + y1 > y2);
}
