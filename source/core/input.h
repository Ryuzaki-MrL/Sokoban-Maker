#ifndef INPUT_H
#define INPUT_H

#define MBT_NONE   0
#define MBT_LEFT   1
#define MBT_RIGHT  2

#define KEY_ANY   -1

#ifdef _3DS
    #include <3ds.h>
    #define KEY_CANCEL  KEY_B
    #define KEY_CONFIRM (KEY_A|KEY_START)
    #define KEY_EXTRA   (KEY_L|KEY_R)
#elif defined(__wiiu__)
    #include "wiiu.h"
    #define KEY_LEFT    (VPAD_BUTTON_LEFT|VPAD_STICK_L_EMULATION_LEFT|VPAD_STICK_R_EMULATION_LEFT)
    #define KEY_RIGHT   (VPAD_BUTTON_RIGHT|VPAD_STICK_L_EMULATION_RIGHT|VPAD_STICK_R_EMULATION_RIGHT)
    #define KEY_UP      (VPAD_BUTTON_UP|VPAD_STICK_L_EMULATION_UP|VPAD_STICK_R_EMULATION_UP)
    #define KEY_DOWN    (VPAD_BUTTON_DOWN|VPAD_STICK_L_EMULATION_DOWN|VPAD_STICK_R_EMULATION_DOWN)
    #define KEY_CANCEL  VPAD_BUTTON_B
    #define KEY_CONFIRM (VPAD_BUTTON_A|VPAD_BUTTON_PLUS)
    #define KEY_X       VPAD_BUTTON_X
    #define KEY_EXTRA   (VPAD_BUTTON_ZL|VPAD_BUTTON_ZR)
#else
    #include <allegro5/allegro.h>
    #define KEY_LEFT    ALLEGRO_KEY_LEFT
    #define KEY_RIGHT   ALLEGRO_KEY_RIGHT
    #define KEY_UP      ALLEGRO_KEY_UP
    #define KEY_DOWN    ALLEGRO_KEY_DOWN
    #define KEY_CANCEL  ALLEGRO_KEY_ESCAPE
    #define KEY_CONFIRM ALLEGRO_KEY_ENTER
    #define KEY_EXTRA   ALLEGRO_KEY_LCTRL
#endif

typedef void(*clb)(void);
typedef void(*qcb)(int);
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

void getUserInput(int size, const char* caption, icb callback);
void error(const char* caption, clb callback);
//void question(const char* question, qcb callback);

void updateUserInput();
void updateError();
void drawUserInput();
void drawError();

#endif // INPUT_H