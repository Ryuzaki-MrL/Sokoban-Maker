#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "save.h"
#include "util.h"

// Estrutura do savefile
// "padding" é usado para deixar o arquivo com
// exatamente 512 bytes. Apenas uma preferência.
struct sSave {
    unsigned char version;
    unsigned char level;
    unsigned char gameclear;
    unsigned char lang;
    unsigned besttimes[20 * 5];
    unsigned int rseed;
    unsigned char padding[104];
};

static struct sSave savefile = { 0 };
static ranksorted = 0;

// Ordena o ranking. Chamada sempre
// que um novo tempo é adicionado
static void sortBestTimes() {
    if (ranksorted) return;
    int i;
    for (i = 0; i < 20; i++)
        arraySort(savefile.besttimes + i*5, 5);
    ranksorted = 1;
}

void newGame() {
    savefile.level = savefile.gameclear = 0;
    srand(time((time_t*)&savefile.rseed));
}

int saveGame() {
    FILE* sf = fopen(getSavePath(), "wb");
    if (!sf) return 0;
    int result = fwrite(&savefile, 1, sizeof(savefile), sf)==sizeof(savefile);
    fclose(sf);
    return result;
}

int loadGame() {
    FILE* sf = fopen(getSavePath(), "rb");
    if (!sf) return 0;
    int result = fread(&savefile, 1, sizeof(savefile), sf)==sizeof(savefile);
    fclose(sf);
    srand(savefile.rseed);
    ranksorted = 0;
    return result;
}

int deleGame() {
    int lang = savefile.lang;
    memset(&savefile, 0, sizeof(savefile));
    srand(time((time_t*)&savefile.rseed));
    memset(savefile.besttimes, 0xFF, sizeof(savefile.besttimes));
    savefile.lang = lang;
    ranksorted = 0;
    return remove(getSavePath());
}

int getLevel() {
    return savefile.level;
}

void setLevel(int lvl) {
    savefile.level = lvl;
    if (lvl == 0xFF) savefile.gameclear = 1;
}

int getLanguage() {
    return savefile.lang;
}

void setLanguage(int lang) {
    savefile.lang = lang & 0xFF;
}

int isGameClear() {
    return savefile.gameclear;
}

const unsigned* getBestTimes() {
    sortBestTimes();
    return savefile.besttimes;
}

void setBestTime(int lvl, unsigned t) {
    sortBestTimes();
    savefile.besttimes[lvl*5 + 4] = t;
    ranksorted = 0;
}
