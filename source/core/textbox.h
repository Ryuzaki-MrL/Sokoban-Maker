#ifndef TEXTBOX_H
#define TEXTBOX_H

typedef void(*clb)(void);
typedef void(*qcb)(int);

void error(const char* caption, clb callback);
void question(const char* question, qcb callback);

void updateError();
void drawError();

void updateQuestion();
void drawQuestion();

#endif // TEXTBOX_H