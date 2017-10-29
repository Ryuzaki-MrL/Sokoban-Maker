#ifndef LEVELLIST_H
#define LEVELLIST_H

#define LISTMODE_DOWNLOAD   0
#define LISTMODE_MYLEVELS   1
#define LISTMODE_LOAD       0
#define LISTMODE_UPLOAD     1

void openUserLevelList(int mode);
void openOnlineLevelList(int mode);

void updateLevelList();
void drawLevelList();

#endif // LEVELLIST_H