#ifndef DRAW_H
#define DRAW_H

#include "sprite.h"

#define C_BLACK al_map_rgb(0, 0, 0)
#define C_WHITE al_map_rgb(255, 255, 255)

void drawInit();
void drawFini();

void drawSetMessageCaption(const char* str);
void drawSetBackground(const char* fname);
void drawToggleGrid();

void draw();

#endif // DRAW_H