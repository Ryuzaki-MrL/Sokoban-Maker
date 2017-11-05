#include <string.h>
#include <stdarg.h>

#include "draw.h"
#include "entity.h"
#include "input.h"
#include "state.h"

#define FONT_SIZE   0.5
#define FONT_YOFF   4

//static int font = 1000;
static int logo = SPRITE_COUNT + 1;
static int tileset = SPRITE_COUNT + 2;
static int background = SPRITE_COUNT + 3;
static int grid = SPRITE_COUNT + 4;

void drawInit() {
    pp2d_init();

    pp2d_load_texture_png(logo, "romfs:/logo3ds.png");
    pp2d_load_texture_png(tileset, "romfs:/tileset.png");
    pp2d_load_texture_png(grid, "romfs:/grid.png");
    // TODO: load font

    loadSprites();
}

void drawFini() {
    pp2d_exit();
}

void drawClearColor(Color color) {
    Color c = ABGR8((color>>24)&0xFF, (color>>16)&0xFF, (color>>8)&0xFF, color&0xFF);
    pp2d_set_screen_color(GFX_TOP, c);
    pp2d_set_screen_color(GFX_BOTTOM, c);
}

void drawBackground() {
    pp2d_set_screen_color(GFX_TOP, ABGR8(0xFF, 0xC0, 0xC0, 0xC0));
    pp2d_set_screen_color(GFX_BOTTOM, ABGR8(0xFF, 0xC0, 0xC0, 0xC0));
}

void drawLogo() {
    pp2d_draw_texture(logo, (DISPLAY_WIDTH>>1) - 111, 96);
}

void drawText(Color color, int x, int y, const char* str) {
    pp2d_draw_text(x, y+FONT_YOFF, FONT_SIZE, FONT_SIZE, color, str);
}

void drawTextFormat(Color color, int x, int y, const char* str, ...) {
    char buffer[256];
    va_list valist;
    va_start(valist, str);
    vsnprintf(buffer, 255, str, valist);
    pp2d_draw_text(x, y+FONT_YOFF, FONT_SIZE, FONT_SIZE, color, buffer);
    va_end(valist);
}

void drawTextCenter(Color color, int x, int y, const char* str) {
    x -= ((int)pp2d_get_text_width(str, FONT_SIZE, FONT_SIZE) >> 1);
    pp2d_draw_text(x, y+FONT_YOFF, FONT_SIZE, FONT_SIZE, color, str);
}

void drawTextMultiline(Color color, int x, int y, int w, int centre, const char* str) {
    if (centre) {
        drawTextCenter(color, x, y, str);
    } else {
        pp2d_draw_text_wrap(x, y+FONT_YOFF, FONT_SIZE, FONT_SIZE, color, w, str);
    }
}

void drawRectangle(int x1, int y1, int x2, int y2, Color color, int fill) {
    // TODO: not fill
    pp2d_draw_rectangle(x1, y1, x2-x1, y2-y1, color);
}

void drawTile(int tile, int x, int y) {
    pp2d_texture_select_part(tileset, x, y, tile<<5, 0, 32, 32);
    pp2d_texture_scale(0.75, 0.75);
    pp2d_texture_draw();
}

void drawSprite(int sprite, int frame, int x, int y) {
    pp2d_texture_select_part(sprite, x, y, frame<<5, 0, 32, 32);
    pp2d_texture_scale(0.75, 0.75);
    pp2d_texture_draw();
}

static void drawTilemap() {
    int x, y;

    int ox = level.cam.scx >> 5;
    int oy = level.cam.scy >> 5;
    for (y = oy; y < oy+25 && y < TILE_ROW; y++) {
        for (x = ox; x < ox+15 && x < TILE_ROW; x++) {
            int i = level.tilemap[x + y*TILE_ROW] - 1;
            if (i < 0) continue;
            pp2d_draw_texture_part(tileset, (x<<5) - level.cam.scx, (y<<5) - level.cam.scy, i<<5, 0, 32, 32);
        }
    }
}

static void drawEntityAux(void* data) {
    entity_t* ent = (entity_t*)data;
    const sprite_t* spr = getSprite(ent->sprite);
    if (!ent || !ent->visible || !spr) return;

    if (ent->frame >= spr->frames)
        ent->frame = ((int)ent->frame) % spr->frames;

    int i = ((int)ent->frame) << 5;
    pp2d_draw_texture_part(ent->sprite, ent->x - level.cam.scx, ent->y - level.cam.scy, i, 0, spr->width, spr->height);
}

static void drawEntities() {
    int i;
    for (i = 0; i < ENT_COUNT; i++) {
        foreach(getEntityList(i), drawEntityAux);
    }
}

void drawSetBackground(const char* fname) {
    pp2d_load_texture_png(background, fname);
}

void drawGridAux(int dx, int dy) {
    pp2d_draw_texture(grid, dx, dy);
}

void drawLevel() {
    drawBackground();
    drawTilemap();
    drawEntities();
}

void draw() {
    pp2d_begin_draw(GFX_TOP, GFX_LEFT);
    if (state == ST_EDITOR) {
        pp2d_draw_on(GFX_BOTTOM, GFX_LEFT);
    }
    switch(state) {
        case ST_PAUSE: drawPauseMenu(); break;
        case ST_TITLE: drawTitleScreen(); break;
        case ST_LEVELS: drawLevelList(); break;
        case ST_EDITOR: drawLevelEditor(); break;
        default: drawLevel(); break;
    }
    if (state != ST_EDITOR) {
        pp2d_draw_on(GFX_BOTTOM, GFX_LEFT);
    }
    switch(hud) {
        case H_MESSAGE: drawError(); break;
        case H_QUESTION: drawQuestion(); break;
    }
    pp2d_end_draw();
}
