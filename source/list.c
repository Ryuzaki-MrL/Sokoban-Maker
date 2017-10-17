/*

Arquivo: list.c
Descrição: minha implementação de lista simplesmente encadeada

*/

#include <string.h>

#include "list.h"
#include "level.h"

struct sNode {
    void* data;
    struct sNode* next;
    //struct sNode* prev;
};

// Retorne o node na posição pos
static node_t* getNode(const list_t* list, int pos) {
    if (pos==-1)
        return list->last;
    node_t* current = list->first;
    size_t offs = 0;
    while(current) {
        if (offs++ == pos)
            return current;
        current = current->next;
    }
    return NULL;
}

// Retorna o dado do node na posição pos
void* getNodeData(const list_t* list, int pos) {
    if (pos==-1 && list->last)
        return list->last->data;
    node_t* current = list->first;
    size_t offs = 0;
    while(current) {
        if (offs++ == pos)
            return current->data;
        current = current->next;
    }
    return NULL;
}

// Procura por um dado na lista, utilizando a
// função "finder" passada pelo cliente
void* search(const list_t* list, fnd finder, int* args) {
    node_t* current = list->first;
    while(current) {
        if (finder(current->data, args))
            return current->data;
        current = current->next;
    }
    return NULL;
}

// Retorna a posição do node encontrado pela
// função "finder" passada pelo cliente
int getNodePos(const list_t* list, fnd finder, int* args) {
    node_t* current = list->first;
    size_t offs = 0;
    while(current) {
        if (finder(current->data, args))
            return offs;
        current = current->next;
        offs++;
    }
    return -1;
}

// Ordena a lista utilizando MergeSort e a
// função "comparator" passada pelo cliente
void sortList(list_t* list, cmp comparator) {
    if (list->size < 2) return;

    void* dataarr[list->size];
    node_t* current = list->first;
    int i = 0;
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
            int j, k;
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

// Percorre cada elemento da lista e executa a
// função "function" passando o elemento como
// parâmetro
void foreach(const list_t* list, bnd function) {
    node_t* current = list->first;
    while(current) {
        function(current->data);
        current = current->next;
    }
}

// Insere um dado "data" na posição "pos" da lista "list"
int insertAt(list_t* list, int pos, void* data) {
    node_t* newnode = (node_t*)calloc(1, sizeof(node_t));
    if (!newnode) return 0;

    newnode->data = data;
    if (!list->size) {
        list->first = newnode;
        list->last = newnode;
    } else if (pos==0) {
        //newnode->prev = NULL;
        newnode->next = list->first;
        //list->first->prev = newnode;
        list->first = newnode;
    } else if (pos==list->size-1 || pos==-1) {
        newnode->next = NULL;
        //newnode->prev = list->last;
        list->last->next = newnode;
        list->last = newnode;
    } else {
        node_t* targ = (node_t*)getNode(list, pos-1);
        if (targ) {
            //newnode->prev = targ;
            newnode->next = targ->next;
            //if (targ->next)
                //targ->next->prev = newnode;
            targ->next = newnode;
        } else {
            return 0;
        }
    }

    list->size++;
    return 1;
}

// Remove um elemento na posição "pos" da lista "list"
int removeAt(list_t* list, int pos, void** out) {
    if (!list->size) return 0;

    node_t* tofree = NULL;
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
    size_t offs = 1;
    while(current) {
        if (current->next) {
            if ((pos==list->size-1 && !current->next->next) || (offs++ == pos)) {
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

// Limpa a lista
void clearList(list_t* list) {
    node_t* current = list->first;
    node_t* next;
    while(current) {
        next = current->next;
        free(current);
        current = next;
    }
    list->size = 0;
    list->first = list->last = NULL;
}

// Copia a lista "src" para a lista "dst"
int copyList(const list_t* src, list_t* dst, size_t datasize) {
    if (!src || !src->size || !dst || src==dst) return 0;

    // Limpa a lista de destino primeiro
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
