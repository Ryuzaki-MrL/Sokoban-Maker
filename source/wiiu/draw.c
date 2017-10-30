#include <string.h>
#include <stdarg.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_IMAGE_H

#include "wiiu.h"

#include "draw.h"
#include "entity.h"
#include "input.h"
#include "state.h"

static unsigned char* screenBuf;
static int scr_buf0_size = 0;
static int scr_buf1_size = 0;
static bool cur_buf1;

static const u8* font = NULL;
static FT_Library ftlib;
static FT_Face ftface;

extern const u8 alterebo_ttf[];
extern const u32 alterebo_ttf_size;

static void drawPixel(int x, int y, Color color) {
    if (x < 0 || y < 0 || x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) return;

    int a = (color >>  0) & 0xFF;
    int b = (color >>  8) & 0xFF;
    int g = (color >> 16) & 0xFF;
    int r = (color >> 24) & 0xFF;
    
    int width = 1280;
    unsigned char* screen = screenBuf;
    int otherBuff0 = scr_buf0_size >> 1;
    int otherBuff1 = scr_buf1_size >> 1;
    float opacity = a / 255.0;

    int xx, yy;
    for (yy = (y * 1.5); yy < ((y * 1.5) + 1); yy++) {
        for (xx = (x * 1.5); xx < ((x * 1.5) + 1); xx++) {
            u32 v = (xx + yy * width) << 2;
            if (cur_buf1) v += otherBuff0;
            screen[v    ] = r * opacity + (1 - opacity) * screen[v];
            screen[v + 1] = g * opacity + (1 - opacity) * screen[v + 1];
            screen[v + 2] = b * opacity + (1 - opacity) * screen[v + 2];
            screen[v + 3] = a;
        }
    }

    width = 896;
    unsigned char* screen2 = screenBuf + scr_buf0_size;
    u32 v = (x + y * width) << 2;
    if (cur_buf1) v += otherBuff1;
    screen2[v    ] = r * opacity + (1 - opacity) * screen2[v];
    screen2[v + 1] = g * opacity + (1 - opacity) * screen2[v + 1];
    screen2[v + 2] = b * opacity + (1 - opacity) * screen2[v + 2];
    screen2[v + 3] = a;
}

static void drawLine(int x1, int y1, int x2, int y2, Color color) {
    int x, y;
    if (x1 == x2) {
        if (y1 < y2) {
            for (y = y1; y <= y2; y++) drawPixel(x1, y, color);
        } else {
            for (y = y2; y <= y1; y++) drawPixel(x1, y, color);
        }
    } else {
        if (x1 < x2) {
            for (x = x1; x <= x2; x++) drawPixel(x, y1, color);
        } else {
            for (x = x2; x <= x1; x++) drawPixel(x, y1, color);
        }
    }
}

static void drawTexture(const sprite_t* spr, int x, int y) {
    if (!spr) return;
    const u32* buffer = (const u32*)spr->pixel_data;
    unsigned i, j;
    for (j = y; j < (y + spr->height); j++) {
        for (i = x; i < (x + spr->width); i++) {
            drawPixel(i, j, buffer[(i - x) + ((j - y) * spr->width * spr->frames)]);
        }
    }
}

static void drawTexturePart(const sprite_t* spr, int x, int y, int xbeg, int ybeg, int wpart, int hpart) {
    if (!spr) return;
    const u32* buffer = (const u32*)spr->pixel_data;
    int i, j;
    for (j = y; j < (y + hpart); j++) {
        for (i = x; i < (x + wpart); i++) {
            drawPixel(i, j, buffer[(i - x + xbeg) + ((j - y + ybeg) * spr->width * spr->frames)]);
        }
    }
}

static void drawFlipBuffers() {
    DCFlushRange(screenBuf + scr_buf0_size, scr_buf1_size);
    DCFlushRange(screenBuf, scr_buf0_size);
    OSScreenFlipBuffersEx(0);
    OSScreenFlipBuffersEx(1);
    cur_buf1 = !cur_buf1;
}

static void drawClearBuffers() {
    int i;
    for (i = 0; i < 2; i++) {
        OSScreenClearBufferEx(0, 0);
        OSScreenClearBufferEx(1, 0);
        drawFlipBuffers();
    }
}

static int drawFontInit(const u8* fontbuf, FT_Long bufsize) {
    FT_Long size;
    font = fontbuf;
    size = bufsize;

    FT_Error error;
    error = FT_Init_FreeType(&ftlib);
    if (error) return 0;

    error = FT_New_Memory_Face(ftlib, font, size, 0, &ftface);
    if (error) return 0;

    error = FT_Set_Pixel_Sizes(ftface, 0, 32);
    if (error) return 0;

    return 1;
}

