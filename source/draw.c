/*

Arquivo: draw.c
Descrição: contém todas as rotinas que renderizam o jogo

Não repare a bagunça.

*/

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <string.h>

#include "draw.h"
#include "entity.h"
#include "save.h"
#include "state.h"
#include "message.h"

static ALLEGRO_FONT* font = NULL;
static ALLEGRO_BITMAP* logo = NULL;
static ALLEGRO_BITMAP* tileset = NULL;
static ALLEGRO_BITMAP* background = NULL;

static char msgcaption[260] = "";

static int showgrid = 1;

static void drawTilemap();
static void drawEntities();
static void drawHud();
static void drawUserInput();
static void drawMessageCaption();
static void drawTitleScreen();
static void drawRankScreen();
static void drawMainGame();
static void drawPauseMenu();
static void drawLevelList();
static void drawLevelEditor();

// Desenharia o background, caso o jogo tivesse algum...
static void drawBackground() {
    if (background) {
        al_draw_bitmap(background, 0, 0, 0);
    } else {
        al_clear_to_color(al_map_rgb(0xC0, 0xC0, 0xC0));
    }
}

void drawInit() {
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

    font = al_load_ttf_font("alterebo.ttf", 28, ALLEGRO_TTF_MONOCHROME);
    logo = al_load_bitmap("logo.png");
    tileset = al_load_bitmap("sprites/tileset.png");

    loadSprites();
}

void drawFini() {
    al_destroy_font(font);
    al_destroy_bitmap(logo);
    al_destroy_bitmap(tileset);
    al_destroy_bitmap(background);
    freeSprites();
}

void drawSetMessageCaption(const char* str) {
    strncpy(msgcaption, str, sizeof(msgcaption)-1);
}

void drawSetBackground(const char* fname) {
    if (background)
        al_destroy_bitmap(background);
    background = al_load_bitmap(fname);
}

// Usado para ativar/desativar a grade no editor de níveis
void drawToggleGrid() {
    showgrid ^= 1;
}

// Renderiza o tilemap do nível
// Função baseada no exemplo de tilemap do allegro
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
    for (y = oy; y < oy+30; y++) {
        for (x = ox; x < ox+40; x++) {
            int i = level.tilemap[x + y*1000] - 1;
            if (i < 0) continue;
            al_draw_bitmap_region(tileset, i<<5, 0, 32, 32, (x%40)<<5, (y%30)<<5, 0);
        }
    }
    al_hold_bitmap_drawing(0);

    al_identity_transform(&transform);
    al_use_transform(&transform);
}

// Função auxiliar que renderiza um objeto
static void drawEntityAux(void* data) {
    entity_t* ent = (entity_t*)data;
    const sprite_t* spr = getSprite(ent->sprite);
    if (!ent || !ent->visible || !spr || !spr->bitmap) return;

    if (ent->frame >= spr->frames)
        ent->frame = ((int)ent->frame) % spr->frames;

    int i = ((int)ent->frame) << 5;
    int f = (ent->xdir == -1) ? ALLEGRO_FLIP_HORIZONTAL : 0;
    al_draw_tinted_scaled_rotated_bitmap_region(spr->bitmap, i, 0, spr->width, spr->height, C_WHITE, 0, 0, ent->x, ent->y, 1.f, 1.f, 0.f, f);
}

// Percorre a lista de objetos e renderiza cada um usando a função acima
static void drawEntities() {
    ALLEGRO_TRANSFORM transform;
    int scx = level.cam.scx;
    int scy = level.cam.scy;

    al_identity_transform(&transform);
    al_translate_transform(&transform, -scx, -scy);
    al_use_transform(&transform);

    int i;
    for (i = 0; i < ENT_COUNT; i++) {
        foreach(getEntityList(i), drawEntityAux);
    }

    al_identity_transform(&transform);
    al_use_transform(&transform);
}

