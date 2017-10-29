#ifndef EDITOR_H
#define EDITOR_H

#include "level.h"

void editorStart(levelmeta_t* meta);
void editorExit(int commit);

void updateLevelEditor();
void drawLevelEditor();

void editorCommitLevel();
void editorPlayLevel();
void editorStopLevel();

#endif // EDITOR_H