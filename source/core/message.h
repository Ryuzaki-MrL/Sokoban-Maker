#ifndef MESSAGE_H
#define MESSAGE_H

#define MSG_NULL       -1
#define MSG_NEWGAME     0
#define MSG_CONTINUE    1
#define MSG_RANKING     2
#define MSG_LANGUAGE    3
#define MSG_EDITOR      4
#define MSG_EXIT        5
#define MSG_LOGOUT      6
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
#define MSG_CONFIRM     18
#define MSG_TITLE       19
#define MSG_AUTHOR      20
#define MSG_LEVELSAVED  21
#define MSG_ONLINEHUB   22
#define MSG_DOWNLEVEL   23
#define MSG_UPLEVEL     24
#define MSG_MYLEVELS    25
#define MSG_MYPROFILE   26
#define MSG_DOWNLOADOK  27
#define MSG_DOWNLOADNOK 28
#define MSG_USERNAME    29
#define MSG_PASSWORD    30
#define MSG_YES         31
#define MSG_NO          32
#define QST_SAVELEVEL   33
#define QST_DELLEVEL    34
#define MSG_UNTESTED    35
#define MSG_VALID       36
#define MSG_UNTESTEDM   37
#define MSG_VALIDM      38

void setLanguage(int newlang);
int getLanguage();

const char* getMessage(int msg);
const char* getLangString();

#endif // MESSAGE_H