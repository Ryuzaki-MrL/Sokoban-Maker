/*

Arquivo: util.c
Descrição: funções que não tinham outro lugar pra ficar

*/

#include <stdio.h>
#include <stdarg.h>
#include <shlobj.h>
#include <sys/stat.h>

#include "util.h"

static char userbasepath[260] = "";
static char usersavepath[260] = "";
static char userlevelpath[260] = "";

// Implementação do ShellSort
void arraySort(unsigned arr[], unsigned n) {
    unsigned h = 1, i, j, e;
    while(h < n)
        h = h * 3 + 1;
    while(h > 1) {
        h /= 3;
        for (i = h; i < n; i++) {
            e = arr[i];
            j = i - h;
            while(j >= 0 && e < arr[j]) {
                arr[j + h] = arr[j];
                j -= h;
            }
            arr[j + h] = e;
        }
    }
}

void initUserFolder() {
    SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, userbasepath);
    sprintf(userbasepath, "%s/Roboban", userbasepath);
    sprintf(usersavepath, "%s/savedata.bin", userbasepath);
    sprintf(userlevelpath, "%s/levels", userbasepath);
    mkdir(userbasepath);
    mkdir(userlevelpath);
}

const char* getUserPath() {
    return userbasepath;
}

const char* getSavePath() {
    return usersavepath;
}

const char* getLevelPath() {
    return userlevelpath;
}

char* strhex(char* out, const char* str, int len) {
    char* hexresult = (char*)calloc(1, len*2 + 1);
    int i;
    for (i = 0; i < len; i++) {
        sprintf(hexresult + (i<<1), "%02x", str[i]&0xff);
    }
    strcpy(out, hexresult);
    free(hexresult);
    return out;
}

void logs(const char* msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
}

void logsf(const char* msg, ...) {
    va_list valist;
    va_start(valist, msg);
    vfprintf(stderr, msg, valist);
    va_end(valist);
}

int rectangleCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && h1 + y1 > y2);
}
