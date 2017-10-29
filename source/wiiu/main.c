#include <stdio.h>
#include <stdlib.h>

#include "level.h"
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

int mainfunc() {
    initUserFolder();
    drawInit();
    networkInit();

    u64 next_t = OSGetTime();

    while(running) {
        int loops = 0;

        while(OSGetTime() > next_t && loops < MAX_FRAMESKIP) {
            updateInput();
            if (isKeyDown(VPAD_BUTTON_HOME))
                running = 0;

            update();

            next_t += SKIP_TICKS;
            loops++;
        }

        draw();
    }

    newLevel();
    networkFini();
    drawFini();

    return 0;
}
