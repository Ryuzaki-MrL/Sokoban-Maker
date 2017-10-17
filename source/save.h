#ifndef SAVE_H
#define SAVE_H

void newGame();
int saveGame();
int loadGame();
int deleGame();

int getLevel();
void setLevel(int lvl);

int getLanguage();
void setLanguage(int lang);

int isGameClear();

const unsigned* getBestTimes();
void setBestTime(int lvl, unsigned t);

#endif // SAVE_H