static void drawFontBitmap(FT_Bitmap* bitmap, FT_Int x, FT_Int y, Color color) {
    FT_Int i, j, p, q;
    FT_Int x_max;
    FT_Int y_max = y + bitmap->rows;

    switch(bitmap->pixel_mode) {
        case FT_PIXEL_MODE_GRAY: {
            x_max = x + bitmap->width;
            for (i = x, p = 0; i < x_max; i++, p++) {
                for (j = y, q = 0; j < y_max; j++, q++) {
                    if (i < 0 || j < 0 || i >= DISPLAY_WIDTH || j >= DISPLAY_HEIGHT) continue;
                    u8 col = bitmap->buffer[q * bitmap->pitch + p];
                    if (col == 0) continue;
                    drawPixel(i, j, color);
                }
            }
            break;
        }
        case FT_PIXEL_MODE_LCD: {
            x_max = x + bitmap->width / 3;
            for (i = x, p = 0; i < x_max; i++, p++) {
                for (j = y, q = 0; j < y_max; j++, q++) {
                    if (i < 0 || j < 0 || i >= DISPLAY_WIDTH || j >= DISPLAY_HEIGHT) continue;
                    u8 cr = bitmap->buffer[q * bitmap->pitch + p * 3];
                    u8 cg = bitmap->buffer[q * bitmap->pitch + p * 3 + 1];
                    u8 cb = bitmap->buffer[q * bitmap->pitch + p * 3 + 2];

                    if ((cr | cg | cb) == 0) continue;
                    drawPixel(i, j, RGBA8(cr, cg, cb, 255));
                }
            }
            break;
        }
    }
}

static void drawTextAux(int x, int y, char* str, int w, Color color) {
    // TODO: fix text vertical align on wiiu version
    if (!font) return;

    FT_GlyphSlot slot = ftface->glyph;
    FT_Error error;
    int pen_x = x, pen_y = y;
    FT_UInt previous_glyph = 0;

    while(*str) {
        uint32_t buf = *str++;

        if ((buf >> 6) == 3) {
            if ((buf & 0xF0) == 0xC0) {
                u8 b1 = buf & 0xFF, b2 = *str++;
                if ((b2 & 0xC0) == 0x80) b2 &= 0x3F;
                buf = ((b1 & 0xF) << 6) | b2;
            } else if ((buf & 0xF0) == 0xD0) {
                u8 b1 = buf & 0xFF, b2 = *str++;
                if ((b2 & 0xC0) == 0x80) b2 &= 0x3F;
                buf = 0x400 | ((b1 & 0xF) << 6) | b2;
            } else if ((buf & 0xF0) == 0xE0) {
                u8 b1 = buf & 0xFF, b2 = *str++, b3 = *str++;
                if ((b2 & 0xC0) == 0x80) b2 &= 0x3F;
                if ((b3 & 0xC0) == 0x80) b3 &= 0x3F;
                buf = ((b1 & 0xF) << 12) | (b2 << 6) | b3;
            }
        } else if (buf & 0x80) {
            str++;
            continue;
        }

        if (buf == '\n') {
            pen_y += (ftface->size->metrics.height >> 6);
            pen_x = x;
            continue;
        }

        FT_UInt glyph_index = FT_Get_Char_Index(ftface, buf);

        if (FT_HAS_KERNING(ftface)) {
            FT_Vector vector;
            FT_Get_Kerning(ftface, previous_glyph, glyph_index, FT_KERNING_DEFAULT, &vector);
            pen_x += (vector.x >> 6);
        }

        error = FT_Load_Glyph(ftface, glyph_index, FT_LOAD_DEFAULT);
        if (error) continue;

        error = FT_Render_Glyph(ftface->glyph, FT_RENDER_MODE_NORMAL);
        if (error) continue;

        if ((pen_x + (slot->advance.x >> 6)) > x + w) {
            pen_y += (ftface->size->metrics.height >> 6);
            pen_x = x;
        }

        drawFontBitmap(&slot->bitmap, pen_x + slot->bitmap_left, (ftface->height >> 6) + pen_y - slot->bitmap_top, color);

        pen_x += (slot->advance.x >> 6);
        previous_glyph = glyph_index;
    }
}

void drawInit() {
    OSScreenInit();
    scr_buf0_size = OSScreenGetBufferSizeEx(0);
    scr_buf1_size = OSScreenGetBufferSizeEx(1);
    screenBuf = MEM1_alloc(scr_buf0_size + scr_buf1_size, 0x100);
    OSScreenSetBufferEx(0, screenBuf);
    OSScreenSetBufferEx(1, (screenBuf + scr_buf0_size));
    OSScreenEnableEx(0, 1);
    OSScreenEnableEx(1, 1);
    drawClearBuffers();

    u32* screen2 = (u32*)screenBuf + scr_buf0_size;
    OSScreenPutPixelEx(1, 0, 0, 0xABCDEFFF);

    cur_buf1 = screen2[0] != 0xABCDEFFF;

    if (!drawFontInit(alterebo_ttf, alterebo_ttf_size)) font = NULL;
}

