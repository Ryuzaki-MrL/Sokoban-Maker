#include <stdio.h>
#include <stdlib.h>

#include "network.h"
#include "update.h"
#include "draw.h"
#include "util.h"

#define TICKS_PER_SECOND 60
#define SKIP_TICKS (1000 / TICKS_PER_SECOND)
#define MAX_FRAMESKIP 5

int state = ST_TITLE;
int hud = 0;
int running = 1;

int main() {
    romfsInit();
    sdmcInit();
    initUserFolder();
    drawInit();
    networkInit();

    u64 next_t = osGetTime();

    while(aptMainLoop() && running) {
        int loops = 0;

        while(osGetTime() > next_t && loops < MAX_FRAMESKIP) {
            updateInput();
            update();

            next_t += SKIP_TICKS;
            loops++;
        }

        draw();
    }

    networkFini();
    drawFini();
    sdmcExit();
    romfsExit();

    return 0;
}
