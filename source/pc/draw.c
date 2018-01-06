#include <string.h>
#include <stdarg.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

#include "draw.h"
#include "robot.h"
#include "input.h"
#include "state.h"

static ALLEGRO_FONT* font = NULL;
static ALLEGRO_BITMAP* logo = NULL;
static ALLEGRO_BITMAP* tileset = NULL;
static ALLEGRO_BITMAP* background = NULL;

void drawInit() {
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

    font = al_load_ttf_font("romfs/alterebo.ttf", 28, ALLEGRO_TTF_MONOCHROME);
    logo = al_load_bitmap("romfs/logo.png");
    tileset = al_load_bitmap("romfs/tileset.png");

    loadSprites();
}

void drawFini() {
    al_destroy_font(font);
    al_destroy_bitmap(logo);
    al_destroy_bitmap(tileset);
    al_destroy_bitmap(background);
    freeSprites();
}

void drawClearColor(Color color) {
    al_clear_to_color(color);
}

void drawBackground() {
    if (background) {
        al_draw_bitmap(background, 0, 0, 0);
    } else {
        al_clear_to_color(BG_COLOR);
    }
}

void drawLogo() {
    if (logo) {
        al_draw_bitmap(logo, 96, 96, 0);
    }
}

void drawText(Color color, int x, int y, const char* str) {
    al_draw_text(font, color, x, y, 0, str);
}

void drawTextFormat(Color color, int x, int y, const char* str, ...) {
    char buffer[256];
    va_list valist;
    va_start(valist, str);
    vsnprintf(buffer, 255, str, valist);
    al_draw_text(font, color, x, y, 0, buffer);
    va_end(valist);
}

void drawTextCenter(Color color, int x, int y, const char* str) {
    al_draw_text(font, color, x, y, ALLEGRO_ALIGN_CENTRE, str);
}

void drawTextMultiline(Color color, int x, int y, int w, int centre, const char* str) {
    al_draw_multiline_text(font, color, x, y, w, 20, centre ? ALLEGRO_ALIGN_CENTRE : 0, str);
}

void drawRectangle(int x1, int y1, int x2, int y2, Color color, int fill) {
    if (fill) {
        al_draw_filled_rectangle(x1, y1, x2, y2, color);
    } else {
        al_draw_rectangle(x1, y1, x2, y2, color, 1);
    }
}

void drawTile(int tile, int x, int y) {
    al_draw_bitmap_region(tileset, tile<<5, 0, 32, 32, x, y, 0);
}

void drawSprite(int sprite, int frame, int x, int y) {
    al_draw_bitmap_region(getSprite(sprite)->bitmap, frame<<5, 0, 32, 32, x, y, 0);
}

// Based on allegro5 tilemap example
static void drawTilemap() {
    if (!tileset) return;

    int x, y;
    ALLEGRO_TRANSFORM transform;
    int scx = level.cam.scx;
    int scy = level.cam.scy;

    al_identity_transform(&transform);
    al_translate_transform(&transform, -scx, -scy);
    al_use_transform(&transform);

    al_hold_bitmap_drawing(1);
    int ox = level.cam.scx >> 5;
    int oy = level.cam.scy >> 5;
    for (y = oy; y < oy+30 && y < TILE_ROW; y++) {
        for (x = ox; x < ox+40 && x < TILE_ROW; x++) {
            int i = level.tilemap[x + y*TILE_ROW] - 1;
            if (i < 0) continue;
            al_draw_bitmap_region(tileset, i<<5, 0, 32, 32, x<<5, y<<5, 0);
        }
    }
    al_hold_bitmap_drawing(0);

    al_identity_transform(&transform);
    al_use_transform(&transform);
}

static void drawEntityAux(void* data) {
    entity_t* ent = (entity_t*)data;
    const sprite_t* spr = getSprite(ent->sprite);
    if (!ent || !ent->visible || !spr || !spr->bitmap) return;

    if (ent->frame >= spr->frames)
        ent->frame = ((int)ent->frame) % spr->frames;

    int i = ((int)ent->frame) << 5;
    int f = (ent->xdir == -1) ? ALLEGRO_FLIP_HORIZONTAL : 0;
    al_draw_bitmap_region(spr->bitmap, i, 0, spr->width, spr->height, ent->x, ent->y, f);
}

static void drawEntities() {
    ALLEGRO_TRANSFORM transform;
    int scx = level.cam.scx;
    int scy = level.cam.scy;

    al_identity_transform(&transform);
    al_translate_transform(&transform, -scx, -scy);
    al_use_transform(&transform);

    entityForeach(drawEntityAux, ENT_ANY);

    al_identity_transform(&transform);
    al_use_transform(&transform);
}

void drawSetBackground(const char* fname) {
    if (background)
        al_destroy_bitmap(background);
    background = al_load_bitmap(fname);
}

void drawLevel() {
    drawBackground();
    drawTilemap();
    drawEntities();
}

void draw() {
    al_clear_to_color(C_BLACK);
    switch(state) {
        case ST_PAUSE: drawPauseMenu(); break;
        case ST_TITLE: drawTitleScreen(); break;
        case ST_LEVELS: drawLevelList(); break;
        case ST_EDITOR: drawLevelEditor(); break;
        default: drawLevel(); break;
    }
    switch(hud) {
        case H_INPUT: drawUserInput(); break;
        case H_MESSAGE: drawError(); break;
        case H_QUESTION: drawQuestion(); break;
    }
    al_flip_display();
}
