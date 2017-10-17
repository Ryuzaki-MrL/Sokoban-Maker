#ifndef STATE_H
#define STATE_H

#include <allegro5/allegro.h>

#define ST_TITLE  0
#define ST_MAIN   1
#define ST_PAUSE  2
#define ST_RANK   3
#define ST_EDITOR 4
#define ST_LEVELS 5

#define H_MESSAGE 1
#define H_INPUT   2

#define MB_NONE   0
#define MB_LEFT   1
#define MB_RIGHT  2

#define KB_LEFT   ALLEGRO_KEY_LEFT
#define KB_RIGHT  ALLEGRO_KEY_RIGHT
#define KB_UP     ALLEGRO_KEY_UP
#define KB_DOWN   ALLEGRO_KEY_DOWN
#define KB_CANCEL ALLEGRO_KEY_ESCAPE
#define KB_PAUSE  ALLEGRO_KEY_ENTER

#define DISPLAY_WIDTH 640
#define DISPLAY_HEIGHT 480

extern int state;
extern int hud;
extern int running;
extern int cursor;
extern int screen;

extern int kdown[ALLEGRO_KEY_MAX];
extern int kheld[ALLEGRO_KEY_MAX];
extern int keyany;
extern int unichar;
extern int mouse_b;
extern int mouse_x;
extern int mouse_y;

extern char* userinput;

extern char userpath[260];

#endif // STATE_H
