#include <string.h>

#include "list.h"
#include "level.h"

struct sNode {
    void* data;
    struct sNode* next;
};

static node_t* getNode(const list_t* list, int pos) {
    if (pos==-1)
        return list->last;
    node_t* current = list->first;
    int offs = 0;
    while(current) {
        if (offs++ == pos)
            return current;
        current = current->next;
    }
    return NULL;
}

void* getNodeData(const list_t* list, int pos) {
    if (pos==-1 && list->last)
        return list->last->data;
    node_t* current = list->first;
    int offs = 0;
    while(current) {
        if (offs++ == pos)
            return current->data;
        current = current->next;
    }
    return NULL;
}

void* search(const list_t* list, fnd finder, const void* args) {
    node_t* current = list->first;
    while(current) {
        if (finder(current->data, args))
            return current->data;
        current = current->next;
    }
    return NULL;
}

int getNodePos(const list_t* list, fnd finder, const void* args) {
    node_t* current = list->first;
    int offs = 0;
    while(current) {
        if (finder(current->data, args))
            return offs;
        current = current->next;
        offs++;
    }
    return -1;
}

void sortList(list_t* list, cmp comparator) {
    if (list->size < 2) return;

    void* dataarr[list->size];
    node_t* current = list->first;
    size_t i = 0;
    while(current) {
        dataarr[i++] = current->data;
        current = current->next;
    }

    size_t curr_size, left;
    size_t n = list->size;
    for (curr_size = 1; curr_size <= n-1; curr_size <<= 1) {
        for (left = 0; left < n-1; left += curr_size<<1) {
            size_t mid = left + curr_size - 1;
            size_t right = MIN(left + 2*curr_size - 1, n-1);
            size_t j, k;
            size_t n1 = mid - left + 1;
            size_t n2 = right - mid;
            void* L[n1];
            void* R[n2];

            for (i = 0; i < n1; i++) L[i] = dataarr[left + i];
            for (j = 0; j < n2; j++) R[j] = dataarr[mid + 1 + j];

            i = j = 0;
            k = left;
            while (i < n1 && j < n2) {
                if (comparator(L[i], R[j])) {
                    dataarr[k++] = L[i++];
                } else {
                    dataarr[k++] = R[j++];
                }
            }

            while (i < n1) dataarr[k++] = L[i++];
            while (j < n2) dataarr[k++] = R[j++];
        }
    }

    current = list->first;
    i = 0;
    while(current) {
        current->data = dataarr[i++];
        current = current->next;
    }
}

void foreach(const list_t* list, bnd function) {
    node_t* current = list->first;
    while(current) {
        function(current->data);
        current = current->next;
    }
}

int insertAt(list_t* list, int pos, void* data) {
    node_t* newnode = (node_t*)calloc(1, sizeof(node_t));
    if (!newnode) return 0;

    newnode->data = data;
    if (!list->size) {
        list->first = newnode;
        list->last = newnode;
    } else if (pos==0) {
        newnode->next = list->first;
        list->first = newnode;
    } else if ((size_t)pos == list->size-1 || pos == -1) {
        newnode->next = NULL;
        list->last->next = newnode;
        list->last = newnode;
    } else {
        node_t* targ = (node_t*)getNode(list, pos-1);
        if (targ) {
            newnode->next = targ->next;
            targ->next = newnode;
        } else {
            return 0;
        }
    }

    list->size++;
    return 1;
}

int removeAt(list_t* list, int pos, void** out) {
    if (!list->size) return 0;

    if (pos == 0) {
        node_t* next = list->first->next;
        if (out) *out = list->first->data;
        else free(list->first->data);
        free(list->first);
        if (list->first == list->last)
            list->last = next;
        list->first = next;
        list->size--;
        return 1;
    }

    node_t* current = list->first;
    int offs = 1;
    while(current) {
        if (current->next) {
            if (((size_t)pos == list->size-1 && !current->next->next) || (offs++ == pos)) {
                node_t* next = current->next->next;
                if (out) *out = current->next->data;
                else free(current->next->data);
                free(current->next);
                if (current->next == list->last)
                    list->last = current;
                current->next = next;
                list->size--;
                return 1;
            }
        }
        current = current->next;
    }
    return 0;
}

void clearList(list_t* list) {
    node_t* current = list->first;
    node_t* next;
    while(current) {
        next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
    list->size = 0;
    list->first = list->last = NULL;
}

int copyList(const list_t* src, list_t* dst, size_t datasize) {
    if (!src || !src->size || !dst || src==dst) return 0;

    clearList(dst);

    dst->first = (node_t*)calloc(1, sizeof(node_t));
    if (!dst->first) return 0;

    node_t* s = src->first;
    node_t* d = dst->first;

    while(s) {
        dst->size++;
        d->data = calloc(1, datasize);
        if (!d->data) continue;
        memcpy(d->data, s->data, datasize);
        s = s->next;
        if (s) {
            d->next = (node_t*)calloc(1, sizeof(node_t));
            d = d->next;
        } else {
            d->next = NULL;
            dst->last = d;
        }
    }

    return (dst->size == src->size);
}
