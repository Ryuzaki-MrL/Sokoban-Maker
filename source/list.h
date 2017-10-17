#ifndef LIST_H
#define LIST_H

#define MIN(x,y) ((x < y) ? x : y)

#include <stdlib.h>

typedef int(*fnd)(const void*, int*);
typedef void(*bnd)(void*);
typedef int(*cmp)(const void*, const void*);

typedef struct sNode node_t;

typedef struct sList {
    node_t* first;
    node_t* last;
    size_t size;
} list_t;

void* getNodeData(const list_t* list, int pos);
void* search(const list_t* list, fnd finder, int* args);
int getNodePos(const list_t* list, fnd finder, int* args);
void sortList(list_t* list, cmp comparator);
void foreach(const list_t* list, bnd function);
int insertAt(list_t* list, int pos, void* data);
int removeAt(list_t* list, int pos, void** out);
void clearList(list_t* list);
int copyList(const list_t* src, list_t* dst, size_t datasize);

#endif // LIST_H
