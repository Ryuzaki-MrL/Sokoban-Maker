#ifndef UTIL_H
#define UTIL_H

#define MIN(x,y) ((x < y) ? x : y)
#define MAX(x,y) ((x > y) ? x : y)

void arraySort(unsigned arr[], unsigned n);

void initUserFolder();
const char* getUserPath();
const char* getSavePath();
const char* getLevelPath();

char* strhex(char* out, const char* str, int len);

void logs(const char* msg);
void logsf(const char* msg, ...);

int rectangleCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);

#endif // UTIL_H