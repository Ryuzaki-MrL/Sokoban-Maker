#ifndef PPFT_H
#define PPFT_H

#include <3ds.h>
#include <stdarg.h>

typedef struct s_ppftfont ppftFont;

ppftFont* ppft_load_font(const char* fname);
void ppft_free_font(ppftFont* font);

float ppft_get_text_width(ppftFont* font, float scale, const char* text);

void ppft_draw_text(ppftFont* font, int x, int y, float scaleX, float scaleY, u32 color, int wrapX, int center, const char* text);
void ppft_draw_textf(ppftFont* font, int x, int y, float scaleX, float scaleY, u32 color, int wrapX, int center, const char* text, ...);

#endif /* PPFT_H */