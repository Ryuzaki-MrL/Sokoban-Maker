#include <string.h>

#include "stack.h"

struct sStack {
    void* data;
    size_t size;
    size_t datasize;
    int top;
};

stack_t* initStack(size_t size, size_t datasize) {
    stack_t* s = (stack_t*)calloc(1, sizeof(stack_t));
    s->data = calloc(size, datasize);
    s->size = size;
    s->datasize = datasize;
    s->top = -1;
    return s;
}

size_t getStackSize(stack_t* stack) {
    return stack->size;
}

int push(stack_t* stack, void* data) {
    if ((size_t)(stack->top+1) >= stack->size)
        return 0;
    memcpy(stack->data + (++stack->top), data, stack->datasize);
    return 1;
}

int pop(stack_t* stack, void* out) {
    if (stack->top < 0 || !stack->size)
        return 0;
    if (out) {
        memcpy(out, stack->data + (stack->top--), stack->datasize);
    }
    return 1;
}

void clearStack(stack_t* stack) {
    if (!stack) return;
    stack->top = -1;
}

void destroyStack(stack_t* stack) {
    if (!stack) return;
    free(stack->data);
    free(stack);
}
