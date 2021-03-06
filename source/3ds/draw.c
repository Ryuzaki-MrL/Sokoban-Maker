#include <string.h>
#include <stdarg.h>

#include "draw.h"
#include "entity.h"
#include "input.h"
#include "state.h"
#include "pp2d/ppft.h"

#define FONT_SIZE   0.5
#define FONT_YOFF   4

#define TEXTURE_LOGO    (SPRITE_COUNT + 1)
#define TEXTURE_TILESET (SPRITE_COUNT + 2)
#define TEXTURE_BG      (SPRITE_COUNT + 3)
#define TEXTURE_GRID    (SPRITE_COUNT + 4)

static gfxScreen_t targscreen = GFX_TOP;
static gfxScreen_t levelscreen = GFX_BOTTOM;
static ppftFont* font = NULL;

void drawInit() {
    pp2d_init();

    pp2d_load_texture_png(TEXTURE_LOGO, "romfs:/logo3ds.png");
    pp2d_load_texture_png(TEXTURE_TILESET, "romfs:/tileset.png");
    pp2d_load_texture_png(TEXTURE_GRID, "romfs:/grid.png");
    font = ppft_load_font("romfs:/alterebro.ppft");

    loadSprites();
}

void drawFini() {
    ppft_free_font(font);
    pp2d_exit();
}

void drawClearColor(Color color) {
    Color c = ABGR8((color>>24)&0xFF, (color>>16)&0xFF, (color>>8)&0xFF, color&0xFF);
    pp2d_set_screen_color(GFX_TOP, c);
    pp2d_set_screen_color(GFX_BOTTOM, c);
}

void drawBackground() {
    drawClearColor(BG_COLOR);
}

void drawLogo() {
    pp2d_draw_texture(TEXTURE_LOGO, (DISPLAY_WIDTH>>1) - 111, 64);
}

void drawText(Color color, int x, int y, const char* str) {
    if (font) {
        ppft_draw_text(font, x, y, 1, 1, color, 0, 0, str);
    } else {
        pp2d_draw_text(x, y+FONT_YOFF, FONT_SIZE, FONT_SIZE, color, str);
    }
}

void drawTextFormat(Color color, int x, int y, const char* str, ...) {
    char buffer[256];
    va_list valist;
    va_start(valist, str);
    vsnprintf(buffer, 256, str, valist);
    if (font) {
        ppft_draw_text(font, x, y, 1, 1, color, 0, 0, buffer);
    } else {
        pp2d_draw_text(x, y+FONT_YOFF, FONT_SIZE, FONT_SIZE, color, buffer);
    }
    va_end(valist);
}

void drawTextCenter(Color color, int x, int y, const char* str) {
    if (font) {
        ppft_draw_text(font, x, y, 1, 1, color, 0, 1, str);
    } else {
        x -= ((int)pp2d_get_text_width(str, FONT_SIZE, FONT_SIZE) >> 1);
        pp2d_draw_text(x, y+FONT_YOFF, FONT_SIZE, FONT_SIZE, color, str);
    }
}

void drawTextMultiline(Color color, int x, int y, int w, int centre, const char* str) {
    if (font) {
        ppft_draw_text(font, x, y, 1, 1, color, w, centre, str);
    } else {
        if (centre) {
            drawTextCenter(color, x, y, str);
        } else {
            pp2d_draw_text_wrap(x, y+FONT_YOFF, FONT_SIZE, FONT_SIZE, color, w, str);
        }
    }
}

void drawRectangle(int x1, int y1, int x2, int y2, Color color, int fill) {
    // TODO: not fill
    pp2d_draw_rectangle(x1, y1, x2-x1, y2-y1, color);
}

void drawTile(int tile, int x, int y) {
    pp2d_texture_select_part(TEXTURE_TILESET, x, y, tile<<5, 0, 32, 32);
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
            pp2d_draw_texture_part(TEXTURE_TILESET, (x<<5) - level.cam.scx, (y<<5) - level.cam.scy, i<<5, 0, 32, 32);
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
    entityForeach(drawEntityAux, ENT_ANY);
}

void drawSetBackground(const char* fname) {
    pp2d_load_texture_png(TEXTURE_BG, fname);
}

void drawGridAux(int dx, int dy) {
    pp2d_draw_texture(TEXTURE_GRID, dx, dy);
}

void drawLevel() {
    drawBackground();
    drawTilemap();
    drawEntities();
}

static void drawSetScreen(gfxScreen_t targ) {
    if (targscreen != targ) {
        targscreen = targ;
        pp2d_draw_on(targ, GFX_LEFT);
    }
}

void drawToggleLevelScreen() {
    levelscreen = !levelscreen;
}

int drawGetScreenWidth() {
    return ((targscreen == GFX_TOP) ? 400 : 320);
}

int drawGetScreenHeight() {
    return 240;
}

void draw() {
    pp2d_begin_draw(GFX_TOP, GFX_LEFT);
    targscreen = GFX_TOP;
    if (state == ST_EDITOR) {
        drawSetScreen(GFX_BOTTOM);
    } else {
        drawSetScreen((state == ST_MAIN) ? levelscreen : GFX_TOP);
    }
    switch(state) {
        case ST_PAUSE: drawPauseMenu(); break;
        case ST_TITLE: drawTitleScreen(); break;
        case ST_LEVELS: drawLevelList(); break;
        case ST_EDITOR: drawLevelEditor(); break;
        default: drawLevel(); break;
    }
    drawSetScreen(GFX_BOTTOM);
    switch(hud) {
        case H_MESSAGE: drawError(); break;
        case H_QUESTION: drawQuestion(); break;
    }
    if (!hud && state == ST_MAIN) {
        drawSetScreen(levelscreen);
    }
    pp2d_end_draw();
}
