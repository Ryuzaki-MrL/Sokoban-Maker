#ifndef STATE_H
#define STATE_H

#define ST_TITLE  0
#define ST_MAIN   1
#define ST_PAUSE  2
#define ST_RANK   3
#define ST_EDITOR 4
#define ST_LEVELS 5

#define H_NONE    0
#define H_MESSAGE 1
#define H_INPUT   2

#include "levellist.h"
#include "editor.h"
#include "maingame.h"
#include "titlescreen.h"
#include "pause.h"

extern int state;
extern int hud;
extern int running;

#endif // STATE_H