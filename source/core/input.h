#ifndef INPUT_H
#define INPUT_H

#include "keydefs.h"

#define MBT_NONE   0
#define MBT_LEFT   1
#define MBT_RIGHT  2

#define KEY_ANY   -1

typedef void(*icb)(const char*);

#if defined(_3DS) || defined(__wiiu__)
    void updateInput();
#else
    void updateInput(ALLEGRO_EVENT* event);
#endif

int isKeyDown(int key);
int isKeyHeld(int key);
int isKeyUp(int key);

int getMouseButton();
int getMouseX();
int getMouseY();

void getUserInput(int size, const char* caption, int password, icb callback);

void updateUserInput();
void drawUserInput();

#endif // INPUT_H