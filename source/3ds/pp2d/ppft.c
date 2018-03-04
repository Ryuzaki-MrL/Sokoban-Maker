/*
  This is my personal extension of pp2d for using
  external fonts. It uses a custom format.
*/

#include "ppft.h"
#include "pp2d.h"

#define TEXTURE_FONT_START 1000

#pragma pack(1)

typedef struct {
    char magic[4]; // PPFT
    uint8_t version;
    uint16_t lineHeight;
    uint16_t baseline;
    uint16_t npages;
} ppftHeader;

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
    int16_t xoffs;
    int16_t yoffs;
    int16_t advance;
} ppftChar;

typedef struct {
    char fname[40];
    size_t texture;
    ppftChar chr[0x100];
} ppftPage;

struct s_ppftfont {
    ppftHeader hdr;
    ppftPage pages[];
};

static size_t texcount = 0;

ppftFont* ppft_load_font(const char* fname) {
    FILE* file = fopen(fname, "rb");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    ppftFont* font = (ppftFont*)malloc(size);
    fread(font, 1, size, file);
    fclose(file);

    int i;
    char pgname[256] = "romfs:/";
    for (i = 0; i < font->hdr.npages; ++i) {
        font->pages[i].texture = TEXTURE_FONT_START + (texcount++);
        sprintf(pgname+7, font->pages[i].fname);
        pp2d_load_texture_png(font->pages[i].texture, pgname);
    }

    return font;
}

void ppft_free_font(ppftFont* font) {
    int i;
    for (i = 0; i < font->hdr.npages; ++i) {
        pp2d_free_texture(font->pages[i].texture);
    }
    free(font);
}

float ppft_get_text_width(ppftFont* font, float scale, const char* text) {
    float w = 0, mw = 0;
    while(*text) {
        u32 c;
        int units = decode_utf8(&c, (const u8*)text);
		if (units == -1) break;
        text += units;
        if (c == '\n') {
            if (w > mw) mw = w;
            w = 0;
            text -= (c == '\n') ? 0 : 1;
        } else {
            w += font->pages[0].chr[c].advance * scale;
        }
    }
    return ((w > mw) ? w : mw);
}

static void ppft_word_wrap(ppftFont* font, float scale, int wrap, char* text) {
    int i, wpos = 0, len = 0, lastlen = 0;
    if (!wrap) return;
    for (i = 0; text[i]; ++i) {
        if (text[i] == '\n') {
            len = 0;
            continue;
        }
        if (text[i] == ' ') {
            wpos = i;
            lastlen = len;
            continue;
        }
        ppftChar chr = font->pages[0].chr[(int)text[i]];
        len += chr.w * scale;
        if (len > wrap) {
            text[wpos] = '\n';
            len -= lastlen;
        }
    }
}

static void ppft_draw_text_internal(ppftFont* font, int x, int y, float scaleX, float scaleY, u32 color, int center, const char* text) {
    //y += font->hdr.baseline;
    if (center) {
        x -= ((int)ppft_get_text_width(font, scaleX, text) >> 1);
    }
    int xx = x, yy = y;
    while(*text) {
        u32 c;
        int units = decode_utf8(&c, (const u8*)text);
		if (units == -1) break;
        text += units;
        if (c == '\n') {
            yy += font->hdr.lineHeight * scaleY;
            xx = x;
            text -= (c == '\n') ? 0 : 1;
        }
        ppftChar chr = font->pages[0].chr[c];
        if (!chr.w) continue;
        pp2d_texture_select_part(font->pages[0].texture, xx + chr.xoffs, yy + chr.yoffs, chr.x, chr.y, chr.w, chr.h);
        pp2d_texture_blend(color);
        pp2d_texture_scale(scaleX, scaleY);
        pp2d_texture_draw();
        xx += chr.advance * scaleX;
    }
}

void ppft_draw_text(ppftFont* font, int x, int y, float scaleX, float scaleY, u32 color, int wrapX, int center, const char* text) {
    char buffer[256];
    strncpy(buffer, text, 255);
    ppft_word_wrap(font, scaleX, wrapX, buffer);
    ppft_draw_text_internal(font, x, y, scaleX, scaleY, color, center, buffer);
}

void ppft_draw_textf(ppftFont* font, int x, int y, float scaleX, float scaleY, u32 color, int wrapX, int center, const char* text, ...) {
    char buffer[256];
	va_list args;
	va_start(args, text);
	vsnprintf(buffer, 255, text, args);
    ppft_word_wrap(font, scaleX, wrapX, buffer);
	ppft_draw_text_internal(font, x, y, scaleX, scaleY, color, center, buffer);
	va_end(args);
}
