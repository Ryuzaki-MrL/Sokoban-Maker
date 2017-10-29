#ifndef DRAW_H
#define DRAW_H

#include "message.h"
#include "sprite.h"

#ifdef _3DS
    #define DISPLAY_WIDTH 400
    #define DISPLAY_HEIGHT 240
    void drawGridAux(int dx, int dy);
    typedef u32 Color;
#elif defined(__wiiu__)
    #define RGBA8(r,g,b,a) ((((a)&0xFF)<<0) | (((b)&0xFF)<<8) | (((g)&0xFF)<<16) | (((r)&0xFF)<<24))
    #define DISPLAY_WIDTH 854
    #define DISPLAY_HEIGHT 480
    typedef u32 Color;
#else
    #define RGBA8(r,g,b,a) al_map_rgba(r,g,b,a)
    #define DISPLAY_WIDTH 640
    #define DISPLAY_HEIGHT 480
    typedef ALLEGRO_COLOR Color;
#endif

#define C_BLACK RGBA8(0, 0, 0, 255)
#define C_WHITE RGBA8(255, 255, 255, 255)

void drawInit();
void drawFini();

void drawClearColor(Color color);
void drawBackground();
void drawLogo();

void drawText(Color color, int x, int y, const char* str);
void drawTextFormat(Color color, int x, int y, const char* str, ...);
void drawTextCenter(Color color, int x, int y, const char* str);
void drawTextMultiline(Color color, int x, int y, int w, int centre, const char* str);
void drawRectangle(int x1, int y1, int x2, int y2, Color color, int fill);

void drawTile(int tile, int x, int y);
void drawSprite(int sprite, int frame, int x, int y);

void drawLevel();

void drawSetBackground(const char* fname);

void draw();

#endif // DRAW_H