void drawFini() {
    FT_Done_Face(ftface);
    FT_Done_FreeType(ftlib);
    MEM1_free(screenBuf);
    screenBuf = NULL;
}

void drawClearColor(Color color) {
    OSScreenClearBufferEx(0, color);
    OSScreenClearBufferEx(1, color);
}

void drawBackground() {
    OSScreenClearBufferEx(0, 0xC0C0C0FF);
    OSScreenClearBufferEx(1, 0xC0C0C0FF);
}

void drawLogo() {
    drawTexture(&logo, (DISPLAY_WIDTH>>1) - (logo.width>>1), 96);
}

void drawText(Color color, int x, int y, const char* str) {
    char buffer[256];
    strcpy(buffer, str);
    drawTextAux(x, y, buffer, DISPLAY_WIDTH, color);
}

void drawTextFormat(Color color, int x, int y, const char* str, ...) {
    char buffer[256];
    va_list valist;
    va_start(valist, str);
    vsnprintf(buffer, 255, str, valist);
    drawTextAux(x, y, buffer, DISPLAY_WIDTH, color);
    va_end(valist);
}

void drawTextCenter(Color color, int x, int y, const char* str) {
    // TODO: fix this
    char buffer[256];
    strcpy(buffer, str);
    drawTextAux(x - (((int)strlen(str)<<4)>>2), y, buffer, DISPLAY_WIDTH, color);
}

void drawTextMultiline(Color color, int x, int y, int w, int centre, const char* str) {
    char buffer[256];
    strcpy(buffer, str);
    drawTextAux(centre ? (x - (((int)strlen(str)<<4)>>2)) : x, y, buffer, w, color);
}

void drawRectangle(int x1, int y1, int x2, int y2, Color color, int fill) {
    if (fill) {
        int X1, X2, Y1, Y2, i, j;
        if (x1 < x2) {
            X1 = x1;
            X2 = x2;
        } else {
            X1 = x2;
            X2 = x1;
        }
        if (y1 < y2) {
            Y1 = y1;
            Y2 = y2;
        } else {
            Y1 = y2;
            Y2 = y1;
        }
        for (i = X1; i <= X2; i++) {
            for (j = Y1; j <= Y2; j++) {
                drawPixel(i, j, color);
            }
        }
    } else {
        drawLine(x1, y1, x2, y1, color);
        drawLine(x2, y1, x2, y2, color);
        drawLine(x1, y2, x2, y2, color);
        drawLine(x1, y1, x1, y2, color);
    }
}

void drawSprite(int sprite, int frame, int x, int y) {
    const sprite_t* spr = getSprite(sprite);
    drawTexturePart(spr, x, y, frame<<5, 0, spr->width, spr->height);
}

void drawTile(int tile, int x, int y) {
    drawTexturePart(&tileset, x, y, tile<<5, 0, 32, 32);
}

// Renderiza o tilemap do nível
static void drawTilemap() {
    int x, y;
    int ox = level.cam.scx >> 5;
    int oy = level.cam.scy >> 5;
    for (y = oy; y < oy+30 && y < TILE_ROW; y++) {
        for (x = ox; x < ox+50 && x < TILE_ROW; x++) {
            int i = level.tilemap[x + y*TILE_ROW] - 1;
            if (i < 0) continue;
            drawTile(i, (x<<5) - level.cam.scx, (y<<5) - level.cam.scy);
        }
    }
}

// Função auxiliar que renderiza um objeto
static void drawEntityAux(void* data) {
    entity_t* ent = (entity_t*)data;
    const sprite_t* spr = getSprite(ent->sprite);
    if (!ent || !ent->visible || !spr) return;

    if (ent->frame >= spr->frames)
        ent->frame = ((int)ent->frame) % spr->frames;

    drawTexturePart(spr, ent->x - level.cam.scx, ent->y - level.cam.scy, ((int)ent->frame)<<5, 0, spr->width, spr->height);
}

// Percorre a lista de objetos e renderiza cada um usando a função acima
static void drawEntities() {
    int i;
    for (i = 0; i < ENT_COUNT; i++) {
        foreach(getEntityList(i), drawEntityAux);
    }
}

void drawSetBackground(const char* fname) {
    // STUB
}

void drawLevel() {
    drawBackground();
    drawTilemap();
    drawEntities();
}

void draw() {
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
    drawFlipBuffers();
}
