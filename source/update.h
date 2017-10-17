#ifndef UPDATE_H
#define UPDATE_H

typedef void(*clb)(void);
typedef void(*qlb)(int);

void gotoState(int newstate);
void gotoScreen(int newscreen);
void getUserInput(char* dst, int size, const char* caption, clb callback);

void error(const char* caption, clb callback);
//void question(const char* question, qlb callback);

void update();

#endif // UPDATE_H