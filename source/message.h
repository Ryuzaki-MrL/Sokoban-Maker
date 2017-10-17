#ifndef MESSAGE_H
#define MESSAGE_H

#define MSG_NULL       -1
#define MSG_NEWGAME     0
#define MSG_CONTINUE    1
#define MSG_RANKING     2
#define MSG_LANGUAGE    3
#define MSG_EDITOR      4
#define MSG_EXIT        5
#define MSG_BESTTIMES   6
#define MSG_LEVEL       7
#define MSG_RESUME      8
#define MSG_RESTART     9
#define MSG_TIME        10
#define MSG_COMPLETE    11
#define MSG_GAMEOVER    12
#define MSG_NEWLEVEL    13
#define MSG_LOADLEVEL   14
#define MSG_PLAYLEVEL   15
#define MSG_SAVELEVEL   16
#define MSG_CANCEL      17
#define MSG_GOBACK      18
#define MSG_TITLE       19
#define MSG_AUTHOR      20
#define MSG_LEVELSAVED  21

int setMessageLanguage(int newlang);
const char* getMessage(int msg);

int getLangCount();
const char* getLangString();

#endif // MESSAGE_H