// Usado apenas para debug
static void drawMainHud() {
#ifdef DEBUG_MODE
    if (font) {
        unsigned t = level.timer;
        al_draw_textf(font, C_WHITE, 0, 0, 0, getMessage(MSG_TIME), t/60, t%60);
        al_draw_textf(font, C_WHITE, 0, 20, 0, "Robot: (%i, %i)", getRobotX(), getRobotY());
        al_draw_textf(font, C_WHITE, 0, 40, 0, "Camera: (%i, %i)", level.cam.scx, level.cam.scy);
        al_draw_textf(font, C_WHITE, 0, 60, 0, "Boxes: %i", level.boxes);
    }
#endif
}

void draw() {
    al_clear_to_color(C_BLACK);
    switch(state) {
        case ST_PAUSE: drawPauseMenu(); break;
        case ST_RANK: drawRankScreen(); break;
        case ST_TITLE: drawTitleScreen(); break;
        case ST_LEVELS: drawLevelList(); break;
        case ST_EDITOR: drawLevelEditor(); break;
        default: drawMainGame(); break;
    }
    switch(hud) {
        case H_INPUT: drawUserInput(); break;
        case H_MESSAGE: drawMessageCaption(); break;
    }
    al_flip_display();
}

static void drawMessageCaption() {
    al_draw_filled_rectangle(180, 164, 460, 260, al_map_rgba(255,255,255,220));
    if (font) {
        al_draw_multiline_text(font, al_map_rgb(16, 16, 16), 320, 180, 260, 20, ALLEGRO_ALIGN_CENTRE, msgcaption);
    }
}

static void drawUserInput() {
    al_draw_filled_rectangle(160, 180, 480, 300, al_map_rgb(220,220,220));
    al_draw_filled_rectangle(184, 232, 456, 264, C_WHITE);
    if (font) {
        al_draw_text(font, al_map_rgb(16, 16, 16), 320, 196, ALLEGRO_ALIGN_CENTRE, msgcaption);
        al_draw_text(font, al_map_rgb(16, 16, 16), 320, 232, ALLEGRO_ALIGN_CENTRE, userinput);
    }
}

static void drawTitleScreen() {
    al_clear_to_color(al_map_rgb(0x7F, 0x7F, 0x7F));
    if (logo)
        al_draw_bitmap(logo, 96, 96, 0);
    if (font) {
        switch(screen) {
            case 0: {
                al_draw_text(font, C_BLACK, 220, 300, 0, getMessage(MSG_NEWGAME));
                al_draw_text(font, C_BLACK, 220, 320, 0, getMessage(MSG_CONTINUE));
                al_draw_text(font, C_BLACK, 220, 340, 0, getMessage(MSG_EDITOR));
                al_draw_text(font, C_BLACK, 220, 360, 0, getMessage(MSG_RANKING));
                al_draw_textf(font,C_BLACK, 220, 380, 0, getMessage(MSG_LANGUAGE), getLangString(getLanguage()));
                al_draw_text(font, C_BLACK, 220, 400, 0, getMessage(MSG_EXIT));
                al_draw_filled_rectangle(208, 308+cursor*20, 216, 316+cursor*20, C_BLACK);
                break;
            }
            case 1: {
                al_draw_text(font, C_BLACK, 220, 340, 0, getMessage(MSG_NEWLEVEL));
                al_draw_text(font, C_BLACK, 220, 360, 0, getMessage(MSG_LOADLEVEL));
                al_draw_text(font, C_BLACK, 220, 380, 0, getMessage(MSG_GOBACK));
                al_draw_filled_rectangle(208, 348+cursor*20, 216, 356+cursor*20, C_BLACK);
            }
        }
    }
}

static void drawRankScreen() {
    al_clear_to_color(C_WHITE);
    if (font) {
        int i;
        const unsigned* timelist = getBestTimes();
        for (i = 0; i < 5; i++) {
            unsigned t = timelist[cursor*5 + i];
            al_draw_text(font, C_BLACK, 320, 80, ALLEGRO_ALIGN_CENTRE, getMessage(MSG_BESTTIMES));
            al_draw_textf(font, C_BLACK, 320, 100, ALLEGRO_ALIGN_CENTRE, getMessage(MSG_LEVEL), cursor+1);
            if (t < 0xFFFF) {
                al_draw_textf(font, C_BLACK, 320, 200+i*20, ALLEGRO_ALIGN_CENTRE, "%dº: %02u:%02u", i+1, t/60, t%60);
            } else {
                al_draw_textf(font, C_BLACK, 320, 200+i*20, ALLEGRO_ALIGN_CENTRE, "%dº: --:--", i+1);
            }
        }
    }
}

