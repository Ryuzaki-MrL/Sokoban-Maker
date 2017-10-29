#ifndef UTIL_H
#define UTIL_H

void initUserFolder();
const char* getUserPath();
const char* getLevelPath();

char* strhex(char* out, const char* str, int len);

int rectangleCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);

#endif // UTIL_H