#ifndef EDITOR_H
#define EDITOR_H

#include "level.h"

void editorStart(levelmeta_t* meta);
void editorExit(int commit);
void editorSaveQuit();

void updateLevelEditor();
void drawLevelEditor();

void editorCommitLevel();
void editorPlayLevel();
void editorStopLevel();
void editorValidateLevel();

#endif // EDITOR_H