static void drawMainGame() {
    drawBackground();
    drawTilemap();
    drawEntities();
    drawMainHud();
}

static void drawPauseMenu() {
    drawBackground();
    al_draw_filled_rectangle(80, 80, 560, 400, al_map_rgba(255,255,255,220));
    switch(screen) {
        case 0: {
            al_draw_text(font, C_BLACK, 220, 210, 0, getMessage(MSG_RESUME));
            al_draw_text(font, C_BLACK, 220, 230, 0, getMessage(MSG_RESTART));
            al_draw_text(font, C_BLACK, 220, 250, 0, getMessage(MSG_EXIT));
            al_draw_filled_rectangle(208, 218+cursor*20, 216, 226+cursor*20, C_BLACK);
            break;
        }
        case 1: {
            al_draw_text(font, C_BLACK, 220, 210, 0, getMessage(MSG_RESUME));
            al_draw_text(font, C_BLACK, 220, 230, 0, getMessage(MSG_SAVELEVEL));
            al_draw_text(font, C_BLACK, 220, 250, 0, getMessage(MSG_PLAYLEVEL));
            al_draw_text(font, C_BLACK, 220, 270, 0, getMessage(MSG_EXIT));
            al_draw_filled_rectangle(208, 218+cursor*20, 216, 226+cursor*20, C_BLACK);
            break;
        }
    }
}

static void drawLevelList() {
    al_clear_to_color(C_WHITE);
    al_draw_filled_rectangle(80, 180, 560, 300, al_map_rgba(255,200,16,220));

    const list_t* l = getUserLevelList(0);
    if (l->size == 0) {
        al_draw_text(font, C_BLACK, DISPLAY_WIDTH>>1, DISPLAY_HEIGHT>>1, ALLEGRO_ALIGN_CENTRE, "¯\\_(\"-.-)_/¯");
    } else {
        int i = 0;
        void drawLevelEntry(void* data) {
            levelmeta_t* m = (levelmeta_t*)data;
            int yoffs = (i++)*132 - cursor*132;
            al_draw_filled_rectangle(88, 180+yoffs, 552, 300+yoffs, al_map_rgba(220,220,220,240));
            al_draw_text(font, C_BLACK, 96, 200+yoffs, 0, m->title);
            al_draw_text(font, C_BLACK, 96, 220+yoffs, 0, m->author);
            al_draw_textf(font, C_BLACK, 96, 240+yoffs, 0, "%.2lf KB", m->filesize/1024.0);
            // TODO: previews dos níveis
        }
        foreach(l, drawLevelEntry);
    }
}

static void drawLevelEditor() {
    drawBackground();
    drawTilemap();
    drawEntities();
    if (showgrid) {
        int ox = level.cam.scx >> 5;
        int oy = level.cam.scy >> 5;
        int x, y;
        for (y = oy; y < oy+30; y++) {
            for (x = ox; x < ox+40; x++) {
                int dx = ((x%40) << 5) - (level.cam.scx & 31);
                int dy = ((y%30) << 5) - (level.cam.scy & 31);
                al_draw_rectangle(dx, dy, dx+32, dy+32, C_BLACK, 1);
            }
        }
    }
    al_draw_filled_rectangle(DISPLAY_WIDTH-80, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, C_WHITE);
    al_draw_filled_rectangle(DISPLAY_WIDTH-64, 32+cursor*48, DISPLAY_WIDTH-16, 80+cursor*48, al_map_rgb(255,0,0));
    int i;
    for (i = 0; i < TILE_COUNT; i++)
        al_draw_bitmap_region(tileset, i<<5, 0, 32, 32, DISPLAY_WIDTH-56, 40+i*48, 0);
    for (i = TILE_COUNT; i < ENT_COUNT+TILE_COUNT; i++)
        al_draw_bitmap_region(getSprite(i-TILE_COUNT)->bitmap, 0, 0, 32, 32, DISPLAY_WIDTH-56, 40+i*48, 0);
    drawMainHud();
}
